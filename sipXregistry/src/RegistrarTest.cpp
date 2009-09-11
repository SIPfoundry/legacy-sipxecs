//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsEventMsg.h"
#include "os/OsTime.h"
#include "os/OsLock.h"
#include "utl/UtlSListIterator.h"
#include "registry/SipRegistrar.h"
#include "SyncRpc.h"
#include "RegistrarPeer.h"
#include "RegistrarTest.h"

// DEFINES
// CONSTANTS

const size_t REGISTER_TEST_INITIAL_WAIT = 2; // Seconds until the first test
const size_t REGISTER_TEST_MAX_WAIT = 32;    // Maximum seconds between tests

// TYPEDEFS
// FORWARD DECLARATIONS

/// constructor
RegistrarTest::RegistrarTest(SipRegistrar& sipRegistrar) :
   OsServerTask("RegistrarTest"),
   mLock(OsBSem::Q_PRIORITY, OsBSem::FULL),
   mTestState(StartupPhase),
   mRetryTimer(getMessageQueue(),0),
   mRetryTime(0),
   mSipRegistrar(sipRegistrar)
{
};

/// Signal that a peer has become UnReachable
void RegistrarTest::check()
{
   OsLock mutex(mLock);

   if ( Idle == mTestState )
   {
      restartTimer();
   }
}

/// Override default run method to do one initial check before waiting for timers
int RegistrarTest::run(void* pArg)
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "RegistrarTest started - starting initial check");
   /*
    * When this thread is first started, all peers should be either Uninitialized or
    * UnReachable, because the registrarSync.pullUpdates never sets them to Reachable.
    * So now we make one pass over all peers, invoking registrarSync.reset to get them
    * to Reachable.
    */
   checkPeers();
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "RegistrarTest initial check complete");

   // from here on, we run only when a timer has expired.
   return OsServerTask::run(pArg);
}

/// Check each unreachable or uninitialized peer
void RegistrarTest::checkPeers()
{
   // A timer has expired, so it's time to check on UnReachable peers
   SipRegistrar* sipRegistrar = NULL;
   UtlSListIterator* peers = NULL;

   sipRegistrar = &mSipRegistrar;
   peers = sipRegistrar ? sipRegistrar->getPeers() : NULL;

   if (sipRegistrar && peers)
   {
      {
         OsLock mutex(mLock);
         mTestState = Checking; // prevent timers from starting while we poll
      }
      /*
       * Do a single check of each uninitialized or unreachable peer.
       */
      RegistrarPeer* peer;
      while (   !isShuttingDown()
             && (peer = dynamic_cast<RegistrarPeer*>((*peers)()))
             )
      {
         RegistrarPeer::SynchronizationState state;

         state = peer->synchronizationState();
         if (   RegistrarPeer::Uninitialized == state
             || RegistrarPeer::UnReachable   == state
             )
         {
            OsSysLog::add( FAC_SIP, PRI_DEBUG, "RegistrarTest invoke SyncRpcReset(%s, %s)"
                          ,sipRegistrar->primaryName().data(), peer->name());

            peer->setState(SyncRpcReset::invoke(sipRegistrar->primaryName(), *peer));

            if (RegistrarPeer::Reachable == peer->synchronizationState())
            {
               OsSysLog::add( FAC_SIP, PRI_NOTICE,
                             "registerSync.reset success to '%s'; update numbering synchronized."
                             ,peer->name()
                             );
            }
         }
      }

      if ( !isShuttingDown() )
      {
         /*
          * If any are still unreachable after all are checked, then
          * the timer is scheduled to retry, using a standard limited
          * exponential backoff.
          */
         bool somePeerIsUnreachable = false;  // be optimistic
         peers->reset();
         OsLock mutex(mLock); // do not do any asynchronous operations holding the lock

         while (   (peer = dynamic_cast<RegistrarPeer*>((*peers)()))
                && !somePeerIsUnreachable // it only takes one, so don't keep checking
                )
         {
            if ( RegistrarPeer::UnReachable == peer->synchronizationState() )
            {
               somePeerIsUnreachable = true;
            }
         }

         if (somePeerIsUnreachable)
         {
            OsSysLog::add( FAC_SIP, PRI_INFO,
                          "RegistrarTest::checkPeers "
                          "- at least one peer is UnReachable, starting timer."
                          );

            restartTimer();
         }
         else // there are no UnReachable peers
         {
            OsSysLog::add( FAC_SIP, PRI_DEBUG,
                          "RegistrarTest::checkPeers - all peers Reachable."
                          );
            mTestState = Idle;
         }
      }

      delete peers;
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_CRIT,
                    "RegistrarTest::checkPeers no peers %p %p",
                    sipRegistrar, peers
                    );
   }
}

void RegistrarTest::requestShutdown(void)
{
   /*
    * This is called from the SipRegistrar task destructor.
    * Use the shutdown message to wake up the RegistrarTest task
    * to stop the timer before starting the shutdown of the task
    */
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "RegistrarTest::requestShutdown" );
   OsMsg msg(OsMsg::OS_SHUTDOWN, 0);

   postMessage(msg); // wake up the task by sending a message to it.
   yield(); // make the caller wait so that RegistrarTest can run.
}

// handle the expiration of the check timer
UtlBoolean RegistrarTest::handleMessage( OsMsg& eventMessage ///< Timer expiration msg
                                        )
{
   UtlBoolean handled = FALSE;

   int msgType    = eventMessage.getMsgType();
   int msgSubType = eventMessage.getMsgSubType();

   if (   OsMsg::OS_EVENT    == msgType
       && OsEventMsg::NOTIFY == msgSubType
       )
   {
      checkPeers();  // this aborts if isShuttingDown() is true
      handled = TRUE;
   }
   else if ( OsMsg::OS_SHUTDOWN == msgType )
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "RegistrarTest::handleMessage received shutdown message"
                    );
      {
         OsLock mutex(mLock);

         if ( TimerRunning == mTestState )
         {
            mRetryTimer.stop();
         }
      }
      OsTask::requestShutdown(); // tell OsServerTask::run to exit
      handled = TRUE;
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_CRIT,
                    "RegistrarTest::handleMessage received unexpected message %d/%d",
                    msgType, msgSubType
                    );
   }

   return handled;
}

/// See if we need to start a timer and do so if needed - caller must be holding mLock
void RegistrarTest::restartTimer()
{
   if (!isShuttingDown())
   {
      if ( mRetryTime == 0 || mTestState == Idle )
      {
         mRetryTime = REGISTER_TEST_INITIAL_WAIT;
      }
      else if ( mRetryTime < REGISTER_TEST_MAX_WAIT ) // has timer reached the backoff limit?
      {
         // no - so back off by doubling it
         mRetryTime *= 2;
      }

      // start the timer
      mTestState = TimerRunning;
      OsSysLog::add( FAC_SIP, PRI_DEBUG,
                    "RegistrarTest::restartTimer %zu"
                    ,mRetryTime
                    );

      mRetryTimer.oneshotAfter(OsTime(mRetryTime,0));
   }
}



/// destructor
RegistrarTest::~RegistrarTest()
{
   waitUntilShutDown();
}
