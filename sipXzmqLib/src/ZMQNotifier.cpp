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

#include <zmq.hpp>

#include "ZMQNotifier.h"
#include <stdint.h>

ZMQNotifier::ZMQNotifier(unsigned int highWaterMark) :
  ZMQSocket(ZMQ_PUB)
{
  uint64_t hwm = highWaterMark;
  _socket.setsockopt( ZMQ_HWM, &hwm, sizeof (hwm));
}

ZMQNotifier::~ZMQNotifier()
{
}

bool ZMQNotifier::notifyEvent(const std::string& eventId, const std::string& localAddress, ZMQMessage& message, ZMQSocket::Error& error)
{
  ZMQMessage eventMessage(eventId);
  if (!sendMore(eventMessage, error))
    return false;

  ZMQMessage eventAddress(localAddress);
  if (!sendMore(eventAddress, error))
    return false;

  return send(message, error);
}

bool ZMQNotifier::notifyEvent(const std::string& eventId, const std::string& localAddress, const std::string& message, ZMQSocket::Error& error)
{
  ZMQMessage notifyMessage(message);
  return notifyEvent(eventId, localAddress, notifyMessage, error);
}

void ZMQNotifier::terminateSubscriptions()
{
  ZMQSocket::Error error;
  notifyEvent("zmq::terminate_subscriptions", "ZMQNotifier", "", error);
}

