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
#include <cp/LinePresenceBase.h>
#include <cp/LinePresenceMonitorMsg.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        LinePresenceMonitorMsg::LinePresenceMonitorMsg
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

LinePresenceMonitorMsg::LinePresenceMonitorMsg(eLinePresenceMonitorMsgSubTypes type,
                                               LinePresenceBase* line,
                                               OsEvent* event)
   : OsMsg(USER_START, type)
{
   mLine = line;
   mEvent = event;
   mContact = NULL;
   mStateChange = StateChangeNotifier::PRESENT; // dummy value
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        LinePresenceMonitorMsg::LinePresenceMonitorMsg
//
//  SYNOPSIS:
//
//  DESCRIPTION: Constructor for SET_STATE messages
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

LinePresenceMonitorMsg::LinePresenceMonitorMsg(const UtlString* contact,
                                               StateChangeNotifier::Status value)
   : OsMsg(USER_START, SET_STATUS)
{
   mLine = NULL;
   mEvent = NULL;
   mContact = contact;
   mStateChange = value;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        LinePresenceMonitorMsg::LinePresenceMonitorMsg
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

LinePresenceMonitorMsg::LinePresenceMonitorMsg(const LinePresenceMonitorMsg& rLinePresenceMonitorMsg)
: OsMsg(rLinePresenceMonitorMsg)
{
   mLine = rLinePresenceMonitorMsg.mLine;
   mStateChange = rLinePresenceMonitorMsg.mStateChange;
   // The event and contact pointers are copied from the source.
   // This means that one and only one of these LinePresenceMonitorMsgs should
   // be sent to the message queue.
   mEvent = rLinePresenceMonitorMsg.mEvent;
   mContact = rLinePresenceMonitorMsg.mContact;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        LinePresenceMonitorMsg::createCopy
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

OsMsg* LinePresenceMonitorMsg::createCopy(void) const
{
   return new LinePresenceMonitorMsg(*this);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        LinePresenceMonitorMsg::~LinePresenceMonitorMsg
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

LinePresenceMonitorMsg::~LinePresenceMonitorMsg()
{
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
