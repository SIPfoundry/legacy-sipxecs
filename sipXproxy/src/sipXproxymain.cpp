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
#include <os/OsConfigDb.h>
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
#include "config.h"

//
// Exception handling
//
#include <stdexcept>
#include <execinfo.h>
#include <mongo/util/assert_util.h>

#define CONFIG_SETTING_CALL_STATE         "SIPX_PROXY_CALL_STATE"
#define CONFIG_SETTING_CALL_STATE_LOG     "SIPX_PROXY_CALL_STATE_LOG"
#define CALL_STATE_LOG_FILE_DEFAULT SIPX_LOGDIR "/sipxproxy_callstate.log"
#define FORWARDING_RULES_FILENAME "forwardingrules.xml"
#define SIPX_PROXY_LOG_FILE "sipXproxy.log"
#define CONFIG_LOG_DIR SIPX_LOGDIR
#define LOG_FACILITY   FAC_SIP

// Configuration names pulled from config-file
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

/**
 * Description:
 * closes any open connections to the IMDB safely using a mutex lock
 */
void initSysLog(OsConfigDb* pConfig)
{
   UtlString consoleLogging;         // Enable console logging by default?
   UtlString fileTarget;             // Path to store log file.
   UtlBoolean bSpecifiedDirError ;   // Set if the specified log dir does not
                                    // exist

   Os::LoggerHelper::instance().processName = "SipXProxy";

   //
   // Get/Apply Log Filename
   //
   if ((pConfig->get(CONFIG_SETTING_LOG_DIR, fileTarget) != OS_SUCCESS) ||
      fileTarget.isNull() || !OsFileSystem::exists(fileTarget))
   {
      bSpecifiedDirError = !fileTarget.isNull() ;

      // If the log file directory exists use that, otherwise place the log
      // in the current directory
      OsPath workingDirectory;
      if (OsFileSystem::exists(CONFIG_LOG_DIR))
      {
         fileTarget = CONFIG_LOG_DIR;
         OsPath path(fileTarget);
         path.getNativePath(workingDirectory);

         osPrintf("%s : %s\n", CONFIG_SETTING_LOG_DIR, workingDirectory.data()) ;
         Os::Logger::instance().log(FAC_SIP, PRI_INFO, "%s : %s", CONFIG_SETTING_LOG_DIR, workingDirectory.data()) ;
      }
      else
      {
         OsPath path;
         OsFileSystem::getWorkingDirectory(path);
         path.getNativePath(workingDirectory);

         osPrintf("%s : %s\n", CONFIG_SETTING_LOG_DIR, workingDirectory.data()) ;
         Os::Logger::instance().log(FAC_SIP, PRI_INFO, "%s : %s", CONFIG_SETTING_LOG_DIR, workingDirectory.data()) ;
      }

      fileTarget = workingDirectory + OsPathBase::separator + SIPX_PROXY_LOG_FILE;
   }
   else
   {
      bSpecifiedDirError = false ;
      osPrintf("%s : %s\n", CONFIG_SETTING_LOG_DIR, fileTarget.data()) ;
      Os::Logger::instance().log(FAC_SIP, PRI_INFO, "%s : %s", CONFIG_SETTING_LOG_DIR, fileTarget.data()) ;

      fileTarget = fileTarget +
         OsPathBase::separator +
         SIPX_PROXY_LOG_FILE;
   }

   

   //
   // Get/Apply Log Level
   //
   SipXecsService::setLogPriority( CONFIG_SETTINGS_FILE, PROXY_CONFIG_PREFIX );
   Os::Logger::instance().setLoggingPriorityForFacility(FAC_SIP_INCOMING_PARSED, PRI_ERR);
   Os::LoggerHelper::instance().initialize(fileTarget.data());
   //
   // Get/Apply console logging
   //
   UtlBoolean bConsoleLoggingEnabled = false ;
   if ((pConfig->get(CONFIG_SETTING_LOG_CONSOLE, consoleLogging) ==
         OS_SUCCESS))
   {
      consoleLogging.toUpper();
      if (consoleLogging == "ENABLE")
      {
         Os::Logger::instance().enableConsoleOutput(true);
         bConsoleLoggingEnabled = true ;
      }
   }

   osPrintf("%s : %s\n", CONFIG_SETTING_LOG_CONSOLE, bConsoleLoggingEnabled ? "ENABLE" : "DISABLE") ;
   Os::Logger::instance().log(FAC_SIP, PRI_INFO, "%s : %s",
                 CONFIG_SETTING_LOG_CONSOLE, bConsoleLoggingEnabled ? "ENABLE" : "DISABLE") ;

   if (bSpecifiedDirError)
   {
      Os::Logger::instance().log(FAC_LOG, PRI_CRIT,
                    "Cannot access %s directory; please check configuration.",
                    CONFIG_SETTING_LOG_DIR);
   }
}

// copy error information to log. registered only after logger has been configured.
void catch_global()
{
#define catch_global_print(msg)  \
  std::ostringstream bt; \
  bt << msg << std::endl; \
  void* trace_elems[20]; \
  int trace_elem_count(backtrace( trace_elems, 20 )); \
  char** stack_syms(backtrace_symbols(trace_elems, trace_elem_count)); \
  for (int i = 0 ; i < trace_elem_count ; ++i ) \
    bt << stack_syms[i] << std::endl; \
  Os::Logger::instance().log(FAC_LOG, PRI_CRIT, bt.str().c_str()); \
  std::cerr << bt.str().c_str(); \
  free(stack_syms);

  try
  {
      throw;
  }
  catch (std::string& e)
  {
    catch_global_print(e.c_str());
  }
#ifdef MONGO_assert
  catch (mongo::DBException& e)
  {
    catch_global_print(e.toString().c_str());
  }
#endif
  catch (boost::exception& e)
  {
    catch_global_print(diagnostic_information(e).c_str());
  }
  catch (std::exception& e)
  {
    catch_global_print(e.what());
  }
  catch (...)
  {
    catch_global_print("Error occurred. Unknown exception type.");
  }

  std::abort();
}


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

    OsMsgQShared::setQueuePreference(OsMsgQShared::QUEUE_UNLIMITED);

    OsSocket::getHostIp(&ipAddress);

    OsPath ConfigfileName = SipXecsService::Path(SipXecsService::ConfigurationDirType,
                                                 CONFIG_SETTINGS_FILE);
    OsConfigDb configDb;

    if(OS_SUCCESS != configDb.loadFromFile(ConfigfileName))
    {      
       exit(1);
    }
    // Initialize the OsSysLog...
    initSysLog(&configDb);
    std::set_terminate(catch_global);

    configDb.get(CONFIG_SETTING_BIND_IP, bindIp);
    if ((bindIp.isNull()) || !OsSocket::isIp4Address(bindIp))
    {
       bindIp = "0.0.0.0";
    }
    Os::Logger::instance().log(FAC_SIP, PRI_INFO, "%s: %s", CONFIG_SETTING_BIND_IP, 
          bindIp.data());
    osPrintf("%s: %s", CONFIG_SETTING_BIND_IP, bindIp.data());    

    UtlString hostname;
    configDb.get("SIPX_PROXY_HOST_NAME", hostname);
    if (!hostname.isNull())
    {
       // bias the selection of SRV records so that if the name of this host is an alternative,
       // it wins in any selection based on random weighting.
       SipSrvLookup::setOwnHostname(hostname);
    }
    Os::Logger::instance().log(FAC_SIP, PRI_INFO, "SIPX_PROXY_HOST_NAME : %s", hostname.data());
    
    proxyUdpPort = configDb.getPort("SIPX_PROXY_UDP_PORT");
    if (!portIsValid(proxyUdpPort))
    {
       proxyUdpPort = 5060;
    }
    Os::Logger::instance().log(FAC_SIP, PRI_INFO, "SIPX_PROXY_UDP_PORT : %d", proxyUdpPort);
    proxyTcpPort = configDb.getPort("SIPX_PROXY_TCP_PORT") ;
    if (!portIsValid(proxyTcpPort))
    {
       proxyTcpPort = 5060;
    }
    Os::Logger::instance().log(FAC_SIP, PRI_INFO, "SIPX_PROXY_TCP_PORT : %d", proxyTcpPort);
    proxyTlsPort = configDb.getPort("SIPX_PROXY_TLS_PORT") ;
    if (!portIsValid(proxyTlsPort))
    {
       proxyTlsPort = 5061;
    }
    Os::Logger::instance().log(FAC_SIP, PRI_INFO, "SIPX_PROXY_TLS_PORT : %d", proxyTlsPort);

    configDb.get("SIPX_PROXY_MAX_FORWARDS", maxForwards);
    if(maxForwards <= 0) maxForwards = SIP_DEFAULT_MAX_FORWARDS;
    Os::Logger::instance().log(FAC_SIP, PRI_INFO, "SIPX_PROXY_MAX_FORWARDS : %d", maxForwards);
    osPrintf("SIPX_PROXY_MAX_FORWARDS : %d\n", maxForwards);

    int branchTimeout = -1;
    configDb.get("SIPX_PROXY_BRANCH_TIMEOUT", branchTimeout);
    if(branchTimeout < 4)
    {
        branchTimeout = 24;
    }

    int defaultExpires;
    int defaultSerialExpires;
    configDb.get("SIPX_PROXY_DEFAULT_EXPIRES", defaultExpires);
    configDb.get("SIPX_PROXY_DEFAULT_SERIAL_EXPIRES", defaultSerialExpires);
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
    configDb.get("SIPX_PROXY_HOST_ALIASES", hostAliases);
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
    configDb.get(CONFIG_SETTING_CALL_STATE, enableCallStateObserverSetting);

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
       configDb.get(CONFIG_SETTING_CALL_STATE_LOG, callStateLogFileName);
       if (callStateLogFileName.isNull())
       {
          callStateLogFileName = CALL_STATE_LOG_FILE_DEFAULT;
       }
       Os::Logger::instance().log(FAC_SIP, PRI_INFO, CONFIG_SETTING_CALL_STATE_LOG " : %s",
                     callStateLogFileName.data());
    }
    
    // Check if CSE logging should go into a database
    UtlString enableCallStateDbObserverSetting;
    configDb.get(CONFIG_SETTING_CALL_STATE_DB, enableCallStateDbObserverSetting);

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
       configDb.get(CONFIG_SETTING_CALL_STATE_DB_HOST, callStateDbHostName);
       if (callStateDbHostName.isNull())
       {
          callStateDbHostName = CALL_STATE_DATABASE_HOST;
       }
       Os::Logger::instance().log(FAC_SIP, PRI_INFO, "%s : %s", CONFIG_SETTING_CALL_STATE_DB_HOST,
                     callStateDbHostName.data());
                     
       configDb.get(CONFIG_SETTING_CALL_STATE_DB_NAME, callStateDbName);
       if (callStateDbName.isNull())
       {
          callStateDbName = CALL_STATE_DATABASE_NAME;
       }
       Os::Logger::instance().log(FAC_SIP, PRI_INFO, "%s : %s",  CONFIG_SETTING_CALL_STATE_DB_NAME,
                     callStateDbName.data());
                     
       configDb.get(CONFIG_SETTING_CALL_STATE_DB_USER, callStateDbUserName);
       if (callStateDbUserName.isNull())
       {
          callStateDbUserName = CALL_STATE_DATABASE_USER;
       }
       Os::Logger::instance().log(FAC_SIP, PRI_INFO, "%s : %s", CONFIG_SETTING_CALL_STATE_DB_USER,
                     callStateDbUserName.data());                                          
                     
       configDb.get(CONFIG_SETTING_CALL_STATE_DB_DRIVER, callStateDbDriver);
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
    configDb.get("SIPX_PROXY_STALE_TCP_TIMEOUT", staleTcpTimeoutStr);
    if (staleTcpTimeoutStr.isNull())
    {
        staleTcpTimeout = 3600;
    }
    else
    {
        // get the parameter value as an integer
        configDb.get("SIPX_PROXY_STALE_TCP_TIMEOUT", staleTcpTimeout);
    }

    if(staleTcpTimeout <= 0) staleTcpTimeout = -1;
    else if(staleTcpTimeout < 180) staleTcpTimeout = 180;
    Os::Logger::instance().log(FAC_SIP, PRI_INFO, "SIPX_PROXY_STALE_TCP_TIMEOUT : %d",
                  staleTcpTimeout);
    osPrintf("SIPX_PROXY_STALE_TCP_TIMEOUT : %d\n", staleTcpTimeout);

    int maxNumSrvRecords = -1;
    configDb.get("SIPX_PROXY_DNSSRV_MAX_DESTS", maxNumSrvRecords);
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
    configDb.get("SIPX_PROXY_DNSSRV_TIMEOUT", dnsSrvTimeout);
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
    configDb.get("SIPX_PROXY_SPECIAL_300", recurseOnlyOne300String);
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
        UtlString ruriParams;
        OsStatus routeStatus = forwardingRules.getRoute(msgUrl, 
                                                        foo, 
                                                        routeTo, 
                                                        routeType,
                                                        authRequired,
                                                        ruriParams);

        Url msgRouteToUri(routeTo);
        osPrintf("Message:\n\tmethod: %s\n\turi: %s\n\tevent: %s\nRouted:\n\tstring: %s\n\turi: %s\n\ttype: %s\n\truriParams: %s\n",
            method, uri, eventType, routeTo.data(), msgRouteToUri.toString().data(), routeType.data(), ruriParams.data());
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
                                       configDb);

    // Start the router running
    pRouter->start();

    // Do not exit, let the proxy do its stuff
    while( !gShutdownFlag ) {
        OsTask::delay(1000);
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

    // End the singleton threads.
    OsTimer::terminateTimerService();
    OsStunAgentTask::releaseInstance();

    return 0 ;
}

void signal_handler(int sig) {
  switch(sig)
  {
    case SIGPIPE:
      Os::Logger::instance().log(FAC_SIP, PRI_INFO, "SIGPIPE caught. Ignored.");
      break;

    case SIGHUP:
      Os::Logger::instance().log(FAC_SIP, PRI_INFO, "SIGHUP caught. Ignored.");
      break;

    case SIGTERM:
      gShutdownFlag = TRUE;
      Os::Logger::instance().log(FAC_SIP, PRI_INFO, "SIGTERM caught. Shutting down.");
      break;

    case SIGUSR1:
      system_tap_start_portlib_instrumentation(true/*Bactrace enabled*/);
      Os::Logger::instance().log(FAC_SIP, PRI_NOTICE, "SIGUSR1 caught. Starting instrumentations.");
      break;
      
    case SIGUSR2:
      system_tap_stop_portlib_instrumentation();
      Os::Logger::instance().log(FAC_SIP, PRI_NOTICE, "SIGUSR2 caught. Starting instrumentations.");
      break;
  }
}

// USAGE:  sipXproxy [-v] [pidfile]
int main(int argc, char* argv[]) {
    char* pidFile = NULL;
    for(int i = 1; i < argc; i++) {
        if(strncmp("-v", argv[i], 2) == 0) {
  	    std::cout << "Version: " << PACKAGE_VERSION << PACKAGE_REVISION << std::endl;
	    exit(0);
	} else {
            pidFile = argv[i];
	}
    }
    if (pidFile) {
      daemonize(pidFile);
    }
    signal(SIGHUP, signal_handler); // catch hangup signal
    signal(SIGTERM, signal_handler); // catch kill signal
    signal(SIGPIPE, signal_handler); // r/w socket failure
    signal(SIGUSR1, signal_handler);
    signal(SIGUSR2, signal_handler);
    proxy();
    Os::Logger::instance().log(FAC_SIP, PRI_NOTICE, "Exiting") ;
    Os::Logger::instance().flush();

    mongo::dbexit(mongo::EXIT_CLEAN);
}
