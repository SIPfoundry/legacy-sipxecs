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

#ifndef STATEQUEUEMESSAGE_H
#define	STATEQUEUEMESSAGE_H


#include "sqa/json/reader.h"
#include "sqa/json/writer.h"
#include "sqa/json/elements.h"
#include <cassert>

class StateQueueMessage
{
public:
  enum Type
  {
    Unknown,
    Signin,
    Logout,
    Publish, /// publish an event
    PublishAndPersist, /// Publish an event a persist it right after
    Enqueue, /// enqueue a state
    EnqueueAndPublish, /// Enqueue the message then publish it as watcher data
    Pop, /// Pop the state from the queue
    Erase, /// Erase the state after a Pop
    Remove, /// Remove the record after a Set or a Persist
    Persist, /// Persist the state
    Set, /// Set a value in cache
    Get, /// Retrieve the value from cache
    MapSet, /// Set a union item
    MapGet, /// Get a union item
    MapGetInc, /// Get incremented value of an integer (this also sets the incremented value as the new value)
    MapSetMultiple, /// set multiple union items
    MapGetMultiple, /// get multiple union items
    MapRemove, /// Remove a union item
    MapClear, /// Clear all items in a union
    Ping, /// Ping request
    Pong, /// Ping response
    Data, /// Message that contains data only
    NumType
  };

  StateQueueMessage();
  StateQueueMessage(const std::string& rawData);
  StateQueueMessage(const StateQueueMessage& obj);

  StateQueueMessage& operator=(const StateQueueMessage& obj);


  Type getType() const;
  void setType(Type type);
  json::Object& object();

  bool get(const char* name, std::string& value) const;
  bool get(const char* name, int& value) const;
  void set(const char* name, const std::string& value);
  void set(const char* name, int& value);

  std::string data() const;
  bool parseData(const std::string& rawData);
  bool getMap(std::map<std::string, std::string>& smap);
protected:
  mutable Type _type;
  json::Object _object;
};

//
// Inlines
//

inline StateQueueMessage::StateQueueMessage() :
  _type(Unknown)
{
}

inline StateQueueMessage::StateQueueMessage(const std::string& rawData) :
  _type(Unknown)
{
  parseData(rawData);
}

inline StateQueueMessage::StateQueueMessage(const StateQueueMessage& obj)
{
  _type = obj._type;
  _object = obj._object;
}

inline StateQueueMessage& StateQueueMessage::operator=(const StateQueueMessage& obj)
{
  _type = obj._type;
  _object = obj._object;
  return *this;
}

inline bool StateQueueMessage::parseData(const std::string& rawData)
{
  try
  {
    std::stringstream strm;
    strm << rawData;
    json::Reader::Read(_object, strm);
  }
  catch(std::exception& error)
  {
    return false;
  }
  return getType() != Unknown;
}

inline std::string StateQueueMessage::data() const
{
  try
  {
    std::ostringstream strm;
    json::Writer::Write(_object, strm);
    return strm.str();
  }
  catch(std::exception& error)
  {
  }
  return std::string();
}


inline StateQueueMessage::Type StateQueueMessage::getType() const
{
  if (_type != Unknown && _type < NumType)
    return _type;

  try
  {
    json::String stype = _object["message-type"];
    std::string messageType = stype.Value();
    if (messageType.empty() || messageType == "unknown")
    {
      _type = Unknown;
    }
    else if (messageType == "signin")
    {
      _type = Signin;
    }
    else if (messageType == "logout")
    {
      _type = Logout;
    }
    else if (messageType == "publish")
    {
      _type = Publish;
    }
    else if (messageType == "pap")
    {
      _type = PublishAndPersist;
    }
    else if (messageType == "enqueue")
    {
      _type = Enqueue;
    }
    else if (messageType == "eap")
    {
      _type = EnqueueAndPublish;
    }
    else if (messageType == "pop")
    {
      _type = Pop;
    }
    else if (messageType == "erase")
    {
      _type = Erase;
    }
    else if (messageType == "remove")
    {
      _type = Remove;
    }
    else if (messageType == "persist")
    {
      _type = Persist;
    }
    else if (messageType == "get")
    {
      _type = Get;
    }
    else if (messageType == "set")
    {
      _type = Set;
    }
    else if (messageType == "mget")
    {
      _type = MapGet;
    }
    else if (messageType == "mgeti")
    {
      _type = MapGetInc;
    }
    else if (messageType == "mset")
    {
      _type = MapSet;
    }
    else if (messageType == "mgetm")
    {
      _type = MapGetMultiple;
    }
    else if (messageType == "msetm")
    {
      _type = MapSetMultiple;
    }
    else if (messageType == "mrem")
    {
      _type = MapRemove;
    }
    else if (messageType == "mclr")
    {
      _type = MapClear;
    }
    else if (messageType == "ping")
    {
      _type = Ping;
    }
    else if (messageType == "pong")
    {
      _type = Pong;
    }
    else if (messageType == "data")
    {
      _type = Data;
    }
    else
    {
      _type = Unknown;
    }
  }catch(...)
  {
    return Unknown;
  }
  return _type;
}

inline void StateQueueMessage::setType(Type type)
{
  assert(type < NumType);
  _type = type;
  switch(type)
  {
    case NumType:
      break;
    case Signin:
      _object["message-type"] = json::String("signin");
      break;
    case Logout:
      _object["message-type"] = json::String("logout");
      break;
    case Publish:
      _object["message-type"] = json::String("publish");
      break;
    case PublishAndPersist:
      _object["message-type"] = json::String("pap");
      break;
    case Enqueue:
      _object["message-type"] = json::String("enqueue");
      break;
    case EnqueueAndPublish:
      _object["message-type"] = json::String("eap");
      break;
    case Pop:
      _object["message-type"] = json::String("pop");
      break;
    case Erase:
      _object["message-type"] = json::String("erase");
      break;
    case Remove:
      _object["message-type"] = json::String("remove");
      break;
    case Persist:
      _object["message-type"] = json::String("persist");
      break;
    case Get:
      _object["message-type"] = json::String("get");
      break;
    case Set:
      _object["message-type"] = json::String("set");
      break;
    case MapGet:
      _object["message-type"] = json::String("mget");
      break;
    case MapGetInc:
      _object["message-type"] = json::String("mgeti");
      break;
    case MapSet:
      _object["message-type"] = json::String("mset");
      break;
    case MapGetMultiple:
      _object["message-type"] = json::String("mgetm");
      break;
    case MapSetMultiple:
      _object["message-type"] = json::String("msetm");
      break;
    case MapRemove:
      _object["message-type"] = json::String("mrem");
      break;
    case MapClear:
      _object["message-type"] = json::String("mclr");
      break;
    case Ping:
      _object["message-type"] = json::String("ping");
      break;
    case Pong:
      _object["message-type"] = json::String("pong");
      break;
     case Data:
      _object["message-type"] = json::String("data");
      break;
    case Unknown:
      _object["message-type"] = json::String("unknown");
      break;
  }
}

inline bool StateQueueMessage::get(const char* name, std::string& value) const
{
  if (_object.Find(name) == _object.End())
    return false;
  json::String v = _object[name];
  value = v.Value();
  return true;
}

inline bool StateQueueMessage::get(const char* name, int& value) const
{
  if (_object.Find(name) == _object.End())
    return false;
  json::Number v = _object[name];
  value = v.Value();
  return true;
}

inline void StateQueueMessage::set(const char* name, const std::string& value)
{
  _object[name] = json::String(value.c_str());
}

inline void StateQueueMessage::set(const char* name, int& value)
{
  _object[name] = json::Number(value);
}

inline json::Object& StateQueueMessage::object()
{
  return _object;
}

inline bool StateQueueMessage::getMap(std::map<std::string, std::string>& smap)
{
  for (json::Object::const_iterator iter = _object.Begin(); iter != _object.End(); iter++)
  {
    try
    {
      std::string n = iter->name;
      json::String v = iter->element;
      smap[n] = v.Value();
    }
    catch(...)
    {
      return false;
    }
  }
  return !smap.empty();
}

#endif	/* STATEQUEUEMESSAGE_H */

