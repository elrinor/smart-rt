#ifndef __SMART_SHADERMANAGER_H__
#define __SMART_SHADERMANAGER_H__

#include "common.h"
#include <cassert>
#include <arx/Utility.h>
#include "ShaderClass.h"
#include "ShaderParam.h"
#include "Shader.h"
#include "IdMap.h"

namespace smart {
  /* TODO: think about replacing new with some allocator...
   * idea: allocations and deallocations will come in bunch! */
// -------------------------------------------------------------------------- //
// ShaderManager
// -------------------------------------------------------------------------- //
  class ShaderManager: private arx::noncopyable {
  public:
    ShaderManager() {}

    ~ShaderManager() {
      for(int i = 0; i < mShaderClasses.size(); i++)
        delete mShaderClasses[i];
      for(IdMap<Shader*>::iterator i = mShaders.begin(); i != mShaders.end(); i++) {
        arx::aligned_free(i->second->getUniformParam());
        delete i->second;
      }
    }

    template<class T>
    ShaderClass* newShaderClass() {
      if(ShaderClass::has_transparency<T>::value)
        return newShaderClass<T>(SURFACE_SHADER);
      else if(ShaderClass::has_shade<T>::value)
        return newShaderClass<T>(ENV_SHADER);
      else if(ShaderClass::has_illuminate<T>::value)
        return newShaderClass<T>(LIGHT_SHADER);
      else 
        return newShaderClass<T>(CAMERA_SHADER);
    }

    template<class T>
    ShaderClass* newShaderClass(ShaderClassType type) {
      switch(type) {
      case SURFACE_SHADER:
        assert(ShaderClass::has_shade<T>::value && ShaderClass::has_transparency<T>::value);
        return newShaderClassUnchecked<T, SurfaceShaderClass>();
      case ENV_SHADER:
        assert(ShaderClass::has_shade<T>::value);
        return newShaderClassUnchecked<T, EnvShaderClass>();
      case LIGHT_SHADER:
        assert(ShaderClass::has_illuminate<T>::value);
        return newShaderClassUnchecked<T, LightShaderClass>();
      case CAMERA_SHADER:
        assert(ShaderClass::has_initPrimaryRay<T>::value);
        return newShaderClassUnchecked<T, CameraShaderClass>();
      default:
        Unreachable();
        return NULL; /* Just to make compiler happy. */
      }
    }

    ShaderClass* getShaderClass(int shaderClassId) const {
      assert(hasShaderClass(shaderClassId));
      return mShaderClasses[shaderClassId];
    }

    bool hasShaderClass(int shaderClassId) const {
      return 0 <= shaderClassId && shaderClassId < mShaderClasses.size();
    }

    /** Creates a new shader parameter for the given shader class. */
    template<class T>
    ShaderParam* newShaderParam(ShaderClass* shaderClass, const char* name, ShaderParamType type, int offset) {
      ShaderParam* shaderParam = 
        new (mArena.allocate<ShaderParam>(1)) ShaderParam(shaderClass, name, type, offset, arx::identity<T>());
      
      mShaderParams.push_back(shaderParam);
      shaderParam->setId(mShaderParams.size() - 1);

      mShaderParamMap.insert(std::make_pair(std::make_pair(shaderClass, name), shaderParam));
      return shaderParam;
    }

    ShaderParam* getShaderParam(ShaderClass* shaderClass, const char* name) const {
      assert(hasShaderParam(shaderClass, name));
      return mShaderParamMap.find(std::make_pair(shaderClass, name))->second;
    }

    ShaderParam* getShaderParam(int shaderParamId) const {
      assert(hasShaderParam(shaderParamId));
      return mShaderParams[shaderParamId];
    }

    bool hasShaderParam(ShaderClass* shaderClass, const char* name) const {
      return mShaderParamMap.find(std::make_pair(shaderClass, name)) != mShaderParamMap.end();
    }

    bool hasShaderParam(int shaderParamId) const {
      return 0 <= shaderParamId && shaderParamId < mShaderParams.size();
    }

    /** Creates a new shader object. It is guaranteed that succeeding calls to 
     * newShader will return succeeding integer identifiers.
     * 
     * @param shaderClass class of a shader to add.
     * @returns identifier of a newly created shader. */
    int newShader(ShaderClass* shaderClass) {
      return mShaders.put(new Shader(
        shaderClass, 
        arx::aligned_malloc(shaderClass->getUniformParamSize(), shaderClass->getUniformParamAling())
      ));
    }

    Shader* getShader(int shaderId) const {
      assert(hasShader(shaderId));
      return mShaders[shaderId];
    }

    bool hasShader(int shaderId) const {
      return mShaders.contains(shaderId);
    }

    void releaseShader(int shaderId) {
      assert(mShaders.contains(shaderId));
      arx::aligned_free(mShaders[shaderId]->getUniformParam());
      delete mShaders[shaderId];
      mShaders.remove(shaderId);
    }

    template<class T>
    void setShaderParam(ShaderParam* shaderParam, void* location, T value) const {
      assert(shaderParam->isOfType<T>());
      *reinterpret_cast<T*>(static_cast<char*>(location) + shaderParam->getOffset()) = value;
    }

    void setShaderParam(ShaderParam* shaderParam, void* location, void* value, int size) {
      assert(shaderParam->getSize() == size); /* TODO check this in run-time */
      memcpy(static_cast<char*>(location) + shaderParam->getOffset(), value, size);
    }

    /** Compiles the ShaderInstance structure for the given shader identifier 
     * by copying uniform shading parameters to a newly allocated memory block.
     * For allocation, given memory arena is used.
     *
     * @param instance ShaderInstance structure to compile.
     * @param shaderId shader identifier, might be changed because of renaming.
     * @param renamings map of shader renamings (id to id mapping).
     * @param shadingParamArena memory arena to allocate shading uniform paramenters in. */
    template<class MapType, class MemoryArenaType>
    void compileShader(Shader* instance, int& shaderId, const MapType& renamings, MemoryArenaType& shadingParamArena) const {
      /* Apply renaming first. */
      MapType::const_iterator pos = renamings.find(shaderId);
      if(pos != renamings.end())
        shaderId = pos->second;

      /* Get original shader class and shading param. */
      ShaderClass* shaderClass = getShader(shaderId)->getClass();
      unsigned char* uniformShadingParam = 
        static_cast<unsigned char*>(mShaders[shaderId]->getUniformParam());

      /* Allocate memory for copy of shading param. */
      unsigned char* uniformShadingParamCopy = 
        shadingParamArena.allocate(shaderClass->getUniformParamSize(), shaderClass->getUniformParamAling());

      /* Copy shading param. */
      std::copy(uniformShadingParam, uniformShadingParam + shaderClass->getUniformParamSize(), uniformShadingParamCopy);

      /* Fill ShaderInstance structure. */
      instance->initialize(shaderClass, uniformShadingParamCopy);
    }

  private:
    template<class T, class ClassType>
    ShaderClass* newShaderClassUnchecked() {
      ShaderClass* shaderClass = 
        new (mArena.allocate<ClassType>(1)) ClassType(arx::identity<T>());
      mShaderClasses.push_back(shaderClass);
      shaderClass->setId(mShaderClasses.size() - 1);
      reinterpret_cast<T*>(NULL)->registerParams(ShaderRegistrator(this, shaderClass));
      return shaderClass;
    }

    /** Array of shader classes, indexed by id. Shader class cannot be
     * deallocated, therefore it's better to use an array, not a map here. */
    arx::CheckedArray<ShaderClass*> mShaderClasses;

    /** Array of shader instances, indexed by id. */ 
    IdMap<Shader*> mShaders;

    /** Array of shader parameter descriptors, indexed by id. */
    arx::CheckedArray<ShaderParam*> mShaderParams;

    /** Mapping for fast shader parameter access. */
    std::map<std::pair<ShaderClass*, const char*>, ShaderParam*> mShaderParamMap;

    /** Memory arena for ShaderClass and ShaderParam objects. */
    MemoryArena<> mArena;
  };


// -------------------------------------------------------------------------- //
// Registration cyclic dependency resolved...
// -------------------------------------------------------------------------- //
  namespace detail {
    template<class T>
    inline void registerParam(ShaderManager* shaderManager, ShaderClass* shaderClass, 
      const char* name, ShaderParamType type, int offset) {
      shaderManager->newShaderParam<T>(shaderClass, name, type, offset);
    }
  } // namespace detail


} // namespace smart

#endif // __SMART_SHADERMANAGER_H__
