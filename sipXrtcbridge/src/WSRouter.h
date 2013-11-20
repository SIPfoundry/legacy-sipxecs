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
#include "sipx/proxy/ReproGlue.h"
#include "sipx/bridge/FreeSwitchRunner.h"
#include "sipx/bridge/EslListener.h"

  
class WSRouter : public OsServiceOptions
{
public:
  WSRouter(int argc, char** argv, const std::string& daemonName, const std::string& version, const std::string& copyright);
  
  ~WSRouter();
  
  sipx::proxy::ReproGlue::RequestProcessor::ChainReaction onProcessRequest(sipx::proxy::ReproGlue& repro, RequestContext& context);
  
  sipx::proxy::ReproGlue::RequestProcessor::ChainReaction onProcessResponse(sipx::proxy::ReproGlue& repro, RequestContext& context);
  
  void handleBridgeEvent(const sipx::bridge::EslConnection::Ptr& pConnection, const sipx::bridge::EslEvent::Ptr& pEvent);
  
  bool initialize();
  
  int main();
  
  void setEslPort(int eslPort);
  
  int getEslPort() const;
  
protected:
  sipx::proxy::ReproGlue* _pRepro;
  std::string _proxyAddress;
  int _proxyPort;
  std::string _process;
  std::vector<std::string> _domains;
  std::string _address;
  int _wsPort;
  int _tcpUdpPort;
  std::string _dbPath;
  sipx::bridge::EslListener _eventListener;
  sipx::bridge::FreeSwitchRunner* _pSwitch;
  int _eslPort;
};


inline void WSRouter::setEslPort(int eslPort)
{
  _eslPort = eslPort;
}
  
inline int WSRouter::getEslPort() const
{
  return _eslPort;
}

#endif	// WSROUTER_H_INCLUDED



