#ifndef __SMART_SHADERIMPL_H__
#define __SMART_SHADERIMPL_H__

#include "common.h"
#include "TraceContext.h"
#include "ShaderRegistrator.h"

namespace smart {
// -------------------------------------------------------------------------- //
// ConstantShader
// -------------------------------------------------------------------------- //
  class ConstantShader {
  public:
    ConstantShader(const Radiance& radiance): mRadiance(radiance) {}

    void shade(TraceContext& ctx) const {
      ctx.setRadiance(mRadiance);
    }

    bool transparency(TraceContext& ctx) const {
      return false;
    }

    void registerParams(const ShaderRegistrator& r) const {
      r.registerParam<Radiance>("value", PER_SHADER, offsetof(ConstantShader, mRadiance));
    }

  private:
    Radiance mRadiance;
  };


// -------------------------------------------------------------------------- //
// DiffuseShader
// -------------------------------------------------------------------------- //
  class DiffuseShader {
  public:
    DiffuseShader(const Color& color): mColor(color) {}

    void shade(TraceContext& ctx) const {
      Radiance r(0, 0, 0);
      Vector3f pos = ctx.getPosition();
      Vector3f n = ctx.getInterpolatedNormal();
      Vector3f dir;
      for(int i = 0; i < ctx.getLightCount(); i++) {
        float dist;
        Radiance incoming = ctx.illuminate(i, pos, dir, dist);
        float dot = dir.dot(n);
        if(dot > 0)
          r += incoming * mColor * dot;
      }
      ctx.setRadiance(r);
    }

    bool transparency(TraceContext& ctx) const {
      return false;
    }

    void registerParams(const ShaderRegistrator& r) const {
      r.registerParam<Radiance>("color", PER_SHADER, offsetof(DiffuseShader, mColor));
    }

  private:
    Color mColor;
  };


// -------------------------------------------------------------------------- //
// TexturedDiffuseShader
// -------------------------------------------------------------------------- //
  class TexturedDiffuseShader {
  public:
    TexturedDiffuseShader(int textureId): mTextureId(textureId) {}

    void shade(TraceContext& ctx) const {
      Vector3f texCoord = ctx.getInterpolatedTexCoord();
      Color color = ctx.getTexture(mTextureId)->getColor(texCoord[0], texCoord[1]);
      Radiance r = mRadiance * color;
      Vector3f pos = ctx.getPosition();
      Vector3f n = ctx.getInterpolatedNormal();
      Vector3f dir;
      for(int i = 0; i < ctx.getLightCount(); i++) {
        float dist;
        Radiance incoming = ctx.illuminate(i, pos, dir, dist);
        float dot = dir.dot(n);
        if(dot > 0) {
          if(!ctx.shadow(pos, dir, dist))
            r += incoming * color * dot;
        }
      }
      ctx.setRadiance(r);
    }

    bool transparency(TraceContext& ctx) const {
      return false;
    }

    void registerParams(const ShaderRegistrator& r) const {
      r.registerParam<int>("texture", PER_SHADER, offsetof(TexturedDiffuseShader, mTextureId));
      r.registerParam<Radiance>("radiance", PER_SHADER, offsetof(TexturedDiffuseShader, mRadiance));
    }

  private:
    Radiance mRadiance;
    int mTextureId;
  };


// -------------------------------------------------------------------------- //
// TextureShader
// -------------------------------------------------------------------------- //
  class TextureShader {
  public:
    TextureShader(int textureId): mTextureId(textureId) {}

    void shade(TraceContext& ctx) const {
      Vector3f texCoord = ctx.getInterpolatedTexCoord();
      ctx.setRadiance(Radiance(ctx.getTexture(mTextureId)->getColor(texCoord[0], texCoord[1]).getData()));
    }

    bool transparency(TraceContext& ctx) const {
      return false;
    }

    void registerParams(const ShaderRegistrator& r) const {
      r.registerParam<int>("texture", PER_SHADER, offsetof(TextureShader, mTextureId));
    }

  private:
    int mTextureId;
  };

// -------------------------------------------------------------------------- //
// MirrorShader
// -------------------------------------------------------------------------- //
  class MirrorShader {
  public:
    MirrorShader(float attenuation): 
      mAttenuation(attenuation) {}

    void shade(TraceContext& ctx) const {
      ctx.setRadiance(ctx.trace(ctx.getPosition(), ctx.getReflectedDirection(), mAttenuation));
    }

    bool transparency(TraceContext& ctx) const {
      return false;
    }

    void registerParams(const ShaderRegistrator& r) const {
      r.registerParam<float>("attenuation", PER_SHADER, offsetof(MirrorShader, mAttenuation));
    }

  private:
    float mAttenuation;
  };


// -------------------------------------------------------------------------- //
// SpotLightShader
// -------------------------------------------------------------------------- //
  class SpotLightShader {
  public:
    SpotLightShader(const Vector3f& p, const Radiance& r): mRadiance(r), mPosition(p) {}

    void registerParams(const ShaderRegistrator& r) const {
      r.registerParam<Radiance>("radiance", PER_SHADER, offsetof(SpotLightShader, mRadiance));
      r.registerParam<Vector3f>("position", PER_SHADER, offsetof(SpotLightShader, mPosition));
    }

    bool illuminate(const Vector3f& position, Vector3f& direction, float& distance, Radiance& radiance) const {
      direction = mPosition - position;
      distance = direction.norm();
      float distInv = 1 / distance;
      direction *= distInv;
      radiance = mRadiance * arx::sqr(distInv);
      return true;
    }

  private:
    Vector3f mPosition;
    Radiance mRadiance;
  };


// -------------------------------------------------------------------------- //
// PinholeCameraShader
// -------------------------------------------------------------------------- //
  /** Camera shader representing a classical pinhole camera. */
  class PinholeCameraShader {
  public:
    /** Constructor.
     *
     * @param origin optical center of a camera.
     * @param direction vector pointing from camera optical center to the 
     *   center of a "virtual screen".
     * @param up virtual screen up vector, half of the virtual screen's height 
     *   in lenght.
     * @param right virtual screen right vector, half of the virtual screen's 
     *   width in lenght. */
    PinholeCameraShader(const Vector3f& origin, const Vector3f& direction, 
      const Vector3f& up, const Vector3f& right): mOrigin(origin),
      mLowerLeft(direction - up - right), mVertical(2 * up), mHorizontal(2 * right) {}

    void initPrimaryRay(float x, float y, TraceContext& ctx) const {
      ctx.setRay(Ray(mOrigin, (mLowerLeft + x * mHorizontal + y * mVertical).normalized()));
    }

    void registerParams(const ShaderRegistrator& r) const {
      r.registerParam<Vector3f>("origin",     PER_SHADER, offsetof(PinholeCameraShader, mOrigin));
      r.registerParam<Vector3f>("lowerLeft",  PER_SHADER, offsetof(PinholeCameraShader, mLowerLeft));
      r.registerParam<Vector3f>("vertical",   PER_SHADER, offsetof(PinholeCameraShader, mVertical));
      r.registerParam<Vector3f>("horizontal", PER_SHADER, offsetof(PinholeCameraShader, mHorizontal));
    }

  private:
    Vector3f mOrigin;
    Vector3f mLowerLeft;
    Vector3f mVertical;
    Vector3f mHorizontal;
  };


} // namespace smart

#endif // __SMART_SHADERIMPL_H__
