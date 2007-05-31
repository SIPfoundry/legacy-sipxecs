// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// DEFINES
#define IMDB_BAD_FILE SIPX_TMPDIR "/imdb.bad"

// APPLICATION INCLUDES
#include "os/OsQueuedEvent.h"
#include "os/OsEventMsg.h"
#include "os/OsTimer.h"
#include "os/OsDateTime.h"
#include "os/OsFS.h"
#include "IMDBTaskMonitor.h"
#include "ImportTask.h"
#include "DisplayTask.h"
#include "KeepAliveTask.h"
#include "HeartBeatMonitor.h"

extern UtlBoolean gShutdownFlag;

IMDBTaskMonitor::IMDBTaskMonitor (
    const UtlString& rCommand,
    const UtlString& rArgument,
    const int& rMonitorSecs ) :
    mErrno(0),
    mMonitorSecs ( rMonitorSecs ),
    mCommandName ( rCommand ),
    mCommandEvent(),
    mCommandCompleted (FALSE)
{
    // Create the watchdog event/timer
    mpTimeoutEvent = new OsQueuedEvent( *getMessageQueue(), 0 ) ;

    //set subtype to signify user action
    mpTimeoutEvent->setUserData( USER_TIMEOUT_EVENT);
    mpWatchDogTimer = new OsTimer(*mpTimeoutEvent) ;

    // Finally construct the heartbeat thread that hits the IMDB
    mpIMDBWorkerTask = NULL;

    if ( rCommand.compareTo("heartbeat", UtlString::ignoreCase) == 0 )
    {
        mpIMDBWorkerTask = new HeartBeatMonitor (
            rArgument, *getMessageQueue(), mCommandEvent );
    } else if ( rCommand.compareTo("keepalive", UtlString::ignoreCase) == 0 )
    {
        mpIMDBWorkerTask = new KeepAliveTask (
            rArgument, *getMessageQueue(), mCommandEvent );
    } else if ( rCommand.compareTo("display", UtlString::ignoreCase) == 0 )
    {
        mpIMDBWorkerTask = new DisplayTask (
            rArgument, *getMessageQueue(), mCommandEvent );
    } else if ( rCommand.compareTo("import", UtlString::ignoreCase) == 0 )
    {
        mpIMDBWorkerTask = new ImportTask (
            rArgument, *getMessageQueue(), mCommandEvent );
    } else
    {
    }

    if (mpIMDBWorkerTask != NULL )
    {
        mpIMDBWorkerTask->start();
        // arm the timer to fire periodically, starting one period later
        mpWatchDogTimer->periodicEvery(
            OsTime( rMonitorSecs, 0 ),  // delay
            OsTime( rMonitorSecs, 0 )); // every...
    }
}

// Copy constructor
IMDBTaskMonitor::IMDBTaskMonitor( const IMDBTaskMonitor& rIMDBTaskMonitor ) :
    mErrno(0), mCommandCompleted (FALSE)
{}

// Dtor
IMDBTaskMonitor::~IMDBTaskMonitor()
{
    delete mpTimeoutEvent;
    delete mpWatchDogTimer;
    delete mpIMDBWorkerTask;
}

// Assignment operator
IMDBTaskMonitor&
IMDBTaskMonitor::operator=( const IMDBTaskMonitor& rhs )
{
    if (this == &rhs) // handle the assignment to self case
        return *this;
    return *this;
}

OsStatus
IMDBTaskMonitor::getErrno ( int& rErrnum )
{
    rErrnum = mErrno;
    return OS_SUCCESS;
}

OsStatus
IMDBTaskMonitor::setErrno ( int errnum )
{
    mErrno = errnum;
    return OS_SUCCESS;
}

UtlBoolean
IMDBTaskMonitor::handleMessage ( OsMsg &rMsg )
{
    UtlBoolean returnValue = TRUE;

    // Check for a timer event (indicated by the MsgType)
    if ( ( rMsg.getMsgType() == OsMsg::OS_EVENT ) )
    {
        // Timer Message, see if we still have a working task
        if ( mpIMDBWorkerTask != NULL )
        {
            // test to see if the worker completed the last task
            if ( !mCommandCompleted )
            {
                // firstly stop the timer as things have gone really wrong
                mpWatchDogTimer->stop();

                // kill the worker heartbeat test task and keep the timer alive
                // we'll eventually be killed by the restart script
                // it is important not to exit here as the watchdog will notice and
                // restart us
                osPrintf( "'%s' command [FAILED]\n", mCommandName.data() );
                OsSysLog::add( LOG_FACILITY, PRI_DEBUG, "'%s' command [FAILED]\n", mCommandName.data() );

                // timer expired waiting for the Ping command to complete
                if ( mCommandName.compareTo("heartbeat", UtlString::ignoreCase) == 0 )
                {
                    osPrintf( "Notifying 'pd_monitor' process via 'imdb.bad' file\n" );
                    OsSysLog::add( LOG_FACILITY, PRI_DEBUG, "Notifying pd_monitor process via 'imdb.bad' file\n" );
                    OsFile imdbBrokenIPCFile ( IMDB_BAD_FILE );
                    imdbBrokenIPCFile.touch();
                    imdbBrokenIPCFile.close();
                    delete mpIMDBWorkerTask;
                    mpIMDBWorkerTask = NULL;
                }

                // waiting around until we're killed off by the watchdog script
                osPrintf ("Waiting for Watchdog Restart (or Ctrl+C)\n");
                OsSysLog::add( LOG_FACILITY, PRI_DEBUG, "Waiting for Watchdog Restart (or Ctrl+C)\n");

                setErrno (1);
            } else
            {   // complete the handshake by toggling the flag
                mCommandCompleted = FALSE;

                // signal the worker thread to immediately kick off another one
                mCommandEvent.signal(0);
            }
        }
    } else if ( (rMsg.getMsgType() == OsMsg::USER_START) && (rMsg.getMsgSubType() == USER_HEARTBEAT_SUCCESS_EVENT) )
    {
        // command completed successfully
        mCommandCompleted = TRUE;
        osPrintf ("%s [OK]\n", mCommandName.data());
        OsSysLog::add(LOG_FACILITY, PRI_DEBUG, "%s [OK]\n", mCommandName.data());
        setErrno (0);
    } else if ( (rMsg.getMsgType() == OsMsg::USER_START) && (rMsg.getMsgSubType() == USER_IMPORT_SUCCESS_EVENT) )
    {
        // import command completed
        mCommandCompleted = TRUE;
        mpWatchDogTimer->stop();
        osPrintf ("%s [OK]\n", mCommandName.data());
        OsSysLog::add(LOG_FACILITY, PRI_DEBUG, "%s [OK]\n", mCommandName.data());
        setErrno (0);
        gShutdownFlag = TRUE;
    } else if ( (rMsg.getMsgType() == OsMsg::USER_START) && (rMsg.getMsgSubType() == USER_DISPLAY_SUCCESS_EVENT) )
    {
        // display command completed
        mCommandCompleted = TRUE;
        mpWatchDogTimer->stop();
        osPrintf ("%s [OK]\n", mCommandName.data());
        OsSysLog::add(LOG_FACILITY, PRI_DEBUG, "%s [OK]\n", mCommandName.data());
        setErrno (0);
        gShutdownFlag = TRUE;
    }
    return( returnValue );
}

