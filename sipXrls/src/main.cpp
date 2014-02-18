//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <signal.h>

// APPLICATION INCLUDES
#include <os/OsConfigDb.h>
#include <os/OsLogger.h>
#include <os/OsFS.h>
#include <os/OsSocket.h>
#include <os/UnixSignals.h>
#include <os/OsTimer.h>
#include <xmlparser/tinystr.h>
#include "xmlparser/ExtractContent.h"
#include <utl/UtlString.h>
#include <utl/XmlContent.h>
#include <net/NameValueTokenizer.h>
#include <net/SipDialogEvent.h>
#include <os/OsLogger.h>
#include <os/OsLoggerHelper.h>
#include <os/OsMsgQ.h>
#include <os/OsResourceLimit.h>
#include "ResourceListServer.h"
#include <net/SipLine.h>
#include <net/SipLineMgr.h>
#include <net/HttpMessage.h>
#include <sipXecsService/SipXecsService.h>
#include <sipXecsService/daemon.h>
#include <sipdb/EntityDB.h>

#include "sipXecsService/SipXApplication.h"

// DEFINES
#include "config.h"

// SIPXRLS User ID
#define SIPXRLS_APP_NAME              "sipxrls"
#define RLSSERVER_ID_TOKEN            "~~id~sipXrls"
#define CONFIG_SETTINGS_FILE          "sipxrls-config"
#define CONFIG_ETC_DIR                SIPX_CONFDIR

#define CONFIG_LOG_FILE               "sipxrls.log"
#define CONFIG_LOG_DIR                SIPX_LOGDIR

#define CONFIG_SETTING_PREFIX         "SIP_RLS"
#define CONFIG_SETTING_LOG_DIR        "SIP_RLS_LOG_DIR"
#define CONFIG_SETTING_LOG_CONSOLE    "SIP_RLS_LOG_CONSOLE"
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

#define LOG_FACILITY                  FAC_RLS

#define RLS_DEFAULT_UDP_PORT          5130       // Default UDP port
#define RLS_DEFAULT_TCP_PORT          5130       // Default TCP port
#define RLS_DEFAULT_BIND_IP           "0.0.0.0"  // Default bind ip; all interfaces
#define RLS_DEFAULT_RESUBSCRIBE_INTERVAL (60 * 60) // Default subscription resubscribe interval.
#define RLS_DEFAULT_MIN_RESUBSCRIBE_INTERVAL (40 * 60) // Default minimum subscription resubscribe interval.
#define RLS_PUBLISH_DELAY             100        // Delay time (in milliseconds) before publishing RLMI.
#define RLS_DEFAULT_SERVER_MIN_EXPIRATION 32     // Default minimum expiration to be given by subscribe server
#define RLS_DEFAULT_SERVER_DEFAULT_EXPIRATION 3600 // Default default expiration to be given by subscribe server
#define RLS_DEFAULT_SERVER_MAX_EXPIRATION 86400   // Default maximum expiration to be given by subscribe server

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
SipLineMgr* addCredentials (UtlString domain, UtlString realm, EntityDB& entityDb)
{
   SipLine* line = NULL;
   SipLineMgr* lineMgr = NULL;
   UtlString user;

   Url identity;

  identity.setUserId(RLSSERVER_ID_TOKEN);
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
        // lineMgr does not take ownership of *line, so we have to delete it.
        delete line;
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
// The main entry point to sipXrls.
//
int main(int argc, char* argv[])
{
  SipXApplicationData rlsData =
  {
      SIPXRLS_APP_NAME,
      CONFIG_SETTINGS_FILE,
      CONFIG_LOG_FILE,
      "",
      CONFIG_SETTING_PREFIX,
      true, // check mongo connection
      true, // increase application file descriptor limits
      SipXApplicationData::ConfigFileFormatConfigDb, // format type for configuration file
      OsMsgQShared::QUEUE_UNLIMITED,
  };

  // NOTE: this might exit application in case of failure
  SipXApplication::instance().init(argc, argv, rlsData);

  const OsConfigDb& configDb = SipXApplication::instance().getConfig().getOsConfigDb();

  // Read the user agent parameters from the config file.
  int udpPort;
  if (configDb.get(CONFIG_SETTING_UDP_PORT, udpPort) != OS_SUCCESS)
  {
    udpPort = RLS_DEFAULT_UDP_PORT;
  }

  int tcpPort;
  if (configDb.get(CONFIG_SETTING_TCP_PORT, tcpPort) != OS_SUCCESS)
  {
    tcpPort = RLS_DEFAULT_TCP_PORT;
  }

  UtlString bindIp;
  if (configDb.get(CONFIG_SETTING_BIND_IP, bindIp) != OS_SUCCESS ||
      !OsSocket::isIp4Address(bindIp))
    bindIp = RLS_DEFAULT_BIND_IP;

  UtlString resourceListFile;
  if ((configDb.get(CONFIG_SETTING_RLS_FILE, resourceListFile) !=
      OS_SUCCESS) ||
      resourceListFile.isNull())
  {
    Os::Logger::instance().log(LOG_FACILITY, PRI_CRIT,
        "Resource list file name is not configured");
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

   int resubscribeInterval;
   if (configDb.get(CONFIG_SETTING_RESUBSCRIBE_INTERVAL, resubscribeInterval) != OS_SUCCESS)
   {
      resubscribeInterval = RLS_DEFAULT_RESUBSCRIBE_INTERVAL;
   }

   int minResubscribeInterval;
   if (configDb.get(CONFIG_SETTING_MIN_RESUBSCRIBE_INTERVAL, minResubscribeInterval) != OS_SUCCESS)
   {
       minResubscribeInterval = RLS_DEFAULT_MIN_RESUBSCRIBE_INTERVAL;
   }

   int serverMinExpiration;
   if (configDb.get(CONFIG_SETTING_SERVER_MIN_EXPIRATION, serverMinExpiration) != OS_SUCCESS)
   {
       serverMinExpiration = RLS_DEFAULT_SERVER_MIN_EXPIRATION;
   }

   int serverDefaultExpiration;
   if (configDb.get(CONFIG_SETTING_SERVER_DEFAULT_EXPIRATION, serverDefaultExpiration) != OS_SUCCESS)
   {
       serverDefaultExpiration = RLS_DEFAULT_SERVER_DEFAULT_EXPIRATION;
   }

   int serverMaxExpiration;
   if (configDb.get(CONFIG_SETTING_SERVER_MAX_EXPIRATION, serverMaxExpiration) != OS_SUCCESS)
   {
       serverMaxExpiration = RLS_DEFAULT_SERVER_MAX_EXPIRATION;
   }

   std::string errmsg;
   MongoDB::ConnectionInfo gInfo = MongoDB::ConnectionInfo::globalInfo();

   EntityDB entityDb(gInfo);

   // add the ~~sipXrls credentials so that sipXrls can respond to challenges
   SipLineMgr* lineMgr = addCredentials(domainName, realm, entityDb);
   if(NULL == lineMgr)
   {
     SipXApplication::instance().terminate();
     return 1;
   }
   SubscribeDB* subscribeDb = NULL;


   if (!SipXApplication::instance().terminationRequested())
   {
      // Initialize the ResourceListServer.
      subscribeDb = SubscribeDB::CreateInstance();
      ResourceListServer rls(domainName, realm, lineMgr,
                             DIALOG_EVENT_TYPE, DIALOG_EVENT_CONTENT_TYPE,
                             tcpPort, udpPort, PORT_NONE, bindIp,
                             &resourceListFile,
                             resubscribeInterval, minResubscribeInterval,
                             RLS_PUBLISH_DELAY,
                             20, 20, 20, 20,
                             serverMinExpiration,
                             serverDefaultExpiration,
                             serverMaxExpiration,
                             *subscribeDb,
                             entityDb);
      rls.start();

      // Loop forever until signaled to shut down
      while (!SipXApplication::instance().terminationRequested())
      {
        //std::cout << SipXApplication::instance().terminationRequested() <<std::endl;
         OsTask::delay(2000);
         // See if the list configuration file has changed.
         rls.getResourceListFileReader().refresh();
      }

      // Shut down the server.
      rls.shutdown();
   }

   lineMgr->requestShutdown();
   
   while (!lineMgr->isShutDown())
   {
      OsTask::delay(100);
   }
   
   // Delete the LineMgr Object
   delete lineMgr;

   if (subscribeDb)
     delete subscribeDb;

   SipXApplication::instance().terminate();

   return 0;
}

// Stub to avoid pulling in ps library
int JNI_LightButton(long)
{
  return 0;
}
