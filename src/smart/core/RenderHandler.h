#ifndef __SMART_RENDERHANDLER_H__
#define __SMART_RENDERHANDLER_H__

#include "common.h"
#include "ImageTile.h"
#include "RenderTask.h"
#include "Tracer.h"

namespace smart {
// -------------------------------------------------------------------------- //
// RenderHandler
// -------------------------------------------------------------------------- //
  class RenderHandler {
  public:
    void taskAdded(RenderTask* task);

    void renderTile(RenderTask* task, const ImageTile& tile);
  };


// -------------------------------------------------------------------------- //
// LocalRenderHandler
// -------------------------------------------------------------------------- //
  class LocalRenderHandler {
  public:
    void taskAdded(RenderTask* task) {
      return;
    }

    void renderTile(RenderTask* task, const ImageTile& tile) {
      float hRec = 1.0f / task->getImage().getHeight();
      float wRec = 1.0f / task->getImage().getWidth();

      int y = tile.getY();
      float fy = y * hRec;
      for(; y < tile.getY() + tile.getHeight(); y++, fy += hRec) {
        int x = tile.getX();
        float fx = x * wRec;
        for(; x < tile.getX() + tile.getWidth(); x++, fx += wRec) {
          TraceContext ctx;
          ctx.setScene(task->getScene());
          ctx.setDepth(0);
          task->getScene()->getCameraShader()->initPrimaryRay(fx, fy, ctx);

          //ctx.setRay(Ray(Vector3f(0,0,0),Vector3f(i,2*i,3*i).normalized()));

          Tracer::trace(ctx);

          task->getImage().setPixel(x, y, ctx.getRadiance().toColor3f());
        }
      }
    }
  };



// -------------------------------------------------------------------------- //
// AbstractRenderHandler
// -------------------------------------------------------------------------- //
  class AbstractRenderHandler {
  public:
    virtual void taskAdded(RenderTask* task) = 0;

    virtual void renderTile(RenderTask* task, const ImageTile& tile) = 0;
  };


// -------------------------------------------------------------------------- //
// RenderHandlerAdapter
// -------------------------------------------------------------------------- //
  template<class Handler>
  class RenderHandlerAdapter: public AbstractRenderHandler {
  public:
    RenderHandlerAdapter(Handler handler): mHandler(handler) {}

    virtual void taskAdded(RenderTask* task) {
      mHandler.taskAdded(task);
    }

    virtual void renderTile(RenderTask* task, const ImageTile& tile) {
      mHandler.renderTile(task, tile);
    }

  private:
    Handler mHandler;
  };

} // namespace smart

#endif // __SMART_RENDERHANDLER_H__
