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
#include "RegistrarPersist.h"
#include "registry/SipRegistrar.h"
#include "SipRegistrarServer.h"

// DEFINES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

const int RegistrarPersist::SIP_REGISTRAR_DEFAULT_PERSIST_INTERVAL = 20;  // units are seconds

// TYPEDEFS
// FORWARD DECLARATIONS


/// Constructor
RegistrarPersist::RegistrarPersist(SipRegistrar& sipRegistrar) :
   mLock(OsBSem::Q_PRIORITY, OsBSem::FULL),
   mIsTimerRunning(false),
   mPersistTimer(getMessageQueue(), 0),
   mRegistrar(sipRegistrar),
   mPersistInterval(-1)
{
   // Get the persist interval.  This configuration parameter is provided mainly for
   // testing and debugging.  Usually it is not set explicitly in the configuration file.
   mRegistrar.getConfigDB()->get("SIP_REGISTRAR_PERSIST_INTERVAL", mPersistInterval);
   if (mPersistInterval == -1)
   {
      mPersistInterval = SIP_REGISTRAR_DEFAULT_PERSIST_INTERVAL;
   }
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "RegistrarPersist::RegistrarPersist "
                 "persist interval = %d seconds", mPersistInterval);
};


/// Destructor
RegistrarPersist::~RegistrarPersist()
{
}


// Start the timer that triggers garbage collection and DB persistence, if it's not running.
void RegistrarPersist::scheduleCleanAndPersist()
{
   OsLock mutex(mLock);

   if (!mIsTimerRunning)
   {
      // Start the timer
      mIsTimerRunning = true;
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "RegistrarPersist::scheduleCleanAndPersist in %d seconds"
                    ,mPersistInterval);
      assert(mPersistInterval > 0);
      mPersistTimer.oneshotAfter(OsTime(mPersistInterval, 0));
   }
}


// Handle the expiration of the persist timer
UtlBoolean RegistrarPersist::handleMessage(OsMsg& eventMessage)    ///< Timer expiration msg
{
   UtlBoolean handled = TRUE;

   int msgType    = eventMessage.getMsgType();
   int msgSubType = eventMessage.getMsgSubType();

   if (   OsMsg::OS_EVENT    == msgType
       && OsEventMsg::NOTIFY == msgSubType
       )
   {
      cleanAndPersist();
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_CRIT,
                    "RegistrarPersist::handleMessage received unexpected message %d/%d",
                    msgType, msgSubType);
   }
   
   return handled;
}


/// Garbage-collection and persist the registration DB
void RegistrarPersist::cleanAndPersist()
{
   // Change state to indicate that the timer is no longer running.
   // Do this *before* saving the DB so that any DB changes that happen just after
   // we write the changes to disk will cause the timer to restart.
   {
      OsLock mutex(mLock);
      mIsTimerRunning = false;
   }

   // Save the DB
   getRegistrarServer().cleanAndPersist();
}


SipRegistrarServer& RegistrarPersist::getRegistrarServer()
{
   return mRegistrar.getRegistrarServer();
}
