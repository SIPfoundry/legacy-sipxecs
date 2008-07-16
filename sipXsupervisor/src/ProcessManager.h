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
#include "utl/UtlHashBag.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS
class Process;

/// Instantiates Process objects and provides access to them.
/**
 * 
 */
class ProcessManager
{
  public:

   /// Singleton accessor
   static ProcessManager* getInstance();
   
   /// destructor
   virtual ~ProcessManager();

   /// Locate a Process object by name.
   Process* findProcess(const UtlString& processName);

   /// Parse the XML of a process definition
   void instantiateProcesses(const OsPath& processDefinitionDirectory);
   
  protected:
   friend class ProcessManagerTest;
      
  private:

   static OsBSem sSingletonLock;       ///< protects access to spSingleton
   static ProcessManager* spSingleton; ///< pointer to the one ProcessManager
   
   OsBSem     mProcessTableLock;  ///< protects access to mProcesses
   UtlHashBag mProcesses;              ///< contains all Process objects
   
   /// constructor
   ProcessManager();

   // @cond INCLUDENOCOPY
   /// There is no copy constructor.
   ProcessManager(const ProcessManager& nocopyconstructor);

   /// There is no assignment operator.
   ProcessManager& operator=(const ProcessManager& noassignmentoperator);
   // @endcond     
};

#endif // _PROCESSMANAGER_H_
