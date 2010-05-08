#ifndef __SMART_SHADERTYPE_H__
#define __SMART_SHADERTYPE_H__

#include "common.h"
#include <arx/Mpl.h>
#include <arx/Utility.h>
#include "Utility.h"
#include "ShaderRegistrator.h"
#include "Radiance.h"
#include "Idded.h"

namespace smart {
  class SurfaceShaderClass;
  class EnvShaderClass;
  class LightShaderClass;
  class CameraShaderClass;

  class TraceContext;

  /** Type of a shader. */
  enum ShaderClassType {
    SURFACE_SHADER,
    ENV_SHADER,
    LIGHT_SHADER,
    CAMERA_SHADER
  };

// -------------------------------------------------------------------------- //
// ShaderClass
// -------------------------------------------------------------------------- //
  /**
   * Base class for all shader types.
   */
  class ShaderClass: public arx::noncopyable, public Idded {
  public:
    /** Type of a function that performs shading operation. */
    typedef void (*ShadeFunc)(const void* /* uniform */, TraceContext* /* ctx */);

    /** Type of a function that performs surface transparency evaluation. */
    typedef bool (*TransparencyFunc)(const void* /* uniform */, TraceContext* /* ctx */);

    /** Type of a function that performs camera shader operation. */
    typedef void (*InitPrimaryRayFunc)(const void* /* uniform */, float /* x */, float /* y */, TraceContext* /* ctx */);

    /** Type of a function that performs light shader operation. */
    typedef bool (*IlluminateFunc)(const void* /* uniform */, const Vector3f* /* position */, Vector3f* /* direction */, float* /* distance */, Radiance* /* radiance */);

    template<class T>
    ShaderClass(ShaderClassType type, arx::identity<T> /* impl */) {
      STATIC_ASSERT((is_shader<T>::value));
      initialize(type, sizeof(T), arx::alignment_of<T>::value);
    }

    ShaderClass(ShaderClassType type, int uniformParamSize, int uniformParamAlign) {
      initialize(type, uniformParamSize, uniformParamAlign);
    }

    int getUniformParamSize() const {
      return mUniformParamSize;
    }

    int getUniformParamAling() const {
      return mUniformParamAlign;
    }

    int getType() const {
      return mType;
    }

    const SurfaceShaderClass* asSurface() const {
      assert(mType == SURFACE_SHADER);
      return reinterpret_cast<const SurfaceShaderClass*>(this);
    }

    SurfaceShaderClass* asSurface() {
      assert(mType == SURFACE_SHADER);
      return reinterpret_cast<SurfaceShaderClass*>(this);
    }

    const EnvShaderClass* asEnv() const {
      assert(mType == ENV_SHADER);
      return reinterpret_cast<const EnvShaderClass*>(this);
    }

    EnvShaderClass* asEnv() {
      assert(mType == ENV_SHADER);
      return reinterpret_cast<EnvShaderClass*>(this);
    }

    const LightShaderClass* asLight() const {
      assert(mType == LIGHT_SHADER);
      return reinterpret_cast<const LightShaderClass*>(this);
    }

    LightShaderClass* asLight() {
      assert(mType == LIGHT_SHADER);
      return reinterpret_cast<LightShaderClass*>(this);
    }

    const CameraShaderClass* asCamera() const {
      assert(mType == CAMERA_SHADER);
      return reinterpret_cast<const CameraShaderClass*>(this);
    }

    CameraShaderClass* asCamera() {
      assert(mType == CAMERA_SHADER);
      return reinterpret_cast<CameraShaderClass*>(this);
    }

    template<class T>
    const T* as() const {
      assert(mType == defaultType<T>::value);
      return reinterpret_cast<const T*>(this);
    }

    template<class T>
    T* as() {
      assert(mType == defaultType<T>::value);
      return reinterpret_cast<T*>(this);
    }

  protected:
    SMART_DEFINE_HAS_MEMBER(registerParams, void (U::*Func)(const ShaderRegistrator&) const,                            &T::registerParams);
    SMART_DEFINE_HAS_MEMBER(shade,          void (U::*Func)(TraceContext&) const,                                 &T::shade);
    SMART_DEFINE_HAS_MEMBER(transparency,   bool (U::*Func)(TraceContext&) const,                                 &T::transparency);
    SMART_DEFINE_HAS_MEMBER(illuminate,     bool (U::*Func)(const Vector3f&, Vector3f&, float&, Radiance&) const, &T::illuminate);
    SMART_DEFINE_HAS_MEMBER(initPrimaryRay, void (U::*Func)(float, float, TraceContext&) const,                   &T::initPrimaryRay);

#define SMART_DEFINE_WRAPPER(MEMBER_NAME, RETURN_TYPE, PARAM_LIST, CALL_LIST)   \
    template<class T, bool hasMember = ARX_JOIN(has_, MEMBER_NAME)<T>::value>   \
    struct ARX_JOIN(MEMBER_NAME, Wrapper) {                                     \
      static RETURN_TYPE MEMBER_NAME PARAM_LIST {                               \
        return static_cast<const T*>(uniform)->MEMBER_NAME CALL_LIST;           \
      }                                                                         \
    };                                                                          \
    template<class T>                                                           \
    struct ARX_JOIN(MEMBER_NAME, Wrapper)<T, false> {                           \
      static RETURN_TYPE MEMBER_NAME PARAM_LIST {                               \
        Unreachable();                                                          \
        return RETURN_TYPE(); /* Just to make compiler happy. */                \
      }                                                                         \
    };

    SMART_DEFINE_WRAPPER(shade,          void, (const void* uniform, TraceContext* ctx), (*ctx));
    SMART_DEFINE_WRAPPER(transparency,   bool, (const void* uniform, TraceContext* ctx), (*ctx));
    SMART_DEFINE_WRAPPER(initPrimaryRay, void, (const void* uniform, float x, float y, TraceContext* ctx), (x, y, *ctx));
    SMART_DEFINE_WRAPPER(illuminate,     bool, 
      (const void* uniform, const Vector3f* position, Vector3f* direction, float* distance, Radiance* radiance), 
      (*position, *direction, *distance, *radiance));

#undef SMART_DEFINE_WRAPPER

    template<class T> struct is_shader: public arx::and_<
      has_registerParams<T>,
      arx::or_<
        arx::and_<has_shade<T>, has_transparency<T> >,
        has_shade<T>, 
        has_illuminate<T>,
        has_initPrimaryRay<T>
      >
    > {};

  private:
    friend class SmartCore;
    friend class ShaderManager;

    template<class T> struct defaultType;
    template<> struct defaultType<SurfaceShaderClass> : public arx::int_<SURFACE_SHADER> {};
    template<> struct defaultType<EnvShaderClass>     : public arx::int_<ENV_SHADER> {};
    template<> struct defaultType<LightShaderClass>   : public arx::int_<LIGHT_SHADER> {};
    template<> struct defaultType<CameraShaderClass>  : public arx::int_<CAMERA_SHADER> {};

    void initialize(ShaderClassType type, int uniformParamSize, int uniformParamAlign) {
      mType = type;
      mUniformParamSize = uniformParamSize;
      mUniformParamAlign = uniformParamAlign;
    }

    ShaderClassType mType;    /**< Type of this shader. */
    int mUniformParamSize;    /**< Size in bytes of the per-shader shading parameters. */
    int mUniformParamAlign;   /**< Alignment in bytes of the per-shader shading parameters. */
  };


// -------------------------------------------------------------------------- //
// SurfaceShaderClass
// -------------------------------------------------------------------------- //
  /**
   * Type for surface shader.
   */
  class SurfaceShaderClass: public ShaderClass {
  public:
    template<class T>
    SurfaceShaderClass(arx::identity<T> impl): 
      ShaderClass(SURFACE_SHADER, impl) {
      typedef typename arx::if_<has_TriParam<T>, get_TriParam<T>, arx::identity<void> >::type::type TriParam;
      typedef typename arx::if_<has_AttParam<T>, get_AttParam<T>, arx::identity<void> >::type::type AttParam;
      enum {
        triParamIsVoid = arx::is_same<TriParam, void>::value,
        attParamIsVoid = arx::is_same<AttParam, void>::value,
      };
      initialize(
        arx::if_c<triParamIsVoid, arx::int_<0>, arx::sizeof_<TriParam> >::type::value,
        arx::if_c<triParamIsVoid, arx::int_<1>, arx::alignment_of<TriParam> >::type::value,
        arx::if_c<attParamIsVoid, arx::int_<0>, arx::sizeof_<AttParam> >::type::value,
        arx::if_c<attParamIsVoid, arx::int_<1>, arx::alignment_of<AttParam> >::type::value,
        &shadeWrapper<T>::shade,
        &transparencyWrapper<T>::transparency
      );
    }

    template<class T>
    SurfaceShaderClass(int triangleParamSize, int triangleParamAlign,
                      int attribParamSize, int attribParamAlign, 
                      arx::identity<T> impl): 
      ShaderClass(SURFACE_SHADER, impl, &shadingFuncWrapper<T>) {
      initialize(
        triangleParamSize, 
        triangleParamAlign, 
        attribParamSize, 
        attribParamAlign
        &shadeWrapper<T>,
        &transparencyWrapper<T>
      );
    }

    SurfaceShaderClass(int uniformParamSize, int uniformParamAlign, 
                      int triangleParamSize, int triangleParamAlign,
                      int attribParamSize, int attribParamAlign,
                      ShadeFunc shadingFunc, TransparencyFunc transparencyFunc): 
      ShaderClass(SURFACE_SHADER, uniformParamSize, uniformParamAlign) {
      initialize(
        triangleParamSize, 
        triangleParamAlign, 
        attribParamSize, 
        attribParamAlign,
        shadingFunc,
        transparencyFunc
      );
    }

    int getAttribParamSize() const {
      return mAttribParamSize;
    }

    int getAttribParamAlign() const {
      return mAttribParamAlign;
    }

    int getTriangleParamSize() const {
      return mTriangleParamSize;
    }

    int getTriangleParamAlign() const {
      return mTriangleParamAlign;
    }

    void shade(const void* uniform, TraceContext* ctx) const {
      mShade(uniform, ctx);
    }

    bool transparency(const void* uniform, TraceContext* ctx) const {
      return mTransparency(uniform, ctx);
    }

  private:
    SMART_DEFINE_HAS_MEMBER(TriParam, class T, typename T::TriParam);
    SMART_DEFINE_HAS_MEMBER(AttParam, class T, typename T::AttParam);
    template<class T> struct get_TriParam { typedef typename T::TriParam type; };
    template<class T> struct get_AttParam { typedef typename T::AttParam type; };

    void initialize(int triangleParamSize, int triangleParamAlign,
                    int attribParamSize, int attribParamAlign,
                    ShadeFunc shadingFunc, TransparencyFunc transparencyFunc) {
      mTriangleParamSize = triangleParamSize;
      mTriangleParamAlign = triangleParamAlign;
      mAttribParamSize = attribParamSize;
      mAttribParamAlign = attribParamAlign;
      mShade = shadingFunc;
      mTransparency = transparencyFunc;
    }

    ShadeFunc mShade;
    TransparencyFunc mTransparency;
    int mTriangleParamSize;   /**< Size in bytes of the per-triangle shading parameters. */
    int mTriangleParamAlign;  /**< Alignment in bytes of the per-triangle shading parameters. */
    int mAttribParamSize;     /**< Size in bytes of the per-vertex shading parameters. */
    int mAttribParamAlign;    /**< Alignment in bytes of the per-vertex shading parameters. */
  };


// -------------------------------------------------------------------------- //
// EnvShaderClass
// -------------------------------------------------------------------------- //
  /**
   * Type for Environmental shaders.
   */
  class EnvShaderClass: public ShaderClass {
  public:
    template<class T>
    EnvShaderClass(arx::identity<T> impl): 
      ShaderClass(ENV_SHADER, impl), mShade(&shadeWrapper<T>::shade) {}

    EnvShaderClass(int uniformParamSize, int uniformParamAlign, ShadeFunc shadingFunc): 
      ShaderClass(ENV_SHADER, uniformParamSize, uniformParamAlign), mShade(shadingFunc) {}

    void shade(const void* uniform, TraceContext* ctx) const {
      mShade(uniform, ctx);
    }

  private:
    ShadeFunc mShade;
  };


// -------------------------------------------------------------------------- //
// LightShaderClass
// -------------------------------------------------------------------------- //
  /**
   * Type for Light shaders.
   */
  class LightShaderClass: public ShaderClass {
  public:
    template<class T>
    LightShaderClass(arx::identity<T> impl): 
      ShaderClass(LIGHT_SHADER, impl), mIlluminate(&illuminateWrapper<T>::illuminate) {}

    LightShaderClass(int uniformParamSize, int uniformParamAlign, IlluminateFunc illuminateFunc): 
      ShaderClass(LIGHT_SHADER, uniformParamSize, uniformParamAlign), mIlluminate(illuminateFunc) {}

    bool illuminate(const void* uniform, const Vector3f* position, Vector3f* direction, float* distance, Radiance* radiance) const {
      return mIlluminate(uniform, position, direction, distance, radiance);
    }

  private:
    IlluminateFunc mIlluminate;
  };


// -------------------------------------------------------------------------- //
// CameraShaderClass
// -------------------------------------------------------------------------- //
  /**
   * Type for Camera shaders.
   */
  class CameraShaderClass: public ShaderClass {
  public:
    template<class T>
    CameraShaderClass(arx::identity<T> impl): 
      ShaderClass(CAMERA_SHADER, impl), mInitPrimaryRay(&initPrimaryRayWrapper<T>::initPrimaryRay) {}

    CameraShaderClass(int uniformParamSize, int uniformParamAlign, InitPrimaryRayFunc cameraFunc): 
      ShaderClass(CAMERA_SHADER, uniformParamSize, uniformParamAlign), mInitPrimaryRay(cameraFunc) {}

    void initPrimaryRay(const void* uniform, float x, float y, TraceContext* ctx) const {
      mInitPrimaryRay(uniform, x, y, ctx);
    }

  private:
    InitPrimaryRayFunc mInitPrimaryRay;
  };


} // namespace smart

#endif // __SMART_SHADERTYPE_H__
