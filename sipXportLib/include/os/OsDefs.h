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
#ifdef _VXWORKS
#include "os/Vxw/OsVxwDefs.h"
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
