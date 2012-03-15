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


#include "ZMQProactor.h"
#include "ZMQPoll.h"


ZMQProactor::ZMQProactor() :
  _taskSocket(ZMQ_REP),
  _controlSocket(ZMQ_PAIR),
  _isReading(false),
  _pReadThread(0)
{
}

ZMQProactor::~ZMQProactor()
{
  stopProactor();
  delete _pReadThread;
  ZMQ_LOG_INFO("ZMQProactor DESTROYED");
}

bool ZMQProactor::startProactor(const std::string& serviceId, const std::string& localAddressId, TaskCallBack task, ZMQSocket::Error& error)
{
  if (_isReading || _pReadThread)
  {
    error.num = -1;
    error.what = "ZMQProactor::startProactor failed condition !_isReading && !_pReadThread";
    return false;
  }
  _task = task;
  _serviceId = serviceId;
  _localAddressId = localAddressId;
  _controlSocketUUID = "inproc://ZMQProactor.control-";
  _controlSocketUUID += ZMQSocket::generateId();
  if (!_controlSocket.bind(_controlSocketUUID, error))
    return false;

  //
  // Start the proactor loop
  //
  _pReadThread = new boost::thread(boost::bind(&ZMQProactor::proactorLoop, this));
  return true;
}

void ZMQProactor::stopProactor()
{
  if (!_isReading || !_pReadThread)
    return;
  ZMQSocket::Error error;
  ZMQSocket controller(ZMQ_PAIR);

  if (controller.connect(_controlSocketUUID, error))
  {
    ZMQMessage quit("ZMQProactor::stopProactor");
    controller.send(quit, error);
  }
  _pReadThread->join();
}

void ZMQProactor::proactorLoop()
{
  ZMQPoll2 poller(&_taskSocket, &_controlSocket);
  _isReading = true;
  ZMQSocket::Error error;
  while(_isReading)
  {
    if (!poller.poll())
    {
      continue;
    }
    if (_taskSocket.is_POLLIN())
    {
      ZMQMessage task;
      ZMQMessage response;
      if (_taskSocket.receive(task, error))
      {
        handleTask(task, response);
        _taskSocket.send(response, error);
      }
    }
    
    if (_controlSocket.is_POLLIN())
    {
        ZMQMessage control;
        _controlSocket.receive(control, error);
        break; /// all signal is a kill
    }
  }
  ZMQ_LOG_INFO( "ZMQProactor::proactorLoop ENDED");
  _isReading = false;
}

void ZMQProactor::handleTask(ZMQMessage& task, ZMQMessage& response)
{
  ZMQMessage::Headers headers;
  if (!task.parseHeaders(headers))
  {
    std::ostringstream strm;
    strm << headers.identifier << " " << headers.address << " zmq.error='Unable to parse request headers'";
    response.data() = strm.str();
    return;
  }

  if (headers.identifier.size() < _serviceId.size())
  {
    std::ostringstream strm;
    strm << headers.identifier << " " << headers.address << " zmq.error='Invalid Service ID'";
    response.data() = strm.str();
    return;
  }

  std::string prefix = headers.identifier.substr(0, _serviceId.size());
  if (prefix !=  _serviceId)
  {
    std::ostringstream strm;
    strm << headers.identifier << " " << headers.address <<   "zmq.error='Invalid Service ID'";
    response.data() = strm.str();
    return;
  }

  if (!_task)
  {
    std::ostringstream strm;
    strm << headers.identifier << " " << headers.address << " zmq.error='Server Failure (Task is null)'";
    response.data() = strm.str();
    return;
  }

  ZMQMessage newTask(headers.body);
  ZMQMessage newResponse;
  _task(newTask, newResponse);

  std::ostringstream final;
  final << headers.identifier << " " << headers.address << " " << _localAddressId << " " << newResponse.data();
  response.data() = final.str();
}
