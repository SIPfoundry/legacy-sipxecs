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

#include "SqldbResource.h"
#include "SqldbResourceManager.h"

// DEFINES
// CONSTANTS
// STATICS
// STATICS
OsBSem               SqldbResourceManager::sSingletonLock(OsBSem::Q_PRIORITY, OsBSem::FULL);
SqldbResourceManager* SqldbResourceManager::spSingleton;

// FORWARD DECLARATIONS

/// constructor
SqldbResourceManager::SqldbResourceManager() :
   mSqldbResourceTableLock(OsBSem::Q_PRIORITY, OsBSem::FULL)
{

}

SqldbResourceManager* SqldbResourceManager::getInstance()
{
   OsLock singletonMutex(sSingletonLock);

   if (!spSingleton)
   {
      spSingleton = new SqldbResourceManager();
   }

   return spSingleton;
}


/// Return an existing SqldbResource or NULL if no SqldbResource is found.
SqldbResource* SqldbResourceManager::find(const char* sqldb /**< name of the sqldb */)
{
   OsLock tableMutex(mSqldbResourceTableLock);

   UtlString sqldbName(sqldb);

   return dynamic_cast<SqldbResource*>(mSqldbResourceTable.find(&sqldbName));
}

/// Return an existing SqldbResource or NULL if no SqldbResource is found.
void SqldbResourceManager::save(SqldbResource* sqldbResource)
{
   OsLock tableMutex(mSqldbResourceTableLock);

   if (!mSqldbResourceTable.find(sqldbResource))
   {
      OsSysLog::add(FAC_SUPERVISOR, PRI_INFO, "SqldbResourceManager::save "
                    "SqldbResource('%s')", sqldbResource->data());
      mSqldbResourceTable.insert(sqldbResource);
   }
   else
   {
      OsSysLog::add(FAC_SUPERVISOR, PRI_CRIT, "SqldbResourceManager::save "
                    "duplicate SqldbResource('%s')", sqldbResource->data());
   }
}


/// destructor
SqldbResourceManager::~SqldbResourceManager()
{
   OsLock tableMutex(mSqldbResourceTableLock);

   OsSysLog::add(FAC_SUPERVISOR, PRI_CRIT, "SqldbResourceManager::~ "
                 "delete %zu SqldbResources", mSqldbResourceTable.entries());

   mSqldbResourceTable.destroyAll();
}
