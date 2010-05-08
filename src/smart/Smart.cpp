#include <arx/config.h>
#include "util/DisableMSVCWarnings.h"
#include "Smart.h"
#include "SmartPlus.h"
#include "core/SmartCore.h"

#if defined(ARX_MSVC) || defined(ARX_ICC)
#  define _USE_MATH_DEFINES
#endif
#include <cmath>
#ifndef M_PI
#  define M_PI 3.14159265358979323846
#endif


// -------------------------------------------------------------------------- //
// Defines
// -------------------------------------------------------------------------- //
#define PRECONDITION_RET(CONDITION, ERRORCODE, RETVALUE)                        \
  {                                                                             \
    if(!(CONDITION)) {                                                          \
      st.signalError(ERRORCODE);                                                \
      return RETVALUE;                                                          \
    }                                                                           \
  }

#define PRECONDITION(CONDITION, ERRORCODE)                                      \
  PRECONDITION_RET(CONDITION, ERRORCODE, ARX_EMPTY())

#define PRECONDITION_NOT_IN_BEGIN_END_RET(RETVALUE)                             \
  PRECONDITION_RET(!st.insideBeginEnd, RT_INVALID_OPERATION, RETVALUE)

#define PRECONDITION_NOT_IN_BEGIN_END()                                         \
  PRECONDITION_NOT_IN_BEGIN_END_RET(ARX_EMPTY())

#define PRECONDITION_IN_BEGIN_END()                                             \
  PRECONDITION(st.insideBeginEnd, RT_INVALID_OPERATION)

#define PRECONDITION_NOT_IN_NEWOBJECT_RET(RETVALUE)                             \
  PRECONDITION_RET(!st.insideNewObject, RT_INVALID_OPERATION, RETVALUE)

#define PRECONDITION_NOT_IN_NEWOBJECT()                                         \
  PRECONDITION_NOT_IN_NEWOBJECT_RET(ARX_EMPTY())

#define PRECONDITION_IN_NEWOBJECT_RET(RETVALUE)                                 \
  PRECONDITION_RET(st.insideNewObject, RT_INVALID_OPERATION, RETVALUE)

#define PRECONDITION_IN_NEWOBJECT()                                             \
  PRECONDITION_IN_NEWOBJECT_RET(ARX_EMPTY())

#define SMART_MATRIX_STACK_DEPTH 1024

// -------------------------------------------------------------------------- //
// Global state
// -------------------------------------------------------------------------- //
namespace smart {
  namespace cface {
    SmartState st;
  }
}

using smart::cface::st;

// -------------------------------------------------------------------------- //
// Implementation - C++
// -------------------------------------------------------------------------- //
void rtiNewTriangle(int id0, int id1, int id2) {
  /* ids cannot be equal, so we don't check it. */
  const smart::Vector3f& a = st.model->getCoord(id0);
  const smart::Vector3f& b = st.model->getCoord(id1);
  const smart::Vector3f& c = st.model->getCoord(id2);

  if((a - b).squaredNorm() < SMART_NOT_A_TRIANGLE_EPS ||
     (b - c).squaredNorm() < SMART_NOT_A_TRIANGLE_EPS ||
     (c - a).squaredNorm() < SMART_NOT_A_TRIANGLE_EPS) {
    return; /* Don't signal error here. */
  }

  st.model->newTriangle(id0, id1, id2, st.shaderId, st.shaderAttribParam);
}

RTAPI RTvoid RTAPIENTRY rtVertex(const smart::Vector4f& v) {
  PRECONDITION_IN_BEGIN_END();
  PRECONDITION(st.shader != NULL, RT_INVALID_OPERATION);
  PRECONDITION(st.shaderClass->getType() == smart::SURFACE_SHADER, RT_INVALID_OPERATION);

  int id = st.model->newVertex(
    smart::transform(v, st.matrix), 
    smart::transform(st.geometry.normal, st.matrix), 
    smart::transform(st.geometry.texCoord, smart::Matrix4f::Identity()),
    st.shaderTriangleParam
  );

  smart::cface::GeometryData& g = st.geometry;
  switch(st.geometry.mode) {
  case RT_TRIANGLES:
    if(g.n != 2) {
      g.ids[g.n] = id;
      g.n++;
    } else {
      rtiNewTriangle(g.ids[0], g.ids[1], id);
      g.n = 0;
    }
    break;
  case RT_TRIANGLE_STRIP:
    if(g.n == 2) {
      rtiNewTriangle(g.ids[0], g.ids[1], id);
      g.ids[0] = g.ids[1];
      g.ids[1] = id;
    } else {
      g.ids[g.n] = id;
      g.n++;
    }
    break;
  case RT_POLYGON:
  case RT_TRIANGLE_FAN:
    if(g.n == 2) {
      rtiNewTriangle(g.ids[0], g.ids[1], id);
      g.ids[1] = id;
    } else {
      g.ids[g.n] = id;
      g.n++;
    }
    break;
  case RT_QUADS:
    if(g.n != 3) {
      g.ids[g.n] = id;
      g.n++;
    } else  {
      rtiNewTriangle(g.ids[0], g.ids[1], id);
      rtiNewTriangle(id, g.ids[1], g.ids[2]);
      g.n = 0;
    }
    break;
  case RT_QUAD_STRIP:
    if(g.n == 3) {
      rtiNewTriangle(g.ids[0], g.ids[1], g.ids[2]);
      rtiNewTriangle(g.ids[2], g.ids[1], id);
      g.ids[0] = g.ids[2];
      g.ids[1] = id;
      g.n = 2;
    } else {
      g.ids[g.n] = id;
      g.n++;
    }
    break;
  default:
    Unreachable();
    break;
  }
}

RTAPI RTvoid RTAPIENTRY rtNormal(const smart::Vector4f& n) {
  PRECONDITION_IN_BEGIN_END();

  st.geometry.normal = n;
}

RTAPI RTvoid RTAPIENTRY rtTexCoord(const smart::Vector4f& t) {
  PRECONDITION_IN_BEGIN_END();

  st.geometry.texCoord = t;
}

RTAPI RTvoid RTAPIENTRY rtVertex(const smart::Vector3f& v) {
  rtVertex(smart::Vector4f(v[0], v[1], v[2], 1.0f));
}

RTAPI RTvoid RTAPIENTRY rtNormal(const smart::Vector3f& n) {
  rtNormal(smart::Vector4f(n[0], n[1], n[2], 1.0f));
}

RTAPI RTvoid RTAPIENTRY rtTexCoord(const smart::Vector3f& t) {
  rtTexCoord(smart::Vector4f(t[0], t[1], t[2], 1.0f));
}

RTAPI RTvoid RTAPIENTRY rtVertex(const smart::Vector2f& v) {
  rtVertex(smart::Vector4f(v[0], v[1], 0.0f, 1.0f));
}

RTAPI RTvoid RTAPIENTRY rtNormal(const smart::Vector2f& n) {
  rtNormal(smart::Vector4f(n[0], n[1], 0.0f, 1.0f));
}

RTAPI RTvoid RTAPIENTRY rtTexCoord(const smart::Vector2f& t) {
  rtTexCoord(smart::Vector4f(t[0], t[1], 0.0f, 1.0f));
}

RTAPI RTvoid RTAPIENTRY rtLoadMatrix(const smart::Matrix4f& m) {
  PRECONDITION_NOT_IN_BEGIN_END();

  st.matrix = m;
}

RTAPI RTvoid RTAPIENTRY rtMultMatrix(const smart::Matrix4f& m) {
  PRECONDITION_NOT_IN_BEGIN_END();

  st.matrix *= m;
}

RTAPI RTuint RTAPIENTRY rtGenTexture(const arx::Image3f& image) {
  PRECONDITION_NOT_IN_BEGIN_END_RET(RT_INVALID);

  return st.core->newTexture(image)->getId();
}

RTAPI RTvoid RTAPIENTRY rtDeleteTexture(RTuint textureId) {
  PRECONDITION_NOT_IN_BEGIN_END();
  PRECONDITION(st.core->hasTexture(textureId), RT_INVALID_VALUE);

  st.core->releaseTexture(st.core->getTexture(textureId));
}


extern "C" {

// -------------------------------------------------------------------------- //
// Implementation - C, Core API
// -------------------------------------------------------------------------- //
RTAPI RTvoid RTAPIENTRY rtInit(int *argcp, char *argv[]) {
  st.core = new smart::SmartCore();

  st.errorCode = RT_NO_ERROR;
  st.geometry.mode = RT_INVALID;

  st.scene = st.core->newScene();
  st.sceneModel = st.core->newModel();
  st.model = st.sceneModel;
  st.renderTask = NULL;
  st.shader = NULL;
  st.shaderId = smart::SMART_INVALID_ID;
  st.shaderClass = NULL;
  st.shaderTriangleParam = NULL;
  st.shaderAttribParam = NULL;
  st.shaderTriangleParamUsed = false;
  st.shaderAttribParamUsed = false;

  st.insideBeginEnd = false;
  st.insideNewObject = false;

  st.matrix = smart::Matrix4f::Identity();
}

RTAPI RTvoid RTAPIENTRY rtExit(void) {
  delete st.core;
  st.matrixStack.clear();
  st.frameBuffer = arx::Image3f();
}

RTAPI RTenum RTAPIENTRY rtGetError(void) {
  /* rtGetError was called between a call to rtBegin and the corresponding call to rtEnd. */
  PRECONDITION_NOT_IN_BEGIN_END_RET(RT_NO_ERROR);

  return st.errorCode;
}


// -------------------------------------------------------------------------- //
// Implementation - C, Rendering API
// -------------------------------------------------------------------------- //
RTAPI RTvoid RTAPIENTRY rtSetFrameBuffer(RTubyte *fb, RTuint width, RTuint height, RTuint format) {
  PRECONDITION_NOT_IN_BEGIN_END();

  switch(format) {
  case RT_BGR_32:
    st.frameBuffer = arx::Image3f(width, height, 3 * width * sizeof(RTfloat), fb, new arx::EmptyImageDeallocator());
    break;
  default:
    st.signalError(RT_INVALID_ENUM);
    return;
  }
}

RTAPI RTvoid RTAPIENTRY rtRender(void) {
  if(st.renderTask == NULL)
    rtStartRendering();
  rtEndRendering();
}

RTAPI RTvoid RTAPIENTRY rtStartRendering(void) {
  PRECONDITION_NOT_IN_BEGIN_END();
  PRECONDITION_NOT_IN_NEWOBJECT();
  PRECONDITION(st.renderTask == NULL, RT_INVALID_OPERATION);

  st.scene->decompile();
  st.renderTask = st.core->startRendering(st.scene, st.frameBuffer);
  st.core->releaseScene(st.scene);
  st.core->releaseModel(st.sceneModel);

  st.scene = st.core->newScene();
  st.sceneModel = st.core->newModel();
  st.model = st.sceneModel;
}

RTAPI RTvoid RTAPIENTRY rtEndRendering(void) {
  PRECONDITION_NOT_IN_BEGIN_END();
  PRECONDITION_NOT_IN_NEWOBJECT();
  PRECONDITION(st.renderTask != NULL, RT_INVALID_OPERATION);

  st.core->endRendering(st.renderTask);
  st.renderTask = NULL;
}



// -------------------------------------------------------------------------- //
// Implementation - C, Geometry API
// -------------------------------------------------------------------------- //
RTAPI RTvoid RTAPIENTRY rtBegin(RTenum mode) {
  PRECONDITION_NOT_IN_BEGIN_END();
  switch(mode) {
    case RT_TRIANGLES:
    case RT_TRIANGLE_STRIP:
    case RT_TRIANGLE_FAN:
    case RT_QUADS:
    case RT_QUAD_STRIP:
    case RT_POLYGON:
      break;
    default:
      st.signalError(RT_INVALID_ENUM);
      return;
  }

  st.geometry.n = 0;
  st.geometry.mode = mode;
  st.insideBeginEnd = true;
}


RTAPI RTvoid RTAPIENTRY rtEnd(void) {
  PRECONDITION_IN_BEGIN_END();

  st.insideBeginEnd = false;
}


#define SMART_REDIRECT(FROM, FROM_N, FROM_TYPE, FROM_ARRAY, TO, TO_TYPE, TO_CONSTRUCT) \
  RTAPI RTvoid RTAPIENTRY FROM                                                  \
    ARX_IF(FROM_ARRAY,                                                          \
      (FROM_TYPE* v),                                                           \
      (FROM_TYPE x, FROM_TYPE y                                                 \
        ARX_COMMA_IF(ARX_NOT_EQUAL(FROM_N, 2))                                  \
        ARX_IF(ARX_NOT_EQUAL(FROM_N, 2), FROM_TYPE z, ARX_EMPTY())              \
        ARX_COMMA_IF(ARX_EQUAL(FROM_N, 4))                                      \
        ARX_IF(ARX_EQUAL(FROM_N, 4), FROM_TYPE w, ARX_EMPTY())                  \
      )                                                                         \
    ) {                                                                         \
    TO(TO_CONSTRUCT(                                                            \
      static_cast<TO_TYPE>(ARX_IF(FROM_ARRAY, v[0], x)),                        \
      static_cast<TO_TYPE>(ARX_IF(FROM_ARRAY, v[1], y)),                        \
      ARX_IF(ARX_NOT_EQUAL(FROM_N, 2), static_cast<TO_TYPE>(ARX_IF(FROM_ARRAY, v[2], z)), static_cast<TO_TYPE>(0)), \
      ARX_IF(ARX_EQUAL(FROM_N, 4), static_cast<TO_TYPE>(ARX_IF(FROM_ARRAY, v[3], w)), static_cast<TO_TYPE>(1)) \
    ));                                                                         \
  }

#define SMART_REDIRECT_TYPES(NAME, FROM_N, TO_TYPE, TO_CONSTRUCT)               \
  SMART_REDIRECT(ARX_JOIN_3(NAME, FROM_N, d),  FROM_N, RTdouble, 0, NAME, TO_TYPE, TO_CONSTRUCT) \
  SMART_REDIRECT(ARX_JOIN_3(NAME, FROM_N, f),  FROM_N, RTfloat,  0, NAME, TO_TYPE, TO_CONSTRUCT) \
  SMART_REDIRECT(ARX_JOIN_3(NAME, FROM_N, i),  FROM_N, RTint,    0, NAME, TO_TYPE, TO_CONSTRUCT) \
  SMART_REDIRECT(ARX_JOIN_3(NAME, FROM_N, s),  FROM_N, RTshort,  0, NAME, TO_TYPE, TO_CONSTRUCT) \
  SMART_REDIRECT(ARX_JOIN_3(NAME, FROM_N, dv), FROM_N, RTdouble, 1, NAME, TO_TYPE, TO_CONSTRUCT) \
  SMART_REDIRECT(ARX_JOIN_3(NAME, FROM_N, fv), FROM_N, RTfloat,  1, NAME, TO_TYPE, TO_CONSTRUCT) \
  SMART_REDIRECT(ARX_JOIN_3(NAME, FROM_N, iv), FROM_N, RTint,    1, NAME, TO_TYPE, TO_CONSTRUCT) \
  SMART_REDIRECT(ARX_JOIN_3(NAME, FROM_N, sv), FROM_N, RTshort,  1, NAME, TO_TYPE, TO_CONSTRUCT)

#define SMART_REDIRECT_ALL(NAME)                                                \
  SMART_REDIRECT_TYPES(NAME, 2, float, smart::Vector4f)                         \
  SMART_REDIRECT_TYPES(NAME, 3, float, smart::Vector4f)                         \
  SMART_REDIRECT_TYPES(NAME, 4, float, smart::Vector4f)

SMART_REDIRECT_ALL(rtVertex)
SMART_REDIRECT_ALL(rtNormal)
SMART_REDIRECT_ALL(rtTexCoord)

#undef SMART_REDIRECT_ALL
#undef SMART_REDIRECT_TYPES
#undef SMART_REDIRECT


// -------------------------------------------------------------------------- //
// Implementation - C, Matrix Stack API
// -------------------------------------------------------------------------- //
RTAPI RTvoid RTAPIENTRY rtPushMatrix(void) {
  PRECONDITION_NOT_IN_BEGIN_END();
  PRECONDITION(st.matrixStack.size() < SMART_MATRIX_STACK_DEPTH, RT_STACK_OVERFLOW);

  st.matrixStack.push_back(st.matrix);
}

RTAPI RTvoid RTAPIENTRY rtPopMatrix(void) {
  PRECONDITION_NOT_IN_BEGIN_END();
  PRECONDITION(st.matrixStack.size() > 0, RT_STACK_UNDERFLOW);

  st.matrix = st.matrixStack.back();
  st.matrixStack.pop_back();
}

RTAPI RTvoid RTAPIENTRY rtLoadIdentity(void) {
  PRECONDITION_NOT_IN_BEGIN_END();

  st.matrix = smart::Matrix4f::Identity();
}

RTAPI RTvoid RTAPIENTRY rtLoadMatrixd(const RTdouble *m) {
  PRECONDITION_NOT_IN_BEGIN_END();

  /* Assert col-major storage. */
  assert(&st.matrix(1, 0) == &st.matrix(1));

  /* Copy */
  for(int i = 0; i < 16; i++)
    st.matrix(i) = static_cast<float>(m[i]);
}

RTAPI RTvoid RTAPIENTRY rtLoadMatrixf(const RTfloat *m) {
  PRECONDITION_NOT_IN_BEGIN_END();

  /* Assert col-major storage. */
  assert(&st.matrix(1, 0) == &st.matrix(1));

  /* Assert size & position. */
  STATIC_ASSERT((sizeof(smart::Matrix4f) == 16 * sizeof(RTfloat)));
  assert(static_cast<void*>(&st.matrix) == static_cast<void*>(&st.matrix(0)));

  /* Copy. */
  rtLoadMatrix(*reinterpret_cast<const smart::Matrix4f*>(m));
}

RTAPI RTvoid RTAPIENTRY rtMultMatrixd(const RTdouble *m) {
  PRECONDITION_NOT_IN_BEGIN_END();

  smart::Matrix4f tmp;

  /* Assert col-major storage. */
  assert(&tmp(1, 0) == &tmp(1));

  /* Copy */
  for(int i = 0; i < 16; i++)
    tmp(i) = static_cast<float>(m[i]);

  /* Multiply. */
  rtMultMatrix(tmp);
}

RTAPI RTvoid RTAPIENTRY rtMultMatrixf(const RTfloat *m) {
  PRECONDITION_NOT_IN_BEGIN_END();

  /* Assert col-major storage. */
  assert(&st.matrix(1, 0) == &st.matrix(1));

  /* Assert size & position. */
  STATIC_ASSERT((sizeof(smart::Matrix4f) == 16 * sizeof(RTfloat)));
  assert(static_cast<void*>(&st.matrix) == static_cast<void*>(&st.matrix(0)));

  /* Multiply. */
  rtMultMatrix(*reinterpret_cast<const smart::Matrix4f*>(m));
}

RTAPI RTvoid RTAPIENTRY rtRotated(RTdouble angle, RTdouble x, RTdouble y, RTdouble z) {
  rtRotatef(
    static_cast<RTfloat>(angle), 
    static_cast<RTfloat>(x), 
    static_cast<RTfloat>(y), 
    static_cast<RTfloat>(z)
  );
}

RTAPI RTvoid RTAPIENTRY rtRotatef(RTfloat angle, RTfloat x, RTfloat y, RTfloat z) {
  PRECONDITION_NOT_IN_BEGIN_END();
  
  float angleRadians = angle * (static_cast<float>(M_PI) / 180.0f);
  
  smart::Vector3f axis(x, y, z);
  axis.normalize();

  smart::Matrix4f hat = smart::Matrix4f::Zero();
  hat(1, 0) =  axis[2];
  hat(2, 0) = -axis[1];
  hat(2, 1) =  axis[0];
  hat(0, 1) = -axis[2];
  hat(0, 2) =  axis[1];
  hat(1, 2) = -axis[0];

  rtMultMatrix(arx::Matrix4f::Identity() + hat * sin(angleRadians) + hat * hat * (1 - cos(angleRadians)));
}

RTAPI RTvoid RTAPIENTRY rtScaled(RTdouble x, RTdouble y, RTdouble z) {
  rtScalef(
    static_cast<RTfloat>(x), 
    static_cast<RTfloat>(y), 
    static_cast<RTfloat>(z)
  );
}

RTAPI RTvoid RTAPIENTRY rtScalef(RTfloat x, RTfloat y, RTfloat z) {
  PRECONDITION_NOT_IN_BEGIN_END();

  st.matrix.col(0) *= x;
  st.matrix.col(1) *= y;
  st.matrix.col(2) *= z;
}

RTAPI RTvoid RTAPIENTRY rtTranslated(RTdouble x, RTdouble y, RTdouble z) {
  rtTranslatef(
    static_cast<RTfloat>(x), 
    static_cast<RTfloat>(y), 
    static_cast<RTfloat>(z)
  );
}

RTAPI RTvoid RTAPIENTRY rtTranslatef(RTfloat x, RTfloat y, RTfloat z) {
  PRECONDITION_NOT_IN_BEGIN_END();

  smart::Matrix4f m = smart::Matrix4f::Identity();
  m(0, 3) = x;
  m(1, 3) = y;
  m(2, 3) = z;
  rtMultMatrix(m);
}


// -------------------------------------------------------------------------- //
// Implementation - C, Shader API
// -------------------------------------------------------------------------- //
RTAPI RTvoid RTAPIENTRY rtUseLight(RTuint lightShaderId) {
  PRECONDITION(!st.scene->hasLightShader(lightShaderId), RT_INVALID_OPERATION);
  rtUseLightFast(lightShaderId);
}

RTAPI RTvoid RTAPIENTRY rtUseLightFast(RTuint lightShaderId) {
  PRECONDITION_NOT_IN_BEGIN_END();
  PRECONDITION(st.core->hasShader(lightShaderId), RT_INVALID_VALUE);
  PRECONDITION(st.core->getShader(lightShaderId)->getClass()->getType() == smart::LIGHT_SHADER, RT_INVALID_OPERATION);

  st.scene->useLightShader(lightShaderId);
}

RTAPI RTvoid RTAPIENTRY rtUseCamera(RTuint cameraId) {
  PRECONDITION_NOT_IN_BEGIN_END();
  PRECONDITION(st.core->hasShader(cameraId), RT_INVALID_VALUE);
  PRECONDITION(st.core->getShader(cameraId)->getClass()->getType() == smart::CAMERA_SHADER, RT_INVALID_OPERATION);

  st.scene->useCameraShader(cameraId);
}

RTAPI RTvoid RTAPIENTRY rtUseEnvironmentShader(RTuint shaderId) {
  PRECONDITION_NOT_IN_BEGIN_END();
  PRECONDITION(st.core->hasShader(shaderId), RT_INVALID_VALUE);
  PRECONDITION(st.core->getShader(shaderId)->getClass()->getType() == smart::ENV_SHADER, RT_INVALID_OPERATION);

  st.scene->useEnvShader(shaderId);
}

RTAPI RTuint RTAPIENTRY rtParameterHandle(const char *paramName) {
  PRECONDITION_NOT_IN_BEGIN_END_RET(RT_INVALID);
  PRECONDITION_RET(st.shaderClass != NULL, RT_INVALID_OPERATION, RT_INVALID);
  PRECONDITION_RET(st.core->hasShaderParam(st.shaderClass, paramName), RT_INVALID_VALUE, RT_INVALID);

  return st.core->getShaderParam(st.shaderClass, paramName)->getId();
}

RTAPI RTvoid RTAPIENTRY rtParameter1i(RTuint paramHandle, RTint v) {
  rtParameterRaw(paramHandle, &v, sizeof(RTint));
}

RTAPI RTvoid RTAPIENTRY rtParameter1f(RTuint paramHandle, RTfloat v) {
  rtParameterRaw(paramHandle, &v, sizeof(RTfloat));
}

RTAPI RTvoid RTAPIENTRY rtParameter2f(RTuint paramHandle, RTfloat v0, RTfloat v1) {
  RTfloat v[2] = {v0, v1};
  rtParameter2fv(paramHandle, v);
}

RTAPI RTvoid RTAPIENTRY rtParameter3f(RTuint paramHandle, RTfloat v0, RTfloat v1, RTfloat v2) {
  RTfloat v[3] = {v0, v1, v2};
  rtParameter3fv(paramHandle, v);
}

RTAPI RTvoid RTAPIENTRY rtParameter4f(RTuint paramHandle, RTfloat v0, RTfloat v1, RTfloat v2, RTfloat v3) {
  RTfloat v[4] = {v0, v1, v2, v3};
  rtParameter4fv(paramHandle, v);
}

RTAPI RTvoid RTAPIENTRY rtParameter16f(RTuint paramHandle, 
                                       RTfloat v0, RTfloat v1, RTfloat v2, RTfloat v3,
                                       RTfloat v4, RTfloat v5, RTfloat v6, RTfloat v7,
                                       RTfloat v8, RTfloat v9, RTfloat va, RTfloat vb,
                                       RTfloat vc, RTfloat vd, RTfloat ve, RTfloat vf) {
  RTfloat v[16] = {v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, va, vb, vc, vd, ve, vf};
  rtParameter16fv(paramHandle, v);
}

RTAPI RTvoid RTAPIENTRY rtParameter1fv(RTuint paramHandle, RTfloat *v) {
  rtParameterRaw(paramHandle, v, 1 * sizeof(RTfloat));
}

RTAPI RTvoid RTAPIENTRY rtParameter2fv(RTuint paramHandle, RTfloat *v) {
  rtParameterRaw(paramHandle, v, 2 * sizeof(RTfloat));
}

RTAPI RTvoid RTAPIENTRY rtParameter3fv(RTuint paramHandle, RTfloat *v) {
  rtParameterRaw(paramHandle, v, 3 * sizeof(RTfloat));
}

RTAPI RTvoid RTAPIENTRY rtParameter4fv(RTuint paramHandle, RTfloat *v) {
  rtParameterRaw(paramHandle, v, 4 * sizeof(RTfloat));
}

RTAPI RTvoid RTAPIENTRY rtParameter16fv(RTuint paramHandle, RTfloat *v) {
  rtParameterRaw(paramHandle, v, 16 * sizeof(RTfloat));
}

RTAPI RTvoid RTAPIENTRY rtParameterRaw(RTuint paramHandle, RTvoid *data, RTuint size) {
  PRECONDITION(st.core->hasShaderParam(paramHandle), RT_INVALID_VALUE);
  /* If we have this param registered, then st.shaderClass cannot be NULL. */

  smart::ShaderParam* param = st.core->getShaderParam(paramHandle);

  PRECONDITION(st.shaderClass->getId() == param->getClass()->getId(), RT_INVALID_OPERATION);

  switch(param->getType()) {
  case smart::PER_SHADER:
    st.core->setShaderParam(param, st.core->getShader(st.shaderId)->getUniformParam(), data, size);
    break;
  case smart::PER_TRIANGLE:
    if(st.shaderTriangleParamUsed) {
      st.shaderTriangleParam = st.model->newShaderTriangleParam(st.shaderClass->asSurface());
      st.shaderTriangleParamUsed = false;
    }
    st.core->setShaderParam(param, st.shaderTriangleParam, data, size);
    break;
  case smart::PER_VERTEX:
    if(st.shaderAttribParamUsed) {
      st.shaderAttribParam = st.model->newShaderAttribParam(st.shaderClass->asSurface());
      st.shaderAttribParamUsed = false;
    }
    st.core->setShaderParam(param, st.shaderTriangleParam, data, size);
    break;
  default:
    Unreachable();
    break;
  }
}

RTAPI RTvoid RTAPIENTRY rtParameterString(RTuint paramHandle, const char *s) {
  rtParameterRaw(paramHandle, &s, sizeof(s));
}

RTAPI RTuint RTAPIENTRY rtGenNewShaderClass(const char *shaderName, const char *fileName);

RTAPI RTuint RTAPIENTRY rtGenNewShader(void) {
  PRECONDITION_NOT_IN_BEGIN_END_RET(RT_INVALID);
  PRECONDITION_RET(st.shaderClass != NULL, RT_INVALID_OPERATION, RT_INVALID);

  int shaderId = st.core->newShader(st.shaderClass);
  rtBindShader(shaderId);
  return shaderId;
}

RTAPI RTvoid RTAPIENTRY rtBindShaderClass(RTuint shaderClassId) {
  PRECONDITION_NOT_IN_BEGIN_END();
  PRECONDITION(st.core->hasShaderClass(shaderClassId), RT_INVALID_OPERATION);
  
  st.shaderClass = st.core->getShaderClass(shaderClassId);
  st.shader = NULL;
  st.shaderId = smart::SMART_INVALID_ID;
}

RTAPI RTvoid RTAPIENTRY rtBindShader(RTuint id) {
  PRECONDITION_NOT_IN_BEGIN_END();
  PRECONDITION(st.core->hasShader(id), RT_INVALID_OPERATION);
  
  st.shaderId = id;
  st.shader = st.core->getShader(id);
  st.shaderClass = st.shader->getClass();

  if(st.shaderClass->getType() == smart::SURFACE_SHADER) {
    st.shaderAttribParam = st.model->newShaderAttribParam(st.shaderClass->asSurface());
    st.shaderAttribParamUsed = false;
    st.shaderTriangleParam = st.model->newShaderAttribParam(st.shaderClass->asSurface());
    st.shaderTriangleParamUsed = false;
  }
}


// -------------------------------------------------------------------------- //
// Implementation - C, Object Manipulation API
// -------------------------------------------------------------------------- //
RTAPI RTvoid RTAPIENTRY rtInstantiateObject(RTuint objectId) {
  PRECONDITION_NOT_IN_BEGIN_END();
  PRECONDITION_NOT_IN_NEWOBJECT();
  PRECONDITION(st.core->hasModel(objectId), RT_INVALID_VALUE);

  st.scene->newObject(st.core->getModel(objectId), st.matrix);
}

RTAPI RTuint RTAPIENTRY rtNewObject(RTenum mode) {
  PRECONDITION_NOT_IN_BEGIN_END_RET(RT_INVALID);
  PRECONDITION_NOT_IN_NEWOBJECT_RET(RT_INVALID);
  switch(mode) {
    case RT_DEFINE:
    case RT_DEFINE_AND_INSTANTIATE:
      break;
    default:
      st.signalError(RT_INVALID_ENUM);
      return RT_INVALID;
  }

  st.insideNewObject = true;
  st.newObjectMode = mode;
  st.sceneModel = st.model;
  st.model = st.core->newModel();
  return st.model->getId();
}

RTAPI RTvoid RTAPIENTRY rtEndObject(void) {
  PRECONDITION_NOT_IN_BEGIN_END();
  PRECONDITION_IN_NEWOBJECT();

  if(st.newObjectMode == RT_DEFINE_AND_INSTANTIATE)
    rtInstantiateObject(st.model->getId());

  st.model->compileGeometry();

  st.model = st.sceneModel;
  st.insideNewObject = false;
}

RTAPI RTvoid RTAPIENTRY rtRemoveObject(RTuint objectId) {
  PRECONDITION_NOT_IN_BEGIN_END();
  PRECONDITION_NOT_IN_NEWOBJECT();
  PRECONDITION(st.core->hasModel(objectId), RT_INVALID_VALUE);

  st.core->releaseModel(st.core->getModel(objectId));
}


} // extern "C"

