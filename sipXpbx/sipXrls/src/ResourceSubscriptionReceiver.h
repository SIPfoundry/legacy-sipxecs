//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _ResourceSubscriptionReceiver_h_
#define _ResourceSubscriptionReceiver_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES

#include <net/SipMessage.h>
#include <net/SipSubscribeClient.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS


//! Abstract class for processing subscription state change callbacks within a ResourceListSet.
/** When a subscription state callback is received by a ResourceListSet,
 *  once the thread obtains the lock on the ResourceListSet, it looks
 *  up the dialog handle in mSubscriptionMap to find the
 *  ResourceSubscriptionReceiver responsible for the subscription, and
 *  calls its subscriptionEventCallback method.
 */
class ResourceSubscriptionReceiver
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

   //! Construct an instance
   ResourceSubscriptionReceiver();

   //! Destructor
   virtual ~ResourceSubscriptionReceiver();

   /** Handle a subscription state callback for the subscription
    *  handled by this ResourceSubscriptionReceiver.
    *  Overridden by every subclass.
    */
   virtual void subscriptionEventCallback(
      const UtlString* earlyDialogHandle,
      const UtlString* dialogHandle,
      SipSubscribeClient::SubscriptionState newState,
      const UtlString* subscriptionState
      ) = 0;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

   //! Disabled copy constructor
   ResourceSubscriptionReceiver(const ResourceSubscriptionReceiver& rResourceSubscriptionReceiver);

   //! Disabled assignment operator
   ResourceSubscriptionReceiver& operator=(const ResourceSubscriptionReceiver& rhs);

};

#endif  // _ResourceSubscriptionReceiver_h_
