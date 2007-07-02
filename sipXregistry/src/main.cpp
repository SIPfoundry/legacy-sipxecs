// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <stdio.h>
#include <signal.h>
#include <iostream>

// APPLICATION INCLUDES
#ifndef SIPX_VERSION
#  include "sipxregistry-buildstamp.h"
#  define SIPXCHANGE_VERSION SipXregistryVersion
#  define SIPXCHANGE_VERSION_COMMENT SipXregistryBuildStamp
#else
#  define SIPXCHANGE_VERSION SIPX_VERSION
#  define SIPXCHANGE_VERSION_COMMENT ""
#endif

#include "os/OsConfigDb.h"
#include "os/OsFS.h"
#include "os/OsTime.h"
#include "os/OsTask.h"

#include "net/NameValueTokenizer.h"
#include "sipdb/SIPDBManager.h"
#include "sipdb/ExtensionDB.h"
#include "registry/SipRegistrar.h"

// DEFINES
#define CONFIG_SETTINGS_FILE  "registrar-config"
#define CONFIG_LOG_FILE       "sipregistrar.log"
#define CONFIG_LOG_DIR        SIPX_LOGDIR
#define CONFIG_ETC_DIR        SIPX_CONFDIR

#define CONFIG_SETTING_LOG_LEVEL      "SIP_REGISTRAR_LOG_LEVEL"
#define CONFIG_SETTING_LOG_CONSOLE    "SIP_REGISTRAR_LOG_CONSOLE"
#define CONFIG_SETTING_LOG_DIR        "SIP_REGISTRAR_LOG_DIR"
#define LOG_FACILITY                  FAC_SIP

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
typedef void (*sighandler_t)(int);

using namespace std ;

// FUNCTIONS
extern "C" {
   void  sigHandler( int sig_num );
   sighandler_t pt_signal( int sig_num, sighandler_t handler );
}

// FORWARD DECLARATIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// GLOBAL VARIABLE INITIALIZATIONS
OsServerTask* pServerTask   = NULL;
UtlBoolean     gShutdownFlag = FALSE;
UtlBoolean     gClosingIMDB  = FALSE;
OsMutex*       gpLockMutex = new OsMutex(OsMutex::Q_FIFO);

/* ============================ FUNCTIONS ================================= */

/**
 * Description:
 * closes any open connections to the IMDB safely using a mutex lock
 */
void
closeIMDBConnections ()
{
   // Critical Section here
   OsLock lock( *gpLockMutex );
   SIPDBManager* db = SIPDBManager::getInstance();
   db->releaseAllDatabase();

   delete db;
}

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

   // Minimize the chance that we loose log data
   if (SIGTERM == sig_num)
   {
      OsSysLog::add( LOG_FACILITY, PRI_INFO, "sigHandler: terminate signal received.");
   }
   else
   {
      OsSysLog::add( LOG_FACILITY, PRI_CRIT, "sigHandler: caught signal: %d", sig_num );
   }
   OsSysLog::flush();
}

// Initialize the OsSysLog
void
initSysLog(OsConfigDb* pConfig)
{
   UtlString logLevel;               // Controls Log Verbosity
   UtlString consoleLogging;         // Enable console logging by default?
   UtlString fileTarget;             // Path to store log file.
   UtlBoolean bSpecifiedDirError ;   // Set if the specified log dir does not
   // exist
   struct tagPriorityLookupTable
   {
      const char*      pIdentity;
      OsSysLogPriority ePriority;
   };

   struct tagPriorityLookupTable lkupTable[] =
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
   OsSysLog::initialize(0, "SipRegistrar");


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
         OsSysLog::add(LOG_FACILITY, PRI_INFO, "%s : %s", CONFIG_SETTING_LOG_DIR, workingDirectory.data()) ;
      }
      else
      {
         OsPath path;
         OsFileSystem::getWorkingDirectory(path);
         path.getNativePath(workingDirectory);

         osPrintf("%s : %s\n", CONFIG_SETTING_LOG_DIR, workingDirectory.data()) ;
         OsSysLog::add(LOG_FACILITY, PRI_INFO, "%s : %s", CONFIG_SETTING_LOG_DIR, workingDirectory.data()) ;
      }

      fileTarget = workingDirectory +
         OsPathBase::separator +
         CONFIG_LOG_FILE;
   }
   else
   {
      bSpecifiedDirError = false ;
      osPrintf("%s : %s\n", CONFIG_SETTING_LOG_DIR, fileTarget.data()) ;
      OsSysLog::add(LOG_FACILITY, PRI_INFO, "%s : %s", CONFIG_SETTING_LOG_DIR, fileTarget.data()) ;

      fileTarget = fileTarget +
         OsPathBase::separator +
         CONFIG_LOG_FILE;
   }
   OsSysLog::setOutputFile(0, fileTarget) ;


   //
   // Get/Apply Log Level
   //
   if ((pConfig->get(CONFIG_SETTING_LOG_LEVEL, logLevel) != OS_SUCCESS) ||
       logLevel.isNull())
   {
      logLevel = "NOTICE";
   }
   logLevel.toUpper();
   OsSysLogPriority priority = PRI_ERR;
   int iEntries = sizeof(lkupTable)/sizeof(struct tagPriorityLookupTable);
   for (int i=0; i<iEntries; i++)
   {
      if (logLevel == lkupTable[i].pIdentity)
      {
         priority = lkupTable[i].ePriority;
         osPrintf("%s : %s\n", CONFIG_SETTING_LOG_LEVEL, lkupTable[i].pIdentity) ;
         OsSysLog::add(LOG_FACILITY, PRI_INFO, "%s : %s", CONFIG_SETTING_LOG_LEVEL, lkupTable[i].pIdentity) ;
         break;
      }
   }
   OsSysLog::setLoggingPriority(priority);
   OsSysLog::setLoggingPriorityForFacility(FAC_SIP_INCOMING_PARSED, PRI_ERR);

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
         OsSysLog::enableConsoleOutput(true);
         bConsoleLoggingEnabled = true ;
      }
   }

   OsSysLog::add(LOG_FACILITY, PRI_INFO, "%s : %s", CONFIG_SETTING_LOG_CONSOLE,
                 bConsoleLoggingEnabled ? "ENABLE" : "DISABLE") ;

   if (bSpecifiedDirError)
   {
      OsSysLog::add(FAC_LOG, PRI_CRIT,
                    "Cannot access directory '%s'; please check configuration.",
                    CONFIG_SETTING_LOG_DIR);
   }
}

/** The main entry point to the sipregistrar */
int
main(int argc, char* argv[] )
{
   // Configuration Database (used for OsSysLog)
   OsConfigDb* configDb = new OsConfigDb();

   // Register Signal handlers to close IMDB
   pt_signal(SIGINT,   sigHandler);    // Trap Ctrl-C on NT
   pt_signal(SIGILL,   sigHandler);
   // do not catch SIGABRT - allow assert() to core dump
   pt_signal(SIGFPE,   sigHandler);    // Floading Point Exception
   pt_signal(SIGSEGV,  sigHandler);    // Address access violations signal 11
   pt_signal(SIGTERM,  sigHandler);    // Trap kill -15 on UNIX
#if defined(__pingtel_on_posix__)
   pt_signal(SIGHUP,   sigHandler);    // Hangup
   pt_signal(SIGQUIT,  sigHandler);
   pt_signal(SIGPIPE,  SIG_IGN);    // Handle TCP Failure
   pt_signal(SIGBUS,   sigHandler);
   pt_signal(SIGSYS,   sigHandler);
   pt_signal(SIGXCPU,  sigHandler);
   pt_signal(SIGXFSZ,  sigHandler);
   pt_signal(SIGUSR1,  sigHandler);
   pt_signal(SIGUSR2,  sigHandler);
#endif

   UtlBoolean interactiveSet = false;
   UtlString argString;
   for (int argIndex = 1; argIndex < argc; argIndex++)
   {
      osPrintf("arg[%d]: %s\n", argIndex, argv[argIndex]);
      argString = argv[argIndex];
      NameValueTokenizer::frontBackTrim(&argString, "\t ");
      if (argString.compareTo("-v") == 0)
      {
         osPrintf("Version: %s (%s)\n", SIPXCHANGE_VERSION, SIPXCHANGE_VERSION_COMMENT);
         return(1);
      }
      else if ( argString.compareTo("-i") == 0)
      {
         interactiveSet = true;
         osPrintf("Entering Interactive Mode\n");
      }
      else
      {
         osPrintf("usage: %s [-v] [-i]\nwhere:\n -v provides the software version\n"
                  " -i start the server in an interactive mode\n",
                  argv[0]);
         return(1);
      }
   }

   // initialize log file
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

   bool configLoaded = ( configDb->loadFromFile(fileName) == OS_SUCCESS );
   if (!configLoaded)
   {
      configDb->set("SIP_REGISTRAR_UDP_PORT", "5070");
      configDb->set("SIP_REGISTRAR_TCP_PORT", "5070");
      configDb->set("SIP_REGISTRAR_TLS_PORT", "5071");
      configDb->set("SIP_REGISTRAR_PROXY_PORT", "5060");
      configDb->set("SIP_REGISTRAR_MAX_EXPIRES", "");
      configDb->set("SIP_REGISTRAR_MIN_EXPIRES", "");
      configDb->set("SIP_REGISTRAR_DOMAIN_NAME", "");
      //configDb->set("SIP_REGISTRAR_AUTHENTICATE_SCHEME", "");
      configDb->set("SIP_REGISTRAR_AUTHENTICATE_ALGORITHM", "");
      configDb->set("SIP_REGISTRAR_AUTHENTICATE_QOP", "");
      configDb->set("SIP_REGISTRAR_AUTHENTICATE_REALM", "");

      configDb->set("SIP_REGISTRAR_MEDIA_SERVER", "");
      configDb->set("SIP_REGISTRAR_VOICEMAIL_SERVER", "");
      configDb->set(CONFIG_SETTING_LOG_DIR, "");
      configDb->set(CONFIG_SETTING_LOG_LEVEL, "");
      configDb->set(CONFIG_SETTING_LOG_CONSOLE, "");
   }

   initSysLog(configDb) ;
   OsSysLog::add(FAC_SIP, PRI_NOTICE,
                 "SipRegistrar >>>>>>>>>>>>>>>> STARTED"
                 );
   if (configLoaded)
   {
      OsSysLog::add(FAC_SIP, PRI_INFO, "Read config %s", fileName.data());
   }
   else
   {
      if (configDb->storeToFile(fileName) == OS_SUCCESS)
      {
         OsSysLog::add( FAC_SIP, PRI_INFO, "Default config written to: %s",
                       fileName.data()
                       );
      }
      else
      {
         OsSysLog::add( FAC_SIP, PRI_ERR, "Default config write failed to: %s",
                       fileName.data());
      }
   }
      
   // Fetch Pointer to the OsServer task object, note that
   // object uses the IMDB so it is important to shut this thread
   // cleanly before the signal handler exits
   SipRegistrar* registrar = SipRegistrar::getInstance(configDb);

   registrar->start();

   pServerTask = static_cast<OsServerTask*>(registrar);

   // Do not exit, let the services run...
   while( !gShutdownFlag )
   {
      OsTask::delay(1 * OsTime::MSECS_PER_SEC);
   }
   OsSysLog::add(LOG_FACILITY, PRI_NOTICE, "main: cleaning up.");

   // This is a server task so gracefully shut down the
   // server task using the waitForShutdown method, this
   // will implicitly request a shutdown for us if one is
   // not already in progress
   if ( pServerTask != NULL )
   {
      // Deleting a server task is the only way of
      // waiting for shutdown to complete cleanly
      OsSysLog::add(LOG_FACILITY, PRI_DEBUG, "main: shut down server task.");
      delete pServerTask;
      pServerTask = NULL;
   }

   // now deregister this process's database references from the IMDB
   closeIMDBConnections();

   if ( configDb != NULL )
   {
      delete configDb;
   }

   // Flush the log file
   OsSysLog::flush();

   return 0;
}


// The infamous JNI_LightButton stub, to resolve the reference in libsipXcall.
int JNI_LightButton(long)
{
   return 0;
}
