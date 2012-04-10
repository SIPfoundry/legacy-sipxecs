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

#ifndef ZMQSUBSCRIBER_H_INCLUDED
#define	ZMQSUBSCRIBER_H_INCLUDED

#include "ZMQSocket.h"
#include "ZMQNotifier.h"
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

class ZMQSubscriber : protected ZMQSocket
{
  //
  // Implements a subscriber object to events posted by ZMQNotifier
  //
public:
  typedef boost::function<bool(const std::string&, const std::string&, ZMQMessage&)> Callback;
  ZMQSubscriber();
  ~ZMQSubscriber();
  bool subscribe(const std::string& address, const std::string& eventString, ZMQSocket::Error& error);
  void startReadingEvents(Callback eventCallback);
  void stopReadingEvents();
protected:
  void readEvents();
  bool receiveEvents(ZMQMessage& message, std::string& eventId, std::string& address, ZMQSocket::Error& error);
  bool receiveEvents(std::string& message, std::string& eventId, std::string& address, ZMQSocket::Error& error);
private:
  Callback _eventCallBack;
  boost::thread* _pReadThread;
  bool _isReading;
  ZMQNotifier _exitNotifier;
  std::string _exitNotifierAddress;
};

#endif	/* ZMQSUBSCRIBER_H_INCLUDED */

