//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _ContactSet_h_
#define _ContactSet_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES

#include "ResourceSubscriptionReceiver.h"
#include "ResourceNotifyReceiver.h"
#include "ResourceCached.h"
#include <utl/UtlContainableAtomic.h>
#include <utl/UtlString.h>
#include <utl/UtlHashBag.h>
#include <utl/UtlHashMap.h>
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

class ResourceCached;
class ResourceInstance;


//! Container for a set of subscriptions to the contacts of an AOR.
/** The ContactSet does not have an independent life beyond the
 *  ResourceListSet that contains it.  Access to it is locked by the
 *  ResourceListSet containing it.
 *  The ContactSet is created and directed to a URI which is presumed to be
 *  an Address Of Record.  It sends a SUBSCRIBE for "reg" (registration)
 *  events to the AOR.  Any NOTIFYs it receives are used to update a list
 *  of contact addresses for the AOR.  For each contact address, a
 *  SubscriptionList is maintained.
 *  A ContactSet is a child of a Resource.
 *  The constructor/destructor update the mSubscribeMap and mNotifyMap
 *  of the ResourceListSet so it can process callbacks for its "reg"
 *  subscription.
 */
class ContactSet : public UtlContainableAtomic,
                   public ResourceSubscriptionReceiver,
                   public ResourceNotifyReceiver
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

   //! Construct a subscription set.
   ContactSet(ResourceCached* resource,
              ///< ancestor Resource object
              UtlString& uri
              ///< the AOR
      );

   //! Destructor
   virtual ~ContactSet();

   //! Get the parent ResourceCached.
   ResourceCached* getResourceCached() const;
   //! Get the ancestor ResourceListSet.
   ResourceListSet* getResourceListSet() const;
   //! Get the ancestor ResourceListServer.
   ResourceListServer* getResourceListServer() const;

   /** Handle a subscription state callback for the subscription
    *  handled by this ContactSet.
    */
   virtual void subscriptionEventCallback(
      const UtlString* earlyDialogHandle,
      const UtlString* dialogHandle,
      SipSubscribeClient::SubscriptionState newState,
      const UtlString* subscriptionState
      );

   //! Process a notify event callback.
   virtual void notifyEventCallback(const UtlString* dialogHandle,
                                    const UtlString* content);

   //! Add to an HttpBody the current state of the resource instances.
   void generateBody(/// the RLMI XML to be appended to
                     UtlString& rlmi,
                     /// the HttpBody to which to add a part for this resource
                     HttpBody& body,
                     /// True if resource instances are to be consolidated.
                     UtlBoolean consolidated,
                     /// The local display name (if consolidated = TRUE)
                     const UtlString& displayName) const;

   //! Remove dialogs in terminated state and terminated resource instances.
   void purgeTerminated();

   //! Dump the object's internal state.
   void dumpState();

   /**
    * Get the ContainableType for a UtlContainable-derived class.
    */
   virtual UtlContainableType getContainableType() const;

   static const UtlContainableType TYPE;    /** < Class type used for runtime checking */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:

   //! Update the subscriptions we maintain to agree with mSubscriptions.
   void updateSubscriptions();

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

   //! The parent ResourceCached.
   ResourceCached* mResource;

   //! The AOR.
   UtlString mUri;

   //! The early dialog handle for the subscription.
   UtlString mSubscriptionEarlyDialogHandle;

   /** The set of subscriptions established by the "reg" SUBSCRIBE.
    *  Each subscription is represented by one entry whose key is the
    *  dialog handle and whose value is a UtlHashMap.
    *  The UtlHashMap's carry the most recent state returned by that
    *  subscription:  The keys are the id attributes of the <contact>
    *  elements and the values are "call-id;uri", with call-id being
    *  the value of the call-id attribute and uri being the content of
    *  the <uri> element.
    */
   UtlHashMap mSubscriptions;

   //! The set of SubscriptionSet's for each contact of this URI.
   /** This hash map is indexed by strings "call-id;URI", where:
    * call-id is the Call-Id of the registration pseudo-dialog for the
    * contact as reported in the <uri> element of the <contact>
    * element in the reg event, and
    * URI is the contact URI as reported in the <uri> element.
    * This indexing allows us to detect when a phone reboots and
    * re-registers the same contact URI with a different REGISTER
    * Call-Id.
    */
   UtlHashMap mSubscriptionSets;

   //! Disabled copy constructor
   ContactSet(const ContactSet& rContactSet);

   //! Disabled assignment operator
   ContactSet& operator=(const ContactSet& rhs);

};

// Get the parent ResourceCached.
inline ResourceCached* ContactSet::getResourceCached() const
{
   return mResource;
}

// Get the ancestor ResourceListSet.
inline ResourceListSet* ContactSet::getResourceListSet() const
{
   return mResource->getResourceListSet();
}

// Get the ancestor ResourceListServer.
inline ResourceListServer* ContactSet::getResourceListServer() const
{
   return mResource->getResourceListServer();
}

#endif  // _ContactSet_h_
