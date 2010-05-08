#ifndef __SMART_SMARTCORE_H__
#define __SMART_SMARTCORE_H__

#include "common.h"
#include <arx/Collections.h>
#include <arx/Utility.h>
#include "IdMap.h"
#include "CoreModel.h"
#include "ShadedModel.h"
#include "CoreScene.h"
#include "ShadedScene.h"
#include "ShaderManager.h"
#include "RenderManager.h"
#include "Texture.h"

#include "ShaderImpl.h"

namespace smart {
// -------------------------------------------------------------------------- //
// SmartCore
// -------------------------------------------------------------------------- //
  class SmartCore: public arx::noncopyable {
  public:
    SmartCore() {
      /* Initialize destroyers. */
      mCoreModelDestroyer.initialize(this);
      mCoreSceneDestroyer.initialize(this);
      mShadedModelDestroyer.initialize(this);
      mShadedSceneDestroyer.initialize(this);

      /* Add local renderers. */
      unsigned int hardwareConcurrency = std::max(1u, arx::thread::hardware_concurrency());
      for(unsigned int i = 0; i < /*hardwareConcurrency*/ 2; i++) /* TODO */
        mRenderManager.addRenderer(LocalRenderHandler());
      /*for(unsigned int i = 0; i < 1; i++)
        mRenderManager.addRenderer(LocalRenderHandler()); TODO: this deadlocks. */
    }

    ~SmartCore() {
      // TODO
    }

    /** Creates a new ShadedScene, saving it in internal structures.
     * 
     * @returns a newly created scene. */
    ShadedScene* newScene() {
      /* First create CoreScene. */
      CoreScene* coreScene = new CoreScene();
      coreScene->setId(mCoreScenes.put(coreScene));
      coreScene->setDestroyer(&mCoreSceneDestroyer);

      /* Then create ShadedScene. */
      ShadedScene* shadedScene = new ShadedScene(coreScene, &mShaderManager, &mTextureManager);
      shadedScene->setId(mShadedScenes.put(shadedScene));
      shadedScene->setDestroyer(&mShadedSceneDestroyer);

      /* Now we can release ownership CoreScene, since it is now owned by 
       * ShadedScene */
      coreScene->releaseOwnership();

      /* We're done */
      return shadedScene;
    }

    /** Creates a new ShadedModel, saving it in internal structures.
     * 
     * @returns a newly created model. */
    ShadedModel* newModel() {
      /* First create a CoreModel. */
      CoreModel* coreModel = new CoreModel();
      coreModel->setId(mCoreModels.put(coreModel));
      coreModel->setDestroyer(&mCoreModelDestroyer);
      
      /* Then create a ShadedModel. */
      ShadedModel* shadedModel = new ShadedModel(coreModel, &mShaderManager);
      shadedModel->setId(mShadedModels.put(shadedModel));
      shadedModel->setDestroyer(&mShadedModelDestroyer);

      /* Now CoreModel is owned by ShadedModel, we don't need it anymore. */
      coreModel->releaseOwnership();

      /* We're done */
      return shadedModel;
    }

    bool hasModel(int modelId) const {
      return mShadedModels.contains(modelId);
    }

    ShadedModel* getModel(int modelId) const {
      assert(hasModel(modelId));
      return mShadedModels[modelId];
    }

    /** Releases the ownership of the given scene. The memory associated with it
     * will be freed as soon as all the objects using it are destroyed.
     *
     * @param scene scene to release. */
    void releaseScene(ShadedScene* scene) {
      mShadedScenes.remove(scene->getId());
      scene->releaseOwnership();
    }

    /** Releases the ownership of the given model. The memory associated with it
     * will be freed as soon as all the objects using it are destroyed.
     *
     * @param model model to free. */
    void releaseModel(ShadedModel* model) {
      mShadedModels.remove(model->getId());
      model->releaseOwnership();
    }

    Texture* newTexture(arx::Image3f image) {
      return mTextureManager.newTexture(image);
    }

    bool hasTexture(int textureId) const {
      return mTextureManager.hasTexture(textureId);
    }

    Texture* getTexture(int textureId) const {
      return mTextureManager.getTexture(textureId);
    }

    void releaseTexture(Texture* texture) {
      mTextureManager.releaseTexture(texture);
    }
    
    template<class T>
    ShaderClass* newShaderClass() {
      return mShaderManager.newShaderClass<T>();
    }

    /** Registers a new shader class and returns a newly created corresponding
     * ShaderClass object.
     * 
     * @param T shader type to create ShaderClass object from.
     * @param type type of a shader (e.g. SURFACE_SHADER, LIGHT_SHADER, etc...).
     * @returns newly created ShaderClass instance. */
    template<class T>
    ShaderClass* newShaderClass(ShaderClassType type) {
      return mShaderManager.newShaderClass<T>(type);
    }

    ShaderClass* getShaderClass(int shaderClassId) const {
      return mShaderManager.getShaderClass(shaderClassId);
    }

    bool hasShaderClass(int shaderClassId) const {
      return mShaderManager.hasShaderClass(shaderClassId);
    }

    /** Creates a new shader of the given ShaderClass.
     * 
     * @param shaderClass class of a shader to create.
     * @returns identifier of a newly created shader. */
    int newShader(ShaderClass* shaderClass) {
      return mShaderManager.newShader(shaderClass);
    }

    Shader* getShader(int shaderId) const {
      return mShaderManager.getShader(shaderId);
    }

    bool hasShader(int shaderId) const {
      return mShaderManager.hasShader(shaderId);
    }

    /** Releases a previously created shader with the given identifier.
     * All uncompiled models which use this shader will fail to compile.
     *
     * @param shaderId identifier of a shader to release. */
    void releaseShader(int shaderId) {
      mShaderManager.releaseShader(shaderId);
    }

    ShaderParam* getShaderParam(ShaderClass* shaderClass, const char* name) const {
      return mShaderManager.getShaderParam(shaderClass, name);
    }

    ShaderParam* getShaderParam(int shaderParamId) const {
      return mShaderManager.getShaderParam(shaderParamId);
    }

    bool hasShaderParam(ShaderClass* shaderClass, const char* name) const {
      return mShaderManager.hasShaderParam(shaderClass, name);
    }

    bool hasShaderParam(int shaderParamId) const {
      return mShaderManager.hasShaderParam(shaderParamId);
    }

    template<class T>
    void setShaderParam(ShaderParam* shaderParam, void* location, T value) const {
      mShaderManager.setShaderParam(shaderParam, location, value);
    }

    void setShaderParam(ShaderParam* shaderParam, void* location, void* value, int size) {
      mShaderManager.setShaderParam(shaderParam, location, value, size);
    }

    /** Starts rendering of the given scene. */
    template<class Tiler>
    RenderTask* startRendering(ShadedScene* scene, arx::Image3f& target, Tiler tiler) {
      /* Make sure it's compiled. */
      scene->compile();

      /* Create task and add it to render manager. */
      RenderTask* task = new RenderTask(scene, target, tiler);
      mRenderManager.addRenderTask(task);

      return task;
    }

    /** Starts rendering of the given scene. 
     *
     * Just a convenient overload. */
    RenderTask* startRendering(ShadedScene* scene, arx::Image3f& target) {
      return startRendering(scene, target, LinearTiler(SMART_DEFAULT_TILE_SIZE));
    }

    /** Waits for the completion of rendering of the given scene.
     * Must be called after a call to startRendering. */
    void endRendering(RenderTask* task) {
      task->endRendering();
      delete task;
    }

  private:
    friend class SmartAccessor;

    template<class T>
    class SmartDestroyer: public Destroyer<T> {
    public:
      SmartDestroyer(SmartCore* smart): mSmart(smart) {}

      SmartDestroyer(): mSmart(NULL) {}

      void initialize(SmartCore* smart) {
        assert(mSmart == NULL);
        mSmart = smart;
      }

      virtual void operator()(T* ptr) {
        mSmart->destroy(ptr);
      }

    private:
      SmartCore* mSmart;
    };

    void destroy(CoreModel* model) { mCoreModels.remove(model->getId()); delete model; }
    void destroy(CoreScene* scene) { mCoreScenes.remove(scene->getId()); delete scene; }
    void destroy(ShadedModel* model) { /*mShadedModels.remove(model->getId());*/ delete model; }
    void destroy(ShadedScene* scene) { /*mShadedScenes.remove(scene->getId());*/ delete scene; }

    /* TODO: use small object allocator in idmaps here */

    /* Bunch of destroyers for owned objects. */
    SmartDestroyer<CoreScene> mCoreSceneDestroyer;
    SmartDestroyer<CoreModel> mCoreModelDestroyer;
    SmartDestroyer<ShadedModel> mShadedModelDestroyer;
    SmartDestroyer<ShadedScene> mShadedSceneDestroyer;

    /** Array of CoreScene objects, indexed by id. */ 
    IdMap<CoreScene*> mCoreScenes;

    /** Array of CoreModel objects, indexed by id. */ 
    IdMap<CoreModel*> mCoreModels;

    /** Array of ShadedModel objects, indexed by id. */ 
    IdMap<ShadedModel*> mShadedModels;

    /** Array of ShadedScene objects, indexed by id. */ 
    IdMap<ShadedScene*> mShadedScenes;

    /** Texture manager object. */
    TextureManager mTextureManager;

    /** Shader manager object. */
    ShaderManager mShaderManager;

    /** Render manager object. */
    RenderManager mRenderManager;
  };

} // namespace smart

#endif // __SMART_SMARTCORE_H__
