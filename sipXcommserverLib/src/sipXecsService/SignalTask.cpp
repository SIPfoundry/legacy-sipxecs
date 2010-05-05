//
// Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <signal.h>

// APPLICATION INCLUDES
#include "sipXecsService/SignalTask.h"
#include "os/OsSysLog.h"
#include "os/OsTask.h"

// DEFINES
// TYPEDEFS
// CONSTANTS

   int
   ChildSignalTask::run(void *pArg)
   {
       int sig_num ;
       OsStatus res ;

       // Wait for a signal.  This will unblock signals
       // for THIS thread only, so this will be the only thread
       // to catch an async signal directed to the process
       // from the outside.
       OsSysLog::add( FAC_KERNEL, PRI_INFO, "SignalTask: calling awaitSignal");
       OsTask::blockSignals();
       res = awaitSignal(sig_num);
       if (res == OS_SUCCESS)
       {
          if (SIGTERM == sig_num)
          {
             OsSysLog::add( FAC_KERNEL, PRI_INFO, "SignalTask: terminate signal received.");
          }
          else
          {
            OsSysLog::add( FAC_KERNEL, PRI_CRIT, "SignalTask: caught signal: %d", sig_num );
          }
       }
       else
       {
            OsSysLog::add( FAC_KERNEL, PRI_CRIT, "SignalTask: awaitSignal() failed");
       }
       // set the global shutdown flag
       mShutdownFlag = true ;
       return 0 ;
   }

