//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _PtDefs_h_
#define _PtDefs_h_

// SYSTEM INCLUDES
#include <string.h>

// APPLICATION INCLUDES
#include "os/OsDefs.h"

// DEFINES
//#define PTAPI_TEST

// MACROS

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#ifdef LONG_EVENT_RESPONSE_TIMEOUTS
#  define PT_CONST_EVENT_WAIT_TIMEOUT   2592000   // 30 days in seconds
#else
#  define PT_CONST_EVENT_WAIT_TIMEOUT    40               // time out, seconds
#endif

// STRUCTS
// TYPEDEFS
#undef PTAPI_DEBUG_TRACE

#ifdef PTAPI_DEBUG_TRACE
#define EVENT_TRACE(x) osPrintf(x)
#else
#define EVENT_TRACE(x) //* osPrintf(x) *//
#endif

// FORWARD DECLARATIONS

//:Status codes returned by Pingtel API methods.

enum PtStatus
{
   PT_SUCCESS,
   PT_AUTH_FAILED,
   PT_FAILED,
   PT_EXISTS,
   PT_HOST_NOT_FOUND,
   PT_IN_PROGRESS,
   PT_INVALID_ARGUMENT,
   PT_INVALID_PARTY,
   PT_INVALID_STATE,
   PT_INVALID_IP_ADDRESS,
   PT_INVALID_SIP_DIRECTORY_SERVER,
   PT_INVALID_SIP_URL,
   PT_MORE_DATA,
   PT_NO_MORE_DATA,
   PT_NOT_FOUND,
   PT_PROVIDER_UNAVAILABLE,
   PT_RESOURCE_UNAVAILABLE,
   PT_BUSY
};

typedef int PtBoolean;

#define PT_CLASS_INFO_MEMBERS static const char* sClassName;
#define PT_NO_PARENT_CLASS
#define PT_IMPLEMENT_CLASS_INFO(CHILD, PARENT) \
const char* CHILD::sClassName = #CHILD; \
PtBoolean isInstanceOf(const char* name); \
\
const char* CHILD::className() { return(sClassName);} \
\
PtBoolean CHILD::isClass(const char* name) { return(strcmp(name, className()) == 0);} \
\
PtBoolean CHILD::isInstanceOf(const char* name) \
{\
    PtBoolean isInstance = isClass(name); \
    if(!isInstance) isInstance = PARENT::isInstanceOf(name); \
    return(isInstance); \
}

/* ============================ INLINE METHODS ============================ */

#endif  // _PtDefs_h_
