//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _SUBSCRIBEPERSISTTHREAD_H_
#define _SUBSCRIBEPERSISTTHREAD_H_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsTimer.h"
#include "os/OsServerTask.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS
class StatusServer;
class SubscribeServerThread;


/**
 * This thread is responsible for periodically persisting the subscription DB
 */
class SubscribePersistThread : public OsServerTask
{
public:
   /// Constructor
   SubscribePersistThread(StatusServer& statusServer);

   /// Destructor
   virtual ~SubscribePersistThread();

   /// Start the timer that triggers DB persistence, if it's not running
   void schedulePersist();

protected:
   /// Default persist interval, in units of seconds
   static const int SIP_STATUS_DEFAULT_PERSIST_INTERVAL;

   /// Mutex must be locked with OsLock to access any shared member variables
   OsBSem mLock;

   bool mIsTimerRunning;

   OsTimer mPersistTimer;

   /// The top level task - do not delete
   StatusServer& mStatusServer;

   /// Time interval for garbage collection and persistence
   int mPersistInterval;

   /// Handle the expiration of the persist timer
   UtlBoolean handleMessage(OsMsg& eventMessage);    ///< Timer expiration msg

   /// Persist the subscription DB
   void persist();

   SubscribeServerThread* getSubscribeServerThread();

private:
   /// There is no copy constructor
   SubscribePersistThread(const SubscribePersistThread&);

   /// There is no assignment operator
   SubscribePersistThread& operator=(const SubscribePersistThread&);
};

#endif // _SUBSCRIBEPERSISTTHREAD_H_
