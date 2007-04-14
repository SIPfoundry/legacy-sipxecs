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
#include "os/OsDateTime.h"
#include "os/OsEventMsg.h"
#include "os/OsQueuedEvent.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsQueuedEvent::OsQueuedEvent(OsMsgQ& rMsgQ, const int userData)
:  mUserData(userData),
   mpMsgQ(&rMsgQ)
{
}

// Destructor
OsQueuedEvent::~OsQueuedEvent()
{
   mpMsgQ = NULL;
   // no work required
}

/* ============================ MANIPULATORS ============================== */

// Set the event data and send an event message to the designated queue
// Return the result of the message send operation.
OsStatus OsQueuedEvent::signal(const int eventData)
{
   OsStatus res;

   res = doSendEventMsg(OsEventMsg::NOTIFY, eventData);
   return res;
}

// Set the user data value for this object
// Always returns OS_SUCCESS.
OsStatus OsQueuedEvent::setUserData(int userData)
{
   mUserData = userData;
   return OS_SUCCESS;
}

/* ============================ ACCESSORS ================================= */

// Return the user data specified when this object was constructed.
// Always returns OS_SUCCESS.
OsStatus OsQueuedEvent::getUserData(int& rUserData) const
{
   rUserData = mUserData;
   return OS_SUCCESS;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

// Send an event message to the designated message queue.
// Return the result of the message send operation.
OsStatus OsQueuedEvent::doSendEventMsg(const int msgType,
                                       const int eventData) const
{
   OsTime timestamp;

   OsDateTime::getCurTimeSinceBoot(timestamp);

   OsEventMsg msg(msgType,     // event msg type
                  *this,       // this event
                  eventData,   // data signaled with event
                  timestamp);  // event timestamp (time since boot)

   OsStatus rc;
   if (mpMsgQ)
   {
      rc = mpMsgQ->send(msg);
   }
   else
   {
      rc = OS_FAILED;
   }
   return rc;
}

/* ============================ FUNCTIONS ================================= */


