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

#include <os/OsMsg.h>
#include <net/SipSubscribeClient.h>
#include <utl/UtlString.h>

// DEFINES

#define RLS_SUBSCRIPTION_MSG (OsMsg::USER_START)
#define RLS_NOTIFY_MSG (OsMsg::USER_START + 1)
#define RLS_SUBSCRIPTION_SET_MSG (OsMsg::USER_START + 2)

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
                     const char* content_bytes,
                     int content_length);
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
   virtual const UtlString* getContent() const;

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   //! The dialogHandle, identifying the subscription.
   UtlString mDialogHandle;

   //! The content of the NOTIFY.
   UtlString mContent;

};

/* ============================ INLINE METHODS ============================ */

//: Message to trigger creation of a new SubscriptionSet
class SubscriptionSetMsg : public OsMsg
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

    SubscriptionSetMsg(/// The handler for this message which has knowledge on how to process it.
            const UtlString& uri,
            /// The contact for the new SubscriptionSet.
            const UtlString& callidContact);
   //:Constructor

    SubscriptionSetMsg(const SubscriptionSetMsg& rhs);
   //:Copy constructor

   virtual OsMsg* createCopy(void) const;
   //:Create a copy of this msg object (which may be of a derived type)

   virtual ~SubscriptionSetMsg();
   //:Destructor

/* ============================ MANIPULATORS ============================== */

   SubscriptionSetMsg& operator=(const SubscriptionSetMsg& rhs);
   //:Assignment operator

/* ============================ ACCESSORS ================================= */

   virtual int getMsgSize(void) const;
     //:Return the size of the message in bytes
     // This is a virtual method so that it will return the accurate size for
     // the message object even if that object has been upcast to the type of
     // an ancestor class.

   //: Return the handler.
   virtual const UtlString& getUri() const;

   //: Return pointer to callid contact.
   virtual const UtlString& getCallidContact() const;

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   UtlString _uri;
   UtlString _callidContact;
};
#endif  // _ResourceListMsg_h_
