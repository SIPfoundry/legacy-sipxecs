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
#include "ACDCall.h"
#include "ACDQueueMsg.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueueMsg::ACDQueueMsg
//
//  SYNOPSIS:
//
//  DESCRIPTION: Constructor for ADD_CALL, REMOVE_CALL, CALL_CONNECTED & MAX_WAIT_TIME messages
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ACDQueueMsg::ACDQueueMsg(eACDQueueMsgSubTypes type, ACDCall* pCallRef)
: OsMsg(USER_START, type)
{
   mpACDCallReference = pCallRef;
   mACDCallRouteState = ACDCallRouteState::ROUTE_STATE_UNDEFINED;
   mpACDAgentReference = NULL;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueueMsg::ACDQueueMsg
//
//  SYNOPSIS:
//
//  DESCRIPTION: Constructor for UPDATE_ROUTE_STATE message
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ACDQueueMsg::ACDQueueMsg(eACDQueueMsgSubTypes type, ACDCall* pCallRef, ACDCallRouteState::eRouteState state)
: OsMsg(USER_START, type)
{
   mpACDCallReference = pCallRef;
   mACDCallRouteState = state;
   mpACDAgentReference = NULL;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueueMsg::ACDQueueMsg
//
//  SYNOPSIS:
//
//  DESCRIPTION: Constructor for AGENT_AVAILABLE messages
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ACDQueueMsg::ACDQueueMsg(eACDQueueMsgSubTypes type, ACDAgent* pAgentRef)
: OsMsg(USER_START, type)
{
   mpACDCallReference = NULL;
   mACDCallRouteState = ACDCallRouteState::ROUTE_STATE_UNDEFINED;
   mpACDAgentReference = pAgentRef;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueueMsg::ACDQueueMsg
//
//  SYNOPSIS:
//
//  DESCRIPTION: Copy constructor
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ACDQueueMsg::ACDQueueMsg(const ACDQueueMsg& rACDQueueMsg)
: OsMsg(rACDQueueMsg)
{
   mpACDCallReference  = rACDQueueMsg.mpACDCallReference;
   mACDCallRouteState  = rACDQueueMsg.mACDCallRouteState;
   mpACDAgentReference = rACDQueueMsg.mpACDAgentReference;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueueMsg::createCopy
//
//  SYNOPSIS:
//
//  DESCRIPTION: Create a copy of this msg object (which may be of a derived type)
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

OsMsg* ACDQueueMsg::createCopy(void) const
{
   return new ACDQueueMsg(*this);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueueMsg::~ACDQueueMsg
//
//  SYNOPSIS:
//
//  DESCRIPTION: Destructor
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ACDQueueMsg::~ACDQueueMsg()
{
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
