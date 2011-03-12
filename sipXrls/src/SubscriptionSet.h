//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _SubscriptionSet_h_
#define _SubscriptionSet_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES

#include "ResourceSubscriptionReceiver.h"
#include <utl/UtlContainableAtomic.h>
#include <utl/UtlString.h>
#include <utl/UtlSList.h>
#include <net/SipSubscribeClient.h>
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
class ResourceInstance;

//! Container for a set of subscriptions created by a SUBSCRIBE request.
/** The SubscriptionSet does not have an independent life beyond the
 *  ResourceListSet that contains it.  Access to it is locked by the
 *  ResourceListSet containing it.
 *  When the SubscriptionSet is created, it generates the SUBSCRIBE request.
 *  It also receives the subscription-state callbacks, and creates/deletes
 *  ResourceInstance children as necessary.
 *  A SubscriptionSet is a child of a Resource (if the URI is "provisioned"),
 *  or of a ContactSet (if the Resource's URI is "registered").
 *  The constructor/destructor update the mSubscribeMap of the ResourceListSet
 *  so this SubscribeSet can always be found by its early dialog handle.
 */
class SubscriptionSet : public UtlContainableAtomic,
                        public ResourceSubscriptionReceiver
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

   //! Construct a subscription set.
   SubscriptionSet(ResourceCached* resource,
                   ///< ancestor ResourceCached object
                   UtlString& uri
                   ///< the URI to subscribe to
      );

   //! Destructor
   virtual ~SubscriptionSet();

   //! Get the parent ResourceCached.
   ResourceCached* getResourceCached() const;
   //! Get the ancestor ResourceListSet.
   ResourceListSet* getResourceListSet() const;
   //! Get the ancestor ResourceListServer.
   ResourceListServer* getResourceListServer() const;

   //! Get the subscribed URI.
   const UtlString* getUri() const;

   /** Handle a subscription state callback for the subscription
    *  handled by this SubscriptionSet.
    *  Overrides ResourceSubscriptionReceiver::subscriptionEventCallback.
    */
   virtual void subscriptionEventCallback(
      const UtlString* earlyDialogHandle,
      const UtlString* dialogHandle,
      SipSubscribeClient::SubscriptionState newState,
      const UtlString* subscriptionState
      );

   //! Insert a subscription into the set.
   //  Also adds the dialog handle to mHandleMap of the ResourceListSet and
   //  publishes the content.
   void addInstance(const char* instanceName,
                    const char* subscriptionState);

   //! Find a subscription in the set.
   //  Returns the ResourceInstance or NULL if not found.
   ResourceInstance* getInstance(const char* instanceName);

   //! Delete a subscription from the set.
   //  Also removes the dialog handle from mHandleMap of the ResourceListSet
   //  and publishes the termination state.
   void deleteInstance(const char* instanceName,
                       const char* subscriptionState,
                       const char* resourceSubscriptionState);

   //! Remove dialogs in terminated state and terminated resource instances.
   void purgeTerminated();

   //! Add to the HttpBody the current state of the resource instances.
   void generateBody(/// the RLMI XML to be appended to
                     UtlString& rlmi,
                     /// the HttpBody to which to add a part for this resource
                     HttpBody& body,
                     /// True if resource instances are to be consolidated.
                     UtlBoolean consolidated,
                     /// The local display name (if consolidated = TRUE)
                     const UtlString& displayName) const;

   //! Dump the object's internal state.
   void dumpState() const;

   /**
    * Get the ContainableType for a UtlContainable-derived class.
    */
   virtual UtlContainableType getContainableType() const;

   static const UtlContainableType TYPE;    /** < Class type used for runtime checking */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

   //! The ancestor ResourceCached.
   ResourceCached* mResource;

   //! The URI to subscribe to.
   UtlString mUri;

   //! The early dialog handle for the subscription.
   UtlString mSubscriptionEarlyDialogHandle;

   /** The set of subscriptions established by the SUBSCRIBE, as a UtlSList
    *  of ResourceInstance's.
    */
   UtlSList mSubscriptions;

   //! Disabled copy constructor
   SubscriptionSet(const SubscriptionSet& rSubscriptionSet);

   //! Disabled assignment operator
   SubscriptionSet& operator=(const SubscriptionSet& rhs);

};

/* ============================ INLINE METHODS ============================ */

// Put #include of ResourceCached down here to avoid circular
// include problems.
#include "ResourceCached.h"

// Get the parent ResourceCached.
inline ResourceCached* SubscriptionSet::getResourceCached() const
{
   return mResource;
}

// Get the ancestor ResourceListSet.
inline ResourceListSet* SubscriptionSet::getResourceListSet() const
{
   return mResource->getResourceListSet();
}

// Get the ancestor ResourceListServer.
inline ResourceListServer* SubscriptionSet::getResourceListServer() const
{
   return mResource->getResourceListServer();
}

// Get the subscribed URI.
inline const UtlString* SubscriptionSet::getUri() const
{
   return &mUri;
}

#endif  // _SubscriptionSet_h_
