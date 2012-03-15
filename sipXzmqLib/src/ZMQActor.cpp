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

#include "ZMQActor.h"


ZMQActor::ZMQActor(const std::string& address) :
  _actor(ZMQ_PAIR),
  _taskQueue(ZMQ_PAIR),
  _address(address),
  _pReadThread(0),
  _isReading(false),
  _isSubscribed(false)
{
}

ZMQActor::~ZMQActor()
{
  stopPerformingTasks();
  delete _pReadThread;
}

void ZMQActor::putTask(ZMQMessage& task)
{
  ZMQSocket::Error error;
  _taskQueue.send(task, error);
}

void ZMQActor::readTasks()
{
  while(_isReading)
  {
    ZMQMessage message;
    ZMQSocket::Error error;
    std::string id;
    std::string address;
    if (_actor.receive(message, error))
    {
      if (message.data() == "ZMQActor::stopPerformingTasks")
      {
        break;
      }
      if (_taskCallBack)
        if (!_taskCallBack(message))
          break;
    }
    else
    {
      break;
    }
  }
  ZMQ_LOG_INFO("ZMQActor::readTasks ENDED");
  _isReading = false;
}

bool ZMQActor::startPerformingTasks(TaskCallBack taskCallBack, ZMQSocket::Error& error)
{
  if (_pReadThread || _isReading)
  {
    error.num = -1;
    error.what = "FAILED condition (_pReadThread || _isReading)";
    return false;
  }



  if (_address.empty())
    _address = std::string("inproc://ZMQActor-") + ZMQSocket::generateId();
  

  if (!_taskQueue.bind(_address, error))
  {
    return false;
  }

  if (!_actor.connect(_address, error))
  {
    return false;
  }

  _isReading = true;
  _taskCallBack = taskCallBack;
  _pReadThread = new boost::thread(boost::bind(&ZMQActor::readTasks, this));

  return true;
}

void ZMQActor::stopPerformingTasks()
{

  if (_isSubscribed)
    stopReadingEvents();
  
  if (_isReading)
  {
    ZMQMessage closeMsg("ZMQActor::stopPerformingTasks");
    putTask(closeMsg);

    _isReading = false;
    if (_pReadThread)
      _pReadThread->join();
  }

  _taskQueue.close(100);
  _actor.close(100);

}

bool ZMQActor::readReactorTasks(const std::string& id, const std::string& address, ZMQMessage& message)
{
  std::ostringstream task;
  task << id << " " << address << " " << message.data();
  ZMQMessage reactorMessage(task.str());
  putTask(reactorMessage);
  return true;
}

bool ZMQActor::subscibeToReactor(const std::string& reactorAddress, const std::string& eventStringFilter, ZMQSocket::Error& error)
{
  if (!subscribe(reactorAddress, eventStringFilter, error))
    return false;
  //
  // We got a subscription.  start reading events
  //
  startReadingEvents(boost::bind(&ZMQActor::readReactorTasks, this, _1, _2, _3));
  _isSubscribed = true;
  _eventStringFilter = eventStringFilter;
  return true;
}


