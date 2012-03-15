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

#ifndef ZMQCONTEXT_H
#define	ZMQCONTEXT_H

#include "zmq.hpp"
#include <boost/thread.hpp>

class ZMQContext
{
protected:
  typedef boost::mutex mutex_critic_sec;
  typedef boost::lock_guard<mutex_critic_sec> mutex_critic_sec_lock;
  ZMQContext();
  ~ZMQContext();
protected:
  static mutex_critic_sec _scriticalSection;
  static int _refCount;
  static zmq::context_t* _context;
  friend class ZMQSocket;
};



#endif	/* ZMQCONTEXT_H */

