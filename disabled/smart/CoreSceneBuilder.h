#ifndef __SMART_CORESCENEBUILDER_H__
#define __SMART_CORESCENEBUILDER_H__

#include "common.h"
#include <arx/Utility.h>
#include "CoreScene.h"

namespace smart {
// -------------------------------------------------------------------------- //
// CoreSceneBuilder
// -------------------------------------------------------------------------- //
  class CoreSceneBuilder: public arx::noncopyable {
  public:
    CoreSceneBuilder(CoreScene* scene): mScene(scene) {
      assert(!mScene->mCompiled && !mScene->mBeingEdited);
      mScene->mBeingEdited = true;
    }

    ~CoreSceneBuilder() {
      if(mScene)
        endEditing();
    }

    void endEditing() {
      assert(mScene && mScene->mBeingEdited);
      mScene->mBeingEdited = false;
      mScene = NULL;
    }

    /** Adds the given model to the scene, applying the given transformation
     * to it. */
    void addModel(CoreModel* model, const Matrix4f& transform) {
      assert(mScene);
      mScene->mObjects.push_back(CoreObject(model, transform));
    }

  private:
    CoreScene* mScene;
  };

} // namespace smart

#endif // __SMART_CORESCENEBUILDER_H__