//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//
// $$
////////////////////////////////////////////////////////////////////////

#ifndef _OsProcessLinux_h_
#define _OsProcessLinux_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
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

//: This encapsulates a pid, and allows querying, killing and all the 
//: other cool things you want to do to a process.

class OsProcessLinux : public OsProcessBase
{

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
   OsProcessLinux();
     //:Default constructor

   virtual ~OsProcessLinux();
     //:Destructor

/* ============================ MANIPULATORS ============================== */
    virtual OsStatus launch(UtlString &rAppName, UtlString parameters[], OsPath &startupDir,
                    OsProcessPriorityClass prio = NormalPriorityClass, UtlBoolean bExeclusive = FALSE);
    //: Pass the appname and parameters to start the process
    //: Returns TRUE if process started ok.
    //: If bExclusive is TRUE and another process by the same name already
    //: is running the return is OS_FAILED


    virtual OsStatus kill();
    //: Kills the process specified by pid

    virtual OsStatus setPriority(int prio);
    //: Changes the process priority.  Must own the process for this to be legal.
    
    static OsStatus getByPID(PID pid, OsProcessLinux &rProcess);
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

    int wait(int WaitInSecs);
    //: waits n seconds for the process to terminate.
    //: if you pass 0 then it waits indefinately

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

/* ============================ INLINE METHODS ============================ */


#endif  // _OsProcessLinux_h_


