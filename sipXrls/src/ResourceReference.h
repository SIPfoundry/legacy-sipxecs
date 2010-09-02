//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _ResourceReference_h_
#define _ResourceReference_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES

#include <utl/UtlContainableAtomic.h>
#include <utl/UtlString.h>
#include <utl/UtlRandom.h>
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
class ResourceList;
class ResourceCached;
class ContactSet;
class SubscriptionSet;
class ResourceInstance;


//! Container for a resource within a ResourceList.
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
class ResourceReference : public UtlContainableAtomic
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

   //! Construct a resource in a ResourceList.
   ResourceReference(ResourceList* resourceList,
                     const char* uri,
                     const char* nameXml,
                     const char* display_name);

   //! Destructor
   virtual ~ResourceReference();

   //! Get the parent ResourceList.
   ResourceList* getResourceList() const;
   //! Get the ancestor ResourceListSet.
   ResourceListSet* getResourceListSet() const;
   //! Get the ancestor ResourceListServer.
   ResourceListServer* getResourceListServer() const;

   //! Add to the HttpBody the current state of the resource.
   void generateBody(/// the RLMI XML to be appended to
                     UtlString& rlmi,
                     /// the HttpBody to which to add a part for this resource
                     HttpBody& body,
                     /// True if resource instances are to be consolidated.
                     UtlBoolean consolidated) const;

   //! Compare the DisplayName of the ResourceReference.
   int compareDisplayName(const char* newDisplayName);

   //! Compare the NameXML of the ResourceReference.
   //  This function will use formatNameXml on newNameXml
   //  in order to match the mNameXml.
   int compareNameXml(const char* newNameXml);

   //! Get the DisplayName of the ResourceReference.
   const UtlString getDisplayName() const;

   //! Get the NameXml of the ResourceReference.
   const UtlString getNameXml() const;

   //! Get the URI of the ResourceReference.
   const UtlString* getUri() const;

   //! Dump the object's internal state.
   void dumpState();

   /**
    * Get the ContainableType for a UtlContainable-derived class.
    */
   virtual UtlContainableType getContainableType() const;

   static const UtlContainableType TYPE;    /** < Class type used for runtime checking */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:

   // Callback functions to use with SipSubscribeClient::addSubscription.

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

   //! Formats the XML fragment containing the <name> elements
   //  for the resource.  It is to make sure the XML fragment
   //  ends with a CR-LF character if it did not end with a
   //  LF character to begin with.
   void formatNameXml(UtlString& nameXml);

   //! The containing ResourceList.
   ResourceList* mResourceList;

   //! The ResourceCached that implements this resource.
   ResourceCached* mResourceCached;

   //! The XML fragment containing the <name> elements for the resource.
   //  This may differ in different ResourceList's, so it is stored in
   //  the ResourceReference.
   UtlString mNameXml;
   /** The string to use as the display name (<local><identity display="...">)
    *  in consolidated event notices.
    */
   //  This may differ in different ResourceList's, so it is stored in
   //  the ResourceReference.
   UtlString mDisplayName;

   //! Disabled copy constructor
   ResourceReference(const ResourceReference& rResourceReference);

   //! Disabled assignment operator
   ResourceReference& operator=(const ResourceReference& rhs);

};

/* ============================ INLINE METHODS ============================ */

// Put #include's of ResourceList and ResourceCached down here to avoid circular
// include problems.
#include "ResourceList.h"
#include "ResourceCached.h"

//! Get the parent ResourceList.
inline ResourceList* ResourceReference::getResourceList() const
{
   return mResourceList;
}

// Get the ancestor ResourceListSet.
inline ResourceListSet* ResourceReference::getResourceListSet() const
{
   return mResourceList->getResourceListSet();
}

// Get the ancestor ResourceListServer.
inline ResourceListServer* ResourceReference::getResourceListServer() const
{
   return mResourceList->getResourceListServer();
}

// Get the DisplayName of the ResourceReference.
inline const UtlString ResourceReference::getDisplayName() const
{
   return mDisplayName;
}

// Get the NameXml of the ResourceReference.
inline const UtlString ResourceReference::getNameXml() const
{
   return mNameXml;
}


//! Get the URI of the ResourceReference.
inline const UtlString* ResourceReference::getUri() const
{
   return mResourceCached->getUri();
}

#endif  // _ResourceReference_h_
