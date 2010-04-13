//
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef _DIRECTORYRESOURCEMANAGER_H_
#define _DIRECTORYRESOURCEMANAGER_H_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "utl/UtlHashBag.h"
#include "DirectoryResource.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Owns all existing DirectoryResource objects.
/**
 * This is a singleton class: use the DirectoryResourceManager::getInstance method to get a
 * pointer to the one object.
 */
class DirectoryResourceManager
{
  public:

   /// Singleton accessor
   static DirectoryResourceManager* getInstance();

   /// Return an existing DirectoryResource or NULL if no DirectoryResource is found for 'fileName'.
   DirectoryResource* findFilename(const char* fullName /**< full path to the file */);

   /// Locate an existing DirectoryResource based on directory path and pattern
   DirectoryResource* find(const UtlString& path,    ///< path of the directory
                           const UtlString& pattern  ///< pattern within directory
                           );
   
   /// Add the new fileResource to those available in the system.
   void save(DirectoryResource* directoryResource /**< new DirectoryResource object */);

   /// destructor
   ~DirectoryResourceManager();

  protected:

  private:

   /// constructor
   DirectoryResourceManager();

   static OsBSem sSingletonLock;            ///< protects access to spSingleton
   static DirectoryResourceManager* spSingleton; ///< pointer to the one DirectoryResourceManager

   OsBSem   mDirectoryResourceTableLock;
   UtlSList mDirectoryResourceTable; ///< contains DirectoryResource objects

   // @cond INCLUDENOCOPY
   /// There is no copy constructor.
   DirectoryResourceManager(const DirectoryResourceManager& nocopyconstructor);

   /// There is no assignment operator.
   DirectoryResourceManager& operator=(const DirectoryResourceManager& noassignmentoperator);
   // @endcond
};

#endif // _DIRECTORYRESOURCEMANAGER_H_
