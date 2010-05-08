#ifndef __SMART_SCENEINTERNAL_H__
#define __SMART_SCENEINTERNAL_H__

#include "common.h"
#include <arx/Utility.h>
#include <arx/Collections.h>
#include "ExplicitlyCounted.h"
#include "CoreObject.h"
#include "Idded.h"

namespace smart {

  /** Class representing the whole rendering scene. */
  class CoreScene: private arx::noncopyable, private ExplicitlyCounted<CoreScene>, public Idded {
  public:
    ~CoreScene() {
      for(int i = 0; i < mObjects.size(); i++)
         mObjects[i]->~CoreObject();
    }

    const BoundingBox& getBoundingBox() const {
      return getObject(0)->getModel()->getBoundingBox(); // TODO
    }

    /** @returns an Object with the given index. */
    const CoreObject* getObject(int index) const {
      return mObjects[index];
    }

    /** @returns an Object with the given index. */
    CoreObject* getObject(int index) {
      return mObjects[index];
    }

    /** @returns the total number of Object instances in this scene. */
    const int getObjectCount() const {
      return mObjects.size();
    }

    /** Adds the given model to the scene, applying the given transformation
     * to it. */
    void newObject(ShadedModel* model, const Matrix4f& transform) {
      assert(!mCompiled);
      mObjects.push_back(new (mArena.allocate<CoreObject>(1)) CoreObject(model, transform));
    }

    void compile() {
      if(mCompiled)
        return;

      /* Compile all the underlying models. */
      for(int i = 0; i < mObjects.size(); i++)
        mObjects[i]->getModel()->compile();

      mCompiled = true;
    }

    bool isCompiled() const {
      return mCompiled;
    }

  private:
    friend class SmartCore;
    friend class ShadedScene;

    CoreScene() {
      initialize(16);
    }

    CoreScene(int objectCapacity) {
      initialize(objectCapacity);
    }

    /** Initializer. Called from constructors. */
    void initialize(int objectCapacity) {
      ExplicitlyCounted::initialize(this);
      mObjects.reserve(objectCapacity);
      mCompiled = false;
    }

    /** Array of objects in this scene. Note that scene owns all its objects,
     * and they are released together with the scene. */
    arx::CheckedArray<CoreObject*> mObjects;

    /** Memory arena for objects. */
    MemoryArena<MulDivAdd<3, 2, 0> > mArena;

    /** Is this scene compiled? */
    bool mCompiled;
  };

} // namespace smart


#endif // __SMART_SCENEINTERNAL_H__
