#ifndef __SMART_TRIACCEL_H__
#define __SMART_TRIACCEL_H__

#include "common.h"
#include <arx/Memory.h>

namespace smart {

  /** Triangle intersection test acceleration structure, as described in 
   * Ingo Wald's PhD Thesis. */
  ALIGN(16) class TriAccel: public arx::WithAlignedOperatorNew<16> {
  public:
    /* Constructor.
     * 
     * @param triangle triangle to create acceleration structure for. */
    template<class TriangleType>
    TriAccel(const TriangleType& triangle) {
      /* Create shortcuts. */
      const Vector3f& a = triangle[0];
      const Vector3f& b = triangle[1] - a;
      const Vector3f& c = triangle[2] - a;

      /* Compute normal. */
      Vector3f n = b.cross(c);

      /* Compute splitting dimension. For the reasons of numerical stability, 
       * projection is done to the plane where triangle has maximal projected 
       * area. */
      Vector3f absN = n.cwise().abs();
      if(absN[0] > absN[1]) {
        if(absN[0] > absN[2]) k = 0; else k = 2;
      } else {
        if(absN[1] > absN[2]) k = 1; else k = 2;
      }

      /* Calculate indices of other two dimensions. */
      int u = fastModulo3(k + 1);
      int v = fastModulo3(k + 2);

      /* n[k] is never zero, so we can divide normal by n[k] and store
       * only n[u] and n[v]. */
      float kRec = 1.0f / n[k];
      nU = n[u] * kRec;
      nV = n[v] * kRec;
      nD = n.dot(a) * kRec;

      /* Second barycentric coordinate equation. */
      float iRec = 1.0f / (b[u] * c[v] - b[v] * c[u]);
      bnU = c[v] * iRec;
      bnV = -c[u] * iRec;
      bD = -(c[v] * a[u] - c[u] * a[v]) * iRec;

      /* Third barycentric coordinate equation. */
      cnU = -b[v] * iRec;
      cnV = b[u] * iRec;
      cD = -(b[u] * a[v] - b[v] * a[u]) * iRec;
    }

  public:
    ALIGN(16) struct {
      float nU;  /**< n[u] */
      float nV;  /**< n[v] */
      float nD;  /**< n.dot(A) */
      int k;     /**< Projection dimension index. Note that n[k] == 1. */
    };

    /** Equation coefficients for second (beta) barycentric coordinate. */
    ALIGN(16) struct {
      float bnU;
      float bnV;
      float bD;
    };

    /** Equation coefficients for third (gamma) barycentric coordinate. */
    ALIGN(16) struct {
      float cnU;
      float cnV;
      float cD;
    };
  };

} // namespace smart

#endif // __SMART_TRIACCEL_H__
