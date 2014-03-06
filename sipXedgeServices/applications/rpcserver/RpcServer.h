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


#ifndef RPCSERVER_H
#define	RPCSERVER_H

#include <jsonrpc/rpc.h>
#include <os/OsServiceOptions.h>
#include <os/OsLogger.h>
#include <sipdb/EntityDB.h>
#include <sipdb/RegDB.h>

using namespace jsonrpc;

class RpcServer : public AbstractServer<RpcServer>
{
public:
  RpcServer(OsServiceOptions& options, int port);
  ~RpcServer();
  
  //
  // RPC methods
  //
  void getSipPassword(const Json::Value& request, Json::Value& response);
  
  void isRtcTarget(const Json::Value& request, Json::Value& response);
  
private:
  OsServiceOptions& _options;
  OsServiceOptions* _pPasswordFile;
  EntityDB* _pEntityDb;
  RegDB* _pRegDb;
};

#endif	/* RPCSERVER_H */

