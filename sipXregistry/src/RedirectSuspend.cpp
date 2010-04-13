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
#include "registry/RedirectSuspend.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const UtlContainableType RedirectSuspend::TYPE = "RedirectSuspend";

// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

RedirectSuspend::RedirectSuspend(int noRedirectors) :
   mSuspendCount(0),
   mNoRedirectors(noRedirectors),
   mRedirectors((struct redirector*)
                malloc(noRedirectors * sizeof (struct redirector)))
{
   for (int i = 0; i < noRedirectors; i++)
   {
      mRedirectors[i].suspended = FALSE;
      mRedirectors[i].needsCancel = FALSE;
      mRedirectors[i].privateStorage = NULL;
   }
}

RedirectSuspend::~RedirectSuspend()
{
   for (int i = 0; i < mNoRedirectors; i++)
   {
      if (mRedirectors[i].privateStorage)
      {
         delete mRedirectors[i].privateStorage;
      }
   }
   free(mRedirectors);
}

const char* const RedirectSuspend::getContainableType() const
{
  return TYPE;
}
