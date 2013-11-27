#include "RpcServer.h"


RpcServer::RpcServer(OsServiceOptions& options, int port) :
  AbstractServer<RpcServer>(new HttpServer(port)),
  _options(options),
  _pPasswordFile(0)
{
  std::string passwordFile;
  _options.getOption("password-file", passwordFile);
  _pPasswordFile = new OsServiceOptions(passwordFile);
  _pPasswordFile->parseOptions();
  
  bindAndAddMethod(new Procedure("getSipPassword", 
    PARAMS_BY_NAME, JSON_STRING, "user", JSON_STRING, NULL), 
    &RpcServer::getSipPassword);
}  

RpcServer::~RpcServer()
{
  delete _pPasswordFile;
}

void RpcServer::getSipPassword(const Json::Value& request, Json::Value& response)
{
  
  std::string user;
  std::string password;
  if (request.isMember("user"))
  {
    user = request["user"].asString();
    OS_LOG_INFO(FAC_NET, "RpcServer::getSipPassword user=" << user);
    
    
    if (_pPasswordFile)
      _pPasswordFile->getOption(user, password);
    
    //if (identity == "2017@ezuce.com")
    //  password = "20172010";
    //else if (identity == "2059@ezuce.com")
    //  password = "20592010";
  }
    
  if (!password.empty())
  {
    response["password"] = password;
    OS_LOG_INFO(FAC_NET, "RpcServer::getSipPassword user=" << user << " SUCCEEDED");
  }
  else
  {
    OS_LOG_INFO(FAC_NET, "RpcServer::getSipPassword identity=" << user << " User Not Found");
    response["error"] = "User Not Found";
  }
}

int main(int argc, char** argv)
{
  OsServiceOptions options(argc, argv, "sipXrtcrpc", "4.6", "(c) EZuce Inc - All Rights Reserved.");
  options.addDaemonOptions();
  options.addOptionInt("http-port", ": The port where the RPC service will listen for connections.", OsServiceOptions::CommandLineOption, true);
  options.addOptionString("password-file", ": The file where the SIP passwords are stored.", OsServiceOptions::CommandLineOption, true);
  options.parseOptions();
  
  int port = 8080;
  options.getOption("http-port", port);
  
  RpcServer server(options, port);
  if (server.StartListening())
  {
    options.waitForTerminationRequest();
    server.StopListening();
  }
  
  return 0;
}
