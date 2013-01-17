/*
 * Copyright (c) eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */

#ifndef OSPOOLEDTASK_H_INCLUDED
#define	OSPOOLEDTASK_H_INCLUDED


#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>

#include "os/OsTask.h"
#include "os/OsMsgQ.h"
#include "os/OsTimer.h"


class OsPooledTask : boost::noncopyable
{
public:

  static const int DEF_MAX_MSGS;

  typedef std::vector<boost::thread*> ThreadPool;
  typedef boost::recursive_mutex mutex;
  typedef boost::lock_guard<mutex> mutex_lock;

  OsPooledTask(
    const char* name,
    std::size_t poolSize,
    const int maxRequestQMsgs = DEF_MAX_MSGS,
    bool synchronizeHandler = true);

  virtual ~OsPooledTask();

  //
  // Pure virtual overrides from OsTaskBase
  //
  /// Spawn a new task and invoke its run() method..
   UtlBoolean start(void);
   /**<
    * @Return TRUE if the spawning of the new task is successful,
    *         FALSE if the task spawn fails or if the task has already been started.
    */

   /// Request a task shutdown.
   void requestShutdown(void);

   /// Handles an incoming message.
   virtual UtlBoolean handleMessage(OsMsg& rMsg);
   /**<
    * If the message is not one that the object is prepared to process,
    * the handleMessage() method in the derived class should return FALSE
    * which will cause the default OsServerTask handler method to be
    * invoked on the message.
    */

   /// Posts a message to this task.
   OsStatus postMessage(const OsMsg& rMsg);

   /// Get the pointer to the incoming message queue.
   OsMsgQ* getMessageQueue();

   const UtlString& getName(void) const;

   UtlBoolean waitUntilShutDown();

   UtlBoolean isStarted() const;

   static OsStatus delay(const int milliSecs);
private:
   /// Waits for a message to arrive on the task's incoming message queue.
  OsStatus receiveMessage(OsMsg*& rpMsg);


  /// The entry point for the task.
  int run(void* pArg);


  OsMsgQ _messageQueue;                 ///< Queue for incoming messages.
  ThreadPool _threadPool;
  std::size_t _poolSize;
  bool _isTerminated;
  bool _synchronizeHandler;
  mutex _mutex;
protected:
  UtlString mName; /// For backward compatibility with OsServerTask
};


//
// Inlines
//
inline OsMsgQ* OsPooledTask::getMessageQueue()
{
  return &_messageQueue;
}

inline const UtlString& OsPooledTask::getName(void) const
{
  return mName;
}

// Waits for a message to arrive on the task's incoming message queue.
inline OsStatus OsPooledTask::receiveMessage(OsMsg*& rpMsg)
{
   return _messageQueue.receive(rpMsg);
}

inline UtlBoolean OsPooledTask::waitUntilShutDown()
{
  requestShutdown();
  return TRUE;
}

inline UtlBoolean OsPooledTask::isStarted() const
{
  return !_isTerminated;
}

inline OsStatus OsPooledTask::delay(const int milliSecs)
{
  OsTimer::wait(boost::posix_time::milliseconds(milliSecs));
  return OS_SUCCESS;
}

// Post a message to this task.
// Return the result of the message send operation.
inline OsStatus OsPooledTask::postMessage(const OsMsg& rMsg)
{
  return _messageQueue.send(rMsg, OsTime::OS_INFINITY);
}

inline UtlBoolean OsPooledTask::handleMessage(OsMsg& rMsg)
{
  ///  noop
  return FALSE;
}

#endif	// OSPOOLEDTASK_H_INCLUDED

