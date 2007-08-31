// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// System includes
#include <stdio.h>
#include <signal.h>
#if defined(_WIN32)
#include <windows.h>
#elif defined(__pingtel_on_posix__)
#include <unistd.h>
#endif

#include <os/OsFS.h>
#include <os/OsConfigDb.h>
#include <os/OsSocket.h>
#include <os/OsTask.h>
#include <net/SipUserAgent.h>
#include <net/NameValueTokenizer.h>
#include <xmlparser/tinyxml.h>
#include <sipXecsService/SipXecsService.h>
#include <sipXecsService/SharedSecret.h>

#include <SipRouter.h>
#include <ForwardRules.h>

#include <ForkingProxyCseObserver.h>

#ifndef SIPX_VERSION
#  include "sipxproxy-buildstamp.h"
#  define SIPX_VERSION SipXproxyVersion
#  define SIPX_BUILD   SipXproxyBuildStamp
#else
#  define SIPX_BUILD   ""
#endif

//uncomment next line to enable bound checker checking with 'b' key
//#define BOUNDS_CHECKER

#ifdef BOUNDS_CHECKER
	#include "D:\Program Files\Compuware\BoundsChecker\ERptApi\apilib.h"
	#pragma comment(lib, "D:\\Program Files\\Compuware\\BoundsChecker\\ERptApi\\nmapi.lib")
#endif

#define CONFIG_ETC_DIR SIPX_CONFDIR
#define FORWARDING_RULES_FILENAME "forwardingrules.xml"
#define SIP_PROXY_LOG "sipproxy.log"
#define CONFIG_LOG_DIR SIPX_LOGDIR
#define LOG_FACILITY   FAC_SIP
#define CALL_STATE_LOG_FILE_DEFAULT SIPX_LOGDIR "/sipproxy_callstate.log"

// Configuration names pulled from config-file
#define CONFIG_SETTING_LOG_LEVEL      "SIP_PROXY_LOG_LEVEL"
#define CONFIG_SETTING_LOG_CONSOLE    "SIP_PROXY_LOG_CONSOLE"
#define CONFIG_SETTING_LOG_DIR        "SIP_PROXY_LOG_DIR"
#define CONFIG_SETTING_CALL_STATE     "SIP_PROXY_CALL_STATE"
#define CONFIG_SETTING_CALL_STATE_LOG "SIP_PROXY_CALL_STATE_LOG"

static const char* CONFIG_SETTING_CALL_STATE_DB =
   "SIP_PROXY_CALL_STATE_DB";
static const char* CONFIG_SETTING_CALL_STATE_DB_HOST =
   "SIP_PROXY_CALL_STATE_DB_HOST";
static const char* CONFIG_SETTING_CALL_STATE_DB_NAME =
   "SIP_PROXY_CALL_STATE_DB_NAME";   
static const char* CONFIG_SETTING_CALL_STATE_DB_USER =
   "SIP_PROXY_CALL_STATE_DB_USER";   
static const char* CONFIG_SETTING_CALL_STATE_DB_DRIVER =
   "SIP_PROXY_CALL_STATE_DB_DRIVER";   
static const char* CALL_STATE_DATABASE_HOST =
   "localhost";   
static const char* CALL_STATE_DATABASE_NAME =
   "SIPXCDR";
static const char* CALL_STATE_DATABASE_USER =
   "postgres";
static const char* CALL_STATE_DATABASE_DRIVER =
   "{PostgreSQL}";

#define PRINT_ROUTE_RULE(APPEND_STRING, FROM_HOST, TO_HOST) \
    APPEND_STRING.append("\t<route mappingType=\"local\">\n\t\t<routeFrom>"); \
    APPEND_STRING.append(FROM_HOST); \
    APPEND_STRING.append("</routeFrom>\n\t\t<routeTo>"); \
    APPEND_STRING.append(TO_HOST); \
    APPEND_STRING.append("</routeTo>\n\t</route>\n");

// TYPEDEFS
typedef void (*sighandler_t)(int);

// FUNCTIONS
extern "C" {
    void  sigHandler( int sig_num );
    sighandler_t pt_signal( int sig_num, sighandler_t handler );
}

// GLOBALS
UtlBoolean gShutdownFlag = FALSE;

/**
 * Description:
 * This is a replacement for signal() which registers a signal handler but sets
 * a flag causing system calls ( namely read() or getchar() ) not to bail out 
 * upon recepit of that signal. We need this behavior, so we must call 
 * sigaction() manually.
 */
sighandler_t 
pt_signal( int sig_num, sighandler_t handler)
{
#if defined(__pingtel_on_posix__)
    struct sigaction action[2];
    action[0].sa_handler = handler;
    sigemptyset(&action[0].sa_mask);
    action[0].sa_flags = 0;
    sigaction ( sig_num, &action[0], &action[1] );
    return action[1].sa_handler;
#else
    return signal( sig_num, handler );
#endif
}

/** 
 * Description: 
 * This is the signal handler, When called this sets the 
 * global gShutdownFlag allowing the main processing
 * loop to exit cleanly.
 */
void 
sigHandler( int sig_num )
{
    // set a global shutdown flag
    gShutdownFlag = TRUE;

    // Unregister interest in the signal to prevent recursive callbacks
    pt_signal( sig_num, SIG_DFL );

    // Minimize the chance that we lose log data
    OsSysLog::flush();
    if (SIGTERM == sig_num)
    {
       OsSysLog::add( LOG_FACILITY, PRI_INFO, "sigHandler: terminate signal received.");
    }
    else
    {
       OsSysLog::add( LOG_FACILITY, PRI_CRIT, "sigHandler: caught signal: %d", sig_num );
    }
    OsSysLog::add( LOG_FACILITY, PRI_CRIT, "sigHandler: closing IMDB connections" );
    OsSysLog::flush();
}

// Initialize the OsSysLog
void initSysLog(OsConfigDb* pConfig)
{
   UtlString logLevel;               // Controls Log Verbosity
   UtlString consoleLogging;         // Enable console logging by default?
   UtlString fileTarget;             // Path to store log file.
   UtlBoolean bSpecifiedDirError ;   // Set if the specified log dir does not 
                                    // exist
   struct tagPrioriotyLookupTable
   {
      const char*      pIdentity;
      OsSysLogPriority ePriority;
   };

   struct tagPrioriotyLookupTable lkupTable[] =
   {
      { "DEBUG",   PRI_DEBUG},
      { "INFO",    PRI_INFO},
      { "NOTICE",  PRI_NOTICE},
      { "WARNING", PRI_WARNING},
      { "ERR",     PRI_ERR},
      { "CRIT",    PRI_CRIT},
      { "ALERT",   PRI_ALERT},
      { "EMERG",   PRI_EMERG},
   };
   OsSysLog::initialize(0, "SipProxy");


   //
   // Get/Apply Log Filename
   //
   fileTarget.remove(0) ;
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
         OsSysLog::add(FAC_SIP, PRI_INFO, "%s : %s", CONFIG_SETTING_LOG_DIR, workingDirectory.data()) ;
      }
      else
      {
         OsPath path;
         OsFileSystem::getWorkingDirectory(path);
         path.getNativePath(workingDirectory);

         osPrintf("%s : %s\n", CONFIG_SETTING_LOG_DIR, workingDirectory.data()) ;
         OsSysLog::add(FAC_SIP, PRI_INFO, "%s : %s", CONFIG_SETTING_LOG_DIR, workingDirectory.data()) ;
      }

      fileTarget = workingDirectory + 
         OsPathBase::separator +
         SIP_PROXY_LOG;         
   }
   else
   {
      bSpecifiedDirError = false ;
      osPrintf("%s : %s\n", CONFIG_SETTING_LOG_DIR, fileTarget.data()) ;
      OsSysLog::add(FAC_SIP, PRI_INFO, "%s : %s", CONFIG_SETTING_LOG_DIR, fileTarget.data()) ;

      fileTarget = fileTarget + 
         OsPathBase::separator +
         SIP_PROXY_LOG;
   }
   OsSysLog::setOutputFile(0, fileTarget) ;


   //
   // Get/Apply Log Level
   //
   if ((pConfig->get(CONFIG_SETTING_LOG_LEVEL, logLevel) != OS_SUCCESS) ||
         logLevel.isNull())
   {
      logLevel = "ERR";
   }
   logLevel.toUpper();
   OsSysLogPriority priority = PRI_ERR;
   int iEntries = sizeof(lkupTable)/sizeof(struct tagPrioriotyLookupTable);
   for (int i=0; i<iEntries; i++)
   {
      if (logLevel == lkupTable[i].pIdentity)
      {
         priority = lkupTable[i].ePriority;
         osPrintf("%s : %s\n", CONFIG_SETTING_LOG_LEVEL, lkupTable[i].pIdentity) ;
         OsSysLog::add(FAC_SIP, PRI_INFO, "%s : %s", CONFIG_SETTING_LOG_LEVEL, lkupTable[i].pIdentity) ;
         break;
      }
   }
   OsSysLog::setLoggingPriority(priority);
   // disable extra logging of every message after parsing
   OsSysLog::setLoggingPriorityForFacility(FAC_SIP_INCOMING_PARSED, PRI_ERR);

   //
   // Get/Apply console logging
   //
   if ((pConfig->get(CONFIG_SETTING_LOG_CONSOLE, consoleLogging) == 
         OS_SUCCESS))
   {
      consoleLogging.toUpper();
      if (consoleLogging == "ENABLE")
      {
         OsSysLog::enableConsoleOutput(true);        
         osPrintf("%s : %s\n", CONFIG_SETTING_LOG_CONSOLE, "ENABLE") ;
         OsSysLog::add(FAC_SIP, PRI_INFO, "%s : %s", CONFIG_SETTING_LOG_CONSOLE, "ENABLE") ;
      }
      else
      {
         osPrintf("%s : %s\n", CONFIG_SETTING_LOG_CONSOLE, "DISABLE") ;
         OsSysLog::add(FAC_SIP, PRI_INFO, "%s : %s", CONFIG_SETTING_LOG_CONSOLE, "DISABLE") ;
      }
   }   

   if (bSpecifiedDirError)
   {
      OsSysLog::add(FAC_LOG, PRI_CRIT, "Cannot access %s directory; please check configuration.", CONFIG_SETTING_LOG_DIR);
   }
}

/** The main entry point to the proxy */
int
main(int argc, char* argv[])
{
    // Register Signal handlers to close IMDB
    pt_signal(SIGINT,   sigHandler);    // Trap Ctrl-C on NT
    pt_signal(SIGILL,   sigHandler); 
    pt_signal(SIGABRT,  sigHandler);    // Abort signal 6
    pt_signal(SIGFPE,   sigHandler);    // Floading Point Exception
    pt_signal(SIGSEGV,  sigHandler);    // Address access violations signal 11 
    pt_signal(SIGTERM,  sigHandler);    // Trap kill -15 on UNIX
#if defined(__pingtel_on_posix__)
    pt_signal(SIGHUP,   sigHandler);    // Hangup
    pt_signal(SIGQUIT,  sigHandler); 
    pt_signal(SIGPIPE,  SIG_IGN);    
    pt_signal(SIGBUS,   sigHandler); 
    pt_signal(SIGSYS,   sigHandler); 
    pt_signal(SIGXCPU,  sigHandler); 
    pt_signal(SIGXFSZ,  sigHandler);
    pt_signal(SIGUSR1,  sigHandler); 
    pt_signal(SIGUSR2,  sigHandler); 
#endif

    UtlString argString;
    UtlBoolean interactiveSet = false;

    for(int argIndex = 1; argIndex < argc; argIndex++)
    {
        osPrintf("arg[%d]: %s\n", argIndex, argv[argIndex]);
        argString = argv[argIndex];
        NameValueTokenizer::frontBackTrim(&argString, "\t ");
        if(argString.compareTo("-v") == 0)
        {
            osPrintf("Version: %s %s\n", SIPX_VERSION, SIPX_BUILD);
            return(1);
        }
        else if( argString.compareTo("-i") == 0)
        {
           interactiveSet = true;
           osPrintf("Entering Interactive Mode\n");
        }
         else
         {
            osPrintf("usage: %s [-v] [-i]\nwhere:\n -v provides the software version\n"
               " -i starts the server in an interactive mode\n",
            argv[0]);
            return(1);
         }
    }
   

    int proxyTcpPort;
    int proxyUdpPort;
    int proxyTlsPort;
    UtlString domainName;
    UtlString proxyRecordRoute;
    int maxForwards;

    UtlString ipAddress;

    OsSocket::getHostIp(&ipAddress);

    OsPath DomainConfigfileName = SipXecsService::domainConfigPath();
    OsConfigDb domainConfigDb;

    if(OS_SUCCESS != domainConfigDb.loadFromFile(DomainConfigfileName))
    {      
       osPrintf("Failed to open domain config file: %s\n", DomainConfigfileName.data());
    }

    OsPath ConfigfileName = SipXecsService::Path(SipXecsService::ConfigurationDirType,
                                                 "proxy-config");
    OsConfigDb configDb;

    if(OS_SUCCESS != configDb.loadFromFile(ConfigfileName))
    {      
       osPrintf("Failed to open config file: %s\n", ConfigfileName.data());
    }
    
    SharedSecret LoopDetectionSecret(domainConfigDb);
    BranchId::setSecret(LoopDetectionSecret);
    
    // Initialize the OsSysLog...
    initSysLog(&configDb);    

    OsSysLog::add(FAC_SIP, PRI_INFO, ">>>>>>>>>>>>>>>> Starting - version %s build %s",
                  SIPX_VERSION, SIPX_BUILD
                  );

    configDb.get("SIP_PROXY_DOMAIN_NAME", domainName);
    if(domainName.isNull())
    {
        domainName = ipAddress;
    }
    OsSysLog::add(FAC_SIP, PRI_INFO, "SIP_PROXY_DOMAIN_NAME : %s", domainName.data());
    osPrintf("SIP_PROXY_DOMAIN_NAME : %s\n", domainName.data());

    proxyUdpPort = configDb.getPort("SIP_PROXY_UDP_PORT") ;
    if (!portIsValid(proxyUdpPort))
    {
       proxyUdpPort = 5060;
    }
    OsSysLog::add(FAC_SIP, PRI_INFO, "SIP_PROXY_UDP_PORT : %d", proxyUdpPort);
    proxyTcpPort = configDb.getPort("SIP_PROXY_TCP_PORT") ;
    if (!portIsValid(proxyTcpPort))
    {
       proxyTcpPort = 5060;
    }
    OsSysLog::add(FAC_SIP, PRI_INFO, "SIP_PROXY_TCP_PORT : %d", proxyTcpPort);
    proxyTlsPort = configDb.getPort("SIP_PROXY_TLS_PORT") ;
    if (!portIsValid(proxyTlsPort))
    {
       proxyTlsPort = 5061;
    }
    OsSysLog::add(FAC_SIP, PRI_INFO, "SIP_PROXY_TLS_PORT : %d", proxyTlsPort);

    configDb.get("SIP_PROXY_RECORD_ROUTE", proxyRecordRoute);
    UtlBoolean recordRouteEnabled = FALSE;
    proxyRecordRoute.toLower();
    if(proxyRecordRoute.compareTo("enable") == 0)
    {
        recordRouteEnabled = TRUE;
        OsSysLog::add(FAC_SIP, PRI_INFO, "SIP_PROXY_RECORD_ROUTE : ENABLE");
        osPrintf("SIP_PROXY_RECORD_ROUTE : ENABLE\n");
    }    

    configDb.get("SIP_PROXY_MAX_FORWARDS", maxForwards);
    if(maxForwards <= 0) maxForwards = SIP_DEFAULT_MAX_FORWARDS;
    OsSysLog::add(FAC_SIP, PRI_INFO, "SIP_PROXY_MAX_FORWARDS : %d", maxForwards);
    osPrintf("SIP_PROXY_MAX_FORWARDS : %d\n", maxForwards);

    int branchTimeout = -1;
    configDb.get("SIP_PROXY_BRANCH_TIMEOUT", branchTimeout);
    if(branchTimeout < 4)
    {
        branchTimeout = 24;
    }

    UtlBoolean authEnabled = TRUE;
    UtlString authServerEnabled;
    configDb.get("SIP_PROXY_USE_AUTH_SERVER", authServerEnabled);
    authServerEnabled.toLower();
    if(authServerEnabled.compareTo("disable") == 0)
    {
        authEnabled = FALSE;
    }
    OsSysLog::add(FAC_SIP, PRI_INFO, "SIP_PROXY_USE_AUTH_SERVER : %s", authEnabled ? "ENABLE" : "DISABLE");
    osPrintf("SIP_PROXY_USE_AUTH_SERVER : %s\n", authEnabled ? "ENABLE" : "DISABLE");

    UtlString authServer;
    configDb.get("SIP_PROXY_AUTH_SERVER", authServer);
    if(authEnabled &&
       authServer.isNull())
    {
        authServer = ipAddress;
        authServer.append(":5080");
    }
    OsSysLog::add(FAC_SIP, PRI_INFO, "SIP_PROXY_AUTH_SERVER : %s", authServer.data());
    osPrintf("SIP_PROXY_AUTH_SERVER : %s\n", authServer.data());

    int defaultExpires;
    int defaultSerialExpires;
    configDb.get("SIP_PROXY_DEFAULT_EXPIRES", defaultExpires);
    configDb.get("SIP_PROXY_DEFAULT_SERIAL_EXPIRES", defaultSerialExpires);
    if(defaultExpires <= 0 ||
       defaultExpires > 180) defaultExpires = 180;
    if(defaultSerialExpires <= 0 ||
       defaultSerialExpires >= defaultExpires) defaultSerialExpires = 20;
    OsSysLog::add(FAC_SIP, PRI_INFO, "SIP_PROXY_DEFAULT_EXPIRES : %d", defaultExpires);
    osPrintf("SIP_PROXY_DEFAULT_EXPIRES : %d\n", defaultExpires);
    OsSysLog::add(FAC_SIP, PRI_INFO, "SIP_PROXY_DEFAULT_SERIAL_EXPIRES : %d", defaultSerialExpires);
    osPrintf("SIP_PROXY_DEFAULT_SERIAL_EXPIRES : %d\n", defaultSerialExpires);

    // Set the maximum amount of time that TCP connections can
    // stay around when they are not used.
    int      staleTcpTimeout = 3600;
    UtlString staleTcpTimeoutStr;

    // Check for missing parameter or empty value
    configDb.get("SIP_PROXY_STALE_TCP_TIMEOUT", staleTcpTimeoutStr);
    if (staleTcpTimeoutStr.isNull())
    {
        staleTcpTimeout = 3600;
    }
    else
    {
        // get the parameter value as an integer
        configDb.get("SIP_PROXY_STALE_TCP_TIMEOUT", staleTcpTimeout);
    }

    if(staleTcpTimeout <= 0) staleTcpTimeout = -1;
    else if(staleTcpTimeout < 180) staleTcpTimeout = 180;
    OsSysLog::add(FAC_SIP, PRI_INFO, "SIP_PROXY_STALE_TCP_TIMEOUT : %d",
                  staleTcpTimeout);
    osPrintf("SIP_PROXY_STALE_TCP_TIMEOUT : %d\n", staleTcpTimeout);

    int maxNumSrvRecords = -1;
    configDb.get("SIP_PROXY_DNSSRV_MAX_DESTS", maxNumSrvRecords);
    OsSysLog::add(FAC_SIP, PRI_INFO, "SIP_PROXY_DNSSRV_MAX_DESTS : %d",
              maxNumSrvRecords);
    // If explicitly set to a valid number
    if(maxNumSrvRecords > 0)
    {
        osPrintf("SIP_PROXY_DNSSRV_MAX_DESTS : %d\n", maxNumSrvRecords);
    }
    else
    {
        maxNumSrvRecords = 4;
    }

    int dnsSrvTimeout = -1; //seconds
    configDb.get("SIP_PROXY_DNSSRV_TIMEOUT", dnsSrvTimeout);
        OsSysLog::add(FAC_SIP, PRI_INFO, "SIP_PROXY_DNSSRV_TIMEOUT : %d",
                  dnsSrvTimeout);
    // If explicitly set to a valid number
    if(dnsSrvTimeout > 0)
    {
        osPrintf("SIP_PROXY_DNSSRV_TIMEOUT : %d\n", dnsSrvTimeout);
    }
    else
    {
        dnsSrvTimeout = 4;
    }

    UtlString hostAliases;
    configDb.get("SIP_PROXY_HOST_ALIASES", hostAliases);
    if(hostAliases.isNull())
    {
        hostAliases = ipAddress;
        char portBuf[20];
        sprintf(portBuf, ":%d", proxyUdpPort);
        hostAliases.append(portBuf);
    }
    OsSysLog::add(FAC_SIP, PRI_INFO, "SIP_PROXY_HOST_ALIASES : %s",
                  hostAliases.data());
    osPrintf("SIP_PROXY_HOST_ALIASES : %s\n", hostAliases.data());

    UtlString enableCallStateObserverSetting;
    configDb.get(CONFIG_SETTING_CALL_STATE, enableCallStateObserverSetting);

    bool enableCallStateLogObserver;
    if (   (enableCallStateObserverSetting.isNull())
        || (0== enableCallStateObserverSetting.compareTo("disable", UtlString::ignoreCase))
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
       OsSysLog::add(FAC_SIP, PRI_ERR, "SipForkingProxyMain invalid configuration value for "
                     CONFIG_SETTING_CALL_STATE " '%s' - should be 'enable' or 'disable'",
                     enableCallStateObserverSetting.data()
                     );
    }
    OsSysLog::add(FAC_SIP, PRI_INFO, CONFIG_SETTING_CALL_STATE " : %s",
                  enableCallStateLogObserver ? "ENABLE" : "DISABLE" );

    UtlString callStateLogFileName;
    if (enableCallStateLogObserver)
    {
       configDb.get(CONFIG_SETTING_CALL_STATE_LOG, callStateLogFileName);
       if (callStateLogFileName.isNull())
       {
          callStateLogFileName = CALL_STATE_LOG_FILE_DEFAULT;
       }
       OsSysLog::add(FAC_SIP, PRI_INFO, CONFIG_SETTING_CALL_STATE_LOG " : %s",
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
       OsSysLog::add(FAC_SIP, PRI_ERR, "SipForkingProxyMain:: invalid configuration value for "
                     "%s '%s' - should be 'enable' or 'disable'", CONFIG_SETTING_CALL_STATE_DB,
                     enableCallStateDbObserverSetting.data()
                     );
    }
    OsSysLog::add(FAC_SIP, PRI_INFO, "%s : %s", CONFIG_SETTING_CALL_STATE_DB,
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
       OsSysLog::add(FAC_SIP, PRI_INFO, "%s : %s", CONFIG_SETTING_CALL_STATE_DB_HOST,
                     callStateDbHostName.data());
                     
       configDb.get(CONFIG_SETTING_CALL_STATE_DB_NAME, callStateDbName);
       if (callStateDbName.isNull())
       {
          callStateDbName = CALL_STATE_DATABASE_NAME;
       }
       OsSysLog::add(FAC_SIP, PRI_INFO, "%s : %s",  CONFIG_SETTING_CALL_STATE_DB_NAME,
                     callStateDbName.data());
                     
       configDb.get(CONFIG_SETTING_CALL_STATE_DB_USER, callStateDbUserName);
       if (callStateDbUserName.isNull())
       {
          callStateDbUserName = CALL_STATE_DATABASE_USER;
       }
       OsSysLog::add(FAC_SIP, PRI_INFO, "%s : %s", CONFIG_SETTING_CALL_STATE_DB_USER,
                     callStateDbUserName.data());                                          
                     
       configDb.get(CONFIG_SETTING_CALL_STATE_DB_DRIVER, callStateDbDriver);
       if (callStateDbDriver.isNull())
       {
          callStateDbDriver = CALL_STATE_DATABASE_DRIVER;
       }
       OsSysLog::add(FAC_SIP, PRI_INFO, "%s : %s",  CONFIG_SETTING_CALL_STATE_DB_DRIVER,
                     callStateDbDriver.data());                          
    }    
    
    // Select logging method - database takes priority over XML file
    if (enableCallStateLogObserver && enableCallStateDbObserver)
    {
       enableCallStateLogObserver = false;
       OsSysLog::add(FAC_SIP, PRI_WARNING, "SipForkingProxyMain:: both XML and database call state "
                     "logging was enabled - turning off XML log, only use database logging");       
    }    
    // This is an obnoxious special option to work around a 
    // problem with Sonus gateways.  The Sonus proxy or  redirect
    // server gives a list of possible gateways to recurse in a
    // 300 response.  It does not assign any Q values so the proxy
    // gets the impression that it should fork them all in parallel.
    // When this option is enabled we recurse only the one with the
    // highest Q value.
    UtlString recurseOnlyOne300String;
    configDb.get("SIP_PROXY_SPECIAL_300", recurseOnlyOne300String);
    recurseOnlyOne300String.toLower();
    UtlBoolean recurseOnlyOne300 = FALSE;
    if(recurseOnlyOne300String.compareTo("enable") == 0) 
    {
        recurseOnlyOne300 = TRUE;
        OsSysLog::add(FAC_SIP, PRI_INFO, "SIP_PROXY_SPECIAL_300 : ENABLE");
        osPrintf("SIP_PROXY_SPECIAL_300 : ENABLE\n");
    }

    // Get the mapped and local domains
    OsConfigDb mappedDomains ;
    configDb.getSubHash("SIP_DOMAINS.", mappedDomains);
    if(mappedDomains.isEmpty())
    {
        //UtlString proxydomain(ipAddress);
        //proxydomain.append(":5060");
        //UtlString registryDomain(ipAddress);
        //registryDomain.append(":4000");
        //mappedDomains.set(proxydomain, registryDomain.data());
    }
    else
    {
        OsSysLog::add(FAC_SIP, PRI_WARNING, "WARNING: SIP_DOMAINS. parameters IGNORED");
    }

    // Initialize the domaim mapping from the routeRules XML
    // file
    //OsConfigDb mapRulesDb;
   

    OsPath fileName = SipXecsService::Path(SipXecsService::ConfigurationDirType,
                                           FORWARDING_RULES_FILENAME);

    ForwardRules forwardingRules;

    OsFile ruleFile(fileName);
    UtlBoolean useDefaultRules = FALSE;
    if(ruleFile.exists())
    {
        if(OS_SUCCESS != forwardingRules.loadMappings(fileName))
        {
            OsSysLog::add(FAC_SIP, PRI_WARNING, "WARNING: Failed to load: %s",
                fileName.data());
            osPrintf("WARNING: Failed to load: %s\n", fileName.data());
            useDefaultRules = TRUE;
        }
    }
    else
    {
        OsSysLog::add(FAC_SIP, PRI_INFO, "forwarding rules '%s' not found",
            fileName.data());
    }


#ifdef TEST_PRINT
    { // scope the test stuff
        SipMessage foo;
        const char* uri = "sip:10.1.20.3:5100";
        const char* method = "ACK"; //SIP_SUBSCRIBE_METHOD;
        const char* eventType = "sip-config"; //SIP_EVENT_CONFIG;
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
    SipUserAgent sipUserAgent(proxyTcpPort, 
        proxyUdpPort,
        proxyTlsPort,
        NULL, // public IP address (nopt used in proxy)
        NULL, // default user (not used in proxy)
        NULL, // default SIP address (not used in proxy)
        (authEnabled && !authServer.isNull()) ? authServer.data() : NULL,
        NULL, // directory server
        NULL, // registry server
        NULL, // auth scheme
        NULL, //auth realm
        NULL, // auth DB
        NULL, // auth user IDs
        NULL, // auth passwords
        NULL, // nat ping URL
        0, // nat ping frequency
        "PING", // nat ping method
        NULL, // line mgr
        SIP_DEFAULT_RTT, // first resend timeout
        TRUE, // default to UA transaction
        SIPUA_DEFAULT_SERVER_UDP_BUFFER_SIZE, // socket layer read buffer size
        SIPUA_DEFAULT_SERVER_OSMSG_QUEUE_SIZE // OsServerTask message queue size
        );
    sipUserAgent.setIsUserAgent(FALSE);
    sipUserAgent.setUserAgentHeaderProperty("sipX/forkingproxy");
    sipUserAgent.setMaxForwards(maxForwards);
    sipUserAgent.setDnsSrvTimeout(dnsSrvTimeout);
    sipUserAgent.setMaxSrvRecords(maxNumSrvRecords);

    sipUserAgent.setDefaultExpiresSeconds(defaultExpires);
    sipUserAgent.setDefaultSerialExpiresSeconds(defaultSerialExpires);
    sipUserAgent.setMaxTcpSocketIdleTime(staleTcpTimeout);
    sipUserAgent.setHostAliases(hostAliases);
    sipUserAgent.setRecurseOnlyOne300Contact(recurseOnlyOne300);
    sipUserAgent.start();

    UtlString buffer;

    // Create and start a router to route stuff either
    // to a local server or on out to the real world
    SipRouter router(sipUserAgent, 
                    forwardingRules,
                    authEnabled, 
                    authServer.data(), 
                    recordRouteEnabled);
    router.start();

    ForkingProxyCseObserver* cseObserver = NULL;
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
       sipUserAgent.getViaInfo(protocol, domainName, port);

       char portString[12];
       sprintf(portString,":%d", port);
       domainName.append(portString);
       
       // and start the observer
       cseObserver = new ForkingProxyCseObserver(sipUserAgent, domainName, pEventWriter);
       cseObserver->start();
    }
    else
    {
      // Only log error if any event logging was enabled
      if (enableCallStateLogObserver || enableCallStateDbObserver)
      {
         OsSysLog::add(FAC_SIP, PRI_ERR,
                       "SipForkingProxyMain:: EventWriter could not be allocated"
                      );
         enableCallStateLogObserver = false;
         enableCallStateDbObserver = false;
      }
    }   
    
    // Do not exit, let the proxy do its stuff
    while( !gShutdownFlag )
    {
      if( interactiveSet)
      {
         int charCode = getchar();

         if(charCode != '\n' && charCode != '\r')
         {
            if( charCode == 'e')
            {
               OsSysLog::enableConsoleOutput(TRUE);
            }
            else if( charCode == 'd')
            {
               OsSysLog::enableConsoleOutput(FALSE);
            }
#ifdef BOUNDS_CHECKER
            else if( charCode == 'b')
            {
              NMMemPopup( );
            }
#endif
            else
            {
               sipUserAgent.printStatus();
               sipUserAgent.getMessageLog(buffer);
               printf("=================>\n%s\n", buffer.data());
            }
         }
      }
      else
         OsTask::delay(2000);
    }

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
    
    // Flush the log file
    OsSysLog::flush();

    return(1);
}
