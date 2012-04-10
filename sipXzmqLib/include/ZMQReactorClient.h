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

#ifndef ZMQREACTORCLIENT_H
#define	ZMQREACTORCLIENT_H


#include "ZMQSocket.h"

class ZMQReactorClient : protected ZMQSocket
  //
  // Implements a client for the reactor service
  // 
{
public:
  ZMQReactorClient();
  ~ZMQReactorClient();
  bool connectToService(const std::string& brokerAddress, ZMQSocket::Error& error);
  bool postEvent(ZMQMessage& message, ZMQSocket::Error& error);
  void terminateReactor();
};

//
// Inlines
//

inline bool ZMQReactorClient::connectToService(const std::string& brokerAddress, ZMQSocket::Error& error)
{
  return connect(brokerAddress, error);
}



#endif	/* ZMQREACTORCLIENT_H */

