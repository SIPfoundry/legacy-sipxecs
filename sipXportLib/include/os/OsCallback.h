//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsCallback_h_
#define _OsCallback_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsNotification.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS

// TYPEDEFS
typedef void (*OsCallbackFunc) (void* userData, intptr_t eventData);

// FORWARD DECLARATIONS

//:Callback function that is executed when the event is signaled.
//:Event notification method where a callback function is executed in the
//:Notifier's context when the corresponding event occurs.

// <p><b>Background</b>
// <p>First, a little bit of terminology.  The task that wishes to be notified
// when an event occurs is the "Listener" task. The task that signals when
// a given event occurs is the "Notifier" task.  A Notifier informs the
// Listener that a given event has occurred by sending an "Event
// Notification".<br><br>
//
// <p><b>Expected Usage</b>
// <p>When the corresponding event occurs, the designated callback routine is
// invoked in the Notifier's task context.

class OsCallback : public OsNotification
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   OsCallback(void* userData, const OsCallbackFunc func);
     //:Constructor

   virtual
      ~OsCallback();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   virtual OsStatus signal(intptr_t eventData);
     //:Signal the occurrence of the event by executing the callback function
     // Always return OS_SUCCESS.

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   OsCallbackFunc mFunc;     // routine that will be invoked in the notifier's
                             //  context to signal the event
   void*          mUserData; // data specified on behalf of the user and
                             //  not otherwise used by this class -- the user
                             //  data is specified as an argument to the class
                             //  constructor

   OsCallback(const OsCallback& rOsCallback);
     //:Copy constructor (not implemented for this class)

   OsCallback& operator=(const OsCallback& rhs);
     //:Assignment operator (not implemented for this class)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsCallback_h_
