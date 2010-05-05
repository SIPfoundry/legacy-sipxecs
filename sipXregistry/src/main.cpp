//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <stdio.h>
#include <iostream>

// APPLICATION INCLUDES
#ifndef SIPX_VERSION
#  include "sipxregistry-buildstamp.h"
#  define SIPXCHANGE_VERSION SipXregistryVersion
#  define SIPXCHANGE_VERSION_COMMENT SipXregistryBuildStamp
#else
#  define SIPXCHANGE_VERSION SIPX_VERSION
#  define SIPXCHANGE_VERSION_COMMENT ""
#endif

#include "net/NameValueTokenizer.h"
#include "sipXecsService/SipXecsService.h"
#include "RegistrarService.h"

// DEFINES

#define CONFIG_SETTING_PREFIX         "SIP_REGISTRAR"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const char* SERVICE_NAME = "sipregistrar";
// STRUCTS
// TYPEDEFS

using namespace std ;

// FUNCTIONS
// FORWARD DECLARATIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// GLOBAL VARIABLE INITIALIZATIONS

/* ============================ FUNCTIONS ================================= */

/** The main entry point to the sipregistrar */
int
main(int argc, char* argv[] )
{
   UtlString argString;
   for (int argIndex = 1; argIndex < argc; argIndex++)
   {
      argString = argv[argIndex];
      NameValueTokenizer::frontBackTrim(&argString, "\t ");
      if (argString.compareTo("-v") == 0)
      {
         printf("Version: %s (%s)\n", SIPXCHANGE_VERSION, SIPXCHANGE_VERSION_COMMENT);
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
   RegistrarService registrarService(SERVICE_NAME, CONFIG_SETTING_PREFIX, SIPXCHANGE_VERSION);

   registrarService.run();

   return 0;
}


// The infamous JNI_LightButton stub, to resolve the reference in libsipXcall.
int JNI_LightButton(long)
{
   return 0;
}
