#ifndef __ARX_RENDERTASK_H__
#define __ARX_RENDERTASK_H__

#include "common.h"
#include <iostream>
#include <arx/Utility.h>
#include <arx/Collections.h>
#include <arx/Thread.h>
#include "RenderTiler.h"
#include "MemoryArena.h"
#include "ShadedScene.h"
#include "ShaderManager.h"

namespace smart {
// -------------------------------------------------------------------------- //
// RenderTask
// -------------------------------------------------------------------------- //
  class RenderTask: public arx::noncopyable {
  public:
    template<class Tiler>
    RenderTask(ShadedScene* scene, arx::Image3f& image, Tiler tiler) {
      mScene = scene;
      mImage = image;
      mTiler = new RenderTilerAdapter<Tiler>(tiler);

      mTilesQueued = 0;
      mTilesRendered = 0;
      mNoMoreTiles = false;

      mScene->claimOwnership();
      mEndLock.lock();
    }

    ~RenderTask() {
      delete mTiler;
      mScene->releaseOwnership();
    }

    ShadedScene* getScene() const {
      return mScene;
    }

    AbstractRenderTiler* getTiler() const {
      return mTiler;
    }

    arx::Image3f& getImage() {
      return mImage;
    }

  private:
    friend class RenderManager;
    friend class SmartCore;

    void endRendering() {
      /* Wait for being unlocked by the rendering thread. */
      mEndLock.lock();
      mEndLock.unlock();
    }

    /* These functions are called from locked context, so it's OK not to lock. */
    void tilesQueued(int count) {
      mTilesQueued += count;
    }

    void tilesRendered(int count) {
      mTilesRendered += count;
      if(mNoMoreTiles && mTilesQueued == mTilesRendered)
        mEndLock.unlock();
    }

    void noMoreTiles() {
      mNoMoreTiles = true;
      if(mTilesQueued == mTilesRendered)
        mEndLock.unlock();
    }

    ShadedScene* mScene;
    AbstractRenderTiler* mTiler; 
    arx::Image3f mImage; /**< This one has reference-counted implementation, so should store it by value... */
    arx::mutex mEndLock;
    arx::mutex mDataLock;

    int mTilesQueued;
    int mTilesRendered;
    bool mNoMoreTiles;
  };

} // namespace smart


#endif // __ARX_RENDERTASK_H__