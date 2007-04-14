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
#ifndef WIN32
#include <unistd.h>
#endif

// APPLICATION INCLUDES
#include <net/NameValueTokenizer.h>
#include <os/OsTask.h>
#include <os/OsSysLog.h>
#include <utl/UtlString.h>
#include "ACDServer.h"

// DEFINES
#ifndef SIPX_VERSION
   #include "sipxacd-buildstamp.h"
   #define SIPXCHANGE_VERSION          SipXacdVersion
   #define SIPXCHANGE_VERSION_COMMENT  SipXacdBuildStamp
#else
   #define SIPXCHANGE_VERSION          SIPX_VERSION
   #define SIPXCHANGE_VERSION_COMMENT  ""
#endif

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
OsSysLogPriority gACD_DEBUG = PRI_DEBUG;
UtlBoolean       gShutdownFlag = FALSE;



/* ============================ FUNCTIONS ================================= */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ::pt_signal
//
//  SYNOPSIS:
//
//  DESCRIPTION: This is a replacement for signal() which registers a signal handler but sets
//               a flag causing system calls ( namely read() or getchar() ) not to bail out
//               upon recepit of that signal. We need this behavior, so we must call
//               sigaction() manually.
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

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


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ::sigHandler
//
//  SYNOPSIS:
//
//  DESCRIPTION: This is the signal handler, When called this sets the global gShutdownFlag
//               allowing the main processing loop to exit cleanly.
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void sigHandler( int sig_num )
{
   // set a global shutdown flag
   gShutdownFlag = TRUE;

   // Unregister interest in the signal to prevent recursive callbacks
   pt_signal( sig_num, SIG_DFL );

   // Minimize the chance that we loose log data
   OsSysLog::flush();
   OsSysLog::add(LOG_FACILITY, PRI_CRIT, "sigHandler: caught signal: %d", sig_num);
   OsSysLog::flush();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ::main
//
//  SYNOPSIS:
//
//  DESCRIPTION: This is the main entry point to the sipXACD Server
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
   int provisioningAgentPort = 8110;
   ACDServer* pAcdServer;

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

   // Disable osPrintf's
   enableConsoleOutput(false);

   UtlString argString;
   for (int argIndex = 1; argIndex < argc; argIndex++) {
      osPrintf("arg[%d]: %s\n", argIndex, argv[argIndex]);
      argString = argv[argIndex];
      NameValueTokenizer::frontBackTrim(&argString, "\t ");
      if (argString.compareTo("-v") == 0) {
         enableConsoleOutput(true);
         osPrintf("Version: %s (%s)\n", SIPXCHANGE_VERSION, SIPXCHANGE_VERSION_COMMENT);
         return(1);
      }
      else if (argString.compareTo("-c") == 0) {
         // Enable osPrintf's
         enableConsoleOutput(true);
      }
      else if (argString.compareTo("-d") == 0) {
         // Switch ACDServer PRI_DEBUG to PRI_NOTICE
         gACD_DEBUG = PRI_NOTICE;
      }
      else if (argString.compareTo("-P") == 0) {
         argString = argv[++argIndex];
         provisioningAgentPort = atoi(argString.data());
      }
      else {
         enableConsoleOutput(true);
         osPrintf("usage: %s [-v] [-c] [-d] [-P port]\nwhere:\n -v      Provides the software version\n"
                  " -c      Enables console output of log and debug messages\n"
                  " -d      Changes ACDServer DEBUG logging to run at NOTICE level\n"
                  " -P port Specifies the provisioning interface port number\n",
                  argv[0]);
         return(1);
      }
   }


   // Create the ACDServer and get to work.
   pAcdServer = new ACDServer( provisioningAgentPort);

   // Loop forever until signaled to shut down
   while (!gShutdownFlag) {
      OsTask::delay(2000);
   }

   // Shut down the ACDServer.
   delete pAcdServer;

   // Flush the log file
   OsSysLog::flush();

   // Say goodnight Gracie...
   return 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ::JNI_LightButton
//
//  SYNOPSIS:    JNI_LightButton(long)
//
//  DESCRIPTION: Stub to avoid pulling in the ps library
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

int JNI_LightButton(long)
{
   return 0;
}

