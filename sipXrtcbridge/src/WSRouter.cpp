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

#include <boost/algorithm/string.hpp>
#include <os/OsLogger.h>
#include <resip/stack/ExtensionParameter.hxx>
#include "WSRouter.h"


using namespace sipx;
using namespace sipx::proxy;
using namespace sipx::bridge;

#define SWITCH_APPLICATION_NAME "WebRtcBridge"
#define DEFAULT_ESL_ADDRESS "127.0.0.1"
#define DEFAULT_ESL_PORT 2022

//
// Extension URI Parameters
//
static const resip::ExtensionParameter p_xthost("x-thost");
static const resip::ExtensionParameter p_xtport("x-tport");
static const resip::ExtensionParameter p_xtscheme("x-tscheme");
static const resip::ExtensionParameter p_xtrtc("x-trtc");


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
  _tcpUdpPort(0),
  _pSwitch(0),
  _eslPort(DEFAULT_ESL_PORT)
{
  addDaemonOptions();
  addOptionString("ip-address", ": IP Address the wsrouter would bind to.", CommandLineOption, true);
  addOptionStringVector("domain", ": Local domains we are responsible for.  Can be set multiple times", CommandLineOption, true);
  addOptionInt("ws-port", ": WebSocket Port the wsrouter would bind to.", CommandLineOption, true);
  addOptionInt("tcp-udp-port", ": UDP/TCP Port the wsrouter would bind to.", CommandLineOption, true);
  addOptionInt("bridge-tcp-udp-port", ": UDP/TCP Port the rtc bridge would bind to.", CommandLineOption, false);
  addOptionString("proxy-address", ": The address of the Proxy.", CommandLineOption, true);
  addOptionInt("proxy-port", ": The port of the Proxy.", CommandLineOption, false);
  addOptionFlag("enable-library-logging", ": Log library level messages.", CommandLineOption);
  addOptionString("db-path", ": Specify a specific directory where application databases will be saved.", CommandLineOption, false);
  addOptionInt("bridge-esl-port", ": The ESL Port where the bridge will listen for incoming connection from the switch.", CommandLineOption, false);
  addOptionInt("switch-esl-port", ": The ESL Port where the switch will listen for incoming connection from the bridge.", CommandLineOption, false);
}

WSRouter::~WSRouter()
{
  delete _pRepro;
}

bool WSRouter::initialize()
{
  parseOptions();
  
  assert(getOption("ip-address", _address));
  assert(getOption("ws-port", _wsPort));
  assert(getOption("tcp-udp-port", _tcpUdpPort));
  assert(getOption("proxy-address", _proxyAddress));
  getOption("proxy-port", _proxyPort, 0);
  getOption("db-path", _dbPath, SIPX_DBDIR);
  assert(getOption("domain", _domains));
  
  if (hasOption("enable-library-logging", true))
  {
    gEnableReproLogging = true;
  }
  
  assert(!_pRepro);
  _pRepro = new ReproGlue(_process, _dbPath);
  
  std::ostringstream transport;
  transport << _address << ":" << _tcpUdpPort;
  _pRepro->addTransport("UDP", transport.str(), true);
  _pRepro->addTransport("TCP", transport.str(), true);
  
  std::ostringstream wstransport;
  wstransport << _address << ":" << _wsPort;
  _pRepro->addTransport("WS", wstransport.str(), true);
  
  //
  // Add our IP address as a valid domain
  //
  _domains.push_back(_address);

  for (std::vector<std::string>::const_iterator iter = _domains.begin(); iter != _domains.end(); iter++)
    _pRepro->addDomain(*iter, 5061);
  

 
  //
  // Prepare repro instance
  //
  _pRepro->setStaticRouteHandler(boost::bind(&WSRouter::onProcessRequest, this, _1, _2));
  _pRepro->setResponseHandler(boost::bind(&WSRouter::onProcessResponse, this, _1, _2));
  _pRepro->setLogCallBack(boost::bind(log_callback, _1, _2));
  _pRepro->setLogLevel(ReproGlue::ReproLogger::Debug);
  _pRepro->setProxyConfigValue("DisableRegistrar", "true");
  _pRepro->setProxyConfigValue("DisableAuth", "true");
  _pRepro->setProxyConfigValue("EnableCertServer", "false");
  
  //
  // Initialize the switch
  //
  _pSwitch = FreeSwitchRunner::instance();
  if (!_pSwitch)
    return false;
  
  _pSwitch->setSipAddress(_address);
  
  if (hasOption("bridge-tcp-udp-port", true))
  {
    int bridgePort = 0;
    if (getOption("bridge-tcp-udp-port", bridgePort) && bridgePort)
    {
      _pSwitch->setSipPort(bridgePort);
    }
  }
  
  return true;
}
  
int WSRouter::main()
{
  if (initialize())
  {
    bool noconsole = hasOption("daemonize", true);

    //
    // run the repro instance
    //
    
    _pRepro->run();
    
    //
    // Run the freeswitch instance
    //
    //
    // Run the ESL Event Layer
    //
    if (!_eventListener.listenForEvents(boost::bind(&WSRouter::handleBridgeEvent, this, _1, _2), DEFAULT_ESL_ADDRESS, _eslPort))
      return -1;
    _eventListener.run();

    //
    // Run the switch
    //
    _pSwitch->setApplicationName(SWITCH_APPLICATION_NAME);
    _pSwitch->setEventSocketLayerPort(_eslPort);

    _pSwitch->initialize();
    _pSwitch->run(noconsole);
    
    
    //
    // If switch did not start the console, wait for a termination request
    //
    if (noconsole)
      waitForTerminationRequest();

    OS_LOG_INFO(FAC_SIP, "WSRouter::main() process TERMINATED");

    return 0;
  }
  
  return -1;
}

static void addWsContactParams(resip::SipMessage& msg)
{
  if (msg.getSource().getType() == resip::WS)
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
  }
}

ReproGlue::RequestProcessor::ChainReaction WSRouter::onProcessRequest(ReproGlue& repro, RequestContext& context)
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
  
  //
  // Check if request-uri contains the ws parameters
  //
  bool isRtcTarget = ruri.exists(resip::p_wsSrcIp) && ruri.exists(resip::p_wsSrcPort); 
  bool isRtcOffer = false;
  bool isBridgedRtc = ruri.exists(p_xtrtc); 
  //
  // Check if SDP has RTP/SAVPF format.  This means we are being offered RTC media stream
  //
  if (msg.getContents())
  {
    Data content = msg.getContents()->getBodyData();
    std::string sdp = content.c_str();
    boost::to_upper(sdp);
    isRtcOffer = sdp.find("RTP/SAVPF") != std::string::npos;
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
    
    addWsContactParams(msg);
    
   
    if (msg.method() == resip::REGISTER)
    {
      //
      // all registers go to the proxy
      //
      NameAddr path;
      path.uri().scheme() = "sip";
      path.uri().host() = _address.c_str();
      path.uri().port() = _tcpUdpPort;
      path.uri().param(resip::p_lr);
      msg.header(h_Paths).push_front(path);
      
    }
    else if (msg.method() == resip::INVITE)
    {
      if (isBridgedRtc)
      {
        //
        // This has already been bridged.  Reconstruct the original URI and relay it
        //
        std::string thost;
        std::string tport;
        std::string tscheme;

        if (ruri.exists(p_xthost) && ruri.exists(p_xtport) && ruri.exists(p_xtscheme))
        {
          thost = ruri.param(p_xthost).c_str();
          tport = ruri.param(p_xtport).c_str();
          tscheme = ruri.param(p_xtscheme).c_str();
          
          ruri.remove(p_xthost);
          ruri.remove(p_xtport);
          ruri.remove(p_xtscheme);
          ruri.remove(p_xtrtc);
        }
        
        ruri.host() = thost.c_str();
        ruri.port() = boost::lexical_cast<int>(tport);
        ruri.param(p_transport) = resip::Data("ws");
      }
      else if (isRtcOffer && !isRtcTarget)
      {
        //
        // Offer is webrtc but target is legacy endpoint. bridge it
        //
        
        ruri.param(p_xthost) = ruri.host();
        ruri.param(p_xtport) = resip::Data(boost::lexical_cast<std::string>(ruri.port()).c_str());
        ruri.param(p_xtscheme) = resip::Data("sip");
        
        ruri.host() = _pSwitch->getSipAddress().c_str();
        ruri.port() = _pSwitch->getSipPort();
      }
      else if (!isRtcOffer && isRtcTarget)
      {
        //
        // Offer is legacy terminating to a webrtc endpoint
        //       
        ruri.param(p_xthost) = ruri.host();
        ruri.param(p_xtport) = resip::Data(boost::lexical_cast<std::string>(ruri.port()).c_str());
        ruri.param(p_xtscheme) = resip::Data("ws");
        
        ruri.host() = _pSwitch->getSipAddress().c_str();
        ruri.port() = _pSwitch->getSipPort();
      }
      
      //
      // TODO: 
      // - Do not bridge WS->WS calls
      //
    }
    
    context.getResponseContext().addTarget(resip::NameAddr(ruri));
  }
  else
  {
    //
    // This request is towards an external destination.  
    // Mid-dialog requests and calls towards registered agents also fall in this category
    //
    bool isMidDialog = msg.header(h_To).exists(p_tag);
 
   
    if (isMidDialog)
    {
      //
      // Relay all mid-dialog request
      //
      context.getResponseContext().addTarget(resip::NameAddr(ruri));
      return ReproGlue::RequestProcessor::SkipThisChain;
    }
    else if (!isMidDialog && msg.method() == resip::INVITE)
    {
      if (isRtcOffer && isRtcTarget)
      {
        //
        // Relay it.  This is a pure webrtc call
        //
        context.getResponseContext().addTarget(resip::NameAddr(ruri));
        return ReproGlue::RequestProcessor::SkipThisChain;
      }
      else if (isRtcOffer && !isRtcTarget)
      {
        //
        // Offer is webrtc but target is legacy endpoint. bridge it
        //
        
        ruri.param(p_xthost) = ruri.host();
        ruri.param(p_xtport) = resip::Data(boost::lexical_cast<std::string>(ruri.port()).c_str());
        ruri.param(p_xtscheme) = resip::Data("sip");
        
        ruri.host() = _pSwitch->getSipAddress().c_str();
        ruri.port() = _pSwitch->getSipPort();
      }
      else if (!isRtcOffer && isRtcTarget)
      {
        //
        // Offer is legacy terminating to a webrtc endpoint
        //       
        ruri.param(p_xthost) = ruri.host();
        ruri.param(p_xtport) = resip::Data(boost::lexical_cast<std::string>(ruri.port()).c_str());
        ruri.param(p_xtscheme) = resip::Data("ws");
        
        ruri.host() = _pSwitch->getSipAddress().c_str();
        ruri.port() = _pSwitch->getSipPort();
      }
    }

    //
    // Relay everything else
    //
    context.getResponseContext().addTarget(resip::NameAddr(ruri));
  }
  return ReproGlue::RequestProcessor::SkipThisChain;
}

ReproGlue::RequestProcessor::ChainReaction WSRouter::onProcessResponse(ReproGlue& repro, RequestContext& context)
{
  resip::Message* msg = context.getCurrentEvent();
  if(!msg)
  {
    return ReproGlue::RequestProcessor::Continue;
  }

  resip::SipMessage* sip = dynamic_cast<resip::SipMessage*>(msg);
  if(sip && sip->isResponse())
  {
    addWsContactParams(*sip);
  }
  return ReproGlue::RequestProcessor::Continue;
}


void WSRouter::handleBridgeEvent(const sipx::bridge::EslConnection::Ptr& pConnection, const sipx::bridge::EslEvent::Ptr& pEvent)
{
  //
  // Establish early media
  //
  pConnection->execute("pre_answer");
    
  //
  // Bridge everything to the proxy (for now)
  //
 
  
  std::string variable_sip_req_params = pEvent->getHeader("variable_sip_req_params");
  std::string variable_sip_req_uri = pEvent->getHeader("variable_sip_req_uri");
  std::string variable_sip_full_to = pEvent->getHeader("variable_sip_full_to");
  std::string variable_sip_from_host = pEvent->getHeader("variable_sip_from_host");
    
  std::ostringstream ruri;
    ruri << "sip:" << variable_sip_req_uri;
  if (!variable_sip_req_params.empty())
    ruri << ";" << variable_sip_req_params;
    
  resip::Uri requestUri(ruri.str().c_str());
  
  std::string thost;
  std::string tport;
  std::string tscheme;
  
  if (requestUri.exists(p_xthost) && requestUri.exists(p_xtport) && requestUri.exists(p_xtscheme))
  {
    thost = requestUri.param(p_xthost).c_str();
    tport = requestUri.param(p_xtport).c_str();
    tscheme = requestUri.param(p_xtscheme).c_str();
  }
  
  if (thost.empty() || tport.empty() || tscheme.empty())
  {
    //
    // Do nothing.  This will result to the INVITE getting rejected
    //
    return;
  }
  
  if (tscheme == "ws")
  {
    //
    // Route this back to resiprocate
    //
    requestUri.host() = resip::Data(_address.c_str());
    requestUri.port() = _tcpUdpPort;
    //
    // Tag it as an RTC enabled call
    //
    requestUri.param(p_xtrtc) = resip::Data("yes");
  }
  else
  {
    //
    // Route it as normal SIP
    //
    requestUri.host() = resip::Data(thost.c_str());
    requestUri.port() = boost::lexical_cast<int>(tport);
    
    requestUri.remove(p_xthost);
    requestUri.remove(p_xtport);
    requestUri.remove(p_xtscheme);
  }
   
  std::ostringstream arg;
  if (tscheme == "ws")
    arg << "{media_webrtc=true}";
  
  arg << "{sip_invite_domain=" << variable_sip_from_host << "}";
  arg << "{sip_invite_to_uri=" << variable_sip_full_to << "}";

#if 1
        arg << "{sip_auth_username=" << "2017" << "}";
        arg << "{sip_auth_password=" << "20172010" << "}";
#endif
        
  arg << "sofia/" << SWITCH_APPLICATION_NAME << "/" << requestUri;

  pConnection->execute("set", "hangup_after_bridge=true");
  pConnection->execute("bridge", arg.str().c_str(), 0);
}