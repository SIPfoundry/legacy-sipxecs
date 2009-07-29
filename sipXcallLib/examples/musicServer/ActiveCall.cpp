//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <ActiveCall.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
const UtlContainableType TYPE = "ActiveCall";


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
ActiveCall::ActiveCall(UtlString& callId, CallObject* call)
{
   mCallId = callId;
   mpCall = call;
}


ActiveCall::ActiveCall(UtlString& callId)
{
   mCallId = callId;
}


ActiveCall::~ActiveCall()
{
   mpCall = NULL;
}


int ActiveCall::compareTo(const UtlContainable *b) const
{
   return mCallId.compareTo(((ActiveCall *)b)->mCallId);
}


unsigned int ActiveCall::hash() const
{
    return mCallId.hash();
}

const UtlContainableType ActiveCall::getContainableType() const
{
    return TYPE;
}


/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */
