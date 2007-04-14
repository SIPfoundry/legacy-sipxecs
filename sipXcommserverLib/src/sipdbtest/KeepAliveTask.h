// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef KEEPALIVETASK_H
#define KEEPALIVETASK_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "IMDBWorkerTask.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class OsEvent;

class KeepAliveTask : public IMDBWorkerTask
{
public:
    /**
     * Ctor
     * 
     * @param rCommand
     * @param rArgument
     * @param rMsgQ
     */
    KeepAliveTask ( 
        const UtlString& rArgument, 
        OsMsgQ& rMsgQ,
        OsEvent& rCommandEvent);

    /**
     * Dtor
     */
    virtual ~KeepAliveTask();
    
    /**
     * The worker method, this is where all the command types execute, they
     * run as separate threads
     * 
     * @param runArg
     * 
     * @return 
     */
    virtual int run( void* runArg );
private:
    /** Adds a reference to each of the important IMDB tables */
    OsStatus openIMDBTables() const;
};

#endif  // KEEPALIVETASK_H


