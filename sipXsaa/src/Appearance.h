//
// Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _Appearance_h_
#define _Appearance_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES

#include "net/SipSubscribeClient.h"
#include "utl/UtlContainableAtomic.h"
#include "utl/UtlString.h"
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

class AppearanceGroupSet;
class AppearanceGroup;
class SipDialogEvent;


//! Container for a set of subscriptions created by a SUBSCRIBE request.
/** The Appearance does not have an independent life beyond the
 *  AppearanceGroupSet that contains it.  Access to it is locked by the
 *  AppearanceGroupSet containing it.
 *  When the Appearance is created, it generates the SUBSCRIBE request.
 *  It also receives the subscription-state callbacks.
 *  The constructor/destructor update the mSubscribeMap of the AppearanceGroupSet
 *  so this Appearance can always be found by its early dialog handle.
 */
class Appearance : public UtlContainableAtomic,
                   public ResourceSubscriptionReceiver,
                   public ResourceNotifyReceiver
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

   //! Construct an Appearance (subscription to a shared line from a UA).
   Appearance(AppearanceAgent* appAgent,
              AppearanceGroup* appGroup,        ///< ancestor AppearanceGroup object
              UtlString& uri                    ///< the URI to subscribe to
      );

   //! Destructor
   virtual ~Appearance();

   //! Get the ancestor AppearanceGroupSet.
   AppearanceGroupSet& getAppearanceGroupSet() const;
   //! Get the ancestor AppearanceAgent.
   AppearanceAgent* getAppearanceAgent() const;
   //! Get the ancestor AppearanceGroup.
   AppearanceGroup* getAppearanceGroup() const;

   //! Get the subscribed URI.
   const UtlString* getUri() const;

   //! Get the subscribed dialog handle.
   const UtlString* getDialogHandle() const;

   /** Handle a subscription state callback for the subscription
    *  handled by this Appearance.
    *  Overrides ResourceSubscriptionReceiver::subscriptionEventCallback.
    */
   virtual void subscriptionEventCallback(
      const UtlString* earlyDialogHandle,
      const UtlString* dialogHandle,
      SipSubscribeClient::SubscriptionState newState,
      const UtlString* subscriptionState
      );

   /// Process a notify event callback.
   virtual void notifyEventCallback(const UtlString* dialogHandle,
                                    const SipMessage* msg);

   /** Mark dialogs at this appearance as terminated.
    *  Held dialogs are only terminated based on provided flag.
    *  Returns true if content changed
    * (i.e. dialogs were terminated and should be published)
    */
   bool terminateDialogs(bool terminateHeldDialogs);

   /// Terminate this appearance by ending the parent subscription
   /// Notify mapping is also removed.
   bool terminate();

   /// Add dialogs managed by this appearance into the passed dialog event
   void getDialogs(SipDialogEvent *fullContent);

   /// Save or remove the dialogs depending on state.  Returns true if partial update is required.
   bool updateState(SipDialogEvent *partialContent, bool& bFullContentChanged);

   // Returns true if the consolidated state of this appearance instance is busy
   bool appearanceIsBusy();

   /// Returns true if appearance instance has been completely terminated by ending the parent subscription
   bool isTerminated();

   // Returns true if the specified appearanceId (sometimes called x-line-id) is "seized"
   // by this appearance
   bool appearanceIdIsSeized(const UtlString& appearanceId);

   // Change the timeout for the subscription.  It is shortened while the shared line
   // is "seized" so that the line will not be tied up if the set dies.
   void setResubscribeInterval(bool bShortTimeout);

   //! Dump the object's internal state.
   void dumpState();

   /**
    * Get the ContainableType for a UtlContainable-derived class.
    */
   virtual UtlContainableType getContainableType() const;

   static const UtlContainableType TYPE;    //< Class type used for runtime checking

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

   AppearanceAgent* mAppearanceAgent;

   AppearanceGroup* mAppearanceGroup;

   /** The URI to subscribe to (provided in the Contact of a
    *  UA which registers to the shared AOR).
    */
   UtlString mUri;

   // Whether the current subscription expiration time is short (i.e. line is seized by this app)
   bool mbShortTimeout;

   /// Will be set to true when this appearance is terminated
   bool _isTerminated;

   //! The early dialog handle for the subscription.
   UtlString mSubscriptionEarlyDialogHandle;

   /// The established dialog handle for the subscription.
   UtlString mDialogHandle;

   // List of dialogs currently being managed by this instance.
   UtlHashMap mDialogs;

   //! Disabled copy constructor
   Appearance(AppearanceAgent* appAgent,
              const Appearance& rAppearance);

   //! Disabled assignment operator
   Appearance& operator=(const Appearance& rhs);

};

/* ============================ INLINE METHODS ============================ */

// Get the ancestor AppearanceGroupSet.
inline AppearanceGroupSet& Appearance::getAppearanceGroupSet() const
{
   return getAppearanceAgent()->getAppearanceGroupSet();
}

// Get the ancestor AppearanceAgent.
inline AppearanceAgent* Appearance::getAppearanceAgent() const
{
   return mAppearanceAgent;
}

// Get the parent AppearanceGroup.
inline AppearanceGroup* Appearance::getAppearanceGroup() const
{
   return mAppearanceGroup;
}

// Get the subscribed URI.
inline const UtlString* Appearance::getUri() const
{
   return &mUri;
}

// Get the subscribed dialog handle.
inline const UtlString* Appearance::getDialogHandle() const
{
   return &mDialogHandle;
}


#endif  // _Appearance_h_
