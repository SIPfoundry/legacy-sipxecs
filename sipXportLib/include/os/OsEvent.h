//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsEvent_h_
#define _OsEvent_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsBSem.h"
#include "os/OsNotification.h"
#include "os/OsTime.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Events are used to synchronize a task with an ISR or between two tasks
// Events consist of event data (an integer) that is set when the event is
// signaled and a state variable that indicates whether the event has been
// signaled. When first initialized, an OsEvent is ready to be signaled.
// However, once signaled, the OsEvent must be explicitly reset before it
// may be signaled again. An OsEvent is intended for use in synchronizing
// one notifier (task or ISR) with one listener task. If an OsEvent object
// is intended for use with more than one notifier or listener, then an
// external mutex must be used to serialize access and avoid race
// conditions.
//
// <p><b>Background</b>
// <p>First, a little bit of terminology.  The task that wishes to be notified
// when an event occurs is the "Listener" task. The task that signals when
// a given event occurs is the "Notifier" task.  A Notifier informs the
// Listener that a given event has occurred by sending an "Event
// Notification".
//
// <p><b>Expected Usage</b>
// <p>The Listener passes an event object to the Notifier.  When the
// corresponding event occurs, the Notifier uses the event object
// to signal the occurrence of the event. The Listener may receive
// event notifications by: polling, blocking until the event is
// signaled, or blocking until either the event is signaled or a
// timeout expires.  When the Listener receives the event
// notification, it can then invoke the appropriate event handler.
// This handler will run in the Listener's task context.
//
// <p>Note: Using a busy loop to poll for event status is considered
// anti-social behavior.  However, when using the event object
// approach, a task can perform a blocking wait for only one event
// at a time.  A solution that allows a task to receive signals
// for multiple events is given below (event / work message queue).

class OsEvent : public OsNotification
{

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   OsEvent(void* userData = 0);
     //:Constructor

   virtual
   ~OsEvent();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   virtual OsStatus signal(intptr_t eventData);
     //:Set the event data and signal the occurrence of the event
     // Return OS_ALREADY_SIGNALED if the event has already been signaled
     // (and has not yet been cleared), otherwise return OS_SUCCESS.

   virtual OsStatus reset(void);
     //:Reset the event so that it may be signaled again
     // Return OS_NOT_SIGNALED if the event has not been signaled (or has
     // already been cleared), otherwise return OS_SUCCESS.

   virtual OsStatus wait(const OsTime& rTimeout = OsTime::OS_INFINITY);
     //:Wait for the event to be signaled
     // Return OS_BUSY if the timeout expired, otherwise return OS_SUCCESS.

   virtual OsStatus setUserData(void* userData);
     //:Sets the user data specified.  There are situations (such as the OsProtedtedEvent)
     // when the user data can not be specified when this object was constructed
     // so that this method is necessary to set the user data.
     // Always returns OS_SUCCESS.

/* ============================ ACCESSORS ================================= */

   virtual OsStatus getEventData(intptr_t& rEventData);
     //:Return the event data that was signaled by the notifier task.
     // Return OS_NOT_SIGNALED if the event has not been signaled (or has
     // already been cleared), otherwise return OS_SUCCESS.

   virtual OsStatus getUserData(void*& rUserData) const;
     //:Return the user data specified when this object was constructed.
     // Always returns OS_SUCCESS.

/* ============================ INQUIRY =================================== */

   virtual UtlBoolean isSignaled(void);
     //:Return TRUE if the event has been signaled, otherwise FALSE

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   intptr_t  mEventData;   // data set when the event was signaled
   UtlBoolean mIsSignaled;  // indicates whether the event has been signaled
   OsBSem    mSignalSem;   // semaphore used to queue up tasks waiting for
                           //  the event to be signaled
   void*     mUserData;    // data specified on behalf of the user and
                           //  not otherwise used by this class -- the user
                           //  data is specified as an argument to the class
                           //  constructor

   OsEvent(const OsEvent& rOsEvent);
     //:Copy constructor (not implemented for this class)

   OsEvent& operator=(const OsEvent& rhs);
     //:Assignment operator (not implemented for this class)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsEvent_h_
