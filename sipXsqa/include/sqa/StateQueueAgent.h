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

#ifndef STATEQUEUEAGENT_H_INCLUDED
#define STATEQUEUEAGENT_H_INCLUDED


#include "ServiceOptions.h"
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include "StateQueueConnection.h"
#include "TimedQueue.h"
#include "StateQueuePersistence.h"
#include "StateQueuePublisher.h"
#include "StateQueueRecord.h"
#include "StateQueueMessage.h"
#include "StateQueueListener.h"
#include "zmq.hpp"
#include "RedisClientAsync.h"


class StateQueueAgent : boost::noncopyable
{
public:
  StateQueueAgent(const std::string& agentId, ServiceOptions& options);
  ~StateQueueAgent();
  void run();
  void stop();
  void onIncomingConnection(StateQueueConnection::Ptr conn);
  void onIncomingRequest(StateQueueConnection& conn, const char* bytes, std::size_t bytes_transferred);

  void fillEventRecord(StateQueueRecord &record, StateQueueConnection& conn, ConnectionEvent connnectionEvent);
  void fillEventRecord(
          StateQueueRecord &record,
          const std::string &messageId,
          const std::string &messageData,
          int expires = 0,
          bool watcherData = true
          );

  void handleSignin(StateQueueConnection& conn, StateQueueMessage& message,
    const std::string& id, const std::string& appId);

  void handleLogout(StateQueueConnection& conn, StateQueueMessage& message,
    const std::string& id, const std::string& appId);

  void handleEnqueue(StateQueueConnection& conn, StateQueueMessage& message,
    const std::string& id, const std::string& appId);
  void enqueue(StateQueueRecord& record);

  void handleEnqueueAndPublish(StateQueueConnection& conn, StateQueueMessage& message,
    const std::string& id, const std::string& appId);

  void handlePublish(StateQueueConnection& conn, StateQueueMessage& message,
    const std::string& id, const std::string& appId);

  void publish(StateQueueRecord& record);

  void handlePublishAndSet(StateQueueConnection& conn, StateQueueMessage& message,
    const std::string& id, const std::string& appId);

  void handlePop(StateQueueConnection& conn, StateQueueMessage& message,
    const std::string& id, const std::string& appId);
  bool pop(const std::string& appId, const std::string& id, StateQueueRecord& record, int expires);

  void handlePersist(StateQueueConnection& conn, StateQueueMessage& message,
    const std::string& id, const std::string& appId);
  void persist(const std::string& id, int workspaceId, int expires);

  void handleGet(StateQueueConnection& conn, StateQueueMessage& message,
    const std::string& id, const std::string& appId);
  bool get(const std::string& appId, const std::string& dataId, int workspaceId, StateQueueRecord& record);

  void handleSet(StateQueueConnection& conn, StateQueueMessage& message,
    const std::string& id, const std::string& appId);
  void set(const std::string& id, int workspaceId, StateQueueRecord& record, int expires);

  void handleMapGet(StateQueueConnection& conn, StateQueueMessage& message,
    const std::string& id, const std::string& appId);
  bool mget(const std::string& appId, const std::string& mapId,
    const std::string& dataId, int workspaceId, StateQueueRecord& record);

  void handleMapGetMultiple(StateQueueConnection& conn, StateQueueMessage& message,
    const std::string& id, const std::string& appId);
  bool mgetm(const std::string& appId, const std::string& mapId,
    int workspaceId, StateQueueRecord& record);

  void handleMapGetInc(StateQueueConnection& conn, StateQueueMessage& message,
    const std::string& id, const std::string& appId);
  bool mgeti(const std::string& appId, const std::string& mapId,
    const std::string& dataId, int workspaceId, StateQueueRecord& record);

  void handleMapSet(StateQueueConnection& conn, StateQueueMessage& message,
    const std::string& id, const std::string& appId);
  void mset(const std::string& mapId, int workspaceId, StateQueueRecord& record, int expires);

  void handleRemove(StateQueueConnection& conn, StateQueueMessage& message,
    const std::string& id, const std::string& appId);
  bool remove(const std::string& appId, const std::string& dataId, int workspaceId);

  void handleErase(StateQueueConnection& conn, StateQueueMessage& message,
    const std::string& id, const std::string& appId);
  void erase(const std::string& id);

  void handlePing(StateQueueConnection& conn, StateQueueMessage& message, const std::string& appId);
  
  void onQueueTimeout(const std::string& id, const boost::any& data);

  ServiceOptions& options();
  StateQueueListener& listener();
  int& inactivityThreshold();
  
  void onDestroyConnection(StateQueueConnection::Ptr conn);

  const char* getClassName();
protected:
  void internal_run_io_service();
  void sendErrorResponse(StateQueueMessage::Type type, StateQueueConnection& conn, const std::string& messageId, const std::string& error);
  void sendOkResponse(StateQueueMessage::Type type, StateQueueConnection& conn, const std::string& messageId, const std::string& messageData);

  void onRedisWatcherEvent(const std::vector<std::string>& event);
  void onRedisWatcherConnect(int status);
  void onRedisWatcherDisconnect(int status);
  /// Set a publisher to be used instead of the internally created one.
  /// After set the publisher pointer is owned by this class.
  void setPublisher(StateQueuePublisher* publisher);

  
  std::string _agentId;
  ServiceOptions& _options;
  boost::thread* _pIoServiceThread;
  boost::asio::io_service _ioService;
  TimedQueue _cache;
  StateQueuePublisher* _publisher;
  StateQueuePersistence _dataStore;
  unsigned _queueWorkSpaceIndex;
  StateQueueListener _listener;
  int _inactivityThreshold;
  std::string _publisherAddress;
  boost::thread* _pRedisWatcherThread;
  //RedisClientAsync _redisWatcher;
  bool _terminated;
  friend class StateQueueListener;
  friend class StateQueueConnection;
  friend class StateQueueConnectionTest;
  friend class StateQueueAgentTest;
};


//
// Inlines
//
inline const char* StateQueueAgent::getClassName()
{
  return "StateQueueAgent";
}

inline ServiceOptions& StateQueueAgent::options()
{
  return _options;
}

inline StateQueueListener& StateQueueAgent::listener()
{
  return _listener;
}

inline int& StateQueueAgent::inactivityThreshold()
{
  return _inactivityThreshold;
}

#endif
