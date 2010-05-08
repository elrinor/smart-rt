#ifndef __SMART_SHADERPARAM_H__
#define __SMART_SHADERPARAM_H__

#include "common.h"
#include <arx/Utility.h>
#include "Idded.h"

namespace smart {
  class ShaderClass;

  enum ShaderParamType {
    PER_SHADER,
    PER_VERTEX,
    PER_TRIANGLE
  };

// -------------------------------------------------------------------------- //
// ShaderParam
// -------------------------------------------------------------------------- //
  class ShaderParam: public arx::noncopyable, public Idded {
  public:
    ShaderClass* getClass() const {
      return mShaderClass;
    }

    const char* getName() const {
      return mName;
    }

    int getType() const {
      return mType;
    }

    int getOffset() const {
      return mOffset;
    }

    int getSize() const {
      return mSize;
    }

    template<class T> 
    bool isOfType() {
      static const char* name = typeid(T).name();
      return strcmp(name, mValueTypeName) == 0;
    }

  private:
    template<class T>
    ShaderParam(ShaderClass* shaderClass, const char* name, ShaderParamType type, int offset, arx::identity<T> /* impl */) {
      assert(offset >= 0);
      assert(type == PER_SHADER || shaderClass->getType() == SURFACE_SHADER);

      mShaderClass = shaderClass;
      mName = name;
      mValueTypeName = typeid(T).name();
      mType = type;
      mOffset = offset;
      mSize = sizeof(T);
    }

    friend class ShaderManager;

    ShaderClass* mShaderClass;
    const char* mName;
    const char* mValueTypeName;
    ShaderParamType mType;
    int mOffset;
    int mSize;
  };


} // namespace smart

#endif // __SMART_SHADERPARAM_H__
