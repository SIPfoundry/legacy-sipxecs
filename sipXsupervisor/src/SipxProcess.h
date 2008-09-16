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
#include "os/OsFS.h"
#include "os/OsProcess.h"

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
 * Testing [label="Testing"];
 * Starting [label="Starting"];
 * AwaitingReferences [label="AwaitingReferences"];
 * Stopping [label="Stopping"];
 * 
 * ConfigurationMismatch  [label="ConfigurationMismatch"];
 * ResourceRequired       [label="ResourceRequired"];
 * ConfigurationTestFailed [label="ConfigurationTestFailed"];
 * Failed [label="Failed"];
 * 
 * Disabled -> Testing [label="enable"];
 * 
 * Testing -> Starting [label="passed"];
 * 
 * Testing -> ConfigurationMismatch [label="failed"];
 * 
 * ConfigurationMismatch ->Testing [label="set cfg version"];
 * 
 * Testing -> ResourceRequired [label="failed"];
 * 
 * ResourceRequired -> Testing [label="created"];
 * 
 * Testing -> ConfigurationTestFailed [label="failed"];
 * 
 * ConfigurationTestFailed -> Testing [label="created"];
 * 
 * Starting -> Running [label="started"];
 * 
 * Running -> AwaitingReferences [label="stop"];
 * 
 * AwaitingReferences -> Stopping [label="dependencies stopped"];
 *
 * Stopping -> Disabled [label="exit"];
 * 
 * Running -> Failed [label="exit"];
 * 
 * Running -> Starting [label="restart"];
 * 
 * Starting -> Failed [label="exit"];
 * 
 * Failed -> Testing [label="retry"];
 * 
 * Undefined -> Testing [label="boot"];
 * 
 * };
 * @enddot
 *
 */
class SipxProcess : public UtlString
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

   /// The current condition of the service this SipxProcess object controls.
   /*    If you modify this, you must also modify the SipxProcessStateNames constant
    *    in SipxProcess.ccp */
   typedef enum
   {
      Undefined,               ///< SipxProcess definition is still being parsed.
      Disabled,                ///< SipxProcess is not started when instantiated.
      Testing,                 ///< Checking resources to see if it can start.
      ResourceRequired,        ///< Waiting for some resource.
      ConfigurationMismatch,   ///< The configuration version does not match this SipxProcess
      ConfigurationTestFailed, ///< The configuration validation test failed
      Starting,                ///< Start command executed, service is not yet running.
      Running,                 ///< Service is running.
      AwaitingReferences,      ///< Waiting for dependent processes to stop before Stopping.
      Stopping,                ///< Stop command executed, service process still exists.
      Failed                   ///< Service process exitted unexpectedly.
   } State;

   /// Return the current state of the SipxProcess.
   State getState();

   /// Return whether or not the service for this process should be Running.
   bool isEnabled();
   /**< @returns true if the desired state of this service is Running;
    *            this does not indicate anything about the current state of
    *            the service: for that see getState
    */

   /// Set the persistent desired state of the SipxProcess to Running.
   void enable();

   /// Set the persistent desired state of the SipxProcess to Disabled.
   void disable();

   /// Shutting down sipXsupervisor, so shut down the service.
   void shutdown();
   ///< This does not affect the persistent state of the service.


///@}
// ================================================================
/** @name           Events
 *
 */
///@{

  public:

   /// Begin monitoring the process and getting it to the desired state.
   void start();
   /**< Before this call, state changes and events modify the
    *   internal state of the object but don't actually change
    *   the corresponding system process (if any).
    */

   /// Notify the SipxProcess that some configuration change has occurred.
   void configurationChange(const SipxResource& changedResource);

   /// Events that may trigger state changes.
   typedef enum 
   {
      Startup,
      ConfigurationChange,
      ConfigurationVersionUpdate,
      TestPass,
      TestFail,
      CheckState,
      ProcessRunning,
      ProcessExit,
      Shutdown
   } Event;

   const char* eventName(Event event);

   /// Compare actual process state to the desired state, and attempt to change it if needed.
   void checkService(Event event);
   ///< This should be called only from the SipxProcessTask thread.

  private:

   /// Signal the SipxProcessTask to do a checkService call.
   void triggerServiceCheck(Event event);
   
   /// convenience routine to log an error
   void unexpectedEvent(const char* methodName, Event event);
   
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

  private:

   /// Check that all resources on the mRequiredResources list are ready so this can start.
   bool resourcesAreReady();
   
///@}
// ================================================================
/** @name           Persistent State Manipulation
 *
 */
///@{

  public:

   /// Translate the string from of the state name to the enum
   static State state(const UtlString& stringStateValue);
   
   /// Translate the string from of the state name to the enum
   static const char* state(State stateValue);

  protected:

   /// Save the persistent desired state from mDesiredState.
   void persistDesiredState();
   ///< caller must be holding mLock.

   /// Read the persistent desired state into mDesiredState.
   void readPersistentState();
   ///< sets mDesiredState to Undefined if no persistent desired state is set.
   
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

   OsBSem           mLock;          ///< must be held to access to other member variables.

   SipxProcessResource* mSelfResource;  ///< the SipxProcessResource for this SipxProcess.

   UtlString        mVersion;       /**< Version of the process definition.
                                     *   For comparison with the configuration version.
                                     *   The SipxProcess may not be started unless they match.
                                     */
   UtlString        mConfigVersion; /**< Version of the process definition.
                                     *   For comparison with the configuration version.
                                     *   The SipxProcess may not be started unless they match.
                                     */
   State            mDesiredState;  ///< May be only Running or Disabled.
   State            mState;         ///< actual state of the process.

   SipxProcessTask* mpProcessTask;  ///< Receives timer events for this service.

   SipxProcessCmd*  mConfigtest;    ///< from the sipXecs-process/commands/configtest element
   SipxProcessCmd*  mStart;         ///< from the sipXecs-process/commands/start element
   SipxProcessCmd*  mStop;          ///< from the sipXecs-process/commands/stop element
   SipxProcessCmd*  mReconfigure;   ///< from the sipXecs-process/commands/reconfigure element

   OsProcess*       mpProcessInfo;  ///< object used to remember forked process's info
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

   friend class SipxProcessDefinitionParserTest;
   friend class SipxProcessStateTest;
   friend class SipxProcessVersionTest;
   friend class SipxResourceUseTest;
};

#endif // _SIPXPROCESS_H_
