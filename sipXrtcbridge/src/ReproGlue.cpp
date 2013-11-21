
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


#include <iostream>
#include <fstream>
#include <stdexcept>
#include <sstream>

#undef HAVE_CONFIG_H

#include "rutil/Log.hxx"
#include "rutil/Logger.hxx"
#include "rutil/DnsUtil.hxx"
#include "rutil/dns/DnsStub.hxx"
#include "rutil/GeneralCongestionManager.hxx"
#include "rutil/TransportType.hxx"

#include "resip/stack/SipStack.hxx"
#include "resip/stack/Compression.hxx"
#include "resip/stack/EventStackThread.hxx"
#include "resip/stack/InteropHelper.hxx"
#include "resip/stack/ConnectionManager.hxx"

#include "resip/dum/InMemorySyncRegDb.hxx"
#include "resip/dum/MasterProfile.hxx"
#include "resip/dum/DialogUsageManager.hxx"
#include "resip/dum/DumThread.hxx"
#include "resip/dum/TlsPeerAuthManager.hxx"
#include "resip/dum/WsCookieAuthManager.hxx"

#include "repro/AsyncProcessorWorker.hxx"
#include "repro/ReproRunner.hxx"
#include "repro/Proxy.hxx"
#include "repro/ProxyConfig.hxx"
#include "repro/BerkeleyDb.hxx"
#include "repro/Dispatcher.hxx"
#include "repro/UserAuthGrabber.hxx"
#include "repro/ProcessorChain.hxx"
#include "repro/ReproVersion.hxx"
#include "repro/WebAdmin.hxx"
#include "repro/WebAdminThread.hxx"
#include "repro/Registrar.hxx"
#include "repro/ReproServerAuthManager.hxx"
#include "repro/RegSyncClient.hxx"
#include "repro/RegSyncServer.hxx"
#include "repro/RegSyncServerThread.hxx"
#include "repro/CommandServer.hxx"
#include "repro/CommandServerThread.hxx"
#include "repro/BasicWsConnectionValidator.hxx"
#include "repro/monkeys/CookieAuthenticator.hxx"
#include "repro/monkeys/IsTrustedNode.hxx"
#include "repro/monkeys/AmIResponsible.hxx"
#include "repro/monkeys/DigestAuthenticator.hxx"
#include "repro/monkeys/LocationServer.hxx"
#include "repro/monkeys/RecursiveRedirect.hxx"
#include "repro/monkeys/SimpleStaticRoute.hxx"
#include "repro/monkeys/StaticRoute.hxx"
#include "repro/monkeys/StrictRouteFixup.hxx"
#include "repro/monkeys/OutboundTargetHandler.hxx"
#include "repro/monkeys/QValueTargetHandler.hxx"
#include "repro/monkeys/SimpleTargetHandler.hxx"
#include "repro/monkeys/GeoProximityTargetSorter.hxx"
#include "repro/monkeys/RequestFilter.hxx"
#include "repro/monkeys/MessageSilo.hxx"
#include "repro/monkeys/CertificateAuthenticator.hxx"

#include "sipx/proxy/ReproGlue.h"




#define RESIPROCATE_SUBSYSTEM resip::Subsystem::REPRO

using namespace resip;
using namespace repro;


namespace sipx {
namespace proxy {


/////////////
// Monkeys //
/////////////

ReproGlue::RequestProcessor::RequestProcessor(
  repro::Processor* pDefaultChain,
  ReproGlue& reproGlue, 
  const std::string& name)
  : Processor(name.c_str()), 
    _reproGlue(reproGlue),
    _pDefaultChain(pDefaultChain)
{
}

ReproGlue::RequestProcessor::~RequestProcessor()
{
  std::cout << "RequestProcessor::~RequestProcessor DESTROEYED" << std::endl;
  delete _pDefaultChain;
}

void ReproGlue::RequestProcessor::setHandler(RequestCallBack callback)
{
  _callback = callback;
}

repro::Processor::processor_action_t ReproGlue::RequestProcessor::process(RequestContext& context)
{
  //
  // If the callback is not defined, call the default processor
  //
  ChainReaction reaction = Continue;
  
  if (_pDefaultChain || _callback)
  {
  
    if (!_callback)
      return _pDefaultChain->process(context);
    else 
      reaction = _callback(_reproGlue, context);
    //
    // If the _callback says that we still need to call the default chain, we obey.
    //
    if (reaction == CallDefaultChain)
      return _pDefaultChain->process(context);
  }
  
  return (processor_action_t)(int)reaction;
}

////////////
// Logger //
////////////

ReproGlue::ReproLogger ReproGlue::_logger;

ReproGlue::ReproLogger::ReproLogger() :
  _initialized(false),
  _logLevel(Info)
{
}
    
ReproGlue::ReproLogger::~ReproLogger()
{
}

bool ReproGlue::ReproLogger::operator()(Log::Level level,
                        const Subsystem& subsystem, 
                        const Data& appName,
                        const char* file,
                        int line,
                        const Data& message,
                        const Data& messageWithHeaders)
{
  (void)subsystem;
  (void)appName;
  (void)file;
  (void)line;
  (void)(messageWithHeaders);
  
  if (_callback)
   _callback(level, message.c_str());
  
  return !_callback;
}
    
void ReproGlue::ReproLogger::setLogLevel(ReproGlue::ReproLogger::LogLevel level)
{
  _logLevel = level;
  if (_initialized)
    Log::setLevel((Log::Level)(int)level);
}


void ReproGlue::ReproLogger::setLogCallBack(ReproGlue::ReproLogger::LogCallBack callBack)
{
  _callback = callBack;
}

bool ReproGlue::ReproLogger::initialize()
{
  if (_initialized)
    return false;
  //
  // NONE|CRIT|ERR|WARNING|INFO|DEBUG|STACK
  //
  Data level;

  switch (_logLevel)
  {
    case None:
      level = "NONE";
      break;
    case Crit:
      level = "CRIT";
      break;
    case Err:
      level = "ERR";
      break;
    case Warning:
      level = "WARNING";
      break;
    case Info:
      level = "INFO";
      break;
    case Debug:
      level = "DEBUG";
      break;
    case Stack:
      level = "STACK";
      break;
    default:
      level = "INFO";
  };

  Log::initialize("cout", 
                  level, 
                  "ReproGlue", 
                  "", this);

  _initialized = true;
  
  return _initialized;
}





///////////////
// ReproGlue //
///////////////

ReproGlue::ReproGlue(const std::string& applicationName, const std::string& databaseDir) :
  _pReproConfig(0),
  _applicationName(applicationName),
  _pStaticRouter(0),
  _pTargetProcessor(0),
  _pResponseProcessor(0),
  _currentTransportCount(0),
  _enableWebAdmin(false)
{
  mProxyConfig = _pReproConfig = new ReproConfig(); // This pointer is owned by ReproRunner.
                                                    // Do not delete it here.

  //
  // Create the data store early so we can allow add/update operations
  // upon construction of ReproGlue.
  //
  _pReproConfig->setDefaultConfigValue("DatabasePath", databaseDir.c_str());
  createDatastore();

  //
  // Data stores have been created.  We can now create custom processor chains
  //
  _pStaticRouter = new RequestProcessor(new StaticRoute(*mProxyConfig), *this, "CustomStaticRoute");
  _pTargetProcessor = new RequestProcessor(0, *this, "CustomOutboundRule");
  _pResponseProcessor = new RequestProcessor(0, *this, "CustomResponseRule");
}


void ReproGlue::setProxyConfigValue(const std::string& name, const std::string& value, bool custom)
{
  if (!name.empty())
  {
    if (custom)
      _pReproConfig->_customConfigMap[name] = value;
    else
      _pReproConfig->_configMap[name] = value;
  }
}

void ReproGlue::ReproConfig::setDefaultConfigValue(const std::string& name, const std::string& value)
{
  if (name.empty())
    return;
  
  ConfigMap::iterator override = _configMap.find(name);
  if (override != _configMap.end())
  {
    Data configName(override->first.c_str());
    Data configValue(override->second.c_str());
    configName.lowercase();
    insertConfigValue(configName, configValue);
  }
  else
  {
    Data configName(name.c_str());
    Data configValue(value.c_str());
    configName.lowercase();
    insertConfigValue(configName, configValue);
  }
}

void ReproGlue::ReproConfig::setCustomConfigurations()
{
  for (ConfigMap::const_iterator iter = _customConfigMap.begin(); iter != _customConfigMap.end(); iter++)
  {
    Data configName(iter->first.c_str());
    Data configValue(iter->second.c_str());
    configName.lowercase();
    insertConfigValue(configName, configValue);
  }
}

void ReproGlue::configureRepro()
{
  //
  // Populate the custom configuration entries first
  //
  _pReproConfig->setCustomConfigurations();
  
//
//
//  ########################################################
//  # Log settings
//  ########################################################
//
//  # Logging Type: syslog|cerr|cout|file
//  # Note:  Logging to cout can negatively effect performance.
//  #        When repro is placed into production 'file' or 
//  #        'syslog' should be used.
//  LoggingType = cout 
  
  _pReproConfig->setDefaultConfigValue("LoggingType", "cout");
  
//
//  # Logging level: NONE|CRIT|ERR|WARNING|INFO|DEBUG|STACK
//  LogLevel = INFO
  
  _pReproConfig->setDefaultConfigValue("LogLevel", "INFO");
  
//
//  # Log Filename
//  LogFilename = repro.log
  
  _pReproConfig->setDefaultConfigValue("LogFilename", "reproglue.log");
  
//
//  # Log file Max Bytes
//  LogFileMaxBytes = 5242880
  
  _pReproConfig->setDefaultConfigValue("LogFileMaxBytes", "5242880");
  
//
//  # Instance name to be shown in logs, very useful when multiple instances
//  # logging to syslog concurrently
//  # If unspecified, defaults to argv[0] (name of the executable)
//  #LoggingInstanceName repro-dev
  
  
//
//  ########################################################
//  # Transport settings
//  ########################################################
//
//  # Local IP Address to bind SIP transports to. If left blank
//  # repro will bind to all adapters.
//  #IPAddress = 192.168.1.106
//  #IPAddress = 2001:5c0:1000:a::6d
//  IPAddress =
  
  _pReproConfig->setDefaultConfigValue("IPAddress", "");
  
//
//  # Local port to listen on for SIP messages over UDP - 0 to disable
//  UDPPort = 5060
  
  _pReproConfig->setDefaultConfigValue("UDPPort", "5060");
  
//
//  # Local port to listen on for SIP messages over TCP - 0 to disable
//  TCPPort = 5060
  
  _pReproConfig->setDefaultConfigValue("TCPPort", "5060");
  
//
//  # Local port to listen on for SIP messages over TLS - 0 to disable
//  TLSPort = 0
  
  _pReproConfig->setDefaultConfigValue("TLSPort", "0");
  
//
//  # Local port to listen on for SIP messages over WS (WebSocket) - 0 to disable
//  WSPort = 5062
  
  _pReproConfig->setDefaultConfigValue("WSPort", "5062");
  
//
//  # Local port to listen on for SIP messages over WSS (WebSocket TLS) - 0 to disable
//  WSSPort = 0
  _pReproConfig->setDefaultConfigValue("WSSPort", "0");
  
//
//  # Local port to listen on for SIP messages over DTLS - 0 to disable
//  DTLSPort = 0
  
  _pReproConfig->setDefaultConfigValue("DTLSPort", "0");
  
//
//  # TLS domain name for this server (note: domain cert for this domain must be present)
//  TLSDomainName =
  
  _pReproConfig->setDefaultConfigValue("TLSDomainName", "");
  
//
//  # PEM-encoded X.509 certificate for TLS
//  # Must contain any intermediate certificates from the CA
//  # The TLSCertificate and TLSPrivateKey parameters are optional.  The stack
//  # will also try to automatically detect any suitable certificates
//  # in the directory specified by CertificatePath
//  TLSCertificate = 
  
  _pReproConfig->setDefaultConfigValue("TLSCertificate", "");
  
//
//  # PEM-encoded private key for TLS
//  TLSPrivateKey =
  
  _pReproConfig->setDefaultConfigValue("TLSPrivateKey", "");
  
//
//  # Whether or not we ask for (Optional) or expect (Mandatory) TLS
//  # clients to present a client certificate
//  # Possible values:
//  #  None: client can connect without any cert, if a cert is sent, it is not checked
//  #  Optional: client can connect without any cert, if a cert is sent, it must be acceptable to us
//  #  Mandatory: client can not connect without any cert, cert must be acceptable to us
//  # How we decide if a cert is acceptable: it must meet two criteria:
//  # 1. it must be signed by a CA that we trust (see CADirectory)
//  # 2. the domain or full sip: URI in the cert must match the From: URI of all
//  #    SIP messages coming from the peer
//  TLSClientVerification = None
  
  _pReproConfig->setDefaultConfigValue("TLSClientVerification", "none");
  
//
//  # Whether we accept the subjectAltName email address as if it was a SIP
//  # address (when checking the validity of a client certificate)
//  # Very few commercial CAs offer support for SIP addresses in subjectAltName
//  # For many purposes, an email address subjectAltName may be considered
//  # equivalent within a specific domain.
//  # Currently, this accepts such certs globally (for any incoming connection),
//  # not just for connections from the local users.
//  TLSUseEmailAsSIP = false
  
  _pReproConfig->setDefaultConfigValue("TLSUseEmailAsSIP", "false");
  
//
//  # Alternate and more flexible method to specify transports to bind to.  If specified here
//  # then IPAddress, and port settings above are ignored.
//  # Transports MUST be numbered in sequential order, starting from 1.  Possible settings are:
//  # Transport<Num>Interface = <IPAddress>:<Port> - Note:  For IPv6 addresses last colon separates
//  #                                                IP Address and Port - square bracket notation
//  #                                                is not used.
//  # Transport<Num>Type = <'TCP'|'UDP'|'TLS'|'DTLS'|'WS'|'WSS'> - default is UDP if missing
//  # Transport<Num>TlsDomain = <TLSDomain> - only required if transport is TLS, DTLS or WSS
//  # Transport<Num>TlsCertificate = <TLSCertificate> - only for TLS, DTLS or WSS
//  # Transport<Num>TlsPrivateKey = <TLSPrivateKey> - only for TLS, DTLS or WSS
//  # Transport<Num>TlsClientVerification = <'None'|'Optional'|'Mandatory'> - default is None
//  # Transport<Num>RecordRouteUri = <'auto'|URI> - if set to auto then record route URI
//  #                                               is automatically generated from the other
//  #                                               transport settings.  Otherwise explicity
//  #                                               enter the full URI you want repro to use.
//  #                                               Do not specify 'auto' if you specified
//  #                                               the IPAddress as INADDR_ANY (0.0.0.0).
//  #                                               If nothing is specified then repro will
//  #                                               use the global RecordRouteUri setting.
//  #
//  # Transport<Num>RcvBufLen = <SocketReceiveBufferSize> - currently only applies to UDP transports,
//  #                                                       leave empty to use OS default
//  # Example:
//  # Transport1Interface = 192.168.1.106:5060
//  # Transport1Type = TCP
//  # Transport1RecordRouteUri = auto
//  #
//  # Transport2Interface = 192.168.1.106:5060
//  # Transport2Type = UDP
//  # Transport2RecordRouteUri = auto
//  # Transport2RcvBufLen = 10000
//  #
//  # Transport3Interface = 192.168.1.106:5061
//  # Transport3Type = TLS
//  # Transport3TlsDomain = sipdomain.com
//  # Transport3TlsCertificate = /etc/ssl/crt/sipdomain.com.crt
//  # Transport3TlsPrivateKey = /etc/ssl/private/sipdomain.com.key
//  # Transport3TlsClientVerification = Mandatory
//  # Transport3RecordRouteUri = sip:h1.sipdomain.com;transport=TLS
//  #
//  # Transport4Interface = 2666:f0d0:1008:88::4:5060
//  # Transport4Type = UDP
//  # Transport4RecordRouteUri = auto
//
//  # Transport5Interface = 192.168.1.106:5062
//  # Transport5Type = WS
//  # Transport5RecordRouteUri = auto
//
//  # Transport6Interface = 192.168.1.106:5063
//  # Transport6Type = WSS
//  # Transport6TlsDomain = sipdomain.com
//  # Transport6TlsClientVerification = None
//  # Transport6RecordRouteUri = sip:h1.sipdomain.com;transport=WS
  

  
//
//  # Comma separated list of DNS servers, overrides default OS detected list (leave blank 
//  # for default)
//  DNSServers =
  
  _pReproConfig->setDefaultConfigValue("DNSServers", "");
  
//
//  # Enable IPv6
//  EnableIPv6 = true
  
  _pReproConfig->setDefaultConfigValue("EnableIPv6", "true");
  
//
//  # Enable IPv4
//  DisableIPv4 = false
  
  _pReproConfig->setDefaultConfigValue("DisableIPv4", "false");
  
//
//  # Comma separated list of IP addresses used for binding the HTTP configuration interface
//  # and/or certificate server. If left blank it will bind to all adapters.
//  HttpBindAddress =
  
  _pReproConfig->setDefaultConfigValue("HttpBindAddress", "");
  
//
//  # Port on which to run the HTTP configuration interface and/or certificate server 
//  # 0 to disable (default: 5080)
//  HttpPort = 5080
  
  _pReproConfig->setDefaultConfigValue("HttpPort", "8080");
  
//
//  # disable HTTP challenges for web based configuration GUI
//  DisableHttpAuth = false
  
  _pReproConfig->setDefaultConfigValue("DisableHttpAuth", "false");
  
//
//  # Web administrator password
//  HttpAdminPassword = admin
  
  _pReproConfig->setDefaultConfigValue("HttpAdminPassword", "admin");
  
//
//  # Comma separated list of IP addresses used for binding the Command Server listeners.
//  # If left blank it will bind to all adapters.
//  CommandBindAddress =
  
  _pReproConfig->setDefaultConfigValue("CommandBindAddress", "");
  
//
//  # Port on which to listen for and send XML RPC messaging used in command processing 
//  # 0 to disable (default: 5081)
//  CommandPort = 5081
  
  _pReproConfig->setDefaultConfigValue("CommandPort", "5081");
  
//
//  # Port on which to listen for and send XML RPC messaging used in registration sync 
//  # process - 0 to disable (default: 0)
//  RegSyncPort = 0
  
  _pReproConfig->setDefaultConfigValue("RegSyncPort", "0");
  
  
//
//  # Hostname/ip address of another instance of repro to synchronize registrations with 
//  # (note xmlrpcport must also be specified)
//  RegSyncPeer =
  
  _pReproConfig->setDefaultConfigValue("RegSyncPeer", "");
  
//
//
//  ########################################################
//  # Misc settings
//  ########################################################
//
//  # Drop privileges and run as some other user and group
//  # If RunAsUser is specified and RunAsGroup is not specified,
//  # then setgid will be invoked using the default group for
//  # the specified user
//  # If neither option is specified, then no attempt will be made
//  # to call setuid/setgid (there is no default value)
//  #RunAsUser = repro
//  #RunAsGroup = repro
//
//  # Must be true or false, default = false, not supported on Windows
//  Daemonize = false
  
  _pReproConfig->setDefaultConfigValue("Daemonize", "false");
  
//
//  # On UNIX it is normal to create a PID file
//  # if unspecified, no attempt will be made to create a PID file
//  #PidFile = /var/run/repro/repro.pid
//
//  # Path to load certificates from (optional, there is no default)
//  # Note that repro loads ALL root certificates found by any of the settings
//  #
//  #    CertificatePath
//  #    CADirectory
//  #    CAFile
//  #
//  # Setting one option does not disable the other options.
//  #
//  # Certificates in this location have to match one of the filename
//  # patterns expected by the legacy reSIProcate SSL code:
//  #
//  #   domain_cert_NAME.pem, root_cert_NAME.pem, ...
//  #
//  # For domain certificates, it is recommended to use the options
//  # for individual transports, such as TransportXTlsCertificate and
//  # TransportXTlsPrivateKey and not set CertificatePath at all.
//  #
//  CertificatePath =
  
  _pReproConfig->setDefaultConfigValue("CertificatePath", "");
  
//
//  # Path to load root certificates from
//  # Iff this directory is specified, all files in the directory
//  # will be loaded as root certificates, prefixes and suffixes are
//  # not considered
//  # Note that repro loads ALL root certificates found by the settings
//  # CertificatePath, CADirectory and CAFile.  Setting one option does
//  # not disable the other options.
//  # On Debian, the typical location is /etc/ssl/certs
//  #CADirectory = /etc/ssl/certs
//
//  # Specify a single file containing one or more root certificates
//  # and possible chain/intermediate certificates to be loaded
//  # Iff this filename is specified, the certificates in the file will
//  # be loaded as root certificates
//  #
//  # This does NOT currently support bundles of unrelated root certificates
//  # stored in the same PEM file, it ONLY supports related/chained root
//  # certificates.  If multiple roots must be supported, use the CADirectory
//  # option.
//  #
//  # In the future, this behavior may change to load a bundle,
//  # such as /etc/ssl/certs/ca-certificates.txt on Debian and
//  # /etc/pki/tls/cert.pem on Red Hat/CentOS
//  #
//  # Note that repro loads ALL root certificates found by the settings
//  # CertificatePath, CADirectory and CAFile.  Setting one option does
//  # not disable the other options.
//  #
//  # This example loads just the CACert.org chain, which typically
//  # includes the class 1 root and the class 3 root (signed by the class 1 root)
//  #CAFile = /etc/ssl/certs/cacert.org.pem
//
//  # The Path to read and write Berkely DB database files
//  DatabasePath = ./
  
//  _pReproConfig->setDefaultConfigValue("DatabasePath", SIPX_DBDIR);
  
//
//  # The hostname running MySQL server to connect to, leave blank to use BerkelyDB.
//  # The value of host may be either a host name or an IP address. If host is "localhost",
//  # a connection to the local host is assumed. For Windows, the client connects using a
//  # shared-memory connection, if the server has shared-memory connections enabled. Otherwise,
//  # TCP/IP is used. For Unix, the client connects using a Unix socket file. For a host value of
//  # "." on Windows, the client connects using a named pipe, if the server has named-pipe
//  # connections enabled. If named-pipe connections are not enabled, an error occurs.
//  # WARNING: repro must be compiled with the USE_MYSQL flag in order for this work.
//  MySQLServer =
  
  _pReproConfig->setDefaultConfigValue("MySQLServer", "");
  
//
//  # The MySQL login ID to use when connecting to the MySQL Server. If user is empty string "",
//  # the current user is assumed. Under Unix, this is the current login name. Under Windows,
//  # the current user name must be specified explicitly.
//  MySQLUser = root
  
  _pReproConfig->setDefaultConfigValue("MySQLUser", "root");
  
//
//  # The password for the MySQL login ID specified.
//  MySQLPassword = root
  
  _pReproConfig->setDefaultConfigValue("MySQLPassword", "root");
  
//
//  # The database name on the MySQL server that contains the repro tables
//  MySQLDatabaseName = repro
  
  _pReproConfig->setDefaultConfigValue("MySQLDatabaseName", "repro");
  
//
//  # If port is not 0, the value is used as the port number for the TCP/IP connection. Note that
//  # the host parameter determines the type of the connection.
//  MySQLPort = 3306
  
  _pReproConfig->setDefaultConfigValue("MySQLPort", "3306");
  
//
//  # The Users and MessageSilo database tables are different from the other repro configuration
//  # database tables, in that they are accessed at runtime as SIP requests arrive.  It may be
//  # desirable to use BerkeleyDb for the other repro tables (which are read at starup time, then 
//  # cached in memory), and MySQL for the runtime accessed tables; or two seperate MySQL instances 
//  # for these different table sets.  Use the following settings in order to specify a seperate 
//  # MySQL instance for use by the Users and MessageSilo tables.
//  #
//  # WARNING: repro must be compiled with the USE_MYSQL flag in order for this work.
//  # 
//  # Note:  If this setting is left blank then repro will fallback all remaining my sql
//  # settings to use the global MySQLServer settings.  If the MySQLServer setting is also
//  # blank, then repro will use BerkelyDB for all configuration tables.  See the 
//  # documentation on the global MySQLServer settings for more details on the following 
//  # individual settings.
//  RuntimeMySQLServer =
  
  _pReproConfig->setDefaultConfigValue("RuntimeMySQLServer", "");
  
//  RuntimeMySQLUser = root
  
  _pReproConfig->setDefaultConfigValue("RuntimeMySQLUser", "root");
  
//  RuntimeMySQLPassword = root
  
  _pReproConfig->setDefaultConfigValue("RuntimeMySQLPassword", "root");
  
//  RuntimeMySQLDatabaseName = repro
  
  _pReproConfig->setDefaultConfigValue("RuntimeMySQLDatabaseName", "repro");
  
//  RuntimeMySQLPort = 3306
  
  _pReproConfig->setDefaultConfigValue("RuntimeMySQLPort", "3306");
  
//
//  # If you would like to be able to authenticate users from a MySQL source other than the repro user
//  # database table itself, then specify the query here.  The following conditions apply:
//  # 1.  The database table must reside on the same MySQL server instance as the repro database
//  #     or Runtime tables database.
//  # 2.  The statement provided will be UNION'd with the hardcoded repro query, so that auth from
//  #     both sources is possible.  Note:  If the same user exists in both tables, then the repro
//  #     auth info will be used.
//  # 3.  The provided SELECT statement must return the SIP A1 password hash of the user in question.
//  # 4.  The provided SELECT statement must contain two tags embedded into the query: $user and $domain
//  #     These tags should be used in the WHERE clause, and repro will replace these tags with the
//  #     actual user and domain being queried.
//  # Example:  SELECT sip_password_ha1 FROM directory.users WHERE sip_userid = '$user' AND 
//  #           sip_domain = '$domain' AND account_status = 'active'
//  MySQLCustomUserAuthQuery =
  
  _pReproConfig->setDefaultConfigValue("MySQLCustomUserAuthQuery", "");
  
//
//  # Session Accounting - When enabled resiprocate will push a JSON formatted 
//  # events for sip session related messaging that the proxy receives,
//  # to a persistent message queue that uses berkeleydb backed storage.
//  # The following session events are logged:
//  #   Session Created - INVITE passing authentication was received
//  #   Session Routed - received INVITE was forward to a target
//  #   Session Redirected - session was 3xx redirected or REFERed
//  #   Session Established - there was 2xx answer to an INVITE (only generate for first 2xx)
//  #   Session Cancelled - CANCEL was received
//  #   Session Ended - BYE was received from either end
//  #   Session Error - a 4xx, 5xx, or 6xx response was sent to the inviter
//  # Consuming Accounting Events:
//  # Users must ensure that this message queue is consumed, or it will grow without
//  # bound.  A queuetostream consumer process is provided, that will consume the 
//  # events from the message queue and stream them to stdout.  This output stream can
//  # be consumed by linux scripting tools and converted to database records or some
//  # other relevant representation of the data.  
//  # For example: ./queuetostream ./sessioneventqueue > streamconsumer
//  # In the future a MySQL consumer may also be provided in order to update
//  # session accounting records in a MySQL database table.
//  SessionAccountingEnabled = false
  
  _pReproConfig->setDefaultConfigValue("SessionAccountingEnabled", "false");
  
//
//  # The following setting determines if repro will add routing header information
//  # (ie. Route, and Record-Route headers)to the Session Created, Session Routed
//  # and Session Established events.
//  SessionAccountingAddRoutingHeaders = false
  
  _pReproConfig->setDefaultConfigValue("SessionAccountingAddRoutingHeaders", "false");
  
//
//  # The following setting determines if we will add via header information to
//  # the Session Created event.  
//  SessionAccountingAddViaHeaders = false
  
  _pReproConfig->setDefaultConfigValue("SessionAccountingAddViaHeaders", "false");
  
//
//  # Registration Accounting - When enabled resiprocate will push a JSON formatted 
//  # events for every registration, re-registration, and unregistration message
//  # received to a persistent message queue that uses berkeleydb backed storage.
//  # The following registration events are logged:
//  #   Registration Added - initial registration received
//  #   Registration Refreshed - registration refresh received / re-register
//  #   Registration Removed - registration removed by client / unregister
//  #   Registration Removed All - all contacts registration remove / unregister
//  # Consuming Accounting Events:
//  # Users must ensure that this message queue is consumed, or it will grow without
//  # bound.  A queuetostream consumer process is provided, that will consume the 
//  # events from the message queue and stream them to stdout.  This output stream can
//  # be consumed by linux scripting tools and converted to database records or some
//  # other relevant representation of the data.  
//  # For example: ./queuetostream ./regeventqueue > streamconsumer
//  # In the future a MySQL consumer may also be provided in order to update 
//  # login/registration accounting records in a MySQL database table.
//  RegistrationAccountingEnabled = false
  
  _pReproConfig->setDefaultConfigValue("RegistrationAccountingEnabled", "false");
  
//
//  # The following setting determines if repro will add routing header information
//  # (ie. Route and Path headers)to registration accounting events.
//  RegistrationAccountingAddRoutingHeaders = false
  
  _pReproConfig->setDefaultConfigValue("RegistrationAccountingAddRoutingHeaders", "false");
  
//
//  # The following setting determines if we will add via header information to
//  # the registration accounting events.
//  RegistrationAccountingAddViaHeaders = false
  
  _pReproConfig->setDefaultConfigValue("RegistrationAccountingAddViaHeaders", "false");
  
//
//  # The following setting determines if we log the RegistrationRefreshed events
//  RegistrationAccountingLogRefreshes = false
  
  _pReproConfig->setDefaultConfigValue("RegistrationAccountingLogRefreshes", "false");
  
//
//  # Run a Certificate Server - Allows PUBLISH and SUBSCRIBE for certificates
//  EnableCertServer = false
  
  _pReproConfig->setDefaultConfigValue("EnableCertServer", "false");
  
//
//  # Value of server header for local UAS responses
//  ServerText =
  
  _pReproConfig->setDefaultConfigValue("ServerText", "");
  
//
//  # Enables Congestion Management
//  CongestionManagement = true
  
  _pReproConfig->setDefaultConfigValue("CongestionManagement", "true");
  
//
//  # Congestion Management Metric - can take one of the following values:
//  # SIZE : Based solely on the number of messages in each fifo
//  # TIME_DEPTH : Based on the age of the oldest (front-most) message 
//  #              in each fifo.
//  # WAIT_TIME : Based on the expected wait time for each fifo; this is 
//  #             calculated by multiplying the size by the average service time. 
//  #             This is the recommended metric.
//  CongestionManagementMetric = WAIT_TIME
  
  _pReproConfig->setDefaultConfigValue("CongestionManagementMetric", "WAIT_TIME");
  
//
//  # Congestion Management Tolerance for the given metric.  This determines when the RejectionBehavior 
//  # changes.
//  # 0-80 percent of max tolerance -> NORMAL (Not rejecting any work.)
//  # 80-100 percent of max tolerance -> REJECTING_NEW_WORK (Refuses new work, 
//  #        not continuation of old work.)
//  # >100 percent of max tolerance -> REJECTING_NON_ESSENTIAL (Rejecting all work 
//  #      that is non-essential to the health of the system (ie, if dropping 
//  #      something is liable to cause a leak, instability, or state-bloat, don't drop it. 
//  #      Otherwise, reject it.)
//  # Units specified are dependent on Metric specified above:
//  #  If Metric is SIZE then units are number of messages
//  #  If Metric is TIME_DEPTH then units are the number seconds old the oldest message is
//  #  If Metric is WAIT_TIME then units are the expected wait time of each fifo in milliseconds
//  CongestionManagementTolerance = 200
  
  _pReproConfig->setDefaultConfigValue("CongestionManagementTolerance", "200");
  
//
//  # Specify the number of seconds between writes of the stack statistics block to the log files.
//  # Specifying 0 will disable the statistics collection entirely.  If disabled the statistics
//  # also cannot be retreived using the reprocmd interface.
//  StatisticsLogInterval = 3600
  
  _pReproConfig->setDefaultConfigValue("StatisticsLogInterval", "3600");
  
//
//  # Use MultipleThreads stack processing.
//  ThreadedStack = true
  
  _pReproConfig->setDefaultConfigValue("ThreadedStack", "true");
  
//
//  # The number of worker threads used to asynchronously retrieve user authentication information
//  # from the database store.
//  NumAuthGrabberWorkerThreads = 2
  
  _pReproConfig->setDefaultConfigValue("NumAuthGrabberWorkerThreads", "2");
  
//
//  # The number of worker threads in Async Processor tread pool.  Used by all Async Processors
//  # (ie. RequestFilter)
//  NumAsyncProcessorWorkerThreads = 2
  
  _pReproConfig->setDefaultConfigValue("NumAsyncProcessorWorkerThreads", "2");
  
//
//  # Specify domains for which this proxy is authorative (in addition to those specified on web 
//  # interface) - comma separate list
//  # Notes: * Domains specified here cannot be used when creating users, domains used in user
//  #          AORs must be specified on the web interface.
//  #        * In previous versions of repro, localhost, 127.0.0.1, the machine's hostname,
//  #          and all interface addresses would automatically be appended to this
//  #          configuration parameter.  From now on, such values must be listed
//  #          here explicitly if required, e.g.
//  #
//  #             Domains = localhost, 127.0.0.1, sip-server.example.org, 10.83.73.80
//  #
//  #          although when using TLS only, it is not desirable or necessary to
//  #          add such values.
//  #
//  Domains =
  
  _pReproConfig->setDefaultConfigValue("Domains", "");
  
//
//  # Uri to use as Record-Route
//  RecordRouteUri =
  
  _pReproConfig->setDefaultConfigValue("RecordRouteUri", "");
  
//
//  # Force record-routing
//  # WARNING: Before enabling this, ensure you have a RecordRouteUri setup, or are using
//  # the alternate transport specification mechanism and defining a RecordRouteUri per
//  # transport: TransportXRecordRouteUri
//  ForceRecordRouting = false
  
  _pReproConfig->setDefaultConfigValue("ForceRecordRouting", "false");
  
//
//  # Assume path option
//  AssumePath = false
  
  _pReproConfig->setDefaultConfigValue("AssumePath", "false");
  
//
//  # Disable registrar
//  DisableRegistrar = false
  
  _pReproConfig->setDefaultConfigValue("DisableRegistrar", "false");
  
//
//  # Specify a comma separate list of enum suffixes to search for enum dns resolution
//  EnumSuffixes =
  
  _pReproConfig->setDefaultConfigValue("EnumSuffixes", "");
  
//
//  # Specify the target domain(s) for ENUM logic support.  When a dialed SIP URI
//  # is addressed to +number@somedomain,
//  # where somedomain is an element of EnumDomains,
//  # the ENUM logic will be applied for the number
//  # If empty, ENUM is never used
//  EnumDomains = 
  
  _pReproConfig->setDefaultConfigValue("EnumDomains", "");
  
//
//  # Specify length of timer C in sec (0 or negative will disable timer C) - default 180
//  TimerC = 180
  
  _pReproConfig->setDefaultConfigValue("TimerC", "180");
  
//
//  # Override the default value of T1 in ms (you probably should not change this) - leave 
//  # as 0 to use default of 500ms)
//  TimerT1 = 0
  
  _pReproConfig->setDefaultConfigValue("TimerT1", "0");
  
//
//  # Disable outbound support (RFC5626)
//  # WARNING: Before enabling this, ensure you have a RecordRouteUri setup, or are using
//  # the alternate transport specification mechanism and defining a RecordRouteUri per
//  # transport: TransportXRecordRouteUri
//  DisableOutbound = true
  
  _pReproConfig->setDefaultConfigValue("DisableOutbound", "true");
  
//
//  # Set the draft version of outbound to support (default: RFC5626)
//  # Other accepted values are the versions of the IETF drafts, before RFC5626 was issued
//  # (ie. 5, 8, etc.)
//  OutboundVersion = 5626
  
  _pReproConfig->setDefaultConfigValue("OutboundVersion", "5626");
  
//
//  # There are cases where the first hop in a particular network supports the concept of outbound
//  # and ensures all messaging for a client is delivered over the same connection used for
//  # registration.  This could be a SBC or other NAT traversal aid router that uses the Path 
//  # header.  However such endpoints may not be 100% compliant with outbound RFC and may not 
//  # include a ;ob parameter in the path header.  This parameter is required in order for repro
//  # to have knowledge that the first hop does support outbound, and it will reject registrations
//  # that appear to be using outboud (ie. instanceId and regId) with a 439 (First Hop Lacks Outbound
//  # Support).  In this case it can be desirable when using repro as the registrar to not reject
//  # REGISTRATION requests that contain an instanceId and regId with a 439.
//  # If this setting is enabled, then repro will assume the first hop supports outbound 
//  # and not return this error.
//  AssumeFirstHopSupportsOutbound = false
  
  _pReproConfig->setDefaultConfigValue("AssumeFirstHopSupportsOutbound", "false");
  
//
//  # Enable use of flow-tokens in non-outbound cases
//  # WARNING: Before enabling this, ensure you have a RecordRouteUri setup, or are using
//  # the alternate transport specification mechanism and defining a RecordRouteUri per
//  # transport: TransportXRecordRouteUri
//  EnableFlowTokens = false
  
  _pReproConfig->setDefaultConfigValue("EnableFlowTokens", "false");
  
//
//  # Enable use of flow-tokens in non-outbound cases for clients detected to be behind a NAT.  
//  # This a more selective flow token hack mode for clients not supporting RFC5626.  The 
//  # original flow token hack (EnableFlowTokens) will use flow tokens on all client requests.  
//  # Possible values are:  DISABLED, ENABLED and PRIVATE_TO_PUBLIC.
//  # WARNING: Before enabling this, ensure you have a RecordRouteUri setup, or are using
//  # the alternate transport specification mechanism and defining a RecordRouteUri per
//  # transport: TransportXRecordRouteUri
//  ClientNatDetectionMode = DISABLED
  
  _pReproConfig->setDefaultConfigValue("ClientNatDetectionMode", "DISABLED");
  
//
//  # Set to greater than 0 to enable addition of Flow-Timer header to REGISTER responses if 
//  # outbound is enabled (default: 0)
//  FlowTimer = 0
  
  _pReproConfig->setDefaultConfigValue("FlowTimer", "0");
  
//
//
//  ########################################################
//  # CertificateAuthenticator Monkey Settings
//  ########################################################
//
//  # Enables certificate authenticator - note you MUST use a TlsTransport
//  # with TlsClientVerification set to Optional or Mandatory.
//  # There are two levels of checking:
//  # a) cert must be signed by a CA trusted by the stack
//  # b) the CN or one of the subjectAltName values must match the From:
//  #    header of each SIP message on the TlsConnection
//  # Examples:
//  # Cert 1:
//  #    common name = daniel@pocock.com.au
//  #    => From: <daniel@pocock.com.au> is the only value that will pass
//  # Cert 2:
//  #    subjectAltName = pocock.com.au
//  #    => From: <<anything>@pocock.com.au> will be accepted
//  # Typically, case 1 is for a real client connection (e.g. Jitsi), case 2
//  # (whole domain) is for federated SIP proxy-to-proxy communication (RFC 5922)
//  EnableCertificateAuthenticator = false
  
  _pReproConfig->setDefaultConfigValue("EnableCertificateAuthenticator", "false");
  
//
//  # A static text file that contains mappings of X.509 Common Names to
//  # permitted SIP `From:' addresses
//  #
//  # Without this file, the default behavior of the CertificateAuthenticator
//  # ensures that the `From:' address in SIP messages must match the 
//  # Common Name or one of the subjectAltNames from the X.509 certificate
//  #
//  # When this file is supplied, the CertificateAuthenticator will continue
//  # to allow SIP messages where there is an exact match between the
//  # certificate and the `From:' address, but it will also allow
//  # the holder of a particular certificate to use any of the `mapped'
//  # `From:' addresses specified in the mappings file
//  #
//  # Default: there is no default value: if this filename is not specified,
//  #          repro will not look for it
//  #
//  # File format:
//  # common name<TAB><mapping>,<mapping>,...
//  #
//  #    where:
//  #        <TAB> is exactly one tab
//  #        <mapping> is `user@domain' or just `domain'
//  #
//  #CommonNameMappings = /etc/repro/tlsUserMappings.txt
//
//
//  ########################################################
//  # DigestAuthenticator Monkey Settings
//  ########################################################
//
//  # Disable DIGEST challenges - disables this monkey
//  DisableAuth = false
  
  _pReproConfig->setDefaultConfigValue("DisableAuth", "false");
  
//
//  # Http hostname for this server (used in Identity headers)
//  HttpHostname =
  
  _pReproConfig->setDefaultConfigValue("HttpHostname", "");
  
//
//  # Disable adding identity headers
//  DisableIdentity = false
  
  _pReproConfig->setDefaultConfigValue("DisableIdentity", "false");
  
//
//  # Enable addition and processing of P-Asserted-Identity headers
//  EnablePAssertedIdentityProcessing = false
  
  _pReproConfig->setDefaultConfigValue("EnablePAssertedIdentityProcessing", "false");
  
//
//  # Disable auth-int DIGEST challenges
//  DisableAuthInt = false
  
  _pReproConfig->setDefaultConfigValue("DisableAuthInt", "false");
  
//
//  # Send 403 if a client sends a bad nonce in their credentials (will send a new 
//  # challenge otherwise)
//  RejectBadNonces = false
  
  _pReproConfig->setDefaultConfigValue("RejectBadNonces", "false");
  
//
//  # allow To tag in registrations
//  AllowBadReg = false
  
  _pReproConfig->setDefaultConfigValue("AllowBadReg", "false");
  
//
//  ########################################################
//  # Cookie Authentication Settings
//  ########################################################
//
//  # Shared secret for cookie HMAC validation. If there is no WSCookieAuthSharedSecret
//  # there will be no cookie validation.
//  # WSCookieAuthSharedSecret =
  
  _pReproConfig->setDefaultConfigValue("WSCookieAuthSharedSecret", "");
  
//
//  ########################################################
//  # RequestFilter Monkey Settings
//  ########################################################
//
//  # Disable RequestFilter monkey processing
//  DisableRequestFilterProcessor = false
  
  _pReproConfig->setDefaultConfigValue("DisableRequestFilterProcessor", "false");
  
//
//  # Default behavior for when no matching filter is found.  Leave empty to allow 
//  # request processing to continue.  Otherwise set to a SIP status error code
//  # (400-699) that should be used to reject the request (ie. 500, Server Internal
//  # Error).
//  # The status code can optionally be followed by a , and SIP reason text.
//  RequestFilterDefaultNoMatchBehavior =
  
  _pReproConfig->setDefaultConfigValue("RequestFilterDefaultNoMatchBehavior", "");
  
//
//  # Default behavior for SQL Query db errors.  Leave empty to allow request processing
//  # to continue.  Otherwise set to a SIP status error code (400-699) that should be  
//  # used to reject the request (ie. 500 - Server Internal Error).
//  # The status code can optionally be followed by a , and SIP reason text.
//  # Note: DB support for this action requires MySQL support.
//  RequestFilterDefaultDBErrorBehavior = 500, Server Internal DB Error
  
  _pReproConfig->setDefaultConfigValue("RequestFilterDefaultDBErrorBehavior", "500, Server Internal DB Error");
  
//
//  # The hostname running MySQL server to connect to for any blocked entries
//  # that are configured to used a SQL statement.
//  # WARNING: repro must be compiled with the USE_MYSQL flag in order for this work.
//  #
//  # Note:  If this setting is left blank then repro will fallback all remaining my sql
//  # settings to use the global RuntimeMySQLServer or MySQLServer settings.  See the 
//  # documentation on the global MySQLServer settings for more details on the following 
//  # individual settings.
//  RequestFilterMySQLServer =
  
  _pReproConfig->setDefaultConfigValue("RequestFilterMySQLServer", "");
  
//  RequestFilterMySQLUser = root
  
  _pReproConfig->setDefaultConfigValue("RequestFilterMySQLUser", "root");
  
//  RequestFilterMySQLPassword = root
  
  _pReproConfig->setDefaultConfigValue("RequestFilterMySQLPassword", "root");
  
//  RequestFilterMySQLDatabaseName = 
  
  _pReproConfig->setDefaultConfigValue("RequestFilterMySQLDatabaseName", "");
  
//  RequestFilterMySQLPort = 3306
  
  _pReproConfig->setDefaultConfigValue("RequestFilterMySQLPort", "3306");
  
//
//
//  ########################################################
//  # StaticRoute Monkey Settings
//  ########################################################
//
//  # Specify where to route requests that are in this proxy's domain - disables the 
//  # routes in the web interface and uses a SimpleStaticRoute monkey instead.
//  # A comma seperated list of routes can be specified here and each route will
//  # be added to the outbound Requests with the RequestUri left in tact.
//  Routes =
  
  _pReproConfig->setDefaultConfigValue("Routes", "");
  
//
//  # Parallel fork to all matching static routes
//  ParallelForkStaticRoutes = false
  
  _pReproConfig->setDefaultConfigValue("ParallelForkStaticRoutes", "false");
  
//
//  # By default (false) we will stop looking for more Targets if we have found
//  # matching routes.  Setting this value to true will allow the LocationServer Monkey
//  # to run after StaticRoutes have been found.  In this case the matching
//  # StaticRoutes become fallback targets, processed only after all location server 
//  # targets fail.
//  ContinueProcessingAfterRoutesFound = false
  
  _pReproConfig->setDefaultConfigValue("ContinueProcessingAfterRoutesFound", "false");
  
//
//
//  ########################################################
//  # Message Silo Monkey Settings
//  ########################################################
//
//  # Specify where the Message Silo is enabled or not.  If enabled,
//  # then repro will store MESSAGE requests for users that are not online.
//  # When the user is back online (ie. registers with repro), the stored 
//  # messages will be delivered.
//  MessageSiloEnabled = false
  
  _pReproConfig->setDefaultConfigValue("MessageSiloEnabled", "false");
  
//
//  # A regular expression that can be used to filter which URI's not to
//  # do message storage (siloing) for.  Destination/To URI's matching
//  # this regular expression will not be silo'd.
//  MessageSiloDestFilterRegex =
  
  _pReproConfig->setDefaultConfigValue("MessageSiloDestFilterRegex", "");
  
//
//  # A regular expression that can be used to filter which body/content/mime
//  # types not to do message storage (siloing) for.  Content-Type's matching
//  # this regular expression will not be silo'd.
//  MessageSiloMimeTypeFilterRegex = application\/im\-iscomposing\+xml
  
  _pReproConfig->setDefaultConfigValue("MessageSiloMimeTypeFilterRegex", "application\\/im\\-iscomposing\\+xml");
  
//
//  # The number of seconds a message request will be stored in the message silo.
//  # Messages older than this time, are candidates for deletion.  
//  # Default (259200 seconds = 30 days)
//  MessageSiloExpirationTime = 2592000
  
  _pReproConfig->setDefaultConfigValue("MessageSiloExpirationTime", "2592000");
  
//
//  # Flag to indicate if a Date header should be added to replayed SIP 
//  # MESSAGEs from the silo, when a user registers.
//  MessageSiloAddDateHeader = true
  
  _pReproConfig->setDefaultConfigValue("MessageSiloAddDateHeader", "true");
  
//
//  # Defines the maximum message content length (bytes) that will be stored in
//  # the message silo.  Messages with a Content-Length larger than this 
//  # value will be discarded.
//  # WARNING:  Do not increasing this value beyond the capabilities of the
//  # database storage or internal buffers.
//  # Note: AbstractDb uses a read buffer size of 8192 - do not exceed this size.
//  MessageSiloMaxContentLength = 4096
  
  _pReproConfig->setDefaultConfigValue("MessageSiloMaxContentLength", "4096");
  
//
//  # The status code returned to the sender when a messages is successfully
//  # silo'd.
//  MessageSiloSuccessStatusCode = 202
  
  _pReproConfig->setDefaultConfigValue("MessageSiloSuccessStatusCode", "202");
  
//
//  # The status code returned to the sender when a messages mime-type matches
//  # the MessageSiloMimeTypeFilterRegex.  Can be used to avoid sending errors
//  # to isComposing mime bodies that don't need to be silod.  Set to 0 to use
//  # repro standard response (ie. 480).
//  MessageSiloFilteredMimeTypeStatusCode = 200
  
  _pReproConfig->setDefaultConfigValue("MessageSiloFilteredMimeTypeStatusCode", "200");
  
//
//  # The status code returned to the sender when a messages is not silo'd due
//  # to the MaxContentLength being exceeded.
//  MessageSiloFailureStatusCode = 480
  
  _pReproConfig->setDefaultConfigValue("MessageSiloFailureStatusCode", "480");
  
//
//
//  ########################################################
//  # Recursive Redirect Lemur Settings
//  ########################################################
//
//  # Handle 3xx responses in the proxy - enables the Recursive Redirect Lemur
//  RecursiveRedirect = false
  
  _pReproConfig->setDefaultConfigValue("RecursiveRedirect", "false");
  
//
//
//  ########################################################
//  # Geo Proximity Target Sorter Baboon Settings
//  ########################################################
//
//  # If enabled, then this baboon can post-process the target list.  
//  # This includes targets from the StaticRoute monkey and/or targets
//  # from the LocationServer monkey.  Requests that meet the filter 
//  # criteria will have their Target list, flatened (serialized) and
//  # ordered based on the proximity of the target to the client sending
//  # the request.  Proximity is determined by looking for a 
//  # x-repro-geolocation="<latitude>,<longitude>" parameter on the Contact
//  # header of a received request, or the Contact headers of Registration
//  # requests.  If this parameter is not found, then this processor will
//  # attempt to determine the public IP address closest to the client or
//  # target and use the MaxMind Geo IP library to lookup the geo location.
//  GeoProximityTargetSorting = false
  
  _pReproConfig->setDefaultConfigValue("GeoProximityTargetSorting", "false");
  
//
//  # Specify the full path to the IPv4 Geo City database file
//  # Note:  A free version of the database can be downloaded from here:
//  # http://geolite.maxmind.com/download/geoip/database/GeoLiteCity.dat.gz
//  # For a more accurate database, please see the details here:
//  # http://www.maxmind.com/app/city
//  GeoProximityIPv4CityDatabaseFile = GeoLiteCity.dat
  
  _pReproConfig->setDefaultConfigValue("GeoProximityIPv4CityDatabaseFile", "GeoLiteCity.dat");
  
//
//  # Specify the full path to the IPv6 Geo City database file
//  # Note:  A free version of the database can be downloaded from here:
//  # http://geolite.maxmind.com/download/geoip/database/GeoLiteCityv6-beta/
//  # For a more accurate database, please see the details here:
//  # http://www.maxmind.com/app/city
//  # Leave blank to disable V6 lookups.  Saves memory (if not required).
//  #GeoProximityIPv6CityDatabaseFile = GeoLiteCityv6.dat
//  GeoProximityIPv6CityDatabaseFile =
  
  _pReproConfig->setDefaultConfigValue("GeoProximityIPv6CityDatabaseFile", "");
  
//
//  # This setting specifies a PCRE compliant regular expression to attempt
//  # to match against the request URI of inbound requests.  Any requests
//  # matching this expression, will have their targets sorted as described
//  # above.  Leave blank to match all requests.
//  GeoProximityRequestUriFilter = ^sip:mediaserver.*@mydomain.com$
  
  _pReproConfig->setDefaultConfigValue("GeoProximityRequestUriFilter", "^sip:mediaserver.*@mydomain.com$");
  
//
//  # The distance (in Kilometers) to use for proximity sorting, when the 
//  # Geo Location of a target cannot be determined. 
//  GeoProximityDefaultDistance = 0
  
  _pReproConfig->setDefaultConfigValue("GeoProximityDefaultDistance", "0");
  
//
//  # If enabled, then targets that are determined to be of equal distance
//  # from the client, will be placed in a random order.
//  LoadBalanceEqualDistantTargets = true
  
  _pReproConfig->setDefaultConfigValue("LoadBalanceEqualDistantTargets", "true");
  
//
//
//  ########################################################
//  # Q-Value Target Handler Baboon Settings
//  ########################################################
//
//  # Enable sequential q-value processing - enables the Baboon
//  QValue = true
  
  _pReproConfig->setDefaultConfigValue("QValue", "true");
  
//
//  # Specify forking behavior for q-value targets: FULL_SEQUENTIAL, EQUAL_Q_PARALLEL, 
//  # or FULL_PARALLEL
//  QValueBehavior = EQUAL_Q_PARALLEL
  
  _pReproConfig->setDefaultConfigValue("QValueBehavior", "EQUAL_Q_PARALLEL");
  
//
//  # Whether to cancel groups of parallel forks after the period specified by the 
//  # QValueMsBeforeCancel parameter.
//  QValueCancelBetweenForkGroups = true
  
  _pReproConfig->setDefaultConfigValue("QValueCancelBetweenForkGroups", "true");
  
//
//  # msec to wait before cancelling parallel fork groups when QValueCancelBetweenForkGroups
//  # is true
//  QValueMsBeforeCancel = 30000
  
  _pReproConfig->setDefaultConfigValue("QValueMsBeforeCancel", "30000");
  
//
//  # Whether to wait for parallel fork groups to terminate before starting new fork-groups.
//  QValueWaitForTerminateBetweenForkGroups = true
  
  _pReproConfig->setDefaultConfigValue("QValueWaitForTerminateBetweenForkGroups", "true");
  
//
//  # msec to wait before starting new groups of parallel forks when 
//  # QValueWaitForTerminateBetweenForkGroups is false
//  QValueMsBetweenForkGroups = 3000
  
  _pReproConfig->setDefaultConfigValue("QValueMsBetweenForkGroups", "300");
}

bool ReproGlue::run()
{
   if(mRunning) return false;

   try
   {
      configureRepro();
   }
   catch(BaseException& ex)
   {
      std::cerr << "Error parsing configuration: " << ex << std::endl;
      return false;
   }

   //
   // Initialize the logger
   //
   _logger.initialize();
               
   InfoLog( << "Starting repro version " << VersionUtils::instance().releaseVersion() << "...");

   // Create SipStack and associated objects
   if(!createSipStack())
   {
      return false;
   }

   // Drop privileges (can do this now that sockets are bound)
   //Data runAsUser = mProxyConfig->getConfigData("RunAsUser", "", true);
   //Data runAsGroup = mProxyConfig->getConfigData("RunAsGroup", "", true); 
   //if(!runAsUser.empty())
   //{
   //   InfoLog( << "Trying to drop privileges, configured uid = " << runAsUser << " gid = " << runAsGroup);
   //   dropPrivileges(runAsUser, runAsGroup);
   //}


   // Create DialogUsageManager that handles ServerRegistration,
   // and potentially certificate subscription server
   createDialogUsageManager();

   // Create the Proxy and associate objects
   if(!createProxy())
   {
      return false;
   }

   // Create HTTP WebAdmin and Thread
   if(_enableWebAdmin && !createWebAdmin())
   {
      return false;
   }
   
   if (!_enableWebAdmin)
   {
     //
     // Simply create an empty list of webadmins so cleanup won't complain
     //
     mWebAdminList = new std::list<WebAdmin*>;
   }

   // Create reg sync components if required
   createRegSync();

   // Create command server if required
   if(!mRestarting)
   {
      createCommandServer();
   }

   // Make it all go - startup all threads
   mThreadedStack = mProxyConfig->getConfigBool("ThreadedStack", true);
   if(mThreadedStack)
   {
      // If configured, then start the sub-threads within the stack
      mSipStack->run();
   }
   mStackThread->run();
   if(mDumThread)
   {
      mDumThread->run();
   }
   mProxy->run();
   if(_enableWebAdmin && mWebAdminThread)
   {
      mWebAdminThread->run();
   }
   if(!mRestarting && mCommandServerThread)
   {
      mCommandServerThread->run();
   }
   if(mRegSyncServerThread)
   {
      mRegSyncServerThread->run();
   }
   if(mRegSyncClient)
   {
      mRegSyncClient->run();
   }

   mRunning = true;

   return true;
}


void ReproGlue::makeRequestProcessorChain(ProcessorChain& chain)
{
   assert(mProxyConfig);
   assert(mRegistrationPersistenceManager);

   // Add strict route fixup monkey
   addProcessor(chain, std::auto_ptr<Processor>(new StrictRouteFixup));

   // Add is trusted node monkey
   addProcessor(chain, std::auto_ptr<Processor>(new IsTrustedNode(*mProxyConfig)));

   // Add Certificate Authenticator - if required
   if(mProxyConfig->getConfigBool("EnableCertificateAuthenticator", false))
   {
      // TODO: perhaps this should be initialised from the trusted node
      // monkey?  Or should the list of trusted TLS peers be independent
      // from the trusted node list?
      // Should we used the same trustedPeers object that was
      // passed to TlsPeerAuthManager perhaps?
      std::set<Data> trustedPeers;
      loadCommonNameMappings();
      addProcessor(chain, std::auto_ptr<Processor>(new CertificateAuthenticator(*mProxyConfig, mSipStack, trustedPeers, true, mCommonNameMappings)));
   }

   Data wsCookieAuthSharedSecret = mProxyConfig->getConfigData("WSCookieAuthSharedSecret", "");
   if(mSipAuthDisabled && !wsCookieAuthSharedSecret.empty())
   {
      addProcessor(chain, std::auto_ptr<Processor>(new CookieAuthenticator(wsCookieAuthSharedSecret, mSipStack)));
   }

   // Add digest authenticator monkey - if required
   if (!mSipAuthDisabled)
   {
      assert(mAuthRequestDispatcher);
      DigestAuthenticator* da = new DigestAuthenticator(*mProxyConfig, mAuthRequestDispatcher);

      addProcessor(chain, std::auto_ptr<Processor>(da)); 
   }

#if 0
   // Add am I responsible monkey
   addProcessor(chain, std::auto_ptr<Processor>(new AmIResponsible)); 
#endif
   
   // Add RequestFilter monkey
   if(!mProxyConfig->getConfigBool("DisableRequestFilterProcessor", false))
   {
      if(mAsyncProcessorDispatcher)
      {
         addProcessor(chain, std::auto_ptr<Processor>(new RequestFilter(*mProxyConfig, mAsyncProcessorDispatcher)));
      }
      else
      {
         WarningLog(<< "Could not start RequestFilter Processor due to no worker thread pool (NumAsyncProcessorWorkerThreads=0)");
      }
   }

   // [TODO] support for GRUU is on roadmap.  When it is added the GruuMonkey will go here
      
   // [TODO] support for Manipulating Tel URIs is on the roadmap.
   //        When added, the telUriMonkey will go here 

   //
   // Insert the external static router if it is set and bypass the default router
   //
   //if (_pStaticRouter)
   //{
   //  addProcessor(chain, std::auto_ptr<Processor>(_pStaticRouter));
   //}
   //else
   
   //
   // Add the custom static router
   //
   addProcessor(chain, std::auto_ptr<Processor>(_pStaticRouter));

   // Add location server monkey
   addProcessor(chain, std::auto_ptr<Processor>(new LocationServer(*mProxyConfig, *mRegistrationPersistenceManager, mAuthRequestDispatcher)));

   // Add message silo monkey
   if(mProxyConfig->getConfigBool("MessageSiloEnabled", false))
   {
      if(mAsyncProcessorDispatcher && mRegistrar)
      {
         MessageSilo* silo = new MessageSilo(*mProxyConfig, mAsyncProcessorDispatcher);
         mRegistrar->addRegistrarHandler(silo);
         addProcessor(chain, std::auto_ptr<Processor>(silo));
      }
      else
      {
         WarningLog(<< "Could not start MessageSilo Processor due to no worker thread pool (NumAsyncProcessorWorkerThreads=0) or Registrar");
      }
   }
}

void ReproGlue::makeResponseProcessorChain(ProcessorChain& chain)
{  
  ReproRunner::makeResponseProcessorChain(chain);
  addProcessor(chain, std::auto_ptr<Processor>(_pResponseProcessor));
}

void ReproGlue::makeTargetProcessorChain(ProcessorChain& chain)
{
  ReproRunner::makeTargetProcessorChain(chain);
  addProcessor(chain, std::auto_ptr<Processor>(_pTargetProcessor));
}

bool ReproGlue::addTrustedNode(const std::string& address, const short& port, const short& transport)
{
  Data addr(address.c_str());
  return mProxyConfig->getDataStore()->mAclStore.addAcl(addr, port, transport);
}

bool ReproGlue::addUser( const std::string& user_, 
                const std::string& domain_, 
                const std::string& realm_, 
                const std::string& password_, 
                const std::string& fullName_,
                const std::string& emailAddress_ )
{
  Data user(user_.c_str());
  Data domain(domain_.c_str());
  Data realm(realm_.c_str());
  Data password(password_.c_str());
  Data fullName(fullName_.c_str());
  Data emailAddress(emailAddress_.c_str());
  UserStore& userStore = mProxyConfig->getDataStore()->mUserStore;
  //
  // Check if this user is already in the user store
  //
  Data authInfo;
  authInfo = userStore.getUserAuthInfo(user, realm);
  if (authInfo.empty())
  {
    //
    // New user
    //
    return userStore.addUser(user, domain, realm, password, true, fullName, emailAddress);
  }
  else
  {
    //
    // This is an update
    //
    Data key = user + Data("@") + realm;
    return userStore.updateUser(key, user, domain, realm, password, true, fullName, emailAddress);
  }
}


  
void ReproGlue::addTransport(
  const std::string& transport, 
  const std::string& hostPort, 
  bool recordRoute)
{
  assert(!transport.empty());
  assert(transport == "UDP" || transport == "TCP" || transport == "WS");
  assert(!hostPort.empty());
  
  std::ostringstream prefix;
  prefix << "Transport" << ++_currentTransportCount; 
  std::string interface = prefix.str() + std::string("Interface");
  std::string type = prefix.str() + std::string("Type");
  std::string recordroute = prefix.str() + std::string("RecordRouteUri");
  
  setProxyConfigValue(interface, hostPort, true);
  setProxyConfigValue(type, transport, true);
  
  if (recordRoute)
  {
    std::ostringstream rruri;
    rruri << "sip:" << hostPort << ";transport=";
    if (transport == "UDP")
      rruri << "udp";
    else if (transport == "TCP")
      rruri << "tcp";
    else if (transport == "WS")
      rruri << "ws";
    
    InfoLog(<< "Setting record route to " << recordroute << "=" << rruri.str());
    
    setProxyConfigValue(recordroute, "auto", true);
  }
}

void ReproGlue::addSecureTransport(
  const std::string& transport, 
  const std::string& hostPort,
  const std::string& tlsDomain,
  const std::string& tlsCertificate,
  const std::string& tlsPrivateKey,
  const std::string& tlsClientVerification,
  bool recordRoute)
{
  assert(!transport.empty());
  assert(transport == "TLS" || transport == "DTLS" || transport == "WSS");
  assert(!hostPort.empty());
  
  std::ostringstream prefix;
  prefix << "Transport" << ++_currentTransportCount; 
  std::string interface = prefix.str() + std::string("Interface");
  std::string type = prefix.str() + std::string("Type");
  std::string recordroute = prefix.str() + std::string("RecordRouteUri");
  std::string tlsdomain = prefix.str() + std::string("TlsDomain");
  std::string tlscertificate = prefix.str() + std::string("TlsCertificate");
  std::string tlsprivatekey = prefix.str() + std::string("TlsPrivateKey");
  std::string tlsclientverification = prefix.str() + std::string("TlsPrivateKey");
  
  setProxyConfigValue(interface, hostPort, true);
  setProxyConfigValue(type, transport, true);
  setProxyConfigValue(tlsdomain, tlsDomain, true);
  setProxyConfigValue(tlscertificate, tlsCertificate, true);
  setProxyConfigValue(tlsprivatekey, tlsPrivateKey, true);
  setProxyConfigValue(tlsclientverification, tlsClientVerification, true);
  
  if (recordRoute)
    setProxyConfigValue(recordroute, "auto", true);
}

  
void ReproGlue::removeUser(const std::string& user, const std::string& realm)
{
  Data key = Data(user.c_str()) + Data("@") + Data(realm.c_str());
  UserStore& userStore = mProxyConfig->getDataStore()->mUserStore;
  userStore.eraseUser(key);
}

bool ReproGlue::addDomain(const std::string& domain_, const int port)
{
  Data domain(domain_.c_str());
  ConfigStore& configStore = mProxyConfig->getDataStore()->mConfigStore;
  return configStore.addDomain(domain, port);
}
      
void ReproGlue::removeDomain(const std::string& domain_)
{
  Data domain(domain_.c_str());
  ConfigStore& configStore = mProxyConfig->getDataStore()->mConfigStore;
  configStore.eraseDomain(domain);
}
  
bool ReproGlue::createDatastore()
{
   // Create Database access objects
   assert(!mAbstractDb);
   assert(!mRuntimeAbstractDb);
#ifdef USE_MYSQL
   Data mySQLServer;
   mProxyConfig->getConfigValue("MySQLServer", mySQLServer);
   if(!mySQLServer.empty())
   {
      mAbstractDb = new MySqlDb(mySQLServer, 
                       mProxyConfig->getConfigData("MySQLUser", ""), 
                       mProxyConfig->getConfigData("MySQLPassword", ""),
                       mProxyConfig->getConfigData("MySQLDatabaseName", ""),
                       mProxyConfig->getConfigUnsignedLong("MySQLPort", 0),
                       mProxyConfig->getConfigData("MySQLCustomUserAuthQuery", ""));
   }
   Data runtimeMySQLServer;
   mProxyConfig->getConfigValue("RuntimeMySQLServer", runtimeMySQLServer);
   if(!runtimeMySQLServer.empty())
   {
      mRuntimeAbstractDb = new MySqlDb(runtimeMySQLServer,
                       mProxyConfig->getConfigData("RuntimeMySQLUser", ""), 
                       mProxyConfig->getConfigData("RuntimeMySQLPassword", ""),
                       mProxyConfig->getConfigData("RuntimeMySQLDatabaseName", ""),
                       mProxyConfig->getConfigUnsignedLong("RuntimeMySQLPort", 0),
                       mProxyConfig->getConfigData("MySQLCustomUserAuthQuery", ""));
   }
#endif
   if (!mAbstractDb)
   {
      Data dbPrefix(_applicationName.c_str());
      mAbstractDb = new BerkeleyDb(mProxyConfig->getConfigData("DatabasePath", "./", true), dbPrefix);
   }
   assert(mAbstractDb);
   if(!mAbstractDb->isSane())
   {
      CritLog(<<"Failed to open configuration database");
      cleanupObjects();
      return false;
   }
   if(mRuntimeAbstractDb && !mRuntimeAbstractDb->isSane())
   {
      CritLog(<<"Failed to open runtime configuration database");
      cleanupObjects();
      return false;
   }
   mProxyConfig->createDataStore(mAbstractDb, mRuntimeAbstractDb);

   // Create ImMemory Registration Database
   mRegSyncPort = mProxyConfig->getConfigInt("RegSyncPort", 0);
   // We only need removed records to linger if we have reg sync enabled
   if(!mRestarting)  // If we are restarting then we left the InMemoryRegistrationDb intact at shutdown - don't recreate
   {
      assert(!mRegistrationPersistenceManager);
      mRegistrationPersistenceManager = new InMemorySyncRegDb(mRegSyncPort ? 86400 /* 24 hours */ : 0 /* removeLingerSecs */);  // !slg! could make linger time a setting
   }
   assert(mRegistrationPersistenceManager);

   // Copy contacts from the StaticRegStore to the RegistrationPersistanceManager
   populateRegistrations();

   return true;
}

bool ReproGlue::getRequestSourceAddress(RequestContext& context, std::string& address, unsigned short& port)
{
  resip::SipMessage& request = context.getOriginalRequest();
  resip::Tuple source = request.getSource();
  address = resip::Tuple::inet_ntop(source).c_str();
  port = source.getPort();
  return !address.empty();
}

bool ReproGlue::getRequestViaAddress(RequestContext& context, std::string& transport, std::string& address, unsigned short& port)
{
  resip::SipMessage& request = context.getOriginalRequest();
  
  if (!request.exists(h_Vias) || request.header(h_Vias).empty())
    return false;
  
  resip::Via& via = request.header(h_Vias).front();
  if (!via.isWellFormed())
    return false;
  
  address = via.sentHost().c_str();
  port = via.sentPort();
  transport = via.transport().c_str();
  return true;
}


} } // sipx::proxy



