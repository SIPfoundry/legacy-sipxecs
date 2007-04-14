//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsProcessMgr_h_
#define _OsProcessMgr_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "os/OsFS.h"
#include "os/OsProcess.h"
#include "os/OsProcessIterator.h"
#include "os/OsMutex.h"

//to turn on debug string uncomment the next line
//#define DEBUG_OUTPUT

// DEFINES

#define PROCESS_ALIAS_FILE        "processAlias.dat"
#define PROCESS_ALIAS_LOCK_FILE   "locked.lck"
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
    const int PROCESS_STARTED = 1;
    const int PROCESS_STOPPED = 2;
    const int PROCESS_FAILED  = 3;
    const int PROCESS_NEVERRUN = 4;
    const int PROCESS_STOPPING = 5;
    const int PROCESS_STARTING = 6;

// User requested states
    const int USER_PROCESS_NONE = 0;
    const int USER_PROCESS_START = 1;
    const int USER_PROCESS_STOP = 2;
    const int USER_PROCESS_RESTART = 3;

// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS

//: This encapsulates a pid, and allows querying, killing and all the
//: other cool things you want to do to a process.

class OsProcessMgr
{
    friend class MonitoredProcess;

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
   OsProcessMgr(const char* workingDirectory);

     //:Default constructor

   virtual ~OsProcessMgr();
     //:Destructor

/* ============================ MANIPULATORS ============================== */
   OsStatus startProcess(UtlString &rAlias, UtlString &rExeName, UtlString rParameters[], UtlString &startupDir,
            OsProcessBase::OsProcessPriorityClass prio = OsProcessBase::NormalPriorityClass,
       UtlBoolean bExeclusive = FALSE);
   //: Start process

   OsStatus setIORedirect(OsPath &rStdInputFilename, OsPath &rStdOutputFilename, OsPath &rStdErrorFilename);
   //: Sets the standard input, output and/or stderror
   //: Applies to all processes created from this point on
   //: Set them to "" to provide the defaul action.

   OsStatus stopProcess(UtlString &rAlias);
   //: Stop process

   OsStatus stopProcess(PID pid);
   //: Stop process by id

   void setProcessListFilename(UtlString &rFilename);
   //: Overrides the init file of processAlias.dat
   //: Working directory will be prepended.

   void setAliasStopped(UtlString &rAlias);
    
   OsStatus setUserRequestState(UtlString &rAlias, int userRequestedState);
   //: Sets a state which watchdog or another program can use to determine
   //: if a user wishes to change the state of a process.  
   //: ProcessMgr directly does not change the state of the process.
   //: This function is used to allow users to set a new state (as the third param)
   //: in processAlias.dat.  It is up to an external program to read this
   //: state via getUserRequestState and change the process state.

/* ============================ ACCESSORS ================================= */
   int getUserRequestState(UtlString &rAlias);
   //: Gets the state which a user set when they wish to change the state of a process.  
   //: ProcessMgr directly does not change the state of the process.
   //: This function is used to allow users to set a new state (as the third param)
   //: in processAlias.dat.  It is up to an external program to read this
   //: state and change the process state.

   OsStatus getProcessByAlias(UtlString &rAlias, OsProcess &rProcess);
   //: Retrieve process object given ID.

   OsStatus getAliasByPID(PID pid ,UtlString &rAlias);
   //: Retrieves the alias if you know the pid
   //: Returns OS_SUCCESS if found, or OS_FAILED if....failed.

   static OsProcessMgr *getInstance(const char* workingDirectory);
   //: returns the one and only process manager


/* ============================ INQUIRY =================================== */
  UtlBoolean isStarted(UtlString &rAlias);

  int getAliasState(UtlString &rAlias);
   //: Return the state of the alias

  void lockAliasFile();
  void unlockAliasFile();

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    OsStatus setAliasState(UtlString &rAlias,int state);


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    void getAliasFirstValue(UtlString &rinValue);
    //: helper func to pull the first part of the two part value

    void getAliasSecondValue(UtlString &rinValue);
    //: helper func to pull the second part of the two part value

    static OsProcessMgr * spManager;
    //: pointer to the one and only process manager
    
    OsPath mProcessFilename;
    //: Name of file which store the alias and PID

    OsPath mProcessLockFilename;
    //: Simple Lock file for cross process locking

    OsPath mWorkPath;
    //:Where files will be stored
    //: Defaults to ProcessMgr

    OsPath mStdInputFilename;
    //: Where will input come from?

    OsPath mStdOutputFilename;
    //: Where will output go to?

    OsPath mStdErrorFilename;
    //: Where will errors be sent?

    OsConfigDb *pProcessList;
    // Internal list of alias and PID's

    OsStatus addEntry(UtlString &rAlias, int pid);
    //: Add an entry to our process file
    //: Returns OS_SUCCESS if added ok
    //: or OS_FAILED on failure

    OsStatus removeEntry(UtlString &rAlias);
    //: Remove the entry from the file
    //: Returns OS_SUCCESS if removed
    //: or OS_FAILED on failure

    OsStatus loadProcessFile();
    //: Returns OS_SUCCESS if saved ok.
    //: or OS_FAILED on failure

    OsStatus storeProcessFile();
    //: Returns OS_SUCCESS if saved ok.
    //: or OS_FAILED on failure

    int mAliasLockFileCount;

    /** lock for synchronization */
    OsMutex mMutex;

};

/* ============================ INLINE METHODS ============================ */


#endif  // _OsProcessMgr_h_


