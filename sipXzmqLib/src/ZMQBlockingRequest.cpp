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


#include "ZMQBlockingRequest.h"
#include "ZMQLogger.h"


ZMQBlockingRequest::ZMQBlockingRequest() :
  ZMQMessage(),
  _responseReady(false)
{
}

ZMQBlockingRequest::ZMQBlockingRequest(const std::string& data) :
  ZMQMessage(data),
  _responseReady(false)
{

}

ZMQBlockingRequest::~ZMQBlockingRequest()
{
}

void ZMQBlockingRequest::unblock(ZMQMessage& response)
{
  {
      boost::lock_guard<boost::mutex> lock(_mutex);
      _response = response;
      assert(!_response.data().empty());
      _responseReady=true;
  }
  _cond.notify_one();
}

bool ZMQBlockingRequest::wait(unsigned int milliseconds)
{
  if (!milliseconds)
  {
    boost::unique_lock<boost::mutex> lock(_mutex);
    while (!_responseReady)
      _cond.wait(lock);
    return true;
  }
  else
  {
    boost::unique_lock<boost::mutex> lock(_mutex);
    if (!_responseReady)
      _cond.timed_wait(lock, boost::posix_time::milliseconds(milliseconds));
    return true;
  }
  return false;
}
