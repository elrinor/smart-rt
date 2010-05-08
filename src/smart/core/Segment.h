#ifndef __SMART_SEGMENT_H__
#define __SMART_SEGMENT_H__

#include "common.h"

namespace smart {

  /** Segment class represents a line segment [min, max). */
  class Segment {
  public:
    Segment() {}

    Segment(float min, float max): mMin(min), mMax(max) {}

    template<bool includeMin, bool includeMax>
    bool isEmpty() const {
      return Less<includeMin && includeMax>()(mMax, mMin);
    }

    bool isEmpty(float eps) const {
      return mMax < mMin - eps;
    }

    template<bool includeMin, bool includeMax>
    bool contains(float point) const {
      return Less<!includeMin>()(mMin, point) && Less<!includeMax>()(point, mMax);
    }

    bool contains(float point, float eps) const {
      /* Here eps is either non-negative, or both eps and point are nans. */
      assert(eps >= 0 || (!(point > 0) && !(point <= 0) && !(eps > 0) && !(eps <= 0)));
      return mMin <= point + eps && point - eps <= mMax;
    }

    float getMin() const {
      return mMin;
    }

    float getMax() const {
      return mMax;
    }

    void setMin(float min) {
      mMin = min;
    }

    void setMax(float max) {
      mMax = max;
    }

  private:
    template<bool strict> struct Less {
      bool operator() (float a, float b) {
        return a < b;
      }
    };

    template<> struct Less<false> {
      bool operator() (float a, float b) {
        return a <= b;
      }
    };


    float mMin, mMax;
  };

} // namespace smart

#endif // __SMART_SEGMENT_H__
