//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <net/NameValueTokenizer.h>
#include "ProxyService.h"

#ifndef SIPX_VERSION
#  include "sipxproxy-buildstamp.h"
#  define SIPX_VERSION SipXproxyVersion
#  define SIPX_BUILD   SipXproxyBuildStamp
#else
#  define SIPX_BUILD   ""
#endif

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
static const char* PROXY_CONFIG_PREFIX = "SIPX_PROXY";
const char* PROXY_SERVICE_NAME = "sipXproxy";

// STRUCTS
// TYPEDEFS

#ifndef _WIN32
using namespace std ;
#endif

// FUNCTIONS

// FORWARD DECLARATIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// GLOBAL VARIABLE INITIALIZATIONS

/* ============================ FUNCTIONS ================================= */

int
main(int argc, char* argv[] )
{
   UtlString argString;
   for(int argIndex = 1; argIndex < argc; argIndex++)
   {
       argString = argv[argIndex];
       NameValueTokenizer::frontBackTrim(&argString, "\t ");
       if(argString.compareTo("-v") == 0)
       {
           printf("Version: %s %s\n", SIPX_VERSION, SIPX_BUILD);
           return(1);
       } else
       {
            osPrintf("usage: %s [-v] \nwhere:\n -v provides the software version\n",
               argv[0]);
           return(1);
       }
   }

   // Create sipXecsService framework
   ProxyService proxyService(PROXY_SERVICE_NAME, PROXY_CONFIG_PREFIX, SIPX_VERSION);

   proxyService.run();

   return 0;
}
