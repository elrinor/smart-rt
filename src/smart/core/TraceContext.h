#ifndef __SMART_TRACECONTEXT_H__
#define __SMART_TRACECONTEXT_H__

#include "common.h"
#include "Ray.h"
#include "Segment.h"
#include "Radiance.h"
#include "ShadedModel.h"

namespace smart {
  class ShadedScene;
  class ShadedModel;

// -------------------------------------------------------------------------- //
// TraceContext
// -------------------------------------------------------------------------- //
  class TraceContext {
  public:
    const Radiance& getRadiance() const {
      return radiance;
    }

    /* Camera shader interface. */
    void setScene(const ShadedScene* s) {
      scene = s;
    }

    void setDepth(int d) {
      depth = d;
    }

    void setRay(const Ray& r) {
      ray = r;
    }

    /* Shader Interface */
    void setRadiance(const Radiance& r) {
      radiance = r;
    }

    Vector3f getPosition() const {
      return ray.getOrigin() + tAlongRay * ray.getDirection();
    }

    Vector3f getInterpolatedTexCoord() const {
      return
        model->getTexCoord(triangleId, 0) * barycentricCoord[0] +
        model->getTexCoord(triangleId, 1) * barycentricCoord[1] +
        model->getTexCoord(triangleId, 2) * barycentricCoord[2];
    }

    Vector3f getIncomingDirection() const {
      assert(abs(ray.getDirection().squaredNorm() - 1.0f) < 1.0e-5);
      return ray.getDirection();
    }

    Vector3f getInterpolatedNonNormalizedNormal() const {
      return transform(Vector3f(
        model->getNormal(triangleId, 0) * barycentricCoord[0] + 
        model->getNormal(triangleId, 1) * barycentricCoord[1] + 
        model->getNormal(triangleId, 2) * barycentricCoord[2]
      ), object->getLocalToWorldTransform()) - 
        transform(Vector3f(0, 0, 0), object->getLocalToWorldTransform());
    }

    Vector3f getInterpolatedNormal() const {
      return getInterpolatedNonNormalizedNormal().normalized();
    }

    Vector3f getReflectedDirection() const {
      Vector3f n = getInterpolatedNonNormalizedNormal();
      Vector3f result = getIncomingDirection() - n * (2.0f * getIncomingDirection().dot(n));
      if(getIncomingDirection().dot(n) > 0) {
        const TriAccel& a = model->getTriAccel(triangleId);
        n[a.k] = 1;
        n[fastModulo3(a.k + 1)] = a.nU;
        n[fastModulo3(a.k + 2)] = a.nV;
        result = result - n * (2.0f * result.dot(n));
      }
      return result.normalized();
    }

    int getLightCount() const {
      return scene->getLightShaderCount();
    }

    const Texture* getTexture(int textureId) const {
      return scene->getTexture(textureId);
    }

    /* These are in Tracer.h. */
    Radiance illuminate(int lightIndex, const Vector3f& position, Vector3f& direction, float& distance) const;
    Radiance trace(const Vector3f& position, const Vector3f& direction, float k) const;
    bool shadow(const Vector3f& position, const Vector3f& direction, float distance) const;

  private:
    friend class Tracer;

    Ray ray; /* Direction is always normalized. */
    Segment segment;
    Radiance radiance;

    void* shaderUniformParam;
    void* shaderTriangleParam;
    void* shaderAttribParam[3];

    const CoreObject* object;
    const ShadedScene* scene;
    const ShadedModel* model;
    int triangleId;
    Vector3f barycentricCoord;
    float tAlongRay; /*  TODO remove this with segment.max */

    int depth;

    /* IDEA: we can get hit coord from ray & segment.getMax() */
  };

} // namespace smart

#endif // __SMART_TRACECONTEXT_H__
