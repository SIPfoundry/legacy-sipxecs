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

   /// constructor
   ProcessManager();

   /// destructor
   virtual ~ProcessManager();

   /// Locate a Process object by name.
   Process* findProcess(const UtlString& processName);

  protected:
   friend class ProcessManagerTest;
      
   /// Parse the XML of a process definition
   instantiateProcesses(const OsPath& processDefinitionDirectory);
   
  private:

   UtlHashMap mProcesses;
   
   // @cond INCLUDENOCOPY
   /// There is no copy constructor.
   ProcessManager(const ProcessManager& nocopyconstructor);

   /// There is no assignment operator.
   ProcessManager& operator=(const ProcessManager& noassignmentoperator);
   // @endcond     
};

#endif // _PROCESSMANAGER_H_
