// 
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsLock.h"
#include "os/OsSysLog.h"
#include "utl/UtlString.h"

#include "ProcessResource.h"
#include "ProcessResourceManager.h"

// DEFINES
// CONSTANTS
// STATICS
// STATICS
OsBSem               ProcessResourceManager::sSingletonLock(OsBSem::Q_PRIORITY, OsBSem::FULL);
ProcessResourceManager* ProcessResourceManager::spSingleton;

// FORWARD DECLARATIONS

/// constructor
ProcessResourceManager::ProcessResourceManager() :
   mProcessResourceTableLock(OsBSem::Q_PRIORITY, OsBSem::FULL)
{
   
}

ProcessResourceManager* ProcessResourceManager::getInstance()
{
   OsLock singletonMutex(sSingletonLock);

   if (!spSingleton)
   {
      spSingleton = new ProcessResourceManager();
   }
   
   return spSingleton;
}


/// Return an existing ProcessResource or NULL if no ProcessResource is found.
ProcessResource* ProcessResourceManager::find(const char* process /**< name of the process */)
{
   OsLock tableMutex(mProcessResourceTableLock);

   UtlString processName(process);

   return dynamic_cast<ProcessResource*>(mProcessResourceTable.find(&processName));
}

/// Return an existing ProcessResource or NULL if no ProcessResource is found.
void ProcessResourceManager::save(ProcessResource* processResource)
{
   OsLock tableMutex(mProcessResourceTableLock);

   if (!mProcessResourceTable.find(processResource))
   {
      OsSysLog::add(FAC_WATCHDOG, PRI_INFO, "ProcessResourceManager::save"
                    " ProcessResource('%s')", processResource->data());
      mProcessResourceTable.insert(processResource);
   }
   else
   {
      OsSysLog::add(FAC_WATCHDOG, PRI_CRIT, "ProcessResourceManager::save"
                    " duplicate ProcessResource('%s')", processResource->data());
   }
}


/// destructor
ProcessResourceManager::~ProcessResourceManager()
{
   OsLock tableMutex(mProcessResourceTableLock);

   OsSysLog::add(FAC_WATCHDOG, PRI_CRIT, "ProcessResourceManager::~ "
                 "delete %zu ProcessResources", mProcessResourceTable.entries());

   mProcessResourceTable.destroyAll();
}
