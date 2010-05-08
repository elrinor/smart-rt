#ifndef __SMART_SCENE_H__
#define __SMART_SCENE_H__

#include "common.h"
#include "core/CoreScene.h"
#include "Model.h"
# if 0
namespace smart {
// -------------------------------------------------------------------------- //
// Scene
// -------------------------------------------------------------------------- //
  class Scene {
  public:
    void addModel(Model& model, const Matrix4f& transform) {
      mScene->newObject(model.mModel, transform);
    }

    void compile() {
      // mScene->compile();
    }

  private:
    friend class Smart;

    Scene(CoreScene* scene): mScene(scene) {}

    CoreScene* mScene;
  };
} // namespace smart
#endif
#endif // __SMART_SCENE_H__
