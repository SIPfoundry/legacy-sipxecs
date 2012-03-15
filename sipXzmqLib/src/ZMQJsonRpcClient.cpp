
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

#include <sstream>
#include "ZMQJsonRpcClient.h"


ZMQJsonRpcClient::ZMQJsonRpcClient() :
  _reactor(ZMQ_PUSH, true)
{
}

ZMQJsonRpcClient::~ZMQJsonRpcClient()
{
  stopRpcService();
}

bool ZMQJsonRpcClient::connect(const std::string& protocol, const std::string& serverAddress, unsigned short rpcPort)
{
  if (protocol != "ipc" && protocol != "inproc" && protocol != "tcp")
  {
    ZMQ_LOG_ERROR("ZMQJsonRpcClient::connect - " << " Unsupported protocol " << protocol);
    return false;
  }

  std::ostringstream proactorAddress;
  std::ostringstream reactorAddress;
  
  proactorAddress << protocol << "://" << serverAddress;
  if (protocol == "tcp")
    proactorAddress << ":" << rpcPort;

  reactorAddress << protocol << "://" << serverAddress;
  if (protocol == "tcp")
    reactorAddress << ":" << rpcPort + 1;

  ZMQSocket::Error error;
  if (!_proactor.connect(proactorAddress.str(), error))
  {
    ZMQ_LOG_ERROR("ZMQJsonRpcClient::connect - " << error.what);
    return false;
  }

  if (!_reactor.connect(reactorAddress.str(), error))
  {
    ZMQ_LOG_ERROR("ZMQJsonRpcClient::connect - " << error.what);
    return false;
  }
  return true;
}

bool ZMQJsonRpcClient::startRpcService(const std::string& localIdentity)
{
  ZMQSocket::Error error;
  if (!_proactor.startProactorClient("jsonrpc", localIdentity, error))
  {
    ZMQ_LOG_ERROR("ZMQJsonRpcClient::startProactorClient - " << error.what);
    return false;
  }
  return true;
}

void ZMQJsonRpcClient::stopRpcService()
{
  _proactor.stopProactorClient();
  _reactor.close(0);
}

bool ZMQJsonRpcClient::execute(const std::string& method, const json::Object& params, ZMQJsonMessage& result)
{
  std::ostringstream id;
  id << method << "-" <<  ZMQSocket::generateId();

  ZMQJsonMessage request;
  request.setVersion("2.0");
  request.setId(id.str());
  request.setMethod(method);
  request.setParams(params);
  
  ZMQBlockingRequest::Ptr rpcRequest(new ZMQBlockingRequest(request.data()));
  ZMQSocket::Error error;
  ZMQ_LOG_DEBUG("ZMQJsonRpcClient::execute - REQUEST " << request.data());
  if (!_proactor.blocking_send(rpcRequest, error))
  {
    ZMQ_LOG_ERROR("ZMQJsonRpcClient::execute - " << error.what);
    return false;
  }
  ZMQ_LOG_DEBUG("ZMQJsonRpcClient::execute - RESPONSE " << rpcRequest->response().data());
  try
  {
    result = rpcRequest->response().data();
  }
  catch(std::exception& err)
  {
    ZMQ_LOG_ERROR("ZMQJsonRpcClient::execute (PARSE ERROR) - " << err.what());
  }
  return true;
}

void ZMQJsonRpcClient::notify(std::string identity, const std::string& event, const json::Object& params)
{
  ZMQJsonMessage request;
  request.setMethod(event);
  request.setParams(params);
  ZMQSocket::Error error;
  ZMQMessage message(request.data());
  if (!_reactor.send(message, error))
    ZMQ_LOG_ERROR("ZMQJsonRpcClient::notify - " << error.what);
}




