// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _ResourceList_h_
#define _ResourceList_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES

#include <utl/UtlContainableAtomic.h>
#include <utl/UtlString.h>
#include <utl/UtlSList.h>
#include <net/HttpBody.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

class ResourceListServer;
class ResourceListSet;


/**
 * This class maintains information about a resource list that is needed to
 * generate resource list events, as described in RFC 4662.
 * It lives within a ResourceListSet and has no independent life.
 */

class ResourceList : public UtlContainableAtomic
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

   //! Construct a resource list.
   ResourceList(/// The parent ResourceListSet.
                ResourceListSet* resourceListSet,
                /// The user-part of the resource list URI.
                const char* userPart);

   virtual ~ResourceList();

   //! Get the parent ResourceListSet.
   ResourceListSet* getResourceListSet() const;
   //! Get the ancestor ResourceListServer.
   ResourceListServer* getResourceListServer() const;

   //! Refresh the subscriptions of all resources.
   void refreshAllResources();

   //! Delete all Resource's in the resource list.
   void deleteAllResources();

   //! Create and add a resource to the resource list.
   void addResource(const char* uri,
                    const char* nameXml,
                    const char* display_name);

   //! Declare that the contents have changed and need to be published.
   //  Does not start the publishing timer.
   void setToBePublished();

   //! Publish the contents if necessary and clear the publishing indicator.
   void publishIfNecessary();

   //! Get the user-part for this resource list.
   const UtlString* getUserPart() const;

   //! Get the resource list URI.
   const UtlString* getResourceListUri();

   //! Get the resource list consolidated URI.
   const UtlString* getResourceListUriCons();

   /**
    * Get the ContainableType for a UtlContainable-derived class.
    */
   virtual UtlContainableType getContainableType() const;

   static const UtlContainableType TYPE;    /** < Class type used for runtime checking */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:
   
   //! Generate and publish the content for the resource list.
   void publish();

   //! Generate the HttpBody for the current state of the resource list.
   //  The caller owns the returned HttpBody.
   HttpBody* generateBody(// True if resource instances are to be consolidated.
                          UtlBoolean consolidated) const;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

   //! The userPart for the resource list.
   UtlString mUserPart;

   //! Resource list name, the URI of the resource list showing the sipX domain
   UtlString mResourceListName;
   //! Resource list URI, the URI of the SUBSCRIBE as it reaches us.
   UtlString mResourceListUri;

   // These values are for the "consolidated" resource list, which conflates
   // all resource instances for a resource into one resource instance.
   //! Resource list name, the URI of the resource list showing the sipX domain
   UtlString mResourceListNameCons;
   //! Resource list URI, the URI of the SUBSCRIBE as it reaches us.
   UtlString mResourceListUriCons;

   //! version number
   mutable int mVersion;

   //! The containing ResourceListSet.
   ResourceListSet* mResourceListSet;

   //! List of contained ResourceReference's.
   UtlSList mResources;

   /** True if there has been a change to the ResourceList's content
    *  that has not yet been published.
    */
   UtlBoolean mChangesToPublish;

   //! Disabled copy constructor
   ResourceList(const ResourceList& rResourceList);

   //! Disabled assignment operator
   ResourceList& operator=(const ResourceList& rhs);

};

/* ============================ INLINE METHODS ============================ */

// Put #include of ResourceListSet down here to avoid circular
// include problems.
#include "ResourceListSet.h"

// Get the parent ResourceListSet.
inline ResourceListSet* ResourceList::getResourceListSet() const
{
   return mResourceListSet;
}

// Get the ancestor ResourceListServer.
inline ResourceListServer* ResourceList::getResourceListServer() const
{
   return mResourceListSet->getResourceListServer();
}

// Get the user-part for this resource list.
inline const UtlString* ResourceList::getUserPart() const
{
   return &mUserPart;
}

// Get the resource list URI.
inline const UtlString* ResourceList::getResourceListUri()
{
   return &mResourceListUri;
}

// Get the resource list consolidated URI.
inline const UtlString* ResourceList::getResourceListUriCons()
{
   return &mResourceListUriCons;
}

#endif  // _ResourceList_h_
