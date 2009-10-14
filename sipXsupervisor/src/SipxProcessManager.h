//
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef _PROCESSMANAGER_H_
#define _PROCESSMANAGER_H_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsFS.h"
#include "utl/UtlHashBag.h"
#include "utl/UtlHashMap.h"

// DEFINES
#define SUPERVISOR_PROCESS_NAME "sipxsupervisor"

// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipxProcess;

/// Instantiates SipxProcess objects and provides access to them.
/**
 *
 */
class SipxProcessManager
{
  public:

   /// Singleton accessor
   static SipxProcessManager* getInstance();

   /// destructor
   virtual ~SipxProcessManager();

   /// Save a process definition.
   void save(SipxProcess* process);
   ///< called from within SipxProcess::createFromDefinition

   /// Locate a SipxProcess object by name.
   SipxProcess* findProcess(const UtlString& processName);

   /// Parse the XML of a process definition
   void instantiateProcesses(const OsPath& processDefinitionDirectory);

   /// Activate all process monitors
   void startProcesses();

   /// Shutdown all processes
   void shutdown();

   /// Fill in a map of process names and states (as UtlStrings)
   void getProcessStateAll(UtlHashMap& processStates ///< key->name, value->state string
                            );
   ///< @note The caller is responsible for destroying the map contents.

  protected:
   friend class SipxProcessManagerTest;

  private:

   static OsBSem sSingletonLock;       ///< protects access to spSingleton
   static SipxProcessManager* spSingleton; ///< pointer to the one SipxProcessManager

   OsBSem     mProcessTableLock;  ///< protects access to mProcesses
   UtlHashBag mProcesses;              ///< contains all SipxProcess objects

   /// constructor
   SipxProcessManager();

   // @cond INCLUDENOCOPY
   /// There is no copy constructor.
   SipxProcessManager(const SipxProcessManager& nocopyconstructor);

   /// There is no assignment operator.
   SipxProcessManager& operator=(const SipxProcessManager& noassignmentoperator);
   // @endcond
};

#endif // _PROCESSMANAGER_H_
