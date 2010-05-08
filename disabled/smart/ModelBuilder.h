#ifndef __SMART_MODELBUILDER_H__
#define __SMART_MODELBUILDER_H__

#include "common.h"
#include "SmartAccessor.h"
#include "Source.h"
#include "Builder.h"

namespace smart {
  /* Source class trick for Builder. */
  SMART_DEFINE_SOURCE_CLASS(ModelBuilderSource, ModelBuilder, Model, arx::shared_ptr<CoreModelBuilder>)

// -------------------------------------------------------------------------- //
// ModelBuilder
// -------------------------------------------------------------------------- //
  class ModelBuilder {
  public:
    SMART_CONSTRUCTOR_FROM_SOURCE(ModelBuilder, ModelBuilderSource, mBuilder) {}

    template<BuilderMode mode> 
    Builder<mode> begin() {
      coreModel().startPrimitive();
      return BuilderSource<mode>(mSmart, mId);
    }

    Builder<> begin(int mode) {
      coreModel().startPrimitive();
      return BuilderSource<SMART_DYNAMIC>(mSmart, mId, mode);
    }

    void end() {
      coreModel().endEditing();
    }

  private:
    arx::shared_ptr<CoreModelBuilder> mBuilder;
  };

} // namespace smart

#endif // __SMART_MODELBUILDER_H__
