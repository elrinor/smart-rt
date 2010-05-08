#ifndef __SMART_SHADERINTERFACE_H__
#define __SMART_SHADERINTERFACE_H__

#include "common.h"
#include "Radiance.h"
#include "Ray.h"
#include "ShaderRegistrator.h"

namespace smart {
  class TraceContext;

// -------------------------------------------------------------------------- //
// SurfaceShader
// -------------------------------------------------------------------------- //
  /** Interface for surface shaders. */
  class SurfaceShader {
  public:
    typedef void TriangleParam;
    typedef void AttribParam;

    void shade(TraceContext& ctx) const;

    bool transparency(TraceContext& ctx) const;

    void registerParams(const ShaderRegistrator& registrator) const;
  };


// -------------------------------------------------------------------------- //
// EnvShader
// -------------------------------------------------------------------------- //
  class EnvShader {
  public:
    void shade(TraceContext& ctx) const;

    void registerParams(const ShaderRegistrator& registrator) const;
  };


// -------------------------------------------------------------------------- //
// LightShader
// -------------------------------------------------------------------------- //
  class LightShader {
  public:
    /* TODO: remove distance. */
    bool illuminate(const Vector3f& position, Vector3f& direction, float& distance, Radiance& radiance) const;

    void registerParams(const ShaderRegistrator& registrator) const;
  };


// -------------------------------------------------------------------------- //
// CameraShader
// -------------------------------------------------------------------------- //
  /** Interface for camera shaders.  */
  class CameraShader {
  public:
    /** This function requests a ray from a camera shader corresponding to
     * (x, y) position on a virtual screen. Note that x and y are always in 
     * range [0, 1]. When rendering, (0, 0) coordinate of a virtual screen
     * corresponds to the lower left corner of an image. 
     * 
     * The returned ray must be normalized. */
    void initPrimaryRay(float x, float y, TraceContext& ctx) const;

    void registerParams(const ShaderRegistrator& registrator) const;
  };

} // namespace smart

#endif // __SMART_SHADERINTERFACE_H__
