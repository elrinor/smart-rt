#ifndef __SMART_INTERSECTION_H__
#define __SMART_INTERSECTION_H__

#include "common.h"
#include "CoreModel.h"
#include "CoreObject.h"

namespace smart {

  /** Intersection class represents an intersection of a Ray with an arbitrary
   * triangle. */
  class Intersection {
  public:
    Intersection(): mValid(false) {}

    bool isValid() const {
      return mValid;
    }

    void setValid(bool valid) {
      mValid = valid;
    }

    const Vector3f& getBarycentricCoord() const {
      return mBarycentricCoord;
    }

    void setBarycentricCoord(float alpha, float beta, float gamma) {
      mBarycentricCoord = Vector3f(alpha, beta, gamma);
    }

    CoreModel::TriangleType getTriangle() const {
      return mModel->getTriangle(mTriangleId);
    }

    void setTriangleId(int triangleId) {
      mTriangleId = triangleId;
    }

    const CoreModel& getModel() const {
      return *mModel;
    }

    void setModel(const CoreModel& model) {
      mModel = &model;
    }

    const CoreObject& getObject() const {
      return *mObject;
    }

    void setObject(const CoreObject& object) {
      mObject = &object;
    }

  private:
    bool mValid; 
    Vector3f mBarycentricCoord;
    int mTriangleId;
    const CoreModel* mModel;
    const CoreObject* mObject;
  };

} // namespace smart

#endif // __SMART_INTERSECTION_H__
