//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsTimer_h_
#define _OsTimer_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsMsgQ.h"
#include "os/OsDateTime.h"
#include "os/OsNotification.h"
#include "os/OsStatus.h"
#include "os/OsTime.h"
#include "utl/UtlContainableAtomic.h"


// DEFINES

// Macro to check that 'x' is an OsTimer* by testing its
// getContainableType value.  This is to catch misuses of the OsTimer
// methods.
#define CHECK_VALIDITY(x) \
            assert((x)->getContainableType() == OsTimer::TYPE)

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * This class implements one-shot and periodic timers.
 *
 * Once a timer is created, it must be started.  After the specified time,
 * the timer expires or "fires", at which point (depending on how the
 * timer was created) an OsNotification object is used to signal an
 * event, or a message is posted to a specified queue.
 *
 * A timer may be stopped at any time (except when the timer is being
 * destroyed).  The destructor calls stop() before freeing the timer.
 *
 * If the stop() is synchronous, it may block, but it ensures that any
 * event routine call will have finished before stop() returns.  If
 * the stop() is asynchronous, it will not block, but an event routine
 * execution that has been previously committed may execute after stop()
 * returns.  (For one-shot timers, this can be detected by examining the
 * return value of stop().)
 *
 * Once a timer is stopped with stop() or by firing (if it is a one-shot
 * timer), it can be started again.  The time interval of a timer can be
 * changed every time it is started, but its notification information is
 * fixed when it is created.
 *
 * All methods can be used concurrently, except that no other method may be
 * called concurrently with the destructor (which cannot be made to work,
 * as the destructor deletes the timer's memory).  Note that a timer may
 * fire while it is being deleted; the destructor handles this situation
 * correctly, the timer is guaranteed to exist until after the event
 * routine returns.
 *
 * An event routine should be non-blocking, because it is called on
 * the timer task thread.  Within an event routine, all non-blocking
 * methods may be executed on the timer.  When the event routine of a
 * one-shot timer is entered, the timer is in the stopped state.  When
 * the event routine of a periodic timer is entered, the timer is
 * still in the running state.
 *
 * (If mbManagedNotifier is set, the timer may not be destroyed (using
 * deleteAsync, which is non-blocking), as that destroys the
 * OsNotifier object whose method is the event notifier that is
 * currently running.  But there is no current interface for creating
 * that situation.)
 *
 * Most methods are non-blocking, except to seize the timer's mutex
 * and to post messages to the timer task's message queue.  The
 * exceptions are the destructor and synchronous stops, which must
 * block until they get a response from the timer task.
 *
 * If VALGRIND_TIMER_ERROR is defined, additional code is created to
 * detect and backtrace errors in timer usage.  This code causes run-time
 * errors that Valgrind can detect to produce backtraces of where the
 * invalid method invocations were made.
 *
 * If NDEBUG is defined, some checking code that is used only to trigger
 * asserts is omitted.  (To prevent chaos when different libraries are
 * compiled with different options, defining NDEBUG does *not* change
 * the members of the objects.)
 *
 * @nosubgrouping
 */
class OsTimer : public UtlContainableAtomic
{
   friend class OsTimerTask;
   friend class OsTimerTest;

/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

   /// The states a timer can be in.
   enum OsTimerState
      {
         STOPPED,                /**< timer has not been started, or has fired
                                  *   or been stopped.
                                  */
         STARTED                 ///< timer is running and will fire.
      };

   /// type for absolute time in microseconds since the epoch
   typedef Int64 Time;
   /// type for time interval in microseconds
   typedef Int64 Interval;

/* ============================ CREATORS ================================== */

   /** @name Constructors
    *
    * Constructors specify how fired timers will signal the application.
    * The event specification does not change over the lifetime of the
    * timer.  The timer period information is specified by the start
    * method, and can be different for different starts.
    *
    * @{
    */

   /** Construct a timer that signals by calling
    *  @code
    *  rNotifier.signal((intptr_t) this)
    *  @endcode
    */
   OsTimer(OsNotification& rNotifier ///< OsNotification object to report event
      );

   /** Construct a timer that signals by calling
    *  @code
    *  pQueue->send(new OsEventMsg(OsEventMsg::NOTIFY, this, userData))
    *  @endcode
    */
   OsTimer(OsMsgQ* pQueue,      ///< Queue to send OsEventMsg::NOTIFY message
           void* userData       ///< userData value to store in OsQueuedEvent
      );

   /** Construct a timer that signals by calling
    *  @code
    *  pQueue->send(*pMsg)      // Note that pQueue->send() copies *pMsg.
    *  @endcode
    */
   OsTimer(OsMsg* pMsg,         ///< Message to send.
           OsMsgQ* pQueue       ///< Queue to send message to.
      );
   /* The OsTimer takes ownersip of *pMsg.
    * Note that the order of the arguments is opposite of the preceeding
    * constructor, to avoid ambiguity in calls.
    * When the OsTimer fires, a copy of *pMsg is queued to *pQueue.
    * (The copy is made using pMsg->createCopy().)
    */

   /// @}

   /// Destructor
   virtual ~OsTimer();

   /// Non-blocking asynchronous delete operation
   static void deleteAsync(OsTimer* timer);
   /**<
    * Stops the timer, then sends a message to the timer task, which will
    * eventually delete it.  Provides a non-blocking way to delete an
    * OsTimer.
    */

/* ============================ MANIPULATORS ============================== */

   /** @name Start methods
    *
    * These methods start the timer.  They may be called when the timer is
    * in any state, but if the timer is already started, they have no
    * effect. They return a value that reflects that state:  OS_SUCCESS if
    * the start operation was successful and OS_FAILED if it failed
    * (because the timer was already started).
    *
    * @{
    */

   /// Start the timer to fire once at the indicated date/time
   virtual OsStatus oneshotAt(const OsDateTime& when);

   /// Start the timer to fire once at the current time + offset
   virtual OsStatus oneshotAfter(const OsTime& offset);

   /// Start the timer to fire periodically starting at the indicated date/time
   virtual OsStatus periodicAt(const OsDateTime& when, OsTime period);

   /// Start the timer to fire periodically starting at current time + offset
   virtual OsStatus periodicEvery(OsTime offset, OsTime period);

   /// @}

   /// Stop the timer if it has been started
   virtual OsStatus stop(UtlBoolean synchronous = TRUE);
   /**<
    * stop() can be called when the timer is in any state, and returns a
    * value that reflects that state:
    *
    * @returns OS_SUCCESS if the timer was started and OS_FAILED if the
    * timer was not started, was already stopped, or is a one-shot
    * timer and has fired.
    *
    * Thus, if it is a one-shot timer, and there are one or more calls to
    * stop(), if the event has been signaled, all calls will return
    * OS_FAILED.  But if the event has not been signaled, exactly one call
    * will return OS_SUCCESS.  This allows the caller of stop() to know
    * whether to clean up or not.
    *
    * If synchronous is TRUE, the call will block if necessary to
    * ensure that any event routine execution for this timer will
    * finish before stop() returns.  If synchronous is FALSE, the call
    * will not block, but a previously committed event routine
    * execution may happen after stop() returns.
    */

/* ============================ ACCESSORS ================================= */

   /// Return the OsNotification object for this timer
   virtual OsNotification* getNotifier(void) const;
   /**<
    * If the timer was constructed with OsTimer(OsMsgQ*, const int),
    * it returns the address of an internally allocated OsNotification.
    */

   /// Get the userData value of a timer constructed with OsTimer(OsMsgQ*, int).
   virtual void* getUserData();

   /// Get the ContainableType for a UtlContainable derived class.
   virtual UtlContainableType getContainableType() const;

/* ============================ INQUIRY =================================== */

   /// Compare the this object to another like-object.
   virtual int compareTo(UtlContainable const *) const;
   /**
    * Results for comparing with a non-like object are undefined.
    *
    * @returns 0 if equal, < 0 if less-than and > 0 if greater-than.
    */

   /// Return the state value for this OsTimer object
   virtual OsTimerState getState(void);

   /// Return all the state information for this OsTimer object.
   virtual void getFullState(enum OsTimerState& state,
                             Time& expiresAt,
                             UtlBoolean& periodic,
                             Interval& period);

   /// Get the current time as a Time.
   static Time now();

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:

   static const UtlContainableType TYPE;
   /**< Class type used for runtime checking */

   OsBSem          mBSem;      //< semaphore to lock access to members

   unsigned int    mApplicationState;
   //< state as seen by application methods
   unsigned int    mTaskState; //< state as seen by the timer task
   UtlBoolean      mDeleting;  //< TRUE if timer is being deleted
   UtlBoolean      mProcessingInProgress;  //< TRUE if OsTimerTask is currently processing a message related to this timer
   OsNotification* mpNotifier; //< used to signal timer expiration event
   UtlBoolean      mbManagedNotifier;
   /**< TRUE if OsTimer destructor
    *   should delete *mpNotifier
    */

   Time            mExpiresAt; //< expire time of timer
   UtlBoolean      mPeriodic;  //< TRUE if timer fires repetitively
   Interval        mPeriod;    //< repetition time

   /// Copies of time values for use by timer task.
   Time            mQueuedExpiresAt; //< expire time of timer
   UtlBoolean      mQueuedPeriodic;  //< TRUE if timer fires repetitively
   Interval        mQueuedPeriod;    //< repetition time

   int             mOutstandingMessages;
   /**< number of messages for this timer in
    *   the timer task's queue
    */

   OsTimer*        mTimerQueueLink;
   ///< to chain together timers

   UtlBoolean      mFiring;
   ///< True while a timer is being fired by OsTimerTask.

   /// The null value for the type Interval.
   static const Interval nullInterval;
   ///< nullInterval may not be used in arithmetic operations.

   /// Start a timer.
   OsStatus startTimer(Time start,
                       UtlBoolean periodic,
                       Interval period);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

   /// Copy constructor (not implemented for this class)
   OsTimer(const OsTimer& rOsTimer);

   /// Assignment operator (not implemented for this class)
   OsTimer& operator=(const OsTimer& rhs);

/* ============================ INLINE METHODS ============================ */
  protected:

   /// Test whether a status indicates the timer has been started.
   inline static UtlBoolean isStarted(int status)
   {
      return (status & 1) == 1;
   }

   /// Test whether a status indicates the timer has been stopped.
   inline static UtlBoolean isStopped(int status)
   {
      return (status & 1) == 0;
   }

   /// Convert an OsDateTime into a Time.
   inline static Time cvtToTime(const OsDateTime& t)
   {
      OsTime t_os;
      t.cvtToTimeSinceEpoch(t_os);
      return (Time)(t_os.seconds()) * 1000000 + t_os.usecs();
   }

   /// Convert an OsTime into an Interval.
   inline static Interval cvtToInterval(const OsTime& t)
   {
      return (Interval)(t.seconds()) * 1000000 + t.usecs();
   }

   /// Convert a Timer into an OsTime.
   inline static void cvtToOsTime(OsTime& out, Time in)
   {
      OsTime temp(in / 1000000, in % 1000000);
      out = temp;
   }

   /// Add an Interval to a Time.
   inline static Time addInterval(Time a, Interval b)
   {
      return a + b;
   }

   /// Take the difference of two Time's.
   inline static Time subtractTimes(Time a, Time b)
   {
      return a - b;
   }

   /// Compare two Time's.
   inline static int compareTimes(Time a, Time b)
   {
      // We can't use the usual trick of returning (a - b), because
      // that difference is a long long int, but the return type
      // of compareTimes is int, and truncating may change the sign
      // of the value.
      Time difference = a - b;
      return
         difference < 0 ? -1 :
         difference == 0 ? 0 :
         1;
   }
   ///< Returns > 0, 0, or < 0 according to whether a > b, a == b, or a < b.

};

#endif  // _OsTimer_h_
