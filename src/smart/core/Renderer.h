#ifndef __SMART_RENDERER_H__
#define __SMART_RENDERER_H__

#include "common.h"
#include <arx/Utility.h>
#include <arx/Thread.h>
#include "RenderTask.h"
#include "RenderHandler.h"
#include "Idded.h"
#include "ImageTile.h"

namespace smart {
// -------------------------------------------------------------------------- //
// Renderer
// -------------------------------------------------------------------------- //
  template<class NotificationConsumer>
  class Renderer: private arx::noncopyable, public Idded {
  public:
    template<class Handler>
    Renderer(Handler renderHandler, NotificationConsumer notificationConsumer):
      mNotificationConsumer(notificationConsumer) {
      mRenderHandler = new RenderHandlerAdapter<Handler>(renderHandler);
      mWaiting = false;

      mDeadLock.lock();
      mWaitLock.lock();
      mThread = new arx::thread(ThreadFunc(this));
    }

    ~Renderer() {
      /* Wait for thread death. */
      mDeadLock.lock();

      /* Detach. */
      delete mThread;

      /* Delete handler. */
      delete mRenderHandler;
    }

    void addSuicideJob() {
      Job job;
      job.mType = Job::SUICIDE;
      mJobs.push_back(job);
    }

    void addRenderTileJob(RenderTask* task, const ImageTile& tile) {
      Job job;
      job.mType = Job::RENDER_TILE;
      job.mTask = task;
      job.mTile = tile;
      mJobs.push_back(job);
    }

    void addNewRenderTaskJob(RenderTask* task) {
      Job job;
      job.mType = Job::NEW_RENDERTASK;
      job.mTask = task;
      mJobs.push_back(job);
    }

    void wake() {
      if(mWaiting) {
        mWaiting = false;
        mWaitLock.unlock();
      }
    }

  protected:
    class ThreadFunc {
    public:
      ThreadFunc(Renderer* renderer): mRenderer(renderer) {}

      void operator()() {
        while(true) {
          /* Notify our manager that we are free. */
          mRenderer->mNotificationConsumer(mRenderer);

          /* If nothing there - wait. */
          if(mRenderer->mJobs.size() == 0) {
            mRenderer->mWaiting = true;
            mRenderer->mWaitLock.lock();
            continue;
          }

          /* Our manager must have added some jobs for us. */
          for(unsigned int i = 0; i < mRenderer->mJobs.size(); i++) {
            const Job& task = mRenderer->mJobs[i];
            switch(task.mType) {
            case Job::SUICIDE:
              mRenderer->mDeadLock.unlock();
              return;
            case Job::NEW_RENDERTASK:
              mRenderer->mRenderHandler->taskAdded(task.mTask);
              break;
            case Job::RENDER_TILE:
              mRenderer->mRenderHandler->renderTile(task.mTask, task.mTile);
              break;
            default:
              Unreachable();
            }
          }

          /* Clear job queue. */
          mRenderer->mJobs.clear();
        }
      }

    private:
      Renderer* mRenderer;
    };

    friend class ThreadFunc;

  private:
    struct Job {
      enum Type {
        SUICIDE,
        RENDER_TILE,
        NEW_RENDERTASK
      };

      Type mType;
      RenderTask* mTask;
      ImageTile mTile;
    };

    NotificationConsumer mNotificationConsumer;
    AbstractRenderHandler* mRenderHandler;

    arx::thread* mThread;
    arx::mutex mDeadLock;
    arx::mutex mWaitLock;
    bool mWaiting;

    std::vector<Job> mJobs;
  };


} // namespace smart 

#endif // __SMART_RENDERER_H__
