// 
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//////////////////////////////////////////////////////////////////////////////
#ifndef _PROCESSRESOURCEMANAGER_H_
#define _PROCESSRESOURCEMANAGER_H_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "utl/UtlHashBag.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS
class ProcessResource;

/// Owns all existing ProcessResource objects.
/**
 * This is a singleton class: use the ProcessResourceManager::getInstance method to get a
 * pointer to the one object.
 */
class ProcessResourceManager
{
  public:

   /// Singleton accessor
   static ProcessResourceManager* getInstance();
   
   /// Return an existing ProcessResource or NULL if no ProcessResource is found for 'process'.
   ProcessResource* find(const char* process /**< name of the process */);
   
   /// Add the new fileResource to those available in the system.
   void save(ProcessResource* processResource /**< new ProcessResource object */);
   
   /// destructor
   ~ProcessResourceManager();

  protected:

  private:

   /// constructor
   ProcessResourceManager();

   static OsBSem sSingletonLock;            ///< protects access to spSingleton
   static ProcessResourceManager* spSingleton; ///< pointer to the one ProcessResourceManager
   
   OsBSem     mProcessResourceTableLock;
   UtlHashBag mProcessResourceTable; ///< contains ProcessResource objects

   // @cond INCLUDENOCOPY
   /// There is no copy constructor.
   ProcessResourceManager(const ProcessResourceManager& nocopyconstructor);

   /// There is no assignment operator.
   ProcessResourceManager& operator=(const ProcessResourceManager& noassignmentoperator);
   // @endcond     
};

#endif // _PROCESSRESOURCEMANAGER_H_
