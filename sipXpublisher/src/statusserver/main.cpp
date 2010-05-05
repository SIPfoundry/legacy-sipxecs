//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////
// StatusServer.cpp : Defines the entry point for the console application.
//

// SYSTEM INCLUDES
#include <iostream>
#include <stdio.h>

// APPLICATION INCLUDES
#ifndef SIPX_VERSION
#  include "sipxpublisher-buildstamp.h"
#  define SIPX_VERSION SipXpublisherVersion
#  define SIPX_BUILD SipXpublisherBuildStamp
#else
#  define SIPX_BUILD ""
#endif
#include "os/OsConfigDb.h"
#include "os/OsFS.h"
#include "os/OsTask.h"

#include "net/NameValueTokenizer.h"
#include "sipXecsService/SipXecsService.h"
#include "MwiStatusService.h"


// DEFINES
#define CONFIG_SETTING_PREFIX         "SIP_STATUS"
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
// EXTERNAL VARIABLES
// CONSTANTS
const char* SERVICE_NAME = "sipstatus";
// STATIC VARIABLE INITIALIZATIONS
// GLOBAL VARIABLE INITIALIZATIONS

using namespace std;

/** The main entry point to the StatusServer */
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
         printf("Version: %s (%s)\n", SIPX_VERSION, SIPX_BUILD);
         return(1);
      }
      else
      {
         printf("usage: %s [-v] \nwhere:\n -v provides the software version\n",
         argv[0]);
         return(1);
      }
   }

   // Create sipXecsService framework
   MwiStatusService statusService(SERVICE_NAME, CONFIG_SETTING_PREFIX, SIPX_VERSION);

   statusService.run();

    return 0;
}
