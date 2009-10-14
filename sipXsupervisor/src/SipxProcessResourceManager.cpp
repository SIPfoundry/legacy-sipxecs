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

#include "SipxProcessResource.h"
#include "SipxProcessResourceManager.h"

// DEFINES
// CONSTANTS
// STATICS
// STATICS
OsBSem               SipxProcessResourceManager::sSingletonLock(OsBSem::Q_PRIORITY, OsBSem::FULL);
SipxProcessResourceManager* SipxProcessResourceManager::spSingleton;

// FORWARD DECLARATIONS

/// constructor
SipxProcessResourceManager::SipxProcessResourceManager() :
   mProcessResourceTableLock(OsBSem::Q_PRIORITY, OsBSem::FULL)
{

}

SipxProcessResourceManager* SipxProcessResourceManager::getInstance()
{
   OsLock singletonMutex(sSingletonLock);

   if (!spSingleton)
   {
      spSingleton = new SipxProcessResourceManager();
   }

   return spSingleton;
}


/// Return an existing SipxProcessResource or NULL if no SipxProcessResource is found.
SipxProcessResource* SipxProcessResourceManager::find(const char* process /**< name of the process */)
{
   OsLock tableMutex(mProcessResourceTableLock);

   UtlString processName(process);

   return dynamic_cast<SipxProcessResource*>(mProcessResourceTable.find(&processName));
}

/// Return an existing SipxProcessResource or NULL if no SipxProcessResource is found.
void SipxProcessResourceManager::save(SipxProcessResource* processResource)
{
   OsLock tableMutex(mProcessResourceTableLock);

   if (!mProcessResourceTable.find(processResource))
   {
      OsSysLog::add(FAC_SUPERVISOR, PRI_INFO, "SipxProcessResourceManager::save"
                    " SipxProcessResource('%s')", processResource->data());
      mProcessResourceTable.insert(processResource);
   }
   else
   {
      OsSysLog::add(FAC_SUPERVISOR, PRI_CRIT, "SipxProcessResourceManager::save"
                    " duplicate SipxProcessResource('%s')", processResource->data());
   }
}


/// destructor
SipxProcessResourceManager::~SipxProcessResourceManager()
{
   OsLock tableMutex(mProcessResourceTableLock);

   OsSysLog::add(FAC_SUPERVISOR, PRI_CRIT, "SipxProcessResourceManager::~ "
                 "delete %zu SipxProcessResources", mProcessResourceTable.entries());

   mProcessResourceTable.destroyAll();
}
