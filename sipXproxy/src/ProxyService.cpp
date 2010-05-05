//
//
// Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <os/OsFS.h>
#include <os/OsConfigDb.h>
#include <os/OsSocket.h>
#include <os/OsTask.h>
#include <os/OsTimerTask.h>
#include <os/OsStunAgentTask.h>
#include <net/SipMessage.h>
#include <sipdb/SIPDBManager.h>
#include <sipdb/UserForwardDB.h>
#include "sipXecsService/SipXecsService.h"
#include "SipRouter.h"
#include "SipXProxyCseObserver.h"
#include "ProxyService.h"

//uncomment next line to enable bound checker checking with 'b' key
//#define BOUNDS_CHECKER

#ifdef BOUNDS_CHECKER
    #include "D:\Program Files\Compuware\BoundsChecker\ERptApi\apilib.h"
    #pragma comment(lib, "D:\\Program Files\\Compuware\\BoundsChecker\\ERptApi\\nmapi.lib")
#endif

// DEFINES

#define CALL_STATE_LOG_FILE_DEFAULT SIPX_LOGDIR "/sipxproxy_callstate.log"
#define FORWARDING_RULES_FILENAME "forwardingrules.xml"
#define LOG_FACILITY   FAC_SIP

// Configuration names pulled from config-file
#define CONFIG_SETTING_BIND_IP        "SIPX_PROXY_BIND_IP"
#define CONFIG_SETTING_CALL_STATE     "SIPX_PROXY_CALL_STATE"
#define CONFIG_SETTING_CALL_STATE_LOG "SIPX_PROXY_CALL_STATE_LOG"

// Default expiry times (in seconds)
#define DEFAULT_SIP_TRANSACTION_EXPIRES 180
#define DEFAULT_SIP_SERIAL_EXPIRES 20

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const char* CONFIG_SETTING_CALL_STATE_DB =
   "SIPX_PROXY_CALL_STATE_DB";
static const char* CONFIG_SETTING_CALL_STATE_DB_HOST =
   "SIPX_PROXY_CALL_STATE_DB_HOST";
static const char* CONFIG_SETTING_CALL_STATE_DB_NAME =
   "SIPX_PROXY_CALL_STATE_DB_NAME";
static const char* CONFIG_SETTING_CALL_STATE_DB_USER =
   "SIPX_PROXY_CALL_STATE_DB_USER";
static const char* CONFIG_SETTING_CALL_STATE_DB_DRIVER =
   "SIPX_PROXY_CALL_STATE_DB_DRIVER";
static const char* CALL_STATE_DATABASE_HOST =
   "localhost";
static const char* CALL_STATE_DATABASE_NAME =
   "SIPXCDR";
static const char* CALL_STATE_DATABASE_USER =
   POSTGRESQL_USER;
static const char* CALL_STATE_DATABASE_DRIVER =
   "{PostgreSQL}";
// STRUCTS
// TYPEDEFS
// FUNCTIONS
// FORWARD DECLARATIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// GLOBAL VARIABLE INITIALIZATIONS


/* ============================ FUNCTIONS ================================= */

//
// Create the service wrapper.  It doesn't do much until run is called.
//
ProxyService::ProxyService(
      const char* serviceName, const char* servicePrefix, const char* version
      ) :
   SipXecsService(serviceName, servicePrefix, version),
   mClosingIMDB(false),
   mLockMutex(OsMutex::Q_FIFO)
{
}

ProxyService::~ProxyService()
{
}

/**
 * Description:
 * closes any open connections to the IMDB safely using a mutex lock
 */
void
ProxyService::closeIMDBConnections ()
{
    // Critical Section here
    OsLock lock( mLockMutex );

    // now deregister this process's database references from the IMDB
    // and also ensure that we do not cause this code recursively
    // specifically SIGABRT or SIGSEGV could cause problems here
    if ( !mClosingIMDB )
    {
        mClosingIMDB = TRUE;
        // if deleting this causes another problem in this process
        // the gClosingIMDB flag above will protect us
        delete SIPDBManager::getInstance();
    }
}

//
// Pull out important parameters from the config DB.
// These parameters are set at runtime and cannot be changed without a restart.
//
UtlBoolean ProxyService::loadInitialConfig(
      int& proxyTcpPort,
      int& proxyUdpPort,
      int& proxyTlsPort,
      UtlString& bindIp,
      int& maxForwards,
      UtlString& domainName,
      UtlString& proxyRecordRoute,
      UtlString& routeName,
      UtlString& authScheme,
      UtlString& ipAddress,
      bool& enableCallStateLogObserver,
      bool& enableCallStateDbObserver,
      int& dnsSrvTimeout,
      int& maxNumSrvRecords,
      UtlString& callStateLogFileName,
      UtlBoolean& recurseOnlyOne300,
      UtlString& hostAliases,
      int&      staleTcpTimeout,
      UtlString& callStateDbHostName,
      UtlString& callStateDbName,
      UtlString& callStateDbUserName,
      UtlString& callStateDbDriver,
      ForwardRules& forwardingRules
      )
{
   UtlBoolean ret = true;

   // SipXecsService owns the main configDb
   OsConfigDb& configDb = getConfigDb();

    OsSocket::getHostIp(&ipAddress);

    configDb.get(CONFIG_SETTING_BIND_IP, bindIp);
    if ((bindIp.isNull()) || !OsSocket::isIp4Address(bindIp))
    {
       bindIp = "0.0.0.0";
    }
    OsSysLog::add(FAC_SIP, PRI_INFO, "%s: %s", CONFIG_SETTING_BIND_IP,
          bindIp.data());

    UtlString hostname;
    configDb.get("SIPX_PROXY_HOST_NAME", hostname);
    if (!hostname.isNull())
    {
       // bias the selection of SRV records so that if the name of this host is an alternative,
       // it wins in any selection based on random weighting.
       SipSrvLookup::setOwnHostname(hostname);
    }
    OsSysLog::add(FAC_SIP, PRI_INFO, "SIPX_PROXY_HOST_NAME : %s", hostname.data());

    proxyUdpPort = configDb.getPort("SIPX_PROXY_UDP_PORT");
    if (!portIsValid(proxyUdpPort))
    {
       proxyUdpPort = 5060;
    }
    OsSysLog::add(FAC_SIP, PRI_INFO, "SIPX_PROXY_UDP_PORT : %d", proxyUdpPort);
    proxyTcpPort = configDb.getPort("SIPX_PROXY_TCP_PORT") ;
    if (!portIsValid(proxyTcpPort))
    {
       proxyTcpPort = 5060;
    }
    OsSysLog::add(FAC_SIP, PRI_INFO, "SIPX_PROXY_TCP_PORT : %d", proxyTcpPort);
    proxyTlsPort = configDb.getPort("SIPX_PROXY_TLS_PORT") ;
    if (!portIsValid(proxyTlsPort))
    {
       proxyTlsPort = 5061;
    }
    OsSysLog::add(FAC_SIP, PRI_INFO, "SIPX_PROXY_TLS_PORT : %d", proxyTlsPort);

    configDb.get("SIPX_PROXY_MAX_FORWARDS", maxForwards);
    if(maxForwards <= 0) maxForwards = SIP_DEFAULT_MAX_FORWARDS;
    OsSysLog::add(FAC_SIP, PRI_INFO, "SIPX_PROXY_MAX_FORWARDS : %d", maxForwards);

    int branchTimeout = -1;
    configDb.get("SIPX_PROXY_BRANCH_TIMEOUT", branchTimeout);
    if(branchTimeout < 4)
    {
        branchTimeout = 24;
    }

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
       OsSysLog::add(FAC_SIP, PRI_NOTICE,
                     "SIPX_PROXY_HOST_ALIASES not configured"
                     " using implied value: %s",
                     hostAliases.data());
    }
    OsSysLog::add(FAC_SIP, PRI_INFO, "SIPX_PROXY_HOST_ALIASES : %s",
                  hostAliases.data());

    UtlString enableCallStateObserverSetting;
    configDb.get(CONFIG_SETTING_CALL_STATE, enableCallStateObserverSetting);

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
       OsSysLog::add(FAC_SIP, PRI_ERR, "SipXproxymain:: invalid configuration value for "
                     CONFIG_SETTING_CALL_STATE " '%s' - should be 'enable' or 'disable'",
                     enableCallStateObserverSetting.data()
                     );
    }
    OsSysLog::add(FAC_SIP, PRI_INFO, CONFIG_SETTING_CALL_STATE " : %s",
                  enableCallStateLogObserver ? "ENABLE" : "DISABLE" );


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
       OsSysLog::add(FAC_SIP, PRI_ERR, "SipAuthProxyMain:: invalid configuration value for %s "
                     " '%s' - should be 'enable' or 'disable'",
                     CONFIG_SETTING_CALL_STATE_DB, enableCallStateDbObserverSetting.data()
                     );
    }
    OsSysLog::add(FAC_SIP, PRI_INFO, "%s : %s", CONFIG_SETTING_CALL_STATE_DB,
                  enableCallStateDbObserver ? "ENABLE" : "DISABLE" );

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
       OsSysLog::add(FAC_SIP, PRI_WARNING, "SipXproxymain:: both XML and database call state "
                     "logging was enabled - turning off XML log, only use database logging");
    }

    // Set the maximum amount of time that TCP connections can
    // stay around when they are not used.
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
    OsSysLog::add(FAC_SIP, PRI_INFO, "SIPX_PROXY_STALE_TCP_TIMEOUT : %d",
                  staleTcpTimeout);

    configDb.get("SIPX_PROXY_DNSSRV_MAX_DESTS", maxNumSrvRecords);
    OsSysLog::add(FAC_SIP, PRI_INFO, "SIPX_PROXY_DNSSRV_MAX_DESTS : %d",
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

    configDb.get("SIPX_PROXY_DNSSRV_TIMEOUT", dnsSrvTimeout);
    OsSysLog::add(FAC_SIP, PRI_INFO, "SIPX_PROXY_DNSSRV_TIMEOUT : %d",
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
    if(recurseOnlyOne300String.compareTo("enable") == 0)
    {
        recurseOnlyOne300 = TRUE;
        OsSysLog::add(FAC_SIP, PRI_INFO, "SIPX_PROXY_SPECIAL_300 : ENABLE");
    }

    OsPath fileName = SipXecsService::Path(SipXecsService::ConfigurationDirType,
                                            FORWARDING_RULES_FILENAME);


    OsFile ruleFile(fileName);
    UtlBoolean useDefaultRules = FALSE;
    if(ruleFile.exists())
    {
        if(OS_SUCCESS != forwardingRules.loadMappings(fileName))
        {
            OsSysLog::add(FAC_SIP, PRI_WARNING, "WARNING: Failed to load: %s",
                fileName.data());
            useDefaultRules = TRUE;
        }
    }
    else
    {
        // forwardingrules.xml is not readable; no processing is possible.
        OsSysLog::add(FAC_SIP, PRI_CRIT,
                      "forwarding rules '%s' not found -- exiting",
                      fileName.data());
        ret = false;
    }

   return ret;
}

//
// Pull out important parameters from the config DB.
// These parameters take effect immediately.
//
void ProxyService::loadConfig(
    )
{
    int defaultExpires;
    int defaultSerialExpires;

    getConfigDb().get("SIPX_PROXY_DEFAULT_EXPIRES", defaultExpires);
    if(defaultExpires <= 0 )
    {
      defaultExpires = DEFAULT_SIP_TRANSACTION_EXPIRES;
    }
    OsSysLog::add(FAC_SIP, PRI_INFO, "SIPX_PROXY_DEFAULT_EXPIRES : %d", defaultExpires);
    //TODO: this function is not thread-safe
    mpSipUserAgent->setDefaultExpiresSeconds(defaultExpires);

    getConfigDb().get("SIPX_PROXY_DEFAULT_SERIAL_EXPIRES", defaultSerialExpires);
    if(defaultSerialExpires <= 0 ||
       defaultSerialExpires >= defaultExpires)
    {
      defaultSerialExpires = DEFAULT_SIP_SERIAL_EXPIRES;
    }
    OsSysLog::add(FAC_SIP, PRI_INFO, "SIPX_PROXY_DEFAULT_SERIAL_EXPIRES : %d", defaultSerialExpires);
    //TODO: this function is not thread-safe
    mpSipUserAgent->setDefaultSerialExpiresSeconds(defaultSerialExpires);
}

//
// Create the real Server, passing in the configured parameters
//
void ProxyService::run()
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
    bool enableCallStateLogObserver;
    bool enableCallStateDbObserver;
    int dnsSrvTimeout = -1; //seconds
    int maxNumSrvRecords = -1;
    UtlString callStateLogFileName;
    UtlBoolean recurseOnlyOne300 = FALSE;
    UtlString hostAliases;
    int      staleTcpTimeout = 3600;
    UtlString callStateDbHostName;
    UtlString callStateDbName;
    UtlString callStateDbUserName;
    UtlString callStateDbDriver;
    ForwardRules forwardingRules;

    loadInitialConfig(
          proxyTcpPort,
          proxyUdpPort,
          proxyTlsPort,
          bindIp,
          maxForwards,
          domainName,
          proxyRecordRoute,
          routeName,
          authScheme,
          ipAddress,
          enableCallStateLogObserver,
          enableCallStateDbObserver,
          dnsSrvTimeout,
          maxNumSrvRecords,
          callStateLogFileName,
          recurseOnlyOne300,
          hostAliases,
          staleTcpTimeout,
          callStateDbHostName,
          callStateDbName,
          callStateDbUserName,
          callStateDbDriver,
          forwardingRules
          );

   // initialize other pieces
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
    mpSipUserAgent = new SipUserAgent(
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


    if (!mpSipUserAgent->isOk())
    {
        OsSysLog::add(FAC_SIP, PRI_EMERG, "SipUserAgent reported a problem, setting shutdown flag...");
        setShutdownFlag(true);
    }
    mpSipUserAgent->setDnsSrvTimeout(dnsSrvTimeout);
    mpSipUserAgent->setMaxSrvRecords(maxNumSrvRecords);
    mpSipUserAgent->setUserAgentHeaderProperty("sipXecs/sipXproxy");
    mpSipUserAgent->setMaxForwards(maxForwards);
    mpSipUserAgent->setMaxTcpSocketIdleTime(staleTcpTimeout);
    mpSipUserAgent->setHostAliases(hostAliases);
    mpSipUserAgent->setRecurseOnlyOne300Contact(recurseOnlyOne300);
    // load and set configurable parameters
    loadConfig();

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
       mpSipUserAgent->getViaInfo(protocol, domainName, port);

       char portString[12];
       sprintf(portString,":%d", port);
       domainName.append(portString);

       // and start the observer
       cseObserver = new SipXProxyCseObserver(*mpSipUserAgent, domainName, pEventWriter);
       cseObserver->start();
    }
    else
    {
      // Only log error if any event logging was enabled
      if (enableCallStateLogObserver || enableCallStateDbObserver)
      {
         OsSysLog::add(FAC_SIP, PRI_ERR,
                       "SipAuthProxyMain:: EventWriter could not be allocated"
                       );
         enableCallStateLogObserver = false;
         enableCallStateDbObserver = false;
      }
    }

    // Create a router to route stuff either
    // to a local server or on out to the real world
    SipRouter* pRouter = new SipRouter(*mpSipUserAgent,
                                       forwardingRules,
                                       getConfigDb());

    // Start the router running
    pRouter->start();

    // Do not exit, let the proxy do its stuff
    while( !getShutdownFlag() )
    {
      OsTask::delay(2000);
    }

    // This is a server task so gracefully shutdown the
    // router task by deleting it.
    delete pRouter ;

    // Stop the SipUserAgent.
    mpSipUserAgent->shutdown();
    // And delete it, too.
    delete mpSipUserAgent ;

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
    OsTimerTask::destroyTimerTask();
    OsStunAgentTask::releaseInstance();

   // now deregister this process's database references from the IMDB
   closeIMDBConnections();
}

void ProxyService::configDbChanged(UtlString& configDbFile)
{
   // call the super's configDbChanged, which loads the new DB and adjusts the log level
   SipXecsService::configDbChanged(configDbFile);
   // now load changes specific to this service
   // TODO: make the fns called in here threadsafe, then we can load them dynamically
   // loadConfig();
}

// Note that this method is called in the StdinListener thread, so the implementation must be threadsafe.
void ProxyService::resourceChanged(UtlString& fileType, UtlString& configFile)
{
   // load changes specific to this service
   OsSysLog::add(LOG_FACILITY, PRI_INFO,
                 "ProxyService::resourceChanged: %s (type %s)",
                 configFile.data(), fileType.data() );

   //TODO: make any run-time config changes
}

