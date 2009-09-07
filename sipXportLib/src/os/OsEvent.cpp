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
#include "os/OsEvent.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

// Events are used to synchronize a task with an ISR or between two tasks.
// Events consist of event data (an integer) that is set when the event is
// signaled and a state variable that indicates whether the event has been
// signaled. When first initialized, an OsEvent is ready to be signaled.
// However, once signaled, the OsEvent must be explicitly cleared before it
// may be signaled again. An OsEvent is intended for use in synchronizing
// one notifier (task or ISR) with one listener task. If an OsEvent object
// is intended for use with more than one notifier or listener, then an
// external mutex must be used to serialize access and avid race conditions.

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsEvent::OsEvent(void* userData)
:  mEventData(-1),
   mIsSignaled(FALSE),
   mSignalSem(OsBSem::Q_PRIORITY, OsBSem::EMPTY),
   mUserData(userData)
{
   // all work is done by the initializers, no other work required
}

// Destructor
OsEvent::~OsEvent()
{
   // no work required
}

/* ============================ MANIPULATORS ============================== */

// Set the event data and signal the occurrence of the event.
// Return OS_ALREADY_SIGNALED if the event has already been signaled
// (and has not yet been cleared), otherwise return OS_SUCCESS.
OsStatus OsEvent::signal(intptr_t eventData)
{
   OsStatus res;

   if (mIsSignaled)
   {
      return OS_ALREADY_SIGNALED;    // error - has already been signaled
   }                                 //  (and not yet cleared)
   else
   {
      // Signal the event, making sure that the following conditions hold:
      //  1) if mIsSignaled == TRUE, then mEventData must be valid
      //  2) before releasing the signal semaphore, mIsSignaled must be TRUE
      mEventData  = eventData;       // set the event data
      mIsSignaled = TRUE;            // set the isSignaled flag
      res = mSignalSem.release();    // post the signal indication event
      assert(res == OS_SUCCESS);
      return OS_SUCCESS;
   }
}

// Reset the event so that it may be signaled again.
// Return OS_NOT_SIGNALED if the event has not been signaled (or has
// already been cleared), otherwise return OS_SUCCESS.
OsStatus OsEvent::reset(void)
{
   OsStatus res;

   if (mIsSignaled)
   {                                 // if the event has been signaled
      mEventData = -1;               //  set the event data to -1 and
      mIsSignaled = FALSE;           //  clear the isSignaled flag

      res = mSignalSem.tryAcquire(); // rearm the event by acquiring the
                                     //  semaphore whose release is used
                                     //  to signal this event
      assert(res == OS_SUCCESS || res == OS_BUSY);
      return OS_SUCCESS;
   }
   else
   {
      return OS_NOT_SIGNALED;        // error - had not yet been signaled
   }
}

// Wait for the event to be signaled.
// Return OS_BUSY if the timeout expired, otherwise return OS_SUCCESS.
OsStatus OsEvent::wait(const OsTime& rTimeout)
{
   return mSignalSem.acquire(rTimeout);
}

OsStatus OsEvent::setUserData(void* userData)
{
   mUserData = userData;
   return OS_SUCCESS;
}

/* ============================ ACCESSORS ================================= */

// Return the event data that was signaled by the notifier task.
// Return OS_NOT_SIGNALED if the event has not been signaled (or has
// already been cleared), otherwise return OS_SUCCESS.
OsStatus OsEvent::getEventData(intptr_t& rEventData)
{
   if (mIsSignaled)
   {
      rEventData = mEventData;
      return OS_SUCCESS;
   }
   else
   {
      return OS_NOT_SIGNALED;        // error - had not yet been signaled
   }
}

// Return the user data specified when this object was constructed.
// Always returns OS_SUCCESS.
OsStatus OsEvent::getUserData(void*& rUserData) const
{
   rUserData = mUserData;
   return OS_SUCCESS;
}

/* ============================ INQUIRY =================================== */

// Return TRUE if the event has been signaled, otherwise FALSE.
UtlBoolean OsEvent::isSignaled(void)
{
   return mIsSignaled;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
