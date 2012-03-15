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


#include "ZMQSocket.h"
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cassert>
#include <stdint.h>

#define ZMQLOCK if (_isThreaded) _criticalSection.lock();
#define ZMQUNLOCK if (_isThreaded) _criticalSection.unlock();

std::set<ZMQSocket::ZMQSocketPtr> ZMQSocket::_socketList;
ZMQSocket::mutex_critic_sec ZMQSocket::_socketListMutex;
void ZMQSocket::closeAllSockets()
{
  _socketListMutex.lock();
  for (std::set<ZMQSocket::ZMQSocketPtr>::iterator iter = ZMQSocket::_socketList.begin();
    iter != ZMQSocket::_socketList.end(); iter++)
  {
    ZMQSocket* pSocket = *iter;
    pSocket->close(0);
  }
  _socketListMutex.unlock();
}


ZMQSocket::ZMQSocket(int type, bool isThreaded) :
  _zmqContext(),
  _socket(*_zmqContext._context, type),
  _isOpen(false),
  _isThreaded(isThreaded),
  _is_POLLIN(false),
  _is_POLLOUT(false)
{
  _socketListMutex.lock();
  _socketList.insert(this);
  _socketListMutex.unlock();
}

ZMQSocket::~ZMQSocket()
{
  _socketListMutex.lock();
  _socketList.erase(this);
  _socketListMutex.unlock();
}

void ZMQSocket::close(int linger)
{
  if (!_isOpen)
    return;

  ZMQLOCK;
  try
  {
    _socket.setsockopt(ZMQ_LINGER, &linger, sizeof (linger));
    _socket.close();
  }
  catch(zmq::error_t& error_)
  {
    //
    // We dont need to handle any exceptions here.  Just absorb.
    //
  }
  _isOpen = false;
  ZMQUNLOCK;
}

bool ZMQSocket::bind(const std::string& bindAddress, ZMQSocket::Error& error)
{
  try
  {
    _socket.bind(bindAddress.c_str());
  }
  catch(zmq::error_t& error_)
  {
    convertError(error_, error);
    return false;
  }
  _bindAddress = bindAddress;
  _isOpen = true;
  return true;
}

bool ZMQSocket::receive(ZMQMessage& message, ZMQSocket::Error& error, int flags)
{
  assert(!message.isConsumed());
  if (message.isConsumed())
  {
    error.num = -1;
    error.what = "Message already consumed by previous call to ZMQSocket::send or ZMQSocket::receive.";
    return false;
  }

  try
  {
    _socket.recv(&(message._message), flags);
    message.initReceivedData();
    message._isConsumed = true;
  }
  catch(zmq::error_t& error_)
  {
    convertError(error_, error);
    return false;
  }

  return true;
}

bool ZMQSocket::send(ZMQMessage& message, ZMQSocket::Error& error, int flags)
{
  assert(!message.isConsumed());
  ZMQLOCK;
  if (message.isConsumed())
  {
    error.num = -1;
    error.what = "Message already consumed by previous call to ZMQSocket::send or ZMQSocket::receive.";
    ZMQUNLOCK;
    return false;
  }

  try
  {
    message.initData();
    std::string buff = std::string(static_cast<char*>(message._message.data()), message._message.size());
    _socket.send(message._message, flags);
    message._isConsumed = true;
  }
  catch(zmq::error_t& error_)
  {
    convertError(error_, error);
    ZMQUNLOCK;
    return false;
  }
  ZMQUNLOCK;
  return true;
}

bool ZMQSocket::connect(const std::string& address, ZMQSocket::Error& error)
{
  try
  {
    _socket.connect(address.c_str());
    _connectAddress.insert(address);
  }
  catch(zmq::error_t& error_)
  {
    convertError(error_, error);
    return false;
  }
  _isOpen = true;
  return true;
}

bool ZMQSocket::hasMore()
{
  ZMQLOCK;
  int64_t more;
  size_t more_size = sizeof (more);
  _socket.getsockopt(ZMQ_RCVMORE, &more, &more_size);
  ZMQUNLOCK;
  return more;
}

std::string ZMQSocket::generateId()
{
  #define within(num) (int) ((float) (num) * random () / (RAND_MAX + 1.0))
  std::stringstream ss;
  ss << std::hex << std::uppercase
        << std::setw(4) << std::setfill('0') << within (0x10000) << "-"
        << std::setw(4) << std::setfill('0') << within (0x10000);
  return ss.str();
}