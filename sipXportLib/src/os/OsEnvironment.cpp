// 
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// constructor

#ifndef HAVE_SETENV
int setenv(const char *name, const char *value, int overwrite)
{
   size_t length;
   const char* equals = "=";
   
   length = strlen(name) + strlen(equals) + strlen(value) + 1;

   char assignment[length];
   strcat(assignment,name);
   strcat(assignment,equals);
   strcat(assignment,value);
   putenv(assignment);
   return 0;
}
#endif

