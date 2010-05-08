#ifndef __SMART_SHADEDSCENE_H__
#define __SMART_SHADEDSCENE_H__

#include "common.h"
#include <arx/Utility.h>
#include "ExplicitlyCounted.h"
#include "Idded.h"
#include "CoreScene.h"
#include "TextureManager.h"
#include "ShaderManager.h"

namespace smart {
// -------------------------------------------------------------------------- //
// ShadedScene
// -------------------------------------------------------------------------- //
  class ShadedScene: private arx::noncopyable, private ExplicitlyCounted<ShadedScene>, public Idded {
  public:
    const BoundingBox& getBoundingBox() const {
      return mScene->getBoundingBox();
    }

    const CoreObject* getObject(int index) const {
      return mScene->getObject(index);
    }

    const int getObjectCount() const {
      return mScene->getObjectCount();
    }

    void newObject(ShadedModel* model, const Matrix4f& transform) {
      mScene->newObject(model, transform);
    }

    void useCameraShader(int cameraShaderId) {
      assert(!mCompiled);
      assert(mShaderManager->getShader(cameraShaderId)->getClass()->getType() == CAMERA_SHADER);
      mCameraShaderId = cameraShaderId;
    }

    const Texture* getTexture(int textureId) const {
      return mTextureManager->getTexture(textureId);
    }

    const Shader* getCameraShader() const {
      assert(mCompiled);
      return &mCameraShader;
    }

    void useEnvShader(int envShaderId) {
      assert(!mCompiled);
      assert(mShaderManager->getShader(envShaderId)->getClass()->getType() == ENV_SHADER);
      mEnvShaderId = envShaderId;
    }

    const Shader* getEnvShader() const {
      assert(mCompiled);
      return &mEnvShader;
    }

    void useLightShader(int lightShaderId) {
      assert(!mCompiled);
      assert(mShaderManager->getShader(lightShaderId)->getClass()->getType() == LIGHT_SHADER);
      assert(!hasLightShader(lightShaderId));
      
      mLightShaderParamsMemoryNeeded += 
        mShaderManager->getShader(lightShaderId)->getClass()->getUniformParamSize();
      mLightShaderIds.push_back(lightShaderId);
    }

    bool hasLightShader(int lightShaderId) {
      return 
        std::find(mLightShaderIds.begin(), mLightShaderIds.end(), lightShaderId) != mLightShaderIds.end();
    }

    const Shader* getLightShaderByIndex(int index) const {
      assert(mCompiled);
      return &mLightShaders[index];
    }

    int getLightShaderCount() const {
      return mLightShaders.size();
    }

    void replaceShader(int oldShaderId, int newShaderId) {
      assert(!mCompiled);
      mShaderRenamings[oldShaderId] = newShaderId;
    }

    void compileGeometry() {
      mScene->compile();
    }

    void compile() {
      assert(mCameraShaderId != SMART_INVALID_ID && mEnvShaderId != SMART_INVALID_ID);

      if(mCompiled)
        return;

      /* First check whether underlying CoreScene is compiled. */
      compileGeometry();

      /* Prepare memory arena. */
      int mShaderParamsMemoryNeeded = 
        mLightShaderParamsMemoryNeeded + 
        mShaderManager->getShader(mEnvShaderId)->getClass()->getUniformParamSize() +
        mShaderManager->getShader(mCameraShaderId)->getClass()->getUniformParamSize();
      mShadingParamArena.reserve(mShaderParamsMemoryNeeded);
      mShadingParamArena.setNextBlockCapacity(mShaderParamsMemoryNeeded / 4 + 1);

      /* Prepare camera shading data,... */
      mShaderManager->compileShader(&mCameraShader, mCameraShaderId, mShaderRenamings, mShadingParamArena);

      /* ...environmental shading data,... */
      mShaderManager->compileShader(&mEnvShader, mEnvShaderId, mShaderRenamings, mShadingParamArena);

      /* ...and light shading data. */
      assert(mLightShaders.size() == 0);
      mLightShaders.reserve(mLightShaderIds.size());
      for(int i = 0; i < mLightShaderIds.size(); i++) {
        mLightShaders.push_back(Shader());
        mShaderManager->compileShader(&mLightShaders.back(), mLightShaderIds[i], mShaderRenamings, mShadingParamArena);
      }
      
      /* We don't need renaming map anymore. */
      mShaderRenamings.clear();

      /* We're done. */
      mCompiled = true;
    }

    void decompile() {
      mCompiled = false;
      mLightShaders.clear();
      mShadingParamArena.clear();

      for(int i = 0; i < mScene->getObjectCount(); i++)
        mScene->getObject(i)->getModel()->decompile();
    }

    ~ShadedScene() {
      mScene->releaseOwnership();
    }

  private:
    friend class SmartCore;
    friend class RenderTask;

    ShadedScene(CoreScene* scene, const ShaderManager* shaderManager, const TextureManager* textureManager) {
      initialize(scene, shaderManager, textureManager);
    }

    void initialize(CoreScene* scene, const ShaderManager* shaderManager, const TextureManager* textureManager) {
      ExplicitlyCounted::initialize(this);
      mScene = scene;
      mScene->claimOwnership();
      mShaderManager = shaderManager;
      mTextureManager = textureManager;
      mCameraShaderId = SMART_INVALID_ID;
      mEnvShaderId = SMART_INVALID_ID;
      mLightShaderParamsMemoryNeeded = 0;
      mCompiled = false;
    }

    /** Identifier of a camera shader used. */ 
    int mCameraShaderId;

    /** Camera shader instance used during rendering. */
    Shader mCameraShader;

    /** Identifier of an environmental shader used. */
    int mEnvShaderId;

    /** Environmental shader instance used during rendering. */
    Shader mEnvShader;

    /** Array of light shader identifiers of this scene. */
    arx::CheckedArray<int> mLightShaderIds;

    /** Array of shader instances used during rendering. */
    arx::FastArray<Shader> mLightShaders;

    /** Map of shader replacements. */
    std::map<int, int> mShaderRenamings;

    /** Arena for storing shading parameters. */
    MemoryArena<> mShadingParamArena;

    /** Amount of memory needed for shading parameters. */
    int mLightShaderParamsMemoryNeeded;

    /** Associated ShaderManager object. */
    const ShaderManager* mShaderManager;

    /** Associated TextureManager object. */
    const TextureManager* mTextureManager;

    /** Is this ShadedScene compiled? */
    bool mCompiled;

    /** Associated CoreScene. */
    CoreScene* mScene;
  };

} // namespace smart

#endif // __SMART_SHADEDSCENE_H__
