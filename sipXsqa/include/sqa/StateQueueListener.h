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


#ifndef STATEQUEUELISTENER_H
#define	STATEQUEUELISTENER_H

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include "StateQueueConnection.h"

class StateQueueAgent;

class StateQueueListener : boost::noncopyable
{
public:
  typedef boost::recursive_mutex mutex;
  typedef boost::lock_guard<mutex> mutex_lock;
  StateQueueListener(StateQueueAgent* agent);
  ~StateQueueListener();
  void run();
  void handleAccept(const boost::system::error_code& e);
  
  void addConnection(StateQueueConnection::Ptr conn);
  void destroyConnection(StateQueueConnection::Ptr conn);


protected:
  StateQueueAgent* _pAgent;
  boost::asio::ip::tcp::acceptor _acceptor;
  boost::asio::ip::tcp::resolver _resolver;
  StateQueueConnection::Ptr _pNewConnection;
  mutex _mutex;
  std::map<StateQueueConnection*, StateQueueConnection::Ptr> _connections;
};


#endif	/* STATEQUEUELISTENER_H */

