#ifndef __SMART_CORE_COMMON_H__
#define __SMART_CORE_COMMON_H__

#include "config.h"

/* We don't need these nasty Windows.h min/max macros. */
#ifdef ARX_WIN32
#  define NOMINMAX
#endif

#ifdef SMART_USE_SSE
#  include <xmmintrin.h>
#endif

#ifdef SMART_USE_SSE2
#  include <emmintrin.h>
#endif

#ifdef SMART_USE_SSE3
#  include <pmmintrin.h>
#endif

#define Unreachable() assert(!"Unreachable")

#include <arx/LinearAlgebra.h>
/*
#ifdef DEBUG
#  include <nvwa/debug_new.h>
#endif
*/
namespace smart {
  using arx::Vector2f;
  using arx::Vector3f;
  using arx::Vector4f;
  using arx::Matrix4f;

  using arx::Vector3i;

  enum {
    SMART_INVALID_ID = -1
  };
} // namespace smart

#endif // __SMART_CORE_COMMON_H__
