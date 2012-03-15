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


#ifndef ZMQJSONRPCCLIENT_H
#define	ZMQJSONRPCCLIENT_H


#include "ZMQJsonMessage.h"
#include "ZMQProactorClient.h"
#include "ZMQReactorClient.h"
#include "ZMQBlockingRequest.h"

class ZMQJsonRpcClient
{
public:
  ZMQJsonRpcClient();
  ~ZMQJsonRpcClient();
  bool connect(const std::string& protocol, const std::string& serverAddress, unsigned short rpcPort);
  bool execute(const std::string& method, const json::Object& params, ZMQJsonMessage& result);
  void notify(std::string identity, const std::string& event, const json::Object& params);
  bool startRpcService(const std::string& localIdentity);
  void stopRpcService();
protected:
  ZMQProactorClient _proactor;
  ZMQSocket _reactor;

};


#endif	/* ZMQJSONRPCCLIENT_H */

