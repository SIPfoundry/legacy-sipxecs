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


template <size_t size>
bool string_sprintf_string(std::string& str, const char * format, ...)
  /// Write printf() style formatted data to string.
  ///
  /// The size template argument specifies the capacity of
  /// the internal buffer that will store the string result.
  /// Take note that if the buffer size is not enough
  /// the result of vsprintf will result to an overrun
  /// and will corrupt memory.
{
    char buffer[size];
    va_list args;
    va_start(args, format);
    int ret = vsprintf(buffer,format, args);
    va_end (args);
    if (ret >= 0)
    {
      str = buffer;
      return true;
    }
    return false;
}

static void escape(std::string& result, const char* _str, const char* validChars = 0)
{
  static const char * safeChars = "abcdefghijklmnopqrstuvwxyz"
                      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                      "0123456789$-_.!*(),+#;";

  int pos = -1;
  char* offSet = const_cast<char*>(_str);
  char* str = const_cast<char*>(_str);
  size_t len = strlen(str);

  std::string front;
  while ((pos += (int)(1+strspn(&str[pos+1], validChars == 0 ? safeChars : validChars))) < len)
  {
    std::string escaped;
    if (!string_sprintf_string<4>(escaped, "%%%02X", static_cast<const unsigned char>(str[pos])))
    {
      front = str;
      return;
    }
    front += std::string(offSet, str + pos );
    front += escaped;
    offSet = const_cast<char*>(str) + pos + 1;
  }
  front += std::string(offSet, str + pos );
  result = front;
}

static void unescape(std::string& result, const char* str)
{
  result = str;
  int pos = -1;

  while ((pos = (int)result.find('%', pos+1)) != std::string::npos)
  {
    int digit1 = result[pos+1];
    int digit2 = result[pos+2];
    if (isxdigit(digit1) && isxdigit(digit2))
    {
      result[pos] = (char)((isdigit(digit2) ? (digit2-'0') : (toupper(digit2)-'A'+10)) +((isdigit(digit1) ? (digit1-'0') : (toupper(digit1)-'A'+10)) << 4));
      result.erase(pos+1, 2);
    }
  }
}

StateQueuePersistence::StateQueuePersistence(unsigned workspaceCount, bool useRedis) :
  _terminated(false),
  _useRedis(useRedis)
{
  _pHousekeeper = new boost::asio::deadline_timer(_ioService, boost::posix_time::milliseconds(3600 * 1000));
  _pHousekeeper->expires_from_now(boost::posix_time::milliseconds(3600 * 1000));
  _pHousekeeper->async_wait(boost::bind(&StateQueuePersistence::onHousekeeping, this, boost::asio::placeholders::error));
  _pIothread = new boost::thread(boost::bind(&boost::asio::io_service::run, &_ioService));

  if (!_useRedis)
  {
    for (unsigned i = 0; i < workspaceCount; i++)
      _workspaces[i] = new TimedQueue(&_ioService);

    for (unsigned i = 0; i < workspaceCount; i++)
      _mapWorkspaces[i] = new TimedMap(&_ioService);
  }
  else
  {
    initRedis(workspaceCount);
  }

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

void StateQueuePersistence::initRedis(int count)
{
  for (int i = 0; i < count; i++)
  {
    RedisClient::Ptr pRedis(new RedisClient());
    pRedis->connect(i);
    _redisWorkspaces[i] = pRedis;
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

  if (!_useRedis)
  {
    assert(workspace < _workspaces.size());
    TimedQueue* pWorkspace = _workspaces[workspace];
    if (pWorkspace)
      pWorkspace->enqueue(record.id, record, boost::bind(&StateQueuePersistence::onDataExpired, this, _1, _2, _3), expires);
  }
  else
  {
    assert(workspace < _redisWorkspaces.size());
    RedisClient::Ptr pWorkspace = _redisWorkspaces[workspace];
    if (pWorkspace)
    {
      std::string json;
      record.toJson(json);
      pWorkspace->set(record.id, json, expires);
    }
  }
}


bool StateQueuePersistence::get(unsigned workspace, const std::string& id, StateQueueRecord& record)
{
  if (_terminated)
    return false;

  if (!_useRedis)
  {
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
  }
  else
  {
    assert(workspace < _redisWorkspaces.size());
    RedisClient::Ptr pWorkspace = _redisWorkspaces[workspace];
    if (pWorkspace)
    {
      std::string json;
      if (!pWorkspace->get(id, json))
        return false;
      return record.fromJson(json);
    }
  }
  return false;
}

bool StateQueuePersistence::erase(unsigned workspace, const std::string& id)
{
  if (_terminated)
    return false;

  if (!_useRedis)
  {
    assert(workspace < _workspaces.size());
    TimedQueue* pWorkspace = _workspaces[workspace];
    if (!pWorkspace)
      return false;
    boost::any queueData;
    return pWorkspace->dequeue(id, queueData);
  }
  else
  {
    assert(workspace < _redisWorkspaces.size());
    RedisClient::Ptr pWorkspace = _redisWorkspaces[workspace];
    if (pWorkspace)
    {
      return pWorkspace->del(id);
    }
  }
  return false;
}


void StateQueuePersistence::mapSet(unsigned workspace, const std::string& mapId, const StateQueueRecord& record, int expires)
{
  if (_terminated)
    return;

  if (!_useRedis)
  {
    assert(workspace < _mapWorkspaces.size());
    TimedMap* pWorkspace = _mapWorkspaces[workspace];
    if (pWorkspace)
      pWorkspace->insert(mapId, record.id, record.data, expires);
  }
  else
  {
    assert(workspace < _redisWorkspaces.size());
    RedisClient::Ptr pWorkspace = _redisWorkspaces[workspace];
    if (pWorkspace)
    {
      std::string data;
      escape(data, record.data.c_str());
      pWorkspace->hset(mapId, record.id, data);
    }
  }
}

bool StateQueuePersistence::mapGet(unsigned workspace, const std::string& mapId, const std::string& id, StateQueueRecord& record)
{
  if (_terminated)
    return false;

  if (!_useRedis)
  {
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
  }
  else
  {
    assert(workspace < _redisWorkspaces.size());
    RedisClient::Ptr pWorkspace = _redisWorkspaces[workspace];
    if (pWorkspace)
    {
      std::string data;
      if (pWorkspace->hget(mapId, id, data))
      {
        std::string unescaped;
        unescape(unescaped, data.c_str());
        record.id = mapId;
        record.data = unescaped;
        return true;
      }
    }
  }

  return false;
}

bool StateQueuePersistence::mapGet(unsigned workspace, const std::string& mapId, RecordVector& records)
{
  if (_terminated)
    return false;
  if (!_useRedis)
  {
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
      return !records.empty();
    }
  }
  else
  {
    assert(workspace < _redisWorkspaces.size());
    RedisClient::Ptr pWorkspace = _redisWorkspaces[workspace];
    if (pWorkspace)
    {
      std::vector<std::string> items;
      if (pWorkspace->hgetall(mapId, items))
      {
        std::size_t elemCount = items.size();
        for (std::size_t i = 0; i < elemCount; i+=2)
        {
          StateQueueRecord record;
          if (i+1 < elemCount) // make sure we do not overrun the vector
          {
            record.id = items[i];
            record.data = items[i+1];
            records.push_back(record);
          }
        }
        return !records.empty();
      }
    }
  }
  return false;
}


bool StateQueuePersistence::mapGetInc(unsigned workspace, const std::string& mapId, const std::string& id, StateQueueRecord& record)
{
  if (_terminated)
    return false;
  if (!_useRedis)
  {
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
  }
  else
  {
    assert(workspace < _redisWorkspaces.size());
    RedisClient::Ptr pWorkspace = _redisWorkspaces[workspace];
    if (pWorkspace)
    {
      long long item;
      if (pWorkspace->hincrby(mapId, id, 1, item))
      {
        try
        {
          record.id = id;
          record.data = boost::lexical_cast<std::string>(item);
        }
        catch(...)
        {
          return false;
        }
        return true;
      }
    }

  }
  return false;
}


void StateQueuePersistence::mapClear(unsigned workspace, const std::string& mapId)
{
  if (_terminated)
    return;
  if (!_useRedis)
  {
    assert(workspace < _mapWorkspaces.size());
    TimedMap* pWorkspace = _mapWorkspaces[workspace];
    if (!pWorkspace)
      return;
    pWorkspace->eraseSet(mapId);
  }
  else
  {
    assert(workspace < _redisWorkspaces.size());
    RedisClient::Ptr pWorkspace = _redisWorkspaces[workspace];
    if (pWorkspace)
    {
      pWorkspace->del(mapId);
    }
  }
}

void StateQueuePersistence::mapRemove(unsigned workspace, const std::string& mapId, const std::string& dataId)
{
  if (_terminated)
    return;
  if (!_useRedis)
  {
    assert(workspace < _mapWorkspaces.size());
    TimedMap* pWorkspace = _mapWorkspaces[workspace];
    if (!pWorkspace)
      return;
    pWorkspace->eraseItem(mapId, dataId);
  }
  else
  {
    assert(workspace < _redisWorkspaces.size());
    RedisClient::Ptr pWorkspace = _redisWorkspaces[workspace];
    if (pWorkspace)
    {
      pWorkspace->hdel(mapId, dataId);
    }
  }
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
