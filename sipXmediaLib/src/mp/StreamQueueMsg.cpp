//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "mp/StreamQueueMsg.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
StreamQueueMsg::StreamQueueMsg()
   : OsMsg(USER_START, EndOfFrameMarker)
{
   memset(mSamples, 0, sizeof(mSamples)) ;

   // all of the required work is done by the initializers
}

// Copy constructor
StreamQueueMsg::StreamQueueMsg(const StreamQueueMsg& rStreamQueueMsg)
   : OsMsg(rStreamQueueMsg)
{
   memcpy(mSamples, rStreamQueueMsg.mSamples, sizeof(mSamples)) ;
}


// Create a copy of this msg object (which may be of a derived type)
OsMsg* StreamQueueMsg::createCopy(void) const
{
   return new StreamQueueMsg(*this);
}


// Destructor
StreamQueueMsg::~StreamQueueMsg()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
StreamQueueMsg& StreamQueueMsg::operator=(const StreamQueueMsg& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   OsMsg::operator=(rhs);

   memcpy(mSamples, rhs.mSamples, sizeof(mSamples)) ;

   return *this;
}

// Set the sample data for this message
void StreamQueueMsg::setSamples(const int16_t* pSamples)
{
   setMsgSubType(AudioFrame) ;
   memcpy(mSamples, pSamples, sizeof(mSamples)) ;
}

/* ============================ ACCESSORS ================================= */

// Get the sample data for this message
UtlBoolean StreamQueueMsg::getSamples(int16_t* pSamples) const
{
   UtlBoolean bRC = FALSE ;

   if (getMsgSubType() == AudioFrame)
   {
      memcpy(pSamples, mSamples, sizeof(mSamples)) ;
      bRC = TRUE ;
   }

   return bRC ;
}


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
