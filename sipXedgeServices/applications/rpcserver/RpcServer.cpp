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

#include "RpcServer.h"

#include <net/Url.h>
#include <sipXecsService/SipXApplication.h>

RpcServer::RpcServer(OsServiceOptions& options, int port) :
  AbstractServer<RpcServer>(new HttpServer(port)),
  _options(options),
  _pPasswordFile(0),
  _pEntityDb(0),
  _pRegDb(0)
{
  if (_options.hasOption("password-file"))
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
  
  _pRegDb = new RegDB(MongoDB::ConnectionInfo::globalInfo());
  
  bindAndAddMethod(new Procedure("getSipPassword", PARAMS_BY_NAME, 
    JSON_STRING, "user", 
    JSON_STRING, "realm", 
    JSON_STRING, 0), 
    &RpcServer::getSipPassword);
  
  bindAndAddMethod(new Procedure("isRtcTarget", PARAMS_BY_NAME, 
    JSON_STRING, "identity", 
    JSON_STRING, 0), 
    &RpcServer::isRtcTarget);
}  

RpcServer::~RpcServer()
{
  delete _pPasswordFile;
  delete _pEntityDb;
  delete _pRegDb;
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

void RpcServer::isRtcTarget(const Json::Value& request, Json::Value& response)
{
  if (request.isMember("identity"))
  {
    std::string identity;
    identity = request["identity"].asString();
    
    int timeNow = OsDateTime::getSecsSinceEpoch();
    RegDB::Bindings bindings;
    static_cast<RegDB*>(_pRegDb)->getUnexpiredContactsUserContaining(
        identity,
        timeNow,
        bindings);

    if (bindings.size() == 1)
    {
      std::string target = bindings[0].getContact();
      Url contact(target.c_str());
      UtlString transport;
      if (contact.getUrlParameter("transport", transport))
      {
        if (!transport.isNull())
        {
          transport.toLower();
          if (transport == "ws")
          {
            OS_LOG_INFO(FAC_NET, "RpcServer::isRtcTarget contact=" << target);
            response["contact"] = target;
            return;
          }
        }
      }
    }
  }
  
  response["error"] = "Not RTC Target Found";
}

int main(int argc, char** argv)
{
  //
  // daemonize early on
  //
  SipXApplication::doDaemonize(argc, argv);

  SipXApplicationData appData =
  {
      "sipxedgerpc",
      "",
      "",
      "",
      "",
      false, // do not check mongo connection
      false,
      true, // increase application file descriptor limits
      true, // block signals on main thread (and all other threads created by main)
            // and process them only on a dedicated thread
      SipXApplicationData::ConfigFileFormatIni, // format type for configuration file
      OsMsgQShared::QUEUE_LIMITED, //limited queue
  };

  SipXApplication& application = SipXApplication::instance();
  OsServiceOptions& options = application.getConfig();

  options.addOptionInt("http-port", ": The port where the RPC service will listen for connections.", OsServiceOptions::CommandLineOption, true);
  options.addOptionString("password-file", ": The file where the SIP passwords are stored.  If not set, mongo will be used.", OsServiceOptions::CommandLineOption, false);

  application.init(argc, argv, appData);
  
  int port = 8080;
  options.getOption("http-port", port);
  
  RpcServer server(options, port);
  if (server.StartListening())
  {
    application.waitForTerminationRequest(1);
    server.StopListening();
  }
  else
  {
      std::cerr << "RPC Server is unable to bind to port " << port << std::endl;
      return -1;
  }
  
  return 0;
}
