//
//
// Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////////

#ifndef _AppAgentTask_h_
#define _AppAgentTask_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES

#include "os/OsServerTask.h"
#include "os/OsMsg.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

class AppearanceAgent;


/**
 * The AppAgentTask accepts and processes asynchronous events for
 * an AppearanceAgent.
 * The asynchronous events are messages from timers that fire and requests
 * to delete subscriptions by methods that already hold
 * AppearanceGroupSet::mSemaphore.
 * Timers are handled this way because it is the only safe way to process
 * timer events in the face of the possibility that any pointed-to object
 * may have already been deleted.
 * Requests to end subscriptions are handled this way because
 * SipSubscribeClient::endSubscriptionGroup can itself call the
 * subscription event callback, and the callback routine seizes
 * AppearanceGroupSet::mSemaphore.  So, to avoid deadlock, a method that
 * holds AppearanceGroupSet::mSemaphore queues a request to the
 * AppAgentTask to call endSubscriptionGroup "from the outside",
 * without holding AppearanceGroupSet::mSemaphore.
 */
class AppearanceAgentTask : public OsServerTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   AppearanceAgentTask(/// the parent AppearanceAgent
         AppearanceAgent* parent);

   ~AppearanceAgentTask();

/* ============================ MANIPULATORS ============================== */

   /// Method to process messages which get queued for this OsServerTask.
   UtlBoolean handleMessage(OsMsg& rMsg);

   /// Process a MESSAGE request, which is used to trigger debugging actions.
   void handleMessageRequest(const SipMessage& msg);

   /// Dump the state of the SAA into the log.
   void debugDumpState(const SipMessage& msg);

   /// Get the parent AppearanceAgent.
   AppearanceAgent* getAppearanceAgent() const;

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   /// The parent AppearanceAgent.
   AppearanceAgent* mAppearanceAgent;

   mutable int mVersion;

   ///Copy constructor
   AppearanceAgentTask(const AppearanceAgentTask& rAppearanceAgentTask);

   ///Assignment operator
   AppearanceAgentTask& operator=(const AppearanceAgentTask& rAppearanceAgentTask);
};

/* ============================ INLINE METHODS ============================ */

// Get the parent AppearanceAgent.
inline AppearanceAgent* AppearanceAgentTask::getAppearanceAgent() const
{
   return mAppearanceAgent;
}

#endif  // _AppAgentTask_h_
