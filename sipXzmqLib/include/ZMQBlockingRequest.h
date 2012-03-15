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

#ifndef ZMQBLOCKINGREQUEST_H
#define	ZMQBLOCKINGREQUEST_H

#include "ZMQMessage.h"
#include <boost/thread.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>


class ZMQBlockingRequest : public ZMQMessage
{
public:
  typedef boost::shared_ptr<ZMQBlockingRequest> Ptr;
  
  ZMQBlockingRequest();
  ZMQBlockingRequest(const std::string& data);
  ~ZMQBlockingRequest();

  void unblock(ZMQMessage& response);
  bool wait(unsigned int milliseconds = 0);
  ZMQMessage& response();
protected:
  std::string _uuid;
  ZMQMessage _response;

private:
  boost::mutex _mutex;
  boost::condition_variable _cond;
  bool _responseReady;
  friend class ZMQProactorClient;
};

//
// Inlines
//
inline ZMQMessage& ZMQBlockingRequest::response()
{
  return _response;
}

#endif	/* ZMQBLOCKINGREQUEST_H */

