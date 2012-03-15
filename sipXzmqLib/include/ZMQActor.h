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

#ifndef ZMQACTOR_H
#define	ZMQACTOR_H

#include "ZMQSubscriber.h"
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

class ZMQActor : protected ZMQSubscriber
{
  //
  // Implements an in-process threaded actor.
  // You can either call putTask directly or you can subscribe
  // to a Reactor instance for tasks
  //
public:
  typedef boost::function<bool(ZMQMessage&)> TaskCallBack;
  ZMQActor(const std::string& address = "");
  ~ZMQActor();
  void putTask(ZMQMessage& task);
  bool startPerformingTasks(TaskCallBack taskCallBack, ZMQSocket::Error& error);
  void stopPerformingTasks();
  const std::string& getAddress() const;
  bool subscibeToReactor(const std::string& reactorAddress, const std::string& eventStringFilter, ZMQSocket::Error& error);
protected:
  void readTasks();
  bool readReactorTasks(const std::string& id, const std::string& address, ZMQMessage& message);
  ZMQSocket _actor;
  ZMQSocket _taskQueue;
  TaskCallBack _taskCallBack;
  std::string _address;
  boost::thread* _pReadThread;
  bool _isReading;
  bool _isSubscribed;
  std::string _eventStringFilter;
};

//
// Inlines
//

inline const std::string& ZMQActor::getAddress() const
{
  return _address;
}

#endif	/* ZMQACTOR_H */

