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

#include <sipXecsService/SipXApplication.h>
#include <sipxyard/RESTServer.h>
#include "Poco/Net/SSLManager.h"
#include <Poco/Net/PrivateKeyPassphraseHandler.h>

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include "YardProcessor.h"
#include "ProcessControl.h"
#include "ConfigDumper.h"


#define APPLICATION_NAME "sipxyard"
#define DEFAULT_PORT 8020
#define DEFAULT_SECURE_PORT 8021
#define TLS_DEFAULT_CIPHER_LIST "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH"

static Poco::Net::Context::Ptr _gpContext;
static Poco::SharedPtr<Poco::Net::PrivateKeyPassphraseHandler> _gpPassphrase;
static Poco::SharedPtr<Poco::Net::InvalidCertificateHandler> _gpAllowInvalidCert;
typedef std::vector<YardProcessor*> CustomHandlers;
static CustomHandlers _gCustomHandlers;
static RESTServer _gHttpServer;
static RESTServer _gHttpsServer;

static void custom_handler(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
{
  std::string path = request.getURI();
  bool handled = false;
  for (CustomHandlers::iterator iter = _gCustomHandlers.begin(); iter != _gCustomHandlers.end(); iter++)
  {
    YardProcessor* pHandler = *iter;
    if (pHandler->willHandleRequest(path))
    {
      pHandler->handleRequest(request, response);
      handled = true;
    }
  }
  
  if (!handled)
  {
    response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
    response.send();
  }
}

static void init_custom_handlers()
{
  ProcessControl* pProcessControl = new ProcessControl();
  pProcessControl->announceAssociatedServer(&_gHttpServer);
  _gCustomHandlers.push_back(pProcessControl);
  
  ConfigDumper* pConfigDumper = new ConfigDumper();
  pConfigDumper->announceAssociatedServer(&_gHttpServer);
  _gCustomHandlers.push_back(pConfigDumper);
}

static void destroy_custom_handlers()
{
  for (CustomHandlers::iterator iter = _gCustomHandlers.begin(); iter != _gCustomHandlers.end(); iter++)
  {
    YardProcessor* pHandler = *iter;
    delete pHandler;
    pHandler = 0;
  }
}

class Passphrase : public Poco::Net::PrivateKeyPassphraseHandler
{
public:
  Passphrase(bool onServerSide, const std::string& passPhrase) :
    Poco::Net::PrivateKeyPassphraseHandler(onServerSide),
    _passPhrase(passPhrase)
  {
  }
  
  void onPrivateKeyRequested(const void* pSender, std::string& privateKey)
  {
    privateKey = _passPhrase;
  }
  
  std::string _passPhrase;
};

class AllowInvalidCert : public Poco::Net::InvalidCertificateHandler
{
public:
  AllowInvalidCert(bool onServerSize, bool allow) :
    Poco::Net::InvalidCertificateHandler(onServerSize),
    _allow(allow)
  {
  }
    
  void onInvalidCertificate(const void* pSender, Poco::Net::VerificationErrorArgs& errorCert)
  {
    errorCert.setIgnoreError(_allow);
  }
  
  bool _allow;
};

int main(int argc, char** argv)
{
  //
  // daemonize early on
  //
  SipXApplication::doDaemonize(argc, argv);

  SipXApplicationData appData =
  {
      APPLICATION_NAME,
      "",
      "",
      "",
      "",
      false, // do not check mongo connection
      false, // increase application file descriptor limits
      true, // block signals on main thread (and all other threads created by main)
            // and process them only on a dedicated thread
      SipXApplicationData::ConfigFileFormatIni, // format type for configuration file
      OsMsgQShared::QUEUE_LIMITED, //limited queue
  };

  SipXApplication& application = SipXApplication::instance();
  OsServiceOptions& options = application.getConfig();
 
  options.addOptionString("host", "The IP Address where the HTTP Server will listen for connections.", OsServiceOptions::CommandLineOption, false);
  options.addOptionInt("port", "The port where the HTTP Server will listen for connections.", OsServiceOptions::CommandLineOption, false);
  options.addOptionString("secure-host", "The IP Address where the HTTP Server will listen for TLS connections.", OsServiceOptions::CommandLineOption, false);
  options.addOptionInt("secure-port", "The port where the HTTP Server will listen for TLS connections.", OsServiceOptions::CommandLineOption, false);
  options.addOptionString("data-directory", "The directory where the HTTP Server will store data.", OsServiceOptions::CommandLineOption, false);
  options.addOptionString("auth-user", "User for Basic/Digest authentication.", OsServiceOptions::CommandLineOption, false);
  options.addOptionString("auth-password", "Password for Basic/Digest authentication.", OsServiceOptions::CommandLineOption, false);
  options.addOptionString("private-key-file", "Contains the path to the private key file used for encryption", OsServiceOptions::CommandLineOption, false);
  options.addOptionString("certificate-file", "Contains the path to the certificate file (in PEM format)", OsServiceOptions::CommandLineOption, false);
  options.addOptionString("ca-location", "Contains the path to the file or directory containing the CA/root certificates.", OsServiceOptions::CommandLineOption, false);
  options.addOptionString("pass-phrase", "Pass phrase if the private key is protected by a password.", OsServiceOptions::CommandLineOption, false);
  options.addOptionFlag("verify-client-certs", "The server sends a client certificate request to the client and verify it.", OsServiceOptions::CommandLineOption);
  options.addOptionFlag("secure-transport-only", "Set this flag if only TLS transport (https) will be allowed.", OsServiceOptions::CommandLineOption);
  
  application.init(argc, argv, appData);
  
  //
  // Initialize the custom handlers
  //
  init_custom_handlers();
  RESTServer::Handler handler = boost::bind(custom_handler, _1, _2);
  _gHttpServer.setCustomHandler(handler);
  _gHttpsServer.setCustomHandler(handler);
  
  std::string user;
  std::string pass;
  std::string host;
  int port = DEFAULT_PORT;
  std::string dataDir;
  std::string secureHost;
  int securePort = DEFAULT_SECURE_PORT;
  std::string privateKeyFile;
  std::string certificateFile;
  std::string caLocation;
  std::string passPhrase;
  bool secureTransportOnly = false;
  bool verifyClientCerts = true;
  bool started = false;
  bool secure_started = false;
  
  options.getOption("auth-user", user);
  options.getOption("auth-password", pass);
  options.getOption("host", host);
  options.getOption("port", port, DEFAULT_PORT);
  options.getOption("data-directory", dataDir); 
  options.getOption("secure-host", secureHost);
  options.getOption("secure-port", securePort, DEFAULT_SECURE_PORT);
  options.getOption("private-key-file", privateKeyFile);
  options.getOption("certificate-file", certificateFile);
  options.getOption("ca-location", caLocation);
  options.getOption("pass-phrase", passPhrase);
  secureTransportOnly = options.hasOption("secure-transport-only");
  verifyClientCerts = options.hasOption("verify-client-certs");
  
  if (!user.empty())
  {
    _gHttpServer.setCredentials(user, pass);
    _gHttpsServer.setCredentials(user, pass);
  }
  
  if (!dataDir.empty())
  {
    _gHttpServer.setDataDirectory(dataDir);
    _gHttpsServer.setDataDirectory(dataDir);
  }
  else
  {
    char *cwd = getcwd(0, 1024);
    dataDir = cwd;
    free(cwd);
    _gHttpServer.setDataDirectory(dataDir);
    _gHttpsServer.setDataDirectory(dataDir);
  }
  
  if (!secureTransportOnly)
  {
    if (!host.empty())
      started = _gHttpServer.start(host, port, false);
    else
      started = _gHttpServer.start(port, false);
  }
  
  if (!caLocation.empty())
  {
    Poco::Net::initializeSSL();
    _gpContext = new Poco::Net::Context(Poco::Net::Context::SERVER_USE, caLocation, verifyClientCerts ? Poco::Net::Context::VERIFY_RELAXED : Poco::Net::Context::VERIFY_NONE, 9, false, TLS_DEFAULT_CIPHER_LIST);
    _gpPassphrase = new Passphrase(true, passPhrase);
    _gpAllowInvalidCert = new AllowInvalidCert(true, false);
    Poco::Net::SSLManager::instance().initializeServer(_gpPassphrase, _gpAllowInvalidCert, _gpContext);
    if (!secureHost.empty())
      secure_started = _gHttpsServer.start(secureHost, securePort, true);
    else
      secure_started = _gHttpsServer.start(securePort, true);
  }
  
  if (started || secure_started)
  {
    application.waitForTerminationRequest(1);
  }
  else
  {
    _exit(-1);
  }
  
  if (started)
  {
    _gHttpServer.stop();
  }
  
  if (secure_started)
  {
    _gHttpsServer.stop();
  }
  
  destroy_custom_handlers();
 
  return 0;
}
