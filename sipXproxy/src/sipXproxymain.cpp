// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$

#include <stdio.h>
#include <signal.h>
#if defined(__pingtel_on_posix__)
#include <unistd.h>
#endif
#include <iostream>
#include <boost/exception/all.hpp>
#include <sipxproxy/SipRouter.h>
#include <os/OsFS.h>
#include <os/OsServiceOptions.h>
#include <os/OsSocket.h>
#include <os/OsTask.h>
#include <os/OsTimer.h>
#include <os/OsStunAgentTask.h>
#include <os/OsLogger.h>
#include <os/OsLoggerHelper.h>
#include <os/UnixSignals.h>
#include <os/OsMsgQ.h>
#include <net/SipMessage.h>
#include <net/SipUserAgent.h>
#include <net/NameValueTokenizer.h>
#include <xmlparser/tinyxml.h>
#include <sipXecsService/SipXecsService.h>
#include <sipXecsService/SharedSecret.h>
#include <sipXecsService/daemon.h>
#include <ForwardRules.h>
#include <SipXProxyCseObserver.h>
#include <utl/Instrumentation.h>
#include <os/OsResourceLimit.h>
#include "config.h"
#include "sipXecsService/SipXApplication.h"

//
// Exception handling
//
#include <stdexcept>
#include <execinfo.h>
#include <mongo/util/assert_util.h>
#include <os/OsExceptionHandler.h>

#define CONFIG_SETTING_CALL_STATE         "SIPX_PROXY_CALL_STATE"
#define CONFIG_SETTING_CALL_STATE_LOG     "SIPX_PROXY_CALL_STATE_LOG"
#define CALL_STATE_LOG_FILE_DEFAULT SIPX_LOGDIR "/sipxproxy_callstate.log"
#define FORWARDING_RULES_FILENAME "forwardingrules.xml"
#define SIPX_PROXY_LOG_FILE "sipXproxy.log"
#define CONFIG_LOG_DIR SIPX_LOGDIR
#define LOG_FACILITY   FAC_SIP

// Configuration names pulled from config-file
#define SIPXPROXY_APP_NAME            "SipXProxy"
#define CONFIG_SETTING_BIND_IP        "SIPX_PROXY_BIND_IP"
#define CONFIG_SETTINGS_FILE          "sipXproxy-config"
#define CONFIG_SETTING_LOG_LEVEL      "SIPX_PROXY_LOG_LEVEL"
#define CONFIG_SETTING_LOG_CONSOLE    "SIPX_PROXY_LOG_CONSOLE"
#define CONFIG_SETTING_LOG_DIR        "SIPX_PROXY_LOG_DIR"
#define CONFIG_SETTING_CALL_STATE     "SIPX_PROXY_CALL_STATE"
#define CONFIG_SETTING_CALL_STATE_LOG "SIPX_PROXY_CALL_STATE_LOG"

// Default expiry times (in seconds)
#define DEFAULT_SIP_TRANSACTION_EXPIRES 180
#define DEFAULT_SIP_SERIAL_EXPIRES 20

static const char* CONFIG_SETTING_CALL_STATE_DB = "SIPX_PROXY_CALL_STATE_DB";
static const char* CONFIG_SETTING_CALL_STATE_DB_HOST = "SIPX_PROXY_CALL_STATE_DB_HOST";
static const char* CONFIG_SETTING_CALL_STATE_DB_NAME = "SIPX_PROXY_CALL_STATE_DB_NAME";
static const char* CONFIG_SETTING_CALL_STATE_DB_USER = "SIPX_PROXY_CALL_STATE_DB_USER";
static const char* CONFIG_SETTING_CALL_STATE_DB_DRIVER = "SIPX_PROXY_CALL_STATE_DB_DRIVER";
static const char* CALL_STATE_DATABASE_HOST = "localhost";
static const char* CALL_STATE_DATABASE_NAME = "SIPXCDR";
static const char* CALL_STATE_DATABASE_USER = POSTGRESQL_USER;
static const char* CALL_STATE_DATABASE_DRIVER = "{PostgreSQL}";
static const char* PROXY_CONFIG_PREFIX = "SIPX_PROXY";

UtlBoolean gShutdownFlag = FALSE;
UtlBoolean gClosingIMDB = FALSE;
OsMutex* gpLockMutex = new OsMutex(OsMutex::Q_FIFO);

int proxy()
{
    int proxyTcpPort;
    int proxyUdpPort;
    int proxyTlsPort;
    UtlString bindIp;
    int maxForwards;    
    UtlString domainName;
    UtlString proxyRecordRoute;
    UtlString routeName;
    UtlString authScheme;    
    UtlString ipAddress;

    // register default exception handler methods
    // exit for mongo tcp related exceptions, core dump for others
    OsExceptionHandler::instance();

    //
    // Raise the file handle limit to maximum allowable
    //
    typedef OsResourceLimit::Limit Limit;
    Limit rescur = 0;
    Limit resmax = 0;
    OsResourceLimit resource;
    if (resource.setApplicationLimits("sipxproxy"))
    {
      resource.getFileDescriptorLimit(rescur, resmax);
      OS_LOG_NOTICE(FAC_KERNEL, "Maximum file descriptors set to " << rescur);
    }
    else
    {
      OS_LOG_ERROR(FAC_KERNEL, "Unable to set file descriptor limit");
    }

    OsServiceOptions& osServiceOptions = SipXApplication::instance().getOsServiceOptions();
    OsSocket::getHostIp(&ipAddress);

    osServiceOptions.getOption(CONFIG_SETTING_BIND_IP, bindIp);

    if ((bindIp.isNull()) || !OsSocket::isIp4Address(bindIp))
    {
       bindIp = "0.0.0.0";
    }
    Os::Logger::instance().log(FAC_SIP, PRI_INFO, "%s: %s", CONFIG_SETTING_BIND_IP, 
          bindIp.data());
    osPrintf("%s: %s", CONFIG_SETTING_BIND_IP, bindIp.data());    

    UtlString hostname;
    osServiceOptions.getOption("SIPX_PROXY_HOST_NAME", hostname);
    if (!hostname.isNull())
    {
       // bias the selection of SRV records so that if the name of this host is an alternative,
       // it wins in any selection based on random weighting.
       SipSrvLookup::setOwnHostname(hostname);
    }
    Os::Logger::instance().log(FAC_SIP, PRI_INFO, "SIPX_PROXY_HOST_NAME : %s", hostname.data());
    
    proxyUdpPort = osServiceOptions.getOsConfigDb().getPort("SIPX_PROXY_UDP_PORT");
    if (!portIsValid(proxyUdpPort))
    {
       proxyUdpPort = 5060;
    }
    Os::Logger::instance().log(FAC_SIP, PRI_INFO, "SIPX_PROXY_UDP_PORT : %d", proxyUdpPort);
    proxyTcpPort = osServiceOptions.getOsConfigDb().getPort("SIPX_PROXY_TCP_PORT") ;
    if (!portIsValid(proxyTcpPort))
    {
       proxyTcpPort = 5060;
    }
    Os::Logger::instance().log(FAC_SIP, PRI_INFO, "SIPX_PROXY_TCP_PORT : %d", proxyTcpPort);
    proxyTlsPort = osServiceOptions.getOsConfigDb().getPort("SIPX_PROXY_TLS_PORT") ;
    if (!portIsValid(proxyTlsPort))
    {
       proxyTlsPort = 5061;
    }
    Os::Logger::instance().log(FAC_SIP, PRI_INFO, "SIPX_PROXY_TLS_PORT : %d", proxyTlsPort);

    osServiceOptions.getOption("SIPX_PROXY_MAX_FORWARDS", maxForwards);
    if(maxForwards <= 0) maxForwards = SIP_DEFAULT_MAX_FORWARDS;
    Os::Logger::instance().log(FAC_SIP, PRI_INFO, "SIPX_PROXY_MAX_FORWARDS : %d", maxForwards);
    osPrintf("SIPX_PROXY_MAX_FORWARDS : %d\n", maxForwards);

    int branchTimeout = -1;
    osServiceOptions.getOption("SIPX_PROXY_BRANCH_TIMEOUT", branchTimeout);
    if(branchTimeout < 4)
    {
        branchTimeout = 24;
    }

    int defaultExpires;
    int defaultSerialExpires;
    osServiceOptions.getOption("SIPX_PROXY_DEFAULT_EXPIRES", defaultExpires);
    osServiceOptions.getOption("SIPX_PROXY_DEFAULT_SERIAL_EXPIRES", defaultSerialExpires);
    if(defaultExpires <= 0 ) 
    {
            defaultExpires = DEFAULT_SIP_TRANSACTION_EXPIRES;
    }
    else if(defaultExpires > DEFAULT_SIP_TRANSACTION_EXPIRES) 
    {
        Os::Logger::instance().log(FAC_SIP, PRI_WARNING,
                      "SipXproxymain::proxy "
                      "large default expires value: %d NOT RECOMMENDED",
                      defaultExpires);
    }
    if(defaultSerialExpires <= 0 ||
       defaultSerialExpires >= defaultExpires) 
    {
            defaultSerialExpires = DEFAULT_SIP_SERIAL_EXPIRES;
    }
    Os::Logger::instance().log(FAC_SIP, PRI_INFO, "SIPX_PROXY_DEFAULT_EXPIRES : %d", defaultExpires);
    osPrintf("SIPX_PROXY_DEFAULT_EXPIRES : %d\n", defaultExpires);
    Os::Logger::instance().log(FAC_SIP, PRI_INFO, "SIPX_PROXY_DEFAULT_SERIAL_EXPIRES : %d", defaultSerialExpires);
    osPrintf("SIPX_PROXY_DEFAULT_SERIAL_EXPIRES : %d\n", defaultSerialExpires);
      
    UtlString hostAliases;
    osServiceOptions.getOption("SIPX_PROXY_HOST_ALIASES", hostAliases);
    if(hostAliases.isNull())
    {
       hostAliases = ipAddress;
       char portBuf[20];
       sprintf(portBuf, ":%d", proxyUdpPort);
       hostAliases.append(portBuf);

       if(!routeName.isNull())
       {
          hostAliases.append(" ");
          hostAliases.append(routeName);
          char portBuf[20];
          sprintf(portBuf, ":%d", proxyUdpPort);
          hostAliases.append(portBuf);
       }
       Os::Logger::instance().log(FAC_SIP, PRI_NOTICE,
                     "SIPX_PROXY_HOST_ALIASES not configured"
                     " using implied value: %s",
                     hostAliases.data());
    }
    Os::Logger::instance().log(FAC_SIP, PRI_INFO, "SIPX_PROXY_HOST_ALIASES : %s",
                  hostAliases.data());

    UtlString enableCallStateObserverSetting;
    osServiceOptions.getOption(CONFIG_SETTING_CALL_STATE, enableCallStateObserverSetting);

    bool enableCallStateLogObserver;
    if (   (enableCallStateObserverSetting.isNull())
        || ((0 == enableCallStateObserverSetting.compareTo("disable", UtlString::ignoreCase)))
        )
    {
       enableCallStateLogObserver = false;
    }
    else if (0 == enableCallStateObserverSetting.compareTo("enable", UtlString::ignoreCase))
    {
       enableCallStateLogObserver = true;
    }
    else
    {
       enableCallStateLogObserver = false;
       Os::Logger::instance().log(FAC_SIP, PRI_ERR, "SipXproxymain:: invalid configuration value for "
                     CONFIG_SETTING_CALL_STATE " '%s' - should be 'enable' or 'disable'",
                     enableCallStateObserverSetting.data()
                     );
    }
    Os::Logger::instance().log(FAC_SIP, PRI_INFO, CONFIG_SETTING_CALL_STATE " : %s",
                  enableCallStateLogObserver ? "ENABLE" : "DISABLE" );

    UtlString callStateLogFileName;

    if (enableCallStateLogObserver)
    {
      osServiceOptions.getOption(CONFIG_SETTING_CALL_STATE_LOG, callStateLogFileName);
       if (callStateLogFileName.isNull())
       {
          callStateLogFileName = CALL_STATE_LOG_FILE_DEFAULT;
       }
       Os::Logger::instance().log(FAC_SIP, PRI_INFO, CONFIG_SETTING_CALL_STATE_LOG " : %s",
                     callStateLogFileName.data());
    }
    
    // Check if CSE logging should go into a database
    UtlString enableCallStateDbObserverSetting;
    osServiceOptions.getOption(CONFIG_SETTING_CALL_STATE_DB, enableCallStateDbObserverSetting);

    bool enableCallStateDbObserver;
    if (   (enableCallStateDbObserverSetting.isNull())
        || ((0 == enableCallStateDbObserverSetting.compareTo("disable", UtlString::ignoreCase)))
        )
    {
       enableCallStateDbObserver = false;
    }
    else if (0 == enableCallStateDbObserverSetting.compareTo("enable", UtlString::ignoreCase))
    {
       enableCallStateDbObserver = true;
    }
    else
    {
       enableCallStateDbObserver = false;
       Os::Logger::instance().log(FAC_SIP, PRI_ERR, "SipAuthProxyMain:: invalid configuration value for %s "
                     " '%s' - should be 'enable' or 'disable'",
                     CONFIG_SETTING_CALL_STATE_DB, enableCallStateDbObserverSetting.data()
                     );
    }
    Os::Logger::instance().log(FAC_SIP, PRI_INFO, "%s : %s", CONFIG_SETTING_CALL_STATE_DB,
                  enableCallStateDbObserver ? "ENABLE" : "DISABLE" );

    UtlString callStateDbHostName;
    UtlString callStateDbName;
    UtlString callStateDbUserName;
    UtlString callStateDbDriver;    
    if (enableCallStateDbObserver)
    {
      osServiceOptions.getOption(CONFIG_SETTING_CALL_STATE_DB_HOST, callStateDbHostName);
      if (callStateDbHostName.isNull())
      {
        callStateDbHostName = CALL_STATE_DATABASE_HOST;
      }
      Os::Logger::instance().log(FAC_SIP, PRI_INFO, "%s : %s", CONFIG_SETTING_CALL_STATE_DB_HOST,
          callStateDbHostName.data());

      osServiceOptions.getOption(CONFIG_SETTING_CALL_STATE_DB_NAME, callStateDbName);
      if (callStateDbName.isNull())
      {
        callStateDbName = CALL_STATE_DATABASE_NAME;
      }
      Os::Logger::instance().log(FAC_SIP, PRI_INFO, "%s : %s",  CONFIG_SETTING_CALL_STATE_DB_NAME,
          callStateDbName.data());

      osServiceOptions.getOption(CONFIG_SETTING_CALL_STATE_DB_USER, callStateDbUserName);
      if (callStateDbUserName.isNull())
      {
        callStateDbUserName = CALL_STATE_DATABASE_USER;
      }
      Os::Logger::instance().log(FAC_SIP, PRI_INFO, "%s : %s", CONFIG_SETTING_CALL_STATE_DB_USER,
          callStateDbUserName.data());

      osServiceOptions.getOption(CONFIG_SETTING_CALL_STATE_DB_DRIVER, callStateDbDriver);
      if (callStateDbDriver.isNull())
      {
        callStateDbDriver = CALL_STATE_DATABASE_DRIVER;
      }
      Os::Logger::instance().log(FAC_SIP, PRI_INFO, "%s : %s",  CONFIG_SETTING_CALL_STATE_DB_DRIVER,
          callStateDbDriver.data());
    }    
    
    // Select logging method - database takes priority over XML file
    if (enableCallStateLogObserver && enableCallStateDbObserver)
    {
       enableCallStateLogObserver = false;
       Os::Logger::instance().log(FAC_SIP, PRI_WARNING, "SipXproxymain:: both XML and database call state "
                     "logging was enabled - turning off XML log, only use database logging");       
    }

    // Set the maximum amount of time that TCP connections can
    // stay around when they are not used.
    int      staleTcpTimeout = 3600;
    UtlString staleTcpTimeoutStr;

    // Check for missing parameter or empty value
    osServiceOptions.getOption("SIPX_PROXY_STALE_TCP_TIMEOUT", staleTcpTimeoutStr);
    if (staleTcpTimeoutStr.isNull())
    {
      staleTcpTimeout = 3600;
    }
    else
    {
      // get the parameter value as an integer
      osServiceOptions.getOption("SIPX_PROXY_STALE_TCP_TIMEOUT", staleTcpTimeout);
    }

    if(staleTcpTimeout <= 0) staleTcpTimeout = -1;
    else if(staleTcpTimeout < 180) staleTcpTimeout = 180;
    Os::Logger::instance().log(FAC_SIP, PRI_INFO, "SIPX_PROXY_STALE_TCP_TIMEOUT : %d",
                  staleTcpTimeout);
    osPrintf("SIPX_PROXY_STALE_TCP_TIMEOUT : %d\n", staleTcpTimeout);

    int maxNumSrvRecords = -1;
    osServiceOptions.getOption("SIPX_PROXY_DNSSRV_MAX_DESTS", maxNumSrvRecords);
    Os::Logger::instance().log(FAC_SIP, PRI_INFO, "SIPX_PROXY_DNSSRV_MAX_DESTS : %d",
              maxNumSrvRecords);
    // If explicitly set to a valid number
    if(maxNumSrvRecords > 0)
    {
        osPrintf("SIPX_PROXY_DNSSRV_MAX_DESTS : %d\n", maxNumSrvRecords);
    }
    else
    {
        maxNumSrvRecords = 4;
    }

    int dnsSrvTimeout = -1; //seconds
    osServiceOptions.getOption("SIPX_PROXY_DNSSRV_TIMEOUT", dnsSrvTimeout);
    Os::Logger::instance().log(FAC_SIP, PRI_INFO, "SIPX_PROXY_DNSSRV_TIMEOUT : %d",
              dnsSrvTimeout);
    // If explicitly set to a valid number
    if(dnsSrvTimeout > 0)
    {
        osPrintf("SIPX_PROXY_DNSSRV_TIMEOUT : %d\n", dnsSrvTimeout);
    }
    else
    {
        dnsSrvTimeout = 4;
    }
    
    // This is an obnoxious special option to work around a 
    // problem with Sonus gateways.  The Sonus proxy or  redirect
    // server gives a list of possible gateways to recurse in a
    // 300 response.  It does not assign any Q values so the proxy
    // gets the impression that it should fork them all in parallel.
    // When this option is enabled we recurse only the one with the
    // highest Q value.
    UtlString recurseOnlyOne300String;
    osServiceOptions.getOption("SIPX_PROXY_SPECIAL_300", recurseOnlyOne300String);
    recurseOnlyOne300String.toLower();
    UtlBoolean recurseOnlyOne300 = FALSE;
    if(recurseOnlyOne300String.compareTo("enable") == 0) 
    {
        recurseOnlyOne300 = TRUE;
        Os::Logger::instance().log(FAC_SIP, PRI_INFO, "SIPX_PROXY_SPECIAL_300 : ENABLE");
        osPrintf("SIPX_PROXY_SPECIAL_300 : ENABLE\n");
    }
    
    OsPath fileName = SipXecsService::Path(SipXecsService::ConfigurationDirType,
                                            FORWARDING_RULES_FILENAME);

    ForwardRules forwardingRules;

    OsFile ruleFile(fileName);
    if(ruleFile.exists())
    {
        if(OS_SUCCESS != forwardingRules.loadMappings(fileName))
        {
            Os::Logger::instance().log(FAC_SIP, PRI_WARNING, "WARNING: Failed to load: %s",
                fileName.data());
            osPrintf("WARNING: Failed to load: %s\n", fileName.data());
        }
    }
    else
    {
        // forwardingrules.xml is not readable; no processing is possible.
        Os::Logger::instance().log(FAC_SIP, PRI_CRIT,
                      "forwarding rules '%s' not found -- exiting",
                      fileName.data());
        exit(1);
    }
 
#ifdef TEST_PRINT
    { // scope the test stuff
        SipMessage foo;
        const char* uri = "sip:10.1.20.3:5100";
        const char* method = SIP_ACK_METHOD;
        const char* eventType = SIP_EVENT_CONFIG;
        foo.setRequestData(method, 
                           uri, //uri, 
                           "sip:1234@doty.com", // fromField, 
                           "\"lajdflsdk ff\"<sip:laksdjf@1234.x.com>", // toField, 
                           "lkadsj902387", // callId, 
                           123, // CSeq,
                           "sip:10.1.1.123");// contactUrl

        // Set the event type
        foo.setHeaderValue(SIP_EVENT_FIELD, 
                            eventType, // event type
                            0);

        Url msgUrl(uri);
        UtlString routeTo;
        UtlString routeType;
        bool authRequired;
        OsStatus routeStatus = forwardingRules.getRoute(msgUrl, 
                                                        foo, 
                                                        routeTo, 
                                                        routeType,
                                                        authRequired);

        Url msgRouteToUri(routeTo);
        osPrintf("Message:\n\tmethod: %s\n\turi: %s\n\tevent: %s\nRouted:\n\tstring: %s\n\turi: %s\n\ttype: %s\n",
            method, uri, eventType, routeTo.data(), msgRouteToUri.toString().data(), routeType.data());
        if(routeStatus != OS_SUCCESS) 
            osPrintf("forwardingRules.getRoute returned: %d\n",
                    routeStatus);
    }
#endif // TEST_PRINT
    
    // Start the sip stack
    SipUserAgent* pSipUserAgent = new SipUserAgent(
        proxyTcpPort,
        proxyUdpPort,
        proxyTlsPort,
        NULL, // public IP address (not used in proxy)
        NULL, // default user (not used in proxy)
        bindIp,
        NULL, // outbound proxy
        NULL, // directory server
        NULL, // registry server
        NULL, // auth realm
        NULL, // auth DB
        NULL, // auth user IDs
        NULL, // auth passwords
        NULL, // line mgr
        SIP_DEFAULT_RTT, // first resend timeout
        FALSE, // default to proxy transaction
        SIPUA_DEFAULT_SERVER_UDP_BUFFER_SIZE, // socket layer read buffer size
        SIPUA_DEFAULT_SERVER_OSMSG_QUEUE_SIZE, // OsServerTask message queue size
        FALSE, // Use Next Available Port
        TRUE,  // Perform message checks 
        TRUE,  // Use symmetric signaling
        SipUserAgent::HANDLE_OPTIONS_AUTOMATICALLY);


    if (!pSipUserAgent->isOk())
    {
        Os::Logger::instance().log(FAC_SIP, PRI_EMERG, "SipUserAgent reported a problem, setting shutdown flag...");
        gShutdownFlag = TRUE;
    }
    pSipUserAgent->setDnsSrvTimeout(dnsSrvTimeout);
    pSipUserAgent->setMaxSrvRecords(maxNumSrvRecords);
    pSipUserAgent->setUserAgentHeaderProperty("sipXecs/sipXproxy");
    pSipUserAgent->setMaxForwards(maxForwards);
    pSipUserAgent->setDefaultExpiresSeconds(defaultExpires);
    pSipUserAgent->setDefaultSerialExpiresSeconds(defaultSerialExpires);
    pSipUserAgent->setMaxTcpSocketIdleTime(staleTcpTimeout);
    pSipUserAgent->setHostAliases(hostAliases);
    pSipUserAgent->setRecurseOnlyOne300Contact(recurseOnlyOne300);
    // Do not start the SipUserAgent until its message listeners are
    // set up by the creation of the SipRouter below.
    
    UtlString buffer;

    // Create the CSE observer, either writing to file or database
    SipXProxyCseObserver* cseObserver = NULL;
    CallStateEventWriter* pEventWriter = NULL;    
    if (enableCallStateLogObserver)
    {
       // Set up the call state event log file
       pEventWriter = new CallStateEventWriter_XML(callStateLogFileName.data());
    }
    else if (enableCallStateDbObserver)
    {
       pEventWriter = new CallStateEventWriter_DB(callStateDbName.data(),
                                                  callStateDbHostName.data(),
                                                  callStateDbUserName,
                                                  callStateDbDriver);      
    }                                            
       
    if (pEventWriter)
    {
       // get the identifier for this observer
       int protocol = OsSocket::UDP;
       UtlString domainName;
       int port;
       pSipUserAgent->getViaInfo(protocol, domainName, port);

       char portString[12];
       sprintf(portString,":%d", port);
       domainName.append(portString);
       
       // and start the observer
       cseObserver = new SipXProxyCseObserver(*pSipUserAgent, domainName, pEventWriter);
       cseObserver->start();
    }
    else
    {
      // Only log error if any event logging was enabled
      if (enableCallStateLogObserver || enableCallStateDbObserver)
      {      
         Os::Logger::instance().log(FAC_SIP, PRI_ERR,
                       "SipAuthProxyMain:: EventWriter could not be allocated"
                       );
         enableCallStateLogObserver = false;
         enableCallStateDbObserver = false;
      }
    }

    // Create a router to route stuff either
    // to a local server or on out to the real world
    SipRouter* pRouter = new SipRouter(*pSipUserAgent, 
                                       forwardingRules,
                                       osServiceOptions.getOsConfigDb());

    // Start the router running
    pRouter->start();

    // Do not exit, let the proxy do its stuff
    if (!gShutdownFlag)
    {
      while(!SipXApplication::instance().terminationRequested())
      {
          OsTask::delay(1000);
      }
    }
 
    // This is a server task so gracefully shutdown the
    // router task by deleting it.
    delete pRouter ;

    // Stop the SipUserAgent.
    pSipUserAgent->shutdown();
    // And delete it, too.
    delete pSipUserAgent ;

    // flush and close the call state event log
    if (enableCallStateLogObserver || enableCallStateDbObserver)
    {
      if (cseObserver)
      {
         delete cseObserver;
      }
      if (pEventWriter)
      {
         delete pEventWriter;
      }
    }

    OsStunAgentTask::releaseInstance();

    return 0 ;
}


// USAGE:  sipXproxy [-v] [pidfile]
int main(int argc, char* argv[]) {

  SipXApplicationData rlsData =
  {
      SIPXPROXY_APP_NAME,
      CONFIG_SETTINGS_FILE,
      SIPX_PROXY_LOG_FILE,
      "",
      PROXY_CONFIG_PREFIX,
      true, // daemonize
      false, // do not check mongo connection
      OsMsgQShared::QUEUE_UNLIMITED,
  };

  // NOTE: this might exit application in case of failure
  SipXApplication::instance().init(argc, argv, rlsData);

  proxy();

  SipXApplication::instance().terminate();
}
