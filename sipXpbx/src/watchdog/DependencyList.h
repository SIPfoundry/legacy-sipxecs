// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _DependencyList_h_
#define _DependencyList_h_

// SYSTEM INCLUDES
#include <stdlib.h>

// APPLICATION INCLUDES
#include "utl/UtlDefs.h"
#include "utl/UtlString.h"
#include "os/OsStatus.h"

// DEFINES
#define MAX_DEPENDENCIES 100
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class DependencyList
{
    /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

    /* ============================ CREATORS ================================== */

    DependencyList();
    //:Default constructor

    DependencyList(const DependencyList& rDependencyList);
    //:Copy constructor

    virtual
    ~DependencyList();
    //:Destructor

    /* ============================ MANIPULATORS ============================== */

    DependencyList& operator=(const DependencyList& rhs);
    void addDependent(UtlString &rDependentName);
    void setName(UtlString &rProcessName);

    void setCanStart(UtlBoolean bCanStart);
    //:can be started by the user

    void setCanStop(UtlBoolean bCanStop);
    //:can be stopped by the user

    void setCanRestart(UtlBoolean bCanRestart);
    //:can be restarted by the user

    void setDelay(int delaySecs);
    //:How long to delay after starting process

    void setVerifyCommand(UtlString &rVerifyCommand);
    //:Command to verify process has started correctly

    void setVerifyParameters(UtlString &rVerifyParameters);
    //:Command to verify process has started correctly

    void setVerifyDefaultDir(UtlString &rVerifyDefaultDir);
    //:Command to verify process has started correctly

    /* ============================ ACCESSORS ================================= */
    UtlString getVerifyCommand();
    //:Command to get verify process command
    //:This command will be used to verify that the process has started correctly

    UtlString getVerifyParameters();
    //: returns the name of the process

    UtlString getVerifyDefaultDir();
    //: returns the name of the process

    UtlString getName();
    //: returns the name of the process

    int getDependencyCount();
    //: returns the count of dependencies stored

    OsStatus getDependency(int index, UtlString &rDependent);
    //:returns one dependency specified by index
    //:return status is OS_SUCCESS if ok
    //:or OS_FAILED if index is out of range

    UtlBoolean getCanStart();
    //:returns TRUE if the process can start by user command

    UtlBoolean getCanStop();
    //:returns TRUE if the process can stop by user command

    UtlBoolean getCanRestart();
    //:returns TRUE if the process can restarted by user command

    int getDelay();
    //:returns TRUE if the process can restarted by user command

    /* ============================ INQUIRY =================================== */
    UtlBoolean isDependent(UtlString &rDependentName);
    //returns TRUE if rDependentName
    /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    /* //////////////////////////// PRIVATE /////////////////////////////////// */
    private:
    UtlString mProcessName;
    UtlString mVerifyCommand;
    UtlString mVerifyParameters;
    UtlString mVerifyDefaultDir;
    UtlString Dependents[MAX_DEPENDENCIES];
    int mNumDependents;
    UtlBoolean mbCanStart;
    UtlBoolean mbCanRestart;
    UtlBoolean mbCanStop;

    int mDelaySecs;
    //: Time in secs to delay after running the process

};

/* ============================ INLINE METHODS ============================ */

#endif  // _DependencyList_h_


