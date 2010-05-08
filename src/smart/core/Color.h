#ifndef __SMART_COLOR_H__
#define __SMART_COLOR_H__

#include "common.h"
#include <cassert>

namespace smart {

  /** Color class represents a property of a surface to affect incoming Illumination. */
  class Color {
  public:
    /** Constructor.
     * 
     * Creates an uninitialized Color object. */
    Color() {}

    explicit Color(const Vector3f& data): mData(data) {}

    /** Constructor.
     *
     * Creates a Color object with the given RGB values. */
    Color(float r, float g, float b): mData(r, g, b) {
      assert(0 <= r && r <= 1);
      assert(0 <= g && g <= 1);
      assert(0 <= b && b <= 1);
    }

    /** @returns an underlying vector representation */ 
    const Vector3f& getData() const {
      return mData;
    }

  private:
    Vector3f mData;
  };

} // namespace smart

#endif // __SMART_COLOR_H__
