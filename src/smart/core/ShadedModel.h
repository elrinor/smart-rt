#ifndef __SMART_SHADEDMODEL_H__
#define __SMART_SHADEDMODEL_H__

#include "common.h"
#include <map>
#include <arx/Utility.h>
#include "ExplicitlyCounted.h"
#include "Idded.h"
#include "CoreModel.h"
#include "ShaderManager.h"
#include "MemoryArena.h"

namespace smart {
// -------------------------------------------------------------------------- //
// ShadedModel
// -------------------------------------------------------------------------- //
  class ShadedModel: private arx::noncopyable, private ExplicitlyCounted<ShadedModel>, public Idded {
  public:
    typedef CoreModel::TriangleType TriangleType;

    const BspTree& getBspTree() const {
      return mModel->getBspTree();
    }

    const BoundingBox& getBoundingBox() const {
      return mModel->getBoundingBox();
    }

    const int getTriangleCount() const {
      return mModel->getTriangleCount();
    }

    const TriAccel& getTriAccel(int id) const {
      return mModel->getTriAccel(id);
    }

    const TriangleType getTriangle(int id) const {
      return mModel->getTriangle(id);
    }

    const Vector3f& getCoord(int vertexId) const {
      return mModel->getCoord(vertexId);
    }

    const Vector3f& getNormal(int vertexId) const {
      return mModel->getNormal(vertexId);
    }

    const Vector3f& getTexCoord(int vertexId) const {
      return mModel->getTexCoord(vertexId);
    }

    const Vector3f& getCoord(int triangleId, int n) const {
      return mModel->getCoord(triangleId, n);
    }

    const Vector3f& getNormal(int triangleId, int n) const {
      return mModel->getNormal(triangleId, n);
    }

    const Vector3f& getTexCoord(int triangleId, int n) const {
      return mModel->getTexCoord(triangleId, n);
    }

    const Shader* getTriangleShader(int id) const {
      assert(mCompiled);
      return &mSurfaceShaders[id];
    }

    void* newShaderTriangleParam(SurfaceShaderClass* shaderClass) {
      return mModel->newShaderTriangleParam(shaderClass);
    }

    void* newShaderAttribParam(SurfaceShaderClass* shaderClass) {
      return mModel->newShaderAttribParam(shaderClass);
    }

    int newTriangle(int vertexId0, int vertexId1, int vertexId2, int shaderId, void* shadingParam) {
      assert(mShaderManager->getShader(shaderId)->getClass()->getType() == SURFACE_SHADER);
      mSurfaceShaderIds.push_back(shaderId);
      mShaderParamsMemoryNeeded += 
        mShaderManager->getShader(shaderId)->getClass()->getUniformParamSize();
      return mModel->newTriangle(vertexId0, vertexId1, vertexId2, shadingParam);
    }

    int newVertex(const Vector3f& coord, const Vector3f& normal, const Vector3f& texCoord, void* shadingParam) {
      return mModel->newVertex(coord, normal, texCoord, shadingParam);
    }

    void replaceShader(int oldShaderId, int newShaderId) {
      assert(!mCompiled);
      mShaderRenamings[oldShaderId] = newShaderId;
    }

    void compileGeometry() {
      mModel->compile();
    }

    void compile() {
      if(mCompiled)
        return;

      /* First check whether underlying CoreModel is compiled. */
      compileGeometry();

      /* Prepare memory arena. */
      mShadingParamArena.reserve(mShaderParamsMemoryNeeded);
      mShadingParamArena.setNextBlockCapacity(mShaderParamsMemoryNeeded / 4 + 1);
  
      /* Then prepare shading instance array. */
      assert(mSurfaceShaders.size() == 0);
      mSurfaceShaders.reserve(mSurfaceShaderIds.size());
      for(int i = 0; i < mSurfaceShaderIds.size(); i++) {
        mSurfaceShaders.push_back(Shader());
        mShaderManager->compileShader(&mSurfaceShaders.back(), mSurfaceShaderIds[i], mShaderRenamings, mShadingParamArena);
      }

      /* We don't need renaming map anymore. */
      mShaderRenamings.clear();

      /* We're done. */
      mCompiled = true;
    }

    void decompile() {
      mCompiled = false;
      mSurfaceShaders.clear();
      mShadingParamArena.clear();
    }

    bool isCompiled() const {
      return mCompiled;
    }

    ~ShadedModel() {
      mModel->releaseOwnership();
    }

  private:
    friend class SmartCore;
    friend class CoreObject;

    ShadedModel(CoreModel* model, const ShaderManager* shaderManager) {
      initialize(model, shaderManager, 16);
    }

    ShadedModel(CoreModel* model, const ShaderManager* shaderManager, int triangleCapacity) {
      initialize(model, shaderManager, triangleCapacity);
    }

    void initialize(CoreModel* model, const ShaderManager* shaderManager, int triangleCapacity) {
      ExplicitlyCounted::initialize(this);
      mShaderManager = shaderManager;
      mSurfaceShaderIds.reserve(triangleCapacity);
      mCompiled = false;
      mModel = model;
      mModel->claimOwnership();
      mShaderParamsMemoryNeeded = 0;
    }

    /** Array of shader instances used during rendering. */
    arx::FastArray<Shader> mSurfaceShaders;

    /** Array of shader identifiers bound to triangles. */
    arx::CheckedArray<int> mSurfaceShaderIds;

    /** Map of shader renamings. */
    std::map<int, int> mShaderRenamings;

    /** Associated ShaderManager object. */
    const ShaderManager* mShaderManager;

    /** Is this ShadedModel compiled? */
    bool mCompiled;

    /** Arena for storing shading parameters. */
    MemoryArena<> mShadingParamArena;

    /** Amount of memory needed for shading parameters. */
    int mShaderParamsMemoryNeeded;
    
    /** Associated CoreModel. */
    CoreModel* mModel;
  };

} // namespace smart

#endif // __SMART_SHADEDMODEL_H__
