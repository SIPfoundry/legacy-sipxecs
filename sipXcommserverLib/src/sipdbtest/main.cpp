// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////
// SYSTEM INCLUDES
#if defined(__pingtel_on_posix__)
#include <pwd.h>
#endif
#include <iostream>
#include <signal.h>

// APPLICATION INCLUDES
#include "os/OsFS.h"
#include "os/OsStatus.h"
#include "sipdb/SIPDBManager.h"
#include "IMDBTaskMonitor.h"

// DEFINES
#define DBMONITOR_LOG_DIR         "/var/log"
#define DBMONITOR_LOG_FILENAME    "dbmonitor.log"
#define DBMONITOR_DEFAULT_LOG_DIR SIPX_LOGDIR
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
UtlBoolean removeAllRows = FALSE;
UtlBoolean gShutdownFlag = FALSE;
UtlBoolean gClosingIMDB  = FALSE;
OsMutex*   gpLockMutex = new OsMutex(OsMutex::Q_FIFO);

OsServerTask* pServerTask   = NULL;

/* ============================ FUNCTIONS ================================= */

/** helper method to determine whether we're running as an appropriate user */
OsStatus
isRunningAsValidUser()
{
    OsStatus result = OS_FAILED;
    UtlString user;
#if defined(__pingtel_on_posix__)
    // don't use getlogin as it is not reliable
    passwd* pUserInfo = getpwuid( geteuid() );
    if ( (pUserInfo != NULL) && (pUserInfo->pw_name != NULL) )
    {
        user = pUserInfo->pw_name;
    }
#else
    user = SIPXCHANGE_USERNAME;
#endif
    if ( user.compareTo(SIPXCHANGE_USERNAME) == 0 )
    {
       result = OS_SUCCESS;
    }
    return result;
}

/**
 * Description:
 * closes any open connections to the IMDB safely using a mutex lock
 */
void
closeIMDBConnections (int sig_num )
{
    // Critical Section here
    OsLock lock( *gpLockMutex );
    // search for a clue as to whether the signal is from an IMDB assertion
    // or not, if it is bypass the code to unregister unregister the PIDs from
    // the IMDB
    if ( (sig_num == SIGABRT) || (sig_num == SIGSEGV) )
    {
        // perform some cleanup
    } else
    {
        // Ensure that this process calls close on the IMDB
        // this will only access the FastDB if it was opened
        // by reference and tables were registers (It checks for
        // pFastDB in its destructor and pFastDB is only created
        // or opened if a user requests a table
        delete SIPDBManager::getInstance();
    }
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
    closeIMDBConnections( sig_num );
}

void
setLogLevel()
{
    OsSysLog::setLoggingPriority( PRI_DEBUG );
    OsSysLog::setLoggingPriorityForFacility(FAC_SIP_INCOMING_PARSED, PRI_ERR);
}

OsStatus
getLogFilePath ( UtlString& logFilePath )
{
    // Make sure that the SIPXCHANGE_HOME ends with
    // a trailing file separator character
    char *sipxHomeEnv = getenv ( "SIPXCHANGE_HOME" );
    OsPath path;
    if ( sipxHomeEnv != NULL )
    {
        UtlString sipxHome = sipxHomeEnv;
        // if the last character is a separator strip it.
        if ( sipxHome ( sipxHome.length() -1 ) == OsPath::separator )
            sipxHome = sipxHome(0, sipxHome.length()-1);

        if ( OsFileSystem::exists( sipxHome + DBMONITOR_LOG_DIR) )
        {
            path = sipxHome + DBMONITOR_LOG_DIR;
        } else
        {   // set to current working directory as the dir does not exist
            OsFileSystem::getWorkingDirectory(path);
        }
    } else // Environment Variable is not defined check default location
    {
        if ( OsFileSystem::exists( DBMONITOR_DEFAULT_LOG_DIR ) )
        {   
	    // Search in DBMONITOR_DEFAULT_LOG_DIR (which is SIPX_LOGDIR).
            path = DBMONITOR_DEFAULT_LOG_DIR;
        } else
        {
            // set to current working directory
            OsFileSystem::getWorkingDirectory(path);
        }
    }

    // now that we have the path to the logfile directory
    // append the name of the log file
    path += OsPath::separator + DBMONITOR_LOG_FILENAME;

    // Finally translate path to native path,
    // this will ensure the volume is set correctly
    OsPath nativePath ;
    path.getNativePath(nativePath);
    logFilePath = nativePath ;
    return OS_SUCCESS;
}

// Print out some command line interface help
void
usage()
{
    cout << "in memory database utility" << endl;
    cout << "==========================" << endl;
    cout << "to populate IMDB tables from an XML file:" << endl;
    cout << "    import xmlfile" << endl;
    cout << "to display the contents of a table:" << endl;
    cout << "    display tablename [retrydelaysecs(30 default)]" << endl;
    cout << "to monitor the IMDB for and write an imdb.bad file if it times out:" << endl;
    cout << "    heartbeat [processinfo|#transactiontime(s)] [heartbeatintervalsecs(30s)]" << endl;
    cout << "to preload the IMDB tables:" << endl;
    cout << "    keepalive tablename" << endl;
    cout << "where 'tablename' is [credential|dialbyname|permission|registration|alias|subscription|extension|huntgroup|authexception|processinfo]" << endl;
}

void
doWaitLoop()
{
    while ( !gShutdownFlag )
        OsTask::delay( 500 );

    if ( pServerTask )
        pServerTask->requestShutdown();
}

/**
 * Launches a worker thread and waits for it to finish
 *
 * @param rCommand
 * @param rCommandArgument
 * @param rTimeoutSecs
 *
 * @return
 */
int
launchWorkerTask (
    const UtlString& rCommand,
    const UtlString& rCommandArgument,
    const int& rTimeoutSecs )
{
    int status = 0;
    pServerTask = new IMDBTaskMonitor( rCommand, rCommandArgument, rTimeoutSecs );
    pServerTask->start();
    doWaitLoop();
    pServerTask->getErrno( status );
    return status;
}

/**
 * Main entry point for the test utility
 *
 * @param argc
 * @param argv
 *
 * @return
 */
int
main( int argc, char *argv[] )
{
    // initialize exit status to success
    int exitCode = EXIT_SUCCESS;

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

    cout << "IMDB Test Harness" << endl;
    // see if we're running as user sipxchange, and warn user if we are not
    if ( isRunningAsValidUser() != OS_SUCCESS )
    {
        cout << "CAUTION: Using this program as any user other than 'sipxchange'\n"
             << "         on a production system may cause database problems\n"
             << "         that can disable the system.  Use this program only\n"
             << "         as instructed by the Pingtel Technical Assistance Center.\n"
             << "\n"
             << endl;
    }

    UtlString logFilePath;
    if ( getLogFilePath ( logFilePath ) == OS_SUCCESS )
    {
        // Initialize the logger.
        OsSysLog::initialize(0, "dbmonitor" );
        OsSysLog::setOutputFile(0, logFilePath );
            setLogLevel();

        OsSysLog::add(LOG_FACILITY, PRI_DEBUG,
            "sipdbtest - Entering main");

        // valid user so check arguments, import/display/heartbeat
        if( argc > 2 )
        {
            int defaultTimeoutSecs = 30;
            if (argc > 3)
                defaultTimeoutSecs = atoi( argv[3] );

            UtlString cmd = argv[1];
            if ( (cmd.compareTo("heartbeat", UtlString::ignoreCase) != 0) &&
                 (cmd.compareTo("keepalive", UtlString::ignoreCase) != 0) &&
                 (cmd.compareTo("display", UtlString::ignoreCase) != 0) &&
                 (cmd.compareTo("import", UtlString::ignoreCase) != 0) )
            {
                usage();
                exitCode = EXIT_BADSYNTAX;
            }
            else
                exitCode = launchWorkerTask (
                    argv[1],    // command
                    argv[2],    // command argument (heartbeat (this is tableinfo/txdelay(s))
                    defaultTimeoutSecs );   // the monitor time
        } else {
            usage();
            exitCode = EXIT_BADSYNTAX;
        }
    }

    if ( exitCode == EXIT_SUCCESS )
    {
        // unregister this process's database references from the IMDB
        cout << "Cleanup...Start" << endl;
        closeIMDBConnections(-1);
        cout << "Cleanup...Finished" << endl;
    }
    return exitCode;
}
