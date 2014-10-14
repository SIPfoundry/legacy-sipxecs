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

#ifndef WSROUTER_H_INCLUDED
#define	WSROUTER_H_INCLUDED

#include <os/OsServiceOptions.h>
#include <jsonrpc/rpc.h>
#include "ReproGlue.h"
#include <esl/EslListener.h>
#include <sipxyard/RESTClient.h>

  
class WSRouter : public OsServiceOptions
{
public:
  WSRouter(int argc, char** argv, const std::string& daemonName);
  
  ~WSRouter();
  
  ReproGlue::RequestProcessor::ChainReaction onProcessRequest(ReproGlue::RequestProcessor& processor, RequestContext& context);
  
  ReproGlue::RequestProcessor::ChainReaction onProcessResponse(ReproGlue::RequestProcessor& processor, RequestContext& context);
  
  ReproGlue::RequestProcessor::ChainReaction onDigestAuthenticate(ReproGlue::RequestProcessor& processor, RequestContext& context);
  
  void handleBridgeEvent(const EslConnection::Ptr& pConnection, const EslEvent::Ptr& pEvent);
  
  void routeToBridge(resip::SipMessage& request, resip::Uri& requestUri, bool isRtcTarget);
  
  void insertAuthenticator(resip::SipMessage& request, resip::Uri& requestUri);
  
  bool isMyDomain(const char* domain);
  
  bool isRTCTarget(const resip::Uri& ruri);
  
  bool initialize();
  
  int main();
  
  void setEslPort(int eslPort);
  
  int getEslPort() const;

  const std::string& getRealm() const;
  
  ReproGlue* repro();
  
  bool mergeOption(const std::string& option, std::string& value, const char* defval = 0);
  
  bool mergeOption(const std::string& option, std::vector<std::string>& value);
  
  bool mergeOption(const std::string& option, int& value, int defval = -1);
  
protected:
  ReproGlue* _pRepro;
  std::string _proxyAddress;
  int _proxyPort;
  std::string _bridgeAddress;
  int _bridgePort;
  std::string _process;
  std::vector<std::string> _domains;
  std::string _realm;
  std::string _address;
  int _wsPort;
  int _tcpUdpPort;
  std::string _dbPath;
  EslListener _eventListener;
  int _eslPort;
  jsonrpc::Client* _pRpc;
  std::string _rpcUrl;
  RESTClient* _pConfigClient;
};


inline void WSRouter::setEslPort(int eslPort)
{
  _eslPort = eslPort;
}
  
inline int WSRouter::getEslPort() const
{
  return _eslPort;
}

inline ReproGlue* WSRouter::repro()
{
  return _pRepro;
}

inline const std::string& WSRouter::getRealm() const
{
  return _realm;
}

#endif	// WSROUTER_H_INCLUDED



