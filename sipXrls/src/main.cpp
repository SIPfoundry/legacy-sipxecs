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
#include "ResourceListServer.h"
#include "main.h"
#include <net/SipLine.h>
#include <net/SipLineMgr.h>
#include <net/HttpMessage.h>
#include <sipXecsService/SipXecsService.h>
#include <sipXecsService/daemon.h>
#include <sipdb/EntityDB.h>

// DEFINES
#include "config.h"

// SIPXRLS User ID
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

UtlBoolean    gShutdownFlag = FALSE;


/* ============================ FUNCTIONS ================================= */


// Initialize the OsSysLog
void initSysLog(OsConfigDb* pConfig)
{
   UtlString logLevel;               // Controls Log Verbosity
   UtlString consoleLogging;         // Enable console logging by default?
   UtlString fileTarget;             // Path to store log file.
   UtlBoolean bSpecifiedDirError ;   // Set if the specified log dir does not
                                    // exist
   Os::LoggerHelper::instance().processName = "sipxrls";

   //
   // Get/Apply Log Filename
   //
   fileTarget.remove(0);
   if ((pConfig->get(CONFIG_SETTING_LOG_DIR, fileTarget) != OS_SUCCESS) ||
      fileTarget.isNull() || !OsFileSystem::exists(fileTarget))
   {
      bSpecifiedDirError = !fileTarget.isNull();

      // If the log file directory exists use that, otherwise place the log
      // in the current directory
      OsPath workingDirectory;
      if (OsFileSystem::exists(CONFIG_LOG_DIR))
      {
         fileTarget = CONFIG_LOG_DIR;
         OsPath path(fileTarget);
         path.getNativePath(workingDirectory);

         osPrintf("%s : %s\n", CONFIG_SETTING_LOG_DIR, workingDirectory.data());
         Os::Logger::instance().log(LOG_FACILITY, PRI_INFO, "%s : %s",
                       CONFIG_SETTING_LOG_DIR, workingDirectory.data());
      }
      else
      {
         OsPath path;
         OsFileSystem::getWorkingDirectory(path);
         path.getNativePath(workingDirectory);

         osPrintf("%s : %s\n", CONFIG_SETTING_LOG_DIR, workingDirectory.data());
         Os::Logger::instance().log(LOG_FACILITY, PRI_INFO, "%s : %s",
                       CONFIG_SETTING_LOG_DIR, workingDirectory.data());
      }

      fileTarget = workingDirectory +
         OsPathBase::separator +
         CONFIG_LOG_FILE;
   }
   else
   {
      bSpecifiedDirError = false;
      osPrintf("%s : %s\n", CONFIG_SETTING_LOG_DIR, fileTarget.data());
      Os::Logger::instance().log(LOG_FACILITY, PRI_INFO, "%s : %s",
                    CONFIG_SETTING_LOG_DIR, fileTarget.data());

      fileTarget = fileTarget +
         OsPathBase::separator +
         CONFIG_LOG_FILE;
   }


   //
   // Get/Apply Log Level
   //
   SipXecsService::setLogPriority(*pConfig, CONFIG_SETTING_PREFIX);
   Os::Logger::instance().setLoggingPriorityForFacility(FAC_SIP_INCOMING_PARSED, PRI_ERR);
   Os::LoggerHelper::instance().initialize(fileTarget);

   //
   // Get/Apply console logging
   //
   UtlBoolean bConsoleLoggingEnabled = false;
   if ((pConfig->get(CONFIG_SETTING_LOG_CONSOLE, consoleLogging) == OS_SUCCESS))
   {
      consoleLogging.toUpper();
      if (consoleLogging == "ENABLE")
      {
         Os::Logger::instance().enableConsoleOutput(true);
         bConsoleLoggingEnabled = true;
      }
   }

   osPrintf("%s : %s\n", CONFIG_SETTING_LOG_CONSOLE,
            bConsoleLoggingEnabled ? "ENABLE" : "DISABLE") ;
   Os::Logger::instance().log(LOG_FACILITY, PRI_INFO, "%s : %s", CONFIG_SETTING_LOG_CONSOLE,
                 bConsoleLoggingEnabled ? "ENABLE" : "DISABLE") ;

   if (bSpecifiedDirError)
   {
      Os::Logger::instance().log(LOG_FACILITY, PRI_CRIT,
                    "Cannot access %s directory; please check configuration.",
                    CONFIG_SETTING_LOG_DIR);
   }
}

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

void signal_handler(int sig) {
    switch(sig) {
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
    }
}

//
// The main entry point to sipXrls.
//
int main(int argc, char* argv[])
{
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

   OsMsgQShared::setQueuePreference(OsMsgQShared::QUEUE_UNLIMITED);

   // Configuration Database (used for OsSysLog)
   OsConfigDb configDb;

   // Load configuration file.
   OsPath workingDirectory;
   if (OsFileSystem::exists(CONFIG_ETC_DIR))
   {
      workingDirectory = CONFIG_ETC_DIR;
      OsPath path(workingDirectory);
      path.getNativePath(workingDirectory);
   }
   else
   {
      OsPath path;
      OsFileSystem::getWorkingDirectory(path);
      path.getNativePath(workingDirectory);
   }

   UtlString fileName =  workingDirectory +
      OsPathBase::separator +
      CONFIG_SETTINGS_FILE;

   if (configDb.loadFromFile(fileName) != OS_SUCCESS)
   {
      fprintf(stderr, "Failed to load config DB from file '%s'",
              fileName.data());
      exit(1);
   }

   // Initialize log file
   initSysLog(&configDb);

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
   mongo::ConnectionString mongoConnectionString = MongoDB::ConnectionInfo::connectionStringFromFile();
   if (false == MongoDB::ConnectionInfo::testConnection(mongoConnectionString, errmsg))
   {
       Os::Logger::instance().log(LOG_FACILITY, PRI_CRIT,
               "Failed to connect to '%s' - %s",
               mongoConnectionString.toString().c_str(), errmsg.c_str());

       return 1;
   }

   mongoConnectionString = MongoDB::ConnectionInfo::connectionStringFromFile();
   EntityDB entityDb(MongoDB::ConnectionInfo(mongoConnectionString, EntityDB::NS));

   // add the ~~sipXrls credentials so that sipXrls can respond to challenges
   SipLineMgr* lineMgr = addCredentials(domainName, realm, entityDb);
   if(NULL == lineMgr)
   {
      return 1;
   }

   if (!gShutdownFlag)
   {
      // Initialize the ResourceListServer.
      mongoConnectionString = MongoDB::ConnectionInfo::connectionStringFromFile();
	  SubscribeDB subscribeDb(MongoDB::ConnectionInfo(mongoConnectionString, SubscribeDB::NS));
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
                             subscribeDb,
                             entityDb);
      rls.start();

      // Loop forever until signaled to shut down
      while (!Os::UnixSignals::instance().isTerminateSignalReceived() && !gShutdownFlag)
      {
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

   //
   // Terminate the timer thread
   //
   OsTimer::terminateTimerService();

   // Say goodnight Gracie...
   Os::Logger::instance().log(FAC_SIP, PRI_NOTICE, "Exiting") ;
   Os::Logger::instance().flush();

   mongo::dbexit(mongo::EXIT_CLEAN);

   return 0;
}

// Stub to avoid pulling in ps library
int JNI_LightButton(long)
{
   return 0;
}
