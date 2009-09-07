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
#include <assert.h>

// APPLICATION INCLUDES
#include "os/OsMsg.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const UtlContainableType OsMsg::TYPE = "OsMsg" ;

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsMsg::OsMsg(const unsigned char msgType, const unsigned char msgSubType)
: mMsgType(msgType), mMsgSubType(msgSubType), mSentFromISR(FALSE),
   mReusable(FALSE), mInUse(TRUE)
{
   // all of the required work is done by the initializers
}

// Copy constructor
OsMsg::OsMsg(const OsMsg& rOsMsg)
{
   mMsgType     = rOsMsg.mMsgType;
   mMsgSubType  = rOsMsg.mMsgSubType;
   mSentFromISR = rOsMsg.mSentFromISR;
   mReusable    = rOsMsg.mReusable;
   mInUse       = rOsMsg.mInUse;
}

// Create a copy of this msg object (which may be of a derived type)
OsMsg* OsMsg::createCopy(void) const
{
   return new OsMsg(*this);
}

// Done with message, delete it or mark it unused
void OsMsg::releaseMsg()
{
   if (isMsgReusable()) {
      setInUse(FALSE);
   } else {
      delete this;
   }
}

// Destructor
OsMsg::~OsMsg()
{
   // no work required
   assert(!isMsgReusable());
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
OsMsg&
OsMsg::operator=(const OsMsg& rhs)
{
   if (this != &rhs)            // handle the assignment to self case
   {
      mMsgType     = rhs.mMsgType;
      mMsgSubType  = rhs.mMsgSubType;
      mSentFromISR = rhs.mSentFromISR;
      mReusable    = rhs.mReusable;
      mInUse       = rhs.mInUse;
   }

   return *this;
}

// Set the message subtype
void OsMsg::setMsgSubType(unsigned char subType)
{
   mMsgSubType = subType;
}

// Set the SentFromISR (interrupt service routine) flag.
void OsMsg::setSentFromISR(UtlBoolean sentFromISR)
{
   mSentFromISR = sentFromISR;
}

// Set the Is Reusable (from permanent pool) flag
void OsMsg::setReusable(UtlBoolean isReusable)
{
   mReusable = isReusable;
}

// Set the Is In Use flag
void OsMsg::setInUse(UtlBoolean isInUse)
{
   mInUse = isInUse;
}

/* ============================ ACCESSORS ================================= */

// Return the message type
unsigned char OsMsg::getMsgType(void) const
{
   return mMsgType;
}

// Return the message subtype
unsigned char OsMsg::getMsgSubType(void) const
{
   return mMsgSubType;
}

// Return the size of the message in bytes.
// This is a virtual method so that it will return the accurate size for
// the message object even if that object has been upcast to the type of
// an ancestor class.
int OsMsg::getMsgSize(void) const
{
   return sizeof(*this);
}

// Return TRUE if msg was sent from an interrupt svc routine, else FALSE.
UtlBoolean OsMsg::getSentFromISR(void) const
{
   return mSentFromISR;
}

// Return TRUE if msg is from a permanent pool, else FALSE
UtlBoolean OsMsg::isMsgReusable(void) const
{
   return mReusable;
}

// Return TRUE if msg is currently in use, else FALSE
UtlBoolean OsMsg::isMsgInUse(void) const
{
   return mInUse;
}

//! Implements the interface for a UtlContainable
unsigned OsMsg::hash() const
{
   return hashPtr(this);
}

UtlContainableType OsMsg::getContainableType() const
{
   return OsMsg::TYPE ;
}

int OsMsg::compareTo(UtlContainable const* other) const
{
   return comparePtrs(this, other);
}


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */
