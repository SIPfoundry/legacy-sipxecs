//
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef _IMDBRESOURCEMANAGER_H_
#define _IMDBRESOURCEMANAGER_H_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "utl/UtlHashBag.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS
class ImdbResource;

/// Owns all existing ImdbResource objects.
/**
 * This is a singleton class: use the ImdbResourceManager::getInstance method to get a
 * pointer to the one object.
 */
class ImdbResourceManager
{
  public:

   /// Singleton accessor
   static ImdbResourceManager* getInstance();

   /// Return an existing ImdbResource or NULL if no ImdbResource is found for 'imdb'.
   ImdbResource* find(const char* imdb /**< name of the imdb */);

   /// Add the new fileResource to those available in the system.
   void save(ImdbResource* imdbResource /**< new ImdbResource object */);

   /// destructor
   ~ImdbResourceManager();

  protected:

  private:

   /// constructor
   ImdbResourceManager();

   static OsBSem sSingletonLock;            ///< protects access to spSingleton
   static ImdbResourceManager* spSingleton; ///< pointer to the one ImdbResourceManager

   OsBSem     mImdbResourceTableLock;
   UtlHashBag mImdbResourceTable; ///< contains ImdbResource objects

   // @cond INCLUDENOCOPY
   /// There is no copy constructor.
   ImdbResourceManager(const ImdbResourceManager& nocopyconstructor);

   /// There is no assignment operator.
   ImdbResourceManager& operator=(const ImdbResourceManager& noassignmentoperator);
   // @endcond
};

#endif // _IMDBRESOURCEMANAGER_H_
