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
#include "config.h"
#include "os/OsConfigDb.h"
#include "os/OsFS.h"
#include "os/OsTime.h"
#include "os/OsTask.h"

#include "net/NameValueTokenizer.h"
#include "sipdb/SIPDBManager.h"
#include "sipdb/ExtensionDB.h"
#include "sipXecsService/SipXecsService.h"
#include "registry/SipRegistrar.h"

// DEFINES
#define CONFIG_SETTINGS_FILE  "registrar-config"
#define CONFIG_LOG_FILE       "sipregistrar.log"
#define CONFIG_LOG_DIR        SIPX_LOGDIR
#define CONFIG_ETC_DIR        SIPX_CONFDIR

#define CONFIG_SETTING_PREFIX         "SIP_REGISTRAR"
#define CONFIG_SETTING_LOG_CONSOLE    "SIP_REGISTRAR_LOG_CONSOLE"
#define CONFIG_SETTING_LOG_DIR        "SIP_REGISTRAR_LOG_DIR"
#define LOG_FACILITY                  FAC_SIP

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

using namespace std ;

// FUNCTIONS
// FORWARD DECLARATIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// GLOBAL VARIABLE INITIALIZATIONS
OsServerTask* pServerTask   = NULL;
UtlBoolean     gShutdownFlag = FALSE;
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

// Initialize the OsSysLog
void
initSysLog(OsConfigDb* pConfig)
{
   UtlString logLevel;               // Controls Log Verbosity
   UtlString consoleLogging;         // Enable console logging by default?
   UtlString fileTarget;             // Path to store log file.
   UtlBoolean bSpecifiedDirError ;   // Set if the specified log dir does not
   // exist

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
   SipXecsService::setLogPriority(*pConfig, CONFIG_SETTING_PREFIX);
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


class SignalTask : public OsTask
{
public:
   SignalTask() : OsTask() {}

   int
   run(void *pArg)
   {
       int sig_num ;
       OsStatus res ;

       // Wait for a signal.  This will unblock signals
       // for THIS thread only, so this will be the only thread
       // to catch an async signal directed to the process
       // from the outside.
       res = awaitSignal(sig_num);
       if (res == OS_SUCCESS)
       {
          if (SIGTERM == sig_num)
          {
             OsSysLog::add( LOG_FACILITY, PRI_INFO, "SignalTask: terminate signal received.");
          }
          else
          {
            OsSysLog::add( LOG_FACILITY, PRI_CRIT, "SignalTask: caught signal: %d", sig_num );
          }
       }
       else
       {
            OsSysLog::add( LOG_FACILITY, PRI_CRIT, "SignalTask: awaitSignal() failed");
       }
       // set the global shutdown flag
       gShutdownFlag = TRUE ;
       return 0 ;
   }
} ;

/** The main entry point to the sipregistrar */
int
main(int argc, char* argv[] )
{
   // Block all signals in this the main thread.
   // Any threads created from now on will have all signals masked.
   OsTask::blockSignals();

   // Create a new task to wait for signals.  Only that task
   // will ever see a signal from the outside.
   SignalTask* signalTask = new SignalTask();
   signalTask->start();

   // Configuration Database (used for OsSysLog)
   OsConfigDb* configDb = new OsConfigDb();

   UtlBoolean interactiveSet = false;
   UtlString argString;
   for (int argIndex = 1; argIndex < argc; argIndex++)
   {
      osPrintf("arg[%d]: %s\n", argIndex, argv[argIndex]);
      argString = argv[argIndex];
      NameValueTokenizer::frontBackTrim(&argString, "\t ");
      if (argString.compareTo("-v") == 0)
      {
         osPrintf("Version: %s (%s)\n", VERSION, PACKAGE_REVISION);
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
      exit(1);
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
   while( !gShutdownFlag && !pServerTask->isShutDown() )
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
