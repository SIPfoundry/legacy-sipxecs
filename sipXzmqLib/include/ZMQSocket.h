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

#ifndef ZMQSOCKET_H_INCLUDED
#define	ZMQSOCKET_H_INCLUDED

#include "ZMQLogger.h"
#include "ZMQMessage.h"
#include "ZMQContext.h"
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include <string>
#include <set>
#include <vector>


class ZMQSocket : boost::noncopyable
{
  //
  // The root of all evil.
  // This class is a wrapper to the ZeroMQ socket.
  // although the implementation documentatin of ZertoMQ
  // claims it is lock free, that claim is only good if
  // you use ZeroMQ in a none threaded environment.
  // We will safely adhere to that by performing all
  // read operations from a single thread while makeing
  // sure that write operations are guarded by a critical section
  // to allow any thread to use the socket.
  //
public:
  struct Error
  {
    std::string what;
    int num;
  };
  typedef boost::recursive_mutex mutex;
  typedef boost::lock_guard<mutex> mutex_lock;
  typedef boost::mutex mutex_critic_sec;
  typedef boost::lock_guard<mutex_critic_sec> mutex_critic_sec_lock;
  typedef boost::shared_mutex mutex_read_write;
  typedef boost::shared_lock<boost::shared_mutex> mutex_read_lock;
  typedef boost::lock_guard<boost::shared_mutex> mutex_write_lock;
  typedef std::set<std::string> ConnectList;
  ZMQSocket(int type, bool isThreaded = false);
  ~ZMQSocket();
  bool bind(const std::string& bindAddress, Error& error);
  bool receive(ZMQMessage& message, ZMQSocket::Error& error, int flags = 0);
  bool send(ZMQMessage& message, ZMQSocket::Error& error, int flags = 0);
  bool sendMore(ZMQMessage& message, ZMQSocket::Error& error);
  void close(int linger);
  bool connect(const std::string& address, ZMQSocket::Error& error);
  bool hasMore();
  bool is_POLLIN();
  bool is_POLLOUT();
  const std::string& getBindAddress() const;
  ConnectList& getConnectList();
  zmq::socket_t& zmqSocket();
  static void initContext(int io_threads_=1);
  static void destroyContext();
  static std::string generateId();
protected:
  void convertError(zmq::error_t& error_, ZMQSocket::Error& error);
  ZMQContext _zmqContext;
  zmq::socket_t _socket;
  bool _isOpen;
  std::string _bindAddress;
  std::set<std::string> _connectAddress;
  bool _isThreaded;
  bool _is_POLLIN;
  bool _is_POLLOUT;
private:
  mutex_critic_sec _criticalSection;

public:
  typedef ZMQSocket* ZMQSocketPtr;
  static std::set<ZMQSocketPtr> _socketList;
  static mutex_critic_sec _socketListMutex;
  static void closeAllSockets();

  friend class ZMQPoll;
  friend class ZMQPoll2;
  friend class ZMQPoll3;
  friend class ZMQPoll4;
  friend class ZMQPoll5;
};


//
// Inlines
//

inline void ZMQSocket::convertError(zmq::error_t& error_, ZMQSocket::Error& error)
{
  error.what = error_.what();
  error.num = error.num;
}

inline bool ZMQSocket::sendMore(ZMQMessage& message, ZMQSocket::Error& error)
{
  return send(message, error, ZMQ_SNDMORE);
}

inline zmq::socket_t& ZMQSocket::zmqSocket()
{
  return _socket;
}

inline const std::string& ZMQSocket::getBindAddress() const
{
  return _bindAddress;
}

inline ZMQSocket::ConnectList& ZMQSocket::getConnectList()
{
  return _connectAddress;
}

inline bool ZMQSocket::is_POLLIN()
{
  return _is_POLLIN;
}

inline bool ZMQSocket::is_POLLOUT()
{
  return _is_POLLOUT;
}

#endif	/* ZMQSOCKET_H_INLCUDED */

