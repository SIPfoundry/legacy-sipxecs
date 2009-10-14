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
class SipxProcessResource;

/// Owns all existing SipxProcessResource objects.
/**
 * This is a singleton class: use the SipxProcessResourceManager::getInstance method to get a
 * pointer to the one object.
 */
class SipxProcessResourceManager
{
  public:

   /// Singleton accessor
   static SipxProcessResourceManager* getInstance();

   /// Return an existing SipxProcessResource or NULL if no SipxProcessResource is found for 'process'.
   SipxProcessResource* find(const char* process /**< name of the process */);

   /// Add the new fileResource to those available in the system.
   void save(SipxProcessResource* processResource /**< new SipxProcessResource object */);

   /// destructor
   ~SipxProcessResourceManager();

  protected:

  private:

   /// constructor
   SipxProcessResourceManager();

   static OsBSem sSingletonLock;            ///< protects access to spSingleton
   static SipxProcessResourceManager* spSingleton; ///< pointer to the one SipxProcessResourceManager

   OsBSem     mProcessResourceTableLock;
   UtlHashBag mProcessResourceTable; ///< contains SipxProcessResource objects

   // @cond INCLUDENOCOPY
   /// There is no copy constructor.
   SipxProcessResourceManager(const SipxProcessResourceManager& nocopyconstructor);

   /// There is no assignment operator.
   SipxProcessResourceManager& operator=(const SipxProcessResourceManager& noassignmentoperator);
   // @endcond
};

#endif // _PROCESSRESOURCEMANAGER_H_
