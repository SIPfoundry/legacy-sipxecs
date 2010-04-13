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
// FORWARD DECLARATIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// GLOBAL VARIABLE INITIALIZATIONS
OsSysLogPriority gACD_DEBUG = PRI_DEBUG;
UtlBoolean       gShutdownFlag = FALSE;
UtlBoolean       gRestartFlag = FALSE;



/* ============================ FUNCTIONS ================================= */



////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        SignalTask
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
   // Block all signals in this the main thread.
   // Any threads created from now on will have all signals masked.
   OsTask::blockSignals();

   // Create a new task to wait for signals.  Only that task
   // will ever see a signal from the outside.
   SignalTask* signalTask = new SignalTask();
   signalTask->start();

   int provisioningAgentPort = 8110;
   int watchdogRpcServerPort = 8092;
   ACDServer* pAcdServer;

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
      else if (argString.compareTo("-W") == 0) {
         argString = argv[++argIndex];
         watchdogRpcServerPort = atoi(argString.data());
      }
      else {
         enableConsoleOutput(true);
         osPrintf("usage: %s [-v] [-c] [-d] [-P port] [-W wport]\nwhere:\n -v      Provides the software version\n"
                  " -c       Enables console output of log and debug messages\n"
                  " -d       Changes ACDServer DEBUG logging to run at NOTICE level\n"
                  " -P port  Specifies the provisioning interface port number\n"
                  " -W wport Specifies the Watchdog interface port number\n",
                  argv[0]);
         return(1);
      }
   }


   // Create the ACDServer and get to work.
   pAcdServer = new ACDServer(provisioningAgentPort, watchdogRpcServerPort);

   // Loop forever until signaled to shut down
   while (!gShutdownFlag) {
      OsTask::delay(2000);
   }

   // Shut down the ACDServer.
   delete pAcdServer;

   // Flush the log file
   OsSysLog::flush();

   // Say goodnight Gracie...

   // Use _exit to avoid the atexit() processing which will cause the
   // destruction of static objects...yet there are still threads out
   // there using those static objects.  This prevents core dumps
   // due to those threads accessing the static objects post destruction.
   //
   // --Woof!

   _exit(0);

   /*NOTREACHED*/
   return 0 ; // To appease the compiler gods
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
