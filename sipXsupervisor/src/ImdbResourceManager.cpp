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

#include "ImdbResource.h"
#include "ImdbResourceManager.h"

// DEFINES
// CONSTANTS
// STATICS
// STATICS
OsBSem               ImdbResourceManager::sSingletonLock(OsBSem::Q_PRIORITY, OsBSem::FULL);
ImdbResourceManager* ImdbResourceManager::spSingleton;

// FORWARD DECLARATIONS

/// constructor
ImdbResourceManager::ImdbResourceManager() :
   mImdbResourceTableLock(OsBSem::Q_PRIORITY, OsBSem::FULL)
{

}

ImdbResourceManager* ImdbResourceManager::getInstance()
{
   OsLock singletonMutex(sSingletonLock);

   if (!spSingleton)
   {
      spSingleton = new ImdbResourceManager();
   }

   return spSingleton;
}


/// Return an existing ImdbResource or NULL if no ImdbResource is found.
ImdbResource* ImdbResourceManager::find(const char* imdb /**< name of the imdb */)
{
   OsLock tableMutex(mImdbResourceTableLock);

   UtlString imdbName(imdb);

   return dynamic_cast<ImdbResource*>(mImdbResourceTable.find(&imdbName));
}

/// Return an existing ImdbResource or NULL if no ImdbResource is found.
void ImdbResourceManager::save(ImdbResource* imdbResource)
{
   OsLock tableMutex(mImdbResourceTableLock);

   if (!mImdbResourceTable.find(imdbResource))
   {
      OsSysLog::add(FAC_SUPERVISOR, PRI_INFO, "ImdbResourceManager::save "
                    "ImdbResource('%s')", imdbResource->data());
      mImdbResourceTable.insert(imdbResource);
   }
   else
   {
      OsSysLog::add(FAC_SUPERVISOR, PRI_CRIT, "ImdbResourceManager::save "
                    "duplicate ImdbResource('%s')", imdbResource->data());
   }
}


/// destructor
ImdbResourceManager::~ImdbResourceManager()
{
   OsLock tableMutex(mImdbResourceTableLock);

   OsSysLog::add(FAC_SUPERVISOR, PRI_CRIT, "ImdbResourceManager::~ "
                 "delete %zu ImdbResources", mImdbResourceTable.entries());

   mImdbResourceTable.destroyAll();
}
