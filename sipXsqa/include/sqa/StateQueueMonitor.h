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


#ifndef STATEQUEUEMONITOR_H
#define	STATEQUEUEMONITOR_H


#include <boost/thread.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include "sqa/sqaclient.h"


class StateQueueMonitor
{
public:
  typedef boost::function<void(const std::string&/*appId*/, const std::string&/*address*/)> Handler;
  StateQueueMonitor();
  ~StateQueueMonitor();
  void handleEvent(const std::string& id, const std::string& eventData);
  void run();
  void stop();

  void setConnectHandler(Handler handler);
  void setSigninHandler(Handler handler);
  void setLogoutHandler(Handler handler);
  void setTerminateHandler(Handler handler);
  void setKeepAliveHandler(Handler handler);

  const char* getLocalAddress();
private:
  void internal_run();
  void handleConnect(const std::string& appId, const std::string& address);
  void handleSignin(const std::string& appId, const std::string& address);
  void handleLogout(const std::string& appId, const std::string& address);
  void handleTerminate(const std::string& appId, const std::string& address);
  void handleKeepAlive(const std::string& appId, const std::string& address);
  SQAWatcher _instanceWatcher;
  boost::thread* _pInstanceWatcherThread;
  bool _isTerminated;
  Handler _connectHandler;
  Handler _signinHandler;
  Handler _logoutHandler;
  Handler _terminateHandler;
  Handler _keepaliveHandler;
};

//
// Inlines
//


inline StateQueueMonitor::StateQueueMonitor() :
  _instanceWatcher("StateQueueMonitor", "connection.", 1, 1000, 1000),
  _pInstanceWatcherThread(0),
  _isTerminated(true)
{
}

inline StateQueueMonitor::~StateQueueMonitor()
{
  if (_pInstanceWatcherThread)
  {
    stop();
  }
}

inline const char* StateQueueMonitor::getLocalAddress()
{
  return _instanceWatcher.getLocalAddress();
}

inline void StateQueueMonitor::run()
{
  if (!_isTerminated || _pInstanceWatcherThread)
    return;
  _pInstanceWatcherThread = new boost::thread(boost::bind(&StateQueueMonitor::internal_run, this));
}

inline void StateQueueMonitor::stop()
{
  _isTerminated = true;
  _instanceWatcher.terminate();
  if (_pInstanceWatcherThread)
  {
    _pInstanceWatcherThread->join();
    delete _pInstanceWatcherThread;
    _pInstanceWatcherThread = 0;
  }
}

inline void StateQueueMonitor::internal_run()
{
  _isTerminated = false;
  while(!_isTerminated)
  {
    SQAEvent* pEvent = _instanceWatcher.watch();
    if (!pEvent)
      break;
    if (strcmp(pEvent->id, SQA_TERMINATE_STRING) == 0)
    {
      delete pEvent;
      break;
    }
    handleEvent(pEvent->id, pEvent->data);
    delete pEvent;
  }
}

inline void StateQueueMonitor::setConnectHandler(Handler handler)
{
  _connectHandler = handler;
}

inline void StateQueueMonitor::setSigninHandler(Handler handler)
{
  _signinHandler = handler;
}

inline void StateQueueMonitor::setLogoutHandler(Handler handler)
{
  _logoutHandler = handler;
}

inline void StateQueueMonitor::setTerminateHandler(Handler handler)
{
  _terminateHandler = handler;
}

inline void StateQueueMonitor::setKeepAliveHandler(Handler handler)
{
  _keepaliveHandler = handler;
}

inline void StateQueueMonitor::handleEvent(const std::string& id, const std::string& eventData)
{
  std::vector<std::string> tokens;
  boost::split(tokens, eventData, boost::is_any_of("|"), boost::token_compress_on);
  if (tokens.size() == 2)
  {
    std::string appId = tokens[0];
    std::string address = tokens[1];

    if (id.find("connection.established") != std::string::npos)
      handleConnect(appId, address);
    else if (id.find("connection.signin") != std::string::npos)
      handleSignin(appId, address);
    else if (id.find("connection.logout") != std::string::npos)
      handleLogout(appId, address);
    else if (id.find("connection.terminated") != std::string::npos)
      handleTerminate(appId, address);
    else if (id.find("connection.keepalive") != std::string::npos)
      handleKeepAlive(appId, address);
  }
}

inline void StateQueueMonitor::handleConnect(const std::string& appId, const std::string& address)
{
  if (_connectHandler)
    _connectHandler(appId, address);
}

inline void StateQueueMonitor::handleSignin(const std::string& appId, const std::string& address)
{
  if (_signinHandler)
    _signinHandler(appId, address);
}

inline void StateQueueMonitor::handleLogout(const std::string& appId, const std::string& address)
{
  if (_logoutHandler)
    _logoutHandler(appId, address);
}

inline void StateQueueMonitor::handleTerminate(const std::string& appId, const std::string& address)
{
  if (_terminateHandler)
    _logoutHandler(appId, address);
}

inline void StateQueueMonitor::handleKeepAlive(const std::string& appId, const std::string& address)
{
  if (_keepaliveHandler)
    _keepaliveHandler(appId, address);
}

#endif	/* STATEQUEUEMONITOR_H */

