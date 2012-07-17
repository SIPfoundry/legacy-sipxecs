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

#include <cassert>
#include "sqa/StateQueuePersistence.h"
#include "os/OsLogger.h"

StateQueuePersistence::StateQueuePersistence(unsigned workspaceCount, bool useRedis) :
  _terminated(false),
  _useRedis(useRedis)
{
  _pHousekeeper = new boost::asio::deadline_timer(_ioService, boost::posix_time::milliseconds(3600 * 1000));
  _pHousekeeper->expires_from_now(boost::posix_time::milliseconds(3600 * 1000));
  _pHousekeeper->async_wait(boost::bind(&StateQueuePersistence::onHousekeeping, this, boost::asio::placeholders::error));
  _pIothread = new boost::thread(boost::bind(&boost::asio::io_service::run, &_ioService));


  //
  // Check if we can use redis by trying to connect to
  //


  for (unsigned i = 0; i < workspaceCount; i++)
    _workspaces[i] = new TimedQueue(&_ioService);

  for (unsigned i = 0; i < workspaceCount; i++)
    _mapWorkspaces[i] = new TimedMap(&_ioService);

}

StateQueuePersistence::~StateQueuePersistence()
{
  stop();

  if (!_useRedis)
  {
    for (unsigned i = 0; i < _workspaces.size(); i++)
    {
      TimedQueue* workspace = _workspaces[i];
      delete workspace;
      workspace = 0;
    }

    for (unsigned i = 0; i < _mapWorkspaces.size(); i++)
    {
      TimedMap* workspace = _mapWorkspaces[i];
      delete workspace;
      workspace = 0;
    }
  }
}

void StateQueuePersistence::stop()
{
  _terminated = true;

  if (!_useRedis)
  {
    for (unsigned i = 0; i < _workspaces.size(); i++)
    {
      TimedQueue* workspace = _workspaces[i];
      workspace->stop();
    }
  }

  _ioService.stop();
  if (_pIothread)
  {
    _pIothread->join();
    delete _pIothread;
    _pIothread = 0;
  }
  if (_pHousekeeper)
  {
    _pHousekeeper->cancel();
    delete _pHousekeeper;
    _pHousekeeper = 0;
  }
}

void StateQueuePersistence::set(unsigned workspace, const StateQueueRecord& record, int expires)
{
  if (_terminated)
    return;
  assert(workspace < _workspaces.size());
  TimedQueue* pWorkspace = _workspaces[workspace];
  if (pWorkspace)
    pWorkspace->enqueue(record.id, record, boost::bind(&StateQueuePersistence::onDataExpired, this, _1, _2, _3), expires);
}


bool StateQueuePersistence::get(unsigned workspace, const std::string& id, StateQueueRecord& record)
{
  if (_terminated)
    return false;
  assert(workspace < _workspaces.size());
  TimedQueue* pWorkspace = _workspaces[workspace];
  if (!pWorkspace)
    return false;
  boost::any queueData;
  if (pWorkspace->get(id, queueData))
  {
    record = boost::any_cast<const StateQueueRecord&>(queueData);
    return true;
  }
  return false;
}

bool StateQueuePersistence::erase(unsigned workspace, const std::string& id)
{
  if (_terminated)
    return false;
  assert(workspace < _workspaces.size());
  TimedQueue* pWorkspace = _workspaces[workspace];
  if (!pWorkspace)
    return false;
  boost::any queueData;
  return pWorkspace->dequeue(id, queueData);
}

void StateQueuePersistence::mapSet(unsigned workspace, const std::string& mapId, const StateQueueRecord& record, int expires)
{
  if (_terminated)
    return;
  assert(workspace < _mapWorkspaces.size());
  TimedMap* pWorkspace = _mapWorkspaces[workspace];
  if (pWorkspace)
    pWorkspace->insert(mapId, record.id, record.data, expires);
}

bool StateQueuePersistence::mapGet(unsigned workspace, const std::string& mapId, const std::string& id, StateQueueRecord& record)
{
  if (_terminated)
    return false;
  assert(workspace < _mapWorkspaces.size());
  TimedMap* pWorkspace = _mapWorkspaces[workspace];
  if (!pWorkspace)
    return false;
  boost::any queueData;
  if (pWorkspace->getItem(mapId, id, queueData))
  {
    record.id = mapId;
    record.data = boost::any_cast<const std::string&>(queueData);
    return true;
  }
  return false;
}

bool StateQueuePersistence::mapGetInc(unsigned workspace, const std::string& mapId, const std::string& id, StateQueueRecord& record)
{
  if (_terminated)
    return false;
  assert(workspace < _mapWorkspaces.size());
  TimedMap* pWorkspace = _mapWorkspaces[workspace];
  if (!pWorkspace)
    return false;
  boost::any queueData;
  int value;
  if (pWorkspace->getIncrementedItem(mapId, id, value))
  {
    try
    {
      record.id = id;
      record.data = boost::lexical_cast<std::string>(value);
    }
    catch(...)
    {
      return false;
    }
    return true;
  }
  return false;
}

bool StateQueuePersistence::mapGet(unsigned workspace, const std::string& mapId, RecordVector& records)
{
  if (_terminated)
    return false;
  assert(workspace < _mapWorkspaces.size());
  TimedMap* pWorkspace = _mapWorkspaces[workspace];
  if (!pWorkspace)
    return false;
  boost::any queueData;
  TimedMap::Items items;
  if (pWorkspace->getItems(mapId,  items))
  {
    for (TimedMap::Items::const_iterator iter = items.begin(); iter != items.end(); iter++)
    {
      StateQueueRecord record;
      record.id = iter->first;
      record.data = boost::any_cast<std::string>(iter->second);
      records.push_back(record);
    }
    return true;
  }
  return false;
}

void StateQueuePersistence::mapClear(unsigned workspace, const std::string& mapId)
{
  if (_terminated)
    return;
  assert(workspace < _mapWorkspaces.size());
  TimedMap* pWorkspace = _mapWorkspaces[workspace];
  if (!pWorkspace)
    return;
  pWorkspace->eraseSet(mapId);
}

void StateQueuePersistence::mapRemove(unsigned workspace, const std::string& mapId, const std::string& dataId)
{
  if (_terminated)
    return;
  assert(workspace < _mapWorkspaces.size());
  TimedMap* pWorkspace = _mapWorkspaces[workspace];
  if (!pWorkspace)
    return;
  pWorkspace->eraseItem(mapId, dataId);
}

void StateQueuePersistence::onDataExpired(const std::string&, const boost::any&, TimedQueue*)
{
  //
  // nothing to do
  //
}

void StateQueuePersistence::onHousekeeping(const boost::system::error_code&)
{
  _pHousekeeper->expires_from_now(boost::posix_time::milliseconds(3600 * 1000));
  _pHousekeeper->async_wait(boost::bind(&StateQueuePersistence::onHousekeeping, this, boost::asio::placeholders::error));
}