//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <stdio.h>
#include <unistd.h>

// APPLICATION INCLUDES
#include "sipXecsService/SipXecsService.h"
#include "net/NameValueTokenizer.h"
#include "ParkService.h"

// DEFINES
#ifndef SIPX_VERSION
#  include "sipxpark-buildstamp.h"
#  define SIPX_VERSION SipXparkVersion
#  define SIPX_BUILD   SipXparkBuildStamp
#else
#  define SIPX_BUILD   ""
#endif

#define CONFIG_SETTING_PREFIX         "SIP_PARK"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const char* SERVICE_NAME = "sipxpark";
// STRUCTS
// TYPEDEFS
// FUNCTIONS
// FORWARD DECLARATIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// GLOBAL VARIABLE INITIALIZATIONS


//
// The main entry point to the sipXpark
//
int main(int argc, char* argv[])
{
    UtlString argString;
    for(int argIndex = 1; argIndex < argc; argIndex++)
    {
        argString = argv[argIndex];
        NameValueTokenizer::frontBackTrim(&argString, "\t ");
        if(argString.compareTo("-v") == 0)
        {
            printf("Version: %s (%s)\n", SIPX_VERSION, SIPX_BUILD);
            return(1);
        } else
        {
           printf("usage: %s [-v]\nwhere:\n -v provides the software version\n",
            argv[0]);
            return(1);
        }
    }

    // Create sipXecsService framework
    ParkService parkService(SERVICE_NAME, CONFIG_SETTING_PREFIX, SIPX_VERSION);

    parkService.run();

    // Say goodnight Gracie...
    return 0;
}


// Stub to avoid pulling in ps library
int JNI_LightButton(long)
{
   return 0;
}
