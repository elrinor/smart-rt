#ifndef __SMART_RENDERERMANAGER_H__
#define __SMART_RENDERERMANAGER_H__

#include "common.h"
#include <queue>
#include "RenderTask.h"
#include "RenderHandler.h"
#include "Renderer.h"

namespace smart {
// -------------------------------------------------------------------------- //
// RenderManager
// -------------------------------------------------------------------------- //
  class RenderManager: public arx::noncopyable {
  public:
    RenderManager() {
      mIsDestroying = false;
    }

    ~RenderManager() {
      mDataMutex.lock();

      /* Set destruction flag. */
      mIsDestroying = true;

      /* Wake everybody. */
      for(unsigned int i = 0; i < mRendererContexts.size(); i++)
        mRendererContexts[i].mRenderer->wake();

      mDataMutex.unlock();

      /* Destroy all renderers.
       * Note that destructors will wait for thread destruction. */
      for(unsigned int i = 0; i < mRendererContexts.size(); i++)
        delete mRendererContexts[i].mRenderer;
    }

    template<class Handler>
    void addRenderer(Handler renderHandler) {
      arx::mutex::scoped_lock lock(mDataMutex);

      RendererContext ctx;
      ctx.mCurrentlyRendering = NULL;
      ctx.mRenderer = new RendererType(renderHandler, NotificationConsumer(this));
      ctx.mRenderer->setId(static_cast<int>(mRendererContexts.size()));
      mRendererContexts.push_back(ctx);
    }

    void addRenderTask(RenderTask* renderTask) {
      arx::mutex::scoped_lock lock(mDataMutex);

      renderTask->getTiler()->nextTask(renderTask->getScene(), 
        renderTask->getImage(), static_cast<int>(mRendererContexts.size()));
      mTasks.push(renderTask);
      for(unsigned int i = 0; i < mRendererContexts.size(); i++)
        mRendererContexts[i].mToNotify.push_back(renderTask);

      if(mTasks.size() == 1)
        for(unsigned int i = 0; i < mRendererContexts.size(); i++)
          mRendererContexts[i].mRenderer->wake();
    }

  private:
    class NotificationConsumer;

    typedef Renderer<NotificationConsumer> RendererType;

    class NotificationConsumer {
    public:
      NotificationConsumer(RenderManager* manager): mManager(manager) {}

      void operator()(Renderer<NotificationConsumer>* renderer) {
        arx::mutex::scoped_lock lock(mManager->mDataMutex);
        if(mManager->mIsDestroying) {
          renderer->addSuicideJob();
        } else {
          int id = renderer->getId();

          RendererContext& ctx = mManager->mRendererContexts[id];
          for(unsigned int i = 0; i < ctx.mToNotify.size(); i++)
            renderer->addNewRenderTaskJob(ctx.mToNotify[i]);
          ctx.mToNotify.clear();

          if(ctx.mCurrentlyRendering != NULL)
            ctx.mCurrentlyRendering->tilesRendered(1);

          while(true) {
            ImageTile tile;
            RenderTask* currentTask = mManager->currentTask();
            if(currentTask != NULL) {
              if(currentTask->getTiler()->nextTile(id, tile)) {
                ctx.mCurrentlyRendering = currentTask;
                currentTask->tilesQueued(1);
                renderer->addRenderTileJob(currentTask, tile);
                break;
              } else {
                currentTask->noMoreTiles();
                mManager->mTasks.pop();
              }
            } else {
              ctx.mCurrentlyRendering = NULL;
              break;
            }
          }
        }
      }
    private:
      RenderManager* mManager;
    };

    RenderTask* currentTask() {
      /* We're called from locked context, so it's OK not to lock */
      return mTasks.size() == 0 ? NULL : mTasks.front();
    }

    struct RendererContext {
      Renderer<NotificationConsumer>* mRenderer;
      std::vector<RenderTask*>  mToNotify;
      RenderTask* mCurrentlyRendering;
    };

    friend class NotificationConsumer;

    std::queue<RenderTask*> mTasks;
    std::vector<RendererContext> mRendererContexts;
    
    bool mIsDestroying;

    arx::mutex mDataMutex;
  };


} // namespace smart

#endif // __SMART_RENDERERMANAGER_H__
