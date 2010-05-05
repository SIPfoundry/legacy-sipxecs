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
#include <cp/SipPresenceMonitor.h>
#include <net/SipUserAgent.h>
#include <os/OsFS.h>
#include <os/OsSysLog.h>
#include <os/OsConfigDb.h>
#include <persist/SipPersistentSubscriptionMgr.h>
#include <ptapi/PtProvider.h>
#include "sipXecsService/SipXecsService.h"
#include "AcdPresenceService.h"

// DEFINES
#define CONFIG_SETTING_DOMAIN_NAME    "SIP_PRESENCE_DOMAIN_NAME"
#define CONFIG_SETTING_UDP_PORT       "SIP_PRESENCE_UDP_PORT"
#define CONFIG_SETTING_TCP_PORT       "SIP_PRESENCE_TCP_PORT"
#define CONFIG_SETTING_BIND_IP        "SIP_PRESENCE_BIND_IP"

#define PRESENCE_DEFAULT_UDP_PORT              5130       // Default UDP port
#define PRESENCE_DEFAULT_TCP_PORT              5130       // Default TCP port
#define PRESENCE_DEFAULT_BIND_IP               "0.0.0.0"  // Default bind ip; all interfaces

#define LOG_FACILITY                  FAC_ACD

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// The name of the persistent file in which we save the presence information,
// relative to SIPX_VARDIR.
static const UtlString sPersistentFileName("sipxpresence/state.xml");

// GLOBAL VARIABLE INITIALIZATIONS


/* ============================ FUNCTIONS ================================= */

//
// Create the service wrapper.  It doesn't do much until run is called.
//
AcdPresenceService::AcdPresenceService(
      const char* serviceName, const char* servicePrefix, const char* version
      ) :
   SipXecsService(serviceName, servicePrefix, version)
{
}

AcdPresenceService::~AcdPresenceService()
{
}


//
// Pull out important parameters from the config DB.
// These parameters are set at runtime and cannot be changed without a restart.
//
UtlBoolean AcdPresenceService::loadConfig(
    int& UdpPort,
    int& TcpPort,
    UtlString& bindIp,
    UtlString&   domain
    )
{
   UtlBoolean ret = true;

   // SipXecsService owns the main configDb
   OsConfigDb& configDb = getConfigDb();

   // Read the user agent parameters from the config file.
   if (configDb.get(CONFIG_SETTING_UDP_PORT, UdpPort) != OS_SUCCESS)
   {
      UdpPort = PRESENCE_DEFAULT_UDP_PORT;
   }

   if (configDb.get(CONFIG_SETTING_TCP_PORT, TcpPort) != OS_SUCCESS)
   {
      TcpPort = PRESENCE_DEFAULT_TCP_PORT;
   }

   if (configDb.get(CONFIG_SETTING_BIND_IP, bindIp) != OS_SUCCESS ||
         !OsSocket::isIp4Address(bindIp))
   {
      bindIp = PRESENCE_DEFAULT_BIND_IP;
   }

   UtlString domainName;
   configDb.get(CONFIG_SETTING_DOMAIN_NAME, domainName);
   // Normalize SIP domain name to all lower case.
   domainName.toLower();

   return ret;
}

//
// Create the real Server, passing in the configured parameters
//
void AcdPresenceService::run()
{
    int UdpPort;
    int TcpPort;
    UtlString bindIp;
    UtlString   domainName;

   loadConfig(
       UdpPort,
       TcpPort,
       bindIp,
       domainName
       );

   // initialize other pieces

   // Bind the SIP user agent to a port and start it up
   SipUserAgent* userAgent = new SipUserAgent(TcpPort, UdpPort, PORT_NONE,
         NULL, NULL, bindIp );
   userAgent->start();

   if (!userAgent->isOk())
   {
      OsSysLog::add(LOG_FACILITY, PRI_EMERG, "SipUserAgent failed to initialize; requesting shutdown");
      setShutdownFlag(true);
   }

   // Create the SipPersistentSubscriptionMgr.
   SipPersistentSubscriptionMgr subscriptionMgr(SUBSCRIPTION_COMPONENT_PRESENCE,
                                                domainName);

   // Determine the name of the persistent file.
   UtlString pathName = SipXecsService::Path(SipXecsService::VarDirType,
                                             sPersistentFileName.data());

   // Create the Sip Presence Monitor, which handles all of the processing.
   SipPresenceMonitor presenceMonitor(userAgent,
                                      &subscriptionMgr,
                                      domainName,
                                      TcpPort,
                                      &getConfigDb(),
                                      true,
                                      pathName.data());

   // Loop forever until signaled to shut down
   while (!getShutdownFlag())
   {
      OsTask::delay(2000);
   }

   // Shut down the sipUserAgent
   userAgent->shutdown(FALSE);

   while(!userAgent->isShutdownDone())
   {
      OsTask::delay(100);
   }

   delete userAgent;

}

void AcdPresenceService::configDbChanged(UtlString& configDbFile)
{
   // call the super's configDbChanged, which loads the new DB and adjusts the log level
   SipXecsService::configDbChanged(configDbFile);
   // now load changes specific to this service
}

// Note that this method is called in the StdinListener thread, so the implementation must be threadsafe.
void AcdPresenceService::resourceChanged(UtlString& fileType, UtlString& configFile)
{
   // load changes specific to this service
   OsSysLog::add(LOG_FACILITY, PRI_INFO,
                 "AcdPresenceService::resourceChanged: %s (type %s)",
                 configFile.data(), fileType.data() );

   //TODO: make any run-time config changes
}

