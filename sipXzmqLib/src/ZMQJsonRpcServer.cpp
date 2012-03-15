/*
 * Copyright (c) 2011 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */

#include "ZMQJsonRpcServer.h"

ZMQJsonRpcServer::ZMQJsonRpcServer() :
  _reactor(ZMQ_PULL),
  _reactorControl(ZMQ_PUSH),
  _pReadThread(0),
  _isReading(false)
{
  _quitUUID = ZMQSocket::generateId();
}

ZMQJsonRpcServer::~ZMQJsonRpcServer()
{
  stopRpcService();
  if(_pReadThread)
    delete _pReadThread;
}

void ZMQJsonRpcServer::registerMethod(const std::string& method, RpcMethodHandler handler)
{
  _methods[method] = handler;
}

void ZMQJsonRpcServer::registerNotifier(const std::string& event, RpcEventHandler handler)
{
  _notifiers[event] = handler;
}

bool ZMQJsonRpcServer::startRpcService(const std::string& identity, const std::string& protocol, const std::string& bindAddress, unsigned short rpcPort)
{
  if (protocol != "ipc" && protocol != "inproc" && protocol != "tcp")
  {
    ZMQ_LOG_ERROR("ZMQJsonRpcServer::startRpcService - " << " Unsupported protocol " << protocol);
    return false;
  }

  std::ostringstream proactorAddress;
  std::ostringstream reactorAddress;

  proactorAddress << protocol << "://" << bindAddress;
  if (protocol == "tcp")
    proactorAddress << ":" << rpcPort;

  reactorAddress << protocol << "://" << bindAddress;
  if (protocol == "tcp")
    reactorAddress << ":" << rpcPort + 1;

  ZMQSocket::Error error;
  if (!_proactor.bind(proactorAddress.str(), error))
  {
    ZMQ_LOG_ERROR("ZMQJsonRpcServer::startRpcService::_proactor.bind - " << error.what);
    return false;
  }

  if (!_proactor.startProactor("jsonrpc", bindAddress,  boost::bind(&ZMQJsonRpcServer::executeRpc, this, _1, _2), error))
  {
    ZMQ_LOG_ERROR("ZMQJsonRpcServer::startRpcService::_proactor.startProactor - " << error.what);
    return false;
  }

  if (!_reactor.bind(reactorAddress.str(), error))
  {
    ZMQ_LOG_ERROR("ZMQJsonRpcServer::startRpcService::_reactor.bind - " << error.what);
    return false;
  }

  //
  // start the reactor thread
  //
  if (!_isReading)
  {
    _isReading = true;
    _pReadThread = new boost::thread(boost::bind(&ZMQJsonRpcServer::readNotifications, this));
  }

  if (!_reactorControl.connect(reactorAddress.str(), error))
  {
    ZMQ_LOG_ERROR("ZMQJsonRpcServer::startRpcService::_reactorControl.connect - " << error.what);
    return false;
  }

  return true;
}

void ZMQJsonRpcServer::stopRpcService()
{
  _proactor.stopProactor();
  if (_isReading)
  {
    ZMQSocket::Error error;
    ZMQMessage quit(_quitUUID);
    _reactorControl.send(quit, error);
    _pReadThread->join();
    _reactor.close(0);
  }
}


void ZMQJsonRpcServer::executeRpc(ZMQMessage& task, ZMQMessage& response)
{
  ZMQJsonMessage request, result;
  request = task.data();

  std::string method = request.getMethod();
  std::string id = request.getId();
  result.setVersion("2.0");
  result.setId(id);
  result.setMethod(method);
  
  if (_methods.find(method) == _methods.end())
  {
    //
    // Send a standard JSON RPC error response
    //
    json::Object error;
    error["code"] = json::Number(2601);
    error["message"] = json::String("Method not found");
    result.setError(error);
    ZMQ_LOG_ERROR("ZMQJsonRpcServer::executeRpc - Unknown method " << method);
    return;
  }
  _methods[method](request, result);
  response = result.data();

}

bool ZMQJsonRpcServer::handleNotify(ZMQMessage& message)
{
  if (message.data() == _quitUUID)
  {
    return false;
  }

  ZMQJsonMessage event;
  event = message.data();

  std::string method = event.getMethod();
  
  if (_notifiers.find(method) != _notifiers.end())
    _notifiers[method](event);
  return true;
}

void ZMQJsonRpcServer::readNotifications()
{
  
  ZMQSocket::Error error;
  while(_isReading)
  {
    ZMQMessage message;
    if (_reactor.receive(message, error))
    {
      if (!handleNotify(message))
        break;
    }
  }
  _isReading = false;
  ZMQ_LOG_INFO("ZMQJsonRpcServer::readNotifications ENDED");
}



