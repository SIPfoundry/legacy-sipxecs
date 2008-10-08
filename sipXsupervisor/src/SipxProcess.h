// 
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//////////////////////////////////////////////////////////////////////////////
#ifndef _SIPXPROCESS_H_
#define _SIPXPROCESS_H_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "utl/UtlSList.h"
#include "os/OsCallback.h"
#include "os/OsFS.h"
#include "os/OsServerTask.h"
#include "os/OsTimer.h"
#include "os/OsProcess.h"
#include "SipxProcessFsm.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipxResource;
class SipxProcessResource;
class SipxProcessTask;
class SipxProcessCmd;

class SipxProcessDefinitionParserTest;
class SipxProcessStateTest;

/// Manage a single sipXecs service process.
/**
 * A SipxProcess object represents the process that is providing some
 * sipXecs service.  It is instantiated by the SipxProcessManager by calling
 * the SipxProcess::createFromDefinition method, passing the path to an XML
 * process definition file.
 *
 * @image html uml.png "Overview of SipxProcesses and SipxResources"
 *
 * To locate a particular SipxProcess object, call SipxProcessManager::find.
 *
 * @par(SipxProcess and Resource creation)
 *
 * SipxProcess objects are created by the createFromDefinition, called from
 * SipxProcessManager::instantiateProcesses with each process definition document (see
 * sipXsupervisor/meta/sipXecs-process.xsd.in). 
 *
 * The parsing of the 'resources' element is delegated to SipxResource::parse,
 * which in turn delegates it to the appropriate resource subclass.  The connections
 * from a SipxProcess to the SipxResources it depends on are created by callbacks from
 * those resource parsing routines:
 
 * @msc
 *    SipxProcess, SipxResource, xxxResource;
 *
 *               ---    [label="Undefined State (parsing definition)"];
 *    SipxProcess        =>     SipxResource               [label="SipxResource::parse"];
 *    SipxProcess           <=  SipxResource               [label="requireResource"];
 *    SipxProcess -> SipxProcess                           [label="insert in mRequiredResources"];
 *    SipxProcess           >>  SipxResource               ;
 *                          SipxResource =>   xxxResource  [label="xxxResource::parse"];
 *                          SipxResource <=   xxxResource  [label="parseAttribute"];
 *    SipxProcess           <=  SipxResource               [label="resourceIsOptional"];
 *    SipxProcess -> SipxProcess                           [label="remove from mRequiredResources"];
 *    SipxProcess           >>  SipxResource               ;
 *                          SipxResource >>   xxxResource  ;
 *                          SipxResource <<   xxxResource  [label="(bool)"];
 *    SipxProcess        <<     SipxResource               [label="(bool)"];
 *
 * @endmsc
 *
 * @note If the SipxResource::parse returns 'false', the definition is considered invalid,
 *       and the SipxProcess object is discarded, so any dependencies that go with it are also
 *       eliminated.  This leaves the SipxProcessResource for this SipxProcess name, and possibly
 *       other SipxResource objects is created 'orphaned', but since they only retain the
 *       pointer to the SipxProcessResource (which continues to exist), this is ok.
 *
 * Each SipxProcess object has a paired SipxProcessResource object.
 *
 * @par(SipxProcess State Machine:)
 *
 * The following shows the states and transition events for a SipxProcess:
 *
 * @dot
 * digraph map {
 * rankdir=LR;
 * node [shape=ellipse];
 *
 * subgraph stable_states {
 * rank=same;
 * rankdir=LR;
 * node [shape=box];
 * Undefined     [label="Undefined"];
 * Disabled      [label="Disabled"];
 * Running [label="Running"];
 * }
 *
 * ConfigurationMismatch  [label="ConfigurationMismatch"];
 * ResourceRequired       [label="ResourceRequired"];
 * Testing [label="Testing"];
 * Starting [label="Starting"];
 * AwaitingReferences [label="AwaitingReferences"];
 * Stopping [label="Stopping"];
 * 
 * ConfigurationTestFailed [label="ConfigurationTestFailed"];
 * Failed [label="Failed"];
 * 
 * Disabled -> ConfigurationMismatch [label="enable"];
 * 
 * ConfigurationMismatch -> ResourceRequired [label="ConfigurationVersionUpdate matches"];
 * 
 * ResourceRequired -> Testing [label="all ResourcesAreReady"];
 * 
 * Testing -> Starting [label="passed"];
 * 
 * Testing -> ConfigurationTestFailed [label="failed"];
 * 
 * ConfigurationTestFailed -> Testing [label="ResourceCreated?"];
 * 
 * Starting -> Running [label="processStarted"];
 * 
 * Running -> AwaitingReferences [label="restart (stopProcess)"];
 * 
 * AwaitingReferences -> Stopping [label="dependencies stopped"];
 *
 * Stopping -> Starting [label="processStopped && isEnabled"];
 * 
 * Stopping -> Disabled [label="processStopped && !isEnabled"];
 * 
 * Running -> Failed [label="processStopped"];
 * 
 * Running -> ShuttingDown [label="shutdown"];
 * 
 * ShuttingDown -> Disabled [label="processStopped"];
 * 
 * Starting -> Failed [label="processStopped"];
 * 
 * Failed -> Testing [label="retry"];
 * 
 * Undefined -> ConfigurationMismatch [label="enable"];
 * 
 * };
 * @enddot
 *
 */
class SipxProcess : public UtlString, OsServerTask
{
  public:

// ================================================================
/** @name           Constructor
 *
 */
///@{
   /// Read a process definition and return a process if definition is valid.
   static SipxProcess* createFromDefinition(const OsPath& definitionFile);

///@}
// ================================================================
/** @name           State Manipulation
 *
 */
///@{

   /// Return whether or not the service for this process should be Running.
   bool isEnabled();
   /**< @returns true if the desired state of this service is Running;
    *            this does not indicate anything about the current state of
    *            the service: for that see isRunning
    */

   /// Return whether or not the service for this process is Running.
   bool isRunning();
   
   /// Set the persistent desired state of the SipxProcess to Running, and start the process.
   void enable();

   /// Set the persistent desired state of the SipxProcess to Disabled, and stop the process.
   void disable();

   /// Stop and restart the process without changing the persistent desired state
   void restart();

   /// Shutting down sipXsupervisor, so shut down the service.
   void shutdown();
   ///< This does not affect the persistent state of the service.

   /// Called from the task which is running the FSM, when it has shut down
   void done();
   
  private:
     /// start the FSM up in its own task
     void startStateMachine();

   ///@}
// ================================================================
/** @name           Events
 *
 */
///@{

  public:

   /// Notify the SipxProcess that some configuration change has occurred.
   void configurationChange(const SipxResource& changedResource);

   /// Notify the SipxProcess that some configuration change has occurred.
   void configurationVersionChange();

   /// Notify the SipxProcess that a command has completed starting.
   void evCommandStarted(const SipxProcessCmd* command);
   
   /// Notify the SipxProcess that a command has stopped.
   //@TODO: need stderr output etc from stopped/failed process
   void evCommandStopped(const SipxProcessCmd* command, int rc);
   
   /// Notify the SipxProcess that a timeout has occurred.
   void evRetryTimeout();
   
   /// Callback for timer task.
   static void timeoutCallback(void* userData, const intptr_t eventData);

   // Trigger SipxProcessCmd to execute commands (in its own task)
   void startConfigTest();
   void startProcess();
   void stopProcess();
   void startRetryTimer();
   void cancelRetryTimer();
   
   bool hadProcessFailed() {return (mRetries > 0);}

  private:

     /// Process asynchronous request from application code
     virtual UtlBoolean handleMessage(OsMsg& rMsg);

     // Handle events in FSM task
     void startStateMachineInTask();
     void evRetryTimeoutInTask();
     void evCommandStartedInTask(const SipxProcessCmd* command);
     void evCommandStoppedInTask(const SipxProcessCmd* command, int rc);
     void enableInTask();
     void disableInTask();
     void restartInTask();
     void shutdownInTask();
   
   
// ================================================================
/** @name           Destructor
 *
 */
///@{

  public:
   
   /// destructor
   virtual ~SipxProcess();

///@}
// ================================================================
/** @name           Container Support Operations
 *
 */
///@{

   /// Determine whether or not the values in a containable are comparable.
   virtual UtlContainableType getContainableType() const;
   /**<
    * This returns a unique type for UtlString
    */

   static const UtlContainableType TYPE;    ///< Class type used for runtime checking 

///@}
// ================================================================
/** @name           Resource Connection Operations
 *
 */
///@{

   /// Get the resource for this process.
   SipxProcessResource* resource();
   /**<
    * Using the resource provides a level of indirection in references to a
    * process.  This is important, since the actual SipxProcess object may never
    * be created if there is some error in the definition.  Since it's difficult
    * to unwind all the side effects of parsing a process definition, the other
    * resources that need to track what SipxProcess objects they are associated with
    * instead keep a pointer to the SipxProcessResource for it, which continues to
    * exist (owned by the SipxProcessResourceManager) even if the SipxProcess object does not.
    */

   /// Add this to the list of objects whose status is checked before starting or stopping.
   void requireResource(SipxResource* resource);

   /// Remove this from the list of objects whose status is checked before starting or stopping.   
   void resourceIsOptional(SipxResource* resource);

   /// Check that all resources on the mRequiredResources list are ready so this can start.
   bool resourcesAreReady();

  private:

   
///@}
// ================================================================
/** @name           Persistent State Manipulation
 *
 */
///@{

  public:

   // /////////////////////////////////////// //
   // state machine framework related methods //
   // /////////////////////////////////////// //
   const SipxProcessFsm* GetCurrentState();              ///< Returns the current state, as required by UtlFsm's StateAlg 
   void SetCurrentState( const SipxProcessFsm* pState ); ///< Sets the current state, as required by UtlFsm's StateAlg
   const char* name( void ) const;                       ///< Returns the state name string
   
   // ///////////////////////////////////////////////////////// //
   // states.  Note: the state objects are shared by all instances of this class.    //
   // ///////////////////////////////////////////////////////// //
   static Disabled*              pDisabled;
   static ConfigurationMismatch* pConfigurationMismatch;
   static ResourceRequired*      pResourceRequired;
   static Testing*               pTesting;
   static ConfigTestFailed*      pConfigTestFailed;
   static Starting*              pStarting;
   static Running*               pRunning;
   static Stopping*              pStopping;
   static Failed*                pFailed;
   static ShuttingDown*          pShuttingDown;
   static ShutDown*              pShutDown;
   static Undefined*             pUndefined;

  protected:

   /// Save the persistent desired state from mDesiredState.
   void persistDesiredState();
   ///< caller must be holding mLock.

   /// Read the persistent desired state into mDesiredState.
   void readPersistentState();
   ///< sets mDesiredState to Undefined if no persistent desired state is set.
   
  private:
   /// Initialize static state pointers
   void initializeStatePointers( void );
     
///@}
// ================================================================
/** @name           Configuration Version Manipulation
 *
 */
///@{
  public:

   /// Check whether or not the configuration version matches the process version.
   bool configurationVersionMatches();
   
   /// Set the version stamp value of the configuration.
   void setConfigurationVersion(const UtlString& version);
   ///< this class persists this value.

   /// Get the version stamp value of the configuration.
   void getConfigurationVersion(UtlString& version);

  protected:

   /// Read version stamp value of the configuration into mConfigVersion.
   void readConfigurationVersion();
   ///< caller must be holding mLock.

///@}
// ================================================================

  private:

   OsMutex          mLock;          ///< must be held to access to other member variables.

   SipxProcessResource* mSelfResource;  ///< the SipxProcessResource for this SipxProcess.

   UtlString        mVersion;       /**< Version of the process definition.
                                     *   For comparison with the configuration version.
                                     *   The SipxProcess may not be started unless they match.
                                     */
   UtlString        mConfigVersion; /**< Version of the process definition.
                                     *   For comparison with the configuration version.
                                     *   The SipxProcess may not be started unless they match.
                                     */
   SipxProcessFsm*       mpDesiredState;  ///< May be only Running or Disabled.
   const SipxProcessFsm* mpCurrentState;

   SipxProcessCmd*  mConfigtest;    ///< from the sipXecs-process/commands/configtest element
   SipxProcessCmd*  mStart;         ///< from the sipXecs-process/commands/start element
   SipxProcessCmd*  mStop;          ///< from the sipXecs-process/commands/stop element
   SipxProcessCmd*  mReconfigure;   ///< from the sipXecs-process/commands/reconfigure element

   UtlString        mPidFile;       ///< from the sipXecs-process/status/pid element
   UtlSList         mLogFiles;      /**< from the sipXecs-process/status/log elements
                                     *   this is a list of FileResource objects created using
                                     *   the FileResource::logFileResource constructor
                                     */
   
   UtlSList         mRequiredResources; /**< Lists SipxResource objects that are required
                                         *   to be ready before starting this service, and
                                         *   wait until they are _not_ ready before stopping
                                         *
                                         *   These objects are owned by the SipxResourceManager,
                                         *   so they are _not_ deleted when this SipxProcess
                                         *   is destructed (but they are removed from
                                         *   this list).
                                         */
   OsPath           mDefinitionFile; ///< path to *-process.xml file that defined this process.
   
   UtlBoolean       mbTaskRunning;     ///< true if the FSM is running in its own task
   OsTimer*         mpTimer;
   OsCallback*      mpTimeoutCallback;
   ssize_t          mRetries;          ///< number of times we have attempted to start process
   
   /// constructor
   SipxProcess(const UtlString& name,
               const UtlString& version,
               const OsPath&    definitionPath
               );

   // @cond INCLUDENOCOPY
   /// There is no copy constructor.
   SipxProcess(const SipxProcess& nocopyconstructor);

   /// There is no assignment operator.
   SipxProcess& operator=(const SipxProcess& noassignmentoperator);
   // @endcond     

   friend class SipxProcessFsm;     // the FSM has no data
   friend class SipxProcessDefinitionParserTest;
   friend class SipxProcessStateTest;
   friend class SipxProcessVersionTest;
   friend class SipxResourceUseTest;
};

/** 
 * Message sent by the SipxProcess in application thread to be handled in task */
class SipxProcessMsg : public OsMsg
{
public:
   
   enum EventSubType
   {   
      START_FSM   = 0,

      // events from "outside" the SipxProcess infrastructure
      ENABLE      = 1,
      DISABLE     = 2,
      RESTART     = 3,
      SHUTDOWN    = 4,
      RETRY_TIMEOUT = 5,
      
      // return events from the SipxProcessCmd task: must supply cmd
      STARTED     = 10,
      STOPPED     = 11
   };   

   /// Constructor.
   SipxProcessMsg(EventSubType eventSubType,
              const SipxProcessCmd* cmd = NULL,     ///< command which is returning
              int   rc = 0                          ///< optional return code from command
              );

   /// Destructor
   virtual ~SipxProcessMsg();

   // Component accessors.
   const SipxProcessCmd* getCmd( void ) const    {return mCmd;}
   int getRc( void )                    const    {return mRc;}
 
protected:
   static const UtlContainableType TYPE;   ///< Class type used for runtime checking

private:
   const SipxProcessCmd* mCmd;               ///< command which is returning
   int   mRc;                                ///< optional return code from command

   /// Copy constructor
   SipxProcessMsg( const SipxProcessMsg& rhs);
   virtual OsMsg* createCopy(void) const;
};

#endif // _SIPXPROCESS_H_
