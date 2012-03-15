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


#include "ZMQReactorClient.h"

ZMQReactorClient::ZMQReactorClient() : ZMQSocket(ZMQ_PUSH, true)
{
}

ZMQReactorClient::~ZMQReactorClient()
{
}

bool ZMQReactorClient::postEvent(ZMQMessage& message, ZMQSocket::Error& error)
{
  send(message, error);
  //
  // Just consume the response.  A reactor really doesnt care about responses.
  // Only a proactor cares
  //
  return true;
}

void ZMQReactorClient::terminateReactor()
{
  ZMQSocket::Error error;
  ZMQMessage terminateReactor("ZMQReactor::terminate ZMQReactorClient ZMQEvent");
  postEvent(terminateReactor, error);
}