#ifndef __SMART_SMARTDEFS_H__
#define __SMART_SMARTDEFS_H__

#ifndef RTAPI
#  define RTAPI
#endif

#define RTAPIENTRY 

// -------------------------------------------------------------------------- //
// Typedefs
// -------------------------------------------------------------------------- //
typedef unsigned int    RTenum;
typedef unsigned char   RTboolean;
typedef unsigned int    RTbitfield;
typedef void            RTvoid;
typedef signed char     RTbyte;
typedef short           RTshort;
typedef int             RTint;
typedef unsigned char   RTubyte;
typedef unsigned short  RTushort;
typedef unsigned int    RTuint;
typedef int             RTsizei;
typedef float           RTfloat;
typedef float           RTclampf; /**< single precision float in [0,1] */
typedef double          RTdouble;
typedef double          RTclampd; /**< double precision float in [0,1] */
typedef unsigned long   RTulong;
typedef long            RTlong;

typedef RTboolean       RTbool;
typedef RTubyte         RTuchar;


// -------------------------------------------------------------------------- //
// Enums
// -------------------------------------------------------------------------- //
#define RT_TYPE_COMBINE(TYPE, VALUE) ((TYPE) | (VALUE))
#define RT_TYPE_OF(VALUE)            ((VALUE) & 0xFFFFFF00)

enum {
  RT_TYPE_GEOMETRY_MODE      = 0x00000100,
  RT_TYPE_NEWOBJECT_MODE     = 0x00000200,
  RT_TYPE_FB_FORMAT          = 0x00000300,
  RT_TYPE_SHADERTYPE         = 0x00000400, 
  RT_TYPE_DATATYPE           = 0x00001400,
  RT_TYPE_ERROR              = 0xFFFFFE00,
  RT_TYPE_INVALID            = 0xFFFFFF00
};

enum {
  RT_TRIANGLES               = RT_TYPE_COMBINE(RT_TYPE_GEOMETRY_MODE, 0x00),
  RT_TRIANGLE_STRIP          = RT_TYPE_COMBINE(RT_TYPE_GEOMETRY_MODE, 0x01),
  RT_TRIANGLE_FAN            = RT_TYPE_COMBINE(RT_TYPE_GEOMETRY_MODE, 0x02),
  RT_QUADS                   = RT_TYPE_COMBINE(RT_TYPE_GEOMETRY_MODE, 0x03),
  RT_QUAD_STRIP              = RT_TYPE_COMBINE(RT_TYPE_GEOMETRY_MODE, 0x04),
  RT_POLYGON                 = RT_TYPE_COMBINE(RT_TYPE_GEOMETRY_MODE, 0x05)
};

enum {
  RT_DEFINE                  = RT_TYPE_COMBINE(RT_TYPE_NEWOBJECT_MODE, 0x00),
  RT_DEFINE_AND_INSTANTIATE  = RT_TYPE_COMBINE(RT_TYPE_NEWOBJECT_MODE, 0x01),
  RT_COMPILE = RT_DEFINE
};

enum {
  RT_BYTE                    = RT_TYPE_COMBINE(RT_TYPE_DATATYPE, 0x00),
  RT_UNSIGNED_BYTE           = RT_TYPE_COMBINE(RT_TYPE_DATATYPE, 0x01),
  RT_SHORT                   = RT_TYPE_COMBINE(RT_TYPE_DATATYPE, 0x02),
  RT_UNSIGNED_SHORT          = RT_TYPE_COMBINE(RT_TYPE_DATATYPE, 0x03),
  RT_INT                     = RT_TYPE_COMBINE(RT_TYPE_DATATYPE, 0x04),
  RT_UNSIGNED_INT            = RT_TYPE_COMBINE(RT_TYPE_DATATYPE, 0x05),
  RT_FLOAT                   = RT_TYPE_COMBINE(RT_TYPE_DATATYPE, 0x06),
  RT_DOUBLE                  = RT_TYPE_COMBINE(RT_TYPE_DATATYPE, 0x0A),
  RT_2_BYTES                 = RT_TYPE_COMBINE(RT_TYPE_DATATYPE, 0x07),
  RT_3_BYTES                 = RT_TYPE_COMBINE(RT_TYPE_DATATYPE, 0x08),
  RT_4_BYTES                 = RT_TYPE_COMBINE(RT_TYPE_DATATYPE, 0x09)
};

enum {
  RT_INVALID                 = RT_TYPE_COMBINE(RT_TYPE_INVALID, 0xFF)
};

enum {
  RT_BGR_32                  = RT_TYPE_COMBINE(RT_TYPE_FB_FORMAT, 0x00)
};

enum {
  /** No error has been recorded. The value of this symbolic constant is 
   * guaranteed to be zero. */
  RT_NO_ERROR                = 0,
  
  /** An unacceptable value is specified for an enumerated argument. The 
   * offending function is ignored, having no side effect other than to set 
   * the error flag. */
  RT_INVALID_ENUM            = RT_TYPE_COMBINE(RT_TYPE_ERROR, 0x01),

  /** A numeric argument is out of range. The offending function is ignored, 
   * having no side effect other than to set the error flag. */
  RT_INVALID_VALUE           = RT_TYPE_COMBINE(RT_TYPE_ERROR, 0x02),

  /** The specified operation is not allowed in the current state. The 
   * offending function is ignored, having no side effect other than to set the 
   * error flag. */
  RT_INVALID_OPERATION       = RT_TYPE_COMBINE(RT_TYPE_ERROR, 0x03),

  /** This function would cause a stack overflow. The offending function is 
   * ignored, having no side effect other than to set the error flag. */
  RT_STACK_OVERFLOW          = RT_TYPE_COMBINE(RT_TYPE_ERROR, 0x04),

  /** This function would cause a stack underflow. The offending function is 
   * ignored, having no side effect other than to set the error flag. */
  RT_STACK_UNDERFLOW         = RT_TYPE_COMBINE(RT_TYPE_ERROR, 0x05),

  /** There is not enough memory left to execute the function. The state of 
   * SMART is undefined, except for the state of the error flags, after this 
   * error is recorded. */
  RT_OUT_OF_MEMORY           = RT_TYPE_COMBINE(RT_TYPE_ERROR, 0x06)
};

enum {
  RT_SURFACE_SHADER          = RT_TYPE_COMBINE(RT_TYPE_SHADERTYPE, 0x00),
  RT_ENV_SHADER              = RT_TYPE_COMBINE(RT_TYPE_SHADERTYPE, 0x01),
  RT_LIGHT_SHADER            = RT_TYPE_COMBINE(RT_TYPE_SHADERTYPE, 0x02),
  RT_CAMERA_SHADER           = RT_TYPE_COMBINE(RT_TYPE_SHADERTYPE, 0x03),
};

#endif // __SMART_SMARTDEFS_H__
