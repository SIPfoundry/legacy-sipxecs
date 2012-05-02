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

#ifndef STATEQUEUEDRIVERTEST_H
#define	STATEQUEUEDRIVERTEST_H

#include "StateQueueClient.h"
#include "StateQueueAgent.h"
#include "StateQueueDialogData.h"
#include "StateQueueDialogDataClient.h"
#include "StateQueueRegData.h"
#include "sqaclient.h"


#include <sstream>
#include <string>
#include <boost/thread.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include "TimedMap.h"


class StateQueueDriverTest : boost::noncopyable
{
public:
  StateQueueDriverTest(StateQueueAgent& agent) : _agent(agent){}
  ~StateQueueDriverTest(){}
  bool runTests();
protected:
  StateQueueAgent& _agent;

};

class ThreadedPop : public StateQueueClient
{
private:
  boost::thread *_pThread;
  boost::function<void(const std::string&, std::string&)> _eventHandler;
public:
  int total;
  ThreadedPop(
        const std::string& applicationId,
        const std::string& serviceAddress,
        const std::string& servicePort,
        const std::string& zmqEventId,
        std::size_t poolSize = 1) :
                StateQueueClient(StateQueueClient::Worker, applicationId, serviceAddress, servicePort, zmqEventId, poolSize),
                _pThread(0),
                total(0)
  {
  }

  ~ThreadedPop()
  {
    stop();
  }

  void stop()
  {
    _eventQueue.terminate();
    if (_pThread)
      _pThread->join();
    delete _pThread;
    _pThread = 0;
  }

  void start(boost::function<void(const std::string&, std::string&)> eventHandler)
  {
    _eventHandler = eventHandler;
    _pThread = new boost::thread(&ThreadedPop::threaded_pop, this);
  }

  void start()
  {
    _pThread = new boost::thread(&ThreadedPop::threaded_pop, this);
  }
protected:

  void threaded_pop()
  {
    while (!_terminate)
    {
      StateQueueMessage ev;
      std::string data;
      std::string id;
      if (pop(id, data))
      {
        if (_eventHandler)
          _eventHandler(_applicationId, data);
        total++;
      }
      else
      {
        break;
      }
    }
  }
};



#endif	/* STATEQUEUEDRIVERTEST_H */

