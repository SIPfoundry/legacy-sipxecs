// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsMsgQ.h"
#include "os/OsProcessIterator.h"
#include "sipdb/SIPDBManager.h"
#include "IMDBTaskMonitor.h"
#include "HeartBeatMonitor.h"

HeartBeatMonitor::HeartBeatMonitor ( 
    const UtlString& rArgument, OsMsgQ& rMsgQ, OsEvent& rCommandEvent) : 
    IMDBWorkerTask( rArgument, rMsgQ, rCommandEvent )
{}

HeartBeatMonitor::~HeartBeatMonitor()
{}

int 
HeartBeatMonitor::run( void* runArg )
{
    osPrintf ("Starting Thread\n");
    OsSysLog::add(LOG_FACILITY, PRI_DEBUG, "Starting Thread\n");
    while ( !isShuttingDown() )
    {
        // Indicate that we're finished, the monitor thread
        // reads this flag and if it's still set
        setBusy (TRUE);

        // print the db meta process info before locking the IMDB
        UtlString databaseInfo;
        getDatabaseInfo ( databaseInfo );
        osPrintf ( "%s", databaseInfo.data() );
        OsSysLog::add(LOG_FACILITY, PRI_DEBUG, databaseInfo.data());

        // print out the IMDB stats, this commmand may hang forever
        if ( mArgument.compareTo("processinfo", UtlString::ignoreCase) == 0 )
        {
            UtlString processInfo;
            getProcessInfo ( FALSE, processInfo );
            osPrintf ("%s\n", processInfo.data());
            OsSysLog::add( LOG_FACILITY, PRI_DEBUG, "%s\n", processInfo.data());
        } else 
        {
            // Access the database using a cursorForUpdate (forcing a lock)
            int transactionLockSecs = atoi( mArgument.data());
            SIPDBManager::getInstance()->
                pingDatabase ( transactionLockSecs, TRUE );
        }

        setBusy (FALSE);

        // send a success message to the sleeping monitor
        notifyMonitor( USER_HEARTBEAT_SUCCESS_EVENT );

        // wait for the next event to be signaled by the notifier
        mCommandEvent.wait();
        mCommandEvent.reset();
    }

    cleanupIMDBResources();
    osPrintf ("Stopping Thread");
    OsSysLog::add(LOG_FACILITY, PRI_DEBUG, "Stopping Thread\n");
    return( TRUE );
}

