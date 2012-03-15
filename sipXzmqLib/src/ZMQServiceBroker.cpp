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


#include "ZMQServiceBroker.h"

static const std::string BROKER_TERMINATE_STRING = "ZMQServiceBroker::terminate";

ZMQServiceBroker::ZMQServiceBroker() :
  _frontEnd(ZMQ_ROUTER),
  _backEnd(ZMQ_DEALER),
  _pReadThread(0),
  _isReading(false)
{
}

ZMQServiceBroker::~ZMQServiceBroker()
{
  stopServingMessages();
  delete _pReadThread;
  ZMQ_LOG_INFO("ZMQServiceBroker DESTROYED");
}

bool ZMQServiceBroker::startServingMessages(
  const std::string& frontEndAddress,
  const std::string& backEndAddress,
  ZMQSocket::Error& error)
{
  if (_pReadThread || _isReading )
  {
    error.num = -1;
    error.what = "FAILED condition (_pReadThread || _isReading)";
    return false;
  }

  if (!_frontEnd.bind(frontEndAddress, error))
    return false;

  if (!_backEnd.bind(backEndAddress, error))
    return false;

  _isReading = true;

  _pReadThread = new boost::thread(boost::bind(&ZMQServiceBroker::serveMessages, this));
  return true;
}

void ZMQServiceBroker::serveMessages()
{
  //  Initialize poll set
  zmq::pollitem_t items [] = {
      { _frontEnd.zmqSocket(), 0, ZMQ_POLLIN, 0 },
      { _backEnd.zmqSocket(),  0, ZMQ_POLLIN, 0 }
  };
  while(_isReading)
  {
    zmq::message_t message;
    int64_t more;           //  Multipart detection
    zmq::poll (&items [0], 2, -1);
    if (items [0].revents & ZMQ_POLLIN)
    {
      while (true)
      {
        //  Process all parts of the message
        _frontEnd.zmqSocket().recv(&message);

        if (message.size() == BROKER_TERMINATE_STRING.size())
        {
          std::string data(static_cast<char*>(message.data()), message.size());
          if (data == BROKER_TERMINATE_STRING)
          {
            ZMQ_LOG_INFO( "ZMQServiceBroker::serveMessages ENDED");
            _isReading = false;
            return;
          }
        }
        size_t more_size = sizeof (more);
        _frontEnd.zmqSocket().getsockopt(ZMQ_RCVMORE, &more, &more_size);
        _backEnd.zmqSocket().send(message, more ? ZMQ_SNDMORE : 0);
        
        if (!more)
          break;      //  Last message part
      }
    }

    if (items [1].revents & ZMQ_POLLIN)
    {
      while (1)
      {
        //  Process all parts of the message
        _backEnd.zmqSocket().recv(&message);
        size_t more_size = sizeof (more);
        _backEnd.zmqSocket().getsockopt(ZMQ_RCVMORE, &more, &more_size);
        _frontEnd.zmqSocket().send(message, more? ZMQ_SNDMORE: 0);
        if (!more)
          break;      //  Last message part
      }
    }
  }
  _isReading = false;
  ZMQ_LOG_INFO( "ZMQServiceBroker::serveMessages ENDED");
}

void ZMQServiceBroker::stopServingMessages()
{
  if (!_isReading)
    return;
  
  ZMQSocket::Error error;
  ZMQSocket _closer(ZMQ_REQ);
  ZMQSocket _closerBackend(ZMQ_REQ);
  ZMQMessage closeMsg(BROKER_TERMINATE_STRING);
  if (_closerBackend.connect(_backEnd.getBindAddress(), error))
  {
    if (_closer.connect(_frontEnd.getBindAddress(), error))
    {
      _closer.send(closeMsg, error);
    }
  }

  if (_pReadThread)
    _pReadThread->join();

  _closer.close(100);
  _frontEnd.close(100);
  _backEnd.close(100);

}