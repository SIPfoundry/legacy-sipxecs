//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _REGISTRARTEST_H_
#define _REGISTRARTEST_H_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsTimer.h"
#include "os/OsServerTask.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS


/**
 * This thread is responsible for periodically attempting
 * to re-establish contact with an UnReachable peer.
 */
class RegistrarTest : public OsServerTask
{
  public:

   // constructor
   RegistrarTest(SipRegistrar& registrar);

   /// destructor
   virtual ~RegistrarTest();

   /// Call from another task to stop the RegistrarTest task.
   virtual void requestShutdown(void);

   /// Override default run method to do one initial check before waiting for timers
   int run(void* pArg);

   /// handle the expiration of the check timer
   UtlBoolean handleMessage( OsMsg& eventMessage ///< Timer expiration msg
                            );

   /// Initiate checking of any unreachable peers.
   void check();
   /**<
    * If there is no currently running timer, start one to initiate
    * reachability checks (if already running, this is a no-op).
    */

  protected:
   /// do the exponential.backoff and start timer
   void restartTimer();

   /// Check each unreachable peer.
   void checkPeers();
   /**<
    * Does a single check of each unreachable peer.
    * If any are still unreachable after all are checked, then
    * the timer is scheduled to retry, using a standard limited
    * exponential backoff.
    */

  private:

   /// mutex must be locked with OsLock to access any other member variable.
   OsBSem mLock;

   /// State of the RegistrarTest thread
   enum TestState
   {
      StartupPhase, ///< In this state, the timer is not started for UnReachable peers
      TimerRunning, ///< The timer is already running, no need to start it
      Checking,     ///< The checkPeers method is checking peers; do not start the timer
      Idle          ///< All peers were reachable, this thread is sleeping, so start the timer
   } mTestState;

   /// The retry timer.
   OsTimer mRetryTimer;

   /// Current value of the retry timer.
   size_t mRetryTime;
   /**
    * Initialized to REGISTER_TEST_INITIAL_WAIT.
    *
    * If after a round of checks,
    * - all peers are Reachable
    *   Reset to REGISTER_TEST_INITIAL_WAIT.
    * - some peer is UnReachable
    *   Set to MIN( mRetryTime * 2, REGISTER_TEST_MAX_WAIT )
    *   and schedule a timer for this new time.
    */

   /// The top level task - do not delete;
   SipRegistrar& mSipRegistrar;

   /// There is no copy constructor.
   RegistrarTest(const RegistrarTest&);

   /// There is no assignment operator.
   RegistrarTest& operator=(const RegistrarTest&);

};

#endif // _REGISTRARTEST_H_
