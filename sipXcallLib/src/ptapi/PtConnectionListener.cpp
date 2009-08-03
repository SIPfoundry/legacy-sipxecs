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
#include "ptapi/PtConnectionListener.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

int FORCE_REFERENCE_PtConnectionListener = 0 ;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
PtConnectionListener::PtConnectionListener(PtEventMask* mask) :
PtCallListener(mask)
{
}

// Copy constructor
PtConnectionListener::PtConnectionListener(const PtConnectionListener& rPtConnectionListener)
{
}

// Destructor
PtConnectionListener::~PtConnectionListener()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
PtConnectionListener&
PtConnectionListener::operator=(const PtConnectionListener& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

void PtConnectionListener::connectionCreated(const PtConnectionEvent& rEvent)
{
}

void PtConnectionListener::connectionAlerting(const PtConnectionEvent& rEvent)
{
}

void PtConnectionListener::connectionDisconnected(const PtConnectionEvent& rEvent)
{
}

void PtConnectionListener::connectionFailed(const PtConnectionEvent& rEvent)
{
}

void PtConnectionListener::connectionUnknown(const PtConnectionEvent& rEvent)
{
}

void PtConnectionListener::connectionDialing(const PtConnectionEvent& rEvent)
{
}

void PtConnectionListener::connectionEstablished(const PtConnectionEvent& rEvent)
{
}

void PtConnectionListener::connectionInitiated(const PtConnectionEvent& rEvent)
{
}

void PtConnectionListener::connectionNetworkAlerting(const PtConnectionEvent& rEvent)
{
}

void PtConnectionListener::connectionNetworkReached(const PtConnectionEvent& rEvent)
{
}

void PtConnectionListener::connectionOffered(const PtConnectionEvent& rEvent)
{
}

void PtConnectionListener::connectionQueued(const PtConnectionEvent& rEvent)
{
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

PT_IMPLEMENT_CLASS_INFO(PtConnectionListener, PtCallListener)

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
