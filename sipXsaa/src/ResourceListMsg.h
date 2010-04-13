//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _ResourceListMsg_h_
#define _ResourceListMsg_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES

#include "net/SipSubscribeClient.h"
#include "os/OsMsg.h"
#include "utl/UtlString.h"

// DEFINES

#define RLS_SUBSCRIPTION_MSG OsMsg::USER_START
#define RLS_NOTIFY_MSG (OsMsg::USER_START + 1)

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS


//: Message to represent subscription state callbacks
class SubscriptionCallbackMsg : public OsMsg
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   SubscriptionCallbackMsg(/// The early dialog handle for the SUBSCRIBE.
                           const char* earlyDialogHandle,
                           /// The dialog handle for the subscription.
                           const char* dialogHandle,
                           /// The new state of the subscription.
                           SipSubscribeClient::SubscriptionState newState,
                           /// The Subscription-State header.
                           const char* subscriptionState);
   //:Constructor

   SubscriptionCallbackMsg(const SubscriptionCallbackMsg& rSubscriptionCallbackMsg);
   //:Copy constructor

   virtual OsMsg* createCopy(void) const;
   //:Create a copy of this msg object (which may be of a derived type)

   virtual ~SubscriptionCallbackMsg();
   //:Destructor

/* ============================ MANIPULATORS ============================== */

   SubscriptionCallbackMsg& operator=(const SubscriptionCallbackMsg& rhs);
   //:Assignment operator

/* ============================ ACCESSORS ================================= */

   virtual int getMsgSize(void) const;
     //:Return the size of the message in bytes
     // This is a virtual method so that it will return the accurate size for
     // the message object even if that object has been upcast to the type of
     // an ancestor class.

   //: Return pointer to mEarlyDialogHandle.
   virtual const UtlString* getEarlyDialogHandle() const;

   //: Return pointer to mDialogHandle.
   virtual const UtlString* getDialogHandle() const;

   //: Return the newState.
   virtual SipSubscribeClient::SubscriptionState getNewState() const;

   //: Return pointer to mSubscriptionState.
   virtual const UtlString* getSubscriptionState() const;

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   //! The earlyDialogHandle, identifying the SUBSCRIBE.
   UtlString mEarlyDialogHandle;
   //! The dialogHandle, identifying the subscription.
   UtlString mDialogHandle;
   //! The new state of the subscription.
   SipSubscribeClient::SubscriptionState mNewState;
   //! The value of the Subscription-State header.
   UtlString mSubscriptionState;

};

/* ============================ INLINE METHODS ============================ */


//: Message to represent NOTIFY callbacks
class NotifyCallbackMsg : public OsMsg
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   NotifyCallbackMsg(/// dialog handle of the subscription
                     const char* dialogHandle,
                     /// location and length of the NOTIFY content
                     //  content_bytes == NULL && content_length == 0 is allowed
                     SipMessage* msg);
   //:Constructor

   NotifyCallbackMsg(const NotifyCallbackMsg& rNotifyCallbackMsg);
   //:Copy constructor

   virtual OsMsg* createCopy(void) const;
   //:Create a copy of this msg object (which may be of a derived type)

   virtual ~NotifyCallbackMsg();
   //:Destructor

/* ============================ MANIPULATORS ============================== */

   NotifyCallbackMsg& operator=(const NotifyCallbackMsg& rhs);
   //:Assignment operator

/* ============================ ACCESSORS ================================= */

   virtual int getMsgSize(void) const;
     //:Return the size of the message in bytes
     // This is a virtual method so that it will return the accurate size for
     // the message object even if that object has been upcast to the type of
     // an ancestor class.

   //: Return pointer to the mDialogHandle.
   virtual const UtlString* getDialogHandle() const;

   //: Return pointer to the mContent.
   virtual const SipMessage* getMsg() const;

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   //! The dialogHandle, identifying the subscription.
   UtlString mDialogHandle;

   //! The content of the NOTIFY.
   SipMessage* mpMsg;

};

/* ============================ INLINE METHODS ============================ */

#endif  // _ResourceListMsg_h_
