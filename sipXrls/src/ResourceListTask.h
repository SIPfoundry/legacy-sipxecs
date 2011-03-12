//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////////

#ifndef _ResourceListTask_h_
#define _ResourceListTask_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES

#include <os/OsServerTask.h>
#include <os/OsMsg.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

class ResourceListServer;


//: The ResourceListTask accepts and processes asynchronous events for
//  a ResourceListServer.
//  The asynchronous events are messages from timers that fire and requests
//  to delete subscriptions by methods that already hold
//  ResourceListSet::mSemaphore.
//  Timers are handled this way because it is the only safe way to process
//  timer events in the face of the possibility that any pointed-to object
//  may have already been deleted.
//  Requests to end subscriptions are handled this way because
//  SipSubscribeClient::endSubscriptionGroup can itself call the
//  subscription event callback, and the callback routine seizes
//  ResourceListSet::mSemaphore.  So, to avoid deadlock, a method that
//  holds ResourceListSet::mSemaphore queues a request to the
//  ResourceListTask to call endSubscriptionGroup "from the outside",
//  without holding ResourceListSet::mSemaphore.
class ResourceListTask : public OsServerTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   ResourceListTask(/// the parent ResourceListServer
                    ResourceListServer* parent);

   ~ResourceListTask();

/* ============================ MANIPULATORS ============================== */

   UtlBoolean handleMessage(OsMsg& rMsg);
     //: Method to process messages which get queued for this OsServerTask.

   //! Process a MESSAGE request, which is used to trigger debugging actions.
   void handleMessageRequest(const SipMessage& msg);

   //! Dump the state of the RLS into the log.
   void debugDumpState(const SipMessage& msg);

   //! Get the parent ResourceListServer.
   ResourceListServer* getResourceListServer() const;

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   //! The parent ResourceListServer.
   ResourceListServer* mResourceListServer;

   ResourceListTask(const ResourceListTask& rResourceListTask);
   //:Copy constructor

   ResourceListTask& operator=(const ResourceListTask& rResourceListTask);
   //:Assignment operator

};

/* ============================ INLINE METHODS ============================ */

// Get the parent ResourceListServer.
inline ResourceListServer* ResourceListTask::getResourceListServer() const
{
   return mResourceListServer;
}

#endif  // _ResourceListTask_h_
