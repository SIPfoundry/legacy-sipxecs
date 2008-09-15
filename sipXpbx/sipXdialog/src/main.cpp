// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

// APPLICATION INCLUDES
#include "dmConfig.h"
#include <net/SipUserAgent.h>
#include <net/SipDialogMonitor.h>
#include <ptapi/PtProvider.h>
#include <net/NameValueTokenizer.h>
#include <net/XmlRpcDispatch.h>
#include <os/OsFS.h>
#include <os/OsSysLog.h>
#include <os/OsConfigDb.h>

// DEFINES
#ifndef SIPX_VERSION
#  include "sipxpbx-buildstamp.h"
#  define SIPXCHANGE_VERSION          SipXpbxVersion
#  define SIPXCHANGE_VERSION_COMMENT  SipXpbxBuildStamp
#else
#  define SIPXCHANGE_VERSION          SIPX_VERSION
#  define SIPXCHANGE_VERSION_COMMENT  ""
#endif

#define CONFIG_SETTINGS_FILE          "sipxdialog-config"
#define CONFIG_ETC_DIR                SIPX_CONFDIR

#define CONFIG_LOG_FILE               "sipxdialog.log"
#define CONFIG_LOG_DIR                SIPX_LOGDIR

#define CONFIG_SETTING_LOG_DIR          "SIP_DIALOG_MONITOR_LOG_DIR"
#define CONFIG_SETTING_LOG_LEVEL        "SIP_DIALOG_MONITOR_LOG_LEVEL"
#define CONFIG_SETTING_LOG_CONSOLE      "SIP_DIALOG_MONITOR_LOG_CONSOLE"
#define CONFIG_SETTING_DOMAIN_NAME      "SIP_DIALOG_MONITOR_DOMAIN_NAME"
#define CONFIG_SETTING_UDP_PORT         "SIP_DIALOG_MONITOR_UDP_PORT"
#define CONFIG_SETTING_TCP_PORT         "SIP_DIALOG_MONITOR_TCP_PORT"
#define CONFIG_SETTING_HTTP_PORT        "SIP_DIALOG_MONITOR_HTTP_PORT"
#define CONFIG_SETTING_REFRESH_INTERVAL "SIP_DIALOG_MONITOR_REFRESH_INTERVAL"

#define LOG_FACILITY                  FAC_ACD

#define DEFAULT_UDP_PORT              5140       // Default UDP port
#define DEFAULT_TCP_PORT              5140       // Default TCP port
#define DEFAULT_HTTP_PORT             8200       // Default HTTP port

#define DEFAULT_REFRESH_INTERVAL      300

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
UtlBoolean    gShutdownFlag = FALSE;



/* ============================ FUNCTIONS ================================= */

/**
 * Description:
 * This is a replacement for signal() which registers a signal handler but sets
 * a flag causing system calls ( namely read() or getchar() ) not to bail out
 * upon recepit of that signal. We need this behavior, so we must call
 * sigaction() manually.
 */
sighandler_t pt_signal( int sig_num, sighandler_t handler)
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
void sigHandler( int sig_num )
{
    // set a global shutdown flag
    gShutdownFlag = TRUE;

    // Unregister interest in the signal to prevent recursive callbacks
    pt_signal( sig_num, SIG_DFL );

    // Minimize the chance that we loose log data
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
      const char*      pIdentity;
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

   OsSysLog::initialize(0, "sipxdialog");


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
         OsSysLog::add(LOG_FACILITY, PRI_INFO, "%s : %s", CONFIG_SETTING_LOG_DIR, workingDirectory.data());
      }
      else
      {
         OsPath path;
         OsFileSystem::getWorkingDirectory(path);
         path.getNativePath(workingDirectory);

         osPrintf("%s : %s\n", CONFIG_SETTING_LOG_DIR, workingDirectory.data());
         OsSysLog::add(LOG_FACILITY, PRI_INFO, "%s : %s", CONFIG_SETTING_LOG_DIR, workingDirectory.data());
      }

      fileTarget = workingDirectory +
         OsPathBase::separator +
         CONFIG_LOG_FILE;
   }
   else
   {
      bSpecifiedDirError = false;
      osPrintf("%s : %s\n", CONFIG_SETTING_LOG_DIR, fileTarget.data());
      OsSysLog::add(LOG_FACILITY, PRI_INFO, "%s : %s", CONFIG_SETTING_LOG_DIR, fileTarget.data());

      fileTarget = fileTarget +
         OsPathBase::separator +
         CONFIG_LOG_FILE;
   }
   OsSysLog::setOutputFile(0, fileTarget);


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
   int iEntries = sizeof(lkupTable) / sizeof(struct tagPrioriotyLookupTable);
   for (int i = 0; i < iEntries; i++)
   {
      if (logLevel == lkupTable[i].pIdentity)
      {
         priority = lkupTable[i].ePriority;
         osPrintf("%s : %s\n", CONFIG_SETTING_LOG_LEVEL, lkupTable[i].pIdentity);
         OsSysLog::add(LOG_FACILITY, PRI_INFO, "%s : %s", CONFIG_SETTING_LOG_LEVEL, lkupTable[i].pIdentity);
         break;
      }
   }
   OsSysLog::setLoggingPriority(priority);
   OsSysLog::setLoggingPriorityForFacility(FAC_SIP_INCOMING_PARSED, PRI_ERR);

   //
   // Get/Apply console logging
   //
   UtlBoolean bConsoleLoggingEnabled = false;
   if ((pConfig->get(CONFIG_SETTING_LOG_CONSOLE, consoleLogging) == OS_SUCCESS))
   {
      consoleLogging.toUpper();
      if (consoleLogging == "ENABLE")
      {
         OsSysLog::enableConsoleOutput(true);
         bConsoleLoggingEnabled = true;
      }
   }

   osPrintf("%s : %s\n", CONFIG_SETTING_LOG_CONSOLE, bConsoleLoggingEnabled ? "ENABLE" : "DISABLE") ;
   OsSysLog::add(LOG_FACILITY, PRI_INFO, "%s : %s", CONFIG_SETTING_LOG_CONSOLE, bConsoleLoggingEnabled ? "ENABLE" : "DISABLE") ;

   if (bSpecifiedDirError)
   {
      OsSysLog::add(FAC_LOG, PRI_CRIT, "Cannot access %s directory; please check configuration.", CONFIG_SETTING_LOG_DIR);
   }
}


//
// The main entry point to the sipXpark
//
int main(int argc, char* argv[])
{
   // Configuration Database (used for OsSysLog)
   OsConfigDb configDb;

   // Register Signal handlers so we can perform graceful shutdown
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

   UtlString argString;
   for (int argIndex = 1; argIndex < argc; argIndex++)
   {
      osPrintf("arg[%d]: %s\n", argIndex, argv[argIndex]);
      argString = argv[argIndex];
      NameValueTokenizer::frontBackTrim(&argString, "\t ");
      if(argString.compareTo("-v") == 0)
      {
         osPrintf("Version: %s (%s)\n", SIPXCHANGE_VERSION, SIPXCHANGE_VERSION_COMMENT);
         return(1);
      }
      else
      {
         osPrintf("usage: %s [-v]\nwhere:\n -v provides the software version\n",
         argv[0]);
         return(1);
      }
   }

   // Load configuration file file
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
      exit(1);
   }

   // Initialize log file
   initSysLog(&configDb);

   // Read the user agent parameters from the config file.
   int UdpPort;
   if (configDb.get(CONFIG_SETTING_UDP_PORT, UdpPort) != OS_SUCCESS)
   {
      UdpPort = DEFAULT_UDP_PORT;
   }
   
   int TcpPort;
   if (configDb.get(CONFIG_SETTING_TCP_PORT, TcpPort) != OS_SUCCESS)
   {
      TcpPort = DEFAULT_TCP_PORT;
   }

   int HttpPort;
   if (configDb.get(CONFIG_SETTING_HTTP_PORT, HttpPort) != OS_SUCCESS)
   {
      HttpPort = DEFAULT_HTTP_PORT;
   }
   
   // Bind the SIP user agent to a port and start it up
   SipUserAgent* userAgent = new SipUserAgent(TcpPort, UdpPort);
   userAgent->start();

   UtlString domainName;
   configDb.get(CONFIG_SETTING_DOMAIN_NAME, domainName);

   int refreshTimeout;
   if (configDb.get(CONFIG_SETTING_REFRESH_INTERVAL, refreshTimeout) != OS_SUCCESS)
   {
      refreshTimeout = DEFAULT_REFRESH_INTERVAL;
   }

   // Create the Sip Dialog Monitor
   SipDialogMonitor dialogMonitor(userAgent,
                                  domainName,
                                  TcpPort,
                                  refreshTimeout,
                                  true);
   // Create a XmlRpcDispatch
   XmlRpcDispatch XmlRpcServer(HttpPort, false, "/RPC2");
   
   // Register all the XML-RPC methods
   DialogMonitorConfig xmlRpcMethods(&XmlRpcServer, &dialogMonitor);
    
   // Loop forever until signaled to shut down
   while (!gShutdownFlag)
   {
      OsTask::delay(2000);
   }

   // Shut down the sipUserAgent
   userAgent->shutdown(FALSE);

   while(!userAgent->isShutdownDone())
   {
      ;
   }

   delete userAgent;

   // Flush the log file
   OsSysLog::flush();

   // Say goodnight Gracie...
   return 0;
}


// Stub to avoid pulling in ps library
int JNI_LightButton(long)
{
   return 0;
}

