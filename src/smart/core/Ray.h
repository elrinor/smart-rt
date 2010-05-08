#ifndef __SMART_RAY_H__
#define __SMART_RAY_H__

#include "common.h"

namespace smart {

  /** Structure representing a single ray. */
  ALIGN(32) struct Ray {
  public:
    Ray() {}

    template<class DirDerived, class OrgDerived>
    Ray(const arx::MatrixBase<OrgDerived>& org, const arx::MatrixBase<DirDerived>& dir):
      mOrigin(org), mDirection(dir) {}

    const Vector3f& getOrigin() const { 
      return mOrigin;
    }

    float getOrigin(int index) const { 
      return mOrigin[index];
    }

    const Vector3f& getDirection() const { 
      return mDirection;
    }

    float getDirection(int index) const { 
      return mDirection[index];
    }

    template<class Derived>
    void setOrigin(const arx::MatrixBase<Derived>& org) {
      mOrigin = org;
    }

    void setOrigin(int index, float value) {
      mOrigin[index] = value;
    }

    template<class Derived>
    void setDirection(const arx::MatrixBase<Derived>& dir) {
      mDirection = dir;
    }

    void setDirection(int index, float value) {
      mDirection[index] = value;
    }

  private:
    ALIGN(16) arx::Vector3f mDirection; /**< Ray direction (not necessarily of unit length). */
    ALIGN(16) arx::Vector3f mOrigin; /**< Ray origin. */
  };

} // namespace smart

#endif // __SMART_RAY_H__
