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

/// The states for any named process.
typedef enum
{
   PROCESS_STARTED = 1,
   PROCESS_STOPPED = 2,
   PROCESS_FAILED  = 3,
   PROCESS_NEVERRUN = 4,
   PROCESS_STOPPING = 5,
   PROCESS_STARTING = 6
} ProcessState;
///< @note This must agree with the names in OsProcessMgr::ProcessStateName.


// User requested states
typedef enum
{
   USER_PROCESS_NONE = 0,
   USER_PROCESS_START = 1,
   USER_PROCESS_STOP = 2,
   USER_PROCESS_RESTART = 3
} RequestedProcessState;

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

   /// Start process.
   OsStatus startProcess(UtlString &rAlias,
                         UtlString &rExeName,
                         UtlString rParameters[],
                         UtlString &startupDir,
                         OsProcessBase::OsProcessPriorityClass prio = OsProcessBase::NormalPriorityClass,
                         UtlBoolean bExeclusive = FALSE
                         );

   /// Sets the standard input, output and/or stderror.
   OsStatus setIORedirect(OsPath &rStdInputFilename,
                          OsPath &rStdOutputFilename,
                          OsPath &rStdErrorFilename
                          );
   /**< Applies to all processes created from this point on
    *   Set them to "" to provide the default action.
    */

   /// Stop process by alias.
   OsStatus stopProcess(UtlString &rAlias);

   /// Stop process by id.
   OsStatus stopProcess(PID pid);

   void setAliasStopped(const UtlString &rAlias);
    
   OsStatus setUserRequestState(const UtlString &rAlias, const int userRequestedState);
   /**<
    * Sets a state which the watchdog periodically checks (handleMessage()) to 
    * determine if a user wishes to change the state of a process.  
    * OsProcessMgr directly does not effect the user requested state change.
    * It is up to the watchdog to read this state via getUserRequestState() 
    * and invoke the change accordingly.
    */

/* ============================ ACCESSORS ================================= */

   /// Gets the state which a user set when they wish to change the state of a process.  
   int getUserRequestState(UtlString &rAlias);

   /// Retrieve process object given ID.
   OsStatus getProcessByAlias(const UtlString &rAlias, OsProcess &rProcess);

   /// Retrieves the alias if you know the pid.
   OsStatus getAliasByPID(PID pid ,UtlString &rAlias);
   ///< @return OS_SUCCESS if found, or OS_FAILED if....failed.

   /// returns the one and only process manager.
   static OsProcessMgr *getInstance();

   static bool getCurrentStateString(const int, UtlString&);
   static bool getCurrentStateFromString(const UtlString&, int&);

   static bool getUserRequestedStateString(const int, UtlString&);
   static bool getUserRequestedStateFromString(const UtlString&, int&);


/* ============================ INQUIRY =================================== */
  UtlBoolean isStarted(UtlString &rAlias);

  /// @return the state of the alias.
  int getAliasState(const UtlString &rAlias);

  /// get the name for a ProcessState value.
  static const char* ProcessStateName(int state);
  
/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    OsStatus setAliasState(const UtlString &rAlias,int state);

    /// pointer to the one and only process manager.
    static OsProcessMgr * spManager;
    
    /// Where will input come from?
    OsPath mStdInputFilename;

    /// Where will output go to?
    OsPath mStdOutputFilename;

    /// Where will errors be sent?
    OsPath mStdErrorFilename;

    /// Add a new entry to the collection of monitored processes.
    void addEntry(const UtlString &rAlias, int pid);
    /**<
     * The removeEntry() method will always be called to first clear any 
     * existing entry.
     * @return OS_SUCCESS if added ok or OS_FAILED on failure.
     */

    /// Remove the entry from the collection of monitored processes.
    void removeEntry(const UtlString &rAlias);

    /// The current PROCESS_XXX state strings indexed for all monitored processes.
    UtlHashMap mCurrentStateMap;
    ///< When PROCESS_STARTED, the value is actually the PID of the started process.


    /// The pending user requested states.
    UtlHashMap mUserRequestedStateMap;
    ///< indexed by the monitored process the change applies to.

};

/* ============================ INLINE METHODS ============================ */


#endif  // _OsProcessMgr_h_


