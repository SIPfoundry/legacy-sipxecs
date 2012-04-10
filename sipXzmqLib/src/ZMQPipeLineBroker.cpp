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


#include "ZMQPipeLineBroker.h"


ZMQPipeLineBroker::ZMQPipeLineBroker() :
   _pReadThread(0),
  _isReading(false),
  _frontEnd(ZMQ_PULL),
   _backEnd(ZMQ_PUSH),
  _controller(ZMQ_PUSH)
{

}

ZMQPipeLineBroker::~ZMQPipeLineBroker()
{
  stopServingMessages();
  delete _pReadThread;
}

bool ZMQPipeLineBroker::startServingMessages(
  const std::string& frontEndAddress,
  const std::string& backEndAddress,
  ZMQSocket::Error& error)
{
  if (_isReading || _pReadThread)
  {
    error.num = -1;
    error.what = "ZMQPipeLineBroker::startServingMessages failed (!_isReading && !_pReadThread) condition.";
    return false;
  }
  _frontEndAddress = frontEndAddress;
  _backEndAddress = backEndAddress;
  if (_frontEnd.bind(_frontEndAddress, error))
  {
    if (_backEnd.bind(_backEndAddress, error))
    {
      _isReading = true;
      _pReadThread = new boost::thread(boost::bind(&ZMQPipeLineBroker::serviceLoop, this));
    }
  }
  return false;
}

void ZMQPipeLineBroker::stopServingMessages()
{
  if (!_isReading || !_pReadThread)
    return;

  ZMQSocket::Error error;

  if (!_controller.connect(_frontEndAddress, error))
  {
    return;
  }

  ZMQMessage quit("ZMQPipeLineBroker::stopServingMessages");
  _controller.send(quit, error);

  _pReadThread->join();
}

void ZMQPipeLineBroker::serviceLoop()
{
  ZMQSocket::Error error;
  while(_isReading)
  {
    ZMQMessage message;
    if (_frontEnd.receive(message, error))
    {
      if (message.data() == "ZMQPipeLineBroker::stopServingMessages")
        break;
      message.setConsumed(false);
      _backEnd.send(message, error);
    }
  }
  _isReading = false;
}



