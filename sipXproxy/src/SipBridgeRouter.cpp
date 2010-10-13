/*
 *
 *
 * Copyright (c) 2010 eZuce, Inc. All rights reserved.
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

#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <iostream>

#include "SipBridgeRouter.h"
#include "os/OsFS.h"
#include "os/OsConfigDb.h"
#include "os/OsSysLog.h"
#include "os/OsEventMsg.h"
#include "utl/UtlRandom.h"
#include "net/NameValueTokenizer.h"
#include "net/SignedUrl.h"
#include "net/SipMessage.h"
#include "net/SipOutputProcessor.h"
#include "net/SipUserAgent.h"
#include "net/SipXauthIdentity.h"
#include "net/SipSrvLookup.h"
#include "sipdb/ResultSet.h"
#include "sipdb/CredentialDB.h"
#include "AuthPlugin.h"
#include "SipRouter.h"
#include "ForwardRules.h"
#include "sipXecsService/SipXecsService.h"
#include "sipXecsService/SharedSecret.h"
#include "xmlparser/tinyxml.h"
#include "sipXecsService/SipXecsService.h"

//
// Globals
//
const char* BRIDGE_CONFIG_SETTINGS_FILE = "sipxbridge.xml";
const char* ITSP_FLAG = "x-itsp-flag";

//
// Utilities
//

#define LOG_ANY(log, priority) \
{ \
  std::ostringstream strm; \
  strm << log; \
  OsSysLog::add(FAC_SIP, priority, strm.str().c_str()); \
}
#define LOG_DEBUG(log) LOG_ANY(log, PRI_DEBUG)
#define LOG_INFO(log) LOG_ANY(log, PRI_INFO)
#define LOG_ERROR(log) LOG_ANY(log, PRI_ERR)
#define LOG_CRITICAL(log) LOG_ANY(log, PRI_CRIT)

template <typename T>
std::string string_from_number(T var)
  /// This is a template function that converts basic types to std::string
{
  std::stringstream strm;
  strm << var;
  return strm.str();
}

template <typename T>
T string_to_number(const char* str)
  /// Convert a string to a numeric value
{
  T value;
  std::istringstream iss(str);
  iss >> value;
  return value;
}

bool SipBridgeRouter::getTopViaAddressInfo(
      SipMessage& sipRequest,
      std::string& proto,
      std::string& host,
      unsigned short& port)
{
  UtlString sAddr, sProto;
  int viaPort = 5060;
  sipRequest.getTopVia( &sAddr, &viaPort, &sProto);
  if (sAddr.isNull() || sProto.isNull())
    return false;
  if (viaPort == 0)
    viaPort = 5060;
  proto = sProto.data();
  host = sAddr.data();
  port = static_cast<unsigned short>(viaPort);
  return true;
}

bool SipBridgeRouter::getFromHost(SipMessage& sipRequest, std::string& host)
{
  Url fromUrl;
  UtlString hostAddress;
  sipRequest.getFromUrl(fromUrl);
  fromUrl.getHostAddress(hostAddress);
  if (hostAddress.isNull())
    return false;
  host = std::string(hostAddress.data());
  return !host.empty();
}
    
bool SipBridgeRouter::getRequestUriHost(SipMessage& sipRequest, std::string& host)
{
  unsigned short port = 0;
  return getRequestUriHostPort(sipRequest, host, port);
}

bool SipBridgeRouter::getRequestUriHostPort(SipMessage& sipRequest, std::string& host, unsigned short& port)
{
  UtlString address, protocol;
  int uriPort;
  sipRequest.getUri(&address, &uriPort, &protocol);
  if (address.isNull())
    return false;
  host = std::string(address.data());
  port = static_cast<unsigned short>(uriPort);
  return !host.empty();
}
    
bool SipBridgeRouter::getRequestUri(SipMessage& sipRequest, Url& uri)
{
  UtlString user, address, protocol;
  int port;
  sipRequest.getUri(&address, &port, &protocol, &user);
  std::string scheme = "sip";
  if (protocol == "tls")
    scheme = "sips";
  
  std::ostringstream strm;
  strm << scheme << ":";
  if (!user.isNull())
    strm << user.data() << "@";
  strm << address.data();
  if (port != 5060 && port != 0)
    strm << ":" << port;
  
  uri.fromString(strm.str().c_str(), TRUE);
  return true;
}

bool SipBridgeRouter::retargetUri(SipMessage& sipRequest, const std::string& host, unsigned short port)
{
  UtlString address, protocol, user;
  int port_; 
  sipRequest.getUri(&address, &port_, &protocol, &user);
  if (port_==0)
    port_=5060;
  if (port==0)
    port = 5060;
  //
  // The must be at least the scheme and the address
  //
  if (protocol.isNull() || address.isNull())
    return false;
  
  if (host == address.data() && port_ == port)
    return true;
  
  std::string scheme = protocol.data();
  if (scheme == "tls")
    scheme = "sips";
  else
    scheme = "sip";
  
  
  std::ostringstream newUri;
  newUri << scheme << ":";
  if (!user.isNull())
    newUri << user.data() << "@";
  newUri << host.data();
  if (port != 5060)
    newUri << ":" << port;
  sipRequest.changeUri(newUri.str().c_str());
  
  return true;
}

//
// Class implementation
//
SipBridgeRouter::SipBridgeRouter(SipRouter* pRouter) : 
  _pRouter(pRouter),
  _bridgeWanPort(5080)
{
  _bridgeConfigFilePath = SipXecsService::Path(SipXecsService::ConfigurationDirType,
                                                 BRIDGE_CONFIG_SETTINGS_FILE);
}

SipBridgeRouter::~SipBridgeRouter()
{
}

bool SipBridgeRouter::initialize()
{
  std::string errorTxt;
  std::string configFile(_bridgeConfigFilePath.data());

  TiXmlDocument doc;
  if (!doc.LoadFile(configFile.c_str()))
    return false;

  TiXmlHandle hDoc(&doc);
  TiXmlElement* docRoot = hDoc.FirstChildElement().Element();
  if (!docRoot)
    return false;
  TiXmlHandle docRootHandle(docRoot);

  TiXmlElement* bridgeConfig = docRootHandle.FirstChild( "bridge-configuration" ).Element();
  if (!bridgeConfig)
    return false;
  
  TiXmlElement* externalAddressNode = bridgeConfig->FirstChildElement("external-address");
  if (!(externalAddressNode && externalAddressNode->FirstChild()))
    return false;
  _bridgeWanAddress = externalAddressNode->FirstChild()->Value();

  TiXmlElement* externalPortNode = bridgeConfig->FirstChildElement("external-port");
  if (!(externalPortNode && externalPortNode->FirstChild()))
    return false;
  std::string externalPort = externalPortNode->FirstChild()->Value();
  _bridgeWanPort = string_to_number<unsigned short>(externalPort.c_str());

  TiXmlElement* localAddressNode = bridgeConfig->FirstChildElement("local-address");
  if (!(localAddressNode && localAddressNode->FirstChild()))
    return false;
  _bridgeLanAddress = localAddressNode->FirstChild()->Value();

  TiXmlElement* localPortNode = bridgeConfig->FirstChildElement("local-port");
  if (!(localPortNode && localPortNode->FirstChild()))
    return false;
  std::string localPort = localPortNode->FirstChild()->Value();
  _bridgeLanPort = string_to_number<unsigned short>(localPort.c_str());
  
  for (TiXmlElement* itspAccountNode = docRootHandle.FirstChild( "itsp-account" ).Element();
      itspAccountNode;
      itspAccountNode = itspAccountNode->NextSiblingElement("itsp-account"))
  {
    TiXmlElement* itspProxyDomainNode = itspAccountNode->FirstChildElement("itsp-proxy-domain");
    if (itspProxyDomainNode && itspProxyDomainNode->FirstChild())
    {
      std::string itspProxyDomain = itspProxyDomainNode->FirstChild()->Value();
      if (!itspProxyDomain.empty())
      {
	LOG_DEBUG("SipBridgeRouter::initialize - " << "Adding " << itspProxyDomain << " to known domain list");
	addItspProxyDomain(itspProxyDomain);
      }
    }
 
    TiXmlElement* itspProxyAddressNode = itspAccountNode->FirstChildElement("itsp-proxy-address");
    if (itspProxyAddressNode && itspProxyAddressNode->FirstChild())
    {
      std::string itspProxyAddress = itspProxyAddressNode->FirstChild()->Value();
      if (!itspProxyAddress.empty())
      {
	LOG_DEBUG("SipBridgeRouter::initialize - " << "Adding " << itspProxyAddress << " to known address list");
	addItspProxyAddress(itspProxyAddress);
      }
    }
  }
  
  if (!_pRouter->mDomainName.isNull())
    _localDomain = _pRouter->mDomainName.data();
  
  return true;
}
   
bool SipBridgeRouter::isLocalDomain(SipMessage& sipRequest)
{
  //
  // Check if request-uri is bound to this domain
  //
  std::string host;
  if (SipBridgeRouter::getRequestUriHost(sipRequest, host))
  {
    if (host == _localDomain)
    {
      LOG_DEBUG("SipBridgeRouter::isLocalDomain() - local:" 
	<< host << "==request-uri:" << host );
      return true;
    }
  }
  
  host = "";
  if (SipBridgeRouter::getFromHost(sipRequest, host))
  {
    if (host == _localDomain)
    {
       LOG_DEBUG("SipBridgeRouter::isLocalDomain() - local:" 
	<< host << "==from-uri:" << host );
      return true;
    }
  }
  
  Url ruri;
  SipBridgeRouter::getRequestUri(sipRequest, ruri);
  
  return _pRouter->mpSipUserAgent->isMyHostAlias(ruri);
}
    
bool SipBridgeRouter::isFromItspToBridge(SipMessage& sipRequest)
{
  std::string proto;
  std::string host;
  unsigned short port;
  
  if (!SipBridgeRouter::getTopViaAddressInfo(sipRequest, proto, host, port))
  {
    LOG_ERROR("SipBridgeRouter::isFromItspToBridge - Unable to retrieve via address information");
    return false;
  }
  
  //
  // Check if the via is pointing to this local domain
  // If it does, then it came from the inside, not from the ITSP
  //
  if (host == _localDomain || host == _bridgeLanAddress)
    return false;
  
  //
  // Via seems to come from the outside.  Check if it's bound to our domain.
  // if it is, the we can route it to the proxy
  //
  //
  // Check if this request is from a known ITSP
  //
  return SipBridgeRouter::isFromKnownItsp(sipRequest);
}

bool SipBridgeRouter::isFromBridgeToItsp(SipMessage& sipRequest)
{
  std::string proto;
  std::string host;
  unsigned short port;
  
  if (!SipBridgeRouter::getTopViaAddressInfo(sipRequest, proto, host, port))
  {
    LOG_ERROR("SipBridgeRouter::isFromBridgeToItsp - Unable to retrieve via address information");
    return false;
  }
  
  //
  // If the host address is the lan address and port is the wan port, then it came from the bridge
  //
  if (host == _bridgeLanAddress && port == _bridgeWanPort)
  {
    //LOG_DEBUG("SipBridgeRouter::isFromBridgeToItsp - " << host << ":" << port << " matches the bridge" );
    return !SipBridgeRouter::isLocalDomain(sipRequest);
  }
  
  return false;
}

bool SipBridgeRouter::verifySharedSecret(SipMessage& sipRequest)
{
  //
  // Implement me!
  //
  return true;
}

SipBridgeRouter::ProxyAction 
  SipBridgeRouter::proxyMessage(SipMessage& sipRequest, SipMessage& sipResponse)
{
  std::string startLine = sipRequest.getFirstHeaderLine();
  //UtlString method;
  //int seq;
  //sipRequest.getCSeqField(&seq, &method);
  //if (method.compareTo(SIP_ACK_METHOD, UtlString::ignoreCase) == 0)
  //{
  //  return SipBridgeRouter::DoNothing;
  //}
  if (isFromItspToBridge(sipRequest))
  {
    LOG_INFO("SipBridgeRouter::proxyMessage - " << startLine << " Request is bound for the bridge."); 
    return proxyToBridge(sipRequest, sipResponse);
  }
  else if (isFromBridgeToItsp(sipRequest))
  {
    LOG_INFO("SipBridgeRouter::proxyMessage - " << startLine << " Request is bound for an ITSP.");
    return proxyToItsp(sipRequest, sipResponse);
  }
  return SipBridgeRouter::DoNothing;
}

bool SipBridgeRouter::isFromKnownItsp(SipMessage& sipRequest)
{
  std::string proto;
  std::string host;
  unsigned short port;
  
  if (!SipBridgeRouter::getTopViaAddressInfo(sipRequest, proto, host, port))
  {
    LOG_ERROR("SipBridgeRouter::isFromKnownItsp - Unable to retrieve via address information");
    return false;
  }
  
  for (std::vector<std::string>::const_iterator iter = _itspProxyAddresses.begin();
    iter != _itspProxyAddresses.end(); iter++)
  {
    if (*iter == host)
    {
      LOG_DEBUG("SipBridgeRouter::isFromKnownItsp - " 
	<< host << " came from a known ITSP");
      return true;
    }
  }
  
  //
  // The via address is not known, lets see if the from host is known
  //
  std::string fromHost;
  if (SipBridgeRouter::getFromHost(sipRequest, fromHost))
  {
    for (std::vector<std::string>::const_iterator iter = _itspProxyDomains.begin();
      iter != _itspProxyDomains.end(); iter++)
    {
      if (*iter == fromHost)
      {
	LOG_DEBUG("SipBridgeRouter::isFromKnownItsp - " 
	<< host << "/" << fromHost << " came from a known ITSP");
	return true;
      }
    }
  }
  
  Url route;
  UtlString topRoute;
  sipRequest.getRouteUri(0, &topRoute);
  UtlString isItsp;
  route.fromString(topRoute, FALSE);
  
  if (route.getUrlParameter(ITSP_FLAG, isItsp, 0) && isItsp == "yes")
  {
    LOG_DEBUG("SipBridgeRouter::isFromKnownItsp - " 
	<< host << "/" << fromHost << " came from a known ITSP.  ITSP flag is set.");
    return true;
  }
	
   //LOG_INFO("SipBridgeRouter::isFromKnownItsp - Unable to match " 
   // << host << "/" << fromHost << " to a known ITSP");
   
  return false;
}

SipBridgeRouter::ProxyAction SipBridgeRouter::proxyToBridge(
  SipMessage& sipRequest,
  SipMessage& sipResponse)
{
  //
  // Ok the ITSP is known.  Now rewrite the request-uri to point to the WAN side of the bridge 
  //
  // Fix strict routes and remove any top route headers that go to myself.
  //
  Url normalizedRequestUri;
  UtlSList removedRoutes;
  sipRequest.normalizeProxyRoutes(_pRouter->mpSipUserAgent,
    normalizedRequestUri, // returns normalized request uri
    &removedRoutes        // route headers popped 
  );
  
  //
  // Retarget the URI to the bridge
  //
  if (!SipBridgeRouter::retargetUri(sipRequest, _bridgeLanAddress, _bridgeWanPort))
    return SipBridgeRouter::DoNothing;
  
  //
  // Decrement max-forwards
  //
  int maxForwards;
  if ( sipRequest.getMaxForwards(maxForwards) )
    maxForwards--;
  else
    maxForwards = _pRouter->mpSipUserAgent->getMaxForwards();
  sipRequest.setMaxForwards(maxForwards);
  
  //if (handleAck(sipRequest))
  //  return SipBridgeRouter::DoneSending;
  
  // Generate the Record-Route string to be used by proxy to Record-Route requests 
  // based on the route name
  UtlString recordRoute;
  Url route(_pRouter->mRouteHostPort);
  route.setUrlParameter("lr",NULL);
  route.toString(recordRoute);
  sipRequest.addRecordRouteUri(recordRoute);
    
  return SipBridgeRouter::SendRequest;
}

SipBridgeRouter::ProxyAction SipBridgeRouter::proxyToItsp(
  SipMessage& sipRequest,
  SipMessage& sipResponse)
{
  //
  // Fix strict routes and remove any top route headers that go to myself.
  //
  Url normalizedRequestUri;
  UtlSList removedRoutes;
  sipRequest.normalizeProxyRoutes(_pRouter->mpSipUserAgent,
    normalizedRequestUri, // returns normalized request uri
    &removedRoutes        // route headers popped 
  );
  
  //
  // Decrement max-forwards
  //
  int maxForwards;
  if ( sipRequest.getMaxForwards(maxForwards) )
    maxForwards--;
  else
    maxForwards = _pRouter->mpSipUserAgent->getMaxForwards();
  sipRequest.setMaxForwards(maxForwards);
  
  
  // Generate the Record-Route string to be used by proxy to Record-Route requests 
  // based on the route name
  UtlString recordRoute;
  Url route(_pRouter->mRouteHostPort);
  route.setUrlParameter("lr",NULL);
  route.setUrlParameter(ITSP_FLAG, "yes");
  route.toString(recordRoute);
  sipRequest.addRecordRouteUri(recordRoute);
  
  //if (handleAck(sipRequest))
  //  return SipBridgeRouter::DoneSending;
  
  //
  // there is no need to rewrite the request-uri.  
  // sipxbridge must already have it set to the correct target
  // 
  return SipBridgeRouter::SendRequest;
}

bool SipBridgeRouter::handleAck(SipMessage& sipRequest)
{
  UtlString method;
  int seq;
  sipRequest.getCSeqField(&seq, &method);
  if (method.compareTo(SIP_ACK_METHOD, UtlString::ignoreCase) == 0)
  {
    UtlString uriField;
    sipRequest.getRequestUri(&uriField);
    UtlString address;
    int       port;
    UtlString protocol;
    UtlString user;
    UtlString userLabel;
    UtlString tag;

    SipMessage::parseAddressFromUri(uriField.data(),
				    &address,
				    &port,
				    &protocol,
				    &user,
				    &userLabel,
				    &tag);

    OsSocket::IpProtocolSocketType protoNumber = OsSocket::UDP;
    if (!protocol.isNull())
      SipMessage::convertProtocolStringToEnum(protocol.data(), protoNumber);
    
    UtlString requestString;
    ssize_t len;
    sipRequest.getBytes(&requestString, &len, true);
    LOG_DEBUG("SipBridgeRouter::handleAck - " << requestString.data());
    // This ACK has no matching INVITE, special case
    _pRouter->mpSipUserAgent->sendStatelessAck(sipRequest,            // this will add via
      address,
      port,
      protoNumber);
    
    return true;
  }
  return false;
}
