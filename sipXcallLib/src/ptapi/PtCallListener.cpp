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
#include "ptapi/PtCallListener.h"
#include "os/OsSocket.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
PtCallListener::PtCallListener(PtEventMask* mask) :
PtEventListener(mask)
{
}

// Copy constructor
PtCallListener::PtCallListener(const PtCallListener& rPtCallListener)
{
}

// Destructor
PtCallListener::~PtCallListener()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
PtCallListener&
PtCallListener::operator=(const PtCallListener& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

void PtCallListener::callEventTransmissionEnded(const PtCallEvent& rEvent)
{
}

void PtCallListener::callActive(const PtCallEvent& rEvent)
{
}

void PtCallListener::callInvalid(const PtCallEvent& rEvent)
{
}

void PtCallListener::callMetaCallStartStarted(const PtCallEvent& rEvent)
{
}

void PtCallListener::callMetaCallStartEnded(const PtCallEvent& rEvent)
{
}

void PtCallListener::callMetaCallEndStarted(const PtCallEvent& rEvent)
{
}

void PtCallListener::callMetaCallEndEnded(const PtCallEvent& rEvent)
{
}

void PtCallListener::callMetaProgressStarted(const PtCallEvent& rEvent)
{
}

void PtCallListener::callMetaProgressEnded(const PtCallEvent& rEvent)
{
}

void PtCallListener::callMetaSnapshotStarted(const PtCallEvent& rEvent)
{
}

void PtCallListener::callMetaSnapshotEnded(const PtCallEvent& rEvent)
{
}

void PtCallListener::callMetaAddPartyStarted(const PtCallEvent& rEvent)
{
}

void PtCallListener::callMetaAddPartyEnded(const PtCallEvent& rEvent)
{
}

void PtCallListener::callMetaRemovePartyStarted(const PtCallEvent& rEvent)
{
}

void PtCallListener::callMetaRemovePartyEnded(const PtCallEvent& rEvent)
{
}

void PtCallListener::multicallMetaMergeStarted(const PtMultiCallMetaEvent& rEvent)
{
}

void PtCallListener::multicallMetaMergeEnded(const PtMultiCallMetaEvent& rEvent)
{
}

void PtCallListener::multicallMetaTransferStarted(const PtMultiCallMetaEvent& rEvent)
{
}

void PtCallListener::multicallMetaTransferEnded(const PtMultiCallMetaEvent& rEvent)
{
}

/* ============================ ACCESSORS ================================= */
PtStatus PtCallListener::getLocation(UtlString* rpLocation)
{

        OsSocket::getHostIp(rpLocation);
        return PT_SUCCESS;
}

/* ============================ INQUIRY =================================== */

PT_IMPLEMENT_CLASS_INFO(PtCallListener, PtEventListener)

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
