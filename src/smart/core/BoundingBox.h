#ifndef __SMART_CORE_BOUNDINGBOX_H__
#define __SMART_CORE_BOUNDINGBOX_H__

#include "common.h"
#include <limits>

namespace smart {

  /** Axis-aligned bounding box. */
  class BoundingBox {
  public:
    /** Default Constructor. 
     *
     * Creates an uninitialized bounding box. */
    BoundingBox() {
      mMin.setConstant(std::numeric_limits<float>::max());
      mMax.setConstant(-std::numeric_limits<float>::max());
    }

    /** Constructor.
     *
     * @param min coordinates of the minimal corner.
     * @param max coordinates of the maximal corner. */
    BoundingBox(const Vector3f& min, const Vector3f& max): mMin(min), mMax(max) {
      assert((min.cwise() <= max).all()); /* We allow "thin" bounding boxes, therefore "<=". */
    }

    /** @returns an empty bounding box. */
    static BoundingBox empty() {
      BoundingBox result;
      result.mMin.setConstant(std::numeric_limits<float>::max());
      result.mMax.setConstant(-std::numeric_limits<float>::max());
      return result;
    }

    /** @returns an infinite bounding box, containing everything. */
    static BoundingBox infinite() {
      BoundingBox result;
      result.mMin.setConstant(-std::numeric_limits<float>::max());
      result.mMax.setConstant(std::numeric_limits<float>::max());
      return result;
    }

    bool isEmpty() const {
      return (mMin.cwise() > mMax).any();
    }

    bool isInfinite() const {
      return 
        (mMax.cwise() >=  std::numeric_limits<float>::max()).any() ||
        (mMin.cwise() <= -std::numeric_limits<float>::max()).any();
    }

    /** @returns coordinates of the minimal corner. */
    const Vector3f& getMin() const {
      return mMin;
    }

    /** @returns coordinates of the maximal corner. */
    const Vector3f& getMax() const {
      return mMax;
    }

    /** @returns coordinate of the minimal corner. */
    float getMin(int index) const {
      return mMin[index];
    }

    /** @returns coordinate of the maximal corner. */
    float getMax(int index) const {
      return mMax[index];
    }

    void setMin(const Vector3f& value) {
      mMin = value;
    }

    void setMin(int index, float value) {
      mMin[index] = value;
    }

    void setMax(const Vector3f& value) {
      mMax = value;
    }

    void setMax(int index, float value) {
      mMax[index] = value;
    }

    /** Determines whether the given point lies inside or on the sides of
     * this bounding box.
     * 
     * @param point point to check.
     * @returns true if point lies inside the bounding box, false otherwise. */
    bool contains(const Vector3f& point) const {
#if 0
      /* This one happened to be way too slow, 
       * and it needed Vector4f for nMin & nMax */
      __m128 tmp;
      if(ALIGNOF(Vector3f) == 16)
        tmp = _mm_load_ps(point.data());
      else
        tmp = _mm_loadu_ps(point.data());

      __m128 min = _mm_load_ps(mMin.data());
      __m128 max = _mm_load_ps(mMax.data());

      __m128 leMax = _mm_cmple_ps(tmp, max);
      __m128 geMin = _mm_cmpge_ps(tmp, min);

      __m128 inSegment = _mm_and_ps(geMin, leMax);

      return (_mm_movemask_ps(inSegment) & 0x7) == 0x7;
#endif
      return (mMin.cwise() <= point).all() && (point.cwise() <= mMax).all();
    }

    bool contains(const BoundingBox& other) const {
      return (mMin.cwise() <= other.mMin).all() && (other.mMax.cwise() <= mMax).all();
    }

    /** Extends this bounding box to include another bounding box. 
     * 
     * @param other bounding box to include. */
    void extend(const BoundingBox& other) {
      mMin = mMin.cwise().min(other.mMin);
      mMax = mMax.cwise().max(other.mMax);
    }

    /** Extends this bounding box to include given point. 
     * 
     * @param other bounding box to include. */
    void extend(const Vector3f& point) {
      mMin = mMin.cwise().min(point);
      mMax = mMax.cwise().max(point);
    }

    int getMaxExtentAxis() const {
      Vector3f size = mMax - mMin;
      if(size[0] >= size[1] && size[0] >= size[2])
        return 0;
      else if(size[1] >= size[0] && size[1] >= size[2])
        return 1;
      else
        return 2;
    }

    Vector3f getExtent() const {
      return mMax - mMin;
    }

    Vector3f getCenter() const {
      return (mMax + mMin) / 2;
    }

  private:
    Vector3f mMin;
    Vector3f mMax;
  };


  /** Helper factory function. */
  template<class TriangleType>
  BoundingBox triangleBoundingBox(const TriangleType& triangle) {
    return BoundingBox(
      triangle[0].cwise().min(triangle[1].cwise().min(triangle[2])),
      triangle[0].cwise().max(triangle[1].cwise().max(triangle[2]))
    );
  }

} // namespace smart

#endif // __SMART_CORE_BOUNDINGBOX_H__
