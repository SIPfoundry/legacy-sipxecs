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


#include "ZMQProactorClient.h"
#include "ZMQPoll.h"

static const std::string ZMQ_RESPONSE_PREFIX = "z.m.q.response.";

ZMQProactorClient::ZMQProactorClient() :
  _proactorClient(ZMQ_REQ)
{
}

ZMQProactorClient::~ZMQProactorClient()
{
  stopProactorClient();
  ZMQ_LOG_INFO("ZMQProactorClient DESTROYED");
}

bool ZMQProactorClient::startProactorClient(const std::string& serviceId, const std::string& localAddressIdentifier, ZMQSocket::Error& error)
{
  _localAddressIdentifier = localAddressIdentifier;
  _serviceId = serviceId;
  return _actor.startPerformingTasks(boost::bind(&ZMQProactorClient::handleRequest, this, _1), error);
}

bool ZMQProactorClient::handleRequest(ZMQMessage& message)
{
  ZMQSocket::Error error;

  ZMQMessage newRequest(message);

  if (_proactorClient.send(newRequest, error))
  {
    ZMQMessage response;
    if (_proactorClient.receive(response, error))
    {
       handleResponse(response);
    }
  }
  return true;
}


bool ZMQProactorClient::handleResponse(ZMQMessage& message)
{
  //
  // Get the response id of the message from the body of the response
  //
  ZMQMessage::Headers headers;

  if (!message.parseHeaders(headers))
  {
    //
    // Do not return false or the read thread will exit
    //
    return true;
  }
  //
  // Check futures if a response context is available
  //
  _blockingRequestsMutex.lock();
  BlockingRequests::iterator pRequest = _blockingRequests.find(headers.identifier);
  if (pRequest == _blockingRequests.end())
  {
    _blockingRequestsMutex.unlock();
    return true;
  }
  //
  // if future is available, signal it to unblock the waiting blocking_send
  //
  pRequest->second->unblock(message);
  _blockingRequests.erase(pRequest);
  _blockingRequestsMutex.unlock();

  return true;
}





bool ZMQProactorClient::blocking_send(ZMQBlockingRequest::Ptr message, unsigned long waitTimeInMilliseconds, ZMQSocket::Error& error)
{
  std::string transactionId = ZMQSocket::generateId();
  std::string requestId = _serviceId + "~" + transactionId;

  std::string oldData = message->data();
  message->data() = requestId + " " + _localAddressIdentifier + " " + oldData;

  _blockingRequestsMutex.lock();
  if (_blockingRequests.find(requestId) != _blockingRequests.end())
  {
    error.num = -1;
    error.what = "ZMQProactorClient::blocking_send already has the request in queue previously.";
    _blockingRequestsMutex.unlock();
    return false;
  }
  _blockingRequests[requestId] = message;
  _blockingRequestsMutex.unlock();

  _actor.putTask(*message);
  if (message->wait(waitTimeInMilliseconds))
  {
    ZMQMessage::Headers headers;
    if (!message->response().parseHeaders(headers))
    {
      error.num = -1;
      error.what = "ZMQProactorClient::blocking_send is unable to parse the response - " + message->response().data();
      ZMQ_LOG_DEBUG(error.what);
      return false;
    }
    //
    // Remove the proprietary headers and just leave the body in the response
    //
    

    size_t offset = headers.body.find_first_of(' ');
    if (offset != std::string::npos && headers.body.size() > offset + 1 )
      message->response().data() = headers.body.substr(offset + 1);
    else
      message->response().data() = headers.body;

    return true;
  }

  return false;
}



void ZMQProactorClient::stopProactorClient()
{
  _actor.stopPerformingTasks();
}

bool ZMQProactorClient::async_send(ZMQMessage& message, ResponseCallback cb, ZMQSocket::Error& error)
{
  return false;
}