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

#ifndef SQACLIENT_H
#define	SQACLIENT_H

#ifndef EXCLUDE_SQA_INLINES
#include "sqa/StateQueueClient.h"
#include <boost/lexical_cast.hpp>
#endif

#include <map>
#include <vector>
#include <string>

#ifdef SWIG
%module sqaclient
%{
#include "sqaclient.h"
%}
%newobject SQAWatcher::watch();
%newobject *::get;
%newobject *::mget;
%newobject SQAWorker::fetchTask();

%include "std_vector.i"
%include "std_string.i"
%include "std_map.i"

namespace std
{
   %template(StringToStringMap) map<string, string>;
}

#endif

class SQALogger
{
public:
  SQALogger();
  void initialize(const char* file, int level);
  std::string getHostName();
  std::string getCurrentTask();
  std::string getProcessName();
protected:
  std::string hostName;
  std::string taskName;
  std::string processName;
};

class SQAEvent
{
public:
  SQAEvent();
  SQAEvent(const SQAEvent& data);
  SQAEvent(const std::string& id_, const std::string& data_);
  ~SQAEvent();

  char* id;
  char* data;
  int data_len;
  int id_len;
};

class SQAWatcher
{
public:
  SQAWatcher(
    const char* applicationId, // Unique application ID that will identify this watcher to SQA
    const char* serviceAddress, // The IP address of the SQA
    const char* servicePort, // The port where SQA is listening for connections
    const char* eventId, // Event ID of the event being watched. Example: "sqa.not"
    int poolSize, // Number of active connections to SQA
    int readTimeout, // read timeout for the control socket
    int writeTimeout // write timeout for the control socket
  );

  SQAWatcher(
    const char* applicationId, // Unique application ID that will identify this watcher to SQA
    const char* eventId, // Event ID of the event being watched. Example: "sqa.not"
    int poolSize, // Number of active connections to SQA
    int readTimeout, // read timeout for the control socket
    int writeTimeout // write timeout for the control socket
  );

  ~SQAWatcher();

  //
  // Returns the local IP address of the client
  //
  const char* getLocalAddress();

  //
  // Returns true if the client is connected to SQA
  //
  bool isConnected();

  //
  // Terminate the event loop
  //
  void terminate();

  //
  // Returns the next event published by SQA.  This Function
  // will block if there is no event in queue
  //
  SQAEvent* watch();

  //
  // Set a value in the event queue workspace
  //
  void set(int workspace, const char* name, const char* data, int expires);
  //
  // Get a value from the event queue workspace
  //
  char* get(int workspace, const char* name);
  //
  // Set the value of a map item.  If the item or map does not exist
  // they will be created
  //
  void mset(int workspace, const char* mapId, const char* dataId, const char* data, int expires);
  //
  // Return a particular map element
  //
  char* mget(int workspace, const char* mapId, const char* dataId);
  //
  // Increment the value of an integer belong to a map
  //
  bool mgeti(int workspace, const char* mapId, const char* dataId, int& data);
  //
  // Get a map of string to string values stored in sqa
  //
  std::map<std::string, std::string> mgetAll(int workspace, const char* name);
private:
  SQAWatcher(const SQAWatcher& copy);
  uintptr_t _connection;
};

class SQAPublisher
{
public:
  SQAPublisher(
    const char* applicationId, // Unique application ID that will identify this watcher to SQA
    const char* serviceAddress, // The IP address of the SQA
    const char* servicePort, // The port where SQA is listening for connections
    int poolSize, // Number of active connections to SQA
    int readTimeout, // read timeout for the control socket
    int writeTimeout // write timeout for the control socket
  );

  SQAPublisher(
    const char* applicationId, // Unique application ID that will identify this watcher to SQA
    int poolSize, // Number of active connections to SQA
    int readTimeout, // read timeout for the control socket
    int writeTimeout // write timeout for the control socket
  );

  ~SQAPublisher();

  bool isConnected();

  bool publish(const char* id, const char* data, bool noresponse);

  bool publish(const char* id, const char* data, int len, bool noresponse);

  bool publishAndPersist(int workspace, const char* id, const char* data, int expires);

  //
  // Returns the local IP address of the client
  //
  const char* getLocalAddress();

  //
  // Set a value in the event queue workspace
  //
  void set(int workspace, const char* name, const char* data, int expires);
  //
  // Get a value from the event queue workspace
  //
  char* get(int workspace, const char* name);
  //
  // Set the value of a map item.  If the item or map does not exist
  // they will be created
  //
  void mset(int workspace, const char* mapId, const char* dataId, const char* data, int expires);
  //
  // Return a particular map element
  //
  char* mget(int workspace, const char* mapId, const char* dataId);
  //
  // Increment the value of an integer belong to a map
  //
  bool mgeti(int workspace, const char* mapId, const char* dataId, int& data);
  //
  // Get a map of string to string values stored in sqa
  //
  std::map<std::string, std::string> mgetAll(int workspace, const char* name);
private:
  SQAPublisher(const SQAPublisher& copy);
  uintptr_t _connection;
};

class SQAWorker
{
public:
  SQAWorker(
    const char* applicationId, // Unique application ID that will identify this watcher to SQA
    const char* serviceAddress, // The IP address of the SQA
    const char* servicePort, // The port where SQA is listening for connections
    const char* eventId, // Event ID of the event being watched. Example: "sqa.not"
    int poolSize, // Number of active connections to SQA
    int readTimeout, // read timeout for the control socket
    int writeTimeout // write timeout for the control socket
  );

  SQAWorker(
    const char* applicationId, // Unique application ID that will identify this watcher to SQA
    const char* eventId, // Event ID of the event being watched. Example: "sqa.not"
    int poolSize, // Number of active connections to SQA
    int readTimeout, // read timeout for the control socket
    int writeTimeout // write timeout for the control socket
  );

  ~SQAWorker();

  //
  // Returns the local IP address of the client
  //
  const char* getLocalAddress();

  //
  // Returns true if the client is connected to SQA
  //
  bool isConnected();

  //
  // Returns the next event published by SQA.  This Function
  // will block if there is no event in queue
  //
  SQAEvent* fetchTask();

  //
  // Delete the task from the cache.  This must be called after fetchTask()
  // work is done
  //
  void deleteTask(const char* id);

  //
  // Set a value in the event queue workspace
  //
  void set(int workspace, const char* name, const char* data, int expires);
  //
  // Get a value from the event queue workspace
  //
  char* get(int workspace, const char* name);
  //
  // Set the value of a map item.  If the item or map does not exist
  // they will be created
  //
  void mset(int workspace, const char* mapId, const char* dataId, const char* data, int expires);
  //
  // Return a particular map element
  //
  char* mget(int workspace, const char* mapId, const char* dataId);
  //
  // Increment the value of an integer belong to a map
  //
  bool mgeti(int workspace, const char* mapId, const char* dataId, int& data);
  //
  // Get a map of string to string values stored in sqa
  //
  std::map<std::string, std::string> mgetAll(int workspace, const char* name);

private:
  SQAWorker(const SQAWorker& copy);
  uintptr_t _connection;
};

class SQADealer
{
public:
  SQADealer(
    const char* applicationId, // Unique application ID that will identify this watcher to SQA
    const char* serviceAddress, // The IP address of the SQA
    const char* servicePort, // The port where SQA is listening for connections
    const char* eventId, // Event ID of the event being watched. Example: "sqa.not"
    int poolSize, // Number of active connections to SQA
    int readTimeout, // read timeout for the control socket
    int writeTimeout // write timeout for the control socket
  );

  SQADealer(
    const char* applicationId, // Unique application ID that will identify this watcher to SQA
    const char* eventId, // Event ID of the event being watched. Example: "sqa.not"
    int poolSize, // Number of active connections to SQA
    int readTimeout, // read timeout for the control socket
    int writeTimeout // write timeout for the control socket
  );

  ~SQADealer();

  //
  // Returns the local IP address of the client
  //
  const char* getLocalAddress();

  //
  // Returns true if the client is connected to SQA
  //
  bool isConnected();


  //
  // Deal a new task
  //
  bool deal(const char* data, int expires);

  //
  // Deal and publish a task
  //
  bool dealAndPublish(const char* data, int expires);

  //
  // Set a value in the event queue workspace
  //
  void set(int workspace, const char* name, const char* data, int expires);
  //
  // Get a value from the event queue workspace
  //
  char* get(int workspace, const char* name);
  //
  // Set the value of a map item.  If the item or map does not exist
  // they will be created
  //
  void mset(int workspace, const char* mapId, const char* dataId, const char* data, int expires);
  //
  // Return a particular map element
  //
  char* mget(int workspace, const char* mapId, const char* dataId);
  //
  // Increment the value of an integer belong to a map
  //
  bool mgeti(int workspace, const char* mapId, const char* dataId, int& data);
  //
  // Get a map of string to string values stored in sqa
  //
  std::map<std::string, std::string> mgetAll(int workspace, const char* name);
private:
  SQADealer(const SQADealer& copy);
  uintptr_t _connection;
};


#ifndef EXCLUDE_SQA_INLINES
//
// Inline implementation of SQAEvent class
//
inline SQAEvent::SQAEvent() :
  id(0),
  data(0),
  data_len(0),
  id_len(0)
{
}

inline SQAEvent::~SQAEvent()
{
  if (NULL != id)
  {
    delete [] id;
    id = NULL;
  }

  if (NULL != data)
  {
    delete [] data;
    data = NULL;
  }
}

inline SQAEvent::SQAEvent(const SQAEvent& ev)
{
  id_len = ev.id_len;
  id = new char[id_len + 1];
  ::memcpy(id, ev.id, id_len);
  id[id_len] = '\0';

  data_len = ev.data_len;
  data = new char[data_len + 1];
  ::memcpy(data, ev.data, data_len);
  data[data_len] = '\0';
}

inline SQAEvent::SQAEvent(const std::string& id_, const std::string& data_)
{
  id_len = id_.size();
  id = new char[id_len + 1];
  std::copy(id_.begin(), id_.end(), id);
  id[id_len] = '\0';

  data_len = data_.size();
  data = new char[data_len + 1];
  std::copy(data_.begin(), data_.end(), data);
  data[data_len] = '\0';
}

//
// Inline implementation for SQAWatcher class
//
inline SQAWatcher::SQAWatcher(
  const char* applicationId, // Unique application ID that will identify this watcher to SQA
  const char* serviceAddress, // The IP address of the SQA
  const char* servicePort, // The port where SQA is listening for connections
  const char* eventId, // Event ID of the event being watched. Example: "reg"
  int poolSize, // Number of active connections to SQA
  int readTimeout, // read timeout for the control socket
  int writeTimeout // write timeout for the control socket
)
{
  _connection = (uintptr_t)(new StateQueueClient(
          StateQueueClient::Watcher,
          applicationId,
          serviceAddress,
          servicePort,
          eventId,
          poolSize,
          readTimeout,
          writeTimeout));
}

inline SQAWatcher::SQAWatcher(
  const char* applicationId, // Unique application ID that will identify this watcher to SQA
  const char* eventId, // Event ID of the event being watched. Example: "reg"
  int poolSize, // Number of active connections to SQA
  int readTimeout, // read timeout for the control socket
  int writeTimeout // write timeout for the control socket
)
{
  _connection = (uintptr_t)(new StateQueueClient(
          StateQueueClient::Watcher,
          applicationId,
          eventId,
          poolSize,
          readTimeout,
          writeTimeout));
}

inline SQAWatcher::SQAWatcher(const SQAWatcher& copy)
{
}

inline SQAWatcher::~SQAWatcher()
{
  delete reinterpret_cast<StateQueueClient*>(_connection);
}

inline const char* SQAWatcher::getLocalAddress()
{
  return reinterpret_cast<StateQueueClient*>(_connection)->getLocalAddress().c_str();
}

inline bool SQAWatcher::isConnected()
{
  return reinterpret_cast<StateQueueClient*>(_connection)->isConnected();
}

inline void SQAWatcher::terminate()
{
  reinterpret_cast<StateQueueClient*>(_connection)->terminate();
}

inline SQAEvent* SQAWatcher::watch()
{
  std::string id;
  std::string data;
  if (!reinterpret_cast<StateQueueClient*>(_connection)->watch(id, data))
    return 0;

  SQAEvent* pEvent = new SQAEvent(id, data);

  return pEvent;
}

//
  // Set a value in the event queue workspace
  //
inline void SQAWatcher::set(int workspace, const char* name, const char* data, int expires)
{
  reinterpret_cast<StateQueueClient*>(_connection)->set(workspace, name, data, expires);
}
  //
  // Get a value from the event queue workspace
  //
inline char* SQAWatcher::get(int workspace, const char* name)
{
  std::string data;
  if (!reinterpret_cast<StateQueueClient*>(_connection)->get(workspace, name, data))
    return 0;
  char* buff = (char*)malloc(data.size() + 1);
  ::memset(buff, 0x00, data.size() + 1);
  ::memcpy(buff, data.c_str(), data.size() + 1);
  return buff;
}

inline void SQAWatcher::mset(int workspace, const char* mapId, const char* dataId, const char* data, int expires)
{
  reinterpret_cast<StateQueueClient*>(_connection)->mset(workspace, mapId, dataId, data, expires);
}

inline char* SQAWatcher::mget(int workspace, const char* mapId, const char* dataId)
{
  std::string data;
  if (!reinterpret_cast<StateQueueClient*>(_connection)->mget(workspace, mapId, dataId, data))
    return 0;
  char* buff = (char*)malloc(data.size() + 1);
  ::memset(buff, 0x00, data.size() + 1);
  ::memcpy(buff, data.c_str(), data.size() + 1);
  return buff;
}

inline bool SQAWatcher::mgeti(int workspace, const char* mapId, const char* dataId, int& incremented)
{
  std::string data;
  if (!reinterpret_cast<StateQueueClient*>(_connection)->mgeti(workspace, mapId, dataId, data))
    return false;
  try
  {
    incremented = boost::lexical_cast<int>(data);
    return true;
  }
  catch(...)
  {
    return false;
  }
}

inline std::map<std::string, std::string> SQAWatcher::mgetAll(int workspace, const char* name)
{
  std::map<std::string, std::string> smap;
  if (!reinterpret_cast<StateQueueClient*>(_connection)->mgetm(workspace, name, smap));
  return smap;
}

//
// Inline implmentation of the SQA Publisher class
//
inline SQAPublisher::SQAPublisher(
  const char* applicationId, // Unique application ID that will identify this watcher to SQA
  const char* serviceAddress, // The IP address of the SQA
  const char* servicePort, // The port where SQA is listening for connections
  int poolSize, // Number of active connections to SQA
  int readTimeout, // read timeout for the control socket
  int writeTimeout // write timeout for the control socket
)
{
  _connection = (uintptr_t)(new StateQueueClient(
          StateQueueClient::Publisher,
          applicationId,
          serviceAddress,
          servicePort,
          "publisher",
          poolSize,
          readTimeout,
          writeTimeout));
}

inline SQAPublisher::SQAPublisher(
  const char* applicationId, // Unique application ID that will identify this watcher to SQA
  int poolSize, // Number of active connections to SQA
  int readTimeout, // read timeout for the control socket
  int writeTimeout // write timeout for the control socket
)
{
  _connection = (uintptr_t)(new StateQueueClient(
          StateQueueClient::Publisher,
          applicationId,
          "publisher",
          poolSize,
          readTimeout,
          writeTimeout));
}

inline SQAPublisher::SQAPublisher(const SQAPublisher& copy)
{
}

inline SQAPublisher::~SQAPublisher()
{
  delete reinterpret_cast<StateQueueClient*>(_connection);
}

inline const char* SQAPublisher::getLocalAddress()
{
  return reinterpret_cast<StateQueueClient*>(_connection)->getLocalAddress().c_str();
}

inline bool SQAPublisher::isConnected()
{
  return reinterpret_cast<StateQueueClient*>(_connection)->isConnected();
}

inline bool SQAPublisher::publish(const char* id, const char* data, bool noresponse)
{
  return reinterpret_cast<StateQueueClient*>(_connection)->publish(id, data, noresponse);
}

inline bool SQAPublisher::publish(const char* id, const char* data, int len, bool noresponse)
{
  return reinterpret_cast<StateQueueClient*>(_connection)->publish(id, data, len, noresponse);
}

inline bool SQAPublisher::publishAndPersist(int workspace, const char* id, const char* data, int expires)
{
  return reinterpret_cast<StateQueueClient*>(_connection)->publishAndPersist(workspace, id, data, expires);
}

inline void SQAPublisher::set(int workspace, const char* name, const char* data, int expires)
{
  reinterpret_cast<StateQueueClient*>(_connection)->set(workspace, name, data, expires);
}
  //
  // Get a value from the event queue workspace
  //
inline char* SQAPublisher::get(int workspace, const char* name)
{
  std::string data;
  if (!reinterpret_cast<StateQueueClient*>(_connection)->get(workspace, name, data))
    return 0;
  char* buff = (char*)malloc(data.size() + 1);
  ::memset(buff, 0x00, data.size() + 1);
  ::memcpy(buff, data.c_str(), data.size() + 1);
  return buff;
}

inline void SQAPublisher::mset(int workspace, const char* mapId, const char* dataId, const char* data, int expires)
{
  reinterpret_cast<StateQueueClient*>(_connection)->mset(workspace, mapId, dataId, data, expires);
}

inline char* SQAPublisher::mget(int workspace, const char* mapId, const char* dataId)
{
  std::string data;
  if (!reinterpret_cast<StateQueueClient*>(_connection)->mget(workspace, mapId, dataId, data))
    return 0;
  char* buff = (char*)malloc(data.size() + 1);
  ::memset(buff, 0x00, data.size() + 1);
  ::memcpy(buff, data.c_str(), data.size() + 1);
  return buff;
}

inline bool SQAPublisher::mgeti(int workspace, const char* mapId, const char* dataId, int& incremented)
{
  std::string data;
  if (!reinterpret_cast<StateQueueClient*>(_connection)->mgeti(workspace, mapId, dataId, data))
    return 0;
  try
  {
    incremented = boost::lexical_cast<int>(data);
    return true;
  }
  catch(...)
  {
    return false;
  }
}

inline std::map<std::string, std::string> SQAPublisher::mgetAll(int workspace, const char* name)
{
  std::map<std::string, std::string> smap;
  if (!reinterpret_cast<StateQueueClient*>(_connection)->mgetm(workspace, name, smap));
  return smap;
}
//
// Inline implementation for SQA Dealer class
//
inline SQADealer::SQADealer(
  const char* applicationId, // Unique application ID that will identify this watcher to SQA
  const char* serviceAddress, // The IP address of the SQA
  const char* servicePort, // The port where SQA is listening for connections
  const char* eventId, // Event ID of the event being watched. Example: "sqa.not"
  int poolSize, // Number of active connections to SQA
  int readTimeout, // read timeout for the control socket
  int writeTimeout // write timeout for the control socket
)
{
  _connection = (uintptr_t)(new StateQueueClient(
          StateQueueClient::Publisher,
          applicationId,
          serviceAddress,
          servicePort,
          eventId,
          poolSize,
          readTimeout,
          writeTimeout));
}

inline SQADealer::SQADealer(
  const char* applicationId, // Unique application ID that will identify this watcher to SQA
  const char* eventId, // Event ID of the event being watched. Example: "sqa.not"
  int poolSize, // Number of active connections to SQA
  int readTimeout, // read timeout for the control socket
  int writeTimeout // write timeout for the control socket
)
{
  _connection = (uintptr_t)(new StateQueueClient(
          StateQueueClient::Publisher,
          applicationId,
          eventId,
          poolSize,
          readTimeout,
          writeTimeout));
}

inline SQADealer::SQADealer(const SQADealer& copy)
{
}

inline SQADealer::~SQADealer()
{
  delete reinterpret_cast<StateQueueClient*>(_connection);
}

inline const char* SQADealer::getLocalAddress()
{
  return reinterpret_cast<StateQueueClient*>(_connection)->getLocalAddress().c_str();
}

inline bool SQADealer::isConnected()
{
  return reinterpret_cast<StateQueueClient*>(_connection)->isConnected();
}

inline bool SQADealer::deal(const char* data, int expires)
{
  return reinterpret_cast<StateQueueClient*>(_connection)->enqueue(data, expires);
}

inline bool SQADealer::dealAndPublish(const char* data, int expires)
{
  return reinterpret_cast<StateQueueClient*>(_connection)->enqueue(data, expires, true);
}

inline void SQADealer::set(int workspace, const char* name, const char* data, int expires)
{
  reinterpret_cast<StateQueueClient*>(_connection)->set(workspace, name, data, expires);
}
  //
  // Get a value from the event queue workspace
  //
inline char* SQADealer::get(int workspace, const char* name)
{
  std::string data;
  if (!reinterpret_cast<StateQueueClient*>(_connection)->get(workspace, name, data))
    return 0;
  char* buff = (char*)malloc(data.size() + 1);
  ::memset(buff, 0x00, data.size() + 1);
  ::memcpy(buff, data.c_str(), data.size() + 1);
  return buff;
}

inline void SQADealer::mset(int workspace, const char* mapId, const char* dataId, const char* data, int expires)
{
  reinterpret_cast<StateQueueClient*>(_connection)->mset(workspace, mapId, dataId, data, expires);
}

inline char* SQADealer::mget(int workspace, const char* mapId, const char* dataId)
{
  std::string data;
  if (!reinterpret_cast<StateQueueClient*>(_connection)->mget(workspace, mapId, dataId, data))
    return 0;
  char* buff = (char*)malloc(data.size() + 1);
  ::memset(buff, 0x00, data.size() + 1);
  ::memcpy(buff, data.c_str(), data.size() + 1);
  return buff;
}

inline bool SQADealer::mgeti(int workspace, const char* mapId, const char* dataId, int& incremented)
{
  std::string data;
  if (!reinterpret_cast<StateQueueClient*>(_connection)->mgeti(workspace, mapId, dataId, data))
    return 0;
  try
  {
    incremented = boost::lexical_cast<int>(data);
    return true;
  }
  catch(...)
  {
    return false;
  }
}

inline std::map<std::string, std::string> SQADealer::mgetAll(int workspace, const char* name)
{
  std::map<std::string, std::string> smap;
  if (!reinterpret_cast<StateQueueClient*>(_connection)->mgetm(workspace, name, smap));
  return smap;
}

//
// Inline implementation for SQAWorker class
//
inline SQAWorker::SQAWorker(
  const char* applicationId, // Unique application ID that will identify this watcher to SQA
  const char* serviceAddress, // The IP address of the SQA
  const char* servicePort, // The port where SQA is listening for connections
  const char* eventId, // Event ID of the event being watched. Example: "sqa.not"
  int poolSize, // Number of active connections to SQA
  int readTimeout, // read timeout for the control socket
  int writeTimeout // write timeout for the control socket
)
{
  _connection = (uintptr_t)(new StateQueueClient(
          StateQueueClient::Worker,
          applicationId,
          serviceAddress,
          servicePort,
          eventId,
          poolSize,
          readTimeout,
          writeTimeout));
}

inline SQAWorker::SQAWorker(
  const char* applicationId, // Unique application ID that will identify this watcher to SQA
  const char* eventId, // Event ID of the event being watched. Example: "sqa.not"
  int poolSize, // Number of active connections to SQA
  int readTimeout, // read timeout for the control socket
  int writeTimeout // write timeout for the control socket
)
{
  _connection = (uintptr_t)(new StateQueueClient(
          StateQueueClient::Worker,
          applicationId,
          eventId,
          poolSize,
          readTimeout,
          writeTimeout));
}

inline SQAWorker::SQAWorker(const SQAWorker& copy)
{
}

inline SQAWorker::~SQAWorker()
{
  delete reinterpret_cast<StateQueueClient*>(_connection);
}

inline const char* SQAWorker::getLocalAddress()
{
  return reinterpret_cast<StateQueueClient*>(_connection)->getLocalAddress().c_str();
}

inline bool SQAWorker::isConnected()
{
  return reinterpret_cast<StateQueueClient*>(_connection)->isConnected();
}

inline SQAEvent* SQAWorker::fetchTask()
{
  std::string id;
  std::string data;
  SQAEvent* pEvent = new SQAEvent();
  if (!reinterpret_cast<StateQueueClient*>(_connection)->pop(id, data))
    return pEvent;

  pEvent->id = (char*)malloc(id.size());
  ::memcpy(pEvent->id, id.data(), id.size());

  pEvent->data = (char*)malloc(data.size());
  ::memcpy(pEvent->data, data.data(), data.size());

  pEvent->data_len = data.size();
  pEvent->id_len = id.size();
  return pEvent;
}


inline void SQAWorker::deleteTask(const char* id)
{
  reinterpret_cast<StateQueueClient*>(_connection)->erase(id);
}

inline void SQAWorker::set(int workspace, const char* name, const char* data, int expires)
{
  reinterpret_cast<StateQueueClient*>(_connection)->set(workspace, name, data, expires);
}
  //
  // Get a value from the event queue workspace
  //
inline char* SQAWorker::get(int workspace, const char* name)
{
  std::string data;
  if (!reinterpret_cast<StateQueueClient*>(_connection)->get(workspace, name, data))
    return 0;
  char* buff = (char*)malloc(data.size() + 1);
  ::memset(buff, 0x00, data.size() + 1);
  ::memcpy(buff, data.c_str(), data.size() + 1);
  return buff;
}

inline void SQAWorker::mset(int workspace, const char* mapId, const char* dataId, const char* data, int expires)
{
  reinterpret_cast<StateQueueClient*>(_connection)->mset(workspace, mapId, dataId, data, expires);
}

inline char* SQAWorker::mget(int workspace, const char* mapId, const char* dataId)
{
  std::string data;
  if (!reinterpret_cast<StateQueueClient*>(_connection)->mget(workspace, mapId, dataId, data))
    return 0;
  char* buff = (char*)malloc(data.size() + 1);
  ::memset(buff, 0x00, data.size() + 1);
  ::memcpy(buff, data.c_str(), data.size() + 1);
  return buff;
}

inline bool SQAWorker::mgeti(int workspace, const char* mapId, const char* dataId, int& incremented)
{
  std::string data;
  if (!reinterpret_cast<StateQueueClient*>(_connection)->mgeti(workspace, mapId, dataId, data))
    return 0;
  try
  {
    incremented = boost::lexical_cast<int>(data);
    return true;
  }
  catch(...)
  {
    return false;
  }
}

inline std::map<std::string, std::string> SQAWorker::mgetAll(int workspace, const char* name)
{
  std::map<std::string, std::string> smap;
  if (!reinterpret_cast<StateQueueClient*>(_connection)->mgetm(workspace, name, smap));
  return smap;
}

inline SQALogger::SQALogger()
{
  hostName = "sqa";
  taskName = "sqa-task";
  processName = "sqaclient";
}

inline void SQALogger::initialize(const char* file, int level)
{
  Os::Logger::instance().initialize<SQALogger>(level, file, *this);
}

inline std::string SQALogger::getHostName()
{
  return hostName;
}

inline std::string SQALogger::getCurrentTask()
{
  return taskName;
}

inline std::string SQALogger::getProcessName()
{
  return processName;
}



#endif //EXCLUDE_SQA_INLINES

#endif	/* SQACLIENT_H */

