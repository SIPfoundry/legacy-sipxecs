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
#include <queue>

// APPLICATION INCLUDES
#include "os/OsTimer.h"
#include "os/OsQueuedEvent.h"
#include "os/OsEvent.h"
#include "os/OsLogger.h"



/* //////////////////////////// PUBLIC //////////////////////////////////// */
static const int TIMER_TIME_UNIT = 1000000;
static const int TIMER_SERVICE_TICK = 1;
#define TIME_TO_INTERVAL(period) (Interval)(period.seconds()) * TIMER_TIME_UNIT + period.usecs()

// Global counter for number of active timers at a given moment. Having active timers
// will prevent the terminateTimerService from actually stopping the service and corrupt memory.
// TODO: This could be done better: the timer service should have a way to signal all timers to stop.
static long int gTimersNum = 0;
// Mutex protecting the timer service
static boost::mutex gTimerServiceMutex;

class TimerService
{
  //
  // The is a wrapper class for a single thread boost::asio::io_service.
  // Timers require an io_service thread to poll for timer signals.
  // When this gets instantiated, a thread will be created and a
  // dummy recurring event _houseKeepingTimer is created to make sure
  // that at least one event is always pending in the io_service.
  // This will keep the io_service thread from exing.  The default behavior of
  // the io_service thread is to terminate when there no longer any pending
  // task in the queue.  
  //
public:
  typedef boost::mutex mutex;
  typedef boost::lock_guard<mutex> mutex_lock;

  TimerService() :
    _ioService(), // The io service
    _houseKeepingTimer(_ioService, boost::posix_time::seconds(10)), // dummy timer just to keep on event in queue.  will fire every hour
    _pIoServiceThread1(0), // The thread pointer
    _pIoServiceThread2(0), // The thread pointer
    _lastTick(0)
  {
  }

  ~TimerService()
  {
    stop();
  }

  void start()
  {
    //
    // Create the dummy house keeper and start the io_service thread
    //
    _houseKeepingTimer.async_wait(boost::bind(&TimerService::onHouseKeepingTimer, this, boost::asio::placeholders::error));
    _pIoServiceThread1 = new boost::thread(boost::bind(&boost::asio::io_service::run, &(_ioService)));
    _pIoServiceThread2 = new boost::thread(boost::bind(&boost::asio::io_service::run, &(_ioService)));
    OS_LOG_NOTICE(FAC_KERNEL, "OsTimer::TimerService STARTED.");
    _lastTick = OsTimer::now();
  }
private:
  void stop()
  {
    //
    // Stop the io_service.  This will cancel all timer events.
    //
    _ioService.stop();


    //
    // Wait until io_service has fully terminated
    //
    if (_pIoServiceThread1)
    {
      _pIoServiceThread1->join();
      delete _pIoServiceThread1;
      _pIoServiceThread1 = 0;
    }

    if (_pIoServiceThread2)
    {
      _pIoServiceThread2->join();
      delete _pIoServiceThread2;
      _pIoServiceThread2 = 0;
    }
  }

public:
  void onHouseKeepingTimer(const boost::system::error_code& e)
  {
    static int iteration = 0;
    //
    // This will fire every hour.  We will simply restart the timer
    // so io_serice thread does not exit.  If the operation is aborted
    // it would mean the io service has been terminated.
    //
    if (e != boost::asio::error::operation_aborted)
    {
      {
        mutex_lock lock(_queueMutex);
        OsTimer::Time now = OsTimer::now();
        OsTimer::Interval skew = now - _lastTick - (TIMER_SERVICE_TICK * TIMER_TIME_UNIT);
        _lastTick = now;

        if (iteration++ >= 10)
        {
          if (skew < TIMER_TIME_UNIT)
          {
            OS_LOG_DEBUG(FAC_KERNEL, "OsTimer::TimerService timer resolution: " << skew << " microseconds.");
          }
          else
          {
            OS_LOG_WARNING(FAC_KERNEL, "OsTimer::TimerService timer resolution: " << skew << " microseconds.");
          }
          iteration = 0;
        }

        //
        // Destroy timers that are queued back to us.
        //
        while (_timerQueue.size() > 0)
          _timerQueue.pop();
      }

      _houseKeepingTimer.expires_from_now(boost::posix_time::seconds(TIMER_SERVICE_TICK));
      _houseKeepingTimer.async_wait(boost::bind(&TimerService::onHouseKeepingTimer, this, boost::asio::placeholders::error));
      
    }
  }


  void queueForDestruction(OsTimer::Timer::Ptr pTimer)
  {
    mutex_lock lock(_queueMutex);
    _timerQueue.push(pTimer);

    // decr global timers count as a timer was schedulled for destruction
    gTimersNum--;
    assert(0 <= gTimersNum);
  }

  boost::asio::io_service _ioService;
  boost::asio::deadline_timer _houseKeepingTimer;
  boost::thread* _pIoServiceThread1;
  boost::thread* _pIoServiceThread2;
  OsTimer::Time _lastTick;
  mutex _queueMutex;
  std::queue<OsTimer::Timer::Ptr> _timerQueue;
};

static TimerService* gpTimerService = 0;

const UtlContainableType OsTimer::TYPE = "OsTimer";


/// Subclass of OsNotification that queues a copy of a message.
class OsQueueMsgNotification : public OsNotification
{
public:
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

   //:Signal the occurrence of the event
   virtual OsStatus signal(intptr_t eventData)
    {
       // mpQueue->send() copies *mpMsg and queues it on *mpQueue.
       return mpQueue->send(*mpMsg);
    }

protected:
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



// Timer expiration event notification happens using the
// newly created OsQueuedEvent object
OsTimer::OsTimer(OsMsgQ* pQueue,
                 void* userData) :
  _pTimer(new Timer(*this)),
  _pNotifier(new OsQueuedEvent(*pQueue, userData)) //< used to signal timer expiration event
{
    // No point in continuing with an invalid queue
    OS_LOG_AND_ASSERT(
            (NULL != pQueue),
            FAC_KERNEL,
            "OsTimer::OsTimer pQueue is NULL"
            );

  _pTimer->takeOwnership(_pNotifier);
}

// The address of "this" OsTimer object is the eventData that is
// conveyed to the Listener when the notification is signaled.
OsTimer::OsTimer(OsNotification& rNotifier) :
  _pTimer(new Timer(*this)),
  _pNotifier(&rNotifier) //< used to signal timer expiration event
{
}

// Timer expiration event notification is done by queueing a copy of
// *pMsg to *pQueue.
OsTimer::OsTimer(OsMsg* pMsg,
                 OsMsgQ* pQueue) :
  _pTimer(new Timer(*this)),
  _pNotifier(new OsQueueMsgNotification(pQueue, pMsg)) //< used to signal timer expiration event
{
    // No point in continuing with an invalid msg and queue
    OS_LOG_AND_ASSERT(
            ((NULL != pMsg) && (NULL != pQueue)),
            FAC_KERNEL,
            "OsTimer::OsTimer pMsg or pQueue are NULL"
            );

	_pTimer->takeOwnership(_pNotifier);
}

/// This constructor accepts a handler function instead of message queue.
/// One must take caution in using this facility to make sure that the handler
/// function will not block becuase it is directly called from the
/// io_service thread;
OsTimer::OsTimer(const Handler& handler) :
  _pTimer(new Timer(*this)),
  _pNotifier(0),
  _handler(handler)
{
}

// Destructor
OsTimer::~OsTimer()
{
  stop();

  gpTimerService->queueForDestruction(_pTimer);
  _pTimer.reset();
}

/* ============================ MANIPULATORS ============================== */

bool OsTimer::signalNotifier()
{
  if (!_pNotifier)
    return false;
  _pNotifier->signal((intptr_t)this);
  return true;
}

bool OsTimer::signalHandler(boost::system::error_code ec)
{
  if (!_handler)
    return false;
  _handler(*this, ec);
  return true;
}

// Disarm the timer
OsStatus OsTimer::stop(UtlBoolean synchronous)
{
  if (!_pTimer->isRunning())
    return OS_FAILED;
  _pTimer->clearPeriodic();
  _pTimer->cancel();
  return OS_SUCCESS;
}

void OsTimer::terminateTimerService()
{
  boost::lock_guard<boost::mutex> lock(gTimerServiceMutex);

  // Verify the following conditions:
  // 1. Timer service has not been deleted already
  // 2. There are not active timers which are using the timer service.
  //    WARN: If there are active timers we'd rather leak mem than corrupt something.
  if (gpTimerService && (0 == gTimersNum))
  {
    delete gpTimerService;
    gpTimerService = 0;
  }
}

/* ============================ ACCESSORS ================================= */



// Get the userData value of a timer constructed with OsTimer(OsMsgQ*, int).
void* OsTimer::getUserData()
{
   // Have to cast mpNotifier into OsQueuedEvent* to get the userData.
   OsQueuedEvent* e = dynamic_cast <OsQueuedEvent*> (_pNotifier);
   assert(e != 0);
   void* userData = 0;
   e->getUserData(userData);
   return userData;
}

/* ============================ INQUIRY =================================== */



int OsTimer::compareTo(UtlContainable const * inVal) const
{
   int result = -1;
   if (inVal->isInstanceOf(OsTimer::TYPE))
      result = comparePtrs(this, inVal);
   return result;
}

/// Return the state value for this OsTimer object
OsTimer::OsTimerState OsTimer::getState(void)
{
  return _pTimer->isRunning() ?  STARTED : STOPPED;
}

/// Return all the state information for this OsTimer object.
void OsTimer::getFullState(enum OsTimerState& state,
                         Time& expiresAt,
                         UtlBoolean& periodic,
                         Interval& period)
{
  state = getState();
  OsTimer::Timer::mutex_lock lock(_pTimer->_mutex);
  expiresAt = _pTimer->_expiresAt;
  periodic = _pTimer->_periodic ? TRUE : FALSE;
  period = _pTimer->_period;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

// Get the current time as a Time.
OsTimer::Time OsTimer::now()
{
   OsTime t;
   OsDateTime::getCurTime(t);
   return (Time)(t.seconds()) * TIMER_TIME_UNIT + t.usecs();
}


/// Perform a blocking wait on the timer.
   /// Returns error code if any.
boost::system::error_code OsTimer::wait(const boost::asio::deadline_timer::duration_type& offset)
{
  if (!gpTimerService)
  {
    gpTimerService = new TimerService();
    gpTimerService->start();
  }
  boost::asio::deadline_timer timer(gpTimerService->_ioService);
  timer.expires_from_now(offset);
  boost::system::error_code ec;
  timer.wait(ec);
  return ec;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */


OsTimer::Timer::Timer(OsTimer& owner) :
  _owner(owner),
  _pDeadline(0),
  _expiresAt(0),
  _periodic(false),
  _period(0),
  _isRunning(false),
  _pNotifier(0)
{
  if (!gpTimerService)
  {
    gpTimerService = new TimerService();
    gpTimerService->start();
  }
  mutex_lock lock(gTimerServiceMutex);
  _pDeadline = new boost::asio::deadline_timer(gpTimerService->_ioService);
  // incr global timers counter as new timer was created
  gTimersNum++;
}

OsTimer::Timer::~Timer()
{
  _periodic = false;

  boost::system::error_code ec;
  _isRunning = false;
  _pDeadline->cancel(ec);

  delete _pDeadline;
  _pDeadline = 0;
  delete _pNotifier; 
  _pNotifier = 0;
}

bool OsTimer::Timer::isRunning()
{
  mutex_lock lock(_mutex);
  return _isRunning;
}

void OsTimer::Timer::cancel()
{
  mutex_lock lock(gTimerServiceMutex);
  boost::system::error_code ec;
  _isRunning = false;
  _pDeadline->cancel(ec);
}

void OsTimer::Timer::onTimerFire(const boost::system::error_code& e, OsTimer* pOwner)
{
  {
    mutex_lock lock(_mutex);
    if (!_isRunning)
    {
      return;
    }
    else
    {
      _isRunning = false;
    }
  }

  if (!pOwner->signalHandler(e))
  {
    if (!e)
      pOwner->signalNotifier();
  }

  {
    mutex_lock lock(_mutex);
    if (!_periodic)
    {
      return;
    }
  }

  //
  // This is a periodic timer
  //
  //
  // This function sets the expiry time. Any pending asynchronous wait
  // operations will be cancelled. The handler for each cancelled operation will
  // be invoked with the boost::asio::error::operation_aborted error code.
  //

  {
    mutex_lock lock(gTimerServiceMutex);
    _expiresAt = OsTimer::now() + _period;
    boost::system::error_code ec;
    _pDeadline->expires_from_now(boost::posix_time::microseconds(_period), ec);
    //
    // Perform an asynchronous wait on the timer
    //
    _pDeadline->async_wait(boost::bind(&OsTimer::Timer::onTimerFire, shared_from_this(), boost::asio::placeholders::error, &_owner));
  }

  {
    mutex_lock lock(_mutex);
    _isRunning = true;
  }
}

/// Start the timer to fire once at the indicated date/time
bool OsTimer::Timer::oneshotAt(const OsDateTime& t)
{
  OsTimer::Time now = OsTimer::now();
  OsTime t_os;
  t.cvtToTimeSinceEpoch(t_os);
  OsTimer::Time expireFromNow = (OsTimer::Time)(t_os.seconds()) * TIMER_TIME_UNIT + t_os.usecs();

  {
    mutex_lock lock(_mutex);
    if (_isRunning)
      return false;
    else
      _isRunning = true;
 
    if (expireFromNow <= now)
    {
      OS_LOG_ERROR(FAC_KERNEL, "OsTimer::Timer::oneshotAt timer expiration is in the past.  Call ignored.");
      _isRunning = false;
      return false;
    }
    _expiresAt = expireFromNow;
  }


  //
  // This function sets the expiry time. Any pending asynchronous wait 
  // operations will be cancelled. The handler for each cancelled operation will
  // be invoked with the boost::asio::error::operation_aborted error code.
  //
  {
    mutex_lock lock(gTimerServiceMutex);
    boost::system::error_code ec;
    _pDeadline->expires_from_now(boost::posix_time::microseconds(expireFromNow - now), ec);
    //
    // Perform an assynchronous wait on the timer
    //
    _pDeadline->async_wait(boost::bind(&OsTimer::Timer::onTimerFire, shared_from_this(), boost::asio::placeholders::error, &_owner));
  }  
  return _isRunning;
}

/// Start the timer to fire once at the current time + offset
/// Start the timer to fire once at the current time + offset
bool OsTimer::Timer::oneshotAfter(const boost::asio::deadline_timer::duration_type& offset)
{
  OsTimer::Time expireFromNow = offset.total_microseconds();
  {
    mutex_lock lock(_mutex);
    if (_isRunning)
      return false;
    else
      _isRunning = true;

    _expiresAt = expireFromNow + OsTimer::now();
  }

  //
  // This function sets the expiry time. Any pending asynchronous wait
  // operations will be cancelled. The handler for each cancelled operation will
  // be invoked with the boost::asio::error::operation_aborted error code.
  //
  {
    mutex_lock lock(gTimerServiceMutex);
    boost::system::error_code ec;
    _pDeadline->expires_from_now(offset, ec);

    //
    // Perform an assynchronous wait on the timer
    //
    _pDeadline->async_wait(boost::bind(&OsTimer::Timer::onTimerFire, shared_from_this(), boost::asio::placeholders::error, &_owner));
  }
  return _isRunning;
}

bool OsTimer::Timer::oneshotAfter(const OsTime& t)
{
  OsTimer::Time expireFromNow = TIME_TO_INTERVAL(t);
  return oneshotAfter(boost::posix_time::microseconds(expireFromNow));
}

/// Start the timer to fire periodically starting at the indicated date/time
bool OsTimer::Timer::periodicAt(const OsDateTime& when, OsTime period)
{
  {
    mutex_lock lock(_mutex);
    _periodic = true;
    _period = TIME_TO_INTERVAL(period);
  }
  return oneshotAt(when);
}


/// Start the timer to fire periodically starting at current time + offset
bool OsTimer::Timer::periodicEvery(const boost::asio::deadline_timer::duration_type& offset,
  const boost::asio::deadline_timer::duration_type& period)
{
  {
    mutex_lock lock(_mutex);
    _periodic = true;
    _period = period.total_microseconds();
  }
  return oneshotAfter(offset);
}

/// Start the timer to fire periodically starting at current time + offset
bool OsTimer::Timer::periodicEvery(OsTime offset, OsTime period)
{
  {
    mutex_lock lock(_mutex);
    _periodic = true;
    _period = TIME_TO_INTERVAL(period);
  }
  return oneshotAfter(offset);
}

void OsTimer::Timer::clearPeriodic()
{
  mutex_lock lock(_mutex);
  _periodic = false;
}




