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

#include <boost/lexical_cast.hpp>

#include "CallThread.h"
#include "CallManager.h"

#define SIP_PROFILE "callgen"

CallThread::CallThread(CallManager& manager, int id) :
  _manager(manager),
  _id(id),
  _pThread(0),
  _pEsl(0)
{
}

CallThread::~CallThread()
{
  if (_pEsl)
  {
    _pEsl->disconnect();
  }

  if (_pThread)
  {
    _pThread->join();
    delete _pThread;
  }

  if (_pEsl)
  {
    delete _pEsl;
  }


  std::cout << "CallThread " << _id << " DESTROYED" << std::endl;
}

void CallThread::run()
{
  _pThread = new boost::thread(boost::bind(&CallThread::internalRun, this));
}

void CallThread::internalRun()
{
  if (!eslConnect())
    _manager.collect(_id);

  makeCall();
  _manager.collect(_id);
}

bool CallThread::eslConnect()
{
  std::cout << "Connecting to " << _manager.getEslHost() << ":" << _manager.getEslPort() << std::endl;
  _pEsl = new EslConnection(_manager.getEslHost().c_str(), 
    boost::lexical_cast<std::string>(_manager.getEslPort()).c_str(),
    _manager.getEslPass().c_str());
  return _pEsl->connected();
}

bool CallThread::makeCall()
{
  _setupTime = boost::posix_time::microsec_clock::local_time();

  if (!_pEsl || !_pEsl->connected())
    return false;

    //("sip-uri", opt::value<std::string>(), "Target Uri")
  // ("sip-domain", opt::value<std::string>(), "Local domain")
  // ("sip-user", opt::value<std::string>(), "SIP user")
  // ("sip-pass", opt::value<std::string>(), "SIP password")

  _manager.getOption("sip-uri", _targetUri);
  _manager.getOption("sip-domain", _domain);
  _manager.getOption("sip-user", _authUser);
  _manager.getOption("sip-pass", _authPass);

  std::ostringstream arg;
  if (!_targetUri.empty())
    arg << "{sip_invite_to_uri=" << _targetUri << "}";

  if (!_domain.empty())
    arg << "{sip_invite_domain=" << _domain << "}";

  if (!_authUser.empty() && !_authPass.empty())
  {
    arg << "{sip_auth_username=" << _authUser << "}";
    arg << "{sip_auth_password=" << _authPass << "}";
  }

  std::ostringstream uuid;
  uuid << "callgen-" << _manager.getTime() << "-" << _id;
  arg <<"{origination_uuid=" << uuid.str() << "}";
  
  // arg << "{media_webrtc=true}";

  arg << "sofia/" << SIP_PROFILE << "/" << _targetUri;

  arg << " &park()";

  EslEvent::Ptr pEvent = _pEsl->api("originate", arg.str().c_str()/*, uuid.str().c_str()*/);

  if (!pEvent)
  {
    return false;
  }

  std::string reply = pEvent->getHeader("Reply-Text");
  if (reply.find("-ERR") != std::string::npos)
  {

    std::cout << "REPLY" << reply << std::endl;
    return false;
  }

  std::string body = pEvent->getBody();
  if (body.find("-ERR") != std::string::npos)
  {

    std::cout << "REPLY" << body << std::endl;
    return false;
  }

  _pEsl->api("uuid_kill", uuid.str().c_str());

  _disconnectTime = boost::posix_time::microsec_clock::local_time();

  return true;
}

