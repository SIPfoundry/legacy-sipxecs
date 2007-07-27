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
#include "utl/UtlHashMap.h"
#include "utl/UtlHashMapIterator.h" 
#include "utl/UtlInt.h"

//to turn on debug string uncomment the next line
//#define DEBUG_OUTPUT

// DEFINES

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

/**
 This class is no longer available as a general mechanism for querying the state 
 of monitoried processes, or instructing the watchdog to perform state changes on 
 the monitoried processes.  It is now to be used by the watchdog process only.
 
 \par
 For monitoried process state queries or state change requests, use the watchdog's 
 XMLRPC interface.
 */
class OsProcessMgr
{
    friend class MonitoredProcess;

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
   OsProcessMgr();

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

   void setAliasStopped(const UtlString &rAlias);
    
   OsStatus setUserRequestState(const UtlString &rAlias, const int userRequestedState);
   //: Sets a state which the watchdog periodically checks (handleMessage()) to 
   //: determine if a user wishes to change the state of a process.  
   //: OsProcessMgr directly does not effect the user requested state change.
   //: It is up to the watchdog to read this state via getUserRequestState() 
   //: and invoke the change accordingly.

/* ============================ ACCESSORS ================================= */
   int getUserRequestState(UtlString &rAlias);
   //: Gets the state which a user set when they wish to change the state of a process.  

   OsStatus getProcessByAlias(const UtlString &rAlias, OsProcess &rProcess);
   //: Retrieve process object given ID.

   OsStatus getAliasByPID(PID pid ,UtlString &rAlias);
   //: Retrieves the alias if you know the pid
   //: Returns OS_SUCCESS if found, or OS_FAILED if....failed.

   static OsProcessMgr *getInstance();
   //: returns the one and only process manager

   static bool getCurrentStateString(const int, UtlString&);
   static bool getCurrentStateFromString(const UtlString&, int&);

   static bool getUserRequestedStateString(const int, UtlString&);
   static bool getUserRequestedStateFromString(const UtlString&, int&);


/* ============================ INQUIRY =================================== */
  UtlBoolean isStarted(UtlString &rAlias);

  int getAliasState(const UtlString &rAlias);
   //: Return the state of the alias


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    OsStatus setAliasState(const UtlString &rAlias,int state);

    static OsProcessMgr * spManager;
    //: pointer to the one and only process manager
    
    OsPath mStdInputFilename;
    //: Where will input come from?

    OsPath mStdOutputFilename;
    //: Where will output go to?

    OsPath mStdErrorFilename;
    //: Where will errors be sent?

    void addEntry(const UtlString &rAlias, int pid);
    //: Add a new entry to the collection of monitored processes.
    //: The removeEntry() method will always be called to first clear any 
    //: existing entry.
    //: Returns OS_SUCCESS if added ok or OS_FAILED on failure.

    void removeEntry(const UtlString &rAlias);
    //: Remove the entry from the collection of monitored processes.

    /** 
     The current PROCESS_XXX state strings indexed for all monitored processes.
     When PROCESS_STARTED, the value is actually the PID of the started process.
     */
    UtlHashMap mCurrentStateMap;

    /** 
     The pending user requested states indexed by the monitored process the  
     change applies to.
     */
    UtlHashMap mUserRequestedStateMap;
};

/* ============================ INLINE METHODS ============================ */


#endif  // _OsProcessMgr_h_


