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

#ifndef STATEQUEUEPUBLISHER_H
#define	STATEQUEUEPUBLISHER_H

#include "StateQueueRecord.h"
#include <boost/thread.hpp>
#include "BlockingQueue.h"
#include "TimedQueue.h"
#include <set>
#include <map>

class StateQueueAgent;

class StateQueuePublisher
{
public:
  typedef boost::shared_mutex mutex_read_write;
  typedef boost::shared_lock<boost::shared_mutex> mutex_read_lock;
  typedef boost::lock_guard<boost::shared_mutex> mutex_write_lock;
  typedef std::set<std::string> Subscribers;
  typedef std::map<std::string, Subscribers> EventSubscribers;

  StateQueuePublisher(StateQueueAgent* pAgent);
  ~StateQueuePublisher();
  bool run();
  void stop();
  void publish(const StateQueueRecord& record);
  void setBindAddress(const std::string& bindAddress);
  void addSubscriber(const std::string& ev, const std::string& applicationId, int expires);
  void removeSubscriber(const std::string& ev, const std::string& applicationId);
  int countSubscribers(const std::string& ev);
protected:
  friend class StateQueueAgent;
  void internal_run();
  void onExpiredSubscription(const std::string& applicationId, const boost::any& event);
  StateQueueAgent* _pAgent;
  BlockingQueue<StateQueueRecord> _queue;
  boost::thread* _pThread;
  std::string _zmqBindAddress;
  bool _terminate;
  TimedQueue _subscriptionTimer;
  mutex_read_write _eventSubscribersMutex;
  EventSubscribers _eventSubscribers;
  TimedQueue::TimedQueueHandler _expireHandler;
};

//
// Inlines
//

inline void StateQueuePublisher::setBindAddress(const std::string& address)
{
  _zmqBindAddress = address;
}

#endif	/* STATEQUEUEPUBLISHER_H */

