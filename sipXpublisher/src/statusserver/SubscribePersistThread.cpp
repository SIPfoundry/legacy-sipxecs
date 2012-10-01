//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsConfigDb.h"
#include "os/OsEventMsg.h"
#include "os/OsTime.h"
#include "os/OsLock.h"
#include "statusserver/SubscribePersistThread.h"
#include "statusserver/SubscribeServerThread.h"
#include "statusserver/StatusServer.h"

// DEFINES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

const int SubscribePersistThread::SIP_STATUS_DEFAULT_PERSIST_INTERVAL = 20;  // units are seconds

// TYPEDEFS
// FORWARD DECLARATIONS


/// Constructor
SubscribePersistThread::SubscribePersistThread(StatusServer& statusServer) :
   OsServerTask("SubscribePersistThread", NULL, 2000),
   mLock(OsBSem::Q_PRIORITY, OsBSem::FULL),
   mIsTimerRunning(false),
   mPersistTimer(getMessageQueue(), 0),
   mStatusServer(statusServer),
   mPersistInterval(-1)
{
   // Get the persist interval.  This configuration parameter is provided mainly for
   // testing and debugging.  Usually it is not set explicitly in the configuration file.
   mStatusServer.getConfigDb().get("SIP_STATUS_PERSIST_INTERVAL", mPersistInterval);
   if (mPersistInterval == -1)
   {
      mPersistInterval = SIP_STATUS_DEFAULT_PERSIST_INTERVAL;
   }
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "SubscribePersistThread::SubscribePersistThread "
                 "persist interval = %d seconds", mPersistInterval);
};


/// Destructor
SubscribePersistThread::~SubscribePersistThread()
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "SubscribePersistThread::~SubscribePersistThread ");
   waitUntilShutDown();
}


// Start the timer that triggers DB persistence, if it's not running.
// This is called by SubscribeServerThread to start the timer.
void SubscribePersistThread::schedulePersist()
{
   OsLock mutex(mLock);

   if (!mIsTimerRunning)
   {
      // Start the timer
      mIsTimerRunning = true;
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SubscribePersistThread::schedulePersist in %d seconds"
                    ,mPersistInterval);
      assert(mPersistInterval > 0);
      mPersistTimer.oneshotAfter(OsTime(mPersistInterval, 0));
   }
}


// Handle the expiration of the persist timer
UtlBoolean SubscribePersistThread::handleMessage(OsMsg& eventMessage)    ///< Timer expiration msg
{
   UtlBoolean handled = FALSE;

   int msgType    = eventMessage.getMsgType();
   int msgSubType = eventMessage.getMsgSubType();

   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SubscribePersistThread::handleMessage received message %d/%d",
                    msgType, msgSubType);

   if (   OsMsg::OS_EVENT    == msgType
       && OsEventMsg::NOTIFY == msgSubType
       )
   {
      persist();
      handled = TRUE;
   }

   return handled;
}


/// Persist the subscription DB
void SubscribePersistThread::persist()
{
   // Change state to indicate that the timer is no longer running.
   // Do this *before* saving the DB so that any DB changes that happen just after
   // we write the changes to disk will cause the timer to restart.
   {
      OsLock mutex(mLock);
      mIsTimerRunning = false;
   }

   // Save the DB
   getSubscribeServerThread()->persist();
}


SubscribeServerThread* SubscribePersistThread::getSubscribeServerThread()
{
   return mStatusServer.getSubscribeServerThread();
}
