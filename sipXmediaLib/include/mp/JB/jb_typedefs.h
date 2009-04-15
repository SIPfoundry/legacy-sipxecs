#ifndef _JB_TYPEDEFS_H_
#define _JB_TYPEDEFS_H_

#ifdef HAVE_GIPS /* [ */

#include "mp/GIPS/gips_typedefs.h"

#else /* HAVE_GIPS ] [ */

class MpJitterBuffer;
typedef MpJitterBuffer JB_inst;

typedef char JB_char;
typedef int JB_code;
typedef int JB_size;
typedef int JB_ret;

typedef unsigned char JB_uchar;
typedef unsigned long JB_ulong;

#endif /* HAVE_GIPS ] */

#endif /* _JB_TYPEDEFS_H_ */
