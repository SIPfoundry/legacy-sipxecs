//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _ResourceCached_h_
#define _ResourceCached_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES

#include "ResourceCache.h"
#include <utl/UtlContainableAtomic.h>
#include <utl/UtlString.h>
#include <utl/UtlRandom.h>
#include <utl/UtlHashBag.h>
#include <os/OsTimer.h>
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
class ContactSet;
class SubscriptionSet;
class ResourceInstance;


//! Container for the state of a resource URI, shared by all the ResourceLists which contain that URI.
/** The (unimplemented) Resource object carries the state information
 *  regarding attempting to get state information from the resource.
 *  The code can be rearranged to use various strategies for getting
 *  the status of the URI.  The current version uses one ContactSet
 *  object which attempts to get the "reg" event package for the URI.
 *  If contacts are obtained via the "reg" event package, the
 *  ContactSet creates a SubscriptionSet object for each contact to
 *  obtain status from it.  If no contacts can be obtained, the
 *  ContactSet creates one SubscriptionSet to SUBSCRIBE to the
 *  resource URI itself.
 *
 *  This implementation splits the implementation of the
 *  (unimplemented) Resource object into two parts:  1) a
 *  ResourceReference object which is owned by the ResourceList
 *  object, and contains the information specific to the appearance of
 *  the Resource in the ResourceList (such as the <name> information),
 *  and 2) a ResourceCached object which is owned by the ResourceCache
 *  object, and contains the all the state information.  Each
 *  ResourceReference points to the ResourceCached which implements
 *  its Resource, and each ResourceCached has a list of all the
 *  ResourceReferences which point to it.
 *
 *  This split allows ResourceList's to share subscriptions to their
 *  target URIs.
 */
// The URI for the resource is contained in the base UtlString.
class ResourceCached : public UtlString
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

   //! Construct a resource.
   ResourceCached(ResourceCache* resourceCache,
                  const char* uri);

   //! Destructor
   virtual ~ResourceCached();

   //! Get the parent ResourceCache.
   ResourceCache* getResourceCache() const;
   //! Get the ancestor ResourceListSet.
   ResourceListSet* getResourceListSet() const;
   //! Get the ancestor ResourceListServer.
   ResourceListServer* getResourceListServer() const;

   //! Add a ResourceReference to the set of references.
   void addReference(ResourceReference* resourceReference);

   //! Remove a ResourceReference from the set of references.
   void deleteReference(ResourceReference* resourceReference);

   //! Determine of there are any references in the set of references.
   UtlBoolean hasReferences();

   //! Declare that the contents have changed and need to be published.
   //  Start the publishing timer, if publishing has not been suppressed.
   void setToBePublished(/// If true, publish now rather than starting the timer.
                         UtlBoolean publishNow = FALSE,
                         /** The URI of the contained resource that has been
                          *  changed and should be published in any partial
                          *  RLMI.
                          */
                         const UtlString* chgUri = NULL);

   //! Add to the HttpBody the current state of the resource.
   void generateBody(/// the RLMI XML to be appended to
                     UtlString& rlmi,
                     /// the HttpBody to which to add a part for this resource
                     HttpBody& body,
                     /// True if resource instances are to be consolidated.
                     UtlBoolean consolidated,
                     /// The name XML for the resource.
                     const UtlString& nameXml,
                     /// The local display name (if consolidated = TRUE)
                     const UtlString& displayName) const;

   //! Remove dialogs in terminated state and terminated resource instances.
   void purgeTerminated();

   //! Get the URI of the resource.
   const UtlString* getUri() const;

   //! Dump the object's internal state.
   void dumpState() const;

   /**
    * Get the ContainableType for a UtlContainable-derived class.
    */
   virtual UtlContainableType getContainableType() const;

   static const UtlContainableType TYPE;    /** < Class type used for runtime checking */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:

   //! Start subscriptions for this resource.
   void startSubscriptions();

   //! Terminate any existing subscriptions for this resource.
   void terminateSubscriptions();

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

   //! The containing ResourceCache.
   ResourceCache* mResourceCache;

   // Note that all name information is carried in the
   // ResourceReference's that point to this ResourceCache, as they
   // may differ in different ResourceLists.

   //! A UtlHashBag of the ResourceReference's that point to this ResourceCached.
   UtlHashBag mReferences;

   /** The dependent ContactSet object, which finds and handles the set
    *  of contacts for the resource.
    */
   ContactSet* mContactSetP;

   //! The sequence number to identify this resource in callback messages.
   int mSeqNo;

   //! Disabled copy constructor
   ResourceCached(const ResourceCached& rResourceCached);

   //! Disabled assignment operator
   ResourceCached& operator=(const ResourceCached& rhs);

};

/* ============================ INLINE METHODS ============================ */

// Put #include's of ResourceListSet and ResourceReference down here
// to avoid circular include problems.
#include "ResourceListSet.h"
#include "ResourceReference.h"

// Get the parent ResourceCache.
inline ResourceCache* ResourceCached::getResourceCache() const
{
   return mResourceCache;
}

// Get the ancestor ResourceListSet.
inline ResourceListSet* ResourceCached::getResourceListSet() const
{
   return mResourceCache->getResourceListSet();
}

// Get the ancestor ResourceListServer.
inline ResourceListServer* ResourceCached::getResourceListServer() const
{
   return mResourceCache->getResourceListServer();
}

// Get the URI of the resource.
inline const UtlString* ResourceCached::getUri() const
{
   return static_cast <const UtlString*> (this);
}

#endif  // _ResourceCached_h_
