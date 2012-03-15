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


#include <boost/lexical_cast.hpp>
#include <zmq.hpp>
#include "ZMQSubscriber.h"


static const std::string& cterm = "zmq::terminate_subscriptions";

ZMQSubscriber::ZMQSubscriber() :
  ZMQSocket(ZMQ_SUB),
  _pReadThread(0),
  _isReading(false),
  _exitNotifier(1)
{
  _exitNotifierAddress = "inproc://";
  _exitNotifierAddress += ZMQSocket::generateId();
  _exitNotifierAddress += ZMQSocket::generateId();
  _exitNotifierAddress += "-";
  _exitNotifierAddress += boost::lexical_cast<int>((int)this);

  ZMQSocket::Error error;
  assert(_exitNotifier.startAcceptingSubscribers(_exitNotifierAddress, error));
  _socket.connect(_exitNotifierAddress.c_str());
  _socket.setsockopt(ZMQ_SUBSCRIBE, cterm.c_str(), cterm.size());
}

ZMQSubscriber::~ZMQSubscriber()
{
  stopReadingEvents();
  delete _pReadThread;
  ZMQ_LOG_INFO("ZMQSubscriber DESTROYED");
}

bool ZMQSubscriber::subscribe(const std::string& address, const std::string& eventString, ZMQSocket::Error& error)
{
  try
  {
    _socket.connect(address.c_str());
    _socket.setsockopt(ZMQ_SUBSCRIBE, eventString.c_str(), eventString.size());
  }
  catch(zmq::error_t& error_)
  {
    convertError(error_, error);
    return false;
  }
  return true;
}

bool ZMQSubscriber::receiveEvents(ZMQMessage& message, std::string& eventId, std::string& address, ZMQSocket::Error& error)
{
  //
  // First receive the id
  //
  ZMQMessage eventIdMessage;
  if (!receive(eventIdMessage, error))
    return false;
  eventId = eventIdMessage.data();
  //
  // Receive the address
  //
  ZMQMessage addressMessage;
  if (!receive(addressMessage, error))
    return false;
  address = addressMessage.data();
  //
  // Now let us receive the actual event
  //
  return receive(message, error);
}

bool ZMQSubscriber::receiveEvents(std::string& message, std::string& eventId, std::string& address, ZMQSocket::Error& error)
{
  ZMQMessage eventMessage;
  if (!receiveEvents(eventMessage, eventId, address, error))
    return false;
  message = eventMessage.data();
  return true;
}

void ZMQSubscriber::readEvents()
{
  while(_isReading)
  {
    ZMQMessage message;
    ZMQSocket::Error error;
    std::string id;
    std::string address;
    if (receiveEvents(message, id, address, error))
    {
      if (id == "zmq::terminate_subscriptions")
        break;
      if (_eventCallBack)
        if (!_eventCallBack(id, address, message))
          break;
    }
    else
    {
      if (_eventCallBack)
      {
        ZMQMessage errorMsg(error.what);
        _eventCallBack("ERROR", "ZMQSubscriber::readEvents", message);
      }
      break;
    }
  }
  ZMQ_LOG_INFO("ZMQSubscriber::readEvents ENDED.");
  _isReading = false;
}

void ZMQSubscriber::startReadingEvents(Callback eventCallback)
{
  if (_pReadThread || _isReading)
    return;
  _eventCallBack = eventCallback;
  _pReadThread = new boost::thread(boost::bind(&ZMQSubscriber::readEvents, this));
  _isReading = true;
}

void ZMQSubscriber::stopReadingEvents()
{
  if (_isReading)
  {
    ZMQSocket::Error error;
    _exitNotifier.notifyEvent(cterm, "ZMQSubscriber::stopReadingEvents", "ZMQEvent", error);
    if (_pReadThread)
      _pReadThread->join();
    close(0);
  }
}

