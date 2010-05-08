#ifndef __SMART_SHADERREGISTRATOR_H__
#define __SMART_SHADERREGISTRATOR_H__

#include "common.h"
#include <arx/Utility.h>
#include "ShaderParam.h"

namespace smart {
  class ShaderManager;
  class ShaderClass;

// -------------------------------------------------------------------------- //
// Some forwarding magic...
// -------------------------------------------------------------------------- //
  namespace detail {
    template<class T>
    inline void registerParam(ShaderManager* shaderManager, ShaderClass* shaderClass, 
      const char* name, ShaderParamType type, int offset);
  } // namespace detail


// -------------------------------------------------------------------------- //
// ShaderRegistrator
// -------------------------------------------------------------------------- //
  class ShaderRegistrator: public arx::noncopyable {
  public:
    template<class T>
    void registerParam(const char* name, ShaderParamType type, int offset) const {
      detail::registerParam<T>(mManager, mShaderClass, name, type, offset);
    }

  private:
    ShaderRegistrator(ShaderManager* shaderManager, ShaderClass* shaderClass):
      mManager(shaderManager), mShaderClass(shaderClass) {}

    friend class ShaderManager;

    ShaderManager* mManager;
    ShaderClass* mShaderClass;
  };

} // namespace smart

#endif // __SMART_SHADERREGISTRATOR_H__
