#ifndef __SMART_DISABLEMSVCVARNINGS_H__
#define __SMART_DISABLEMSVCVARNINGS_H__

#include <arx/config.h>                   /* For ARX_MSVC macro. */

/* Disable this M$ secure shit. */
#ifdef ARX_MSVC
#  pragma push_macro("_SCL_SECURE_NO_DEPRECATE")
#  define _SCL_SECURE_NO_DEPRECATE
#endif

#endif // __SMART_DISABLEMSVCVARNINGS_H__
