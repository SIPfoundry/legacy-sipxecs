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
#include "mp/MpBufferMsg.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

// Message object used to enqueue and dequeue media processing buffers

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MpBufferMsg::MpBufferMsg(int msg, int from, MpBufPtr pTag, Sample* pSamples,
                int len)
:  OsMsg(OsMsg::MP_BUFFER_MSG, msg),
   mFrom(from)
{
   int i;

   setTag(pTag);
   setBuf(pSamples);
   setLen(len);

   for (i=1; i<MAX_BUFFERS_IN_MSG; i++) {
      setTag(NULL, i);
      setBuf(NULL, i);
      setLen(0, i);
   }

}

// Copy constructor
MpBufferMsg::MpBufferMsg(const MpBufferMsg& rMpBufferMsg)
:  OsMsg(rMpBufferMsg)
{
   int i;

   for (i=0; i<MAX_BUFFERS_IN_MSG; i++) {
      mpTag[i] = rMpBufferMsg.mpTag[i];
      mpBuf[i] = rMpBufferMsg.mpBuf[i];
      mLen[i]  = rMpBufferMsg.mLen[i];
   }

   mFrom = rMpBufferMsg.mFrom;
}

// Create a copy of this msg object (which may be of a derived type)
OsMsg* MpBufferMsg::createCopy(void) const
{
   return new MpBufferMsg(*this);
}

// Destructor
MpBufferMsg::~MpBufferMsg()
{
   // no work required
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
MpBufferMsg&
MpBufferMsg::operator=(const MpBufferMsg& rhs)
{
   int i;

   if (this == &rhs)            // handle the assignment to self case
      return *this;

   OsMsg::operator=(rhs);       // assign fields for parent class

   for (i=0; i<MAX_BUFFERS_IN_MSG; i++) {
      mpTag[i] = rhs.mpTag[i];
      mpBuf[i] = rhs.mpBuf[i];
      mLen[i]  = rhs.mLen[i];
   }

   mFrom = rhs.mFrom;

   return *this;
}

// Set pointer 1 (void*) of the buffer message
void MpBufferMsg::setTag(MpBufPtr p, int index)
{
   mpTag[index] = p;
}

// Set pointer 2 (void*) of the buffer message
void MpBufferMsg::setBuf(Sample* p, int index)
{
   mpBuf[index] = p;
}

// Set integer 1 of the buffer message
void MpBufferMsg::setLen(int i, int index)
{
   mLen[index] = i;
}

/* ============================ ACCESSORS ================================= */

// Return the type of the media task message
int MpBufferMsg::getMsg(void) const
{
   return OsMsg::getMsgSubType();
}

// Return pointer 1 (void*) of the buffer message
MpBufPtr MpBufferMsg::getTag(int index) const
{
   return mpTag[index];
}

// Return pointer 2 (void*) of the buffer message
Sample* MpBufferMsg::getBuf(int index) const
{
   return mpBuf[index];
}

// Return integer 1 of the buffer message
int MpBufferMsg::getLen(int index) const
{
   return mLen[index];
}

// Return integer 2 of the buffer message
int MpBufferMsg::getFrom(void) const
{
   return mFrom;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
