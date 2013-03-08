//
// Copyright (C) 2009 Nortel Networks, certain elements licensed under a Contributor Agreement.
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsMsgQShared_h_
#define _OsMsgQShared_h_

// SYSTEM INCLUDES
#include <queue>
#include <boost/thread.hpp>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsMsg.h"
#include "os/OsMsgQ.h"
#include "os/OsTime.h"
#include "utl/UtlDList.h"
#include "os/OsLogger.h"



// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
class UtlString;

// FORWARD DECLARATIONS

//:Message queue implementation for OS's with no native message queue support
class OsMsgQShared : public OsMsgQBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

  friend class OsMsgQSharedTest;

  enum QueuePreference
  {
      QUEUE_LIMITED = 0,  //:Queue will be limited to a max number of elements
                          // that can be enqueued.
      QUEUE_UNLIMITED,    //:Queue will not be limited, any number of elements
                          // can be enqueued.
  };

  typedef boost::mutex mutex_critic_sec;
  typedef boost::lock_guard<mutex_critic_sec> mutex_critic_sec_lock;

/* ============================ CREATORS ================================== */
  // Counting semaphore class with upper limit
  class Semaphore
  {
    // The max semaphore count
    unsigned int _max;

    //The current semaphore count.
    unsigned int _count;

    //_mutex protects _count.
    //Any code that reads or writes the _count data must hold a lock on
    //the mutex.
    boost::mutex _mutex;

    //Code that increments _count must notify the condition variable.
    boost::condition_variable _condition;

  public:
    Semaphore()
        : _max((unsigned int)-1),
          _count(0),
          _mutex(),
          _condition()
    {
    }

    explicit Semaphore(unsigned int maxCount, unsigned int initialCount)
        : _max(maxCount),
          _count(initialCount),
          _mutex(),
          _condition()
    {
    }

    void signal() //called "release" in Java
    {
      boost::unique_lock<boost::mutex> lock(_mutex);
      while (_count >= _max)
      {
        _condition.wait(lock);
      }
      ++_count;

      //Wake up any waiting threads.
      //Always do this, even if _count wasn't 0 on entry.
      //Otherwise, we might not wake up enough waiting threads if we
      //get a number of signal() calls in a row.
      _condition.notify_one();
    }

    void wait() //called "acquire" in Java
    {
      boost::unique_lock<boost::mutex> lock(_mutex);
      while (_count == 0)
      {
        _condition.wait(lock);
      }
      --_count;
    }

    bool wait(int milliseconds)
    {
      boost::unique_lock<boost::mutex> lock(_mutex);
      boost::system_time const timeout = boost::get_system_time()+ boost::posix_time::milliseconds(milliseconds);
      while (_count == 0)
      {
        if (!_condition.timed_wait(lock,timeout))
          return false;
      }
      --_count;
      return true;
    }
 };

  OsMsgQShared(
    const char* name,                        //:global name for this queue
    int         maxMsgs = DEF_MAX_MSGS,      //:max number of messages
    int         maxMsgLen = DEF_MAX_MSG_LEN, //:max msg length (bytes)
    int         options = Q_PRIORITY,        //:how to queue blocked tasks
    bool        reportFull = true
                ///< should be true for work queues of OsServerTask's
                ///< should be false for media processing queues which
                ///< are usually full
    );
   //:Constructor
   // If name is specified but is already in use, throw an exception

  virtual
  ~OsMsgQShared();
   //:Destructor

/* ============================ MANIPULATORS ============================== */

  static void setQueuePreference(QueuePreference preference);
  //:Set the preference to use for newly created queues.

  virtual OsStatus send(const OsMsg& rMsg,
                       const OsTime& rTimeout=OsTime::OS_INFINITY);
   //:Insert a message at the tail of the queue
   // Wait until there is either room on the queue or the timeout expires.

  virtual OsStatus sendP(OsMsg* pMsg,
                        const OsTime& rTimeout=OsTime::OS_INFINITY);
   //:Insert a message at the tail of the queue
   // Wait until there is either room on the queue or the timeout expires.
   // Takes ownership of *pMsg.

  virtual OsStatus sendFromISR(const OsMsg& rMsg);
   //:Insert a message at the tail of the queue.
   // Sending from an ISR has a couple of implications.  Since we can't
   // allocate memory within an ISR, we don't create a copy of the message
   // before sending it and the sender and receiver need to agree on a
   // protocol (outside this class) for when the message can be freed.
   // The sentFromISR flag in the OsMsg object will be TRUE for messages
   // sent using this method.

  virtual OsStatus receive(OsMsg*& rpMsg,
                          const OsTime& rTimeout=OsTime::OS_INFINITY);
   //:Remove a message from the head of the queue
   // Wait until either a message arrives or the timeout expires.
   // Other than for messages sent from an ISR, the receiver is responsible
   // for freeing the received message.

/* ============================ ACCESSORS ================================= */
  virtual int numMsgs(void);
   //:Return the number of messages in the queue


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
  OsStatus doSend(const OsMsg& rMsg,
                 const OsTime& rTimeout,
                 const UtlBoolean isUrgent,
                 const UtlBoolean sendFromISR);
   //:Helper function for sending messages that may not need to be copied

  OsStatus doSendCore(OsMsg* pMsg,
                     const OsTime& rTimeout,
                     UtlBoolean isUrgent,
                     UtlBoolean deleteWhenDone
                     /**< If true, if the message is not sent, doSendCore
                      *   will delete it.  Set if the caller expects the
                      *   recipient to delete *pMsg.
                      */
    );
   //:Core helper function for sending messages.

  OsStatus doReceive(OsMsg*& rpMsg, const OsTime& rTimeout);
   //:Helper function for removing a message from the head of the queue

  OsMsgQShared(const OsMsgQShared& rOsMsgQShared);
   //:Copy constructor (not implemented for this class)

  OsMsgQShared& operator=(const OsMsgQShared& rhs);
   //:Assignment operator (not implemented for this class)


protected:
  void enqueue(OsMsg* data)
  {
    assert(data);

    if (QUEUE_LIMITED == _queuePreference)
    {
      _empty->wait();
    }

    _cs.lock();
    _queue.push(data);
    _cs.unlock();

    _full->signal();
  }

  bool try_enqueue(OsMsg* data, long milliseconds)
  {
    assert(data);

    if (QUEUE_LIMITED == _queuePreference)
    {
      if (!_empty->wait(milliseconds))
      {
        return false;
      }
    }

    _cs.lock();
    _queue.push(data);
    _cs.unlock();

    _full->signal();
    return true;
  }

  void dequeue(OsMsg*& data)
  {
    _full->wait();

    _cs.lock();
    data = _queue.front();
    _queue.pop();
    assert(data);
    _cs.unlock();

    if (QUEUE_LIMITED == _queuePreference)
    {
      _empty->signal();
    }
  }

  bool try_dequeue(OsMsg*& data, long milliseconds)
  {
    if (!_full->wait(milliseconds))
      return false;

    _cs.lock();
    data = _queue.front();
    _queue.pop();
    assert(data);
    _cs.unlock();

    if (QUEUE_LIMITED == _queuePreference)
    {
      _empty->signal();
    }

    return true;
  }

private:
  Semaphore* _empty;
  Semaphore* _full;
  mutex_critic_sec _cs;
  std::queue<OsMsg*> _queue;
  int _maxMsgLen;
  int _options;
  bool _reportFull;
  QueuePreference _queuePreference;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsMsgQShared_h_
