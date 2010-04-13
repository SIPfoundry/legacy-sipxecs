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
#include "os/OsFS.h"
#include "utl/UtlString.h"
#include "utl/UtlSListIterator.h"

#include "DirectoryResource.h"
#include "DirectoryResourceManager.h"

// DEFINES
// CONSTANTS
// STATICS
// STATICS
OsBSem               DirectoryResourceManager::sSingletonLock(OsBSem::Q_PRIORITY, OsBSem::FULL);
DirectoryResourceManager* DirectoryResourceManager::spSingleton;

// FORWARD DECLARATIONS

/// constructor
DirectoryResourceManager::DirectoryResourceManager() :
   mDirectoryResourceTableLock(OsBSem::Q_PRIORITY, OsBSem::FULL)
{

}

DirectoryResourceManager* DirectoryResourceManager::getInstance()
{
   OsLock singletonMutex(sSingletonLock);

   if (!spSingleton)
   {
      spSingleton = new DirectoryResourceManager();
   }

   return spSingleton;
}

DirectoryResource*
DirectoryResourceManager::findFilename(const char* fullName ///< full path to a file
                                       )
{
   OsPath fullFileName(fullName);
   UtlString dirName  = fullFileName.getDirName();
   UtlString fileName;

   fileName.append(fullName, dirName.length(), UtlString::UTLSTRING_TO_END);
   dirName.strip(UtlString::trailing, OsPath::separator[0]);

   DirectoryResource* found = NULL;
   
   OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG,
                 "DirectoryResourceManager::findFilename: path '%s' file '%s'",
                 dirName.data(), fileName.data());

   {
      OsLock tableMutex(mDirectoryResourceTableLock);
      UtlSListIterator directories(mDirectoryResourceTable);
      DirectoryResource* dir;
   
      while (!found
             && (dir = dynamic_cast<DirectoryResource*>(directories.findNext(&dirName))))
      {
         if ( dir->matches(fileName) )
         {
            found = dir;
         }
      }
   }

   if (found)
   {
      UtlString matched;
      found->appendDescription(matched);
      OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG,
                    "DirectoryResourceManager::findFilename: '%s' matches %s",
                    fullName, matched.data());
   }
   else
   {
      OsSysLog::add(FAC_SUPERVISOR, PRI_WARNING,
                    "DirectoryResourceManager::findFilename: no match found for '%s'",
                    fullName);
   }
   
   return found;
}

DirectoryResource*
DirectoryResourceManager::find(const UtlString& path,    ///< path of the directory
                               const UtlString& pattern  ///< pattern within directory
                               )
{
   DirectoryResource* found = NULL;
   
   {
      OsLock tableMutex(mDirectoryResourceTableLock);
      UtlSListIterator directories(mDirectoryResourceTable);
      DirectoryResource* dir;
   
      while (!found
             && (dir = dynamic_cast<DirectoryResource*>(directories.findNext(&path))))
      {
         if ( dir->isFilePattern(pattern) )
         {
            found = dir;
         }
      }
   }

   return found;
}

/// Return an existing DirectoryResource or NULL if no DirectoryResource is found.
void DirectoryResourceManager::save(DirectoryResource* directoryResource)
{
   OsLock tableMutex(mDirectoryResourceTableLock);

   mDirectoryResourceTable.insert(directoryResource);
}


/// destructor
DirectoryResourceManager::~DirectoryResourceManager()
{
   OsLock tableMutex(mDirectoryResourceTableLock);

   OsSysLog::add(FAC_SUPERVISOR, PRI_CRIT, "DirectoryResourceManager::~ "
                 "delete %zu DirectoryResources", mDirectoryResourceTable.entries());

   mDirectoryResourceTable.destroyAll();
}
