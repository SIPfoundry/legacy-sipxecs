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

#ifndef ZMQREACTOR_H
#define	ZMQREACTOR_H

#include "ZMQNotifier.h"

class ZMQReactor : protected ZMQNotifier
{
  //
  // This is a class that implements the reactor design pattern.
  // It receives tasks from a service broker and distribute them
  // accross registered actors
  //
public:
  ZMQReactor(unsigned int highWaterMark = 10000);
  ~ZMQReactor();
  bool startReactor(const std::string& localAddress, const std::string& brokerFrontEndAddress, const std::string& brokerBackEndAddress, ZMQSocket::Error& error);
  void stopReactor();
  void terminateActors();
protected:
  void readServiceRequests();
  ZMQSocket _brokerSocket;
  boost::thread* _pReadThread;
  bool _isReading;
  std::string _exitNotifier;
  std::string _brokerFrontEndAddress;
  std::string _brokerBackEndAddress;
};

//
// Inlines
//

inline void ZMQReactor::terminateActors()
{
  terminateSubscriptions();
}

#endif	/* ZMQREACTOR_H */

