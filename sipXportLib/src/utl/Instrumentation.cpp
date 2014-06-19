/*
 *
 *
 * Copyright (c) eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */

#include "utl/Instrumentation.h"
#include <execinfo.h>
#include <string.h>
#include <map>
#include <set>
#include <vector>
#include <sstream>
#include <boost/thread.hpp>


#define TraceElementSize 100
struct TraceElements
{
  TraceElements()
  {
    for (int i = 0; i < TraceElementSize; i++)
    {
      trace_elems[i] = 0;
    }
    size = 0;
  }

  TraceElements(const TraceElements& elems)
  {
    for (int i = 0; i < TraceElementSize; i++)
    {
      trace_elems[i] = elems.trace_elems[i];
    }
    size = elems.size;
  }

  TraceElements& operator=(const TraceElements& elems)
  {
    for (int i = 0; i < TraceElementSize; i++)
    {
      trace_elems[i] = elems.trace_elems[i];
    }
    size = elems.size;
    return *this;
  }

  void* trace_elems[TraceElementSize];
  int size;
};


typedef boost::mutex mutex_critic_sec;
typedef boost::lock_guard<mutex_critic_sec> mutex_lock;
typedef std::map<intptr_t, TraceElements> ObjectInfo;
typedef std::set<intptr_t> ObjectPointer;
static bool _isIntrumentationEnabled = false;
static bool _isBackTraceEnabled = false;
static boost::thread* _pTapThread = 0;
static mutex_critic_sec _objectInfoMutex;
static ObjectInfo _objectInfo;
static ObjectPointer _objectPointer;
static long _createdObjectCount = 0;
static long _deletedObjectCount = 0;


static const char* system_tap_queue_enqueue_queue;
static int system_tap_queue_enqueue_eventType;
static int system_tap_queue_enqueue_queueSize;





void ubacktrace_elems(TraceElements& elems)
{
  elems.size = backtrace( elems.trace_elems, TraceElementSize);
}

void ubacktrace(TraceElements& elems, std::string& bt)
{
  char** stack_syms(backtrace_symbols(elems.trace_elems, elems.size));
  bt.reserve(5120);
  for (int i = 0 ; i < elems.size ; ++i )
    bt.append(stack_syms[i]).push_back('\n');
  free(stack_syms);
}


void system_tap_queue_enqueue(const char* queue, int eventType, int queueSize)
{
  if (_isIntrumentationEnabled)
  {
    system_tap_queue_enqueue_queue = queue;
    system_tap_queue_enqueue_eventType = eventType;
    system_tap_queue_enqueue_queueSize = queueSize;
  }
}

static const char* system_tap_queue_dequeue_queue;
static int system_tap_queue_dequeue_eventType;
static int system_tap_queue_dequeue_queueSize;

void system_tap_queue_dequeue(const char* queue, int eventType, int queueSize)
{
  if (_isIntrumentationEnabled)
  {
    system_tap_queue_dequeue_queue = queue;
    system_tap_queue_dequeue_eventType = eventType;
    system_tap_queue_dequeue_queueSize = queueSize;
  }
}

static int system_tap_timer_create_expireTime;
void system_tap_timer_create(int expireTime)
{
  if (_isIntrumentationEnabled)
  {
    system_tap_timer_create_expireTime = expireTime;
  }
}

static int system_tap_timer_fire_precision;
static int system_tap_timer_fire_precision_overheadUseq;
static int system_tap_timer_fire_expireTime;
void system_tap_timer_fire(int expireTime, int precision, int overheadUseq)
{
  if (_isIntrumentationEnabled)
  {
    system_tap_timer_fire_expireTime = expireTime;
    system_tap_timer_fire_precision = precision;
    system_tap_timer_fire_precision_overheadUseq = overheadUseq;
  }
}

static int system_tap_timer_destroy_flag;
void system_tap_timer_destroy()
{
  if (_isIntrumentationEnabled)
  {
    system_tap_timer_destroy_flag = true;
  }
}

void system_tap_object_created(intptr_t pointerAddress, const char* className)
{
  if (_isIntrumentationEnabled)
  {
    mutex_lock lock(_objectInfoMutex);
    if (_isBackTraceEnabled)
    {
      TraceElements elem;
      ubacktrace_elems(elem);
      _objectInfo[pointerAddress] = elem;
    }
    else
      _objectPointer.insert(pointerAddress);
    _createdObjectCount++;
  }
}

void system_tap_object_destroyed(intptr_t pointerAddress, const char* className)
{
  if (_isIntrumentationEnabled)
  {
    mutex_lock lock(_objectInfoMutex);
    if (_isBackTraceEnabled)
      _deletedObjectCount += _objectInfo.erase(pointerAddress);
    else
      _deletedObjectCount += _objectPointer.erase(pointerAddress);
  }
}

static void tap_thread_sleep(unsigned long milliseconds)
{
	timeval sTimeout = { milliseconds / 1000, ( milliseconds % 1000 ) * 1000 };
	select( 0, 0, 0, 0, &sTimeout );
}

static long system_tap_report_object_count_created = 0;
static long system_tap_report_object_count_deleted = 0;
void system_tap_report_object_count(long created, long deleted)
{
  system_tap_report_object_count_created = created;
  system_tap_report_object_count_deleted = deleted;
}

static intptr_t system_tap_report_object_trace_pointerAddress = 0;
static const char* system_tap_report_object_trace_trace;
void system_tap_report_object_trace(intptr_t pointerAddress, const char* trace)
{
  system_tap_report_object_trace_pointerAddress = pointerAddress;
  system_tap_report_object_trace_trace = trace;
}

static void tap_thread_run_instrumentations()
{
  //
  // Clear the objects from previous run
  //
  {
    mutex_lock lock(_objectInfoMutex);
    _objectInfo.clear();
    _objectPointer.clear();
    _createdObjectCount = 0;
    _deletedObjectCount = 0;
  }

  _isIntrumentationEnabled = true;

  while (_isIntrumentationEnabled)
  {
    tap_thread_sleep(100);
    mutex_lock lock(_objectInfoMutex);
    system_tap_report_object_count(_createdObjectCount, _deletedObjectCount);
  }

  
  {
    mutex_lock lock(_objectInfoMutex);

    if (_isBackTraceEnabled)
    {
      for (ObjectInfo::iterator iter = _objectInfo.begin(); iter != _objectInfo.end(); iter++)
      {
        std::string bt;
        ubacktrace(iter->second, bt);
        system_tap_report_object_trace(iter->first, bt.c_str());
      }
    }
    else
    {
      for (ObjectPointer::const_iterator iter = _objectPointer.begin(); iter != _objectPointer.end(); iter++)
        system_tap_report_object_trace(*iter, "TRACE DISABLED");
    }

    _objectInfo.clear();
    _objectPointer.clear();
    _createdObjectCount = 0;
    _deletedObjectCount = 0;
  }
}

bool system_tap_start_portlib_instrumentation(bool enableTrace)
{
  if (_pTapThread)
    return false;
  _isBackTraceEnabled = enableTrace;
  _pTapThread = new boost::thread(boost::bind(tap_thread_run_instrumentations));
  return true;
}

bool system_tap_stop_portlib_instrumentation()
{
  if (_pTapThread)
  {
    _isIntrumentationEnabled = false;
    _pTapThread->join();
    delete _pTapThread;
    _pTapThread = 0;
  }
  return true;
}
