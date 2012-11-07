/*
 * Copyright (c) eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */

#ifndef NOTIFICATIONDEALER_H
#define	NOTIFICATIONDEALER_H

#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <cassert>
#include <boost/thread.hpp>
#include <zmq.hpp>
#include "ServiceOptions.h"
#include "sqaclient.h"

class StateQueueNotification
{
public:
  struct NotifyData
  {
    std::string aor;
    std::string contact;
    std::string content;
    std::string key;
  };

  StateQueueNotification(ServiceOptions& options);

  StateQueueNotification(
    const std::string& sqaControlAddress,
    const std::string& sqaControlPort);

  ~StateQueueNotification();

  void run();
  void stop();
  void enqueue(const NotifyData& notification);

private:
  void deal(NotifyData& notification);
  bool dequeue(NotifyData& notification);
  void internal_run();
  boost::condition_variable _queueCond;
  boost::mutex _queueMut;
  std::queue<NotifyData> _queue;
  std::string _exitString;
  boost::thread* _queueThread;
  bool _isRunning;
  SQAPublisher* _pPublisher;
  std::string _sqaControlAddress;
  std::string _sqaControlPort;
};


//
// Inlines
//

inline StateQueueNotification::StateQueueNotification(ServiceOptions& options) :
  _queueThread(0),
  _isRunning(false),
  _pPublisher(0)
{
  #define within(num) (int) ((float) (num) * random () / (RAND_MAX + 1.0))
  std::ostringstream ss;
  ss << std::hex << std::uppercase
     << std::setw(4) << std::setfill('0') << within (0x10000) << "-exit";
  _exitString = ss.str();

  options.getOption("sqa-control-address", _sqaControlAddress);
  options.getOption("sqa-control-port", _sqaControlPort);
}

inline StateQueueNotification::StateQueueNotification(const std::string& sqaControlAddress,
    const std::string& sqaControlPort) :
  _queueThread(0),
  _isRunning(false),
  _pPublisher(0),
  _sqaControlAddress(sqaControlAddress),
  _sqaControlPort(sqaControlPort)
{
  #define within(num) (int) ((float) (num) * random () / (RAND_MAX + 1.0))
  std::ostringstream ss;
  ss << std::hex << std::uppercase
     << std::setw(4) << std::setfill('0') << within (0x10000) << "-exit";
  _exitString = ss.str();
}

inline StateQueueNotification::~StateQueueNotification()
{
  stop();
  delete _queueThread;
  _queueThread = 0;
}

inline void StateQueueNotification::deal(NotifyData& notification)
{
  std::ostringstream ss;
  ss << "sswdata." << notification.aor;
  bool noresponse = true;
   _pPublisher->publish(ss.str().c_str(), notification.content.c_str(), noresponse);
}

inline void StateQueueNotification::run()
{
  assert(!_queueThread);
  assert(!_pPublisher);
  _queueThread = new boost::thread(boost::bind(&StateQueueNotification::internal_run, this));
}

inline void StateQueueNotification::internal_run()
{
  assert(!_isRunning && _queueThread);


  if (_sqaControlAddress.empty() || _sqaControlPort.empty())
    return;

  std::ostringstream ss;
  ss << "ssw-" << std::hex << std::uppercase
     << std::setw(4) << std::setfill('0') << (int) ((float) (0x10000) * random () / (RAND_MAX + 1.0));

  _pPublisher = new SQAPublisher(ss.str().c_str(), _sqaControlAddress.c_str(), _sqaControlPort.c_str(), 1, 100, 100);


  std::string key;
  _isRunning = true;
  while(_isRunning)
  {
    NotifyData notification;
    dequeue(notification);
    if (_exitString == notification.key)
      break;
    deal(notification);
  }
}

inline void StateQueueNotification::stop()
{
  if (_queueThread && _queueThread->joinable())
  {
    NotifyData exitSignal;
    exitSignal.key = _exitString;
    enqueue(exitSignal);
    _isRunning = false;
    _queueThread->join();
    delete _pPublisher;
    _pPublisher = 0;
  }
}

inline bool StateQueueNotification::dequeue(NotifyData& notification)
{
  boost::unique_lock<boost::mutex> lock(_queueMut);
  _queueCond.wait(lock);
  if (_queue.empty())
    return false;
  notification = _queue.front();
  _queue.pop();
  return true;
}

inline void StateQueueNotification::enqueue(const NotifyData& notification)
{
  if (_isRunning)
  {
    boost::lock_guard<boost::mutex> lock(_queueMut);
    if (_queue.size() > 50)
      _queue.pop();
    _queue.push(notification);
    _queueCond.notify_one();
  }
}

#endif

