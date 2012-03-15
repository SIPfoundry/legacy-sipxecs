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

#ifndef ZMQPROACTOR_H
#define	ZMQPROACTOR_H


#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include "ZMQSocket.h"


class ZMQProactor
{
public:
  typedef boost::function<void(ZMQMessage& request, ZMQMessage& response)> TaskCallBack;
  ZMQProactor();
  ~ZMQProactor();
  bool startProactor(const std::string& serviceId, const std::string& localAddressId, TaskCallBack task, ZMQSocket::Error& error);
  bool connect(const std::string& brokerAddress, ZMQSocket::Error& error);
  bool bind(const std::string& localAddress, ZMQSocket::Error& error);
  void stopProactor();
protected:
  void proactorLoop();
  void handleTask(ZMQMessage& task, ZMQMessage& response);
  TaskCallBack _task;
  ZMQSocket _taskSocket;
  ZMQSocket _controlSocket;
  std::string _controlSocketUUID;
  bool _isReading;
  boost::thread* _pReadThread;
  std::string _serviceId;
  std::string _localAddressId;
  std::string _brokerBackEndAddress;
};

//
// Inlines
//

inline bool ZMQProactor::connect(const std::string& brokerAddress, ZMQSocket::Error& error)
{
  return _taskSocket.connect(brokerAddress, error);
}

inline bool ZMQProactor::bind(const std::string& localAddress, ZMQSocket::Error& error)
{
  return _taskSocket.bind(localAddress, error);
}

#endif	/* ZMQPROACTOR_H */

