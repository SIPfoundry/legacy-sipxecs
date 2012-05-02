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

#ifndef TIMEDQUEUE_H
#define	TIMEDQUEUE_H

#include <map>
#include <string>
#include <boost/any.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <set>

class TimedQueue
{
public:
  typedef boost::recursive_mutex mutex;
  typedef boost::lock_guard<mutex> mutex_lock;
  typedef boost::function<void(const std::string&, const boost::any&, TimedQueue*)>  TimedQueueHandler;
  struct TimedQueueRecord
  {
    TimedQueueRecord() :
      deadline(0)
    {
    }

    ~TimedQueueRecord()
    {
      if (deadline)
      {
        deadline->cancel();
        delete deadline;
      }
    }
    std::string id;
    boost::any data;
    boost::asio::deadline_timer* deadline;
    TimedQueueHandler deadlineFunc;
  };

protected:
  boost::asio::io_service* _pIoService;
  boost::thread* _thread;
  boost::asio::deadline_timer* _housekeeper;
  boost::recursive_mutex _mutex;
  std::map<std::string, TimedQueueRecord> _map;
  bool _terminated;
  bool _isLocalIo;
  std::string _name;
public:
  TimedQueue(boost::asio::io_service* pIoService = 0) :
     _pIoService(pIoService),
     _thread(0),
     _housekeeper(0),
     _terminated(false),
     _isLocalIo(false)
  {
    _isLocalIo = (_pIoService == 0);
    if (_isLocalIo)
    {
      _pIoService = new boost::asio::io_service();
      _housekeeper = new boost::asio::deadline_timer(*_pIoService, boost::posix_time::milliseconds(3600 * 1000));
      _housekeeper->expires_from_now(boost::posix_time::milliseconds(3600 * 1000));
      _housekeeper->async_wait(boost::bind(&TimedQueue::onHousekeeping, this, boost::asio::placeholders::error));
      _thread = new boost::thread(boost::bind(&boost::asio::io_service::run, _pIoService));
    }
  }

  ~TimedQueue()
  {
    stop();
    if (_isLocalIo)
      delete _pIoService;
  }

  void stop()
  {
    {
      _terminated = true;
      //
      // Cancel all timers
      //
      mutex_lock lock(_mutex);
      for (std::map<std::string, TimedQueueRecord>::iterator iter = _map.begin(); iter != _map.end(); iter++)
      {
        iter->second.deadline->cancel();
      }

    }

    if (_isLocalIo)
    {
      _pIoService->stop();
      if (_thread)
      {
        _thread->join();
        delete _thread;
        _thread = 0;
      }


      if (_housekeeper)
      {
        _housekeeper->cancel();
        delete _housekeeper;
        _housekeeper = 0;
      }
    }

  }

  void enqueue(const std::string& id, const boost::any& data, const TimedQueueHandler& deadlineFunc, int expires)
  {
    mutex_lock lock(_mutex);
    if (_terminated)
      return;
    _map.erase(id);
    TimedQueueRecord record;
    _map[id] = record;
    _map[id].id = id;
    _map[id].data = data;
    _map[id].deadlineFunc = deadlineFunc;
    boost::asio::deadline_timer* timer = new boost::asio::deadline_timer(*_pIoService, boost::posix_time::milliseconds(expires * 1000));
    _map[id].deadline = timer;
    timer->expires_from_now(boost::posix_time::milliseconds(expires * 1000));
    timer->async_wait(boost::bind(&TimedQueue::onRecordExpire, this, boost::asio::placeholders::error, id));
  }

  void enqueue(const std::string& id, const boost::any& data)
  {
    mutex_lock lock(_mutex);
    if (_terminated)
      return;
    _map.erase(id);
    TimedQueueRecord record;
    record.id = id;
    record.data = data;
    _map[id] = record;
  }
  
  void erase(const std::string& id)
  {
    mutex_lock lock(_mutex);
    if (_terminated)
      return;
    _map.erase(id);
  }

  bool dequeue(const std::string& id, boost::any& data)
  {
    mutex_lock lock(_mutex);
    if (_terminated)
      return false;
    if (_map.find(id) == _map.end())
      return false;
    data = _map[id].data;
    _map.erase(id);
    return true;
  }

  bool increment(const std::string& id, int& newVal)
  {
    mutex_lock lock(_mutex);
    if (_map.find(id) == _map.end())
      return false;
    try
    {
      std::string snum = boost::any_cast<const std::string&>(_map[id].data);
      newVal = boost::lexical_cast<int>(snum);
      snum = boost::lexical_cast<std::string>(++newVal);
      _map[id].data = snum;
    }
    catch(...)
    {
      return false;
    }
    return true;
  }

  bool get(const std::string& id, boost::any& data)
  {
    mutex_lock lock(_mutex);
    if (_terminated)
      return false;
    if (_map.find(id) == _map.end())
      return false;
    data = _map[id].data;
    return true;
  }

  bool getItems(std::map<std::string, boost::any>& items)
  {
    mutex_lock lock(_mutex);
    for (std::map<std::string, TimedQueueRecord>::const_iterator iter = _map.begin(); iter != _map.end(); iter++)
    {
      items[iter->first] = iter->second.data;
    }
    return !items.empty();
  }

  std::size_t getSize()
  {
    mutex_lock lock(_mutex);
    return _map.size();
  }

  std::string& name() {return _name;}

protected:
  void onRecordExpire(const boost::system::error_code& e, const std::string& id)
  {
    if (!e && !_terminated)
    {
    
      _mutex.lock();
      if (_map.find(id) == _map.end())
      {
        _mutex.unlock();
        return;
      }

      TimedQueueHandler deadlineFunc = _map[id].deadlineFunc;
      boost::any data = _map[id].data;
      _map.erase(id);
      _mutex.unlock();

      //
      // mutex is no longer locked.  This callback may reinsert the item to the queue without deadlock
      //
      if (deadlineFunc)
        deadlineFunc(id, data, this);
    }
  }

  void onHousekeeping(const boost::system::error_code&)
  {
    _housekeeper->expires_from_now(boost::posix_time::milliseconds(3600 * 1000));
    _housekeeper->async_wait(boost::bind(&TimedQueue::onHousekeeping, this, boost::asio::placeholders::error));
  }
};


#endif	/* TIMEDQUEUE_H */

