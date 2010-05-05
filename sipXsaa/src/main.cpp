//
//
// Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "config/sipxsaa-buildstamp.h"
#include "net/NameValueTokenizer.h"
#include "os/OsSysLog.h"
#include "sipXecsService/SipXecsService.h"
#include "utl/UtlString.h"
#include "AppearanceAgentService.h"
#include "main.h"

// DEFINES
#ifndef SIPX_VERSION
   #include "sipxsaa-buildstamp.h"
   #define SIPXCHANGE_VERSION          SipXsaaVersion
   #define SIPXCHANGE_VERSION_COMMENT  SipXsaaBuildStamp
#else
   #define SIPXCHANGE_VERSION          SIPX_VERSION
   #define SIPXCHANGE_VERSION_COMMENT  ""
#endif

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const char* SERVICE_NAME = "sipxsaa";
const char* CONFIG_SETTING_PREFIX = "SIP_SAA";
// STRUCTS
// TYPEDEFS
// FUNCTIONS
// FORWARD DECLARATIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// GLOBAL VARIABLE INITIALIZATIONS


/* ============================ FUNCTIONS ================================= */

//
// The main entry point to sipXsaa.
//
int main(int argc, char* argv[])
{
   UtlString argString;
   for (int argIndex = 1; argIndex < argc; argIndex++)
   {
      argString = argv[argIndex];
      NameValueTokenizer::frontBackTrim(&argString, "\t ");
      if (argString.compareTo("-v") == 0)
      {
         printf("Version: %s (%s)\n", SIPXCHANGE_VERSION,
                  SIPXCHANGE_VERSION_COMMENT);
         return 1;
      }
      else
      {
         printf("usage: %s [-v]\nwhere:\n -v provides the software version\n",
                  argv[0]);
         return 1;
      }
   }

   // Create sipXecsService framework
   AppearanceAgentService saaService(SERVICE_NAME, CONFIG_SETTING_PREFIX, SIPXCHANGE_VERSION);

   saaService.run();

   // Say goodnight Gracie...
   return 0;
}

// Stub to avoid pulling in ps library
int JNI_LightButton(long)
{
   return 0;
}
