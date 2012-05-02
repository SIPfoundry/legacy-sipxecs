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

#ifndef TIMEDSET_H
#define	TIMEDSET_H

#include "TimedQueue.h"
#include <set>

class TimedMap
{
public:
  typedef boost::recursive_mutex mutex;
  typedef boost::lock_guard<mutex> mutex_lock;
  typedef boost::function<void(const std::string&, TimedQueue*)>  TimedMapHandler;
  typedef std::map<std::string, boost::any> Items;
  struct TimedMapRecord
  {
    TimedMapRecord() :
      data(0),
      deadline(0)
    {
    }

    ~TimedMapRecord()
    {
      if (deadline)
      {
        deadline->cancel();
        delete deadline;
      }
      if (data)
      {
        data->stop();
        delete data;
      }
    }
    std::string id;
    TimedQueue* data;
    boost::asio::deadline_timer* deadline;
    TimedMapHandler deadlineFunc;
  };
protected:
  boost::asio::io_service* _pIoService;
  boost::thread* _thread;
  boost::asio::deadline_timer* _housekeeper;
  boost::recursive_mutex _mutex;
  std::map<std::string, TimedQueue*> _map;
  std::set<std::string> _deleted;
  bool _isLocalIo;
  bool _terminated;

public:
  TimedMap(boost::asio::io_service* pIoService = 0) :
     _pIoService(pIoService),
     _thread(0),
     _housekeeper(0),
     _isLocalIo(false),
     _terminated(false)
  {
    _isLocalIo = (_pIoService == 0);
    if (_isLocalIo)
    {
      _pIoService = new boost::asio::io_service();
      _housekeeper = new boost::asio::deadline_timer(*_pIoService, boost::posix_time::milliseconds(3600 * 1000));
      _housekeeper->expires_from_now(boost::posix_time::milliseconds(3600 * 1000));
      _housekeeper->async_wait(boost::bind(&TimedMap::onHousekeeping, this, boost::asio::placeholders::error));
      _thread = new boost::thread(boost::bind(&boost::asio::io_service::run, _pIoService));
    }
  }

  ~TimedMap()
  {
    stop();
  }

  void stop()
  {
    _terminated = true;
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

    mutex_lock lock(_mutex);
    for(std::map<std::string, TimedQueue*>::const_iterator iter = _map.begin(); iter != _map.end(); iter++)
      _deleted.insert(iter->second->name());
    cleanup();
  }

  void insert(const std::string& setId, const std::string& itemId, const boost::any& item, int expires = -1)
  {
    cleanup();
    mutex_lock lock(_mutex);
    if (_terminated)
      return;
    //
    // check if the set already exists.  if not create it
    //
    if (_map.find(setId) == _map.end())
    {
      //
      // Create a new set
      //
      TimedQueue* set = new TimedQueue(_pIoService);
      set->name() = setId;
      if (expires > 0)
        set->enqueue(itemId, item, boost::bind(&TimedMap::onItemExpired, this, _1, _2, _3), expires);
      else
        set->enqueue(itemId, item);
      _map[setId] = set;
    }
    else
    {
      if (expires > 0)
        _map[setId]->enqueue(itemId, item, boost::bind(&TimedMap::onItemExpired, this, _1, _2, _3), expires);
      else
        _map[setId]->enqueue(itemId, item);
    }
  }

  void eraseItem(const std::string& setId, const std::string& itemId)
  {
    mutex_lock lock(_mutex);
    if (_terminated)
      return;
    if (_map.find(setId) == _map.end())
      return;
    _map[setId]->erase(itemId);
    
    if (_map[setId]->getSize() == 0)
      eraseSet(setId);
  }
  
  void eraseSet(const std::string& setId)
  {
    mutex_lock lock(_mutex);
    if (_terminated)
      return;
    if (_map.find(setId) == _map.end())
      return;
    TimedQueue* set = _map[setId];
    delete set;
    _map.erase(setId);
  }

  bool getItem(const std::string& setId, const std::string& itemId, boost::any& item)
  {
    mutex_lock lock(_mutex);
    if (_terminated)
      return false;
    if (_map.find(setId) == _map.end())
      return false;
    return _map[setId]->get(itemId, item);
  }

  bool getIncrementedItem(const std::string& setId, const std::string& itemId, int& value)
  {
    mutex_lock lock(_mutex);
    if (_terminated)
      return false;
    if (_map.find(setId) == _map.end())
      return false;
    return _map[setId]->increment(itemId, value);
  }

  bool getItems(const std::string& setId,  Items& items)
  {
    mutex_lock lock(_mutex);
    if (_terminated)
      return false;
    if (_map.find(setId) == _map.end())
      return false;
    return _map[setId]->getItems(items);
  }
  
  void cleanup()
  {
    mutex_lock lock(_mutex);
    for (std::set<std::string>::const_iterator iter = _deleted.begin(); iter != _deleted.end(); iter++)
      eraseSet(*iter);
    _deleted.clear();
  }
protected:
  void onHousekeeping(const boost::system::error_code&)
  {
    _housekeeper->expires_from_now(boost::posix_time::milliseconds(3600 * 1000));
    _housekeeper->async_wait(boost::bind(&TimedMap::onHousekeeping, this, boost::asio::placeholders::error));
  }

  void onItemExpired(const std::string&, const boost::any&, TimedQueue* pQueue)
  {
    if (pQueue->getSize() == 0)
    {
      mutex_lock lock(_mutex);
      //
      // Mark for deletion
      //
      assert(!pQueue->name().empty());
      _deleted.insert(pQueue->name());
    }
  }

  

  void onSetExpired(const std::string& id, TimedQueue* set)
  {
    delete set;
  }
};



#endif	/* TIMEDSET_H */

