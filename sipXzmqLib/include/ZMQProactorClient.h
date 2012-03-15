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

#ifndef ZMQPROACTORCLIENT_H
#define	ZMQPROACTORCLIENT_H

#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <map>

#include "ZMQBlockingRequest.h"
#include "ZMQReactorClient.h"
#include "ZMQReactor.h"
#include "ZMQActor.h"


class ZMQProactorClient
{
  //
  // Implementation of a proactor design pattern where a blocking.
  // Responses can be both aquired using async and blocking modes.
  // Proactors cannot be used with service brokers.
  //
public:
  typedef boost::function<void(ZMQMessage&)> ResponseCallback;
  typedef  std::map<std::string, ZMQBlockingRequest::Ptr> BlockingRequests;
  ZMQProactorClient();
  ~ZMQProactorClient();
  bool connect(const std::string& proactor, ZMQSocket::Error& error);
  bool startProactorClient(const std::string& serviceId, const std::string& localAddressIdentifier, ZMQSocket::Error& error);
  void stopProactorClient();
  bool async_send(ZMQMessage& message, ResponseCallback cb, ZMQSocket::Error& error);
  bool blocking_send(ZMQBlockingRequest::Ptr message, ZMQSocket::Error& error);

protected:
  bool blocking_send(ZMQBlockingRequest::Ptr message, unsigned long waitTimeInMilliseconds, ZMQSocket::Error& error);
  bool handleRequest(ZMQMessage& message);
  bool handleResponse(ZMQMessage& message);
  ZMQActor _actor;
  ZMQSocket _proactorClient;
  std::string _controlUUID;
  BlockingRequests _blockingRequests;
  boost::mutex _blockingRequestsMutex;
  std::string _localAddressIdentifier;
  std::string _serviceId;
};

//
// Inlines
//
inline bool ZMQProactorClient::blocking_send(ZMQBlockingRequest::Ptr message, ZMQSocket::Error& error)
{
  unsigned long waitTimeInMilliseconds = 0;
  return blocking_send(message, waitTimeInMilliseconds, error);
}

inline bool ZMQProactorClient::connect(const std::string& proactor, ZMQSocket::Error& error)
{
  return _proactorClient.connect(proactor, error);
}

#endif	/* ZMQPROACTORCLIENT_H */

