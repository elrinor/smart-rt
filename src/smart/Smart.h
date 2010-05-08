#ifndef __SMART_SMART_H__
#define __SMART_SMART_H__

#include "SmartDefs.h"

#ifdef __cplusplus
extern "C" {
#endif

RTAPI RTvoid RTAPIENTRY rtInit(int *argcp, char *argv[]);
RTAPI RTvoid RTAPIENTRY rtExit(void);
RTAPI RTenum RTAPIENTRY rtGetError(void);

RTAPI RTvoid RTAPIENTRY rtSetFrameBuffer(RTubyte *fb, RTuint width, RTuint height, RTuint format);
RTAPI RTvoid RTAPIENTRY rtRender(void);
RTAPI RTvoid RTAPIENTRY rtStartRendering(void);
RTAPI RTvoid RTAPIENTRY rtEndRendering(void);

RTAPI RTvoid RTAPIENTRY rtBegin(RTenum mode);
RTAPI RTvoid RTAPIENTRY rtEnd(void);

RTAPI RTvoid RTAPIENTRY rtVertex4d(RTdouble x, RTdouble y, RTdouble z, RTdouble w);
RTAPI RTvoid RTAPIENTRY rtVertex4f(RTfloat x, RTfloat y, RTfloat z, RTfloat w);
RTAPI RTvoid RTAPIENTRY rtVertex4i(RTint x, RTint y, RTint z, RTint w);
RTAPI RTvoid RTAPIENTRY rtVertex4s(RTshort x, RTshort y, RTshort z, RTshort w);
RTAPI RTvoid RTAPIENTRY rtVertex4dv(RTdouble *v);
RTAPI RTvoid RTAPIENTRY rtVertex4fv(RTfloat *v);
RTAPI RTvoid RTAPIENTRY rtVertex4iv(RTint *v);
RTAPI RTvoid RTAPIENTRY rtVertex4sv(RTshort *v);

RTAPI RTvoid RTAPIENTRY rtVertex3d(RTdouble x, RTdouble y, RTdouble z);
RTAPI RTvoid RTAPIENTRY rtVertex3f(RTfloat x, RTfloat y, RTfloat z);
RTAPI RTvoid RTAPIENTRY rtVertex3i(RTint x, RTint y, RTint z);
RTAPI RTvoid RTAPIENTRY rtVertex3s(RTshort x, RTshort y, RTshort z);
RTAPI RTvoid RTAPIENTRY rtVertex3dv(RTdouble *v);
RTAPI RTvoid RTAPIENTRY rtVertex3fv(RTfloat *v);
RTAPI RTvoid RTAPIENTRY rtVertex3iv(RTint *v);
RTAPI RTvoid RTAPIENTRY rtVertex3sv(RTshort *v);

RTAPI RTvoid RTAPIENTRY rtVertex2d(RTdouble x, RTdouble y);
RTAPI RTvoid RTAPIENTRY rtVertex2f(RTfloat x, RTfloat y);
RTAPI RTvoid RTAPIENTRY rtVertex2i(RTint x, RTint y);
RTAPI RTvoid RTAPIENTRY rtVertex2s(RTshort x, RTshort y);
RTAPI RTvoid RTAPIENTRY rtVertex2dv(RTdouble *v);
RTAPI RTvoid RTAPIENTRY rtVertex2fv(RTfloat *v);
RTAPI RTvoid RTAPIENTRY rtVertex2iv(RTint *v);
RTAPI RTvoid RTAPIENTRY rtVertex2sv(RTshort *v);

RTAPI RTvoid RTAPIENTRY rtNormal4d(RTdouble x, RTdouble y, RTdouble z, RTdouble w);
RTAPI RTvoid RTAPIENTRY rtNormal4f(RTfloat x, RTfloat y, RTfloat z, RTfloat w);
RTAPI RTvoid RTAPIENTRY rtNormal4i(RTint x, RTint y, RTint z, RTint w);
RTAPI RTvoid RTAPIENTRY rtNormal4s(RTshort x, RTshort y, RTshort z, RTshort w);
RTAPI RTvoid RTAPIENTRY rtNormal4dv(RTdouble *v);
RTAPI RTvoid RTAPIENTRY rtNormal4fv(RTfloat *v);
RTAPI RTvoid RTAPIENTRY rtNormal4iv(RTint *v);
RTAPI RTvoid RTAPIENTRY rtNormal4sv(RTshort *v);

RTAPI RTvoid RTAPIENTRY rtNormal3d(RTdouble x, RTdouble y, RTdouble z);
RTAPI RTvoid RTAPIENTRY rtNormal3f(RTfloat x, RTfloat y, RTfloat z);
RTAPI RTvoid RTAPIENTRY rtNormal3i(RTint x, RTint y, RTint z);
RTAPI RTvoid RTAPIENTRY rtNormal3s(RTshort x, RTshort y, RTshort z);
RTAPI RTvoid RTAPIENTRY rtNormal3dv(RTdouble *v);
RTAPI RTvoid RTAPIENTRY rtNormal3fv(RTfloat *v);
RTAPI RTvoid RTAPIENTRY rtNormal3iv(RTint *v);
RTAPI RTvoid RTAPIENTRY rtNormal3sv(RTshort *v);

RTAPI RTvoid RTAPIENTRY rtNormal2d(RTdouble x, RTdouble y);
RTAPI RTvoid RTAPIENTRY rtNormal2f(RTfloat x, RTfloat y);
RTAPI RTvoid RTAPIENTRY rtNormal2i(RTint x, RTint y);
RTAPI RTvoid RTAPIENTRY rtNormal2s(RTshort x, RTshort y);
RTAPI RTvoid RTAPIENTRY rtNormal2dv(RTdouble *v);
RTAPI RTvoid RTAPIENTRY rtNormal2fv(RTfloat *v);
RTAPI RTvoid RTAPIENTRY rtNormal2iv(RTint *v);
RTAPI RTvoid RTAPIENTRY rtNormal2sv(RTshort *v);

RTAPI RTvoid RTAPIENTRY rtTexCoord4d(RTdouble x, RTdouble y, RTdouble z, RTdouble w);
RTAPI RTvoid RTAPIENTRY rtTexCoord4f(RTfloat x, RTfloat y, RTfloat z, RTfloat w);
RTAPI RTvoid RTAPIENTRY rtTexCoord4i(RTint x, RTint y, RTint z, RTint w);
RTAPI RTvoid RTAPIENTRY rtTexCoord4s(RTshort x, RTshort y, RTshort z, RTshort w);
RTAPI RTvoid RTAPIENTRY rtTexCoord4dv(RTdouble *v);
RTAPI RTvoid RTAPIENTRY rtTexCoord4fv(RTfloat *v);
RTAPI RTvoid RTAPIENTRY rtTexCoord4iv(RTint *v);
RTAPI RTvoid RTAPIENTRY rtTexCoord4sv(RTshort *v);

RTAPI RTvoid RTAPIENTRY rtTexCoord3d(RTdouble x, RTdouble y, RTdouble z);
RTAPI RTvoid RTAPIENTRY rtTexCoord3f(RTfloat x, RTfloat y, RTfloat z);
RTAPI RTvoid RTAPIENTRY rtTexCoord3i(RTint x, RTint y, RTint z);
RTAPI RTvoid RTAPIENTRY rtTexCoord3s(RTshort x, RTshort y, RTshort z);
RTAPI RTvoid RTAPIENTRY rtTexCoord3dv(RTdouble *v);
RTAPI RTvoid RTAPIENTRY rtTexCoord3fv(RTfloat *v);
RTAPI RTvoid RTAPIENTRY rtTexCoord3iv(RTint *v);
RTAPI RTvoid RTAPIENTRY rtTexCoord3sv(RTshort *v);

RTAPI RTvoid RTAPIENTRY rtTexCoord2d(RTdouble x, RTdouble y);
RTAPI RTvoid RTAPIENTRY rtTexCoord2f(RTfloat x, RTfloat y);
RTAPI RTvoid RTAPIENTRY rtTexCoord2i(RTint x, RTint y);
RTAPI RTvoid RTAPIENTRY rtTexCoord2s(RTshort x, RTshort y);
RTAPI RTvoid RTAPIENTRY rtTexCoord2dv(RTdouble *v);
RTAPI RTvoid RTAPIENTRY rtTexCoord2fv(RTfloat *v);
RTAPI RTvoid RTAPIENTRY rtTexCoord2iv(RTint *v);
RTAPI RTvoid RTAPIENTRY rtTexCoord2sv(RTshort *v);

RTAPI RTvoid RTAPIENTRY rtPushMatrix(void);
RTAPI RTvoid RTAPIENTRY rtPopMatrix(void);
RTAPI RTvoid RTAPIENTRY rtLoadIdentity(void);
RTAPI RTvoid RTAPIENTRY rtLoadMatrixd(const RTdouble *m);
RTAPI RTvoid RTAPIENTRY rtLoadMatrixf(const RTfloat *m);
RTAPI RTvoid RTAPIENTRY rtMultMatrixd(const RTdouble *m);
RTAPI RTvoid RTAPIENTRY rtMultMatrixf(const RTfloat *m);
RTAPI RTvoid RTAPIENTRY rtRotated(RTdouble angle, RTdouble x, RTdouble y, RTdouble z);
RTAPI RTvoid RTAPIENTRY rtRotatef(RTfloat angle, RTfloat x, RTfloat y, RTfloat z);
RTAPI RTvoid RTAPIENTRY rtScaled(RTdouble x, RTdouble y, RTdouble z);
RTAPI RTvoid RTAPIENTRY rtScalef(RTfloat x, RTfloat y, RTfloat z);
RTAPI RTvoid RTAPIENTRY rtTranslated(RTdouble x, RTdouble y, RTdouble z);
RTAPI RTvoid RTAPIENTRY rtTranslatef(RTfloat x, RTfloat y, RTfloat z);

RTAPI RTvoid RTAPIENTRY rtUseLight(RTuint lightShaderId);
RTAPI RTvoid RTAPIENTRY rtUseLightFast(RTuint lightShaderId);
RTAPI RTvoid RTAPIENTRY rtUseCamera(RTuint cameraId);
RTAPI RTvoid RTAPIENTRY rtUseRenderingObject(RTuint renderingObjectId);
RTAPI RTvoid RTAPIENTRY rtUseEnvironmentShader(RTuint shaderId);

RTAPI RTuint RTAPIENTRY rtParameterHandle(const char *paramName);
RTAPI RTvoid RTAPIENTRY rtParameter1i(RTuint paramHandle, RTint v);
RTAPI RTvoid RTAPIENTRY rtParameter1f(RTuint paramHandle, RTfloat v);
RTAPI RTvoid RTAPIENTRY rtParameter2f(RTuint paramHandle, RTfloat v0, RTfloat v1);
RTAPI RTvoid RTAPIENTRY rtParameter3f(RTuint paramHandle, RTfloat v0, RTfloat v1, RTfloat v2);
RTAPI RTvoid RTAPIENTRY rtParameter4f(RTuint paramHandle, RTfloat v0, RTfloat v1, RTfloat v2, RTfloat v3);
RTAPI RTvoid RTAPIENTRY rtParameter16f(RTuint paramHandle, 
                                       RTfloat v0, RTfloat v1, RTfloat v2, RTfloat v3,
                                       RTfloat v4, RTfloat v5, RTfloat v6, RTfloat v7,
                                       RTfloat v8, RTfloat v9, RTfloat va, RTfloat vb,
                                       RTfloat vc, RTfloat vd, RTfloat ve, RTfloat vf);
RTAPI RTvoid RTAPIENTRY rtParameter1fv(RTuint paramHandle, RTfloat *v);
RTAPI RTvoid RTAPIENTRY rtParameter2fv(RTuint paramHandle, RTfloat *v);
RTAPI RTvoid RTAPIENTRY rtParameter3fv(RTuint paramHandle, RTfloat *v);
RTAPI RTvoid RTAPIENTRY rtParameter4fv(RTuint paramHandle, RTfloat *v);
RTAPI RTvoid RTAPIENTRY rtParameter16fv(RTuint paramHandle, RTfloat *v);
RTAPI RTvoid RTAPIENTRY rtParameterRaw(RTuint paramHandle, RTvoid *data, RTuint size);
RTAPI RTvoid RTAPIENTRY rtParameterString(RTuint paramHandle, const char *s);

RTAPI RTuint RTAPIENTRY rtGenNewShaderClass(const char *shaderName, const char *fileName);
RTAPI RTuint RTAPIENTRY rtGenNewShader(void);
RTAPI RTvoid RTAPIENTRY rtBindShaderClass(RTuint shaderClassId);
RTAPI RTvoid RTAPIENTRY rtBindShader(RTuint shaderId);

RTAPI RTvoid RTAPIENTRY rtInstantiateObject(RTuint objectId);
RTAPI RTuint RTAPIENTRY rtNewObject(RTenum mode);
RTAPI RTvoid RTAPIENTRY rtEndObject(void);
RTAPI RTvoid RTAPIENTRY rtRemoveObject(RTuint objectId);


#ifdef __cplusplus
}
#endif

#endif // __SMART_SMART_H__
