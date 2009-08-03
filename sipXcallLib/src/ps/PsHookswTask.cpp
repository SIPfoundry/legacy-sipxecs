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
#include "os/OsEventMsg.h"
#include "os/OsNotification.h"
#include "os/OsQueuedEvent.h"
#include "os/OsReadLock.h"
#include "os/OsTimer.h"
#include "os/OsWriteLock.h"
#include "ps/PsHookswTask.h"
#include "ps/PsPhoneTask.h"
#include "tao/TaoPhoneComponentAdaptor.h"

// EXTERNAL FUNCTIONS

// EXTERNAL VARIABLES
// Everything within the HOOKSW_TEMP_HACK defines should be removed when we
// fully switch over to using the new style of hook switch handling
#define HOOKSW_TEMP_HACK
#ifdef HOOKSW_TEMP_HACK
int oldStyleHooksw = 0;                   // 0 ==> optimized hooksw handling
                                          // 1 ==> old-style hooksw handling
#endif

// CONSTANTS
const int DEBOUNCE_TIMER_MSECS = 25;      // 25 milliseconds
const int SHORT_DEBOUNCE_MSECS = 100;     // 100 milliseconds
const int LONG_DEBOUNCE_MSECS  = 400;     // 400 milliseconds
#ifdef HOOKSW_TEMP_HACK
const int DEBOUNCE_MSECS = 20;            //  20 milliseconds
#endif

// STATIC VARIABLE INITIALIZATIONS
PsHookswTask* PsHookswTask::spInstance = 0;
OsBSem        PsHookswTask::sLock(OsBSem::Q_PRIORITY, OsBSem::FULL);

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Return a pointer to the Hooksw task, creating it if necessary
PsHookswTask* PsHookswTask::getHookswTask(void)
{
   UtlBoolean isStarted;

   // If the task object already exists, and the corresponding low-level task
   // has been started, then use it
   if (spInstance != NULL && spInstance->isStarted())
      return spInstance;

   // If the task does not yet exist or hasn't been started, then acquire
   // the lock to ensure that only one instance of the task is started
   sLock.acquire();
   if (spInstance == NULL)
       spInstance = new PsHookswTask();

   isStarted = spInstance->isStarted();
   if (!isStarted)
   {
      isStarted = spInstance->start();
      assert(isStarted);
   }
   sLock.release();

   return spInstance;
}

// Destructor
PsHookswTask::~PsHookswTask()
{
   delete mpHookswDev;
   delete mpTimer;

   spInstance = NULL;
}

/* ============================ MANIPULATORS ============================== */

// Create a hookswitch message and post it to the Hookswitch task.
// Return the result of the message send operation.
OsStatus PsHookswTask::postEvent(const int msg, void* source,
                                 const int hookswState,
                                 const OsTime& rTimeout)
{
   PsMsg    hookswMsg(msg, source, hookswState, 0);
   OsStatus res;

   res = postMessage(hookswMsg, rTimeout);
   return res;
}

/* ============================ ACCESSORS ================================= */

// Return the hookswitch state
const int PsHookswTask::getHookswitchState(void)
{
   OsReadLock lock(mMutex);        // acquire a read lock

   return mHookswState;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

// Default constructor (called only indirectly via getHookswTask())
PsHookswTask::PsHookswTask()
:  OsServerTask("PsHooksw"),
   mMutex(OsRWMutex::Q_PRIORITY),  // create mutex for protecting data
   mHookswState(ON_HOOK),          // initially, assume we are on hook
   mpHookswDev(NULL),              // not yet initialized
   mDebounceState(WAIT_FOR_INTR),  // initial debounce state
   mDebounceTicks(0),              // initial debounce tick count
   mDebounceHookswState(ON_HOOK),  // initial debounce hookswitch state
   mpTimer(NULL),                  // not yet initialized
   mpTimerEvent(NULL)              // not yet initialized
{
   // create the object used to manage the hookswitch device
   mpHookswDev = PsHookswDev::getHookswDev(this);

   // enable hookswitch interrupts
   mpHookswDev->enableIntr(TRUE);  // start by looking for offhook

   // create the timer that will be used to "debounce" the hookswitch
   mpTimer      = new OsTimer(&mIncomingQ, 0);
   mpTimerEvent = (OsQueuedEvent*) mpTimer->getNotifier();
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

// Handle an incoming message.
// Return TRUE if the message was handled, otherwise FALSE.
UtlBoolean PsHookswTask::handleMessage(OsMsg& rMsg)
{
   UtlBoolean processed;
   OsWriteLock lock(mMutex);            // acquire a write lock

   switch (rMsg.getMsgType())
   {
   case OsMsg::OS_EVENT:
      processed = handleEventMessage((OsEventMsg&) rMsg);
      break;
   case OsMsg::PS_MSG:
      processed = handlePhoneMessage((PsMsg&) rMsg);
      break;
   default:
      assert(FALSE);
      processed = FALSE;                // unexpected message type
      break;
   }

   return processed;
}

// Handle an incoming event message (timer expiration).
// Return TRUE if the message was handled, otherwise FALSE.
// A write lock should be acquired before calling this method.
UtlBoolean PsHookswTask::handleEventMessage(const OsEventMsg& rMsg)
{
   int          debounceInterval;
   int          hookswState;
   PsPhoneTask* pPhoneTask;
   UtlBoolean    processed;
   OsTimer*     pTimer;
   OsStatus     res;

   processed = TRUE;
   switch(rMsg.getMsgSubType())
   {
   case OsEventMsg::NOTIFY:
      rMsg.getEventData((intptr_t&) pTimer); // get timer object
      assert(pTimer == mpTimer);

      hookswState = readHwHookswState();// determine HW hook state
#ifdef HOOKSW_TEMP_HACK
      if (oldStyleHooksw)
      {
         if (hookswState == mHookswState)
         {                                 // no change since last time,
            if (mHookswState == ON_HOOK)   //  enable the hooksw interrupt
               mpHookswDev->enableIntr(TRUE);  //  look for off hook
            else
               mpHookswDev->enableIntr(FALSE); //  look for on hook
         }
         else
         {
            mHookswState = hookswState;    // update the hookswitch state

            // Send a hookswitch message to the phone set with the new state
            pPhoneTask = PsPhoneTask::getPhoneTask();
            res = pPhoneTask->postEvent(PsMsg::HOOKSW_STATE, // msg type
                                        this,            // source
                                        mHookswState,    // hooksw state
                                        0);              // not used
            assert(res == OS_SUCCESS);

            startDebounceTimer();          // rearm the debounce timer
         }
      }
      else
      {
#endif
      mDebounceTicks++;
      switch (mDebounceState)
      {
      case SHORT_DEBOUNCE:
      case LONG_DEBOUNCE:
         debounceInterval = ((mDebounceState == SHORT_DEBOUNCE)
                             ? SHORT_DEBOUNCE_MSECS : LONG_DEBOUNCE_MSECS);
         if (mDebounceHookswState != hookswState)  // is state still bouncing?
         {
            mDebounceHookswState = hookswState;    // stay in the current
            mDebounceTicks = 0;                    // state
         }

         if ((mDebounceTicks * DEBOUNCE_TIMER_MSECS) >= debounceInterval)
         {
            if (mDebounceHookswState != mHookswState)
            {
               mHookswState = hookswState;    // update the hookswitch state

               // Send a hookswitch message to the phone set with the new state
               pPhoneTask = PsPhoneTask::getPhoneTask();
               res = pPhoneTask->postEvent(PsMsg::HOOKSW_STATE, // msg type
                                           this,                // source
                                           mHookswState,        // hooksw state
                                           0);                  // not used
               assert(res == OS_SUCCESS);
               mDebounceState = ((mDebounceState == SHORT_DEBOUNCE)
                                 ? LONG_DEBOUNCE : WAIT_FOR_INTR);
               mDebounceTicks = 0;
            }
            else
            {
               mDebounceState = WAIT_FOR_INTR;// enable the hooksw interrupt
               mDebounceTicks = 0;
            }
         }
         break;
      case WAIT_FOR_INTR:                     // fall through
      default:
         assert(FALSE);
         break;
      }
      if (mDebounceState == WAIT_FOR_INTR)
      {
         if (mHookswState == ON_HOOK)       //  enable the hooksw interrupt
            mpHookswDev->enableIntr(TRUE);  //  look for off hook
         else
            mpHookswDev->enableIntr(FALSE); //  look for on hook
      }
      else
      {
         startDebounceTimer();
      }
#ifdef HOOKSW_TEMP_HACK
      }
#endif
      break;
   default:
      processed = FALSE;                // unexpected message subtype
      break;
   }

   return processed;
}

// Handle an incoming hookswitch message (HOOKSW_OFF or HOOKSW_ON).
// Return TRUE if the message was handled, otherwise FALSE.
// A write lock should be acquired before calling this method.
UtlBoolean PsHookswTask::handlePhoneMessage(const PsMsg& rMsg)
{
   PsPhoneTask* pPhoneTask;
   UtlBoolean    processed;
   OsStatus     res;
   int          state;

   state = rMsg.getParam1();
   assert(state == OFF_HOOK || state == ON_HOOK);

   processed = TRUE;
   switch (rMsg.getMsg())
   {
   case PsMsg::HOOKSW_STATE:
#ifdef HOOKSW_TEMP_HACK
    if (oldStyleHooksw)
    {
      if (state != mHookswState)
      {
         mHookswState = state;

                                    // post message to the phone set
         pPhoneTask = PsPhoneTask::getPhoneTask();
         res = pPhoneTask->postEvent(PsMsg::HOOKSW_STATE, // msg type
                                     this,                // source
                                     mHookswState,        // hooksw state
                                     0);                  // not used
         assert(res == OS_SUCCESS);

         startDebounceTimer();
      }
      else
      {
         // no hookswitch state change, just reenable the interrupt
         if (mHookswState == ON_HOOK)
            mpHookswDev->enableIntr(TRUE);  //  look for off hook
         else
            mpHookswDev->enableIntr(FALSE); //  look for on hook
      }
    }
    else
    {
#endif
      switch (mDebounceState)
      {
      case WAIT_FOR_INTR:
         // If the new hookswitch state differs from the old state,
         // then save the new state as mDebounceHookswState,
         // start the debounce timer and go to the SHORT_DEBOUNCE
         // debounce state.
         if (state != mHookswState)
         {
            mDebounceHookswState = state;
            mDebounceTicks = 0;
            mDebounceState = SHORT_DEBOUNCE;
            startDebounceTimer();
         }
         else
         {
            // no hookswitch state change, just reenable the interrupt
            if (mHookswState == ON_HOOK)
               mpHookswDev->enableIntr(TRUE);  //  look for off hook
            else
               mpHookswDev->enableIntr(FALSE); //  look for on hook
         }
         break;
      case SHORT_DEBOUNCE:
      case LONG_DEBOUNCE:
      default:
         // Hookswitch interrupts should be turned off while the phone
         // is in the SHORT_DEBOUNCE and LONG_DEBOUNCE states.  As a result,
         // we shouldn't be calling this routine while the phone is in these
         // states.
         assert(FALSE);
         break;
      }
#ifdef HOOKSW_TEMP_HACK
    }
#endif
      break;
   case PsMsg::HOOKSW_SET_STATE:
#ifdef HOOKSW_TEMP_HACK
   if (oldStyleHooksw)
   {
      if (state != mHookswState)
      {
         mHookswState = state;

                                    // post message to the phone set
         pPhoneTask = PsPhoneTask::getPhoneTask();
         res = pPhoneTask->postEvent(PsMsg::HOOKSW_STATE, // msg type
                                     this,                // source
                                     mHookswState,        // hooksw state
                                     0);                  // not used
         assert(res == OS_SUCCESS);

         startDebounceTimer();
      }
      else
      {
         // no hookswitch state change, just reenable the interrupt
         if (mHookswState == ON_HOOK)
            mpHookswDev->enableIntr(TRUE);  //  look for off hook
         else
            mpHookswDev->enableIntr(FALSE); //  look for on hook
      }
   }
   else
   {
#endif
      if (state != mHookswState)
      {
         mHookswState = state;

                                    // post message to the phone set
         pPhoneTask = PsPhoneTask::getPhoneTask();
         res = pPhoneTask->postEvent(PsMsg::HOOKSW_STATE, // msg type
                                     this,                // source
                                     mHookswState,        // hooksw state
                                     0);                  // not used
         assert(res == OS_SUCCESS);
      }
#ifdef HOOKSW_TEMP_HACK
    }
#endif
      break;
   case PsMsg::HOOKSW_GET_STATE:
      {
         // post message to the phone set
         TaoPhoneComponentAdaptor *pObj = (TaoPhoneComponentAdaptor*) rMsg.getMsgSource();

         TaoMessage *pMsg;
         pMsg = new TaoMessage(TaoMessage::COMPONENT_RESULT,
                               TaoMessage::COMPONENT_RESULT,
                               0,
                               0,
                               mHookswState,
                               0,
                               "");
         pObj->postMessage((OsMsg &) *pMsg);
         delete pMsg;
      }
      break;
   case PsMsg::HOOKSW_GET_CALL:
      // post message to the phone set
      pPhoneTask = PsPhoneTask::getPhoneTask();
      res = pPhoneTask->postEvent(PsMsg::HOOKSW_GET_CALL, // msg type
                                  this,                   // source
                                  mHookswState,           // hooksw state
                                  0);                     // not used
      assert(res == OS_SUCCESS);
      // startDebounceTimer();   // ???: why is this here???
      break;
   default:
      assert(FALSE);
      processed = FALSE;            // unexpected message
      break;
   }

   if (rMsg.getSentFromISR())       // indicate that we are done with the msg
      ((PsMsg&) rMsg).setInUse(FALSE);

   return processed;
}

// Start the debounce timer for the hookswitch
void PsHookswTask::startDebounceTimer(void)
{
   OsStatus res;
   OsTime   debounceTime(0, DEBOUNCE_TIMER_MSECS * 1000);
#ifdef HOOKSW_TEMP_HACK
   OsTime   debounceTime2(0, DEBOUNCE_MSECS * 1000);
#endif

   res = mpTimer->stop();           // just in case the timer is already
   assert(res == OS_SUCCESS);       //  enabled, stop it first.

#ifdef HOOKSW_TEMP_HACK
   if (oldStyleHooksw)
      res = mpTimer->oneshotAfter(debounceTime2);
   else
#endif
   res = mpTimer->oneshotAfter(debounceTime);
   assert(res == OS_SUCCESS);
}

// Read the hookswitch state from the hardware
int PsHookswTask::readHwHookswState(void)
{
   if (mpHookswDev->isOffHook())
      return OFF_HOOK;
   else
      return ON_HOOK;
}

#ifdef HOOKSW_TEMP_HACK
int setNewStyleHookswMode(void)
{
   int mode = oldStyleHooksw;

   oldStyleHooksw = 0;
   return mode;
}

int setOldStyleHookswMode(void)
{
   int mode = oldStyleHooksw;

   oldStyleHooksw = 1;
   return mode;
}
#endif
