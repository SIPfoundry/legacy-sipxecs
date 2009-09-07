//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _UtlDefs_h_
#define _UtlDefs_h_

// SYSTEM INCLUDES
#include <os/qsTypes.h>
#include <os/OsDefs.h>

// APPLICATION INCLUDES
// DEFINES
#ifndef FALSE
#define FALSE (1==0)
#endif

#ifndef TRUE
#define TRUE (1==1)
#endif

#ifndef NULL
#define NULL 0
#endif

#define UTL_NOT_FOUND (-1)

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

typedef int UtlBoolean ;

/** FORMAT_INTLL is a string containing the format length specifier
 *  for printing an Int64 with the 'd', 'x', etc. format specifiers.  E.g.:
 *      Int64 xyz;
 *      printf("The value is %" FORMAT_INTLL "d", xyz);
 *  Note that the '%' before and the format specifier after must be provided.
 *  This must be a preprocessor macro, since this specifier isn't standardized.
 */
#if defined(_WIN32)
#  define   FORMAT_INTLL   "I64"
#elif defined(__pingtel_on_posix__)
#  define   FORMAT_INTLL   "ll"
#else
#  error Unsupported target platform.
#endif

typedef const char* const UtlContainableType ;

// FORWARD DECLARATIONS

#endif // _UtlDefs_h_
