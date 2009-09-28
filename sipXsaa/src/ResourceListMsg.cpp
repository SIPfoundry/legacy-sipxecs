//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// SYSTEM INCLUDES
// APPLICATION INCLUDES

#include "ResourceListMsg.h"
#include <os/OsSysLog.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SubscriptionCallbackMsg::SubscriptionCallbackMsg(const char* earlyDialogHandle,
                                                 const char* dialogHandle,
                                                 SipSubscribeClient:: SubscriptionState newState,
                                                 const char* subscriptionState) :
   OsMsg(RLS_SUBSCRIPTION_MSG, 0),
   mEarlyDialogHandle(earlyDialogHandle),
   mDialogHandle(dialogHandle),
   mNewState(newState),
   mSubscriptionState(subscriptionState)
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "SubscriptionCallbackMsg:: earlyDialogHandle = '%s', dialogHandle = '%s', newState = %d, subscriptionState = '%s'",
                 earlyDialogHandle, dialogHandle, newState, subscriptionState);
}

// Copy constructor
SubscriptionCallbackMsg::SubscriptionCallbackMsg(const SubscriptionCallbackMsg& rSubscriptionCallbackMsg)
:  OsMsg(rSubscriptionCallbackMsg)
{
   mEarlyDialogHandle = rSubscriptionCallbackMsg.mEarlyDialogHandle;
   mDialogHandle      = rSubscriptionCallbackMsg.mDialogHandle;
   mNewState          = rSubscriptionCallbackMsg.mNewState;
   mSubscriptionState = rSubscriptionCallbackMsg.mSubscriptionState;
}

// Create a copy of this msg object (which may be of a derived type)
OsMsg* SubscriptionCallbackMsg::createCopy(void) const
{
   return new SubscriptionCallbackMsg(*this);
}

// Destructor
SubscriptionCallbackMsg::~SubscriptionCallbackMsg()
{
   // no work required
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
SubscriptionCallbackMsg&
SubscriptionCallbackMsg::operator=(const SubscriptionCallbackMsg& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   OsMsg::operator=(rhs);       // assign fields for parent class

   mEarlyDialogHandle = rhs.mEarlyDialogHandle;
   mDialogHandle      = rhs.mDialogHandle;
   mNewState          = rhs.mNewState;
   mSubscriptionState = rhs.mSubscriptionState;

   return *this;
}

/* ============================ ACCESSORS ================================= */

// Return the size of the message in bytes.
// This is a virtual method so that it will return the accurate size for
// the message object even if that object has been upcast to the type of
// an ancestor class.
int SubscriptionCallbackMsg::getMsgSize(void) const
{
   return sizeof(*this);
}

// Return pointer to mEarlyDialogHandle.
const UtlString* SubscriptionCallbackMsg::getEarlyDialogHandle() const
{
   return &mEarlyDialogHandle;
}

// Return pointer to mDialogHandle.
const UtlString* SubscriptionCallbackMsg::getDialogHandle() const
{
   return &mDialogHandle;
}

// Return the newState.
SipSubscribeClient::SubscriptionState SubscriptionCallbackMsg::getNewState() const
{
   return mNewState;
}

// Return pointer to mSubscriptionState.
const UtlString* SubscriptionCallbackMsg::getSubscriptionState() const
{
   return &mSubscriptionState;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
NotifyCallbackMsg::NotifyCallbackMsg(const char* dialogHandle,
                                     SipMessage* msg) :
   OsMsg(RLS_NOTIFY_MSG, 0),
   mDialogHandle(dialogHandle),
   mpMsg(msg)
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "NotifyCallbackMsg:: dialogHandle = '%s'",
                 dialogHandle);
}

// Copy constructor
NotifyCallbackMsg::NotifyCallbackMsg(const NotifyCallbackMsg& rNotifyCallbackMsg)
:  OsMsg(rNotifyCallbackMsg)
{
   mDialogHandle = rNotifyCallbackMsg.mDialogHandle;
   mpMsg         = rNotifyCallbackMsg.mpMsg;
}

// Create a copy of this msg object (which may be of a derived type)
OsMsg* NotifyCallbackMsg::createCopy(void) const
{
   return new NotifyCallbackMsg(*this);
}

// Destructor
NotifyCallbackMsg::~NotifyCallbackMsg()
{
   // no work required
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
NotifyCallbackMsg&
NotifyCallbackMsg::operator=(const NotifyCallbackMsg& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   OsMsg::operator=(rhs);       // assign fields for parent class

   mDialogHandle = rhs.mDialogHandle;
   mpMsg         = rhs.mpMsg;

   return *this;
}

/* ============================ ACCESSORS ================================= */

// Return the size of the message in bytes.
// This is a virtual method so that it will return the accurate size for
// the message object even if that object has been upcast to the type of
// an ancestor class.
int NotifyCallbackMsg::getMsgSize(void) const
{
   return sizeof(*this);
}

// Return pointer to the mDialogHandle.
const UtlString* NotifyCallbackMsg::getDialogHandle() const
{
   return &mDialogHandle;
}

// Return pointer to the NOTIFY SipMessage.
const SipMessage* NotifyCallbackMsg::getMsg() const
{
   return mpMsg;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
