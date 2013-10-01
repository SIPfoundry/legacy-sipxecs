/**
 *
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
 * 
 */


#include <os/OsLogger.h>
#include "WSRouter.h"


static bool gEnableReproLogging = false;

void log_callback(int level, const char* message)
{
  if (gEnableReproLogging)
  {
    switch(level)
    {
      case ReproGlue::ReproLogger::Crit:
        OS_LOG_CRITICAL(FAC_SIP, message);
        break;
      case ReproGlue::ReproLogger::Err:
        OS_LOG_ERROR(FAC_SIP, message);
        break;
      case ReproGlue::ReproLogger::Warning:
        OS_LOG_WARNING(FAC_SIP, message);
        break;
      case ReproGlue::ReproLogger::Info:
        OS_LOG_INFO(FAC_SIP, message);
        break;
      default:
        OS_LOG_DEBUG(FAC_SIP, message);
        break;
    }
  }
}

WSRouter::WSRouter(int argc, char** argv, const std::string& daemonName, const std::string& version, const std::string& copyright) :
  OsServiceOptions(argc, argv, daemonName, version, copyright),
  _pRepro(0),
  _process(daemonName),
  _wsPort(0),
  _tcpUdpPort(0)
{
  addDaemonOptions();
  addOptionString("ip-address", ": IP Address the wsrouter would bind to.", CommandLineOption, true);
  addOptionStringVector("domain", ": Local domains we are responsible for.  Can be set multiple times", CommandLineOption, true);
  addOptionInt("ws-port", ": WebSocket Port the wsrouter would bind to.", CommandLineOption, true);
  addOptionInt("tcp-udp-port", ": UDP/TCP Port the wsrouter would bind to.", CommandLineOption, true);
  addOptionString("proxy-address", ": The address of the Proxy.", CommandLineOption, true);
  addOptionInt("proxy-port", ": The port of the Proxy.", CommandLineOption, false);
  addOptionFlag("enable-library-logging", ": Log library level messages.", CommandLineOption);
}

WSRouter::~WSRouter()
{
  delete _pRepro;
}

bool WSRouter::initialize()
{
  parseOptions();
  
  assert(!_pRepro);
  _pRepro = new ReproGlue(_process);
  
  assert(getOption("ip-address", _address));
  assert(getOption("ws-port", _wsPort));
  assert(getOption("tcp-udp-port", _tcpUdpPort));
  assert(getOption("proxy-address", _proxyAddress));
  getOption("proxy-port", _proxyPort, 0);
  assert(getOption("domain", _domains));
  
  if (hasOption("enable-library-logging", true))
  {
    gEnableReproLogging = true;
  }
  
  std::ostringstream transport;
  transport << _address << ":" << _tcpUdpPort;
  _pRepro->addTransport("UDP", transport.str(), true);
  _pRepro->addTransport("TCP", transport.str(), true);
  
  std::ostringstream wstransport;
  wstransport << _address << ":" << _wsPort;
  _pRepro->addTransport("WS", wstransport.str(), true);

  for (std::vector<std::string>::const_iterator iter = _domains.begin(); iter != _domains.end(); iter++)
    _pRepro->addDomain(*iter, 5061);
 
  //
  // Prepare repro instance
  //
  _pRepro->setStaticRouteHandler(boost::bind(&WSRouter::onProcessRequestContext, this, _1, _2));
  _pRepro->setLogCallBack(boost::bind(log_callback, _1, _2));
  _pRepro->setLogLevel(ReproGlue::ReproLogger::Debug);
  _pRepro->setProxyConfigValue("DisableRegistrar", "true");
  _pRepro->setProxyConfigValue("DisableAuth", "true");
  _pRepro->setProxyConfigValue("EnableCertServer", "false");
  
  return true;
}
  
int WSRouter::main()
{
  if (initialize())
  {
    _pRepro->run();
    waitForTerminationRequest();
    OS_LOG_INFO(FAC_SIP, "WSRouter::main() process TERMINATED");
    return 0;
  }
  
  return -1;
}

ReproGlue::RequestProcessor::ChainReaction WSRouter::onProcessRequestContext(ReproGlue& repro, RequestContext& context)
{
  resip::SipMessage& msg = context.getOriginalRequest();
  resip::Uri ruri(msg.header(h_RequestLine).uri());
  
  bool isLocalDomain = false;
  for (std::vector<std::string>::const_iterator iter = _domains.begin(); iter != _domains.end(); iter++)
  {
    if (*iter == ruri.host().c_str())
    {
      isLocalDomain = true;
      break;
    }
  }
  
  if (isLocalDomain)
  {
    //
    // If the request is towards a known domain, relay it to the proxy
    //
    ruri.scheme() = "sip";
    ruri.host() = _proxyAddress.c_str();
    ruri.port() = _proxyPort;
    ruri.remove(resip::p_transport);
    
    if (msg.method() == resip::REGISTER)
    {
      for (NameAddrs::iterator iter = msg.header(h_Contacts).begin(); 
           iter != msg.header(h_Contacts).end(); ++iter)
      {
         if(iter->isWellFormed())
         {
           //
           // Add SRC IP Here
           //
           iter->uri().param(resip::p_wsSrcIp) = resip::Tuple::inet_ntop(msg.getSource());
           iter->uri().param(resip::p_wsSrcPort) = msg.getSource().getPort();
         }
      }
      
      NameAddr path;
      path.uri().scheme() = "sip";
      path.uri().host() = _address.c_str();
      path.uri().port() = _tcpUdpPort;
      path.uri().param(resip::p_lr);
      msg.header(h_Paths).push_front(path);
    }

    context.getResponseContext().addTarget(resip::NameAddr(ruri));
  }
  else
  {
    //
    // This request is towards an external destination.  relay it
    //
    context.getResponseContext().addTarget(resip::NameAddr(ruri));
  }
  return ReproGlue::RequestProcessor::SkipThisChain;
}

