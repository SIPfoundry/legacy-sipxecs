#include "RpcServer.h"


RpcServer::RpcServer(OsServiceOptions& options, int port) :
  AbstractServer<RpcServer>(new HttpServer(port)),
  _options(options),
  _pPasswordFile(0),
  _pEntityDb(0)
{
  if (_options.hasOption("password-file", true))
  {
    std::string passwordFile;
    _options.getOption("password-file", passwordFile);
    _pPasswordFile = new OsServiceOptions(passwordFile);
    _pPasswordFile->parseOptions();
  }
  else
  {
    _pEntityDb = new EntityDB(MongoDB::ConnectionInfo::globalInfo());
  }
  
  bindAndAddMethod(new Procedure("getSipPassword", PARAMS_BY_NAME, 
    JSON_STRING, "user", 
    JSON_STRING, "realm", 
    JSON_STRING, 0), 
    &RpcServer::getSipPassword);
}  

RpcServer::~RpcServer()
{
  delete _pPasswordFile;
  delete _pEntityDb;
}

void RpcServer::getSipPassword(const Json::Value& request, Json::Value& response)
{
  
  std::string user;
  std::string password;
  std::string realm;
  std::string errorString = "User Not Found";
  
  if (request.isMember("user") && request.isMember("realm"))
  {
    user = request["user"].asString();
    realm = request["realm"].asString();
    
    OS_LOG_INFO(FAC_NET, "RpcServer::getSipPassword user=" << user << " realm=" << realm);
    
    if (_pPasswordFile)
    {
      _pPasswordFile->getOption(user, password);
    }
    else if (_pEntityDb)
    {
      try
      {
        std::ostringstream identity;
        identity << user << "@" << realm;
        EntityRecord entity;
        if (_pEntityDb->findByIdentity(identity.str(), entity))
        {
          password = entity.password();
        }
      }
      catch(std::exception& e)
      {
        errorString = e.what();
      }
    }
  }
    
  if (!password.empty())
  {
    response["password"] = password;
    OS_LOG_INFO(FAC_NET, "RpcServer::getSipPassword user=" << user << " SUCCEEDED");
  }
  else
  {
    OS_LOG_INFO(FAC_NET, "RpcServer::getSipPassword user=" << user << " " << errorString);
    response["error"] = "User Not Found";
  }
}

int main(int argc, char** argv)
{
  OsServiceOptions::daemonize(argc, argv);
  OsServiceOptions options(argc, argv, "sipXrtcrpc", "4.6", "(c) EZuce Inc - All Rights Reserved.");
  options.addDaemonOptions();
  options.addOptionInt("http-port", ": The port where the RPC service will listen for connections.", OsServiceOptions::CommandLineOption, true);
  options.addOptionString("password-file", ": The file where the SIP passwords are stored.  If not set, mongo will be used.", OsServiceOptions::CommandLineOption, false);
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
