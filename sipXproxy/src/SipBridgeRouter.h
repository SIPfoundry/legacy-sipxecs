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

#ifndef SIPBRIDGEROUTER_H_INCLUDED
#define SIPBRIDGEROUTER_H_INCLUDED


#include <string>
#include <vector>

#include "os/OsFS.h"
#include "net/Url.h"

class SipRouter;
class SipMessage;

class SipBridgeRouter
{
  public: 
    //
    // Type definitions
    //
    typedef OsPath FilePath;
    
  public:
    //
    // Enumerations
    //
    enum ProxyAction
    {
	SendRequest,
	SendResponse,
	DoneSending,
	DoNothing
    };
    
  public:
    SipBridgeRouter(SipRouter* pRouter);
    
    ~SipBridgeRouter();
    
    bool initialize();
    
    ProxyAction 
      proxyMessage(SipMessage& sipRequest, SipMessage& sipResponse);
    
    void setSharedSecret(const std::string& sharedSecret);
    
    const std::string& getSharedSecret() const;
    
    void setBridgeWanAddress(const std::string& bridgeWanAddress);
    
    const std::string& getBridgeWanAddress() const;
    
    void setBridgeWanPort(unsigned short bridgeWanPort);
    
    unsigned short getBridgeWanPort() const;
    
    void setBridgeLanAddress(const std::string& bridgeLanAddress);
    
    const std::string& getBridgeLanAddress() const;
    
    void setBridgeLanPort(unsigned short bridgeLanPort);
    
    unsigned short getBridgeLanPort() const;
    
    const std::vector<std::string>& getItspProxyDomains() const;
    
    void addItspProxyDomain(const std::string& itspProxyDomain);
    
    const std::vector<std::string>& getItspProxyAddresses() const;
    
    void addItspProxyAddress(const std::string& itspProxyAddress);
    
    const FilePath& getBridgeConfigFilePath() const;
    
    const std::string& getLocalDomain() const;
    
    void setLocalDomain(const std::string& localDomain);
    
    //
    // Utility functions
    //
    static bool getTopViaAddressInfo(
      SipMessage& sipRequest,
      std::string& proto,
      std::string& host,
      unsigned short& port);
      
    static bool getFromHost(SipMessage& sipRequest, std::string& host);  
    
    static bool getRequestUriHost(SipMessage& sipRequest, std::string& host);
    
    static bool getRequestUriHostPort(SipMessage& sipRequest, std::string& host, unsigned short& port);
    
    static bool getRequestUri(SipMessage& sipRequest, Url& uri);
    
    static bool retargetUri(SipMessage& sipRequest, const std::string& host, unsigned short port);
    
  private:
    bool isLocalDomain(SipMessage& sipRequest);
    
    bool isFromItspToBridge(SipMessage& sipRequest);
    
    bool isFromBridgeToItsp(SipMessage& sipRequest);
    
    bool isFromKnownItsp(SipMessage& sipRequest);
    
    bool verifySharedSecret(SipMessage& sipRequest);
    
    bool handleAck(SipMessage& sipRequest);
    
    ProxyAction proxyToBridge(
      SipMessage& sipRequest,
      SipMessage& sipResponse);
      
    ProxyAction proxyToItsp(
      SipMessage& sipRequest,
      SipMessage& sipResponse);
    
    SipRouter* _pRouter;
    std::string _sharedSecret;
    std::string _bridgeWanAddress;
    unsigned short _bridgeWanPort;
    std::string _bridgeLanAddress;
    unsigned short _bridgeLanPort;
    std::vector<std::string> _itspProxyDomains;
    std::vector<std::string> _itspProxyAddresses;
    FilePath _bridgeConfigFilePath;
    std::string _localDomain;
};


//
//  Inlines
//

inline void SipBridgeRouter::setSharedSecret(const std::string& sharedSecret)
{
  _sharedSecret = sharedSecret;
}
    
inline const std::string& SipBridgeRouter::getSharedSecret() const
{
  return _sharedSecret;
}

inline void SipBridgeRouter::setBridgeWanAddress(const std::string& bridgeWanAddress)
{
  _bridgeWanAddress = bridgeWanAddress;
}

inline const std::string& SipBridgeRouter::getBridgeWanAddress() const
{
  return _bridgeWanAddress;
}

inline void SipBridgeRouter::setBridgeWanPort(unsigned short bridgeWanPort)
{
  _bridgeWanPort = bridgeWanPort;
}

inline unsigned short SipBridgeRouter::getBridgeWanPort() const
{
  return _bridgeWanPort;
}

inline void SipBridgeRouter::setBridgeLanAddress(const std::string& bridgeLanAddress)
{
  _bridgeLanAddress = bridgeLanAddress;
}
    
inline const std::string& SipBridgeRouter::getBridgeLanAddress() const
{
  return _bridgeLanAddress;
}
    
inline void SipBridgeRouter::setBridgeLanPort(unsigned short bridgeLanPort)
{
  _bridgeLanPort = bridgeLanPort;
}
    
inline unsigned short SipBridgeRouter::getBridgeLanPort() const
{
  return _bridgeLanPort;
}

inline const std::vector<std::string>& SipBridgeRouter::getItspProxyDomains() const
{
  return _itspProxyDomains;
}
    
inline void SipBridgeRouter::addItspProxyDomain(const std::string& itspProxyDomain)
{
  _itspProxyDomains.push_back(itspProxyDomain);
}

inline const std::vector<std::string>& SipBridgeRouter::getItspProxyAddresses() const
{
  return _itspProxyAddresses;
}
    
inline void SipBridgeRouter::addItspProxyAddress(const std::string& itspProxyAddress)
{
  _itspProxyAddresses.push_back(itspProxyAddress);
}

inline const SipBridgeRouter::FilePath& SipBridgeRouter::getBridgeConfigFilePath() const
{
  return _bridgeConfigFilePath;
}

inline const std::string& SipBridgeRouter::getLocalDomain() const
{
  return _localDomain;
}
    
inline void SipBridgeRouter::setLocalDomain(const std::string& localDomain)
{
  _localDomain = localDomain;
}


#endif //SIPBRIDGEROUTER_H_INCLUDED

