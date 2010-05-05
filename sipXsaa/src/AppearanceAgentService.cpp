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
#include "config/sipxsaa-buildstamp.h"
#include "net/SipLine.h"
#include "net/SipLineMgr.h"
#include "os/OsSysLog.h"
#include "sipdb/CredentialDB.h"
#include "sipXecsService/SipXecsService.h"
#include "AppearanceAgentService.h"

// DEFINES

// SIPXSAA User ID
#define SAASERVER_ID_TOKEN            "~~id~sipXsaa"

#define CONFIG_SETTING_UDP_PORT                  "SIP_SAA_UDP_PORT"
#define CONFIG_SETTING_TCP_PORT                  "SIP_SAA_TCP_PORT"
#define CONFIG_SETTING_BIND_IP                   "SIP_SAA_BIND_IP"
#define CONFIG_SETTING_SAA_FILE                  "SIP_SAA_FILE_NAME"
#define CONFIG_SETTING_DOMAIN_NAME               "SIP_SAA_DOMAIN_NAME"
#define CONFIG_SETTING_AUTHENTICATE_REALM        "SIP_SAA_AUTHENTICATE_REALM"
#define CONFIG_SETTING_REFRESH_INTERVAL          "SIP_SAA_REFRESH_INTERVAL"
#define CONFIG_SETTING_RESUBSCRIBE_INTERVAL      "SIP_SAA_RESUBSCRIBE_INTERVAL"
#define CONFIG_SETTING_MIN_RESUBSCRIBE_INTERVAL  "SIP_SAA_MIN_RESUBSCRIBE_INTERVAL"
#define CONFIG_SETTING_SEIZED_RESUBSCRIBE_INTERVAL  "SIP_SAA_SEIZED_RESUBSCRIBE_INTERVAL"
#define CONFIG_SETTING_SERVER_MIN_EXPIRATION     "SIP_SAA_SERVER_MIN_EXPIRATION"
#define CONFIG_SETTING_SERVER_DEFAULT_EXPIRATION "SIP_SAA_SERVER_DEFAULT_EXPIRATION"
#define CONFIG_SETTING_SERVER_MAX_EXPIRATION     "SIP_SAA_SERVER_MAX_EXPIRATION"

#define SAA_DEFAULT_UDP_PORT          5170       // Default UDP port
#define SAA_DEFAULT_TCP_PORT          5170       // Default TCP port
#define SAA_DEFAULT_BIND_IP           "0.0.0.0"  // Default bind ip; all interfaces
#define SAA_DEFAULT_REFRESH_INTERVAL  (24 * 60 * 60) // Default subscription refresh interval.
#define SAA_DEFAULT_RESUBSCRIBE_INTERVAL (60 * 60) // Default subscription resubscribe interval.
#define SAA_DEFAULT_MIN_RESUBSCRIBE_INTERVAL (40 * 60) // Default minimum subscription resubscribe interval.
#define SAA_PUBLISH_DELAY             100        // Delay time (in milliseconds) before publishing RLMI.
#define SAA_DEFAULT_SERVER_MIN_EXPIRATION 32     // Default minimum expiration to be given by subscribe server
#define SAA_DEFAULT_SERVER_DEFAULT_EXPIRATION 3600 // Default default expiration to be given by subscribe server
#define SAA_DEFAULT_SERVER_MAX_EXPIRATION 86400   // Default maximum expiration to be given by subscribe server
#define SAA_DEFAULT_SEIZED_RESUBSCRIBE_INTERVAL (5 * 60)      // Shorter expiration while line is "seized"

#define LOG_FACILITY                  FAC_SAA

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const UtlString SAA_DEFAULT_APPEARANCE_GROUP_FILE = "appearance-groups.xml";
// STRUCTS
// TYPEDEFS
// FUNCTIONS
// FORWARD DECLARATIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// GLOBAL VARIABLE INITIALIZATIONS


/* ============================ FUNCTIONS ================================= */


// Get and add the credentials for sipXsaa
SipLineMgr* addCredentials (UtlString domain, UtlString realm)
{
   SipLineMgr* lineMgr = NULL;
   UtlString user;

   CredentialDB* credentialDb;
   if ((credentialDb = CredentialDB::getInstance()))
   {
      Url identity;

      identity.setUserId(SAASERVER_ID_TOKEN);
      identity.setHostAddress(domain);
      UtlString ha1_authenticator;
      UtlString authtype;
      bool bSuccess = false;

      if (credentialDb->getCredential(identity, realm, user, ha1_authenticator, authtype))
      {
         if ((lineMgr = new SipLineMgr()))
         {
            SipLine line(identity // user entered url
                         ,identity // identity url
                         ,user     // user
                         ,TRUE     // visible
                         ,SipLine::LINE_STATE_PROVISIONED
                         ,TRUE     // auto enable
                         ,FALSE    // use call handling
               );
            if (lineMgr->addLine(line))
            {
               lineMgr->startLineMgr();
               if (lineMgr->addCredentialForLine( identity, realm, user, ha1_authenticator
                                                 ,HTTP_DIGEST_AUTHENTICATION
                      )
                   )
               {
                  lineMgr->setDefaultOutboundLine(identity);
                  bSuccess = true;

                  OsSysLog::add(LOG_FACILITY, PRI_INFO,
                                "Added identity '%s': user='%s' realm='%s'"
                                ,identity.toString().data(), user.data(), realm.data()
                     );
               }
               else
               {
                  OsSysLog::add(LOG_FACILITY, PRI_CRIT,
                                "Error adding identity '%s': user='%s' realm='%s'\n",
                                identity.toString().data(), user.data(), realm.data()
                     );
               }
            }
            else
            {
               OsSysLog::add(LOG_FACILITY, PRI_CRIT, "addLine failed" );
               }
            }
         else
         {
            OsSysLog::add(LOG_FACILITY, PRI_CRIT,
                          "Constructing SipLineMgr failed" );
         }
      }
      else
      {
         OsSysLog::add(LOG_FACILITY, PRI_CRIT,
                       "No credential found for '%s' in realm '%s'"
                       ,identity.toString().data(), realm.data()
                       );
      }

      if( !bSuccess )
      {
         delete lineMgr;
         lineMgr = NULL;
      }
   }

   credentialDb->releaseInstance();

   return lineMgr;
}

//
// Create the service wrapper.  It doesn't do much until run is called.
//
AppearanceAgentService::AppearanceAgentService(
      const char* serviceName, const char* servicePrefix, const char* version
      ) :
   SipXecsService(serviceName, servicePrefix, version)
{
   OsSysLog::setLoggingPriorityForFacility(FAC_SIP_INCOMING_PARSED, PRI_ERR);
}

AppearanceAgentService::~AppearanceAgentService()
{
}

//
// Pull out important parameters from the config DB.
//
UtlBoolean AppearanceAgentService::loadConfig(
      int& udpPort,
      int& tcpPort,
      UtlString& bindIp,
      UtlString& appearanceGroupFile,
      UtlString& domainName,
      UtlString& realm,
      int& refreshInterval,
      int& resubscribeInterval,
      int& minResubscribeInterval,
      int& seizedResubscribeInterval,
      int& serverMinExpiration,
      int& serverDefaultExpiration,
      int& serverMaxExpiration
      )
{
   UtlBoolean ret = true;

   // SipXecsService owns the main configDb
   OsConfigDb& configDb = getConfigDb();

   // Read the user agent parameters from the config file.
   if (configDb.get(CONFIG_SETTING_UDP_PORT, udpPort) != OS_SUCCESS)
   {
      udpPort = SAA_DEFAULT_UDP_PORT;
   }

   if (configDb.get(CONFIG_SETTING_TCP_PORT, tcpPort) != OS_SUCCESS)
   {
      tcpPort = SAA_DEFAULT_TCP_PORT;
   }

    if (configDb.get(CONFIG_SETTING_BIND_IP, bindIp) != OS_SUCCESS ||
            !OsSocket::isIp4Address(bindIp))
        bindIp = SAA_DEFAULT_BIND_IP;

   if ((configDb.get(CONFIG_SETTING_SAA_FILE, appearanceGroupFile) !=
        OS_SUCCESS) ||
       appearanceGroupFile.isNull())
   {
      OsSysLog::add(LOG_FACILITY, PRI_WARNING,
                    "Appearance group file name is not configured - using default (%s)",
                    SAA_DEFAULT_APPEARANCE_GROUP_FILE.data());
      appearanceGroupFile = SAA_DEFAULT_APPEARANCE_GROUP_FILE;
   }

   if ((configDb.get(CONFIG_SETTING_DOMAIN_NAME, domainName) !=
        OS_SUCCESS) ||
       domainName.isNull())
   {
      OsSysLog::add(LOG_FACILITY, PRI_CRIT,
                    "Resource domain name is not configured");
      ret = false;
   }

   if ((configDb.get(CONFIG_SETTING_AUTHENTICATE_REALM, realm) !=
        OS_SUCCESS) ||
       realm.isNull())
   {
      OsSysLog::add(LOG_FACILITY, PRI_CRIT,
                    "Resource realm is not configured");
      ret = false;
   }

   if (configDb.get(CONFIG_SETTING_REFRESH_INTERVAL, refreshInterval) != OS_SUCCESS)
   {
      refreshInterval = SAA_DEFAULT_REFRESH_INTERVAL;
   }

   if (configDb.get(CONFIG_SETTING_RESUBSCRIBE_INTERVAL, resubscribeInterval) != OS_SUCCESS)
   {
      resubscribeInterval = SAA_DEFAULT_RESUBSCRIBE_INTERVAL;
   }

   if (configDb.get(CONFIG_SETTING_MIN_RESUBSCRIBE_INTERVAL, minResubscribeInterval) != OS_SUCCESS)
   {
       minResubscribeInterval = SAA_DEFAULT_MIN_RESUBSCRIBE_INTERVAL;
   }

   // shorter expiration while the shared line is "seized" by a set
   if (configDb.get(CONFIG_SETTING_SEIZED_RESUBSCRIBE_INTERVAL, seizedResubscribeInterval) != OS_SUCCESS)
   {
      seizedResubscribeInterval = SAA_DEFAULT_SEIZED_RESUBSCRIBE_INTERVAL;
   }

   if (configDb.get(CONFIG_SETTING_SERVER_MIN_EXPIRATION, serverMinExpiration) != OS_SUCCESS)
   {
       serverMinExpiration = SAA_DEFAULT_SERVER_MIN_EXPIRATION;
   }

   if (configDb.get(CONFIG_SETTING_SERVER_DEFAULT_EXPIRATION, serverDefaultExpiration) != OS_SUCCESS)
   {
       serverDefaultExpiration = SAA_DEFAULT_SERVER_DEFAULT_EXPIRATION;
   }

   if (configDb.get(CONFIG_SETTING_SERVER_MAX_EXPIRATION, serverMaxExpiration) != OS_SUCCESS)
   {
       serverMaxExpiration = SAA_DEFAULT_SERVER_MAX_EXPIRATION;
   }

   return ret;
}

//
// Create the real Appearance Agent, passing in the configured parameters
//
void AppearanceAgentService::run()
{
   int udpPort;
   int tcpPort;
   UtlString bindIp;
   UtlString appearanceGroupFile;
   UtlString domainName;
   UtlString realm;
   int refreshInterval;
   int resubscribeInterval;
   int minResubscribeInterval;
   int seizedResubscribeInterval;
   int serverMinExpiration;
   int serverDefaultExpiration;
   int serverMaxExpiration;

   loadConfig(udpPort,
         tcpPort,
         bindIp,
         appearanceGroupFile,
         domainName,
         realm,
         refreshInterval,
         resubscribeInterval,
         minResubscribeInterval,
         seizedResubscribeInterval,
         serverMinExpiration,
         serverDefaultExpiration,
         serverMaxExpiration);

   // add the ~~sipXsaa credentials so that sipXsaa can respond to challenges
   SipLineMgr* lineMgr = addCredentials(domainName, realm);
   if (NULL == lineMgr)
   {
      OsSysLog::add(FAC_SAA, PRI_CRIT, "failed to add SAA credentials - exiting");
      setShutdownFlag(true);
   }

   if (!getShutdownFlag())
   {
      // Initialize the AppearanceAgent.
      // (Use tcpPort as the TLS port, too.)
      mAppearanceAgent = new AppearanceAgent(this, domainName, realm, lineMgr,
                          tcpPort, udpPort, tcpPort, bindIp,
                          &appearanceGroupFile,
                          refreshInterval,
                          resubscribeInterval, minResubscribeInterval, seizedResubscribeInterval,
                          SAA_PUBLISH_DELAY,
                          20,
                          serverMinExpiration,
                          serverDefaultExpiration,
                          serverMaxExpiration);
      mAppearanceAgent->start();

      // Loop forever until signaled to shut down
      while (!getShutdownFlag())
      {
         OsTask::delay(2 * OsTime::MSECS_PER_SEC);
      }

      // Shut down the server.
      mAppearanceAgent->shutdown();
   }

   // Delete the LineMgr Object
   delete lineMgr;
}

void AppearanceAgentService::configDbChanged(UtlString& configDbFile)
{
   // call the super's configDbChanged, which loads the new DB and adjusts the log level
   SipXecsService::configDbChanged(configDbFile);
   // now load changes specific to this service
}

// Note that this method is called in the StdinListener thread, so the implementation must be threadsafe.
void AppearanceAgentService::resourceChanged(UtlString& fileType, UtlString& configFile)
{
   // load changes specific to this service
   OsSysLog::add(FAC_KERNEL, PRI_INFO,
                 "AppearanceAgentService::resourceChanged: %s (type %s)",
                 configFile.data(), fileType.data() );

   // See if the list configuration file has changed.
   mAppearanceAgent->getAppearanceGroupFileReader().refresh();
}

