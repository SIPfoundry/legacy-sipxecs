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

#ifndef ZMQNOTIFIER_H_INCLUDED
#define	ZMQNOTIFIER_H_INCLUDED

#include "ZMQSocket.h"

class ZMQNotifier : protected ZMQSocket
{
  //
  // Implements a publisher of events sunscribed to by ZMQSubcriber objects
  //
public:
  ZMQNotifier(unsigned int highWaterMark);
  ~ZMQNotifier();
  bool startAcceptingSubscribers(const std::string& address, ZMQSocket::Error& error);
  bool notifyEvent(const std::string& eventId, const std::string& localAddress, ZMQMessage& message, ZMQSocket::Error& error);
  bool notifyEvent(const std::string& eventId, const std::string& localAddress, const std::string& message, ZMQSocket::Error& error);
  void terminateSubscriptions();
};

//
// Inline
//
inline bool ZMQNotifier::startAcceptingSubscribers(const std::string& address, ZMQSocket::Error& error)
{
  return bind(address, error);
}

#endif	/* ZMQNOTIFIER_H_INCLUDED */

