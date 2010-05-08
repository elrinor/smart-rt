#ifndef __SMART_INTERSECTIONTESTS_H__
#define __SMART_INTERSECTIONTESTS_H__

#include "common.h"
#include "Ray.h"
#include "Segment.h"
#include "TriAccel.h"
#include "Utility.h"

namespace smart {
  namespace detail {
    /** Intersects the given ray segment with the given triangle.
     * In case there was in intersection, returns barycentric coordinates of
     * intersection in barycenricCoord parameter and clips the given segment
     * so that its end lies on the triangle plane.
     * 
     * @returns true if there was an intersection, false otherwise. */
    FORCEINLINE bool intersect(const Ray& ray, Segment& segment, const TriAccel& triAccel, Vector3f& barycenricCoord, float epsilon) {
      assert(epsilon >= 0.0f);

      /* Shortcuts. */
      const Vector3f& o = ray.getOrigin();
      const Vector3f& d = ray.getDirection();
      const TriAccel& a = triAccel;

      /* Some macro magic won't hurt... */
#define ak (a.k)
#define au (fastModulo3(a.k + 1))
#define av (fastModulo3(a.k + 2))

      /* Start high-latecy division first - compute reciprocal scalar product of 
       * plane normal and ray direction. */
      const float nd = 1.0f / (d[ak] + a.nU * d[au] + a.nV * d[av]);

      /* Compute t parameter for point of plane-ray intersection. */
      const float t = (a.nD - o[ak] - a.nU * o[au] - a.nV * o[av]) * nd;

      /* If we are outside the needed segment, then it's a miss. */
      if(!(segment.contains(t, epsilon + abs(t) * epsilon))) /* TODO: think about these evil epsilons. */
        return false;

      /* Compute hit point positions on uv plane. */
      const float hu = o[au] + t * d[au];
      const float hv = o[av] + t * d[av];

      /* Check first barycentric coordinate. */
      const float beta = hu * a.bnU + hv * a.bnV + a.bD;
      if (beta < -epsilon) 
        return false;

      /* Check second barycentric coordinate. */
      const float gamma = hu * a.cnU + hv * a.cnV + a.cD;
      if (gamma < -epsilon) 
        return false;

      /* Check third barycentric coordinate. */
      if (gamma + beta > 1.0f + epsilon) 
        return false;

      /* Hit point is valid. */
      segment.setMax(t);
      barycenricCoord = Vector3f(1 - (beta + gamma), beta, gamma);
      return true;

      /* Clean up. */
#undef ak
#undef au
#undef av
    }

    /** SAT checker for box-triangle intersection test. */
    FORCEINLINE bool satFailed(int u, int v, const Vector3f& edge, const Vector3f& offset, 
      const Vector3f& extend, const Vector3f& vertex) {
        /* Projection of offset to edge normal. */
        const float offsetProj = (-edge[v] * offset[u] + edge[u] * offset[v]);

        /* Projection of extend to edge normal. */
        const float extendProj = (abs(edge[v]) * extend[u] + abs(edge[u]) * extend[v]);

        /* Projection of vertex to edge normal. */
        const float vertexProj = (-edge[v] * vertex[u] + edge[u] * vertex[v]);

        /* Apply SAT. */
        if(vertexProj < 0)
          return offsetProj + extendProj < vertexProj || 0 < offsetProj - extendProj;
        else
          return offsetProj + extendProj < 0 || vertexProj < offsetProj - extendProj;
    }

    /** Determines whether a given triangle and a bounding box intersect. */
    template<class TriangleType>
    inline bool intersects(const BoundingBox& boundingBox, const TriangleType& triangle) {
      /* Shortcuts. */
      const Vector3f& a = triangle[0];
      const Vector3f& b = triangle[1];
      const Vector3f& c = triangle[2];

      /* Edges. */
      const Vector3f e0 = b - a;
      const Vector3f e1 = c - b;
      const Vector3f e2 = a - c;

      /* Calculate center of the bounding box. */
      const Vector3f mOrg = (boundingBox.getMin() + boundingBox.getMax()) * 0.5f;

      /* Calculate extends of the bounding box. */
      const Vector3f mExt = mOrg - boundingBox.getMin();

      /* Bounding box center offsets. */
      const Vector3f v0 = mOrg - a;
      const Vector3f v1 = mOrg - b;

      /* First simple AABB vs AABB test in 3D. */
      const Vector3f tMin = a.cwise().min(b.cwise().min(c));
      const Vector3f tMax = a.cwise().max(b.cwise().max(c));
      const Vector3f tOrg = (tMin + tMax) * 0.5;
      const Vector3f tExt = tOrg - tMin;
      if(((tOrg - mOrg).cwise().abs().cwise() > tExt + mExt).any())
        return false;

      /* Then it's time to use the Separating Axis Theorem.
      * See http://www.harveycartel.org/metanet/tutorials/tutorialA.html#section1 for details. */

      /* Apply SAT for triangle normal in 3D. */
      const Vector3f tNorm = e0.cross(e1);
      if(abs(tNorm.dot(v0)) - tNorm.cwise().abs().dot(mExt) > 0)
        return false;

      /* Apply SAT for 9 edge-plane combinations. 
      * XY first. */
      if(satFailed(0, 1, e0, v0, mExt, -e2)) return false;
      if(satFailed(0, 1, e1, v1, mExt, -e0)) return false;
      if(satFailed(0, 1, e2, v0, mExt, -e1)) return false;

      /* Then YZ. */
      if(satFailed(1, 2, e0, v0, mExt, -e2)) return false;
      if(satFailed(1, 2, e1, v1, mExt, -e0)) return false;
      if(satFailed(1, 2, e2, v0, mExt, -e1)) return false;

      /* And finally ZX. */
      if(satFailed(2, 0, e0, v0, mExt, -e2)) return false;
      if(satFailed(2, 0, e1, v1, mExt, -e0)) return false;
      if(satFailed(2, 0, e2, v0, mExt, -e1)) return false;

      /* Intersection! */
      return true;

      /* see
       * http://www.gamedev.net/community/forums/topic.asp?topic_id=307377&whichpage=1? */
    }

  } // namespace detail

// -------------------------------------------------------------------------- //
// Interface section
// -------------------------------------------------------------------------- //
  inline bool intersect(const Ray& ray, Segment& segment, const TriAccel& triAccel, Vector3f& barycenricCoord, float eps) {
    return detail::intersect(ray, segment, triAccel, barycenricCoord, eps);
  }

  inline bool intersect(const Ray& ray, Segment& segment, const TriAccel& triAccel, float eps) {
    Vector3f dummy;
    return detail::intersect(ray, segment, triAccel, dummy, eps);
  }

  template<class TriangleType>
  inline bool intersects(const BoundingBox& boundingBox, const TriangleType& triangle) {
    return detail::intersects(boundingBox, triangle);
  }

  template<class TriangleType>
  inline bool intersects(const TriangleType& triangle, const BoundingBox& boundingBox) {
    return detail::intersects(boundingBox, triangle);
  }

  inline bool intersects(const BoundingBox& a, const BoundingBox& b) {
    return (a.getMax().cwise() >= b.getMin()).all() &&
      (b.getMax().cwise() >= a.getMin()).all();
  }

} // namespace smart

#endif // __SMART_INTERSECTIONTESTS_H__