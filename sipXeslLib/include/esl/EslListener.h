/*
 * Copyright (c) eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */


#ifndef ESLLISTENER_H_INCLUDED
#define	ESLLISTENER_H_INCLUDED

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <string>
#include <esl/EslConnection.h>



class EslListener 
{
public:
  
  typedef boost::function<void(const EslConnection::Ptr&, const EslEvent::Ptr&)> ConnectionHandler;
  
  EslListener();
  
  ~EslListener();
 
  bool listenForEvents(ConnectionHandler eventHandler, unsigned short port);
  
  bool listenForEvents(ConnectionHandler eventHandler, const std::string& address, unsigned short port);
  
  void run();
  
  void stop();
  
protected:

  void runEventLoop();
  
  std::string _eventListenerAddress;
  unsigned short _eventListenerPort;
  EslConnection::Ptr _eventOutbound;
  ConnectionHandler _eventHandler;
  int _eventServerSocket;
  boost::thread* _pEventThread;
};


//
// Inlines
//

inline bool EslListener::listenForEvents(ConnectionHandler eventHandler, unsigned short port)
{
  std::string address = "127.0.0.1";
  return listenForEvents(eventHandler, address, port);
}


#endif	

