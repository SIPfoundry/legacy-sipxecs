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
#include <os/OsSysLog.h>
#include <utl/UtlString.h>
#include <os/OsSysLog.h>
#include <net/SipSubscribeServerEventHandler.h>
#include <net/SipPublishContentMgr.h>
#include <net/SipMessage.h>
#include <net/Url.h>
#include "RlsSubscribePolicy.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
RlsSubscribePolicy::RlsSubscribePolicy()
{
}

// Copy constructor NOT IMPLEMENTED
RlsSubscribePolicy::RlsSubscribePolicy(const RlsSubscribePolicy& rRlsSubscribePolicy)
{
}

// Destructor
RlsSubscribePolicy::~RlsSubscribePolicy()
{
    // Iterate through and delete all the dialogs
    // TODO:
}

/* ============================ MANIPULATORS ============================== */

UtlBoolean RlsSubscribePolicy::isAuthorized(const SipMessage& subscribeRequest,
                                            const UtlString& resourceId,
                                            const UtlString& eventTypeKey,
                                            SipMessage& subscribeResponse)
{
   // SUBSCRIBE is authorized if "eventlist" is supported.
   return subscribeRequest.isInSupportedField("eventlist");
}

/* ============================ ACCESSORS ================================= */


/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
