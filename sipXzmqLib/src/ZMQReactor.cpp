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


#include "ZMQReactor.h"
#include "ZMQReactorClient.h"

ZMQReactor::ZMQReactor(unsigned int highWaterMark) :
  ZMQNotifier(highWaterMark),
  _brokerSocket(ZMQ_PULL),
  _pReadThread(0),
  _isReading(false)
{
  std::ostringstream strm;
  strm << ZMQSocket::generateId() << "-" << "ZMQReactor::stopReactor" << "-" << ZMQSocket::generateId();
  _exitNotifier = strm.str();
}

ZMQReactor::~ZMQReactor()
{
  ZMQ_LOG_INFO("ZMQReactor DESTRUCTOR");
  stopReactor();
  delete _pReadThread;
  ZMQ_LOG_INFO("ZMQReactor DESTROYED");
}

bool ZMQReactor::startReactor(const std::string& localAddress, const std::string& brokerFrontEndAddress, const std::string& brokerBackEndAddress, ZMQSocket::Error& error)
{
  if (_pReadThread || _isReading)
  {
    error.num = -1;
    error.what = "FAILED condition (_pReadThread || _isReading)";
    return false;
  }
  _brokerFrontEndAddress = brokerFrontEndAddress;
  _brokerBackEndAddress = brokerBackEndAddress;
  //
  // First conenct to the broker backend
  //
  if (!_brokerSocket.connect(_brokerBackEndAddress, error))
    return false;

  //
  // Start accepting Actors
  //
  if (!startAcceptingSubscribers(localAddress, error))
    return false;
  //
  // start the reactor thread
  //
  if (!_isReading)
  {
    _isReading = true;
    _pReadThread = new boost::thread(boost::bind(&ZMQReactor::readServiceRequests, this));
  }
  return true;
}


void ZMQReactor::stopReactor()
{
  if (!_pReadThread || !_isReading)
    return;
  ZMQSocket::Error error;
  ZMQReactorClient exitClient;

  if (exitClient.connectToService(_brokerFrontEndAddress, error))
  {
    std::ostringstream quitMsg;
    quitMsg << _exitNotifier << " ZMQReactor::stopReactor ZMQEvent";
    ZMQMessage quit(quitMsg.str());
    exitClient.postEvent(quit, error);
  }
  _pReadThread->join();
  _brokerSocket.close(0);
}

void ZMQReactor::readServiceRequests()
{
  ZMQSocket::Error error;

  while(_isReading)
  {
    ZMQMessage message;
    if (_brokerSocket.receive(message, error))
    {
      ZMQMessage::Headers headers;
      if (message.parseHeaders(headers))
      {
        //
        // We got the headers.  publish it
        //
        if (headers.identifier == "ZMQReactor::terminate" || headers.identifier == _exitNotifier)
          break;
        
        notifyEvent(headers.identifier, headers.address, headers.body, error);
      }
    }
  }
  _isReading = false;
  ZMQ_LOG_INFO("ZMQReactor::readServiceRequests ENDED");
}

