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
#include "mp/MpStreamMsg.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

// Message object used to communicate with the media processing task

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MpStreamMsg::MpStreamMsg(int msg, UtlString& target, StreamHandle handle,
                         void* pPtr1, void* pPtr2, intptr_t int1, intptr_t int2)
:  OsMsg(OsMsg::STREAMING_MSG, msg),
   mTarget(target),
   mHandle(handle),
   mpPtr1(pPtr1),
   mpPtr2(pPtr2),
   mInt1(int1),
   mInt2(int2)
{
   // all of the work is done by the initializers
}

// Copy constructor
MpStreamMsg::MpStreamMsg(const MpStreamMsg& rMpStreamMsg)
:  OsMsg(rMpStreamMsg)
{
   mTarget  = rMpStreamMsg.mTarget;
   mHandle  = rMpStreamMsg.mHandle;
   mpPtr1   = rMpStreamMsg.mpPtr1;
   mpPtr2   = rMpStreamMsg.mpPtr2;
   mInt1    = rMpStreamMsg.mInt1;
   mInt2    = rMpStreamMsg.mInt2;
}

// Create a copy of this msg object (which may be of a derived type)
OsMsg* MpStreamMsg::createCopy(void) const
{
   return new MpStreamMsg(*this);
}

// Destructor
MpStreamMsg::~MpStreamMsg()
{
   // no work required
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
MpStreamMsg&
MpStreamMsg::operator=(const MpStreamMsg& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   OsMsg::operator=(rhs);       // assign fields for parent class

   mTarget  = rhs.mTarget;
   mHandle  = rhs.mHandle;
   mpPtr1   = rhs.mpPtr1;
   mpPtr2   = rhs.mpPtr2;
   mInt1    = rhs.mInt1;
   mInt2    = rhs.mInt2;

   return *this;
}


// Set target id of the stream message
void MpStreamMsg::setTarget(UtlString& target)
{
   mTarget = target;
}

// Set stream handle of the stream message
void MpStreamMsg::setHandle(StreamHandle handle)
{
   mHandle = handle;
}

// Set pointer 1 (void*) of the stream message
void MpStreamMsg::setPtr1(void* p)
{
   mpPtr1 = p;
}

// Set pointer 2 (void*) of the stream message
void MpStreamMsg::setPtr2(void* p)
{
   mpPtr2 = p;
}

// Set integer 1 of the stream message
void MpStreamMsg::setInt1(intptr_t i)
{
   mInt1 = i;
}

// Set integer 2 of the stream message
void MpStreamMsg::setInt2(intptr_t i)
{
   mInt2 = i;
}

/* ============================ ACCESSORS ================================= */

// Return the type of the stream message
int MpStreamMsg::getMsg(void) const
{
   return OsMsg::getMsgSubType();
}

// Return target id of the stream message
UtlString MpStreamMsg::getTarget(void) const
{
   return mTarget;
}

// Return stream handle of the stream message
StreamHandle MpStreamMsg::getHandle(void) const
{
   return mHandle;
}

// Return pointer 1 (void*) of the stream message
void* MpStreamMsg::getPtr1(void) const
{
   return mpPtr1;
}

// Return pointer 2 (void*) of the stream message
void* MpStreamMsg::getPtr2(void) const
{
   return mpPtr2;
}

// Return integer 1 of the stream message
intptr_t MpStreamMsg::getInt1(void) const
{
   return mInt1;
}

// Return integer 2 of the stream message
intptr_t MpStreamMsg::getInt2(void) const
{
   return mInt2;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
