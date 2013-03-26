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

#include "sqa/StateQueueAgent.h"
#include "sqa/StateQueueNotification.h"
#include "os/OsLogger.h"

static const char* REDIS_CHANNEL = REDIS_EVENT_CHANNEL;

StateQueueAgent::StateQueueAgent(ServiceOptions& options) :
  _options(options),
  _pIoServiceThread(0),
  _ioService(),
  _publisher(this),
  _queueWorkSpaceIndex(REDIS_STATEQUEUE_WORKSPACE),
  _listener(this),
  _inactivityThreshold(60),
  _pEntityDb(0),
  _pEntityDbConnectionInfo(0),
  _terminated(false)
{
    std::string port;
    std::string address;
    _options.getOption("zmq-subscription-address", address);
    _options.getOption("zmq-subscription-port", port);
    _publisherAddress.append("tcp://").append(address).append(":").append(port);

    OS_LOG_INFO(FAC_NET, "StateQueueAgent CREATED.");
}

StateQueueAgent::~StateQueueAgent()
{
  delete _pEntityDb;
  delete _pEntityDbConnectionInfo;
  _pEntityDb = 0;
  _pEntityDbConnectionInfo = 0;
  stop();
  OS_LOG_INFO(FAC_NET, "StateQueueAgent DESTROYED.");
}

void StateQueueAgent::run()
{
  assert(!_pIoServiceThread);
  assert(!_publisherAddress.empty());
  _pIoServiceThread = new boost::thread(boost::bind(&StateQueueAgent::internal_run_io_service, this));
}

void StateQueueAgent::onRedisWatcherEvent(const std::vector<std::string>& event)
{
  if (event.size() == 3)
  {
    OS_LOG_INFO(FAC_NET, "StateQueueAgent::onRedisWatcherEvent: "
            << event[0] << " | "
            << event[1] << " | "
            << event[2]);
    
    if (event[0] == "message" && event[1] == REDIS_CHANNEL)
    {
      StateQueueRecord rec;
      rec.id = REDIS_CHANNEL;
      rec.data = event[2];
      publish(rec);
    }
  }
}

void StateQueueAgent::onRedisWatcherConnect(int status)
{
  OS_LOG_NOTICE(FAC_NET, "StateQueueAgent::onRedisWatcherConnect status=" << status);
}

void StateQueueAgent::onRedisWatcherDisconnect(int status)
{
  OS_LOG_NOTICE(FAC_NET, "StateQueueAgent::onRedisWatcherDisconnect status=" << status);
}

void StateQueueAgent::internal_run_io_service()
{
  if (!_publisherAddress.empty())
  {
    _publisher.setBindAddress(_publisherAddress);
    _publisher.run();
  }

  //
  // Check if oplog needs to be published
  //
  std::string oplogConfig;
  if (_options.getOption("publish-entity-oplog-config", oplogConfig))
  {
    try
    {
      const mongo::ConnectionString connString =
        MongoDB::ConnectionInfo::connectionStringFromFile(oplogConfig);
      _pEntityDbConnectionInfo = new MongoDB::ConnectionInfo(connString, "imdb.identity");
      _pEntityDb = new MongoOpLog(*_pEntityDbConnectionInfo);
      _pEntityDb->registerCallback(MongoOpLog::Insert, boost::bind(&StateQueueAgent::onOpLogInsert, this, _1));
      _pEntityDb->registerCallback(MongoOpLog::Update, boost::bind(&StateQueueAgent::onOpLogUpdate, this, _1));
      _pEntityDb->registerCallback(MongoOpLog::Delete, boost::bind(&StateQueueAgent::onOpLogDelete, this, _1));
      _pEntityDb->run();
    }
    catch(...)
    {
    }
  }

  //
  // Connect the redis client
  //
  _redisWatcher.connect(
    boost::bind(&StateQueueAgent::onRedisWatcherConnect, this, _1),
    boost::bind(&StateQueueAgent::onRedisWatcherDisconnect, this, _1),
    boost::bind(&StateQueueAgent::onRedisWatcherEvent, this, _1)
  );
  
  std::vector<std::string> watch;
  watch.push_back("SUBSCRIBE");
  watch.push_back(REDIS_CHANNEL);
  _redisWatcher.asyncCommand(watch);

  _redisWatcher.run();
  
  _listener.run();
  _ioService.run();
}


void StateQueueAgent::stop()
{
  if (_terminated)
    return;

  _terminated = true;

  if (_pEntityDb)
  {
    _pEntityDb->stop();
    delete _pEntityDb;
    _pEntityDb = 0;
  }

  //
  // Unsubscribe from redis channel
  //
  std::vector<std::string> unsubscribe;
  unsubscribe.push_back("UNSUBSCRIBE");
  unsubscribe.push_back(REDIS_CHANNEL);
  _redisWatcher.stop();


  _dataStore.stop();
  _ioService.stop();
  if (_pIoServiceThread && _pIoServiceThread->joinable())
    _pIoServiceThread->join();
  delete _pIoServiceThread;
  _pIoServiceThread = 0;
  _publisher.stop();
}

void StateQueueAgent::onOpLogUpdate(const std::string& opLog)
{
  StateQueueRecord record;
  record.id = "sqw.identity.oplog.update";
  record.data = opLog;
  publish(record);

  record.id = "sqw.identity.oplog.all";
  publish(record);
}

void StateQueueAgent::onOpLogInsert(const std::string& opLog)
{
  StateQueueRecord record;
  record.id = "sqw.identity.oplog.insert";
  record.data = opLog;
  publish(record);

  record.id = "sqw.identity.oplog.all";
  publish(record);
}

void StateQueueAgent::onOpLogDelete(const std::string& opLog)
{
  StateQueueRecord record;
  record.id = "sqw.identity.oplog.delete";
  record.data = opLog;
  publish(record);

  record.id = "sqw.identity.oplog.all";
  publish(record);
}

void StateQueueAgent::onIncomingConnection(StateQueueConnection::Ptr conn)
{
}

void StateQueueAgent::onDestroyConnection(StateQueueConnection::Ptr conn)
{
  //
  // Publish connection destruction to who ever wants to know
  //
  if (conn->isAlphaConnection() && !conn->getApplicationId().empty())
  {
    StateQueueRecord record;
    record.id = "sqw.connection.terminated";
    record.data = conn->getApplicationId();
    record.data += "|";
    record.data += conn->getRemoteAddress();
    publish(record);
  }

  _listener.destroyConnection(conn);
}

void StateQueueAgent::onIncomingRequest(StateQueueConnection& conn, const char* bytes, std::size_t bytes_transferred)
{
  OS_LOG_DEBUG(FAC_NET, "StateQueueAgent::onIncomingRequest processing " << bytes_transferred << " bytes.");
  std::string packet(bytes, bytes_transferred);
  StateQueueMessage message(packet);
  StateQueueMessage::Type type;
  type = message.getType();

  std::string id;
  std::string appId;

  if (type != StateQueueMessage::Ping)
  {
    if (!message.get("message-id", id) || id.empty())
    {
      OS_LOG_INFO(FAC_NET, packet);
      sendErrorResponse(message.getType(), conn, "unknown-id", "Missing required argument message-id.");
      return;
    }

    if (!message.get("message-app-id", appId) || appId.empty())
    {
      sendErrorResponse(type, conn, id, "Missing required argument message-app-id.");
      return;
    }
    
    conn.setApplicationId(appId);

    if (conn.isAlphaConnection() && !conn.isCreationPublished())
    {
      StateQueueRecord record;
      record.id = "sqw.connection.established";
      record.data = conn.getApplicationId();
      record.data += "|";
      record.data += conn.getRemoteAddress();
      publish(record);
      //
      // Mark it as published
      //
      conn.setCreationPublished();
    }
  }
  else
  {
    //
    // This is a PING request
    //
    if (!message.get("message-app-id", appId) || appId.empty())
    {
      sendErrorResponse(type, conn, id, "Missing required argument message-app-id.");
      return;
    }

    if (conn.isAlphaConnection())
    {
      StateQueueRecord record;
      record.id = "sqw.connection.keepalive";
      record.data = conn.getApplicationId();
      record.data += "|";
      record.data += conn.getRemoteAddress();
      publish(record);
    }
  }

  switch (type)
  {
    case StateQueueMessage::Signin:
      handleSignin(conn, message, id, appId);
      break;
    case StateQueueMessage::Logout:
      handleLogout(conn, message, id, appId);
      break;
    case StateQueueMessage::Enqueue:
      handleEnqueue(conn, message, id, appId);
      break;
    case StateQueueMessage::EnqueueAndPublish:
      handleEnqueueAndPublish(conn, message, id, appId);
      break;
    case StateQueueMessage::Publish:
      handlePublish(conn, message, id, appId);
      break;
    case StateQueueMessage::PublishAndPersist:
      handlePublishAndPersist(conn, message, id, appId);
      break;
    case StateQueueMessage::Pop:
      handlePop(conn, message, id, appId);
      break;
    case StateQueueMessage::Erase:
      handleErase(conn, message, id, appId);
      break;
    case StateQueueMessage::Persist:
      handlePersist(conn, message, id, appId);
      break;
    case StateQueueMessage::Set:
      handleSet(conn, message, id, appId);
      break;
    case StateQueueMessage::Get:
      handleGet(conn, message, id, appId);
      break;
    case StateQueueMessage::MapSet:
      handleMapSet(conn, message, id, appId);
      break;
    case StateQueueMessage::MapGet:
      handleMapGet(conn, message, id, appId);
      break;
    case StateQueueMessage::MapGetMultiple:
      handleMapGetMultiple(conn, message, id, appId);
      break;
    case StateQueueMessage::MapGetInc:
      handleMapGetInc(conn, message, id, appId);
      break;
    case StateQueueMessage::Remove:
      handleRemove(conn, message, id, appId);
      break;
    case StateQueueMessage::Ping:
      handlePing(conn, message);
      break;
    default:
      sendErrorResponse(type, conn, id, "Invalid Command!");
  }
}

void StateQueueAgent::sendErrorResponse(
  StateQueueMessage::Type type,
  StateQueueConnection& conn,
  const std::string& messageId,
  const std::string& error)
{
  OS_LOG_WARNING(FAC_SIP, "Message-id: " << messageId << " Error: " << error);
  StateQueueMessage response;
  response.setType(type);
  response.set("message-id", messageId);
  response.set("message-response", "error");
  response.set("message-error", error);
  conn.write(response.data());
}

void StateQueueAgent::sendOkResponse(StateQueueMessage::Type type, StateQueueConnection& conn, const std::string& messageId, const std::string& messageData)
{
  StateQueueMessage response;
  response.setType(type);
  response.set("message-response", "ok");
  response.set("message-id", messageId);
  if (!messageData.empty())
    response.set("message-data", messageData);
  OS_LOG_INFO(FAC_SIP, "Message-id: " << messageId << " Ok: " << messageData);
  conn.write(response.data());
}

void StateQueueAgent::handlePing(StateQueueConnection& conn, StateQueueMessage& message)
{
  StateQueueMessage response;
  response.setType(StateQueueMessage::Pong);
  conn.write(response.data());
  OS_LOG_DEBUG(FAC_NET, "Keep-alive request received from " << conn.getRemoteAddress() << ":" << conn.getRemotePort());
}

void StateQueueAgent::handleEnqueueAndPublish(StateQueueConnection& conn, StateQueueMessage& message,
    const std::string& id, const std::string& appId)
{
  StateQueueRecord record;
  record.id = id;
  message.get("message-data", record.data);
  message.get("message-expires", record.expires);

  OS_LOG_INFO(FAC_NET, "StateQueueAgent::handleEnqueue "
          << "Received new command ENQUEUE. "
          << " message-id: " << record.id
          << " message-expires: " << record.expires);

  OS_LOG_DEBUG(FAC_NET, "StateQueueAgent::handleEnqueue "
          << "Received new command ENQUEUE. "
          << " message-id: " << record.id
          << " message-data: " << record.data);

  enqueue(record);

  record.id = "sqw.";
  record.id += id.substr(4);
  publish(record);

  StateQueueMessage response;
  response.setType(message.getType());
  response.set("message-response", "ok");
  conn.write(response.data());
}

void StateQueueAgent::handleEnqueue(StateQueueConnection& conn, StateQueueMessage& message,
    const std::string& id, const std::string& appId)
{
  StateQueueRecord record;
  record.id = id;
  message.get("message-data", record.data);
  message.get("message-expires", record.expires);

  OS_LOG_INFO(FAC_NET, "StateQueueAgent::handleEnqueue "
          << "Received new command ENQUEUE. "
          << " message-id: " << record.id
          << " message-expires: " << record.expires);

  OS_LOG_DEBUG(FAC_NET, "StateQueueAgent::handleEnqueue "
          << "Received new command ENQUEUE. "
          << " message-id: " << record.id
          << " message-data: " << record.data);

  enqueue(record);

  StateQueueMessage response;
  response.setType(message.getType());
  response.set("message-response", "ok");
  conn.write(response.data());
}

void StateQueueAgent::enqueue(StateQueueRecord& record)
{
  //
  // persist the new record and tell everyne about it.
  //
  if (!record.expires)
    record.expires = 30;
  _dataStore.set(this->_queueWorkSpaceIndex, record, record.expires);
  _publisher.publish(record);
}


void StateQueueAgent::handlePublish(StateQueueConnection& conn, StateQueueMessage& message,
    const std::string& id, const std::string& appId)
{
  StateQueueRecord record;
  record.id = id;
  message.get("message-data", record.data);

  bool noresponse = false;
  if (message.get("noresponse", noresponse) && noresponse)
    noresponse = true;
  else
    noresponse = false;

  OS_LOG_INFO(FAC_NET, "StateQueueAgent::handlePublish "
          << "Received new command PUBLISH. "
          << " message-id: " << record.id)

  OS_LOG_DEBUG(FAC_NET, "StateQueueAgent::handlePublish "
          << "Received new command PUBLISH. "
          << " message-id: " << record.id
          << " message-data: " << record.data);

  publish(record);

  if (!noresponse)
  {
    StateQueueMessage response;
    response.setType(message.getType());
    response.set("message-response", "ok");
    conn.write(response.data());
  }
}

void StateQueueAgent::handlePublishAndPersist(StateQueueConnection& conn, StateQueueMessage& message,
    const std::string& id, const std::string& appId)
{
  int expires;
  if (!message.get("message-expires",  expires) || expires <= 0)
  {
    sendErrorResponse(message.getType(), conn, id, "Missing required argument expires.");
    return;
  }

  std::string dataId;
  if (!message.get("message-data-id",  dataId) || dataId.empty())
  {
    sendErrorResponse(message.getType(), conn, id, "Missing required argument message-data-id.");
    return;
  }

  std::string data;
  if (!message.get("message-data",  data) || data.empty())
  {
    sendErrorResponse(message.getType(), conn, id, "Missing required argument message-data.");
    return;
  }

  int workspace;
  if (!message.get("workspace",  workspace) || workspace < 0)
  {
    sendErrorResponse(message.getType(), conn, id, "Missing required argument workspace.");
    return;
  }

  if (((unsigned)workspace) == _queueWorkSpaceIndex)
  {
    sendErrorResponse(message.getType(), conn, id, "Persisting to default workspace is now allowed.");
    return;
  }

  StateQueueRecord record;
  record.data = data;
  record.id = dataId;
  record.expires = expires;

  OS_LOG_INFO(FAC_NET, "StateQueueAgent::handleSet "
          << "Received new command GET. "
          << " message-data-id: " << dataId
          << " workspace: " << workspace
          << " message-expires: " << expires);

  set(dataId, workspace, record, expires);


  publish(record);

  StateQueueMessage response;
  response.setType(message.getType());
  response.set("message-response", "ok");
  conn.write(response.data());
}

void StateQueueAgent::publish(StateQueueRecord& record)
{
  //
  // persist the new record and tell everyne about it.
  //
  if (!record.expires)
    record.expires = 30;
  record.watcherData = true;
  _publisher.publish(record);
}

void StateQueueAgent::handlePop(StateQueueConnection& conn, StateQueueMessage& message, const std::string& id, const std::string& appId)
{ 
  int expires;
  if (!message.get("message-expires",  expires) || expires <= 0)
  {
    sendErrorResponse(message.getType(), conn, id, "Missing required argument expires.");
    return;
  }

  OS_LOG_INFO(FAC_NET, "StateQueueAgent::handlePop "
          << "Received new command POP. "
          << " message-id: " << id
          << " message-app-id: " << appId
          << " message-expires: " << expires);

  StateQueueRecord record;
  if (!pop(appId, id, record, expires))
  {
    sendErrorResponse(message.getType(), conn, id, "Message non existent or has expired.");
    return;
  }

  sendOkResponse(message.getType(), conn, id, record.data);
}

bool StateQueueAgent::pop(const std::string& appId, const std::string& id, StateQueueRecord& record, int expires)
{
  if (!_dataStore.get(_queueWorkSpaceIndex, id, record))
    return false;
  _dataStore.erase(_queueWorkSpaceIndex, id);
  record.retry++;
  record.exclude.push_back(appId);
  record.expires = expires;
  _cache.enqueue(id, record, boost::bind(&StateQueueAgent::onQueueTimeout, this, _1, _2), expires);
  return true;
}

void StateQueueAgent::handlePersist(StateQueueConnection& conn, StateQueueMessage& message,
    const std::string& id, const std::string& appId)
{
  int expires;
  if (!message.get("message-expires",  expires) || expires <= 0)
  {
    sendErrorResponse(message.getType(), conn, id, "Missing required argument expires.");
    return;
  }

  int workspace;
  if (!message.get("workspace",  workspace) || workspace < 0)
  {
    sendErrorResponse(message.getType(), conn, id, "Missing required argument workspace.");
    return;
  }

  std::string dataId;
  if (!message.get("message-data-id", dataId) || dataId.empty())
  {
    sendErrorResponse(message.getType(), conn, id, "Missing required argument message-data-id.");
    return;
  }

  if (((unsigned)workspace) == _queueWorkSpaceIndex)
  {
    sendErrorResponse(message.getType(), conn, id, "Persisting to default workspace is now allowed.");
    return;
  }
  
  OS_LOG_INFO(FAC_NET, "StateQueueAgent::handlePersist "
          << "Received new command PERSIST. "
          << " message-data-id: " << dataId
          << " workspace: " << workspace
          << " message-expires: " << expires);

  persist(dataId, workspace, expires);
  
  sendOkResponse(message.getType(), conn, id, "");
}

void StateQueueAgent::persist(const std::string& id, int workspaceId, int expires)
{
  StateQueueRecord record;

  boost::any cacheData;
  if (!_cache.dequeue(id, cacheData))
    return;

  record = boost::any_cast<StateQueueRecord>(cacheData);

  _dataStore.erase(_queueWorkSpaceIndex, id);
  set(id, workspaceId, record, expires);
}

void StateQueueAgent::handleSet(StateQueueConnection& conn, StateQueueMessage& message,
    const std::string& id, const std::string& appId)
{
  int expires;
  if (!message.get("message-expires",  expires) || expires <= 0)
  {
    sendErrorResponse(message.getType(), conn, id, "Missing required argument expires.");
    return;
  }

  std::string dataId;
  if (!message.get("message-data-id",  dataId) || dataId.empty())
  {
    sendErrorResponse(message.getType(), conn, id, "Missing required argument message-data-id.");
    return;
  }

  std::string data;
  if (!message.get("message-data",  data) || data.empty())
  {
    sendErrorResponse(message.getType(), conn, id, "Missing required argument message-data.");
    return;
  }

  int workspace;
  if (!message.get("workspace",  workspace) || workspace < 0)
  {
    sendErrorResponse(message.getType(), conn, id, "Missing required argument workspace.");
    return;
  }

  if (((unsigned)workspace) == _queueWorkSpaceIndex)
  {
    sendErrorResponse(message.getType(), conn, id, "Persisting to default workspace is now allowed.");
    return;
  }
  
  StateQueueRecord record;
  record.data = data;
  record.id = dataId;
  record.expires = expires;

  OS_LOG_INFO(FAC_NET, "StateQueueAgent::handleSet "
          << "Received new command GET. "
          << " message-data-id: " << dataId
          << " workspace: " << workspace
          << " message-expires: " << expires);

  set(dataId, workspace, record, expires);

  sendOkResponse(message.getType(), conn, id, "");
}

void StateQueueAgent::set(const std::string& dataId, int workspaceId, StateQueueRecord& record, int expires)
{
  _cache.erase(dataId);
  _dataStore.set(workspaceId, record, expires);
}


void StateQueueAgent::handleGet(StateQueueConnection& conn, StateQueueMessage& message, const std::string& id, const std::string& appId)
{
  std::string dataId;
  if (!message.get("message-data-id",  dataId) || dataId.empty())
  {
    sendErrorResponse(message.getType(), conn, id, "Missing required argument message-data-id.");
    return;
  }

  int workspace;
  if (!message.get("workspace",  workspace) || workspace < 0)
  {
    sendErrorResponse(message.getType(), conn, id, "Missing required argument workspace.");
    return;
  }

  OS_LOG_INFO(FAC_NET, "StateQueueAgent::handleGet "
          << "Received new command GET. "
          << " message-data-id: " << dataId
          << " workspace: " << workspace);


  StateQueueRecord record;
  if (!get(appId, dataId, workspace, record))
  {
    sendErrorResponse(message.getType(), conn, id, "Message non existent or has expired.");
    return;
  }

  sendOkResponse(message.getType(), conn, id, record.data);
}

bool StateQueueAgent::get(const std::string& appId, const std::string& dataId, int workspaceId, StateQueueRecord& record)
{
  return _dataStore.get(workspaceId, dataId, record);
}

void StateQueueAgent::handleMapGet(StateQueueConnection& conn, StateQueueMessage& message,
    const std::string& id, const std::string& appId)
{
  std::string mapId;
  std::string dataId;

  if (!message.get("message-map-id",  mapId) || mapId.empty())
  {
    sendErrorResponse(message.getType(), conn, id, "Missing required argument message-map-id.");
    return;
  }

  if (!message.get("message-data-id",  dataId) || dataId.empty())
  {
    sendErrorResponse(message.getType(), conn, id, "Missing required argument message-data-id.");
    return;
  }

  int workspace;
  if (!message.get("workspace",  workspace) || workspace < 0)
  {
    sendErrorResponse(message.getType(), conn, id, "Missing required argument workspace.");
    return;
  }

  OS_LOG_INFO(FAC_NET, "StateQueueAgent::handleGet "
          << "Received new command GET. "
          << " message-data-id: " << dataId
          << " workspace: " << workspace);


  StateQueueRecord record;
  if (!mget(appId, mapId, dataId, workspace, record))
  {
    sendErrorResponse(message.getType(), conn, id, "Message non existent or has expired.");
    return;
  }

  sendOkResponse(message.getType(), conn, id, record.data);
}

bool StateQueueAgent::mget(const std::string& appId, const std::string& mapId,
        const std::string& dataId, int workspaceId, StateQueueRecord& record)
{
  return _dataStore.mapGet(workspaceId, mapId, dataId, record);
}

void StateQueueAgent::handleMapGetMultiple(StateQueueConnection& conn, StateQueueMessage& message,
    const std::string& id, const std::string& appId)
{
  std::string mapId;

  if (!message.get("message-map-id",  mapId) || mapId.empty())
  {
    sendErrorResponse(message.getType(), conn, id, "Missing required argument message-map-id.");
    return;
  }

  int workspace;
  if (!message.get("workspace",  workspace) || workspace < 0)
  {
    sendErrorResponse(message.getType(), conn, id, "Missing required argument workspace.");
    return;
  }

  OS_LOG_INFO(FAC_NET, "StateQueueAgent::handleGetMultiple "
          << "Received new command GET. "
          << " message-map-id: " << mapId
          << " workspace: " << workspace);


  StateQueueRecord record;
  if (!mgetm(appId, mapId, workspace, record))
  {
    sendErrorResponse(message.getType(), conn, id, "Message non existent or has expired.");
    return;
  }

  sendOkResponse(message.getType(), conn, id, record.data);
}

bool StateQueueAgent::mgetm(const std::string& appId, const std::string& mapId,
        int workspaceId, StateQueueRecord& record)
{
  StateQueuePersistence::RecordVector records;
  if (!_dataStore.mapGet(workspaceId, mapId, records))
    return false;

  StateQueueMessage message;
  message.setType(StateQueueMessage::Data);
  for (StateQueuePersistence::RecordVector::const_iterator iter = records.begin(); iter != records.end(); iter++)
    message.set(iter->id.c_str(), iter->data);
  record.data = message.data();
  record.id = mapId;
  return true;
}

void StateQueueAgent::handleMapGetInc(StateQueueConnection& conn, StateQueueMessage& message,
    const std::string& id, const std::string& appId)
{
  std::string mapId;
  std::string dataId;

  if (!message.get("message-map-id",  mapId) || mapId.empty())
  {
    sendErrorResponse(message.getType(), conn, id, "Missing required argument message-map-id.");
    return;
  }

  if (!message.get("message-data-id",  dataId) || dataId.empty())
  {
    sendErrorResponse(message.getType(), conn, id, "Missing required argument message-data-id.");
    return;
  }

  int workspace;
  if (!message.get("workspace",  workspace) || workspace < 0)
  {
    sendErrorResponse(message.getType(), conn, id, "Missing required argument workspace.");
    return;
  }

  OS_LOG_INFO(FAC_NET, "StateQueueAgent::handleGet "
          << "Received new command GET. "
          << " message-data-id: " << dataId
          << " workspace: " << workspace);


  StateQueueRecord record;
  if (!mgeti(appId, mapId, dataId, workspace, record))
  {
    sendErrorResponse(message.getType(), conn, id, "Message non existent or has expired.");
    return;
  }

  sendOkResponse(message.getType(), conn, id, record.data);
}

bool StateQueueAgent::mgeti(const std::string& appId, const std::string& mapId,
        const std::string& dataId, int workspaceId, StateQueueRecord& record)
{
  return _dataStore.mapGetInc(workspaceId, mapId, dataId, record);
}

void StateQueueAgent::handleMapSet(StateQueueConnection& conn, StateQueueMessage& message,
  const std::string& id, const std::string& appId)
{
  int expires;
  if (!message.get("message-expires",  expires) || expires <= 0)
  {
    sendErrorResponse(message.getType(), conn, id, "Missing required argument expires.");
    return;
  }

  std::string mapId;
  if (!message.get("message-map-id",  mapId) || mapId.empty())
  {
    sendErrorResponse(message.getType(), conn, id, "Missing required argument message-map-id.");
    return;
  }

  std::string dataId;
  if (!message.get("message-data-id",  dataId) || dataId.empty())
  {
    sendErrorResponse(message.getType(), conn, id, "Missing required argument message-data-id.");
    return;
  }

  std::string data;
  if (!message.get("message-data",  data) || data.empty())
  {
    sendErrorResponse(message.getType(), conn, id, "Missing required argument message-data.");
    return;
  }

  int workspace;
  if (!message.get("workspace",  workspace) || workspace < 0)
  {
    sendErrorResponse(message.getType(), conn, id, "Missing required argument workspace.");
    return;
  }

  if (((unsigned)workspace) == _queueWorkSpaceIndex)
  {
    sendErrorResponse(message.getType(), conn, id, "Persisting to default workspace is now allowed.");
    return;
  }

  StateQueueRecord record;
  record.data = data;
  record.id = dataId;
  record.expires = expires;

  OS_LOG_INFO(FAC_NET, "StateQueueAgent::handleSet "
          << "Received new command GET. "
          << " message-data-id: " << dataId
          << " workspace: " << workspace
          << " message-expires: " << expires);

  mset(mapId, workspace, record, expires);

  sendOkResponse(message.getType(), conn, id, "");
}

void StateQueueAgent::mset(const std::string& mapId, int workspaceId, StateQueueRecord& record, int expires)
{
  _dataStore.mapSet(workspaceId, mapId, record, expires);
}

void StateQueueAgent::handleRemove(StateQueueConnection& conn, StateQueueMessage& message, const std::string& id, const std::string& appId)
{
  std::string dataId;
  if (!message.get("message-data-id",  dataId) || dataId.empty())
  {
    sendErrorResponse(message.getType(), conn, id, "Missing required argument message-data-id.");
    return;
  }

  OS_LOG_INFO(FAC_NET, "StateQueueAgent::handleGet "
          << "Received new command REMOVE. "
          << " message-data-id: " << id
          << " message-app-id: " << dataId);

  int workspace;
  if (!message.get("workspace",  workspace) || workspace < 0)
  {
    sendErrorResponse(message.getType(), conn, id, "Missing required argument workspace.");
    return;
  }

  StateQueueRecord record;
  if (!remove(appId, dataId, workspace))
  {
    sendErrorResponse(message.getType(), conn, id, "Message non existent or has expired.");
    return;
  }

  sendOkResponse(message.getType(), conn, id, "");
}

bool StateQueueAgent::remove(const std::string& appId, const std::string& dataId, int workspaceId)
{
  return _dataStore.erase(workspaceId, dataId);
}


void StateQueueAgent::handleErase(StateQueueConnection& conn, StateQueueMessage& message,
    const std::string& id, const std::string& appId)
{
  OS_LOG_INFO(FAC_NET, "StateQueueAgent::handleErase "
          << "Received new command ERASE. "
          << "message-id: " << id);

  erase(id);

  sendOkResponse(message.getType(), conn, id, "");
}

void StateQueueAgent::erase(const std::string& id)
{
  _cache.erase(id);
  _dataStore.erase(_queueWorkSpaceIndex, id);
}

void StateQueueAgent::onQueueTimeout(const std::string& id, const boost::any& data)
{

  StateQueueRecord record = boost::any_cast<const StateQueueRecord&>(data);
   
  if (record.retry < 2)
  {
    OS_LOG_INFO(FAC_NET, "StateQueueAgent::onQueueTimeout "
          << "Message has expired in queue. "
          << " message-id: " << id
          << " retry-count: " << record.retry);
    enqueue(record);
  }
  else
  {
    OS_LOG_WARNING(FAC_NET, "StateQueueAgent::onQueueTimeout "
          << "Message has expired in queue more than once.  Dropping. "
          << " message-id: " << id);
  }
}

void StateQueueAgent::handleSignin(StateQueueConnection& conn, StateQueueMessage& message,
    const std::string& id, const std::string& appId)
{
  int subscriptionExpires;
  if (!message.get("subscription-expires", subscriptionExpires))
  {
    sendErrorResponse(message.getType(), conn, id, "Missing required argument subscription-expires.");
    return;
  }

  std::string subscriptionEvent;
  if (!message.get("subscription-event", subscriptionEvent))
  {
    sendErrorResponse(message.getType(), conn, id, "Missing required argument subscription-event.");
    return;
  }

  std::string serviceType;
  if (!message.get("service-type", serviceType))
  {
    sendErrorResponse(message.getType(), conn, id, "Missing required argument service-type.");
    return;
  }

  if (serviceType == "worker")
    _publisher.addSubscriber(subscriptionEvent, appId, subscriptionExpires);

  OS_LOG_NOTICE(FAC_NET, "StateQueueAgent::handleSignin " << appId << "/" << subscriptionEvent << " RECEIVED");

  sendOkResponse(message.getType(), conn, id, _publisherAddress);

  conn.setApplicationId(appId);


  StateQueueRecord record;
  record.id = "sqw.connection.signin";
  record.data = conn.getApplicationId();
  record.data += "|";
  record.data += conn.getRemoteAddress();
  publish(record);

}

void StateQueueAgent::handleLogout(StateQueueConnection& conn, StateQueueMessage& message,
    const std::string& id, const std::string& appId)
{
  std::string subscriptionEvent;
  if (!message.get("subscription-event", subscriptionEvent))
  {
    sendErrorResponse(message.getType(), conn, id, "Missing required argument subscription-event.");
    return;
  }

  std::string serviceType;
  if (!message.get("service-type", serviceType))
  {
    sendErrorResponse(message.getType(), conn, id, "Missing required argument service-type.");
    return;
  }

  if (serviceType == "worker")
    _publisher.removeSubscriber(subscriptionEvent, appId);

  OS_LOG_NOTICE(FAC_NET, "StateQueueAgent::handleLogout " << appId << "/" << subscriptionEvent << " RECEIVED");

  sendOkResponse(message.getType(), conn, id, "bfn!");

  StateQueueRecord record;
  record.id = "sqw.connection.logout";
  record.data = conn.getApplicationId();
  record.data += "|";
  record.data += conn.getRemoteAddress();
  publish(record);
}




