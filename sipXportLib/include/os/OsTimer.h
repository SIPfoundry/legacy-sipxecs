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
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/system/error_code.hpp>

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
 * stop() is asynchronous, it will not block, but an event routine
 * execution that has been previously committed may execute after stop()
 * returns.  
 *
 * Once a timer is stopped with stop() or by firing (if it is a one-shot
 * timer), it can be started again.  The time interval of a timer can be
 * changed every time it is started, but its notification information is
 * fixed when it is created.
 *
 * All methods can be used concurrently.  Note that a timer may
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
 *
 * @nosubgrouping
 */
class OsTimer : public UtlContainableAtomic
{
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

   /// type for timer boost function callback
   typedef boost::function<void(OsTimer&, boost::system::error_code)> Handler;

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

   OsTimer(const Handler& handler);
   /// This constructor accepts a handler function instead of message queue.
   /// One must take caution in using this facility to make sure that the handler
   /// function will not block becuase it is directly called from the
   /// io_service thread;

   /// @}

   /// Destructor
   virtual ~OsTimer();


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
   OsStatus oneshotAt(const OsDateTime& when);

   /// Start the timer to fire once at the current time + offset
   OsStatus oneshotAfter(const OsTime& offset);


   /// Start the timer to fire periodically starting at the indicated date/time
   OsStatus periodicAt(const OsDateTime& when, OsTime period);

   /// Start the timer to fire periodically starting at current time + offset
   OsStatus periodicEvery(OsTime offset, OsTime period);

   /// Start the timer to fire once at the current time + offset
   OsStatus oneshotAfter(const boost::asio::deadline_timer::duration_type& offset);

   /// Start the timer to fire periodically starting at current time + offset
   OsStatus periodicEvery(const boost::asio::deadline_timer::duration_type& offset, 
    const boost::asio::deadline_timer::duration_type& period);

   /// Perform a blocking wait on the timer.
   /// Returns error code if any.
   static boost::system::error_code wait(const boost::asio::deadline_timer::duration_type& offset);


   /// @}

   /// Stop the timer if it has been started
   OsStatus stop(UtlBoolean synchronous = TRUE);
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
    * All calls to stop are synchronous.  The synchronous flag is no longer
    * used after migration to boost deadline timer.
    *
    */

/* ============================ ACCESSORS ================================= */

   /// Return the OsNotification object for this timer
   OsNotification* getNotifier(void) const;
   /**<
    * If the timer was constructed with OsTimer(OsMsgQ*, const int),
    * it returns the address of an internally allocated OsNotification.
    */

   /// Get the userData value of a timer constructed with OsTimer(OsMsgQ*, int).
   void* getUserData();

   /// Get the ContainableType for a UtlContainable derived class.
   UtlContainableType getContainableType() const;

/* ============================ INQUIRY =================================== */

   /// Compare the this object to another like-object.
   int compareTo(UtlContainable const *) const;
   /**
    * Results for comparing with a non-like object are undefined.
    *
    * @returns 0 if equal, < 0 if less-than and > 0 if greater-than.
    */

   /// Return the state value for this OsTimer object
   OsTimerState getState(void);

   /// Return all the state information for this OsTimer object.
   void getFullState(enum OsTimerState& state,
                             Time& expiresAt,
                             UtlBoolean& periodic,
                             Interval& period);

   /// Get the current time as a Time.
   static Time now();

   /// Terminate the internal timer service thread and cancel all existing timers.
   /// This is normally called prior to program termination.
   /// If the timer service is terminated, it cannot be restarted.
   static void terminateTimerService();
   //NOTE: It's safer to call this after all timers were stopped and destroyed.
   // If this is called while timers are still active it will leak.

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:

   static const UtlContainableType TYPE;
   /**< Class type used for runtime checking */

  

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

   /// Copy constructor (not implemented for this class)
   OsTimer(const OsTimer& rOsTimer);

   /// Assignment operator (not implemented for this class)
   OsTimer& operator=(const OsTimer& rhs);


  protected:

    bool signalNotifier();

    bool signalHandler(boost::system::error_code ec);

    class Timer : public boost::enable_shared_from_this<OsTimer::Timer>
    {
    public:
      typedef boost::shared_ptr<Timer> Ptr;
      typedef boost::mutex mutex;
      typedef boost::lock_guard<mutex> mutex_lock;
      Timer(OsTimer& owner);
      ~Timer();
      void onTimerFire(const boost::system::error_code& e, OsTimer* pOwner);
      /// Start the timer to fire once at the indicated date/time
      bool oneshotAt(const OsDateTime& when);

      /// Start the timer to fire once at the current time + offset
      bool oneshotAfter(const OsTime& offset);

      /// Start the timer to fire periodically starting at the indicated date/time
      bool periodicAt(const OsDateTime& when, OsTime period);

      /// Start the timer to fire periodically starting at current time + offset
      bool periodicEvery(OsTime offset, OsTime period);

      /// Start the timer to fire once at the current time + offset
      bool oneshotAfter(const boost::asio::deadline_timer::duration_type& offset);

      /// Start the timer to fire periodically starting at current time + offset
      bool periodicEvery(const boost::asio::deadline_timer::duration_type& offset,
        const boost::asio::deadline_timer::duration_type& period);

      void cancel();
      
      bool isRunning();

      void clearPeriodic();

      void takeOwnership(OsNotification* pNotifier);


    public:
      OsTimer& _owner;
      boost::asio::deadline_timer* _pDeadline;
      Time _expiresAt;
      bool _periodic;
      Interval _period;
      bool _isRunning;
      mutex _mutex;
      OsNotification* _pNotifier;
    };

    
    friend class OsTimer::Timer;
    friend class TimerService;
    Timer::Ptr _pTimer;
    OsNotification* _pNotifier; //< used to signal timer expiration event
    Handler _handler;

};

//
// Inlines
//


/// Start the timer to fire once at the indicated date/time
inline OsStatus OsTimer::oneshotAt(const OsDateTime& when)
{
  return _pTimer->oneshotAt(when) ? OS_SUCCESS : OS_FAILED;
}

/// Start the timer to fire once at the current time + offset
inline OsStatus OsTimer::oneshotAfter(const OsTime& offset)
{
  return _pTimer->oneshotAfter(offset) ? OS_SUCCESS : OS_FAILED;
}

/// Start the timer to fire periodically starting at the indicated date/time
inline OsStatus OsTimer::periodicAt(const OsDateTime& when, OsTime period)
{
  return _pTimer->periodicAt(when, period) ? OS_SUCCESS : OS_FAILED;
}

/// Start the timer to fire periodically starting at current time + offset
inline OsStatus OsTimer::periodicEvery(OsTime offset, OsTime period)
{
  return _pTimer->periodicEvery(offset, period) ? OS_SUCCESS : OS_FAILED;
}

/// Start the timer to fire once at the current time + offset
inline OsStatus OsTimer::oneshotAfter(const boost::asio::deadline_timer::duration_type& offset)
{
  return _pTimer->oneshotAfter(offset) ? OS_SUCCESS : OS_FAILED;
}

/// Start the timer to fire periodically starting at current time + offset
inline OsStatus OsTimer::periodicEvery(const boost::asio::deadline_timer::duration_type& offset,
    const boost::asio::deadline_timer::duration_type& period)
{
  return _pTimer->periodicEvery(offset, period) ? OS_SUCCESS : OS_FAILED;
}

inline UtlContainableType OsTimer::getContainableType() const
{
    return OsTimer::TYPE;
}

// Return the OsNotification object for this timer
inline OsNotification* OsTimer::getNotifier(void) const
{
  return _pNotifier;
}

inline void OsTimer::Timer::takeOwnership(OsNotification* pNotifier)
{
	_pNotifier = pNotifier;
}

#endif  // _OsTimer_h_
