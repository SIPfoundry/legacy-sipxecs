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


#ifndef OSTHREADPOOL_H_INCLUDED
#define	OSTHREADPOOL_H_INCLUDED


#include <queue>
#include <boost/thread.hpp>
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include <Poco/ThreadPool.h>
#include <Poco/Semaphore.h>

#define POOL_THREAD_STACK_SIZE 0


template <typename event_t, typename handler_t>
class OsRunnable : public Poco::Runnable
{
public:
  OsRunnable(handler_t handler, event_t task) :
    _task(task),
    _handler(handler)
  {
  }

  void run()
  {
    _handler(_task);
    if (_signalExit)
      _signalExit();
    delete this;
  }

  event_t _task;
  handler_t _handler;
  boost::function<void()> _signalExit;
};


template <typename runnable_t, typename event_t, typename handler_t>
class OsThreadPoolBase : boost::noncopyable
{
public:
  
public:
  OsThreadPoolBase(int minCapacity = 2,
		int maxCapacity = 1024,
		int idleTime = 60,
		int stackSize = POOL_THREAD_STACK_SIZE) :
      _threadPool(minCapacity, maxCapacity, idleTime, stackSize)
  {
  }
  
  ~OsThreadPoolBase()
  {
  }
    
  bool schedule(handler_t handler, event_t task)
  {
    runnable_t* pRunnable = new runnable_t(handler, task);

    if (_onSchedule)
      _onSchedule(*pRunnable, task);

    try
    {
      _threadPool.start(*pRunnable);
    }
    catch(...)
    {
      delete pRunnable;
      return false;
    }
    return true;
  }
  
  Poco::ThreadPool& threadPool()
  {
    return _threadPool;
  }

protected:
  boost::function<void(runnable_t&, event_t&)> _onSchedule;


private:
  Poco::ThreadPool _threadPool;

};

#define THREADPOOL_BASE OsThreadPoolBase< \
              OsRunnable< event_t, boost::function<void(event_t)> >, \
              event_t, \
              boost::function<void(event_t)> \
            >

template <typename event_t>
class OsThreadPool
  : public  THREADPOOL_BASE
{
public:
  OsThreadPool(int minCapacity = 2,
		int maxCapacity = 1024,
		int idleTime = 60,
		int stackSize = POOL_THREAD_STACK_SIZE) :
      THREADPOOL_BASE(minCapacity, maxCapacity, idleTime, stackSize)
  {
    THREADPOOL_BASE::_onSchedule = boost::bind(&OsThreadPool<event_t>::onSchedule, this, _1, _2);
    _onRunnableExited = boost::bind(&OsThreadPool<event_t>::onRunnableExited, this);
  }

protected:

  void onSchedule(OsRunnable< event_t, boost::function<void(event_t)> >& runnable, event_t& task)
  {
    runnable._signalExit = _onRunnableExited;
  }
  
private:
  void onRunnableExited()
  {
    //
    // Does nothing as of the moment
    //
  }
  boost::function<void()> _onRunnableExited;
};


#endif	/// OSTHREADPOOL_H_INCLUDED

