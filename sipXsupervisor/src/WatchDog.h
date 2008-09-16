// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _WatchDog_h_
#define _WatchDog_h_

// SYSTEM INCLUDES
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "os/OsServerTask.h"

// APPLICATION INCLUDES
#include "processXMLCommon.h"
#include "MonitoredProcess.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class FailureReporterBase;
class OsQueuedEvent;
class OsTimer;
class XmlRpcDispatch;


//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class WatchDog : public OsServerTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   WatchDog(int nWatchInterval,MonitoredProcess **processList,int processCount,
            const int port,UtlSList& allowedPeers);
   /**<
    * Default constructor (takes time in secs, process list, process count)
    * This object become the owner of the new'd memory in the 'allowedPeers'
    * list.  It will delete them upon destruction.
    */

   virtual
   ~WatchDog();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   WatchDog& operator=(const WatchDog& rhs);
     //:Assignment operator
   
   UtlBoolean handleMessage(OsMsg &rMsg);
    //: Handles our timer message

   /// top level task
   virtual int run(void* pArg);

/* ============================ ACCESSORS ================================= */

   /// Get the XML-RPC dispatcher
   XmlRpcDispatch* getXmlRpcDispatch();

   /// Returns the state of the specified process being monitored by the watchdog.
   int getProcessState(const UtlString& alias);

   /// Returns the state of all processes being monitored by the watchdog.
   void getProcessStateAll(UtlHashMap& process_states);
   /**<
    * Returns a map of each monitored process's alias to its state (both 
    * UtlString pointers to new'd memory.)  The caller is responsible for 
    * destroying the dynamically allocated memory returned in the 
    * process_states object (using destroyAll().)
    * 
    * Locks the semaphore while using the OsProcessMgr singleton.
    * 
    * \param process_states Output of the monitored process aliases and states.
    */

   /// Sets the specified "user request state" for the specified process.
   bool setProcessUserRequestState(const UtlString& alias, const int state);
   /**<
    * Locks the semaphore, then calls 'setProcessUserRequestStateNoLock'.
    * 
    * \see setProcessUserRequestStateNoLock
    */

   /// Sets the specified "user request state" for all processes being monitored by the watchdog.
   void setProcessUserRequestStateAll(const int state, UtlHashMap& process_results);
   /**<
    * The setProcessUserRequestState() is called for each monitored process.  
    * 
    * Returns a map of each monitored process's alias (UtlString pointer) to the 
    * result of the corresponding setProcessUserRequestStateNoLock() call 
    * (UtlBoolean pointer.)  The caller is responsible for destroying the 
    * dynamically allocated memory returned in the process_states object (using 
    * destroyAll().)
    *
    * Locks the semaphore while using the the mpProcessList member and calling 
    * 'setProcessUserRequestStateNoLock'.
    * 
    * \note This method does not block on the process state changes, and 
    * therefore does not report on the result of the requested state changes.
    * (For example, calling this method with USER_PROCESS_STOP will not result
    * in a failure for an already stopped process.)
    * 
    * \param state The state to set.
    * \param process_results Output of the monitored process aliases and change results.
    * \see setProcessUserRequestStateNoLock
    */


   /// Sets the specified "user request state" for all processes in a list.
   void setProcessUserRequestStateList(const int state, const UtlSList& alias_list, UtlHashMap& process_results);
   /**<
    * The setProcessUserRequestState() is called for each process.  
    * 
    * Returns a map of each monitored process's alias (UtlString pointer) to the 
    * result of the corresponding setProcessUserRequestStateNoLock() call 
    * (UtlBoolean pointer.)  The caller is responsible for destroying the 
    * dynamically allocated memory returned in the process_states object (using 
    * destroyAll().)
    *
    * Locks the semaphore while using the the mpProcessList member and calling 
    * 'setProcessUserRequestStateNoLock'.
    * 
    * \note This method does not block on the process state changes, and 
    * therefore does not report on the result of the requested state changes.
    * (For example, calling this method with USER_PROCESS_STOP will not result
    * in a failure for an already stopped process.)
    * 
    * \param state The state to set.
    * \param process_results Output of the monitored process aliases and change results.
    * \see setProcessUserRequestStateNoLock
    */

/* ============================ INQUIRY =================================== */

   /// Returns the alias of the monitored processes with the specified PID.
   UtlString getAliasByPid(const PID pid);
   /**<
    * Logs (at the PRI_INFO level) if the lookup failed.
    * 
    * Locks the semaphore while using the OsProcessMgr singleton.
    * 
    * \param pid The PID of the monitored processes whose alias is to be returned.
    * \return The alias of the monitored processes with the specified PID if a 
   *  matching alias was found, blank otherwise.
    * \see OsProcessMgr
    */

   /// Returns the PID of the monitored processes with the specified alias.
   PID getPidByAlias(const UtlString &alias);
   /**<
    * Logs (at the PRI_INFO level) if the lookup failed.
    * 
    * Locks the semaphore while using the OsProcessMgr singleton.
    * 
    * \param alias The alias of the monitored processes whose PID is to be returned.
    * \return The PID if a matching alias was found, 0 otherwise.
    * \see OsProcessMgr
    */

   /// Returns the PIDs of all monitored processes mapped by alias.  The caller is
   /// responsible for deleting the new'd memory.
   void getAllPids(UtlHashMap& pids);

   /// Returns the PIDs of all requested processes mapped by alias.  The caller is
   /// responsible for deleting the new'd memory.
   void getPidsByAliasList(const UtlSList& alias_list, UtlHashMap& pids);

   /// Whether or not the specified peer is allowed to make XML-RPC Process Manamgement requests.
   bool isAllowedPeer(const UtlString& peer) const;
   /**<
    * The mAllowedPeers list is currently only modified once (during the thread
    * initialization.)  After that all access is thread-safe by virtue of being 
    * read-only.  i.e. Calling this method does not lock the semephore.
    * 
    * \param peer The name of the peer making the request.
    * \return True if the specified peer is allowed, false otherwise.
    * \see mAllowedPeers
    */

   /// Whether or not the specified service name is valid.
   bool isValidService(const UtlString& service) const;
   /**<
    * The semaphore should be locked when this method is called.
    * 
    * \param service The name of the service to search.
    * \return True if a matching service of that name was found, false otherwise.
    */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /// Server for the XML-RPC requests.
   void startRpcServer();

   /// Whether or not the monitored process list has a match for the specified alias.
   bool isValidAlias(const UtlString& alias);
   /**<
    * The semaphore should be locked when this method is called.
    * 
    * \return True if a matching alias was found, false otherwise.
    */

   /// Sets the specified "user request state" for the specified process.
   bool setProcessUserRequestStateNoLock(const UtlString& alias, const int state);
   /**<
    * Checks if the process specified by the alias can undergo a user requested 
    * change to the specified state by calling canProcessStateChange().  This 
    * includes validation of both the specified alias string and state value.
    * 
    * If the parameters are OK, the method then calls setUserRequestState() with
    * on the OsProcessMgr singleton, with the specified alias and state.  The 
    * WatchDog's user event processing will (hopefully) then perform the actual 
    * process state change.  
    * 
    * This method does not alock the semaphore, that is the caller's responsibility.
    * 
    * \note This method does not block on the process state change, and 
    * therefore does not report on the result of the requested state change.
    * (For example, calling this method with USER_PROCESS_STOP on an already 
    * stopped process will not result in a failure.)
    * 
    * \param alias The alias of the process.
    * \param state The state to set.
    * \return True if a matching alias was found, the state was valid, the process
    * can undergo the state change, and OsProcessMgr::setUserRequestState() succeeded.
    * Returns false otherwise.
    * \see OsProcessMgr
    * \see canProcessStateChange
    */


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   OsBSem mLock; // Semaphore to enable thread-safe support.

   OsTimer*            mpWatchDogTimer; // Timer evaluating time to check processes
   OsQueuedEvent*      mpWatchDogEvent;  // Event marking time to check processes

   OsTimer*            mpWatchDogUserChangeTimer; // Timer evaluating time to check if user modified processes
   OsQueuedEvent*      mpWatchDogUserChangeEvent;  // Event marking time to check if user modified processes

   MonitoredProcess**  mpProcessList;    //list of processes to monitor
   int mnProcessCount;   //How many are there to monitor

   int             mXmlRpcPort;
   XmlRpcDispatch* mpXmlRpcDispatch;

   UtlSList  mAllowedPeers;  // The list of peers allowed to make XML-RPC Process 
                             // Management requests.

   WatchDog(const WatchDog& rWatchDog);
     //:Copy constructor (not implemented for this class)


};

/* ============================ INLINE METHODS ============================ */

#endif  // _WatchDog_h_
