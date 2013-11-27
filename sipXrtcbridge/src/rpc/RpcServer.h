
#ifndef RPCSERVER_H
#define	RPCSERVER_H

#include <jsonrpc/rpc.h>
#include <os/OsServiceOptions.h>
#include <os/OsLogger.h>

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
private:
  OsServiceOptions& _options;
  OsServiceOptions* _pPasswordFile;
};

#endif	/* RPCSERVER_H */

