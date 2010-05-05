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
#include "config/sipxrls-buildstamp.h"
#include "net/SipDialogEvent.h"
#include "net/SipLine.h"
#include "net/SipLineMgr.h"
#include "os/OsSysLog.h"
#include "sipdb/CredentialDB.h"
#include "sipXecsService/SipXecsService.h"
#include "RlsService.h"
#include "ResourceListServer.h"

// DEFINES

// SIPXRLS User ID
#define RLSSERVER_ID_TOKEN            "~~id~sipXrls"

#define CONFIG_SETTING_UDP_PORT       "SIP_RLS_UDP_PORT"
#define CONFIG_SETTING_TCP_PORT       "SIP_RLS_TCP_PORT"
#define CONFIG_SETTING_BIND_IP        "SIP_RLS_BIND_IP"
#define CONFIG_SETTING_RLS_FILE       "SIP_RLS_FILE_NAME"
#define CONFIG_SETTING_DOMAIN_NAME    "SIP_RLS_DOMAIN_NAME"
#define CONFIG_SETTING_AUTHENTICATE_REALM "SIP_RLS_AUTHENTICATE_REALM"
#define CONFIG_SETTING_REFRESH_INTERVAL "SIP_RLS_REFRESH_INTERVAL"
#define CONFIG_SETTING_RESUBSCRIBE_INTERVAL "SIP_RLS_RESUBSCRIBE_INTERVAL"
#define CONFIG_SETTING_MIN_RESUBSCRIBE_INTERVAL "SIP_RLS_MIN_RESUBSCRIBE_INTERVAL"
#define CONFIG_SETTING_SERVER_MIN_EXPIRATION "SIP_RLS_SERVER_MIN_EXPIRATION"
#define CONFIG_SETTING_SERVER_DEFAULT_EXPIRATION "SIP_RLS_SERVER_DEFAULT_EXPIRATION"
#define CONFIG_SETTING_SERVER_MAX_EXPIRATION "SIP_RLS_SERVER_MAX_EXPIRATION"

#define RLS_DEFAULT_UDP_PORT          5130       // Default UDP port
#define RLS_DEFAULT_TCP_PORT          5130       // Default TCP port
#define RLS_DEFAULT_BIND_IP           "0.0.0.0"  // Default bind ip; all interfaces
#define RLS_DEFAULT_RESUBSCRIBE_INTERVAL (60 * 60) // Default subscription resubscribe interval.
#define RLS_DEFAULT_MIN_RESUBSCRIBE_INTERVAL (40 * 60) // Default minimum subscription resubscribe interval.
#define RLS_PUBLISH_DELAY             100        // Delay time (in milliseconds) before publishing RLMI.
#define RLS_DEFAULT_SERVER_MIN_EXPIRATION 32     // Default minimum expiration to be given by subscribe server
#define RLS_DEFAULT_SERVER_DEFAULT_EXPIRATION 3600 // Default default expiration to be given by subscribe server
#define RLS_DEFAULT_SERVER_MAX_EXPIRATION 86400   // Default maximum expiration to be given by subscribe server

#define LOG_FACILITY                  FAC_RLS

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FUNCTIONS
// FORWARD DECLARATIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// GLOBAL VARIABLE INITIALIZATIONS


/* ============================ FUNCTIONS ================================= */

// Get and add the credentials for sipXrls
SipLineMgr* addCredentials (UtlString domain, UtlString realm)
{
   SipLineMgr* lineMgr = NULL;
   UtlString user;

   CredentialDB* credentialDb;
   if ((credentialDb = CredentialDB::getInstance()))
   {
      Url identity;

      identity.setUserId(RLSSERVER_ID_TOKEN);
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
RlsService::RlsService(
      const char* serviceName, const char* servicePrefix, const char* version
      ) :
   SipXecsService(serviceName, servicePrefix, version)
{
   OsSysLog::setLoggingPriorityForFacility(FAC_SIP_INCOMING_PARSED, PRI_ERR);
}

RlsService::~RlsService()
{
}

//
// Pull out important parameters from the config DB.
//
UtlBoolean RlsService::loadConfig(
      int& udpPort,
      int& tcpPort,
      UtlString& bindIp,
      UtlString& resourceListFile,
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
      udpPort = RLS_DEFAULT_UDP_PORT;
   }

   if (configDb.get(CONFIG_SETTING_TCP_PORT, tcpPort) != OS_SUCCESS)
   {
      tcpPort = RLS_DEFAULT_TCP_PORT;
   }

    if (configDb.get(CONFIG_SETTING_BIND_IP, bindIp) != OS_SUCCESS ||
            !OsSocket::isIp4Address(bindIp))
        bindIp = RLS_DEFAULT_BIND_IP;

   if ((configDb.get(CONFIG_SETTING_RLS_FILE, resourceListFile) !=
        OS_SUCCESS) ||
       resourceListFile.isNull())
   {
      OsSysLog::add(LOG_FACILITY, PRI_CRIT,
                    "Resource list file name is not configured");
      ret = false;
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

   if (configDb.get(CONFIG_SETTING_RESUBSCRIBE_INTERVAL, resubscribeInterval) != OS_SUCCESS)
   {
      resubscribeInterval = RLS_DEFAULT_RESUBSCRIBE_INTERVAL;
   }

   if (configDb.get(CONFIG_SETTING_MIN_RESUBSCRIBE_INTERVAL, minResubscribeInterval) != OS_SUCCESS)
   {
       minResubscribeInterval = RLS_DEFAULT_MIN_RESUBSCRIBE_INTERVAL;
   }

   if (configDb.get(CONFIG_SETTING_SERVER_MIN_EXPIRATION, serverMinExpiration) != OS_SUCCESS)
   {
       serverMinExpiration = RLS_DEFAULT_SERVER_MIN_EXPIRATION;
   }

   if (configDb.get(CONFIG_SETTING_SERVER_DEFAULT_EXPIRATION, serverDefaultExpiration) != OS_SUCCESS)
   {
       serverDefaultExpiration = RLS_DEFAULT_SERVER_DEFAULT_EXPIRATION;
   }

   if (configDb.get(CONFIG_SETTING_SERVER_MAX_EXPIRATION, serverMaxExpiration) != OS_SUCCESS)
   {
       serverMaxExpiration = RLS_DEFAULT_SERVER_MAX_EXPIRATION;
   }

   return ret;
}

//
// Create the real Resource List Server, passing in the configured parameters
//
void RlsService::run()
{
   int udpPort;
   int tcpPort;
   UtlString bindIp;
   UtlString resourceListFile;
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
         resourceListFile,
         domainName,
         realm,
         refreshInterval,
         resubscribeInterval,
         minResubscribeInterval,
         seizedResubscribeInterval,
         serverMinExpiration,
         serverDefaultExpiration,
         serverMaxExpiration);

   // add the ~~sipXrls credentials so that sipXrls can respond to challenges
   SipLineMgr* lineMgr = addCredentials(domainName, realm);
   if(NULL == lineMgr)
   {
      OsSysLog::add(LOG_FACILITY, PRI_CRIT, "failed to add RLS credentials - exiting");
      setShutdownFlag(true);
   }

   if (!getShutdownFlag())
   {
      // Initialize the ResourceListServer.
      // (Use tcpPort as the TLS port, too.)
      mResourceListServer = new ResourceListServer(this, domainName, realm, lineMgr,
                             DIALOG_EVENT_TYPE, DIALOG_EVENT_CONTENT_TYPE,
                             tcpPort, udpPort, tcpPort, bindIp,
                             &resourceListFile,
                             resubscribeInterval, minResubscribeInterval,
                             RLS_PUBLISH_DELAY,
                             20, 20, 20, 20,
                             serverMinExpiration,
                             serverDefaultExpiration,
                             serverMaxExpiration);

      mResourceListServer->start();

      // Loop forever until signaled to shut down
      while (!getShutdownFlag())
      {
         OsTask::delay(2 * OsTime::MSECS_PER_SEC);
      }

      // Shut down the server.
      mResourceListServer->shutdown();
   }

   // Delete the LineMgr Object
   delete lineMgr;
}

void RlsService::configDbChanged(UtlString& configDbFile)
{
   // call the super's configDbChanged, which loads the new DB and adjusts the log level
   SipXecsService::configDbChanged(configDbFile);
   // now load changes specific to this service
}

// Note that this method is called in the StdinListener thread, so the implementation must be threadsafe.
void RlsService::resourceChanged(UtlString& fileType, UtlString& configFile)
{
   // load changes specific to this service
   OsSysLog::add(LOG_FACILITY, PRI_INFO,
                 "RlsService::resourceChanged: %s (type %s)",
                 configFile.data(), fileType.data() );

   // See if the list configuration file has changed.
   mResourceListServer->getResourceListFileReader().refresh();
}

