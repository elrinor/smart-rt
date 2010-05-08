#ifndef __SMART_UTILITY_H__
#define __SMART_UTILITY_H__

#include "common.h"
#include <cassert>
#include <arx/Preprocessor.h>

#define SMART_DEFINE_HAS_MEMBER(MEMBER_NAME, MEMBER_TYPE, MEMBER_T_ACCESS)      \
  template<class U> struct ARX_JOIN(has_, MEMBER_NAME) {                        \
    typedef char true_type;                                                     \
    struct false_type { true_type dummy[2]; };                                  \
    template<MEMBER_TYPE> struct nothing {};                                    \
    template<class T>                                                           \
    static true_type has_member(T*, nothing<MEMBER_T_ACCESS>* = 0);             \
    static false_type has_member(void*);                                        \
    enum {                                                                      \
      value = sizeof(                                                           \
        ARX_JOIN(has_, MEMBER_NAME)<U>::has_member(static_cast<U*>(NULL))       \
      ) == sizeof(true_type)                                                    \
    };                                                                          \
    typedef arx::bool_<value> type;                                             \
  };

namespace smart {
  /** Transforms the given vector with the given transformation matrix. */
  inline Vector3f transform(const Vector4f& v, const Matrix4f& m) {
    /* Start reciprocal calculation first. */
    float rec = 1.0f / v.dot(m.row(3));

    /* Manually calculate product. */
    return Vector3f(v.dot(m.row(0)), v.dot(m.row(1)), v.dot(m.row(2))) * rec;
  }

  /** Transforms the given vector with the given transformation matrix. */
  FORCEINLINE Vector3f transform(const Vector3f& v, const Matrix4f& m) {
    return transform(Vector4f(v[0], v[1], v[2], 1.0f), m);
  }

  /** Accelerated modulo-3 division for integers in range [0, 4]. */
  FORCEINLINE int fastModulo3(int value) {
    assert(value >= 0 && value <= 5);

    ALIGN(SMART_CACHELINE) static const unsigned int sModulo3[8] = 
      {0, 1, 2, 0, 1, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};
    return sModulo3[value];
  }

  /** FakeArray class provides an array-like interface to identity function object. */
  template<class T>
  class FakeArray {
  public:
    typedef T size_type;
    typedef T value_type;

    FakeArray(size_type size): mSize(size) {}

    value_type operator[](size_type index) const {
      assert(index >= 0 && index < mSize);
      return index;
    }

    size_type size() const {
      return mSize;
    }

  private:
    size_type mSize;
  };

} // namespace smart

#endif // __SMART_UTILITY_H__