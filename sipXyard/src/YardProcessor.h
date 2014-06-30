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

#ifndef REQUESTPROCESSOR_H_INCLUDED
#define	REQUESTPROCESSOR_H_INCLUDED

#include "sipxyard/RESTServer.h"

class YardProcessor
{
public:
  YardProcessor();
  virtual bool willHandleRequest(const std::string& path) = 0;
  virtual void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) = 0;
  void announceAssociatedServer(RESTServer* pServer);
protected:
  RESTServer* _pServer;
};

//
// Inlines
//

inline YardProcessor::YardProcessor() : _pServer(0) {};
inline void YardProcessor::announceAssociatedServer(RESTServer* pServer){_pServer = pServer;}

#endif	// REQUESTPROCESSOR_H_INCLUDED

