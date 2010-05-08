#ifndef __SMART_ILLUMINATION_H__
#define __SMART_ILLUMINATION_H__

#include "common.h"
#include "Color.h"
#include <arx/Image.h>

namespace smart {

  /** Radiance class represents the amount of light transported by the ray. 
   * It is different from the Color class, which actually represents the ability
   * of a material to reflect light. Such an abstraction eases the 
   * implementation of HDR rendering. */
  class Radiance {
  public:
    /** Constructor.
     * 
     * Creates uninitialized Radiance object. */
    Radiance() {}

    explicit Radiance(const Vector3f& data): mData(data) {}

    /** Constructor.
     * 
     * Initialized Radiance object with the given channel values. 
     * Note that by passing negative values we can create "darkener", which
     * has no analogue in real world. */
    Radiance(float r, float g, float b): mData(r, g, b) {}

    /** Sums two Radiance objects, for example for two incoming light rays. */
    Radiance operator+ (const Radiance& other) const {
      return Radiance(other.mData + (Vector3f(1, 1, 1) - other.mData).cwise() * mData);
    }

    Radiance& operator+= (const Radiance& other) {
      return *this = *this + other;
    }

    Radiance operator/ (float k) const {
      return *this * (1 / k);
    }

    inline friend Radiance operator* (const Radiance& radiance, const Color& color) {
      return Radiance(radiance.mData.cwise() * color.getData());
    }

    inline friend Radiance operator* (const Color& color, const Radiance& radiance) {
      return radiance * color;
    }

    inline friend Radiance operator* (const Radiance& radiance, float k) {
      return Radiance(radiance.mData * k);
    }

    inline friend Radiance operator* (float k, const Radiance& radiance) {
      return radiance * k;
    }

    arx::Color3f toColor3f() const {
      return arx::Color3f(mData[0], mData[1], mData[2]);
    }

    /** @returns an underlying vector representation */ 
    const Vector3f& getData() const {
      return mData;
    }

  private:
    Vector3f mData;
  };

} // namespace smart

#endif // __SMART_ILLUMINATION_H__
