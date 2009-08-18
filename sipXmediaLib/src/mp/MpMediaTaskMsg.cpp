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
#include "mp/MpMediaTaskMsg.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

// Message object used to communicate with the media processing task

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MpMediaTaskMsg::MpMediaTaskMsg(int msg, void* pPtr1, void* pPtr2,
      intptr_t int1, intptr_t int2)
:  OsMsg(OsMsg::MP_TASK_MSG, msg),
   mpPtr1(pPtr1),
   mpPtr2(pPtr2),
   mInt1(int1),
   mInt2(int2)
{
   // all of the work is done by the initializers
}

// Copy constructor
MpMediaTaskMsg::MpMediaTaskMsg(const MpMediaTaskMsg& rMpMediaTaskMsg)
:  OsMsg(rMpMediaTaskMsg)
{
   mpPtr1 = rMpMediaTaskMsg.mpPtr1;
   mpPtr2 = rMpMediaTaskMsg.mpPtr2;
   mInt1  = rMpMediaTaskMsg.mInt1;
   mInt2  = rMpMediaTaskMsg.mInt2;
}

// Create a copy of this msg object (which may be of a derived type)
OsMsg* MpMediaTaskMsg::createCopy(void) const
{
   return new MpMediaTaskMsg(*this);
}

// Destructor
MpMediaTaskMsg::~MpMediaTaskMsg()
{
   // no work required
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
MpMediaTaskMsg&
MpMediaTaskMsg::operator=(const MpMediaTaskMsg& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   OsMsg::operator=(rhs);       // assign fields for parent class

   mpPtr1 = rhs.mpPtr1;
   mpPtr2 = rhs.mpPtr2;
   mInt1  = rhs.mInt1;
   mInt2  = rhs.mInt2;

   return *this;
}

// Set pointer 1 (void*) of the media task message
void MpMediaTaskMsg::setPtr1(void* p)
{
   mpPtr1 = p;
}

// Set pointer 2 (void*) of the media task message
void MpMediaTaskMsg::setPtr2(void* p)
{
   mpPtr2 = p;
}

// Set integer 1 of the media task message
void MpMediaTaskMsg::setInt1(intptr_t i)
{
   mInt1 = i;
}

// Set integer 2 of the media task message
void MpMediaTaskMsg::setInt2(intptr_t i)
{
   mInt2 = i;
}

/* ============================ ACCESSORS ================================= */

// Return the type of the media task message
int MpMediaTaskMsg::getMsg(void) const
{
   return OsMsg::getMsgSubType();
}

// Return pointer 1 (void*) of the media task message
void* MpMediaTaskMsg::getPtr1(void) const
{
   return mpPtr1;
}

// Return pointer 2 (void*) of the media task message
void* MpMediaTaskMsg::getPtr2(void) const
{
   return mpPtr2;
}

// Return integer 1 of the media task message
intptr_t MpMediaTaskMsg::getInt1(void) const
{
   return mInt1;
}

// Return integer 2 of the media task message
intptr_t MpMediaTaskMsg::getInt2(void) const
{
   return mInt2;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
