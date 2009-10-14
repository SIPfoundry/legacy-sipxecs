//
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef _SQLDBRESOURCEMANAGER_H_
#define _SQLDBRESOURCEMANAGER_H_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "utl/UtlHashBag.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SqldbResource;

/// Owns all existing SqldbResource objects.
/**
 * This is a singleton class: use the SqldbResourceManager::getInstance method to get a
 * pointer to the one object.
 */
class SqldbResourceManager
{
  public:

   /// Singleton accessor
   static SqldbResourceManager* getInstance();

   /// Return an existing SqldbResource or NULL if no SqldbResource is found for 'sqldb'.
   SqldbResource* find(const char* sqldb /**< name of the sqldb */);

   /// Add the new fileResource to those available in the system.
   void save(SqldbResource* sqldbResource /**< new SqldbResource object */);

   /// destructor
   ~SqldbResourceManager();

  protected:

  private:

   /// constructor
   SqldbResourceManager();

   static OsBSem sSingletonLock;            ///< protects access to spSingleton
   static SqldbResourceManager* spSingleton; ///< pointer to the one SqldbResourceManager

   OsBSem     mSqldbResourceTableLock;
   UtlHashBag mSqldbResourceTable; ///< contains SqldbResource objects

   // @cond INCLUDENOCOPY
   /// There is no copy constructor.
   SqldbResourceManager(const SqldbResourceManager& nocopyconstructor);

   /// There is no assignment operator.
   SqldbResourceManager& operator=(const SqldbResourceManager& noassignmentoperator);
   // @endcond
};

#endif // _SQLDBRESOURCEMANAGER_H_
