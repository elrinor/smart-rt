#ifndef __SMART_H__
#define __SMART_H__

#include "util/DisableMSVCWarnings.h"
#include <arx/Utility.h>
#include "core/SmartCore.h"
#include "Scene.h"
#include "Model.h"

namespace smart {
#if 0
  /** Smart - Smart Minimalistic Arx Ray Tracer. */
  class Smart: public arx::noncopyable {
  public:
    Smart() {}

    /** Creates a new scene. */
    Scene newScene() {
      return Scene(mCore.newScene());
    }

    /** Creates a new model. */
    Model newModel() {
      return Model(mCore.newModel());
    }

    /** Starts rendering of the given scene, locking it. After a call to 
     * this function all editing on the given scene will fail. */
    void startRendering(Scene& scene, arx::Image3f& target) {

    }

    /** Waits for the completion of rendering of the given scene.
     * Must be called after a call to startRendering. After completion of
     * rendering unlocks the given scene, making it editable again. */
    void endRendering(Scene& scene) {

    }
#if 0

    void freeScene(Scene& scene) {
      mScenes.remove(scene.mId);
      scene.invalidate();
    }

    void freeModel(Model& model) {
      mModels.remove(model.mId);
      model.invalidate();
    }

    void render(const Scene& scene, arx::Image3f& target) {
      const CameraShader& camShader = scene.getCameraShader();
      
      float dx = 1.0f / target.getWidth();
      float dy = 1.0f / target.getHeight();
      
      int x, y;
      float fx, fy;
      for(y = 0, fy = 0; y < target.getHeight(); ++y, fy += dy) {
        for(x = 0, fx = 0; x < target.getWidth(); ++x, fx += dx) {
          Ray ray;
          camShader(fx , fy, ray);

          Illumination illumination;
          Engine::trace(ray, scene, illumination);

          //Engine::trace(Ray(Vector3f(-1.9, -1.9, 5), Vector3f(0, 0, -1)), scene, illumination);

          target.setPixel(x, y, illumination.toColor3f());
        }
      }
    }
#endif

  private:
    friend class SmartAccessor;

    SmartCore mCore;
  };


#endif

} // namespace smart

#include "util/EnableMSVCWarnings.h"

#endif // __SMART_H__
