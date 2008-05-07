// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef IMDBWORKERTASK_H
#define IMDBWORKERTASK_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsEvent.h"
#include "os/OsStatus.h"
#include "os/OsTask.h"
#include "os/OsMutex.h"

// DEFINES
#define USER_TIMEOUT_EVENT              1    
#define USER_HEARTBEAT_SUCCESS_EVENT    2
#define USER_IMPORT_SUCCESS_EVENT       3
#define USER_DISPLAY_SUCCESS_EVENT      4

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class TiXmlNode;
class UtlHashMap;

class IMDBWorkerTask : public OsTask
{
public:
    IMDBWorkerTask ( 
        const UtlString& rArgument, 
        OsMsgQ& rMsgQ,
        OsEvent& rCommandEvent );
    /**
     * The worker method, this is where all the command types execute, they
     * run as separate threads
     * 
     * @param runArg
     * 
     * @return 
     */
    virtual int run( void* runArg ) = 0;

    /**
     * busy flag getter. When the command is in progress this flag is set by
     * the worker thread, when the command completes the worker threads is expected
     * to reset this flag.   It's used to contunuously rearm the timer
     * 
     * @return 
     */
    UtlBoolean isBusy() const;

protected:
    /**
     * Helper Method to get the Current PID
     * 
     * @return 
     */
    pid_t getPID () const;

    /**
     * See above, used to set the busy flag (by the worker thread)
     * 
     * @param rFlag
     */
    void setBusy ( const UtlBoolean& rFlag );

    /**
     * 
     * @param rDatabaseInfo
     * 
     * @return 
     */
    OsStatus getDatabaseInfo ( UtlString& rDatabaseInfo ) const;

    /**
     * 
     * @param rIgnoreCurrentProcess
     * 
     * @return 
     */
    int getProcessInfo ( const UtlBoolean& rIgnoreCurrentProcess, UtlString& rProcessInfo ) const;

    /**
     * Notifies the monitor listener that the command has completed
     * 
     * @param rFlag
     */
    OsStatus notifyMonitor ( const int& rMessageIndicator ) const;    

    /** Closes any open connection resources to the IMDB */
    void cleanupIMDBResources () const;

    UtlBoolean           mBusy;
    OsMsgQ*             mpMsgQ;
    OsEvent&            mCommandEvent;
    const UtlString&     mArgument;

    // Singleton and Serialization mutex
    static OsMutex sLockMutex;
};

#endif  // IMDBWORKERTASK_H
