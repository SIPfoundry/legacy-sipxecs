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


#ifndef ZMQSERVICEBROKER_H
#define	ZMQSERVICEBROKER_H

#include <boost/thread.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include "ZMQSocket.h"
#include "ZMQMessage.h"
#include "ZMQPoll.h"

class ZMQServiceBroker
{
public:
  ZMQServiceBroker();
  ~ZMQServiceBroker();
  bool startServingMessages(
    const std::string& frontEndAddress,
    const std::string& backEndAddress,
    ZMQSocket::Error& error);
  void stopServingMessages();
protected:
  void serveMessages();
  ZMQSocket _frontEnd;
  ZMQSocket _backEnd;
  boost::thread* _pReadThread;
  bool _isReading;
  
};


#endif	/* ZMQSERVICEBROKER_H */

