//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsProcessWnt_h_
#define _OsProcessWnt_h_

// SYSTEM INCLUDES
#include <windows.h>

// APPLICATION INCLUDES
#include "utl/UtlDefs.h"
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "os/OsProcess.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
class UtlString;

//: This encapsulates a pid, and allows querying, killing and all the
//: other cool things you want to do to a process.

class OsProcessWnt : public OsProcessBase
{

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
   OsProcessWnt();
     //:Default constructor

   virtual ~OsProcessWnt();
     //:Destructor

/* ============================ MANIPULATORS ============================== */
    virtual OsStatus launch(UtlString &rAppName, UtlString parameters[], OsPath &startDir,
                    OsProcessPriorityClass prio = NormalPriorityClass, UtlBoolean bExclusive = FALSE, UtlBoolean bIgnoreChildSignals = TRUE);
    //: Pass the appname and parameters to start the process
    //: Returns TRUE if process started ok.
    //: If bExclusive is TRUE and another process by the same name already
    //: is running the return is OS_FAILED


    virtual OsStatus kill();
    //: Kills the process specified by pid

    virtual OsStatus setPriority(int prio);
    //: Changes the process priority.  Must own the process for this to be legal.

    static OsStatus getByPID(PID pid, OsProcessWnt &rProcess);
    //: Given a PID, this method will fill in the process passed in so the user
    //: can then manipulate it

    virtual OsStatus setIORedirect(OsPath &rStdInputFilename, OsPath &rStdOutputFilename, OsPath &rStdErrorFilename);
    //: Sets the standard input, output and/or stderror

/* ============================ ACCESSORS ================================= */

    static PID getCurrentPID();
    //: Returns the current process id.

    virtual OsStatus getPriority(int &rPrio);
    //: Returns the process priority.  Must own the process for this to be legal.

    virtual OsStatus getPriorityClass(OsProcessPriorityClass &rPrioClass);
    //: Returns the Priority Class for this process.  Priority is a function of the class.

    virtual OsStatus getMinPriority(int &rMinPrio);
    //: Returns the min priority base on which class is selected

    virtual OsStatus getMaxPriority(int &rMaxPrio);
    //: Returns the max priority base on which class is selected

    virtual OsStatus getInfo(OsProcessInfo &rProcessInfo);
    //: Returns full information on process, including priority.
    //: See OsProcessInfo for more information

    virtual OsStatus getUpTime(OsTime &rUpTime);
     //: How long has this process been runnign for?

/* ============================ INQUIRY =================================== */

    virtual UtlBoolean isRunning () const ;
    //: Returns TRUE if process is still active

    virtual int wait(int numSecs = -1);
    //:waits for a process to complete before returning
    //:or exits when WaitInSecs has completed

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    HANDLE mStdInputHandle;
    HANDLE mStdOutputHandle;
    HANDLE mStdErrorHandle;
    HANDLE mhProcess; //handle to process
    HANDLE mhThread; //handle to main thread

};

/* ============================ INLINE METHODS ============================ */


#endif  // _OsProcessWnt_h_
