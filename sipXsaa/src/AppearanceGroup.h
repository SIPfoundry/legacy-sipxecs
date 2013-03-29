//
// Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _AppearanceGroup_h_
#define _AppearanceGroup_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES

#include "net/SipSubscribeClient.h"
#include "utl/UtlContainableAtomic.h"
#include "utl/UtlString.h"
#include "utl/UtlHashMap.h"
#include "AppearanceAgent.h"
#include "ResourceSubscriptionReceiver.h"
#include "ResourceNotifyReceiver.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

class Appearance;
class SipDialogEvent;


//! Container for a set of subscriptions to the contacts of an AOR.
/** The AppearanceGroup does not have an independent life beyond the
 *  AppearanceGroupSet that contains it.  Access to it is locked by the
 *  AppearanceGroupSet containing it.
 *  The AppearanceGroup is created and directed to a URI which is presumed to be
 *  an Address Of Record.  It sends a SUBSCRIBE for "reg" (registration)
 *  events to the AOR.  Any NOTIFYs it receives are used to update a list
 *  of contact addresses for the AOR.  For each contact address, an
 *  Appearance is maintained.
 *  The constructor/destructor update the mSubscribeMap and mNotifyMap
 *  of the AppearanceGroupSet so it can process callbacks for its "reg"
 *  subscription.
 */
class AppearanceGroup : public UtlContainableAtomic,
                   public ResourceSubscriptionReceiver,
                   public ResourceNotifyReceiver
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

   //! Construct a subscription set.
   AppearanceGroup(AppearanceGroupSet* appearanceGroupSet,
              const char* sharedUser
              ///< the AOR
      );

   //! Destructor
   virtual ~AppearanceGroup();

   /// Add a new Appearance in this group for callid contact
   void addAppearance(const UtlString* callidContact);

   //! Get the ancestor AppearanceAgent.
   AppearanceAgent* getAppearanceAgent() const;

   /// Start the subscription for Reg events to this shared user.
   void startSubscription();

   const UtlString& getUser() const;

   /** Handle a subscription state callback for the subscription
    *  handled by this AppearanceGroup.
    */
   virtual void subscriptionEventCallback(
      const UtlString* earlyDialogHandle,
      const UtlString* dialogHandle,
      SipSubscribeClient::SubscriptionState newState,
      const UtlString* subscriptionState
      );

   //! Process a notify event callback.
   virtual void notifyEventCallback(const UtlString* dialogHandle,
                                    const SipMessage* msg);

   /// Frees all appearances which were marked as terminated
   void deleteTerminatedAppearances();

   /// Format and publish NOTIFY content.
   void publish(bool bSendFullContent, bool bSendPartialContent, SipDialogEvent* lContent);

   /// Process an incoming NOTIFY from an Appearance.  Always sends a response.
   void handleNotifyRequest(const UtlString* dialogHandle,
                            const SipMessage* msg);

   /** add the newState to the consolidated state for the group, and return true
    * if the state allows a call to proceed.
    */
   bool consolidateState( const UtlString& newState );

   //! Remove dialogs in terminated state and terminated resource instances.
   void purgeTerminated();

   //! Dump the object's internal state.
   void dumpState();

   /// Return a pointer to the Appearance object matching this URI, or NULL
   Appearance* findAppearance(UtlString app);

   void getContent(UtlString& b, ssize_t& l);

   /**
    * Get the ContainableType for a UtlContainable-derived class.
    */
   virtual UtlContainableType getContainableType() const;

   static const UtlContainableType TYPE;    //< Class type used for runtime checking

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:

   //! Update the subscriptions we maintain to agree with mSubscriptions.
   void updateSubscriptions();

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

   AppearanceGroupSet* mAppearanceGroupSet;

   //! The AOR.
   UtlString mSharedUser;

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

   //! The set of Appearance's for this shared AOR.
   /** This hash map is indexed by strings "call-id;URI", where:
    * call-id is the Call-Id of the registration pseudo-dialog for the
    * contact as reported in the <uri> element of the <contact>
    * element in the reg event, and
    * URI is the contact URI as reported in the <uri> element.
    * This indexing allows us to detect when a phone reboots and
    * re-registers the same contact URI with a different REGISTER
    * Call-Id.
    */
   UtlHashMap mAppearances;

   /** The set of terminated subscriptions established by the SUBSCRIBE, as a UtlSList
    *  of Appearance's.
    */
   UtlSList mTerminatedAppearances;

   //! Disabled copy constructor
   AppearanceGroup(const AppearanceGroup& rAppearanceGroup);

   //! Disabled assignment operator
   AppearanceGroup& operator=(const AppearanceGroup& rhs);

};

// Get the ancestor AppearanceAgent.
inline AppearanceAgent* AppearanceGroup::getAppearanceAgent() const
{
   return mAppearanceGroupSet->getAppearanceAgent();
}

inline  const UtlString& AppearanceGroup::getUser() const
{
   return mSharedUser;
}

#endif  // _AppearanceGroup_h_
