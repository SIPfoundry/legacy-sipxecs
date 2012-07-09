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

#ifndef SQA_REDISCLIENT_H_INCLUDED
#define	SQA_REDISCLIENT_H_INCLUDED

#include "hiredis/hiredis.h"
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include "json/reader.h"
#include "json/writer.h"
#include "json/elements.h"
#include <map>



class RedisClient : boost::noncopyable
{
public:
  typedef boost::recursive_mutex mutex;
  typedef boost::lock_guard<mutex> mutex_lock;

  enum Type
  {
    TCP,
    UNIX
  };

  RedisClient() :
    _context(0),
    _type(TCP),
    _tcpHost("127.0.0.1"),
    _tcpPort(6379),
    _db(0)
  {

  }

  RedisClient(const std::string& tcpHost, int tcpPort) :
    _context(0),
    _type(TCP),
    _tcpHost(tcpHost),
    _tcpPort(tcpPort),
    _db(0)
  {
  }

  RedisClient(const std::string& unixSocketPath) :
    _context(0),
    _type(UNIX),
    _tcpPort(6379),
    _unixSocketPath(unixSocketPath),
    _db(0)
  {
  }

  virtual ~RedisClient()
  {
    disconnect();
  }

  bool connect(const std::string& password_ = "", int db = 0)
  {
    disconnect();

    std::string password = password_;

    if (password.empty())
      password = _password;

    if (db == 0)
      db = _db;

    _password = password;
    _db = db;

    mutex_lock lock(_mutex);
    if (_type == TCP)
    {
      _context = redisConnect(_tcpHost.c_str(), _tcpPort);
    }else if (_type == UNIX)
    {
      _context = redisConnectUnix(_unixSocketPath.c_str());
    }
    if (_context == 0)
      return false;

    if (_context->err)
    {
      _lastError = _context->errstr;
      return false;
    }

    redisReply *reply;
    //
    // If password is set, then we send auth
    //
    if (!password.empty())
    {
      reply = (redisReply*)redisCommand(_context,"AUTH %s", password.c_str());
      bool authenticated = false;
      if (reply)
      {
        authenticated = reply->type == REDIS_REPLY_STATUS && strcasecmp(reply->str,"ok") == 0;
        freeReplyObject(reply);
        reply = 0;
      }
      if (!authenticated)
      {
        disconnect();
        return false;
      }
    }

    if (db != 0)
    {
      reply = (redisReply*)redisCommand(_context,"SELECT %d", db);
      bool selected;
      if (reply)
      {
        selected = reply->type == REDIS_REPLY_STATUS && strcasecmp(reply->str,"ok") == 0;
        freeReplyObject(reply);
        reply = 0;
      }
      if (!selected)
      {
        disconnect();
        return false;
      }
    }

    return true;
  }

  void disconnect()
  {
    mutex_lock lock(_mutex);
    if (_context)
    {
      redisFree(_context);
      _context = 0;
    }
  }

  virtual bool set(const std::string& key, const json::Object& value, int expires = -1)
  {
    try
    {
      std::ostringstream strm;
      json::Writer::Write(value, strm);
      std::string buff = strm.str();
      return set(key, buff, expires);
    }
    catch(std::exception& error)
    {
      return false;
    }
  }

  virtual bool set(const std::string& key, const std::string& value, int expires = -1)
  {
    mutex_lock lock(_mutex);

    if (!_context)
      if (!connect())
        return false;
    if (!_context)
      return false;

    redisReply *reply;
    if (expires == -1)
      reply = (redisReply*)redisCommand(_context,"SET %s %s",key.c_str(),value.c_str());
    else
      reply = (redisReply*)redisCommand(_context,"SETEX %s %d %s",key.c_str(), expires, value.c_str());

    if (_context->err)
    {
      _lastError = _context->errstr;
      freeReplyObject(reply);

      if (_context->err == REDIS_ERR_EOF || _context->err == REDIS_ERR_IO)
      {
        //
        // Try to reconnect
        //
        if (connect())
        {
          if (expires == -1)
            reply = (redisReply*)redisCommand(_context,"SET %s %s",key.c_str(),value.c_str());
          else
            reply = (redisReply*)redisCommand(_context,"SETEX %s %d %s",key.c_str(), expires, value.c_str());
          bool ok = reply->type == REDIS_REPLY_STATUS && strcasecmp(reply->str,"ok") == 0;
          freeReplyObject(reply);
          return ok;
        }
      }
      else
      {
        //
        // This is not an io error.  bail out
        //
        return false;
      }
    }

    bool ok = false;
    if (reply)
    {
      ok = reply->type == REDIS_REPLY_STATUS && strcasecmp(reply->str,"ok") == 0;
      freeReplyObject(reply);
    }
    return ok;
  }

  virtual bool get(const std::string& key, json::Object& value) const
  {
    std::string buff;
    if (!get(key, buff))
      return false;
    try
    {
      std::stringstream strm;
      strm << buff;
      json::Reader::Read(value, strm);
    }
    catch(std::exception& error)
    {
      return false;
    }
    return true;
  }

  virtual bool get(const std::string& key, std::string& value) const
  {
    mutex_lock lock(_mutex);

    if (!_context)
      if (!const_cast<RedisClient*>(this)->connect())
        return false;
    if (!_context)
      return false;

    bool ok = false;
    redisReply* reply = (redisReply*)redisCommand(_context,"GET %s", key.c_str());

    if (_context->err)
    {
      _lastError = _context->errstr;
      const_cast<RedisClient*>(this)->freeReplyObject(reply);
      if (_context->err == REDIS_ERR_EOF || _context->err == REDIS_ERR_IO)
      {
        //
        // Try to reconnect
        //
        if (const_cast<RedisClient*>(this)->connect())
        {
          reply = (redisReply*)redisCommand(_context,"GET %s", key.c_str());

          if (reply && reply->type == REDIS_REPLY_STRING && reply->len > 0)
          {
            value = std::string(reply->str, reply->len);
            ok = true;
          }
          const_cast<RedisClient*>(this)->freeReplyObject(reply);
          return ok;
        }
      }
      else
      {
        //
        // This is not an io error.  bail out
        //
        return false;
      }
    }

    if (reply && reply->type == REDIS_REPLY_STRING && reply->len > 0)
    {
      value = std::string(reply->str, reply->len);
      ok = true;
    }
    const_cast<RedisClient*>(this)->freeReplyObject(reply);

    if (_context->err)
    {
      _lastError = _context->errstr;
      if (_lastError.empty())
        _lastError = "Unknown exception";
      //throw std::runtime_error(_lastError);
      ok = false;
    }

    return ok;
  }

  virtual bool getKeys(const std::string& pattern, std::vector<std::string>& keys)
  {
    mutex_lock lock(_mutex);

    if (!_context)
      if (!connect())
        return false;
    if (!_context)
      return false;

    bool ok = false;
    redisReply* reply = (redisReply*)redisCommand(_context,"KEYS %s", pattern.c_str());

    if (_context->err)
    {
      _lastError = _context->errstr;
      freeReplyObject(reply);
      if (_context->err == REDIS_ERR_EOF || _context->err == REDIS_ERR_IO)
      {
        //
        // Try to reconnect
        //
        if (connect())
        {
          reply = (redisReply*)redisCommand(_context,"KEYS %s", pattern.c_str());

          if (reply && reply->type == REDIS_REPLY_ARRAY && reply->elements > 0)
          {
            for (size_t i = 0; i < reply->elements; i++)
            {
              redisReply* item = reply->element[i];
              if (item && item->type == REDIS_REPLY_STRING && item->len > 0)
              {
                keys.push_back(std::string(reply->str, reply->len));
                ok = true;
              }
            }

          }
          freeReplyObject(reply);
          return ok;
        }
      }
      else
      {
        //
        // This is not an io error.  bail out
        //
        return false;
      }
    }

    if (reply && reply->type == REDIS_REPLY_ARRAY && reply->elements > 0)
    {
      for (size_t i = 0; i < reply->elements; i++)
      {
        redisReply* item = reply->element[i];
        if (item && item->type == REDIS_REPLY_STRING && item->len > 0)
        {
          keys.push_back(std::string(item->str, item->len));
          ok = true;
        }
      }
    }

    freeReplyObject(reply);

    if (_context->err)
    {
      _lastError = _context->errstr;
      if (_lastError.empty())
        _lastError = "Unknown exception";
      //throw std::runtime_error(_lastError);
      ok = false;
    }

    return ok;
  }

  virtual bool erase(const std::string& key)
  {
    mutex_lock lock(_mutex);

    if (!_context)
      if (!connect())
        return false;
    if (!_context)
      return false;

    redisReply* reply = (redisReply*)redisCommand(_context,"DEL %s", key.c_str());

    if (_context->err)
    {
      _lastError = _context->errstr;
      freeReplyObject(reply);
      if (_context->err == REDIS_ERR_EOF || _context->err == REDIS_ERR_IO)
      {
        //
        // Try to reconnect
        //
        if (connect())
        {
          reply = (redisReply*)redisCommand(_context,"DEL %s", key.c_str());

           bool ok = reply->type == REDIS_REPLY_STATUS && strcasecmp(reply->str,"ok") == 0;
          freeReplyObject(reply);
          return ok;
        }
      }
      else
      {
        //
        // This is not an io error.  bail out
        //
        return false;
      }
    }

    bool ok = false;
    if (reply)
    {
      ok = reply->type == REDIS_REPLY_STATUS && strcasecmp(reply->str,"ok") == 0;
      freeReplyObject(reply);
    }
    return ok;
  }

  void freeReplyObject(redisReply* reply)
  {
    if (reply)
      ::freeReplyObject(reply);
  }

protected:
  mutable mutex _mutex;
  redisContext* _context;
  Type _type;
  std::string _tcpHost;
  int _tcpPort;
  std::string _unixSocketPath;
  mutable std::string _lastError;
  std::string _password;
  int _db;
};

#endif

