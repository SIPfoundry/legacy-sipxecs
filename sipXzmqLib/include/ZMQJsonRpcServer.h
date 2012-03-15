
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


#ifndef ZMQJSONRPCSERVER_H
#define	ZMQJSONRPCSERVER_H

#include "ZMQProactor.h"
#include "ZMQReactor.h"
#include "ZMQJsonMessage.h"
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <map>


class ZMQJsonRpcServer
{
public:
  typedef boost::function<bool(const ZMQJsonMessage& request, ZMQJsonMessage& result)> RpcMethodHandler;
  typedef boost::function<void(const ZMQJsonMessage& event)> RpcEventHandler;
  typedef std::map<std::string, RpcMethodHandler> RpcMethodHandlerMap;
  typedef std::map<std::string, RpcEventHandler> RpcEventHandlerMap;
  
  ZMQJsonRpcServer();
  ~ZMQJsonRpcServer();
  bool startRpcService(const std::string& identity, const std::string& protocol, const std::string& bindAddress, unsigned short rpcPort);
  void stopRpcService();
  void registerMethod(const std::string& method, RpcMethodHandler handler);
  void registerNotifier(const std::string& event, RpcEventHandler handler);
protected:
  void executeRpc(ZMQMessage& task, ZMQMessage& response);
  bool handleNotify(ZMQMessage& message);
  void readNotifications();
  ZMQProactor _proactor;
  ZMQSocket _reactor;
  ZMQSocket _reactorControl;
  std::string _quitUUID;
  boost::thread* _pReadThread;
  bool _isReading;
  RpcMethodHandlerMap _methods;
  RpcEventHandlerMap _notifiers;
};


#endif	/* ZMQJSONRPCSERVER_H */

