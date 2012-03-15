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

#ifndef ZMQPIPELINEBROKER_H
#define	ZMQPIPELINEBROKER_H

#include <boost/thread.hpp>
#include "ZMQSocket.h"


class ZMQPipeLineBroker
{
public:
  //
  // The pipeline pattern is used for distributing data to nodes arranged in a
  // pipeline. Data always flows down the pipeline, and each stage of the
  // pipeline is connected to at least one node. When a pipeline stage is
  // connected to multiple nodes data is load-balanced among all connected nodes.
  //
  ZMQPipeLineBroker();
  ~ZMQPipeLineBroker();
  bool startServingMessages(
    const std::string& frontEndAddress,
    const std::string& backEndAddress,
    ZMQSocket::Error& error);
  void stopServingMessages();
protected:
  void serviceLoop();
  boost::thread* _pReadThread;
  bool _isReading;
  ZMQSocket _frontEnd;
  ZMQSocket _backEnd;
  ZMQSocket _controller;
  std::string _frontEndAddress;
  std::string _backEndAddress;
};

#endif	/* ZMQPIPELINEBROKER_H */

