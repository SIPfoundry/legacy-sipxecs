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


// APPLICATION INCLUDES
//#include "pinger/version.h"
#include "os/OsConfigDb.h"
#include "os/OsFS.h"
#include "os/OsTask.h"
#include "utl/PtTest.h"

#include "net/NameValueTokenizer.h"
#include "emailnotifier/EmailNotifier.h"

// DEFINES
#define CONFIG_LOG_FILE       "sipstatus.log"
#define CONFIG_LOG_DIR        SIPX_LOGDIR
#define CONFIG_ETC_DIR        SIPX_CONFDIR
#define CONFIG_SETTINGS_FILE  "status-config"

// Configuration names pulled from config-file
#define CONFIG_SETTING_LOG_LEVEL      "SIP_STATUS_LOG_LEVEL"
#define CONFIG_SETTING_LOG_CONSOLE    "SIP_STATUS_LOG_CONSOLE"
#define CONFIG_SETTING_LOG_DIR        "SIP_STATUS_LOG_DIR"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
typedef void (*sighandler_t)(int);

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
UtlBoolean     shutdownFlag  = FALSE;

/* ============================ FUNCTIONS ================================= */

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
 * global gShutdownFlag flag to true allowing the main processing
 * loop to exit cleanly.
 */
void 
sigHandler(int sig_num)
{
    // Can only catch SIGSEGV once or we might get stuck in a loop
    if(sig_num == SIGSEGV)
    {
        pt_signal(SIGSEGV, SIG_DFL);
    }

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
    OsSysLog::flush();

    shutdownFlag = TRUE;
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
      char*            pIdentity;
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
   OsSysLog::initialize(0, "SipStatus");

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
         CONFIG_LOG_FILE;
   }
   else
   {
      bSpecifiedDirError = false ;
      osPrintf("%s : %s\n", CONFIG_SETTING_LOG_DIR, fileTarget.data()) ;
      OsSysLog::add(FAC_SIP, PRI_INFO, "%s : %s", CONFIG_SETTING_LOG_DIR, fileTarget.data()) ;

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
   
   osPrintf("%s : %s\n", CONFIG_SETTING_LOG_CONSOLE, bConsoleLoggingEnabled ? "ENABLE" : "DISABLE") ;
   OsSysLog::add(FAC_SIP, PRI_INFO, "%s : %s", CONFIG_SETTING_LOG_CONSOLE, bConsoleLoggingEnabled ? "ENABLE" : "DISABLE") ;

   if (bSpecifiedDirError)
   {
      OsSysLog::add(FAC_LOG, PRI_CRIT, "Cannot access %s directory; please check configuration.", CONFIG_SETTING_LOG_DIR);
   }
}




/** The main entry point to the StatusServer */
int 
main(int argc, char* argv[] )
{
   OsConfigDb  configDb ;  // Params for OsSysLog init

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
   for(int argIndex = 1; argIndex < argc; argIndex++)
   {
      osPrintf("arg[%d]: %s\n", argIndex, argv[argIndex]);
      argString = argv[argIndex];
      NameValueTokenizer::frontBackTrim(&argString, "\t ");
      //if(argString.compareTo("-v") == 0)
      //{
      //   osPrintf("Version: %s (%s)\n", SIPXPBX_VERSION, SIPXPBX_VERSION_COMMENT);
      //   return(1);
      //}
      //else 
      if( argString.compareTo("-i") == 0)
      {
         interactiveSet = true;
         osPrintf("Entering Interactive Mode\n");
      }
      else
      {
         osPrintf("usage: %s [-v] [-i]\nwhere:\n -v provides the software version\n"
            " -i start the server in an interactive made\n",
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
  
   configDb.loadFromFile(fileName) ;
   initSysLog(&configDb) ;

   // Fetch Pointer to the OsServer task object, note that 
   // object uses the IMDB so it is important to shut this thread
   // cleanly before the signal handler exits
   StatusServer* pStatusServer = StatusServer::getInstance();

   pServerTask = static_cast<OsServerTask*>(pStatusServer);

   // Do not exit, let the proxy do its stuff
   while( !shutdownFlag )
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
            else
            {
               //pStatusServer->printMessageLog();
            }
         }
      }
      else
         OsTask::delay(30000);
   }

   // Remove the current process's row from the IMDB
   // Persisting the database if necessary
   cout << "Cleaning Up..Start." << endl;

   // This is a server task so gracefully shutdown the 
   // server task using the waitForShutdown method, this
   // will implicitly request a shutdown for us if one is
   // not already in progress
   if ( pServerTask != NULL )
   {
       // Deleting a server task is the only way of 
       // waiting for shutdown to complete cleanly
       pServerTask->requestShutdown();
       delete pServerTask;
       pServerTask = NULL;
   }

   cout << "Cleanup...Finished" << endl;

   return 0;
}
