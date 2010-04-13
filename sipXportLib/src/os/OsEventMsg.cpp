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
#include "os/OsEventMsg.h"
#include "os/OsQueuedEvent.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsEventMsg::OsEventMsg(const unsigned char subType,
                       const OsQueuedEvent& rEvent,
                       intptr_t eventData,
                       const OsTime& rTimestamp)
:  OsMsg(OsMsg::OS_EVENT, subType),
   mEventData(eventData),
   mTimestamp(rTimestamp)
{
   OsStatus res;

   res = rEvent.getUserData(mUserData);
   assert(res == OS_SUCCESS);
}

OsEventMsg::OsEventMsg(unsigned char subType,
                       intptr_t eventData,
                       void* userData)
:  OsMsg(OsMsg::OS_EVENT, subType),
   mEventData(eventData),
   mUserData(userData)
{
}

// Copy constructor
OsEventMsg::OsEventMsg(const OsEventMsg& rOsEventMsg)
:  OsMsg(rOsEventMsg)
{
   mEventData = rOsEventMsg.mEventData;
   mUserData  = rOsEventMsg.mUserData;
   mTimestamp = rOsEventMsg.mTimestamp;
}

// Create a copy of this msg object (which may be of a derived type)
OsMsg* OsEventMsg::createCopy(void) const
{
   return new OsEventMsg(*this);
}

// Destructor
OsEventMsg::~OsEventMsg()
{
   // no work required
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
OsEventMsg&
OsEventMsg::operator=(const OsEventMsg& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   OsMsg::operator=(rhs);       // assign fields for parent class

   mEventData = rhs.mEventData;
   mUserData  = rhs.mUserData;
   mTimestamp = rhs.mTimestamp;

   return *this;
}

/* ============================ ACCESSORS ================================= */

// Return the size of the message in bytes.
// This is a virtual method so that it will return the accurate size for
// the message object even if that object has been upcast to the type of
// an ancestor class.
int OsEventMsg::getMsgSize(void) const
{
   return sizeof(*this);
}

// Return the event data that was signaled by the notifier task.
// Always returns OS_SUCCESS.
OsStatus OsEventMsg::getEventData(intptr_t& rEventData) const
{
   rEventData = mEventData;
   return OS_SUCCESS;
}

// Return the timestamp associated with this event.
// Always returns OS_SUCCESS.
OsStatus OsEventMsg::getTimestamp(OsTime& rTimestamp) const
{
   rTimestamp = mTimestamp;
   return OS_SUCCESS;
}

// Return the user data specified when the OsQueuedEvent was constructed.
// Always returns OS_SUCCESS.
OsStatus OsEventMsg::getUserData(void*& rUserData) const
{
   rUserData = mUserData;
   return OS_SUCCESS;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
