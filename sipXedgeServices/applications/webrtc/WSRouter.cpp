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
#include <boost/filesystem.hpp>
#include <os/OsLogger.h>
#include <resip/stack/ExtensionParameter.hxx>
#include <resip/stack/ExtensionHeader.hxx>
#include <resip/stack/Helper.hxx>
#include <resip/stack/ParameterTypes.hxx>
#include <sipXecsService/SipXApplication.h>

#include "WSRouter.h"
#include "AuthInformationGrabber.h"
#include "DomainConfig.h"
#include "AuthIdentityEncoder.h"


#define SWITCH_APPLICATION_NAME "WebRtcBridge"
#define DEFAULT_ESL_ADDRESS "127.0.0.1"
#define DEFAULT_ESL_PORT 11000

//
// Extension URI Parameters
//
static const resip::ExtensionParameter p_xthost("x-thost");
static const resip::ExtensionParameter p_xtport("x-tport");
static const resip::ExtensionParameter p_xtscheme("x-tscheme");
static const resip::ExtensionParameter p_xtrtc("x-trtc");
static const resip::ExtensionParameter p_xtauthid("x-tauthid");


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

static bool verify_directory(const std::string path)
{
  if (!boost::filesystem::exists(path.c_str()))
  {
    if (!boost::filesystem::create_directories(path.c_str()))
    {
      std::cerr << "Unable to create data directory " << path.c_str();
      return false;
    }
  }
  
  return true;
}

template <size_t size>
bool string_sprintf_string(std::string& str, const char * format, ...)
  /// Write printf() style formatted data to string.
  ///
  /// The size template argument specifies the capacity of
  /// the internal buffer that will store the string result.
  /// Take note that if the buffer size is not enough
  /// the result of vsprintf will result to an overrun
  /// and will corrupt memory.
{
    char buffer[size];
    va_list args;
    va_start(args, format);
    int ret = vsprintf(buffer,format, args);
    va_end (args);
    if (ret >= 0)
    {
      str = buffer;
      return true;
    }
    return false;
}

static void escape_url_parameter(std::string& result, const char* _str, const char* validChars)
{
  static const char * safeChars = "abcdefghijklmnopqrstuvwxyz"
                      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                      "0123456789$-_.!*'(),+#";

  int pos = -1;
  char* offSet = const_cast<char*>(_str);
  char* str = const_cast<char*>(_str);
  int len = strlen(str);

  std::string front;
  while ((pos += (int)(1+strspn(&str[pos+1], validChars == 0 ? safeChars : validChars))) < len)
  {
    std::string escaped;
    if (!string_sprintf_string<4>(escaped, "%%%02X", static_cast<const unsigned char>(str[pos])))
    {
      front = str;
      return;
    }
    front += std::string(offSet, str + pos );
    front += escaped;
    offSet = const_cast<char*>(str) + pos + 1;
  }
  front += std::string(offSet, str + pos );
  result = front;
}

WSRouter::WSRouter(int argc, char** argv, const std::string& daemonName) :
  OsServiceOptions(argc, argv),
  _pRepro(0),
  _process(daemonName),
  _wsPort(0),
  _tcpUdpPort(0),
  _eslPort(DEFAULT_ESL_PORT),
  _pRpc(0)
{
  setCommandLine(argc, argv);
  addDefaultOptions();
  addOptionString("ip-address", ": IP Address the wsrouter would bind to.", CommandLineOption, true);
  addOptionString("realm", ": The realm used to authenticate SIP requests.", CommandLineOption, false);
  addOptionStringVector("domain", ": Local domains we are responsible for.  Can be set multiple times", CommandLineOption, false);
  addOptionInt("ws-port", ": WebSocket Port the wsrouter would bind to.", CommandLineOption, true);
  addOptionInt("tcp-udp-port", ": UDP/TCP Port the wsrouter would bind to.", CommandLineOption, true);
  addOptionInt("bridge-tcp-udp-port", ": UDP/TCP Port the rtc bridge would bind to.", CommandLineOption, false);
  addOptionString("proxy-address", ": The address of the Proxy.", CommandLineOption, false);
  addOptionString("rpc-url", ": The JSON-RPC URL for remote procedure calls.", CommandLineOption, false);
  addOptionInt("proxy-port", ": The port of the Proxy.", CommandLineOption, false);
  addOptionFlag("enable-library-logging", ": Log library level messages.", CommandLineOption);
  addOptionString("user-cache", ": Static user authentication cache file.", CommandLineOption, false);
  addOptionString("db-path", ": Specify a specific directory where application databases will be saved.", CommandLineOption, false);
  addOptionInt("bridge-esl-port", ": The ESL Port where the bridge will listen for incoming connection from the switch.", CommandLineOption, false);
  addOptionInt("switch-esl-port", ": The ESL Port where the switch will listen for incoming connection from the bridge.", CommandLineOption, false);
}

WSRouter::~WSRouter()
{
  delete _pRepro;
  delete _pRpc;
  DomainConfig::delete_instance();
}

bool WSRouter::initialize()
{
  OS_LOG_INFO(FAC_SIP, "WSRouter::initialize INVOKED")
  assert(getOption("ip-address", _address));
  assert(getOption("ws-port", _wsPort));
  assert(getOption("tcp-udp-port", _tcpUdpPort));
  
  getOption("db-path", _dbPath, SIPX_DBDIR);
  if (!verify_directory(_dbPath))
    return false;
  
  getOption("proxy-address", _proxyAddress);
  getOption("proxy-port", _proxyPort, 0);
  getOption("domain", _domains);
  getOption("realm", _realm);
  
  if (_domains.empty())
  {
    std::vector<std::string> aliases = DomainConfig::instance()->getAliases();
    std::string domain = DomainConfig::instance()->getDomainName();
    
    if (!domain.empty())
      _domains.push_back(domain);
    
    for (std::vector<std::string>::const_iterator iter = aliases.begin(); iter != aliases.end(); iter++)
      _domains.push_back(*iter);
    
  }

  if (_realm.empty())
    _realm = DomainConfig::instance()->getRealm();
  
  if (_domains.empty() || _realm.empty())
  {
    std::cerr << "Unable to determine domain/realm configuration." << std::endl;
    exit(-1);
  }
  
  if (_proxyAddress.empty())
    _proxyAddress = DomainConfig::instance()->getDomainName();
  
  if (_proxyAddress.empty())
  {
    std::cerr << "Unable to determine proxy address configuration." << std::endl;
    exit(-1);
  }
  
  if (hasOption("enable-library-logging"))
  {
    gEnableReproLogging = true;
  }
  
  if (!_pRpc && getOption("rpc-url", _rpcUrl))
  {
    OS_LOG_INFO(FAC_SIP, "WSRouter::initialize - Setting up RPC service to " << _rpcUrl);
    _pRpc = new jsonrpc::Client(new jsonrpc::HttpClient(_rpcUrl));
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
  _pRepro->setDigestAuthenticatorHandler(boost::bind(&WSRouter::onDigestAuthenticate, this, _1, _2));
  _pRepro->setLogCallBack(boost::bind(log_callback, _1, _2));
  _pRepro->setLogLevel(ReproGlue::ReproLogger::Debug);
  _pRepro->setProxyConfigValue("DisableRegistrar", "true");
  _pRepro->setProxyConfigValue("EnableCertServer", "false");
  
  
  //
  // Initialize authentication
  //
  std::string userCache;
  getOption("user-cache", userCache);

  if (!userCache.empty())
  {
    OS_LOG_INFO(FAC_SIP, "Setting Cache File:  " << userCache);
  }
  else
  {
    OS_LOG_INFO(FAC_SIP, "User Cache not set");
  }

  _pRepro->setProxyConfigValue("DisableAuth", "false");
  _pRepro->setProxyConfigValue("DisableAuthInt", "true");
  _pRepro->setExternalAuthGrabber(new AuthInformationGrabber(this, 0, _pRpc, userCache.empty() ? 0 : userCache.c_str()));
  
  
  if (hasOption("bridge-tcp-udp-port"))
  {
    _bridgePort = 0;
    getOption("bridge-tcp-udp-port", _bridgePort);
  }
  
  return true;
}
  
int WSRouter::main()
{

    //
    // run the repro instance
    //
    
    _pRepro->run();
    
 
    //
    // Run the ESL Event Layer
    //
    getOption("bridge-esl-port", _eslPort, DEFAULT_ESL_PORT);

    if (!_eventListener.listenForEvents(boost::bind(&WSRouter::handleBridgeEvent, this, _1, _2), DEFAULT_ESL_ADDRESS, _eslPort))
      return -1;
    _eventListener.run();  

    SipXApplication::instance().waitForTerminationRequest(1);
    
    OS_LOG_INFO(FAC_SIP, "WSRouter::main() process TERMINATED");

    return 0;
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

void WSRouter::routeToBridge(resip::SipMessage& msg, resip::Uri& ruri, bool isRtcTarget)
{
  ruri.param(p_xthost) = ruri.host();
  ruri.param(p_xtport) = resip::Data(boost::lexical_cast<std::string>(ruri.port()).c_str());
    
  if (isRtcTarget)
  {
    ruri.param(p_xtscheme) = resip::Data("ws");
    ruri.param(p_xtrtc) = resip::Data("yes");
  }
  else
  {
    ruri.param(p_xtscheme) = resip::Data("sip");
  }
  
  ruri.remove(resip::p_transport);
       
  ruri.host() = _address.c_str();
  ruri.port() = _bridgePort;
  
  msg.setForceTarget(ruri);
}

bool WSRouter::isMyDomain(const char* domain)
{
  for(std::vector<std::string>::const_iterator iter = _domains.begin(); iter != _domains.end(); iter++)
  {
    if (domain == *iter)
      return true;
  }
  return false;
}

void WSRouter::insertAuthenticator(resip::SipMessage& request, resip::Uri& requestUri)
{
  if (requestUri.exists(p_xtauthid))
    return;
  
  std::string identity;
  std::string callId;
  std::string fromTag;
  std::string authority;
  std::string realm;
  std::string user;
   
  if (!request.exists(resip::h_ProxyAuthorizations))
    return;
  
  if (!request.exists(resip::h_From))
    return;
  
  if (!request.exists(resip::h_CallID))
    return;

  if (!request.header(resip::h_From).exists(resip::p_tag))
    return;
            
 
  fromTag = request.header(resip::h_From).param(resip::p_tag).c_str();
  callId = request.header(resip::h_CallID).value().c_str();
  
  resip::Auths &authHeaders = request.header(resip::h_ProxyAuthorizations); 
  
  for (resip::Auths::iterator iter = authHeaders.begin() ; iter != authHeaders.end() ; iter++)
  {
    if (iter->exists(resip::p_realm))
    {
      if (isMyDomain(iter->param(resip::p_realm).c_str()))
      {
        realm = iter->param(resip::p_realm).c_str();
        if (iter->exists(resip::p_username))
        {
          user = iter->param(resip::p_username).c_str();
          std::ostringstream id;
          id << user << "@" << realm;
          identity = id.str();
          if (AuthIdentityEncoder::encodeAuthority(identity, callId, fromTag, authority))
          {
            std::string authParam;
            escape_url_parameter(authParam, authority.c_str(), 0);
            requestUri.param(p_xtauthid) = resip::Data(authParam.c_str());
            
            return;
          }
        }
      }
    }
  }
}

bool WSRouter::isRTCTarget(const resip::Uri& ruri)
{
  if (ruri.exists(resip::p_transport) && (ruri.param(resip::p_transport) == "WS" || ruri.param(resip::p_transport) == "ws"))
    return true;
  
  if (_pRpc)
  {
    try
    {
      Json::Value result;
      Json::Value params;
      params["identity"] = ruri.getAorNoPort().c_str();
      _pRpc->CallMethod("isRtcTarget", params, result);
      std::string contact;
      if (!result.isMember("error") && result.isMember("contact"))
      {
        contact = result["contact"].asCString();
        resip::Uri contactUri(contact.c_str());
        return contactUri.exists(resip::p_transport) && (contactUri.param(resip::p_transport) == "WS" || contactUri.param(resip::p_transport) == "ws");

      }
      else if (result.isMember("error"))
      {
        OS_LOG_ERROR(FAC_SIP, " WSRouter::isRTCTarget - " << result["error"]);
      }
      else
      {
        OS_LOG_ERROR(FAC_SIP, " WSRouter::isRTCTarget - User Not Found");
      }

      return false;
    }
    catch (jsonrpc::JsonRpcException e)
    {
        OS_LOG_ERROR(FAC_SIP, "WSRouter::isRTCTarget - FAILED with error: "  << e.what());
        return false;
    }
  }
  
  return false;
}

ReproGlue::RequestProcessor::ChainReaction WSRouter::onProcessRequest(ReproGlue::RequestProcessor& processor, RequestContext& context)
{
  resip::SipMessage& msg = context.getOriginalRequest();
  resip::Uri ruri(msg.header(h_RequestLine).uri());
  
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
  bool isRtcTarget = isRTCTarget(ruri); 
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
    
    if (isRtcOffer)
      OS_LOG_INFO(FAC_SIP, "Detected RTP/SAVPF in SDP.  Assuming RTC Media stream in OFFER.");
  }
  
  OS_LOG_INFO(FAC_SIP, "isRtcTarget=" << isRtcTarget 
    << " isRtcOffer=" << isRtcOffer
    << " isBridgedRtc=" << isBridgedRtc
    << " isMidDialog=" << isMidDialog
    << " isLocalDomain=" << isLocalDomain)
  
  if (isLocalDomain)
  {
    addWsContactParams(msg);
    
    if (msg.method() == resip::REGISTER)
    {
      //
      // all registers go to the proxy
      //
      ruri.scheme() = "sip";
      ruri.host() = _proxyAddress.c_str();
      ruri.port() = _proxyPort;
      ruri.remove(resip::p_transport);
    
      //
      // Insert a path header
      //
      NameAddr path;
      path.uri().scheme() = "sip";
      path.uri().host() = _address.c_str();
      path.uri().port() = _tcpUdpPort;
      path.uri().param(resip::p_lr);
      msg.header(h_Paths).push_front(path);
      

      //
      // Does more harm than good.  Specifically GRUU
      //
      if (msg.exists(h_Supporteds))
      {
        msg.remove(h_Supporteds);
      }
      
    }
    else if (msg.method() == resip::INVITE)
    {
      if (isBridgedRtc)
      {
        OS_LOG_INFO(FAC_SIP, "Local Domain: Detected Bridged INVITE.  Reconstructing original destination before bridge.");
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
        
        if (ruri.exists(p_xtrtc))
        {
          ruri.remove(p_xtrtc);
          ruri.param(resip::p_transport) = resip::Data("WS");
        }
                
        ruri.host() = thost.c_str();
        ruri.port() = boost::lexical_cast<int>(tport);
        ruri.param(p_transport) = resip::Data("ws");
      }
      else if (isRtcOffer && !isRtcTarget)
      {
        OS_LOG_INFO(FAC_SIP, "Local Domain: Detected RTC->SIP INVITE.  Bridging call.");
        routeToBridge(msg, ruri, isRtcTarget);
      }
      else if (!isRtcOffer && isRtcTarget)
      {
        //
        // Offer is legacy terminating to a webrtc endpoint
        // 
        
        OS_LOG_INFO(FAC_SIP, "Local Domain: Detected SIP->RTC INVITE.  Bridging call.");
        routeToBridge(msg, ruri, isRtcTarget);
      }
      else
      {
        //
        // ws->ws or sip to sip goes to the pbx proxy
        //
        OS_LOG_INFO(FAC_SIP, "Local Domain: Detected WS->WS or SIP->SIP INVITE.  Bridging not required.");
        
        ruri.scheme() = "sip";
        ruri.host() = _proxyAddress.c_str();
        ruri.port() = _proxyPort;
        ruri.remove(resip::p_transport);
      }
  
    }
    
    context.getResponseContext().addTarget(resip::NameAddr(ruri));
  }
  else // non-local domain
  {
    if (msg.method() == resip::INVITE)
    {
      if (isRtcOffer && isRtcTarget)
      {
        OS_LOG_INFO(FAC_SIP, "Non-Local Domain: Detected WS->WS .  Bridging not required.");
        //
        // Relay it.  This is a pure webrtc call
        //
        context.getResponseContext().addTarget(resip::NameAddr(ruri));
        return ReproGlue::RequestProcessor::SkipThisChain;
      }
      else if (isRtcOffer && !isRtcTarget)
      {
        OS_LOG_INFO(FAC_SIP, "Non-Local Domain: Detected RTC->SIP INVITE.  Bridging call.");
        routeToBridge(msg, ruri, isRtcTarget);
      }
      else if (!isRtcOffer && isRtcTarget)
      {
        //
        // Offer is webrtc but target is legacy endpoint. bridge it
        //
        OS_LOG_INFO(FAC_SIP, "Non-Local Domain: Detected SIP->RTC INVITE.  Bridging call.");
        //
        // Offer is legacy terminating to a webrtc endpoint
        //       
        routeToBridge(msg, ruri, isRtcTarget);
      }
    }
    //
    // Relay everything else
    //
    context.getResponseContext().addTarget(resip::NameAddr(ruri));
  }
  return ReproGlue::RequestProcessor::SkipThisChain;
}

ReproGlue::RequestProcessor::ChainReaction WSRouter::onProcessResponse(ReproGlue::RequestProcessor& processor, RequestContext& context)
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

ReproGlue::RequestProcessor::ChainReaction WSRouter::onDigestAuthenticate(ReproGlue::RequestProcessor& processor, RequestContext& context)
{
  //
  // This disables authentication for all requests
  //
  // return ReproGlue::RequestProcessor::Continue;


  //
  // Skip register request but authenticate everything else
  //
  Message *message = context.getCurrentEvent();
  SipMessage *sipMessage = dynamic_cast<SipMessage*>(message);
  UserInfoMessage *userInfo = dynamic_cast<UserInfoMessage*>(message);

  if (userInfo)
  {
    OS_LOG_DEBUG(FAC_SIP, "WSRouter::onDigestAuthenticate passing user info event to default chain.");
    return ReproGlue::RequestProcessor::CallDefaultChain;
  }


  //
  // Do not authenticate REGISTER.  This is sent directly to sipX.
  // sipX will do the authenticaton for us
  //
  if (sipMessage && sipMessage->method() == resip::REGISTER)
    return ReproGlue::RequestProcessor::Continue;
   
  //
  // ACK and BYE should not be authenticated
  //
  if (sipMessage && (sipMessage->method() == resip::BYE || sipMessage->method() == resip::ACK))
    return ReproGlue::RequestProcessor::Continue;

  //
  // Do not authenticate requests that are not bound to out local domain
  //
  bool isLocalDomain = false;
  if (sipMessage)
  {
    resip::Uri ruri(sipMessage->header(h_RequestLine).uri());
    for (std::vector<std::string>::const_iterator iter = _domains.begin(); iter != _domains.end(); iter++)
    {
      if (*iter == ruri.host().c_str())
      {
        isLocalDomain = true;
        break;
      }
    }
  }

  if (!isLocalDomain)
    return ReproGlue::RequestProcessor::Continue;

#if 0  
  //
  // jeb: sipXecs needs a mechanism to trust nodes with x-sipX-authIdentity header
  // even if route state is not present in the SIP message.
  // This can be done by maintaining an access list of trusted IP addresses
  // who have the correct signature.  This would allow the bridge
  // to send calls through sipx without having to re-authenicate.
  // For now we will simply use the cache approach and authenticate with sipx
  //
  
  if (sipMessage)
  {
    resip::SipMessage& msg = *sipMessage;
    if (msg.isRequest())
    {
      insertAuthenticator(msg, msg.header(h_RequestLine).uri());
    }
  }
#endif

  
  if (sipMessage)
  {
    resip::Data realm;
    resip::Data user;
    bool isAuthenticated = false;
    //
    // If this is a SIP Message we will try to authenticate it using the cached info
    //
    
    if (sipMessage->exists(resip::h_ProxyAuthorizations))
    {
      OS_LOG_DEBUG(FAC_SIP, "Monkey handling request: DigestAuthenticator (CACHED); reqcontext = " << context);
      
      Auths &authHeaders = sipMessage->header(resip::h_ProxyAuthorizations);

      // if we find a Proxy-Authorization header for a realm we handle, 
      // asynchronously fetch the relevant userAuthInfo from the database
      for (Auths::iterator i = authHeaders.begin() ; i != authHeaders.end() ; ++i)
      {
        // !rwm!  TODO sometime we need to have a separate isMyRealm() function
        if (i->exists(resip::p_realm) && isMyDomain(i->param(resip::p_realm).c_str()) && i->exists(resip::p_username))
        {
        
          //
          // We have an authentication header. check if the cached a1 matches
          //
          resip::Data a1Hash;
          user = i->param(resip::p_username);
          realm = i->param(resip::p_realm);
          AuthInformationGrabber* pAuthGrabber = dynamic_cast<AuthInformationGrabber*>(_pRepro->getExternalAuthGrabber());
          AuthInformationGrabber::AuthInfoRecord rec;
          if (pAuthGrabber->getCachedAuthInfo(user, realm, rec))
          {
            //
            // We have the A1 hash in cache
            //
            std::pair<Helper::AuthResult,Data> result =
              Helper::advancedAuthenticateRequest(*sipMessage, realm, rec.a1.c_str(), 3000);
            
            if (result.first == Helper::Failed)
            {
              InfoLog (<< "Authentication (CACHED) failed for " << user << " at realm " << realm);
              isAuthenticated = false;
              break;
            }
            else if (result.first == Helper::Authenticated)
            {
              isAuthenticated = true;
              break;
            }
          }
          else
          {
            InfoLog (<< "Authentication (CACHED) has no entry for " << user << " at realm " << realm);
            isAuthenticated = false;
            break;
          }
        }
      }
    }
    
    if (isAuthenticated)
    {
      // Delete the Proxy-Auth header for this realm.  
      // other Proxy-Auth headers might be needed by a downsteram node

      resip::Auths &authHeaders = sipMessage->header(resip::h_ProxyAuthorizations);
       // if we find a Proxy-Authorization header for a realm we handle, 
      // asynchronously fetch the relevant userAuthInfo from the database
      for (resip::Auths::iterator i = authHeaders.begin(); i != authHeaders.end(); )
      {
        if(i->exists(resip::p_realm) && resip::isEqualNoCase(i->param(resip::p_realm), realm))
        {
          i = authHeaders.erase(i);
        }
        else
        {
          ++i;
        }
      }
      
      InfoLog (<< "Authentication (CACHED) succeeded for " << user << " at realm " << realm);
      
      return ReproGlue::RequestProcessor::Continue;
    }
  }
  
  return ReproGlue::RequestProcessor::CallDefaultChain;
}


void WSRouter::handleBridgeEvent(const EslConnection::Ptr& pConnection, const EslEvent::Ptr& pEvent)
{
  OS_LOG_DEBUG(FAC_SIP, "WSRouter::handleBridgeEvent - \n" << pEvent->toString());
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
  std::string variable_sip_from_user = pEvent->getHeader("variable_sip_from_user");
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
  
  if (requestUri.exists(p_xtauthid))
  {
    std::string authParam;
    escape_url_parameter(authParam, requestUri.param(p_xtauthid).c_str(), 0);
    
    std::ostringstream identity;
    identity << "sip_h_P-Asserted-Identity=<sip:"
      << variable_sip_from_user << "@" << variable_sip_from_host
      << ";signature=" << authParam << ">";
    
    pConnection->set(identity.str().c_str());
    requestUri.remove(p_xtauthid);
  }
   
  std::ostringstream arg;
  if (tscheme == "ws")
    arg << "{media_webrtc=true}";

  
  
  resip::Data user = variable_sip_from_user.c_str();
  resip::Data realm = variable_sip_from_host.c_str();
  
  AuthInformationGrabber* pAuthGrabber = dynamic_cast<AuthInformationGrabber*>(_pRepro->getExternalAuthGrabber());
  AuthInformationGrabber::AuthInfoRecord rec;
  if (pAuthGrabber->getCachedAuthInfo(user, realm, rec))
  {
    arg << "{sip_invite_domain=" << realm << "}";
    arg << "{sip_auth_username=" << user << "}";
    arg << "{sip_auth_password=" << rec.password << "}";
  }
  else
  {
    arg << "{sip_invite_domain=anonymous.invalid}";
  }
  
  arg << "{sip_invite_to_uri=" << variable_sip_full_to << "}";
  arg << "sofia/" << SWITCH_APPLICATION_NAME << "/" << requestUri;

  pConnection->set("hangup_after_bridge=true");
  pConnection->set("bridge_early_media=true");
  pConnection->set("ignore_early_media=false");

  OS_LOG_INFO(FAC_SIP, "WSRouter::handleBridgeEvent - " << "sofia/" << SWITCH_APPLICATION_NAME << "/" << requestUri);
  pConnection->executeAsync("bridge", arg.str().c_str(), 0);
}