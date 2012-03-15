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


#include "ZMQContext.h"
#include <cassert>

static const int ZMQ_IO_THREAD_COUNT = 1;
zmq::context_t* ZMQContext::_context = 0;
int ZMQContext::_refCount = 0;
ZMQContext::mutex_critic_sec ZMQContext::_scriticalSection;

ZMQContext::ZMQContext()
{
  ZMQContext::_scriticalSection.lock();
  if (!ZMQContext::_refCount)
  {
    assert(!ZMQContext::_context);
    ZMQContext::_context = new zmq::context_t(ZMQ_IO_THREAD_COUNT);
  }
  ++ZMQContext::_refCount;
  ZMQContext::_scriticalSection.unlock();
}

ZMQContext::~ZMQContext()
{
  ZMQContext::_scriticalSection.lock();
  --ZMQContext::_refCount;
  if (!ZMQContext::_refCount)
  {
    assert(ZMQContext::_context);
    delete ZMQContext::_context;
  }
  ZMQContext::_scriticalSection.unlock();
}