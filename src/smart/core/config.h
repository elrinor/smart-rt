#ifndef __SMART_CONFIG_H__
#define __SMART_CONFIG_H__

#include <arx/config.h>

#define SMART_USE_SSE2

#ifndef SMART_CACHELINE
#  define SMART_CACHELINE 64
#endif

/** @def SMART_TRACETRIACCEL_EPS
 * Epsilon value used for point-in-segment containment test in ray-triangle
 * intersection routine. */
#ifndef SMART_TRACETRIACCEL_EPS
#  define SMART_TRACETRIACCEL_EPS 1.0e-6f
#endif

/** @def SMART_TRACEBSPNODE_SEGMENTCONTAINS_EPS
 * Epsilon value used for point-in-segment containment test in bsp tree
 * traversal routine. */
#ifndef SMART_TRACEBSPNODE_SEGMENTCONTAINS_EPS
#  define SMART_TRACEBSPNODE_SEGMENTCONTAINS_EPS 1.0e-6f
#endif

/** @def SMART_TRACEBSPNODE_DIRECTIONGEZERO_EPS
 * Epsilon value for greater-than-zero check in bsp tree traversal routine. */
#ifndef SMART_TRACEBSPNODE_DIRECTIONGEZERO_EPS
#  define SMART_TRACEBSPNODE_DIRECTIONGEZERO_EPS 1.0e-6f
#endif

/** @def SMART_TRACEUPPER_SEGMENTSTART_EPS
 * Initial minimal value of a ray segment.
 * Must be larger than SMART_TRACETRIACCEL_EPS to
 * prevent infinite recursion. */
#ifndef SMART_TRACEUPPER_SEGMENTSTART_EPS
#  define SMART_TRACEUPPER_SEGMENTSTART_EPS 1.0e-4f
#endif

/** @def SMART_BSPSAH_SAMEPOS_EPS
 * Epsilon value used for equality testing in BSP SAH construction routine.
 * TODO: less than SMART_TRACETRIACCEL_EPS ? */
#ifndef SMART_BSPSAH_SAMEPOS_EPS
#  define SMART_BSPSAH_SAMEPOS_EPS 1.0e-6f
#endif

/** @def SMART_CLIPTRIANGLEBOX_EPS
 * Epsilon value used in box-triangle clipping routine. */
#ifndef SMART_CLIPTRIANGLEBOX_EPS
#  define SMART_CLIPTRIANGLEBOX_EPS 1.0e-5f
#endif

/**
 * 
 */
#ifndef SMART_NOT_A_TRIANGLE_EPS
#  define SMART_NOT_A_TRIANGLE_EPS 1.0e-6
#endif

/** @def SMART_DEFAULT_TILE_SIZE
 * Default tile size used during tiled rendering. */
#ifndef SMART_DEFAULT_TILE_SIZE
#  define SMART_DEFAULT_TILE_SIZE 32
#endif

/** @def SMART_MAX_BSPTREE_DEPTH
 * Maximal depth of a BSP tree, i.e. maximal length of non-leaf node chain. */
#ifndef SMART_MAX_BSPTREE_DEPTH
#  define SMART_MAX_BSPTREE_DEPTH 64
#endif

/** @def SMART_USE_SSE
 * Use SSE intrinsics */

/** @def SMART_USE_SSE2
 * Use SSE2 intrinsics. */

/** @def SMART_USE_SSE3
 * Use SSE3 intrinsics. */

#if defined(SMART_USE_SSE3) && !defined(SMART_USE_SSE2)
#  define SMART_USE_SSE2
#endif

#if defined(SMART_USE_SSE2) && !defined(SMART_USE_SSE)
#  define SMART_USE_SSE
#endif

#endif