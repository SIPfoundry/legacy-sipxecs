//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsLinuxDefs_h_
#define _OsLinuxDefs_h_

// SYSTEM INCLUDES
#include <errno.h>
#include <stdio.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

// APPLICATION INCLUDES

// DEFINES
// Change the VxWorks defines that interfere with names used by our
// own abstraction layer.
// Not sure if these are present under other OS's but it can't hurt :)
#undef NO_WAIT
#undef OK
#undef WAIT_FOREVER

#define POSIX_OK           0
#define POSIX_NO_WAIT      0
#define POSIX_WAIT_FOREVER (-1)

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

#ifdef __cplusplus
}
#endif

#endif  // _OsLinuxDefs_h_
