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
#include "os/OsTimer.h"
#include "os/OsTimerTask.h"
#include "os/OsQueuedEvent.h"
#include "os/OsLock.h"
#include "os/OsEvent.h"
#include "os/OsTimerMsg.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// STATIC VARIABLE INITIALIZATIONS
const UtlContainableType OsTimer::TYPE = "OsTimer";
const OsTimer::Interval OsTimer::nullInterval = 0;

#ifdef VALGRIND_TIMER_ERROR
// Dummy static variable to receive values from tracking variables.
static char dummy;
#endif /* VALGRIND_TIMER_ERROR */

/// Subclass of OsNotification that queues a copy of a message.
class OsQueueMsgNotification : public OsNotification
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   /* ============================ CREATORS ================================== */

   OsQueueMsgNotification(OsMsgQ* pQueue, ///< Queue to send message to.
                          OsMsg* pMsg     ///< Message to send.
      ) :
      mpQueue(pQueue),
      mpMsg(pMsg)
      {
      }
   /* The OsQueueMsgNotification takes ownersip of *pMsg. */

   virtual
   ~OsQueueMsgNotification()
      {
         // Free *mpMsg, which this OsQueueMsgNotification owns.
         delete mpMsg;
      }
   //:Destructor

   /* ============================ MANIPULATORS ============================== */

   //:Signal the occurrence of the event
   virtual OsStatus signal(intptr_t eventData)
      {
         // Signaling is done by queueing mpMsg->createCopy() to *mpQueue.
         return mpQueue->send(*mpMsg->createCopy());
      }

   /* ============================ ACCESSORS ================================= */

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   OsQueueMsgNotification(const OsQueueMsgNotification& rOsQueueMsgNotification);
   //:Copy constructor (not implemented for this class)

   OsQueueMsgNotification& operator=(const OsQueueMsgNotification& rhs);
   //:Assignment operator (not implemented for this class)

   // The message queue to which to send the message.
   OsMsgQ* mpQueue;

   // The message to be copied and sent.
   OsMsg* mpMsg;

};

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructors

// Timer expiration event notification happens using the
// newly created OsQueuedEvent object
OsTimer::OsTimer(OsMsgQ* pQueue,
                 void* userData) :
   mBSem(OsBSem::Q_PRIORITY, OsBSem::FULL),
   mApplicationState(0),
   mTaskState(0),
   // Always initialize mDeleting, as we may print its value.
   mDeleting(FALSE),
   mProcessingInProgress(FALSE),
   mpNotifier(new OsQueuedEvent(*pQueue, userData)),
   mbManagedNotifier(TRUE),
   mOutstandingMessages(0),
   mTimerQueueLink(0),
   mFiring(FALSE)
{
#ifdef VALGRIND_TIMER_ERROR
   // Initialize the variables for tracking timer access.
   mLastStartBacktrace = NULL;
   mLastDestructorBacktrace = NULL;
#endif /* VALGRIND_TIMER_ERROR */
}

// The address of "this" OsTimer object is the eventData that is
// conveyed to the Listener when the notification is signaled.
OsTimer::OsTimer(OsNotification& rNotifier) :
   mBSem(OsBSem::Q_PRIORITY, OsBSem::FULL),
   mApplicationState(0),
   mTaskState(0),
   // Always initialize mDeleting, as we may print its value.
   mDeleting(FALSE),
   mProcessingInProgress(FALSE),
   mpNotifier(&rNotifier),
   mbManagedNotifier(FALSE),
   mOutstandingMessages(0),
   mTimerQueueLink(0),
   mFiring(FALSE)
{
#ifdef VALGRIND_TIMER_ERROR
   // Initialize the variables for tracking timer access.
   mLastStartBacktrace = NULL;
   mLastDestructorBacktrace = NULL;
#endif /* VALGRIND_TIMER_ERROR */
}

// Timer expiration event notification is done by queueing a copy of
// *pMsg to *pQueue.
OsTimer::OsTimer(OsMsg* pMsg,
                 OsMsgQ* pQueue) :
   mBSem(OsBSem::Q_PRIORITY, OsBSem::FULL),
   mApplicationState(0),
   mTaskState(0),
   // Always initialize mDeleting, as we may print its value.
   mDeleting(FALSE),
   mProcessingInProgress(FALSE),
   // *mpNotifier owns the new OsQueueMsgNotification, which owns *pMsg.
   mpNotifier(new OsQueueMsgNotification(pQueue, pMsg)),
   mbManagedNotifier(TRUE),
   mOutstandingMessages(0),
   mTimerQueueLink(0),
   mFiring(FALSE)
{
#ifdef VALGRIND_TIMER_ERROR
   // Initialize the variables for tracking timer access.
   mLastStartBacktrace = NULL;
   mLastDestructorBacktrace = NULL;
#endif /* VALGRIND_TIMER_ERROR */
}

// Destructor
OsTimer::~OsTimer()
{
#ifndef NDEBUG
   CHECK_VALIDITY(this);
#endif

   // Update members and determine whether we need to send an UPDATE_SYNC
   // to stop the timer or ensure that the timer task has no queued message
   // about this timer.
   UtlBoolean sendMessage = FALSE;
   {
      OsLock lock(mBSem);

#ifndef NDEBUG
      assert(!mDeleting);
      // Lock out all further application methods.
      mDeleting = TRUE;
#endif

      // Check if the timer needs to be stopped.
      if (isStarted(mApplicationState)) {
         sendMessage = TRUE;
         mApplicationState++;
      }
      if( mOutstandingMessages > 0 || mProcessingInProgress || mFiring )
      {
         sendMessage = TRUE;
      }

      // If we have to send a message, make note of it.
      if (sendMessage)
      {
         mOutstandingMessages++;
      }
   }

   // Send a message to the timer task if we need to.
   if (sendMessage) {
      OsEvent event;
      OsTimerMsg msg(OsTimerMsg::UPDATE_SYNC, this, &event);
      OsStatus res = OsTimerTask::getTimerTask()->postMessage(msg);
      assert(res == OS_SUCCESS);
      event.wait();
   }

   // If mbManagedNotifier, free *mpNotifier.
   if (mbManagedNotifier) {
      delete mpNotifier;
   }
}

// Non-blocking asynchronous delete operation
void OsTimer::deleteAsync(OsTimer* timer)
{
#ifndef NDEBUG
   CHECK_VALIDITY(timer);
#endif

   // Update members.
   {
      OsLock lock(timer->mBSem);

#ifndef NDEBUG
      assert(!timer->mDeleting);
      // Lock out all further application methods.
      timer->mDeleting = TRUE;
#endif

      // Check if the timer needs to be stopped.
      if (isStarted(timer->mApplicationState))
      {
         timer->mApplicationState++;
      }

      // Note we will be sending a message.
      timer->mOutstandingMessages++;
   }

   // Send the message.
   OsTimerMsg msg(OsTimerMsg::UPDATE_DELETE, timer, NULL);
   OsStatus res = OsTimerTask::getTimerTask()->postMessage(msg);
   assert(res == OS_SUCCESS);
}

/* ============================ MANIPULATORS ============================== */

// Arm the timer to fire once at the indicated date and time
OsStatus OsTimer::oneshotAt(const OsDateTime& when)
{
   return startTimer(cvtToTime(when), FALSE, nullInterval);
}

// Arm the timer to fire once at the current time + offset
OsStatus OsTimer::oneshotAfter(const OsTime& offset)
{
   return startTimer(now() + cvtToInterval(offset), FALSE, nullInterval);
}

// Arm the timer to fire periodically starting at the indicated date/time
OsStatus OsTimer::periodicAt(const OsDateTime& when, OsTime period)
{
   return startTimer(cvtToTime(when), TRUE, cvtToInterval(period));
}

// Arm the timer to fire periodically starting at current time + offset
OsStatus OsTimer::periodicEvery(OsTime offset, OsTime period)
{
   return startTimer(now() + cvtToInterval(offset), TRUE,
                     cvtToInterval(period));
}

// Disarm the timer
OsStatus OsTimer::stop(UtlBoolean synchronous)
{
#ifndef NDEBUG
   CHECK_VALIDITY(this);
#endif

   OsStatus result;
   UtlBoolean sendMessage = FALSE;

   // Update members.
   {
      OsLock lock(mBSem);

#ifndef NDEBUG
      assert(!mDeleting);
#endif

      // Determine whether the call is successful.
      if (isStarted(mApplicationState))
      {
         // Update state to stopped.
         mApplicationState++;
         result = OS_SUCCESS;
         if (mOutstandingMessages == 0)
         {
            // We will send a message.
            sendMessage = TRUE;
            mOutstandingMessages++;
         }
      }
      else
      {
         result = OS_FAILED;
      }
   }

   // If we need to, send an UPDATE message to the timer task.
   if (sendMessage)
   {
      if (synchronous) {
         // Send message and wait.
         OsEvent event;
         OsTimerMsg msg(OsTimerMsg::UPDATE_SYNC, this, &event);
         OsStatus res = OsTimerTask::getTimerTask()->postMessage(msg);
         assert(res == OS_SUCCESS);
         event.wait();
      }
      else
      {
         // Send message.
         OsTimerMsg msg(OsTimerMsg::UPDATE, this, NULL);
         OsStatus res = OsTimerTask::getTimerTask()->postMessage(msg);
         assert(res == OS_SUCCESS);
      }
   }

   return result;
}

/* ============================ ACCESSORS ================================= */

// Return the OsNotification object for this timer
OsNotification* OsTimer::getNotifier(void) const
{
   return mpNotifier;
}

// Get the userData value of a timer constructed with OsTimer(OsMsgQ*, int).
void* OsTimer::getUserData()
{
   // Have to cast mpNotifier into OsQueuedEvent* to get the userData.
   OsQueuedEvent* e = dynamic_cast <OsQueuedEvent*> (mpNotifier);
   assert(e != 0);
   void* userData;
   e->getUserData(userData);
   return userData;
}

UtlContainableType OsTimer::getContainableType() const
{
    return OsTimer::TYPE;
}

/* ============================ INQUIRY =================================== */

// Return the state value for this OsTimer object
OsTimer::OsTimerState OsTimer::getState(void)
{
   OsLock lock(mBSem);
   return isStarted(mApplicationState) ? STARTED : STOPPED;
}

// Return all the state information for this OsTimer object.
void OsTimer::getFullState(enum OsTimerState& state,
                           Time& expiresAt,
                           UtlBoolean& periodic,
                           Interval& period)
{
   OsLock lock(mBSem);

   state = isStarted(mApplicationState) ? STARTED : STOPPED;
   expiresAt = mExpiresAt;
   periodic = mPeriodic;
   period = mPeriod;
}

int OsTimer::compareTo(UtlContainable const * inVal) const
{
   int result;

   if (inVal->isInstanceOf(OsTimer::TYPE))
   {
      result = comparePtrs(this, inVal);
   }
   else
   {
      result = -1;
   }

   return result;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

// Get the current time as a Time.
OsTimer::Time OsTimer::now()
{
   OsTime t;
   OsDateTime::getCurTime(t);
   return (Time)(t.seconds()) * 1000000 + t.usecs();
}

// Start the OsTimer object.
OsStatus OsTimer::startTimer(Time start,
                             UtlBoolean periodic,
                             Interval period)
{
#ifndef NDEBUG
   CHECK_VALIDITY(this);
#endif

   OsStatus result;
   UtlBoolean sendMessage = FALSE;

   // Update members.
   {
      OsLock lock(mBSem);

#ifndef NDEBUG
      assert(!mDeleting);
#endif

      // Determine whether the call is successful.
      if (isStopped(mApplicationState))
      {
         // Update state to started.
         mApplicationState++;
         result = OS_SUCCESS;
         if (mOutstandingMessages == 0)
         {
            // We will send a message.
            sendMessage = TRUE;
            mOutstandingMessages++;
         }
         // Set time values.
         mExpiresAt = start;
         mPeriodic = periodic;
         mPeriod = period;
      }
      else
      {
         result = OS_FAILED;
      }
   }

   // If we need to, send an UPDATE message to the timer task.
   if (sendMessage)
   {
      OsTimerMsg msg(OsTimerMsg::UPDATE, this, NULL);
      OsStatus res = OsTimerTask::getTimerTask()->postMessage(msg);
      assert(res == OS_SUCCESS);
   }

   return result;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
