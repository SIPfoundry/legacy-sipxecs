
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

