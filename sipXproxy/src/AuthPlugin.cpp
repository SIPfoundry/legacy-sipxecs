// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "AuthPlugin.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Provide a string version of an AuthResult value for logging. 
const char* AuthPlugin::AuthResultStr(AuthResult result)
{
   switch (result)
   {
   case CONTINUE:
      return "CONTINUE";

   case DENY:
      return "DENY";

   case ALLOW:
      return "ALLOW";

   default:
      return "(invalid)";
   }
}

   
