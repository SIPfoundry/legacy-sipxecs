// 
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//////////////////////////////////////////////////////////////////////////////
#ifndef _PROCESS_H_
#define _PROCESS_H_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "utl/UtlSList.h"
#include "os/OsFS.h"
#include "ProcessTask.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipxResource;
class ProcessResource;
class ProcessTask;
class ProcessCmd;

class ProcessTest;

/// Manage a single sipXecs service process.
/**
 * A Process object represents the process that is providing some
 * sipXecs service.  It is instantiated by the ProcessManager by calling
 * the Process::createFromDefinition method, passing the path to an XML
 * process definition file.
 *
 * To locate a particular Process object, call ProcessManager::find.
 *
 * @par(Process and Resource creation)
 *
 * Process objects are created by the createFromDefinition, called from
 * ProcessManager::instantiateProcesses with each process definition document (see
 * sipXsupervisor/meta/sipXecs-process.xsd.in). 
 *
 * The parsing of the 'resources' element is delegated to SipxResource::parse,
 * which in turn delegates it to the appropriate resource subclass.  The connections
 * from a Process to the SipxResources it depends on are created by callbacks from
 * those resource parsing routines:
 
 * @msc
 *    Process, SipxResource, xxxResource;
 *
 *               ---    [label="Undefined State (parsing definition)"];
 *    Process        =>     SipxResource                     [label="SipxResource::parse"];
 *                          SipxResource =>   xxxResource    [label="xxxResource::parse"];
 *    Process           <=                    xxxResource    [label="requireResourceToStart"];
 * Process -> Process                                 [label="insert in mRequiredStart"];
 *    Process           >>                    xxxResource    ;
 *    Process           <=                    xxxResource    [label="checkResourceBeforeStop"];
 * Process -> Process                                 [label="insert in mWaitForStop"];
 *    Process           >>                    xxxResource    ;
 *                          SipxResource <<   xxxResource    [label="(bool)"];
 *    Process        <<     SipxResource                     [label="(bool)"];
 *
 * @endmsc
 *
 * @note If the SipxResource::parse returns 'false', the definition is considered invalid,
 *       and the Process object is discarded, so any dependencies that go with it are also
 *       eliminated.  It is possible that this will result in 'orphaned' SipxResource objects.
 *
 * Each Process object has a paired ProcessResource object.
 *
 * @par(Process State Machine:)
 *
 * The following shows the states and transition events for a Process:
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
class Process : public UtlString
{
  public:

// ================================================================
/** @name           Constructor
 *
 */
///@{
   /// Read a process definition and return a process if definition is valid.
   static Process* createFromDefinition(const OsPath& definitionFile);

///@}
// ================================================================
/** @name           State Manipulation
 *
 */
///@{

   /// The current condition of the service this Process object controls.
   typedef enum
   {
      Undefined,               ///< Process definition is still being parsed.
      Disabled,                ///< Process is not started when instantiated.
      Testing,                 ///< Checking resources to see if it can start.
      ResourceRequired,        ///< Waiting for some resource.
      ConfigurationMismatch,   ///< The stored configuration version does not match this Process
      ConfigurationTestFailed, ///< The stored configuration version does not match this Process
      Starting,                ///< Start command executed, service is not yet running.
      Running,                 ///< Service is running.
      AwaitingReferences,      ///< Waiting for dependent processes to stop before Stopping.
      Stopping,                ///< Stop command executed, service process still exists.
      Failed                   ///< Service process exitted unexpectedly.
   } State;
   
   /// Return the current state of the Process.
   State getState();

   /// Return whether or not the service for this process should be Running.
   bool isEnabled();
   /**< @returns true if the desired state of this service is Running;
    *            this does not indicate anything about the current state of
    *            the service: for that see getState
    */

   /// Set the persistent desired state of the Process to Running.
   void enable();

   /// Set the persistent desired state of the Process to Disabled.
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

   /// Notify the Process that some configuration change has occurred.
   void configurationChange(const SipxResource& changedResource);
   
   /// Notify the Process that the version stamp value of the configuration has changed.
   void configurationVersionChange();
   
   /// Compare actual process state to the desired state, and attempt to change it if needed.
   void checkService();
   
// ================================================================
/** @name           Destructor
 *
 */
///@{

   /// destructor
   virtual ~Process();

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

   void requireResourceToStart(const SipxResource* requiredResource);

   void checkResourceBeforeStop(const SipxResource* requiredResource);

  protected:

   /// Save the persistent desired state.
   void persistDesiredState(State persistentState ///< may only be Disabled or Running
                            );

   /// Read the persistent desired state.
   State readPersistentState();
   ///< @returns Disabled if no persistent desired state is set.

  private:

   OsBSem           mLock;          ///< must be held to access to other member variables.

   UtlString        mVersion;       /**< Version of the process definition.
                                     *   For comparison with the configuration version.
                                     *   The Process may not be started unless they match.
                                     */
   State            mDesiredState;  ///< May be only Running or Disabled.

   ProcessTask*     mpProcessTask;  ///< Receives timer events for this service.

   ProcessCmd*      mConfigtest;    ///< from the sipXecs-process/commands/configtest element
   ProcessCmd*      mStart;         ///< from the sipXecs-process/commands/start element
   ProcessCmd*      mStop;          ///< from the sipXecs-process/commands/stop element
   ProcessCmd*      mReconfigure;   ///< from the sipXecs-process/commands/reconfigure element

   UtlString        mPidFile;       ///< from the sipXecs-process/status/pid element
   UtlSList         mLogFiles;      /**< from the sipXecs-process/status/log elements
                                     *   this is a list of FileResource objects created using
                                     *   the FileResource::logFileResource constructor
                                     */
   
   UtlSList         mRequiredStart; /**< Lists SipxResource objects that are required
                                     *   to be ready before starting this service.
                                     *   These objects are owned by the SipxResourceManager,
                                     *   so they are _not_ deleted when this Process
                                     *   is destructed (but they are removed from
                                     *   this list).
                                     */
   UtlSList         mWaitForStop;   /**< lists SipxResource objects that this process should
                                     *   wait until they are _not_ ready before stopping
                                     *   this service.
                                     *   These objects are owned by the SipxResourceManager,
                                     *   so they are _not_ deleted when this Process
                                     *   is destructed (but they are removed from
                                     *   this list).
                                     */

   /// constructor
   Process(const UtlString& name,
           const UtlString& version
           );

   // @cond INCLUDENOCOPY
   /// There is no copy constructor.
   Process(const Process& nocopyconstructor);

   /// There is no assignment operator.
   Process& operator=(const Process& noassignmentoperator);
   // @endcond     

   friend class ProcessDefinitionParserTest;
};

#endif // _PROCESS_H_
