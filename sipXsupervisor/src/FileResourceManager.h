//
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef _FILERESOURCEMANAGER_H_
#define _FILERESOURCEMANAGER_H_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "utl/UtlHashBag.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS
class FileResource;

/// Owns all existing FileResource objects.
/**
 * This is a singleton class: use the FileResourceManager::getInstance method to get a
 * pointer to the one object.
 */
class FileResourceManager
{
  public:

   /// Singleton accessor
   static FileResourceManager* getInstance();

   /// Control whether or not a find request will check for directory matches
   typedef enum
   {
      CheckForDirectoryMatches,
      RequireExactFileMatch
   } FileMatchRule;

   /// Return an existing FileResource or NULL if no FileResource is found for 'fileName'.
   FileResource* find(const char* fileName, /**< full path to the file */
                      FileMatchRule fileMatchRule = CheckForDirectoryMatches
                      );

   /// Add the new fileResource to those available in the system.
   void save(FileResource* fileResource /**< new FileResource object */);

   /// destructor
   ~FileResourceManager();

  protected:

  private:

   /// constructor
   FileResourceManager();

   static OsBSem sSingletonLock;            ///< protects access to spSingleton
   static FileResourceManager* spSingleton; ///< pointer to the one FileResourceManager

   OsBSem     mFileResourceTableLock;
   UtlHashBag mFileResourceTable; ///< contains FileResource objects

   // @cond INCLUDENOCOPY
   /// There is no copy constructor.
   FileResourceManager(const FileResourceManager& nocopyconstructor);

   /// There is no assignment operator.
   FileResourceManager& operator=(const FileResourceManager& noassignmentoperator);
   // @endcond
};

#endif // _FILERESOURCEMANAGER_H_
