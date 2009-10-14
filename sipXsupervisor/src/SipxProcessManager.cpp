//
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////


// INCLUDES
#include "os/OsFS.h"
#include "os/OsSysLog.h"
#include "utl/UtlHashBagIterator.h"
#include "xmlparser/tinyxml.h"

#include "SipxProcess.h"
#include "SipxSupervisorProcess.h"
#include "SipxProcessManager.h"

// DEFINES
// CONSTANTS
const char* PROCESS_DEFINITION_NAME_PATTERN = ".*-process.xml$";

// STATICS
OsBSem          SipxProcessManager::sSingletonLock(OsBSem::Q_PRIORITY, OsBSem::FULL);
SipxProcessManager* SipxProcessManager::spSingleton;

// FORWARD DECLARATIONS

/// Singleton accessor
SipxProcessManager* SipxProcessManager::getInstance()
{
   OsLock mutex(sSingletonLock);

   if (!spSingleton)
   {
      spSingleton = new SipxProcessManager();
   }
   return spSingleton;
}


/// constructor
SipxProcessManager::SipxProcessManager() :
   mProcessTableLock(OsBSem::Q_PRIORITY, OsBSem::FULL)
{
};

/// Save a process definition.
void SipxProcessManager::save(SipxProcess* process)
{
   // called from within SipxProcess::createFromDefinition
   OsLock mutex(mProcessTableLock);

   OsSysLog::add(FAC_SUPERVISOR, PRI_NOTICE, "SipxProcessManager::save "
                 " SipxProcess '%s'", process->data());
   mProcesses.insert(process);
}



/// Locate a SipxProcess object by name.
SipxProcess* SipxProcessManager::findProcess(const UtlString& processName)
{
   OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG,"SipxProcessManager::findProcess "
                 "searching for '%s'", processName.data()
                 );

   OsLock tableMutex(mProcessTableLock);

   return dynamic_cast<SipxProcess*>(mProcesses.find(&processName));
}

/// Find process definitions and instantiate SipxProcess objects for each.
void SipxProcessManager::instantiateProcesses(const OsPath& processDefinitionDirectory)
{
   OsFileIterator definitions(processDefinitionDirectory);
   OsPath    processDefinitionFile;
   OsStatus  iteratorStatus;

   OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG,"SipxProcessManager::instantiateProcesses searching %s",
                 processDefinitionDirectory.data()
                 );

   for ( iteratorStatus = definitions.findFirst(processDefinitionFile,
                                                PROCESS_DEFINITION_NAME_PATTERN,
                                                OsFileIterator::FILES);
         OS_SUCCESS == iteratorStatus;
         iteratorStatus = definitions.findNext(processDefinitionFile)
        )
   {
      OsPath processDefinitionPath( processDefinitionDirectory
                                   +OsPath::separator
                                   +processDefinitionFile
                                   );
      OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG,"SipxProcessManager::instantiateProcesses reading %s",
                    processDefinitionPath.data()
                    );

      SipxProcess::createFromDefinition(processDefinitionPath);
   }
}

// Fill in a map of process names and states (as UtlStrings)
void SipxProcessManager::getProcessStateAll(UtlHashMap& processStates //< key->name, value->state string
                         )
{
   processStates.destroyAll();
   SipxProcess* process;

   // the lock is not required with the Iterator
   UtlHashBagIterator processes(mProcesses);
   while ((process = dynamic_cast<SipxProcess*>(processes())))
   {
      if ( 0 != process->compareTo(SUPERVISOR_PROCESS_NAME) )
      {
         processStates.insertKeyAndValue(new UtlString(process->data()),
                                      new UtlString(process->GetCurrentState()->name())
                                      );
      }
   }
}

void SipxProcessManager::shutdown()
{
   if (spSingleton)
   {
      delete spSingleton;
      spSingleton = NULL;
   }
}

/// destructor
SipxProcessManager::~SipxProcessManager()
{
   OsLock tableMutex(mProcessTableLock);

   SipxProcess* process;
   // send shutdowns to all processes before waiting for them to delete
   UtlHashBagIterator processes(mProcesses);
   while ((process = dynamic_cast<SipxProcess*>(processes())))
   {
      process->shutdown();
      OsTask::delay(100);
   }

   OsSysLog::add(FAC_SUPERVISOR, PRI_NOTICE, "SipxProcessManager::~ "
                 "delete %zu SipxProcess objects", mProcesses.entries());

   mProcesses.destroyAll();

   while (mProcesses.entries() > 0)
   {
      OsTask::delay(1000);
   }
};
