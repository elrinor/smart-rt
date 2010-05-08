#define NOMINMAX
#include <Windows.h>
#include <gl/gl.h>
#include "smart/util/DisableMSVCWarnings.h"
#include "smart/SmartPlus.h"
#include "cavelibpp/cavelibpp.h"

#include <cstdlib>

#include <algorithm>

#include <arx/Utility.h>

#include <cmath>
#ifndef M_PI
#  define M_PI 3.14159265358979323846
#endif

#define LAND_SIZE 64
#define LAND_STEP 1.0f
#define LAND_MAX_COORD (LAND_SIZE * LAND_STEP / 2)

smart::Vector3f** land; // land

smart::Vector3f getLandPoint ( float x, float y )
{
  int ix1 = x / LAND_STEP + LAND_SIZE / 2;
  int ix2 = ix1 + 1;
  int iy1 = y / LAND_STEP + LAND_SIZE / 2;
  int iy2 = iy1 + 1;

  float x1 = ( ix1 - LAND_SIZE / 2 ) * LAND_STEP;
  float x2 = ( ix2 - LAND_SIZE / 2 ) * LAND_STEP;
  float y1 = ( iy1 - LAND_SIZE / 2 ) * LAND_STEP;
  float y2 = ( iy2 - LAND_SIZE / 2 ) * LAND_STEP;

  smart::Vector3f up = ( land[ix1][iy1] * ( y2 - y ) + land[ix1][iy2] * ( y - y1 ) ) / ( y2 - y1 );
  smart::Vector3f dn = ( land[ix2][iy1] * ( y2 - y ) + land[ix2][iy2] * ( y - y1 ) ) / ( y2 - y1 );
  smart::Vector3f result = ( up * ( x2 - x ) + dn * ( x - x1 ) ) / ( x2 - x1 );
  return result;
}


float rnd ( float f )
{
  return f * rand() / RAND_MAX;
}

void addMound ( smart::Vector3f** land, float x, float y, float r, float h )
{
  for ( int ix = 0; ix < LAND_SIZE; ix++ )
    for ( int iy = 0; iy < LAND_SIZE; iy++ )
      land[ix][iy][2] += h * std::max ( 1.0f / ( 1.0f + ( arx::sqr ( land[ix][iy][0] - x ) + arx::sqr ( land[ix][iy][1] - y ) ) * 4.0f / arx::sqr ( r ) ) - 0.2f, 0.0f );
}

void landVertex ( smart::Vector3f** land, int ix, int iy )
{
  rtTexCoord (smart::Vector3f(land[ix][iy] / 10) );
  rtNormal ( smart::Vector3f(( land[ix + 1][iy] - land[ix - 1][iy] ) .cross ( land[ix][iy + 1] - land[ix][iy - 1] ) ) );
  rtVertex ( land[ix][iy] );
}

int generateDesertList(int a, int b)
{
  land = new smart::Vector3f*[LAND_SIZE];
  for ( int i = 0; i < LAND_SIZE; i++ )
    land[i] = new smart::Vector3f[LAND_SIZE];

  for ( int ix = 0; ix < LAND_SIZE; ix++ ) {
    for ( int iy = 0; iy < LAND_SIZE; iy++ ) {
      land[ix][iy][0] = ( ix - LAND_SIZE / 2 ) * LAND_STEP;
      land[ix][iy][1] = ( iy - LAND_SIZE / 2 ) * LAND_STEP;
      land[ix][iy][2] = 3;
    }
  }

  addMound ( land, 0, 0, 15, -15 );

  for ( int i = 0; i < 1000; i++ ) {
    float x = rnd ( LAND_MAX_COORD ) * 2 - LAND_MAX_COORD;
    float y = rnd ( LAND_MAX_COORD ) * 2 - LAND_MAX_COORD;
    if ( arx::sqr ( x ) + arx::sqr ( y ) < 25 )
      continue;
    addMound ( land, x, y, 1.0 + rnd ( 8 ), rnd ( 2 ) - 1 );
  }

  for ( int i = 0; i < 64; i++ ) {
    float phi = 2 * M_PI * i / 64;
    float x = LAND_MAX_COORD * sin ( phi );
    float y = LAND_MAX_COORD * cos ( phi );
    addMound ( land, x, y, 5.0 + rnd ( 8 ), 1 + rnd ( 10 ) );
  }

  rtBindShader(a);
  int result = rtNewObject ( RT_COMPILE );
  rtBegin ( RT_QUADS );
  for ( int ix = 1; ix < LAND_SIZE - 2; ix++ ) {
    for ( int iy = 1; iy < LAND_SIZE - 2; iy++ ) {
      landVertex ( land, ix    , iy    );
      landVertex ( land, ix + 1, iy    );
      landVertex ( land, ix + 1, iy + 1 );
      landVertex ( land, ix    , iy + 1 );
    }
  }
  rtEnd();

  rtBindShader(b);
  rtBegin ( RT_QUADS );
  rtNormal3f(0,0 ,1);
  rtVertex3f ( -07, -07, -2.5 );
  rtVertex3f ( -07,  07, -2.5 );
  rtVertex3f (  07,  07, -2.5 );
  rtVertex3f (  07, -07, -2.5 );
  rtEnd();

  rtEndObject();

  return result;
}


class InfiniteLightShader {
public:
  InfiniteLightShader(const smart::Vector3f& d, const smart::Radiance& r): mRadiance(r), mDirection(d) {}

  void registerParams(const smart::ShaderRegistrator& r) const {
    r.registerParam<smart::Radiance>("radiance", smart::PER_SHADER, offsetof(InfiniteLightShader, mRadiance));
    r.registerParam<smart::Vector3f>("direction", smart::PER_SHADER, offsetof(InfiniteLightShader, mDirection));
  }

  bool illuminate(const smart::Vector3f& position, smart::Vector3f& direction, float& distance, smart::Radiance& radiance) const {
    direction = mDirection;
    distance = 100;
    radiance = mRadiance;
    return true;
  }
private:
  smart::Vector3f mDirection;
  smart::Radiance mRadiance;
};



class StrangeShader {
public:
  void shade(smart::TraceContext& ctx) const {
    smart::Vector3f v = ctx.getIncomingDirection();
    ctx.setRadiance(smart::Radiance(v[0], v[1], v[2]));
  }

  void registerParams(const smart::ShaderRegistrator& registrator) const {}

};

class Callback: public cave::Callback {
public:
  void quad ( smart::Vector3f v, smart::Vector3f x, smart::Vector3f y )
  {
    rtBegin ( RT_QUADS );
    rtTexCoord2f ( 0, 0 ); rtVertex ( smart::Vector3f(v)        );
    rtTexCoord2f ( 1, 0 ); rtVertex ( smart::Vector3f(v + x)    );
    rtTexCoord2f ( 1, 1 ); rtVertex ( smart::Vector3f(v + x + y) );
    rtTexCoord2f ( 0, 1 ); rtVertex ( smart::Vector3f(v     + y) );
    rtEnd();
  }

  virtual void init() {
    rtGenNewShaderClass<smart::ConstantShader>();
    mConstSfcShaderId  = rtGenNewShader();

    rtGenNewShaderClass<smart::SpotLightShader>();
    mSpotLightShaderId = rtGenNewShader();
    mSpotLightShaderId2 = rtGenNewShader();

    rtGenNewShaderClass<smart::DiffuseShader>();
    mDiffShaderId      = rtGenNewShader();

    rtGenNewShaderClass<StrangeShader>();
    mEnvShaderId       = rtGenNewShader();

    rtGenNewShaderClass<smart::PinholeCameraShader>();
    mCamShaderId       = rtGenNewShader();

    rtGenNewShaderClass<smart::MirrorShader>();
    mMirrorShaderId    = rtGenNewShader();

    rtGenNewShaderClass<smart::TexturedDiffuseShader>();
    mTextureShaderId    = rtGenNewShader();
    rtParameter(rtParameterHandle("radiance"), smart::Radiance(0.4, 0.4, 0.3));
    rtParameter(rtParameterHandle("texture"), rtGenTexture(arx::Image3f::loadFromFile("texture.bmp")));
    mDesertShaderId     = rtGenNewShader();
    rtParameter(rtParameterHandle("radiance"), smart::Radiance(0.4, 0.4, 0.3));
    rtParameter(rtParameterHandle("texture"), rtGenTexture(arx::Image3f::loadFromFile("dirt.bmp")));

    rtGenNewShaderClass<InfiniteLightShader>();
    mInfLightShaderId   = rtGenNewShader();
    rtParameter(rtParameterHandle("radiance"), smart::Radiance(1.0, 1.0, 0.7));
    rtParameter(rtParameterHandle("direction"), smart::Vector3f(0.4, 1, 1).normalized());

    rtGenNewShaderClass<smart::TextureShader>();
    PX = rtGenNewShader();
    rtParameter(rtParameterHandle("texture"), rtGenTexture(arx::Image3f::loadFromFile("skybox_px.bmp")));
    NX = rtGenNewShader();
    rtParameter(rtParameterHandle("texture"), rtGenTexture(arx::Image3f::loadFromFile("skybox_nx.bmp")));
    PY = rtGenNewShader();
    rtParameter(rtParameterHandle("texture"), rtGenTexture(arx::Image3f::loadFromFile("skybox_py.bmp")));
    NY = rtGenNewShader();
    rtParameter(rtParameterHandle("texture"), rtGenTexture(arx::Image3f::loadFromFile("skybox_ny.bmp")));
    PZ = rtGenNewShader();
    rtParameter(rtParameterHandle("texture"), rtGenTexture(arx::Image3f::loadFromFile("skybox_pz.bmp")));
    NZ = rtGenNewShader();
    rtParameter(rtParameterHandle("texture"), rtGenTexture(arx::Image3f::loadFromFile("skybox_nz.bmp")));

//    rtBindShader(mDesertShaderId);
    mDesertId = generateDesertList(mDesertShaderId, mMirrorShaderId);

    mModelId = rtNewObject(RT_DEFINE);
    const int TORUS_FACES = 32;
    const float _R = 2;
    const float _r = 0.6;
    static float torusVertex[TORUS_FACES + 1][TORUS_FACES + 1][3], torusNormal[TORUS_FACES + 1][TORUS_FACES + 1][3], torusTexCoord[TORUS_FACES + 1][TORUS_FACES + 1][2];
    for(int i = 0; i <= TORUS_FACES; i++) 
    {
      for(int j = 0; j <= TORUS_FACES; j++) 
      {
        float phi = 2.0f * M_PI * i / TORUS_FACES;
        float psi = 2.0f * M_PI * j / TORUS_FACES;

        torusVertex[i][j][0] = (_R + _r * sin(psi)) * cos(phi);
        torusVertex[i][j][1] = (_R + _r * sin(psi)) * sin(phi);
        torusVertex[i][j][2] = _r * cos(psi);

        torusNormal[i][j][0] = sin(psi) * cos(phi);
        torusNormal[i][j][1] = sin(psi) * sin(phi);
        torusNormal[i][j][2] = cos(psi);

        torusTexCoord[i][j][0] = 2.0f * i / TORUS_FACES;
        torusTexCoord[i][j][1] = 1.0f * j / TORUS_FACES;
      }
    }
    for(int i = 0; i < TORUS_FACES; i++) 
    {
      for(int j = 0; j < TORUS_FACES; j++) 
      {
        rtBindShader(mMirrorShaderId);
        rtBegin(RT_QUADS);
        rtTexCoord2f(torusTexCoord[i  ][j  ][0], torusTexCoord[i  ][j  ][1]);
        rtNormal3f(torusNormal[i  ][j  ][0], torusNormal[i  ][j  ][1], torusNormal[i  ][j  ][2]);
        rtVertex3f(torusVertex[i  ][j  ][0], torusVertex[i  ][j  ][1], torusVertex[i  ][j  ][2]);

        rtTexCoord2f(torusTexCoord[i+1][j  ][0], torusTexCoord[i+1][j  ][1]);
        rtNormal3f(torusNormal[i+1][j  ][0], torusNormal[i+1][j  ][1], torusNormal[i+1][j  ][2]);
        rtVertex3f(torusVertex[i+1][j  ][0], torusVertex[i+1][j  ][1], torusVertex[i+1][j  ][2]);

        rtTexCoord2f(torusTexCoord[i+1][j+1][0], torusTexCoord[i+1][j+1][1]);
        rtNormal3f(torusNormal[i+1][j+1][0], torusNormal[i+1][j+1][1], torusNormal[i+1][j+1][2]);
        rtVertex3f(torusVertex[i+1][j+1][0], torusVertex[i+1][j+1][1], torusVertex[i+1][j+1][2]);

        rtTexCoord2f(torusTexCoord[i  ][j+1][0], torusTexCoord[i  ][j+1][1]);
        rtNormal3f(torusNormal[i  ][j+1][0], torusNormal[i  ][j+1][1], torusNormal[i  ][j+1][2]);
        rtVertex3f(torusVertex[i  ][j+1][0], torusVertex[i  ][j+1][1], torusVertex[i  ][j+1][2]);
        rtEnd();
      }
    }
    rtEndObject();

    mSphereId = rtNewObject(RT_DEFINE);
    rtBindShader(mTextureShaderId);
    const int _slices = 16;
    const int _stacks = 16;
    int i, j;
    for(i = 0; i <= _slices; i++) {
      float lat0 = M_PI * (-0.5 + (float) (i - 1) / _slices);
      float z0  = sin(lat0);
      float zr0 =  cos(lat0);

      float lat1 = M_PI * (-0.5 + (float) i / _slices);
      float z1 = sin(lat1);
      float zr1 = cos(lat1);

      rtBegin(RT_QUAD_STRIP);
      for(j = 0; j <= _stacks; j++) {
        float lng = 2 * M_PI * (float) (j - 1) / _stacks;
        float x = cos(lng);
        float y = sin(lng);

        rtTexCoord2f((float) (i - 1) / _slices, (float) (j - 1) / _stacks);
        rtNormal3f(x * zr0, y * zr0, z0);
        rtVertex3f(x * zr0, y * zr0, z0);
        rtTexCoord2f((float) i / _slices, (float) (j - 1) / _stacks);
        rtNormal3f(x * zr1, y * zr1, z1);
        rtVertex3f(x * zr1, y * zr1, z1);
      }
      rtEnd();
    }
    rtEndObject();

    {
      using smart::Vector3f;
      const float r = 1000;
      mSkyId = rtNewObject(RT_DEFINE);
      rtBindShader(PZ);
      quad ( Vector3f ( -r, -r, r ), Vector3f ( 2 * r, 0, 0 ), Vector3f ( 0, 2 * r, 0 ) );
      rtBindShader(NZ);
      quad ( Vector3f ( -r, -r, -r ), Vector3f ( 2 * r, 0, 0 ), Vector3f ( 0, 2 * r, 0 ) );
      rtBindShader(PX);
      quad ( Vector3f ( r, r, r ), Vector3f ( 0, -2 * r, 0 ), Vector3f ( 0, 0, -2 * r ) );
      rtBindShader(NX);
      quad ( Vector3f ( -r, -r, r ), Vector3f ( 0, 2 * r, 0 ), Vector3f ( 0, 0, -2 * r ) );
      rtBindShader(PY);
      quad ( Vector3f ( -r, r, r ), Vector3f ( 2 * r, 0, 0 ), Vector3f ( 0, 0, -2 * r ) );
      rtBindShader(NY);
      quad ( Vector3f ( r, -r, r ), Vector3f ( -2 * r, 0, 0 ), Vector3f ( 0, 0, -2 * r ) );
      rtEndObject();
    }


    int size = 256;

    mFirstFrame = true;
    mCurrentTarget = 0;
    mTarget[0] = arx::Image3f(size, size);
    mTarget[1] = arx::Image3f(size, size);

    mTexture = 0;

    glEnable(GL_TEXTURE_2D);
  }
  virtual void frame() {
  }
  virtual void display() {
    using namespace smart;
    
    float t = getFrameStartTime();
    float dt = getTimeFromLastFrame();

    cave::Camera camera = computeCamera();

    rtUseCamera(mCamShaderId);
    rtBindShader(mCamShaderId);
    rtParameter(rtParameterHandle("origin"),     camera.pos);
    rtParameter(rtParameterHandle("lowerLeft"),  Vector3f(camera.dir - camera.up - camera.right));
    rtParameter(rtParameterHandle("vertical"),   Vector3f(camera.up * 2));
    rtParameter(rtParameterHandle("horizontal"), Vector3f(camera.right * 2));

    rtBindShader(mDiffShaderId);
    rtParameter(rtParameterHandle("color"), Color(0, 0.5f + 0.5f * cos(t), 0.5f + 0.5f * sin(t)));

    rtBindShader(mConstSfcShaderId);
    rtParameter(rtParameterHandle("value"), Radiance(1, 1, 1));

    rtBindShader(mMirrorShaderId);
    rtParameter(rtParameterHandle("attenuation"), 1.0f);
/*
    rtUseLight(mSpotLightShaderId);
    rtBindShader(mSpotLightShaderId);
    rtParameter(rtParameterHandle("radiance"), Radiance(300.0, 300.0, 300.0));
    rtParameter(rtParameterHandle("position"), Vector3f(10, 10, 10));
*/
    
    rtUseLight(mInfLightShaderId);

/*    rtUseLight(mSpotLightShaderId2);
    rtBindShader(mSpotLightShaderId2);
    rtParameter(rtParameterHandle("radiance"), Radiance(1.0, 1.0, 1.0));
    rtParameter(rtParameterHandle("position"), Vector3f(2.0f * sin(t + M_PI), 2.0f * cos(t + M_PI), 2.0f));
*/
    
    rtUseEnvironmentShader(mEnvShaderId);

    rtPushMatrix();
    rtTranslatef(0, 0, 6);

    rtPushMatrix();
    //rtScalef(2, 2, 2);
    rtRotatef(2 * t / M_PI * 180, 1, 0, 0);
    //rtTranslatef(2, 0, 0);
    rtInstantiateObject(mModelId);

    rtPopMatrix();

    rtPushMatrix();
    rtTranslatef(0, 2 * cos(2 * t), 2 + 2 * sin(2 * t));
    rtInstantiateObject(mSphereId);
    rtPopMatrix();

    rtPopMatrix();
/*
    rtPushMatrix();
    rtTranslatef(0, 2 * cos(2 * t), -2 + 2 * sin(2 * t));
    rtInstantiateObject(mSphereId);
    rtPopMatrix();
*/

    rtPushMatrix();
    //rtRotatef(10 * t, 0, 0, 1);
    rtInstantiateObject(mDesertId);
    rtPopMatrix();


    rtInstantiateObject(mSkyId);

    if(mFirstFrame)
      mFirstFrame = false;
    else 
      rtEndRendering();
    arx::Image3f target = mTarget[mCurrentTarget];
    
    mCurrentTarget = 1 - mCurrentTarget;
    rtSetFrameBuffer(
      reinterpret_cast<RTubyte*>(mTarget[mCurrentTarget].getPixelData()), 
      mTarget[mCurrentTarget].getWidth(),
      mTarget[mCurrentTarget].getHeight(),
      RT_BGR_32
    );

    rtStartRendering();

    if(mTexture != 0)
      glDeleteTextures(1, &mTexture);

    glGenTextures(1, &mTexture);
    glBindTexture(GL_TEXTURE_2D, mTexture);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    glTexImage2D(GL_TEXTURE_2D, 0, 3, target.getWidth(), target.getHeight(), 
      0, GL_BGR_EXT, GL_FLOAT, target.getPixelData());

    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(-1, -1, 0);
    glTexCoord2f(1, 0); glVertex3f( 1, -1, 0);
    glTexCoord2f(1, 1); glVertex3f( 1,  1, 0);
    glTexCoord2f(0, 1); glVertex3f(-1,  1, 0);
    glEnd();

    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
  }
  virtual void main() {

  }

private:
  arx::Image3f mTarget[2];
  int mCurrentTarget;
  bool mFirstFrame;

  int mCamShaderId;
  int mEnvShaderId;
  int mConstSfcShaderId;
  int mDiffShaderId;
  int mSpotLightShaderId;
  int mSpotLightShaderId2;
  int mInfLightShaderId;
  int mMirrorShaderId;
  int mTextureShaderId;
  int mDesertShaderId;
  int PX, PY, PZ, NX, NY, NZ;


  int mSkyId;
  int mModelId;
  int mSphereId;
  int mDesertId;

  GLuint mTexture;
};


int main(int argc, char **argv) 
{
  srand(clock());

  rtInit(&argc, argv);

  cave::Renderer renderer(cave::Renderer::MODE_GLUT, arx::shared_ptr<cave::Callback>(new Callback()));
  renderer.init(&argc, argv);
  renderer.run();

  return 0;
}


