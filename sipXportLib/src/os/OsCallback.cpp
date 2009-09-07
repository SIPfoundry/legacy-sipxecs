//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "os/OsCallback.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

// Event notification method where a callback function is executed in the
// Notifier's context when the corresponding event occurs.

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsCallback::OsCallback(void* userData, const OsCallbackFunc func)
: mFunc(func),
  mUserData(userData)
{
   // all work is done by the initializers, no other work required
}

// Destructor
OsCallback::~OsCallback()
{
   // no work required
}

/* ============================ MANIPULATORS ============================== */

// Signal the occurrence of the event by executing the callback function.
// Always return OS_SUCCESS.
OsStatus OsCallback::signal(intptr_t eventData)
{
   mFunc(mUserData, eventData);   // execute the callback function

   return OS_SUCCESS;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
