//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsDefs_h_
#define _OsDefs_h_

// SYSTEM INCLUDES
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifdef _VXWORKS
#include "os/Vxw/OsVxwDefs.h"
#include <envLib.h>  //needed for putenv
#endif // _VXWORKS

#ifdef __pingtel_on_posix__
#include "os/linux/OsLinuxDefs.h"
#endif // __pingtel_on_posix__

// APPLICATION INCLUDES
// MACROS
// EXTERNAL FUNCTIONS
// DEFINES

#ifdef __cplusplus
extern "C" {
#endif

void enableConsoleOutput(int bEnable) ;
void osPrintf(const char* format , ...)
#ifdef __GNUC__
            // with the -Wformat switch, this enables format string checking
            __attribute__ ((format (printf, 1, 2)))
#endif
         ;

// to print intptr_t types, we need a platform-specific formatter
#ifdef __pingtel_on_posix__
#  define __STDC_FORMAT_MACROS 1
#  include <inttypes.h>
#else
#  error "Need a definition of PRIdPTR for this platform"
#endif

// @TODO clean up definition of 64 bit integer types - see also UtlDefs.h
#ifdef __pingtel_on_posix__
#ifndef __int64_already_defined
#  if defined(__linux__)
#     include <stdlib.h>
#     include <inttypes.h>
   typedef __int64_t __int64;
#  elif defined(__FreeBSD__)
#     include <stdlib.h>
   typedef __int64_t __int64;
#  elif defined(sun)
#     include <sys/int_types.h>
   typedef int64_t __int64;
#  elif defined(__MACH__) /* OS X */
#     include <sys/types.h>
   typedef int64_t __int64;
#  elif defined(__hpux) /* HP-UX */
#     include <sys/_inttypes.h>
   typedef int64_t __int64;
#  else
#     error "Need a definition of __int64 for this platform"
#  endif
#endif // __int64_already_defined
#endif

// A special value for "port number" which means that no port is specified.
#define PORT_NONE (-1)

// A special value for "port number" which means that some default port number
// should be used.  The default may be defined by the situation, or
// the OS may choose a port number.
// For use when PORT_NONE is used to mean "open no port", and in socket-opening
// calls.
#define PORT_DEFAULT (-2)

// Macro to test a port number for validity as a real port (and not PORT_NONE
// or PORT_DEFAULT).  Note that 0 is a valid port number for the protocol,
// but the Berkeley sockets interface makes it impossible to specify it.
// In addition, RTP treats port 0 as a special value.  Thus we forbid port 0.
#define portIsValid(p) ((p) >= 1 && (p) <= 65535)

// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

#ifdef __cplusplus
}
#endif

#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "time.h"

#endif  // _OsDefs_h_
