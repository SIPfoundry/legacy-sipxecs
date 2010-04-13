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
#include <net/SipUserAgent.h>
#include <cp/SipPresenceMonitor.h>
#include <ptapi/PtProvider.h>
#include <net/NameValueTokenizer.h>
#include <os/OsFS.h>
#include <os/OsSysLog.h>
#include <os/OsConfigDb.h>
#include <persist/SipPersistentSubscriptionMgr.h>
#include <sipXecsService/SipXecsService.h>

// DEFINES
#ifndef SIPX_VERSION
#  include "sipxpresence-buildstamp.h"
#  define SIPXCHANGE_VERSION          SipXpresenceVersion
#  define SIPXCHANGE_VERSION_COMMENT  SipXpresenceBuildStamp
#else
#  define SIPXCHANGE_VERSION          SIPX_VERSION
#  define SIPXCHANGE_VERSION_COMMENT  ""
#endif

#define CONFIG_SETTINGS_FILE          "sipxpresence-config"
#define CONFIG_ETC_DIR                SIPX_CONFDIR

#define CONFIG_LOG_FILE               "sipxpresence.log"
#define CONFIG_LOG_DIR                SIPX_LOGDIR

#define CONFIG_SETTING_PREFIX         "SIP_PRESENCE"
#define CONFIG_SETTING_LOG_DIR        "SIP_PRESENCE_LOG_DIR"
#define CONFIG_SETTING_LOG_CONSOLE    "SIP_PRESENCE_LOG_CONSOLE"
#define CONFIG_SETTING_DOMAIN_NAME    "SIP_PRESENCE_DOMAIN_NAME"
#define CONFIG_SETTING_UDP_PORT       "SIP_PRESENCE_UDP_PORT"
#define CONFIG_SETTING_TCP_PORT       "SIP_PRESENCE_TCP_PORT"
#define CONFIG_SETTING_BIND_IP        "SIP_PRESENCE_BIND_IP"

#define LOG_FACILITY                  FAC_ACD

#define PRESENCE_DEFAULT_UDP_PORT              5130       // Default UDP port
#define PRESENCE_DEFAULT_TCP_PORT              5130       // Default TCP port
#define PRESENCE_DEFAULT_BIND_IP               "0.0.0.0"  // Default bind ip; all interfaces

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

// The name of the persistent file in which we save the presence information,
// relative to SIPX_VARDIR.
static const UtlString sPersistentFileName("sipxpresence/state.xml");

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
   OsSysLog::initialize(0, "sipxpresence");


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
   SipXecsService::setLogPriority(*pConfig, CONFIG_SETTING_PREFIX);
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
   for(int argIndex = 1; argIndex < argc; argIndex++)
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
      UdpPort = PRESENCE_DEFAULT_UDP_PORT;
   }

   int TcpPort;
   if (configDb.get(CONFIG_SETTING_TCP_PORT, TcpPort) != OS_SUCCESS)
   {
      TcpPort = PRESENCE_DEFAULT_TCP_PORT;
   }

   UtlString bindIp;
   if (configDb.get(CONFIG_SETTING_BIND_IP, bindIp) != OS_SUCCESS ||
         !OsSocket::isIp4Address(bindIp))
   {
      bindIp = PRESENCE_DEFAULT_BIND_IP;
   }

   // Bind the SIP user agent to a port and start it up
   SipUserAgent* userAgent = new SipUserAgent(TcpPort, UdpPort, PORT_NONE,
         NULL, NULL, bindIp );
   userAgent->start();

   if (!userAgent->isOk())
   {
      OsSysLog::add(LOG_FACILITY, PRI_EMERG, "SipUserAgent failed to initialize; requesting shutdown");
      gShutdownFlag = TRUE;
   }

   UtlString domainName;
   configDb.get(CONFIG_SETTING_DOMAIN_NAME, domainName);
   // Normalize SIP domain name to all lower case.
   domainName.toLower();

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
                                      &configDb,
                                      true,
                                      pathName.data());

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
