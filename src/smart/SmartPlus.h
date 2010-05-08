#ifndef __SMART_SMARTPLUS_H__
#define __SMART_SMARTPLUS_H__

#include "SmartDefs.h"
#include "Smart.h"
#include "core/SmartCore.h"

// -------------------------------------------------------------------------- //
// SmartState
// -------------------------------------------------------------------------- //
namespace smart {
  namespace cface {
    struct GeometryData {
      smart::Vector4f normal;
      smart::Vector4f texCoord;
      int n;
      int ids[3];
      RTenum mode;
    };

    struct SmartState {
      RTenum errorCode;
      GeometryData geometry;
      smart::SmartCore* core;
      smart::ShadedScene* scene;
      smart::ShadedModel* sceneModel;
      smart::ShadedModel* model;
      smart::ShaderClass* shaderClass;
      smart::Shader* shader;
      int shaderId;
      smart::RenderTask* renderTask;
      smart::Matrix4f matrix;
      arx::CheckedArray<smart::Matrix4f> matrixStack;
      void* shaderTriangleParam;
      bool shaderTriangleParamUsed;
      void* shaderAttribParam;
      bool shaderAttribParamUsed;
      arx::Image3f frameBuffer;

      RTenum newObjectMode;

      bool insideBeginEnd;
      bool insideNewObject;

      FORCEINLINE void signalError(RTenum code) {
        if(errorCode == RT_NO_ERROR)
          errorCode = code;
      }
    };

    extern SmartState st;
  }
}


// -------------------------------------------------------------------------- //
// Interface
// -------------------------------------------------------------------------- //
RTAPI RTvoid RTAPIENTRY rtVertex(const smart::Vector4f& v);
RTAPI RTvoid RTAPIENTRY rtNormal(const smart::Vector4f& n);
RTAPI RTvoid RTAPIENTRY rtTexCoord(const smart::Vector4f& t);
RTAPI RTvoid RTAPIENTRY rtVertex(const smart::Vector3f& v);
RTAPI RTvoid RTAPIENTRY rtNormal(const smart::Vector3f& n);
RTAPI RTvoid RTAPIENTRY rtTexCoord(const smart::Vector3f& t);
RTAPI RTvoid RTAPIENTRY rtVertex(const smart::Vector2f& v);
RTAPI RTvoid RTAPIENTRY rtNormal(const smart::Vector2f& n);
RTAPI RTvoid RTAPIENTRY rtTexCoord(const smart::Vector2f& t);

RTAPI RTvoid RTAPIENTRY rtLoadMatrix(const smart::Matrix4f& m);
RTAPI RTvoid RTAPIENTRY rtMultMatrix(const smart::Matrix4f& m);

RTAPI RTuint RTAPIENTRY rtGenTexture(const arx::Image3f& image);
RTAPI RTvoid RTAPIENTRY rtDeleteTexture(RTuint textureId);


template<class T>
RTAPI inline RTuint RTAPIENTRY rtGenNewShaderClass() {
  using smart::cface::st;

  if(st.insideBeginEnd) {
    st.signalError(RT_INVALID_OPERATION);
    return RT_INVALID;
  }

  smart::ShaderClass* shaderClass = st.core->newShaderClass<T>();
  rtBindShaderClass(shaderClass->getId());
  return st.shaderClass->getId();
}

template<class T>
RTAPI inline RTvoid RTAPIENTRY rtParameter(RTuint paramHandle, T v) {
  rtParameterRaw(paramHandle, &v, sizeof(v));
}

template<class T>
RTAPI inline RTuint RTAPIENTRY rtGenNewShaderClass(RTenum type) {
  using smart::cface::st;

  if(st.insideBeginEnd) {
    st.signalError(RT_INVALID_OPERATION);
    return RT_INVALID;
  }

  smart::ShaderClassType classType;

  switch(type) {
  case RT_SURFACE_SHADER:
    classType = smart::SURFACE_SHADER;
    break;
  case RT_ENV_SHADER:
    classType = smart::ENV_SHADER;
    break;
  case RT_LIGHT_SHADER:
    classType = smart::LIGHT_SHADER;
    break;
  case RT_CAMERA_SHADER:
    classType = smart::CAMERA_SHADER;
    break;
  default:
    st.signalError(RT_INVALID_ENUM);
    return RT_INVALID;
  }

  smart::ShaderClass* shaderClass = st.core->newShaderClass<T>(classType);
  rtBindShaderClass(shaderClass->getId());
  return st.shaderClass->getId();
}


#endif