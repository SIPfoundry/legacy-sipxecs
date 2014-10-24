//
//
// Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <signal.h>

// APPLICATION INCLUDES
#include "net/NameValueTokenizer.h"
#include "net/SipLine.h"
#include "net/SipLineMgr.h"
#include "net/HttpMessage.h"
#include "os/OsConfigDb.h"
#include "os/OsLogger.h"
#include "os/OsLoggerHelper.h"
#include "os/OsTimer.h"
#include "os/UnixSignals.h"
#include "os/OsMsgQ.h"
#include "os/OsResourceLimit.h"
#include "sipdb/EntityDB.h"
#include "sipXecsService/SipXecsService.h"
#include "utl/UtlString.h"
#include "xmlparser/tinystr.h"
#include "AppearanceAgent.h"
#include <sipXecsService/daemon.h>
#include <os/OsExceptionHandler.h>
#include <sipXecsService/SipXApplication.h>

#include <sipXecsService/SipXApplication.h>


// DEFINES
#include "config.h"

// SIPXSAA User ID
#define SIPXSAA_APP_NAME              "sipxsaa"
#define SAASERVER_ID_TOKEN            "~~id~sipXsaa"
#define CONFIG_SETTINGS_FILE          "sipxsaa-config"
#define CONFIG_ETC_DIR                SIPX_CONFDIR

#define CONFIG_LOG_FILE               "sipxsaa.log"
#define CONFIG_LOG_DIR                SIPX_LOGDIR

#define CONFIG_SETTING_PREFIX                    "SIP_SAA"
#define CONFIG_SETTING_LOG_DIR                   "SIP_SAA_LOG_DIR"
#define CONFIG_SETTING_LOG_CONSOLE               "SIP_SAA_LOG_CONSOLE"
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

#define LOG_FACILITY                  FAC_SAA

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

// Get and add the credentials for sipXsaa
SipLineMgr* addCredentials (EntityDB& entityDb, UtlString domain, UtlString realm)
{
   SipLine* line = NULL;
   SipLineMgr* lineMgr = NULL;
   UtlString user;


      Url identity;

      identity.setUserId(SAASERVER_ID_TOKEN);
      identity.setHostAddress(domain);
      UtlString ha1_authenticator;
      UtlString authtype;
      bool bSuccess = false;

      if (entityDb.getCredential(identity, realm, user, ha1_authenticator, authtype))
      {
         if ((line = new SipLine( identity // user entered url
                                 ,identity // identity url
                                 ,user     // user
                                 ,TRUE     // visible
                                 ,SipLine::LINE_STATE_PROVISIONED
                                 ,TRUE     // auto enable
                                 ,FALSE    // use call handling
                                 )))
         {
            if ((lineMgr = new SipLineMgr()))
            {
               if (lineMgr->addLine(*line))
               {
                  if (lineMgr->addCredentialForLine( identity, realm, user, ha1_authenticator
                                                    ,HTTP_DIGEST_AUTHENTICATION
                                                    )
                      )
                  {
                     lineMgr->setDefaultOutboundLine(identity);
                     bSuccess = true;

                     Os::Logger::instance().log(LOG_FACILITY, PRI_INFO,
                                   "Added identity '%s': user='%s' realm='%s'"
                                   ,identity.toString().data(), user.data(), realm.data()
                                   );
                  }
                  else
                  {
                     Os::Logger::instance().log(LOG_FACILITY, PRI_CRIT,
                                   "Error adding identity '%s': user='%s' realm='%s'\n",
                                   identity.toString().data(), user.data(), realm.data()
                                   );
                  }
               }
               else
               {
                  Os::Logger::instance().log(LOG_FACILITY, PRI_CRIT, "addLine failed" );
               }
            }
            else
            {
               Os::Logger::instance().log(LOG_FACILITY, PRI_CRIT,
                             "Constructing SipLineMgr failed" );
            }
         }
         else
         {
            Os::Logger::instance().log(LOG_FACILITY, PRI_CRIT,
                          "Constructing SipLine failed" );
         }
      }
      else
      {
         Os::Logger::instance().log(LOG_FACILITY, PRI_CRIT,
                       "No credential found for '%s' in realm '%s'"
                       ,identity.toString().data(), realm.data()
                       );
      }

      if( !bSuccess )
      {
         delete line;
         line = NULL;

         delete lineMgr;
         lineMgr = NULL;
      }


   return lineMgr;
}

//
// The main entry point to sipXsaa.
//
int main(int argc, char* argv[])
{
  SipXApplicationData saaData =
  {
      SIPXSAA_APP_NAME,
      CONFIG_SETTINGS_FILE,
      CONFIG_LOG_FILE,
      "",
      CONFIG_SETTING_PREFIX,
      true, // check mongo connection
      true, // enable mongo driver logging
      true, // increase application file descriptor limits
      true, // block signals on main thread (and all other threads created by main)
            // and process them only on a dedicated thread
      SipXApplicationData::ConfigFileFormatConfigDb, // format type for configuration file
      OsMsgQShared::QUEUE_UNLIMITED,
  };

  // NOTE: this might exit application in case of failure
  SipXApplication::instance().init(argc, argv, saaData);

  OsConfigDb& configDb = SipXApplication::instance().getConfig().getOsConfigDb();

   // Read the user agent parameters from the config file.
   int udpPort;
   if (configDb.get(CONFIG_SETTING_UDP_PORT, udpPort) != OS_SUCCESS)
   {
      udpPort = SAA_DEFAULT_UDP_PORT;
   }

   int tcpPort;
   if (configDb.get(CONFIG_SETTING_TCP_PORT, tcpPort) != OS_SUCCESS)
   {
      tcpPort = SAA_DEFAULT_TCP_PORT;
   }

    UtlString bindIp;
    if (configDb.get(CONFIG_SETTING_BIND_IP, bindIp) != OS_SUCCESS ||
            !OsSocket::isIp4Address(bindIp))
        bindIp = SAA_DEFAULT_BIND_IP;

   UtlString appearanceGroupFile;
   if ((configDb.get(CONFIG_SETTING_SAA_FILE, appearanceGroupFile) !=
        OS_SUCCESS) ||
       appearanceGroupFile.isNull())
   {
      Os::Logger::instance().log(LOG_FACILITY, PRI_CRIT,
                    "Appearance group file name is not configured");
      return 1;
   }

   UtlString domainName;
   if ((configDb.get(CONFIG_SETTING_DOMAIN_NAME, domainName) !=
        OS_SUCCESS) ||
       domainName.isNull())
   {
      Os::Logger::instance().log(LOG_FACILITY, PRI_CRIT,
                    "Resource domain name is not configured");
      return 1;
   }

   UtlString realm;
   if ((configDb.get(CONFIG_SETTING_AUTHENTICATE_REALM, realm) !=
        OS_SUCCESS) ||
       realm.isNull())
   {
      Os::Logger::instance().log(LOG_FACILITY, PRI_CRIT,
                    "Resource realm is not configured");
      return 1;
   }

   int refreshInterval;
   if (configDb.get(CONFIG_SETTING_REFRESH_INTERVAL, refreshInterval) != OS_SUCCESS)
   {
      refreshInterval = SAA_DEFAULT_REFRESH_INTERVAL;
   }

   int resubscribeInterval;
   if (configDb.get(CONFIG_SETTING_RESUBSCRIBE_INTERVAL, resubscribeInterval) != OS_SUCCESS)
   {
      resubscribeInterval = SAA_DEFAULT_RESUBSCRIBE_INTERVAL;
   }

   int minResubscribeInterval;
   if (configDb.get(CONFIG_SETTING_MIN_RESUBSCRIBE_INTERVAL, minResubscribeInterval) != OS_SUCCESS)
   {
       minResubscribeInterval = SAA_DEFAULT_MIN_RESUBSCRIBE_INTERVAL;
   }

   // shorter expiration while the shared line is "seized" by a set
   int seizedResubscribeInterval;
   if (configDb.get(CONFIG_SETTING_SEIZED_RESUBSCRIBE_INTERVAL, seizedResubscribeInterval) != OS_SUCCESS)
   {
      seizedResubscribeInterval = SAA_DEFAULT_SEIZED_RESUBSCRIBE_INTERVAL;
   }

   int serverMinExpiration;
   if (configDb.get(CONFIG_SETTING_SERVER_MIN_EXPIRATION, serverMinExpiration) != OS_SUCCESS)
   {
       serverMinExpiration = SAA_DEFAULT_SERVER_MIN_EXPIRATION;
   }

   int serverDefaultExpiration;
   if (configDb.get(CONFIG_SETTING_SERVER_DEFAULT_EXPIRATION, serverDefaultExpiration) != OS_SUCCESS)
   {
       serverDefaultExpiration = SAA_DEFAULT_SERVER_DEFAULT_EXPIRATION;
   }

   int serverMaxExpiration;
   if (configDb.get(CONFIG_SETTING_SERVER_MAX_EXPIRATION, serverMaxExpiration) != OS_SUCCESS)
   {
       serverMaxExpiration = SAA_DEFAULT_SERVER_MAX_EXPIRATION;
   }

   std::string errmsg;
   MongoDB::ConnectionInfo gInfo = MongoDB::ConnectionInfo::globalInfo();

   // add the ~~sipXsaa credentials so that sipXsaa can respond to challenges
   SubscribeDB* subscribeDb = SubscribeDB::CreateInstance();
   EntityDB entityDb(gInfo);

   SipLineMgr* lineMgr = addCredentials(entityDb, domainName, realm);
   if(NULL == lineMgr)
   {
     SipXApplication::instance().terminate();
     return 1;
   }

   if (!SipXApplication::instance().terminationRequested())
   {
      // Initialize the AppearanceAgent.
      AppearanceAgent saa(domainName, realm, lineMgr,
                          tcpPort, udpPort, PORT_NONE, bindIp,
                          &appearanceGroupFile,
                          refreshInterval,
                          resubscribeInterval, minResubscribeInterval, seizedResubscribeInterval,
                          SAA_PUBLISH_DELAY,
                          20,
                          serverMinExpiration,
                          serverDefaultExpiration,
                          serverMaxExpiration,
                          *subscribeDb,
                          entityDb);
      saa.start();

      // Loop forever until signaled to shut down
      while (!SipXApplication::instance().terminationRequested())
      {
         OsTask::delay(2 * OsTime::MSECS_PER_SEC);
         // See if the list configuration file has changed.
         saa.getAppearanceGroupFileReader().refresh();
      }

    // Shut down the server.
    saa.shutdown();
  }

  // Delete the LineMgr Object
  delete lineMgr;

  if (subscribeDb)
    delete subscribeDb;

  SipXApplication::instance().terminate();

  // Say goodnight Gracie...
  return 0;
}

// Stub to avoid pulling in ps library
int JNI_LightButton(long)
{
  return 0;
}
