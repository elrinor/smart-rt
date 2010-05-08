#ifndef __SMART_COREOBJECT_H__
#define __SMART_COREOBJECT_H__

#include "common.h"
#include "ShadedModel.h"

namespace smart {

  /** CoreObject represents an instantiation of a model in a scene. 
   * It is defined by a model and a transformation matrix. */
  class CoreObject: public arx::nonassignable {
  public:
    const ShadedModel* getModel() const {
      return mModel;
    }

    ShadedModel* getModel() {
      return mModel;
    }

    /** @returns transformation from object local coordinates to world coordinates. */
    const Matrix4f& getLocalToWorldTransform() const {
      return mLocalToWorldTransform;
    }

    /** @returns transformation from world coordinates to object local coordinates. */
    const Matrix4f& getWorldToLocalTransform() const {
      return mWorldToLocalTransform;
    }

    ~CoreObject() {
      mModel->releaseOwnership();
    }

  private:
    friend class SmartCore;
    friend class CoreScene;

    /** Constructor.
     * 
     * @param model model to instantiate this object for. 
     * @param localToWorldTransform transformation from object local 
     *   coordinates to world coordinates. */
    CoreObject(ShadedModel* model, const Matrix4f& localToWorldTransform): 
      mModel(model), mLocalToWorldTransform(localToWorldTransform) {
      mModel->claimOwnership();
      localToWorldTransform.computeInverse(&mWorldToLocalTransform);
    }

    /** Transformation from object space to world space. */
    Matrix4f mLocalToWorldTransform;

    /** Transformation from world space to object space. */
    Matrix4f mWorldToLocalTransform;
    ShadedModel* mModel;
  };

} // namespace smart

#endif // __SMART_COREOBJECT_H__
