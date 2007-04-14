// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef IMDBTASKMONITOR_H
#define IMDBTASKMONITOR_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsEvent.h"
#include "os/OsStatus.h"
#include "os/OsServerTask.h"

// DEFINES
#define LOG_FACILITY        FAC_DB
#ifndef SIPXCHANGE_USERNAME
# define SIPXCHANGE_USERNAME "sipxchange"
#endif
#define EXIT_SUCCESS        0
#define EXIT_BADUSERID      1
#define EXIT_BADSYNTAX      2
#define EXIT_RUNTIMEERROR   3
#define EXIT_FILENOTFOUND   4

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class OsMsg;
class OsTimer;
class OsQueuedEvent;
class IMDBWorkerTask;

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class IMDBTaskMonitor : public OsServerTask
{
public:
    /**
     * Ctor
     * 
     * @param rCommand              The command (currently, ping, import or export
     * @param rArgument             An Argument sent to the Worker Thread
     * @param taskTestTimeoutSecs   the number of seconds to 
     *                              wait before killing the Worker thread
     * @param taskRetryIntervalSecs 
     */
    IMDBTaskMonitor ( 
        const UtlString& rCommand,
        const UtlString& rArgument,
        const int& mMonitorSecs );

    /**
     * Copy Ctor
     * 
     * @param rIMDBTaskMonitor
     */
    IMDBTaskMonitor ( const IMDBTaskMonitor& rIMDBTaskMonitor );

    /**
     * Dtor
     */
    virtual ~IMDBTaskMonitor();

    /**
     * Event handler for the Monitor, success events from the worker
     * threads or timeout events are routed back here.
     * 
     * @param eventMessage
     * 
     * @return 
     */
    virtual UtlBoolean handleMessage( OsMsg& eventMessage );

    /**
     * Assignment Operator
     * @param rhs
     * 
     * @return 
     */
    IMDBTaskMonitor& operator=( const IMDBTaskMonitor& rhs );

    /**
     * Getter for Main to access the process error info after a failure
     * 
     * @param rErrnum
     * 
     * @return 
     */
    OsStatus getErrno ( int& rErrnum );

    /**
     * Setter to indicate the error
     * 
     * @param errnum
     * 
     * @return 
     */
    OsStatus setErrno ( int errnum );
private:
    OsTimer*        mpWatchDogTimer;         // Timer evaluating time to check processes
    OsQueuedEvent*  mpTimeoutEvent;          // Event marking time to check processes
    IMDBWorkerTask* mpIMDBWorkerTask;        // the task that pings the IMDB
    int             mErrno;                  // Error capture for main exit to watchdog
    int             mMonitorSecs;            // Monitor period
    UtlString        mCommandName;            // the command being executed
    OsEvent         mCommandEvent;           // IPC event object
    UtlBoolean       mCommandCompleted;       // Boolean indicating that the command has completed
};

#endif  // IMDBTASKMONITOR_H

