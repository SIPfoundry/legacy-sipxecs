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
#include "mp/MpFlowGraphMsg.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

// Message object used to communicate with the media processing task

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MpFlowGraphMsg::MpFlowGraphMsg(int msg, MpResource* pMsgDest,
                               void* pPtr1, void* pPtr2,
                               int int1, int int2)
:  OsMsg(OsMsg::MP_FLOWGRAPH_MSG, msg),
   mpMsgDest(pMsgDest),
   mpPtr1(pPtr1),
   mpPtr2(pPtr2),
   mInt1(int1),
   mInt2(int2)
{
}

// Copy constructor
MpFlowGraphMsg::MpFlowGraphMsg(const MpFlowGraphMsg& rMpFlowGraphMsg)
:  OsMsg(rMpFlowGraphMsg)
{
   mpMsgDest = rMpFlowGraphMsg.mpMsgDest;
   mpPtr1    = rMpFlowGraphMsg.mpPtr1;
   mpPtr2    = rMpFlowGraphMsg.mpPtr2;
   mInt1     = rMpFlowGraphMsg.mInt1;
   mInt2     = rMpFlowGraphMsg.mInt2;
}

// Create a copy of this msg object (which may be of a derived type)
OsMsg* MpFlowGraphMsg::createCopy(void) const
{
   return new MpFlowGraphMsg(*this);
}

// Destructor
MpFlowGraphMsg::~MpFlowGraphMsg()
{
   // no work required
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
MpFlowGraphMsg&
MpFlowGraphMsg::operator=(const MpFlowGraphMsg& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   OsMsg::operator=(rhs);       // assign fields for parent class

   mpMsgDest = rhs.mpMsgDest;
   mpPtr1    = rhs.mpPtr1;
   mpPtr2    = rhs.mpPtr2;
   mInt1     = rhs.mInt1;
   mInt2     = rhs.mInt2;

   return *this;
}

// Sets the intended recipient for this message.  Setting the message
// destination to NULL indicates that the message is intended for the
// flow graph itself.
void MpFlowGraphMsg::setMsgDest(MpResource* pMsgDest)
{
   mpMsgDest = pMsgDest;
}


// Set pointer 1 (void*) of the media flow graph message
void MpFlowGraphMsg::setPtr1(void* p)
{
   mpPtr1 = p;
}

// Set pointer 2 (void*) of the media flow graph message
void MpFlowGraphMsg::setPtr2(void* p)
{
   mpPtr2 = p;
}

// Set integer 1 of the media flow graph message
void MpFlowGraphMsg::setInt1(int i)
{
   mInt1 = i;
}

// Set integer 2 of the media flow graph message
void MpFlowGraphMsg::setInt2(int i)
{
   mInt2 = i;
}

/* ============================ ACCESSORS ================================= */

// Return the type of the media flow graph message
int MpFlowGraphMsg::getMsg(void) const
{
   return OsMsg::getMsgSubType();
}

// Returns the MpResource object that is the intended recipient for this
// message.  A NULL return indicates that the message is intended for
// the flow graph itself.
MpResource* MpFlowGraphMsg::getMsgDest(void) const
{
   return mpMsgDest;
}

// Return pointer 1 (void*) of the media flow graph message
void* MpFlowGraphMsg::getPtr1(void) const
{
   return mpPtr1;
}

// Return pointer 2 (void*) of the media flow graph message
void* MpFlowGraphMsg::getPtr2(void) const
{
   return mpPtr2;
}

// Return integer 1 of the media flow graph message
int MpFlowGraphMsg::getInt1(void) const
{
   return mInt1;
}

// Return integer 2 of the media flow graph message
int MpFlowGraphMsg::getInt2(void) const
{
   return mInt2;
}


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
