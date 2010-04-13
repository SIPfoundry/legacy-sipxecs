//
// Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef _SIPXSUPERVISORPROCESS_H_
#define _SIPXSUPERVISORPROCESS_H_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "SipxProcess.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS


/// Manage a single sipXecs service process.
/**
 * A SipxSupervisorProcess object is a special instance of SipxProcess
 * for the sipxsupervisor.  It operates somewhat differently from
 * other sipx services, since it represents "itself" and does not run
 * a state machine in another task.
 *
 */
class SipxSupervisorProcess : public SipxProcess
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

   /// Return whether or not the service for this process is Running.
   bool isRunning();

   /// Set the persistent desired state of the SipxProcess to Running, and start the process.
   bool enable();

   /// Set the persistent desired state of the SipxProcess to Disabled, and stop the process.
   bool disable();

   /// Stop and restart the process without changing the persistent desired state
   bool restart();

   /// Shutting down sipXsupervisor, so shut down the service.
   void shutdown();
   ///< This does not affect the persistent state of the service.

  private:


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
   void evCommandStopped(const SipxProcessCmd* command, int rc);

   /// Notify the SipxProcess that a command has received output.
   void evCommandOutput(const SipxProcessCmd* command,
                        OsSysLogPriority pri,
                        UtlString message);

   // Trigger SipxProcessCmd to execute commands (in its own task)
   void startConfigTest();
   void killConfigTest();
   void startProcess();
   void stopProcess();

  private:

// ================================================================
/** @name           Destructor
 *
 */
///@{

  public:

   /// destructor
   virtual ~SipxSupervisorProcess();

///@}

// ================================================================

  private:

   /// constructor
   SipxSupervisorProcess(const UtlString& name,
               const UtlString& version,
               const OsPath&    definitionPath
               );

   // @cond INCLUDENOCOPY
   /// There is no copy constructor.
   SipxSupervisorProcess(const SipxSupervisorProcess& nocopyconstructor);

   /// There is no assignment operator.
   SipxSupervisorProcess& operator=(const SipxSupervisorProcess& noassignmentoperator);
   // @endcond

};


#endif // _SIPXSUPERVISORPROCESS_H_
