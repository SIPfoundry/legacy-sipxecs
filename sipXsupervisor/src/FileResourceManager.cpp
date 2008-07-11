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

#include "FileResource.h"
#include "FileResourceManager.h"

// DEFINES
// CONSTANTS
// STATICS
// STATICS
OsBSem               FileResourceManager::sSingletonLock(OsBSem::Q_PRIORITY, OsBSem::FULL);
FileResourceManager* FileResourceManager::spSingleton;

// FORWARD DECLARATIONS

/// constructor
FileResourceManager::FileResourceManager() :
   mFileResourceTableLock(OsBSem::Q_PRIORITY, OsBSem::FULL)
{
   
}

FileResourceManager* FileResourceManager::getInstance()
{
   OsLock singletonMutex(sSingletonLock);

   if (!spSingleton)
   {
      spSingleton = new FileResourceManager();
   }
   
   return spSingleton;
}


/// Return an existing FileResource or NULL if no FileResource is found.
FileResource* FileResourceManager::find(const char* fileName /**< full path to the file */)
{
   OsLock tableMutex(mFileResourceTableLock);

   UtlString path(fileName);

   return dynamic_cast<FileResource*>(mFileResourceTable.find(&path));
}

/// Return an existing FileResource or NULL if no FileResource is found.
void FileResourceManager::save(FileResource* fileResource)
{
   OsLock tableMutex(mFileResourceTableLock);

   if (!mFileResourceTable.find(fileResource))
   {
      OsSysLog::add(FAC_WATCHDOG, PRI_INFO, "FileResourceManager::save "
                    "FileResource('%s')", fileResource->data());
      mFileResourceTable.insert(fileResource);
   }
   else
   {
      OsSysLog::add(FAC_WATCHDOG, PRI_CRIT, "FileResourceManager::save "
                    "duplicate FileResource('%s')", fileResource->data());
   }
}


/// destructor
FileResourceManager::~FileResourceManager()
{
   OsLock tableMutex(mFileResourceTableLock);

   OsSysLog::add(FAC_WATCHDOG, PRI_CRIT, "FileResourceManager::~ "
                 "delete %d FileResources", mFileResourceTable.entries());

   mFileResourceTable.destroyAll();
}
