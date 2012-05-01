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

#include "sqa/StateQueuePublisher.h"
#include "zmq.hpp"
#include "os/OsLogger.h"
#include "sqa/StateQueueAgent.h"

//  Convert string to 0MQ string and send to socket
static bool
s_send (zmq::socket_t & socket, const std::string & string) {

    zmq::message_t message(string.size());
    memcpy(message.data(), string.data(), string.size());

    bool rc = socket.send(message);
    return (rc);
}

//  Sends string as 0MQ string, as multipart non-terminal
static bool
s_sendmore (zmq::socket_t & socket, const std::string & string) {

    zmq::message_t message(string.size());
    memcpy(message.data(), string.data(), string.size());

    bool rc = socket.send(message, ZMQ_SNDMORE);
    return (rc);
}

StateQueuePublisher::StateQueuePublisher(StateQueueAgent * pAgent) :
  _pAgent(pAgent),
  _queue(50),
  _pThread(0),
  _terminate(false)
{
  _expireHandler = boost::bind(&StateQueuePublisher::onExpiredSubscription, this, _1, _2);
  OS_LOG_INFO(FAC_NET, "StateQueuePublisher CREATED.");
}

StateQueuePublisher::~StateQueuePublisher()
{
  stop();
  OS_LOG_INFO(FAC_NET, "StateQueuePublisher DESTROYED.");
}

void StateQueuePublisher::stop()
{
  _terminate = true;
  if (_pThread && _pThread->joinable())
  {
    StateQueueRecord record;
    record.id = "~StateQueuePublisher";
    _queue.enqueue(record);
    _pThread->join();
    delete _pThread;
    _pThread = 0;
  }
}

bool StateQueuePublisher::run()
{
  if (_pThread || _zmqBindAddress.empty())
    return false;
  _pThread = new boost::thread(boost::bind(&StateQueuePublisher::internal_run, this));
  return true;
}

void StateQueuePublisher::publish(const StateQueueRecord& record)
{
  _queue.enqueue(record);
}

void StateQueuePublisher::internal_run()
{
  zmq::context_t context(1);
  zmq::socket_t socket(context, ZMQ_PUB);

  try
  {
    socket.bind(_zmqBindAddress.c_str());
  }
  catch(zmq::error_t& error_)
  {
    return;
  }

  OS_LOG_INFO(FAC_NET, "StateQueuePublisher::internal_run() "
          << "Started accepting subscriptions at " << _zmqBindAddress);
  
  while(!_terminate)
  {
    StateQueueRecord record;
    if (_queue.dequeue(record))
    {
      //
      // exit
      //
      if (_terminate)
        break;

      //
      // publish
      //
      std::string eventId = record.id;


      if (!record.watcherData && eventId.size() < 7)
      {
        OS_LOG_ERROR(FAC_NET, "StateQueuePublisher::publish eventId is too short - " << eventId);
        continue;
      }

      try
      {
        s_sendmore(socket, eventId);

        std::string data;

        if (!record.watcherData)
        {
          for (std::vector<std::string>::const_iterator iter = record.exclude.begin();
                  iter != record.exclude.end(); iter++)
          {
            data += *iter;
            data += " ";
          }

          if (data.empty())
            data = "initial_data";

          OS_LOG_DEBUG(FAC_NET, "StateQueuePublisher::publish "
                  << " message-id: " << eventId
                  << " exclude-app-id: " << data);
        }
        else
        {
          data = record.data;
        }

        //
        // Send the address
        //
        s_sendmore(socket, _zmqBindAddress);
        //
        // Send the data vector
        //
        s_sendmore(socket, data);
        //
        // Send the number of subscribers
        //
        std::string ev = eventId.substr(0, 7);
        int count = countSubscribers(ev);
        std::string strcount = boost::lexical_cast<std::string>(count);
        s_send(socket, strcount);


        OS_LOG_DEBUG(FAC_NET, "StateQueuePublisher::publish ZeroMQ send: " << eventId << ":" << data);

        if (!record.watcherData && !count)
        {
          OS_LOG_WARNING(FAC_NET, "StateQueuePublisher::publish "
                << "ZERO subscribers to handle message-id: " << eventId);
        }
      }
      catch(zmq::error_t& error_)
      {
        OS_LOG_WARNING(FAC_NET, "StateQueuePublisher::publish "
                << "ZMQ Error sending publish " << eventId << " Error: " << error_.what());
      }
    }
  }
  OS_LOG_INFO(FAC_NET, "StateQueuePublisher::internal_run() TERMINATED.");
}

void StateQueuePublisher::addSubscriber(const std::string& ev, const std::string& applicationId, int expires)
{
  //
  // Add to the set of active subscribers
  //
  {
  mutex_write_lock lock(_eventSubscribersMutex);
  EventSubscribers::iterator iter = _eventSubscribers.find(ev);
  if (iter == _eventSubscribers.end())
    _eventSubscribers[ev] = Subscribers();
  Subscribers& subscribers = _eventSubscribers[ev];
  subscribers.insert(applicationId);
  }
  //
  // Register the timer for this subscription based on expires
  //
  _subscriptionTimer.enqueue(applicationId, ev, _expireHandler, expires);
}

void StateQueuePublisher::removeSubscriber(const std::string& ev, const std::string& applicationId)
{
  {
  mutex_write_lock lock(_eventSubscribersMutex);
  EventSubscribers::iterator iter = _eventSubscribers.find(ev);
  if (iter == _eventSubscribers.end())
    return;
  Subscribers& subscribers = _eventSubscribers[ev];
  subscribers.erase(applicationId);
  }
  _subscriptionTimer.erase(applicationId);
}

void StateQueuePublisher::onExpiredSubscription(const std::string& applicationId, const boost::any& event)
{
  std::string ev = boost::any_cast<std::string>(event);
  mutex_write_lock lock(_eventSubscribersMutex);
  EventSubscribers::iterator iter = _eventSubscribers.find(ev);
  if (iter == _eventSubscribers.end())
    return;
  Subscribers& subscribers = _eventSubscribers[ev];
  subscribers.erase(applicationId);
}

int StateQueuePublisher::countSubscribers(const std::string& ev)
{
  mutex_write_lock lock(_eventSubscribersMutex);
  EventSubscribers::iterator iter = _eventSubscribers.find(ev);
  if (iter == _eventSubscribers.end())
    return 0;
  Subscribers& subscribers = _eventSubscribers[ev];
  return subscribers.size();
}