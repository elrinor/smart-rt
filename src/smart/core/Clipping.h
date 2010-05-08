#ifndef __SMART_CLIPPING_H__
#define __SMART_CLIPPING_H__

#include "IntersectionTests.h"

namespace smart {
  namespace detail {
    /* Clips the given ray segment against the axis-aligned segment. */
    FORCEINLINE void clip(Segment& segment, const Ray& ray, int axis, const Segment& axisSegment) {
      float min = (axisSegment.getMin() - ray.getOrigin(axis)) / ray.getDirection(axis);
      float max = (axisSegment.getMax() - ray.getOrigin(axis)) / ray.getDirection(axis);
      if(min > max)
        std::swap(min, max);

      if(segment.getMin() < min)
        segment.setMin(min);

      if(segment.getMax() > max)
        segment.setMax(max);
    }

  } // namespace detail

// -------------------------------------------------------------------------- //
// Interface section
// -------------------------------------------------------------------------- //
  /* Clips the given ray segment against the bounding box. */
  inline void clip(Segment& segment, const Ray& ray, const BoundingBox& boundingBox) {
    detail::clip(segment, ray, 0, Segment(boundingBox.getMin(0), boundingBox.getMax(0)));
    detail::clip(segment, ray, 1, Segment(boundingBox.getMin(1), boundingBox.getMax(1)));
    detail::clip(segment, ray, 2, Segment(boundingBox.getMin(2), boundingBox.getMax(2)));
  }

  /* Clips the given triangle to the given bounding box, returning the bounding 
   * box of intersection. 
   *
   * Works by taking max and min of all points in the resulting polygon. */
  template<class TriangleType> 
  inline void clip(const TriangleType& triangle, const TriAccel& triAccel, const BoundingBox& boundingBox, BoundingBox& out) {
    assert(!boundingBox.isInfinite());

    out = BoundingBox::empty();

    /* First 6 (at most) points - triangle edges clipped to bounding box. */
    for(int i = 0; i < 3; i++) {
      Ray r = Ray(triangle[i], triangle[fastModulo3(i + 1)] - triangle[i]);
      Segment s = Segment(0, 1);
      clip(s, r, boundingBox);
      if(!s.isEmpty(SMART_CLIPTRIANGLEBOX_EPS)) {
        out.extend(r.getOrigin() + s.getMin() * r.getDirection());
        out.extend(r.getOrigin() + s.getMax() * r.getDirection());
      }
    }

    /* 12 intersections of bounding box edges with triangle plane. */
    Vector3f extent = boundingBox.getExtent();
    for(int i = 0; i < 3; i++) {
      Ray r;
      Segment s;
      r.setDirection(Vector3f(0, 0, 0));
      r.setDirection(i, extent[i]);
      r.setOrigin(i, boundingBox.getMin(i));

      int j = fastModulo3(i + 1);
      int k = fastModulo3(i + 2);

      /** Here we're using intersect with less-than compare routine to avoid some
       * nasty effects of a triangle vs. infinite bounding box check. */

      s = Segment(0, 1);
      r.setOrigin(j, boundingBox.getMin(j));
      r.setOrigin(k, boundingBox.getMin(k));
      if(intersect(r, s, triAccel, SMART_CLIPTRIANGLEBOX_EPS))
        out.extend(r.getOrigin() + s.getMax() * r.getDirection());

      s = Segment(0, 1);
      r.setOrigin(j, boundingBox.getMax(j));
      r.setOrigin(k, boundingBox.getMin(k));
      if(intersect(r, s, triAccel, SMART_CLIPTRIANGLEBOX_EPS))
        out.extend(r.getOrigin() + s.getMax() * r.getDirection());

      s = Segment(0, 1);
      r.setOrigin(j, boundingBox.getMax(j));
      r.setOrigin(k, boundingBox.getMax(k));
      if(intersect(r, s, triAccel, SMART_CLIPTRIANGLEBOX_EPS))
        out.extend(r.getOrigin() + s.getMax() * r.getDirection());

      s = Segment(0, 1);
      r.setOrigin(j, boundingBox.getMin(j));
      r.setOrigin(k, boundingBox.getMax(k));
      if(intersect(r, s, triAccel, SMART_CLIPTRIANGLEBOX_EPS))
        out.extend(r.getOrigin() + s.getMax() * r.getDirection());
    }

    /* Just-in-case. */
    if(!out.isEmpty()) {
      if((out.getMax().cwise() > boundingBox.getMax()).any()) {
        out.setMax(out.getMax().cwise().min(boundingBox.getMax()));
        out.setMin(out.getMin().cwise().min(out.getMax()));
      }
      if((out.getMin().cwise() < boundingBox.getMin()).any()) {
        out.setMin(out.getMin().cwise().max(boundingBox.getMin()));
        out.setMax(out.getMin().cwise().max(out.getMax()));
      }
    }

    /* Check our just-in-case. */
    assert(boundingBox.contains(out));
  }

} // namespace smart

#endif // __SMART_CLIPPING_H__