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
#include "os/OsTimerMsg.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsTimerMsg::OsTimerMsg(const unsigned char subType,
                       OsTimer* pTimer,
                       OsEvent* pEvent)
: OsRpcMsg(OsMsg::OS_TIMER, subType, *pEvent),
  mpTimer(pTimer)
{
   init();
}

// Copy constructor
OsTimerMsg::OsTimerMsg(const OsTimerMsg& rOsTimerMsg)
: OsRpcMsg(rOsTimerMsg)
{
   mpTimer = rOsTimerMsg.mpTimer;
}

// Create a copy of this msg object (which may be of a derived type)
OsMsg* OsTimerMsg::createCopy(void) const
{
   return new OsTimerMsg(*this);
}

// Destructor
OsTimerMsg::~OsTimerMsg()
{
   // no work required
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
OsTimerMsg&
OsTimerMsg::operator=(const OsTimerMsg& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   OsMsg::operator=(rhs);       // assign fields for parent class

   mpTimer = rhs.mpTimer;

   return *this;
}

/* ============================ ACCESSORS ================================= */

// Return the size of the message in bytes.
// This is a virtual method so that it will return the accurate size for
// the message object even if that object has been upcast to the type of
// an ancestor class.
int OsTimerMsg::getMsgSize(void) const
{
   return sizeof(*this);
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

// Initialization common to all constructors
void OsTimerMsg::init(void)
{
}

/* ============================ FUNCTIONS ================================= */
