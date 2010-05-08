#ifndef __SMART_SHADER_H__
#define __SMART_SHADER_H__

#include "common.h"
#include <arx/Utility.h>
#include "ShaderClass.h"

namespace smart {
// -------------------------------------------------------------------------- //
// Shader
// -------------------------------------------------------------------------- //
  /**
   * Shader class stores shading information used during rendering.
   */
  class Shader {
  public:
    Shader() {}

    ShaderClass* getClass() const {
      return mClass;
    }

    void* getUniformParam() const {
      return mUniformParam;
    }

    void envShade(TraceContext& ctx) const {
      mClass->asEnv()->shade(mUniformParam, &ctx);
    }

    void surfShade(TraceContext& ctx) const {
      mClass->asSurface()->shade(mUniformParam, &ctx);
    }

    bool transparency(TraceContext& ctx) const {
      return mClass->asSurface()->transparency(mUniformParam, &ctx);
    }

    bool illuminate(const Vector3f& position, Vector3f& direction, float& distance, Radiance& radiance) const {
      return mClass->asLight()->illuminate(mUniformParam, &position, &direction, &distance, &radiance);
    }

    void initPrimaryRay(float x, float y, TraceContext& ctx) const {
      mClass->asCamera()->initPrimaryRay(mUniformParam, x, y, &ctx);
    }

  private:
    Shader(ShaderClass* shaderClass, void* uniformParam) {
      initialize(shaderClass, uniformParam);
    }

    void initialize(ShaderClass* shaderClass, void* uniformParam) {
      mClass = shaderClass;
      mUniformParam = uniformParam;
    }

    friend class ShaderManager;

    ShaderClass* mClass;
    void* mUniformParam;
  };

} // namespace smart

#endif // __SMART_SHADER_H__
