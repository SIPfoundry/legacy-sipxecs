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

extern "C"
{
  #include <hiredis/hiredis.h>
}

#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <map>
#include "sqa/ServiceOptions.h"
#include "sqa/StateQueueMessage.h"


//
// Alloted workspaces
//
#define REDIS_STATEQUEUE_WORKSPACE 0
#define REDIS_RLSWATCHLIST_WORKSPACE 9
#define REDIS_RLSREGEVENT_WORKSPACE 10
#define REDIS_SBCDIALOG_WORKSPACE 11
#define REDIS_SBCREGISTRY_WORKSPACE 12
#define REDIS_RLSDIALOG_WORKSPACE 13
#define REDIS_RESOURCELIST_WORKSPACE 14
#define REDIS_RESOURCESTATE_WORKSPACE 15

#ifndef REDIS_EVENT_CHANNEL
#define REDIS_EVENT_CHANNEL "REDIS"
#endif

class RedisClient : boost::noncopyable
{
public:
  typedef boost::recursive_mutex mutex;
  typedef boost::lock_guard<mutex> mutex_lock;
  typedef boost::shared_ptr<RedisClient> Ptr;
  
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

  ~RedisClient()
  {
    disconnect();
  }

protected:

  redisReply* execute(const std::vector<std::string>& args)
  {
    char** argv = (char**)std::malloc((args.size() + 1) * sizeof(char*));
    std::size_t i=0;
    for(std::vector<std::string>::const_iterator iter = args.begin();
        iter != args.end();
        iter++, ++i)
    {
      std::string arg = *iter;
      argv[i] = (char*)std::malloc((arg.length()+1) * sizeof(char));
      std::strcpy(argv[i], arg.c_str());
    }
    argv[args.size()] = NULL; // argv must be NULL terminated

    redisReply* reply = execute(args.size(), argv);

    for (i = 0; i < args.size(); i++)
      free((argv)[i]);
    free(argv);

    return reply;
  }

  redisReply* execute(int argc, char** argv)
  {
    mutex_lock lock(_mutex);

    if (!_context)
      if (!connect())
        return 0;
    if (!_context)
      return 0;

    redisReply* reply = 0;
    reply = (redisReply*)redisCommandArgv(_context, argc, (const char**)argv, 0);

    if (_context->err)
    {
      _lastError = _context->errstr;
       if (_lastError.empty())
        _lastError = "Unknown exception";
      freeReply(reply);
      reply = 0;
      if (_context->err == REDIS_ERR_EOF || _context->err == REDIS_ERR_IO)
      {
        //
        // Try to reconnect
        //
        if (connect())
          reply = (redisReply*)redisCommandArgv(_context, argc, (const char**)argv, 0);
      }
    }

    return reply;
  }

  std::string getReplyString(const std::vector<std::string>& args) const
  {
    redisReply* reply = const_cast<RedisClient*>(this)->execute(args);
    std::string value;
    if (reply && (reply->type == REDIS_REPLY_STRING || reply->type == REDIS_REPLY_ERROR) && reply->len > 0)
    {
      value = std::string(reply->str, reply->len);
    }
    else if (reply && reply->type == REDIS_REPLY_INTEGER)
    {
      try
      {
        value = boost::lexical_cast<std::string>(reply->integer);
      }
      catch(...)
      {
      }
    }

    if (reply)
      const_cast<RedisClient*>(this)->freeReply(reply);

    return value;
  }

  bool getReplyInt(const std::vector<std::string>& args, long long& result) const
  {
    redisReply* reply = const_cast<RedisClient*>(this)->execute(args);
    if (reply && reply->type == REDIS_REPLY_INTEGER)
    {
      try
      {
        result = reply->integer;
        const_cast<RedisClient*>(this)->freeReply(reply);
        return true;
      }
      catch(...)
      {
        return false;
      }
    }
    return false;
  }

  std::vector<std::string> getReplyStringArray(const std::vector<std::string>& args) const
  {
    redisReply* reply = const_cast<RedisClient*>(this)->execute(args);
    std::vector<std::string> array;

    if (reply && reply->type == REDIS_REPLY_ARRAY && reply->elements > 0)
    {
      for (size_t i = 0; i < reply->elements; i++)
      {
        redisReply* item = reply->element[i];
        if (item && item->type == REDIS_REPLY_STRING && item->len > 0)
        {
          array.push_back(std::string(item->str, item->len));
        }
        else if (item && item->type == REDIS_REPLY_INTEGER)
        {
          try
          {
            array.push_back(boost::lexical_cast<std::string>(item->integer));
          }
          catch(...)
          {
          }
        }

      }
      const_cast<RedisClient*>(this)->freeReply(reply);
    }
    return array;
  }

  std::string getStatusString(const std::vector<std::string>& args) const
  {
    redisReply* reply = const_cast<RedisClient*>(this)->execute(args);
    std::string value;
    if (reply)
    {
        if ((reply->type == REDIS_REPLY_STATUS || reply->type == REDIS_REPLY_ERROR) && reply->len > 0)
        {
          value = std::string(reply->str, reply->len);
        }

        if (reply->type == REDIS_REPLY_INTEGER)
        {
          try
          {
            value  = boost::lexical_cast<std::string>(reply->integer);
          }
          catch(...)
          {
          }
        }

      const_cast<RedisClient*>(this)->freeReply(reply);
    }

    return value;
  }
public:

  bool connect(int db)
  {
    try
    {
      std::ostringstream sqaconfig;
      sqaconfig << SIPX_CONFDIR << "/" << "redis-client.ini";
      ServiceOptions configOptions(sqaconfig.str());
      if (configOptions.parseOptions())
      {
        bool enabled = false;
        if (configOptions.getOption("enabled", enabled, enabled) && enabled)
        {
          configOptions.getOption("tcp-address", _tcpHost);
          configOptions.getOption("tcp-port", _tcpPort);
        }
      }
    }
    catch(...)
    {
    }

    _type = TCP;
    return connect("", db);
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
        freeReply(reply);
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
      bool selected = false;
      if (reply)
      {
        selected = reply->type == REDIS_REPLY_STATUS && strcasecmp(reply->str,"ok") == 0;
        freeReply(reply);
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

  bool set(const std::string& key, const StateQueueMessage& value, int expires = -1)
  {
    return set(key, value.data(), expires);
  }

  bool set(const std::string& key, const std::string& value, int expires = -1)
  {
    std::vector<std::string> args;
    if (expires == -1)
    {
      args.push_back("SET");
      args.push_back(key);
      args.push_back(value);
    }
    else
    {
      try
      {
        args.push_back("SETEX");
        args.push_back(key);
        args.push_back(boost::lexical_cast<std::string>(expires));
        args.push_back(value);
      }
      catch(...){}

    }
    std::string status = getStatusString(args);
    return status == "ok";
  }
  
  bool hset(const std::string& key, const std::string& name, const std::string& value)
  {
    std::vector<std::string> args;
    args.push_back("HSET");
    args.push_back(key);
    args.push_back(name);
    args.push_back(value);
    std::string status = getStatusString(args);
    return status == "0" || status == "1";
  }

  bool hincrby(const std::string& key, const std::string& name, int increment, long long& result)
  {
    try
    {
      std::vector<std::string> args;
      args.push_back("HINCRBY");
      args.push_back(key);
      args.push_back(name);
      args.push_back(boost::lexical_cast<std::string>(increment));
      return getReplyInt(args, result);
    }
    catch(...)
    {
      return false;
    }
  }

  bool hmset(const std::string& key, const std::map<std::string, std::string>& hmap)
  {
    std::vector<std::string> args;
    args.push_back("HMSET");

    for (std::map<std::string, std::string>::const_iterator iter = hmap.begin(); iter != hmap.end(); iter++)
    {
      args.push_back(iter->first);
      args.push_back(iter->second);
    }
    std::string status = getStatusString(args);
    return status == "0" || status == "1";
  }

  bool get(const std::string& key, StateQueueMessage& value) const
  {
    std::string buff;
    if (!get(key, buff))
      return false;
    return value.parseData(buff, true);
  }

  bool get(const std::string& key, std::string& value) const
  {
    std::vector<std::string> args;
    args.push_back("GET");
    args.push_back(key);
    value = getReplyString(args);
    return !value.empty();
  }

  bool hget(const std::string& key, const std::string& name, std::string& value) const
  {
    std::vector<std::string> args;
    args.push_back("HGET");
    args.push_back(key);
    args.push_back(name);
    value = getReplyString(args);
    return !value.empty();
  }

  bool hgetall(const std::string& key, std::vector<std::string>& value) const
  {
    std::vector<std::string> args;
    args.push_back("HGETALL");
    args.push_back(key);
    value = getReplyStringArray(args);
    return !value.empty();
  }

  bool hmget(const std::string& key, const std::vector<std::string>& fields, std::vector<std::string>& value) const
  {
    std::vector<std::string> args;
    args.push_back("HMGET");
    args.push_back(key);
    for (std::vector<std::string>::const_iterator iter = fields.begin(); iter != fields.end(); iter++)
      args.push_back(*iter);
    value = getReplyStringArray(args);
    return !value.empty();
  }

  bool getKeys(const std::string& pattern, std::vector<std::string>& keys)
  {
   std::vector<std::string> args;
    args.push_back("KEYS");
    args.push_back(pattern);
    keys = getReplyStringArray(args);
    return !keys.empty();
  }

  bool smembers(const std::string& key, std::vector<std::string>& members)
  {
    std::vector<std::string> args;
    args.push_back("SMEMBERS");
    args.push_back(key);
    members = getReplyStringArray(args);
    return !members.empty();
  }

  long long sadd(const std::string& key, const std::vector<std::string>& members)
  {
    std::vector<std::string> args;
    args.push_back("SADD");
    args.push_back(key);

    for (std::vector<std::string>::const_iterator iter = members.begin(); iter != members.end(); iter++)
      args.push_back(*iter);
    long long result = 0;
    getReplyInt(args, result);
    return result;
  }

  long long scard(const std::string& key)
  {
    std::vector<std::string> args;
    args.push_back("SCARD");
    args.push_back(key);
    long long result = 0;
    getReplyInt(args, result);
    return result;
  }

  long long srem(const std::string& key, const std::vector<std::string>& members)
  {
    std::vector<std::string> args;
    args.push_back("SREM");
    args.push_back(key);

    for (std::vector<std::string>::const_iterator iter = members.begin(); iter != members.end(); iter++)
      args.push_back(*iter);
    long long result = 0;
    getReplyInt(args, result);
    return result;
  }

  std::string spop(const std::string& key)
  {
    std::vector<std::string> args;
    args.push_back("SPOP");
    args.push_back(key);
    return getReplyString(args);
  }


  bool del(const std::string& key)
  {
    std::vector<std::string> args;
    args.push_back("DEL");
    args.push_back(key);
    long long result = 0;
    return getReplyInt(args, result) && result > 0;
  }

  bool hdel(const std::string& key, const std::string& field)
  {
    std::vector<std::string> args;
    args.push_back("HDEL");
    args.push_back(key);
    args.push_back(field);
    long long result = 0;
    return getReplyInt(args, result) && result > 0;
  }

  long long publish(const std::string& channel, const std::string& data)
  {
    std::vector<std::string> args;
    args.push_back("PUBLISH");
    args.push_back(channel);
    args.push_back(data);
    long long result = 0;
    getReplyInt(args, result);
    return result;
  }

  void freeReply(redisReply* reply)
  {
    if (reply)
      freeReplyObject(reply);
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

