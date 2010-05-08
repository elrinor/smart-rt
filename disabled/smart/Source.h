#ifndef __SMART_SOURCE_H__
#define __SMART_SOURCE_H__

#include "common.h"
#include <arx/Preprocessor.h>

namespace smart {
  namespace detail {
    template<class T>
    class SourceBase {
    protected:
      SourceBase(T source): mSource(source) {}
      T mSource;
    };
  }
}

#define SMART_DEFINE_SOURCE_CLASS_EXT(NAME_TEMPLATE, NAME, DEST_TEMPLATE, DEST_NAME, CREATOR_TEMPLATE, CREATOR_NAME, SOURCE_MEMBER_TYPE, DATA_MEMBER_TYPE) \
  DEST_TEMPLATE class DEST_NAME;                                                \
  CREATOR_TEMPLATE class CREATOR_NAME;                                          \
  NAME_TEMPLATE class NAME: private detail::SourceBase<SOURCE_MEMBER_TYPE> {    \
  private:                                                                      \
    DEST_TEMPLATE friend class DEST_NAME;                                       \
    CREATOR_TEMPLATE friend class CREATOR_NAME;                                 \
    NAME(SOURCE_MEMBER_TYPE source                                              \
      ARX_COMMA_IF(ARX_NOT(ARX_IS_EMPTY(DATA_MEMBER_TYPE ARX_EMPTY())))         \
      ARX_IF(ARX_IS_EMPTY(DATA_MEMBER_TYPE ARX_EMPTY()), ARX_EMPTY(), DATA_MEMBER_TYPE data) \
        ): detail::SourceBase<SOURCE_MEMBER_TYPE>(source)                       \
      ARX_COMMA_IF(ARX_NOT(ARX_IS_EMPTY(DATA_MEMBER_TYPE ARX_EMPTY())))         \
      ARX_IF(ARX_IS_EMPTY(DATA_MEMBER_TYPE ARX_EMPTY()), ARX_EMPTY(), mData(data)) \
        {}                                                                      \
    ARX_IF(ARX_IS_EMPTY(DATA_MEMBER_TYPE ARX_EMPTY()),                          \
      ARX_EMPTY(),                                                              \
      DATA_MEMBER_TYPE mData;                                                   \
    )                                                                           \
    const SOURCE_MEMBER_TYPE& getSource() const { return mSource; }             \
    ARX_IF(ARX_IS_EMPTY(DATA_MEMBER_TYPE ARX_EMPTY()),                          \
      ARX_EMPTY(),                                                              \
      const DATA_MEMBER_TYPE& getData() const { return mData; }                 \
    )                                                                           \
  };

#define SMART_DEFINE_SOURCE_CLASS(NAME, DEST_NAME, CREATOR_NAME, SOURCE_MEMBER_TYPE) \
  SMART_DEFINE_SOURCE_CLASS_EXT(ARX_EMPTY(), NAME, ARX_EMPTY(), DEST_NAME, ARX_EMPTY(), CREATOR_NAME, SOURCE_MEMBER_TYPE, ARX_EMPTY())

#define SMART_CONSTRUCTOR_FROM_SOURCE_EXT(THIS_TYPE, SOURCE_TYPE, SOURCE_MEMBER, DATA_MEMBER) \
  THIS_TYPE(const SOURCE_TYPE& src): SOURCE_MEMBER(src.getSource())             \
    ARX_COMMA_IF(ARX_NOT(ARX_IS_EMPTY(DATA_MEMBER ARX_EMPTY())))                \
    ARX_IF(ARX_IS_EMPTY(DATA_MEMBER ARX_EMPTY()), ARX_EMPTY(), DATA_MEMBER(src.getData()))

#define SMART_CONSTRUCTOR_FROM_SOURCE(THIS_TYPE, SOURCE_TYPE, SOURCE_MEMBER) \
  SMART_CONSTRUCTOR_FROM_SOURCE_EXT(THIS_TYPE, SOURCE_TYPE, SOURCE_MEMBER, ARX_EMPTY())


#endif // __SMART_SOURCE_H__
