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
#include "ACDAgentMsg.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgentMsg::ACDAgentMsg
//
//  SYNOPSIS:
//
//  DESCRIPTION: Constructor for UPDATE_STATE message
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ACDAgentMsg::ACDAgentMsg(eACDAgentMsgSubTypes type, LinePresenceBase::ePresenceStateType stateType, bool state)
: OsMsg(USER_START, type)
{
   mPresenceStateType = stateType;
   mPresenceState     = state;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgentMsg::ACDAgentMsg
//
//  SYNOPSIS:
//
//  DESCRIPTION: Constructor for DROP_CALL message
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ACDAgentMsg::ACDAgentMsg(eACDAgentMsgSubTypes type, bool rna)
: OsMsg(USER_START, type)
{
   mRna = rna ;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgentMsg::ACDAgentMsg
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

ACDAgentMsg::ACDAgentMsg(const ACDAgentMsg& rACDAgentMsg)
: OsMsg(rACDAgentMsg)
{
   switch(getMsgSubType())
   {
      case UPDATE_STATE:
         mPresenceStateType = rACDAgentMsg.mPresenceStateType;
         mPresenceState     = rACDAgentMsg.mPresenceState;
         break ;
      case DROP_CALL:
         mRna = rACDAgentMsg.mRna;
         break ;
   }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgentMsg::createCopy
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

OsMsg* ACDAgentMsg::createCopy(void) const
{
   return new ACDAgentMsg(*this);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgentMsg::~ACDAgentMsg
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

ACDAgentMsg::~ACDAgentMsg()
{
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
