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

#include "sipxproxy/ReproGlue.h"

  
class WSRouter : public OsServiceOptions
{
public:
  WSRouter(int argc, char** argv, const std::string& daemonName, const std::string& version, const std::string& copyright);
  ~WSRouter();
  ReproGlue::RequestProcessor::ChainReaction onProcessRequestContext(ReproGlue& repro, RequestContext& context);
  bool initialize();
  int main();
protected:
  ReproGlue* _pRepro;
  std::string _proxyAddress;
  int _proxyPort;
  std::string _process;
  std::vector<std::string> _domains;
  std::string _address;
  int _wsPort;
  int _tcpUdpPort;
};


#endif	// WSROUTER_H_INCLUDED

