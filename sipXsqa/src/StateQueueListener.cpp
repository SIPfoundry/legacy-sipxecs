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

#include "sqa/StateQueueAgent.h"
#include "sqa/StateQueueListener.h"
#include "os/OsLogger.h"

StateQueueListener::StateQueueListener(StateQueueAgent* agent) :
  _pAgent(agent),
  _acceptor(_pAgent->_ioService),
  _resolver(_pAgent->_ioService)
{
  OS_LOG_DEBUG(FAC_NET, "StateQueueListener CREATED");
}

StateQueueListener::~StateQueueListener()
{
  OS_LOG_DEBUG(FAC_NET, "StateQueueListener DESTROYED");
}

void StateQueueListener::run()
{
  _pNewConnection = StateQueueConnection::Ptr(new StateQueueConnection(_pAgent->_ioService, *_pAgent));
  std::string address;
  std::string port;
  _pAgent->options().getOption("sqa-control-address", address);
  _pAgent->options().getOption("sqa-control-port", port);

  boost::asio::ip::tcp::resolver::query query(address, port);
  boost::asio::ip::tcp::endpoint endpoint = *_resolver.resolve(query);
  _acceptor.open(endpoint.protocol());
  _acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
  _acceptor.bind(endpoint);
  _acceptor.listen();
  _acceptor.async_accept(dynamic_cast<StateQueueConnection*>(_pNewConnection.get())->socket(),
      boost::bind(&StateQueueListener::handleAccept, this,
        boost::asio::placeholders::error));

  OS_LOG_INFO(FAC_NET, "StateQueueListener::run "
    << " started accepting connections at bind address tcp://" << address << ":" << port);
}

void StateQueueListener::handleAccept(const boost::system::error_code& e)
{
  addConnection(_pNewConnection);
  _pAgent->onIncomingConnection(_pNewConnection);
  if (_acceptor.is_open())
  {
    _pNewConnection.reset(new StateQueueConnection(_pAgent->_ioService, *_pAgent));
    _acceptor.async_accept(dynamic_cast<StateQueueConnection*>(_pNewConnection.get())->socket(),
      boost::bind(&StateQueueListener::handleAccept, this,
        boost::asio::placeholders::error));
  }
}


void StateQueueListener::addConnection(StateQueueConnection::Ptr conn)
{
  mutex_lock lock(_mutex);
  _connections[conn.get()] = conn;
  OS_LOG_INFO(FAC_NET, "StateQueueListener::addConnection "
    << " connection accepted "  << conn.get());
  conn->start();
}

void StateQueueListener::destroyConnection(StateQueueConnection::Ptr conn)
{
  mutex_lock lock(_mutex);
  _connections.erase(conn.get());
  OS_LOG_INFO(FAC_NET, "StateQueueListener::destroyConnection "
    << " connection removed - " << conn.get());
  conn->stop();
}