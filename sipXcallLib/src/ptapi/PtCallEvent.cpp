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
#include <string.h>

// APPLICATION INCLUDES
#include "ptapi/PtMultiCallMetaEvent.h"
#include "ptapi/PtSingleCallMetaEvent.h"
#include "ptapi/PtCallEvent.h"
#include "ptapi/PtCall.h"
#include "tao/TaoClientTask.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

//  Default Constructor
PtCallEvent::PtCallEvent(PtEvent::PtEventId eventId) :
PtEvent(eventId)
{
        mCallId = "";
        mpClient = 0;

}

PtCallEvent::PtCallEvent(PtEventId eventId,
                                int metaCode,
                                const char* callId,
                                TaoClientTask *pClient,
                                int sipResponseCode,
                                const char* sipResponseText,
                                const char* newCallId,
                                const char** oldCallIds,
                                int numOldCalls)
: PtEvent(eventId, metaCode, numOldCalls, callId, pClient, sipResponseCode, sipResponseText, oldCallIds, newCallId)
{
}

PtCallEvent::PtCallEvent(TaoClientTask *pClient)
: PtEvent()
{
        mpClient = pClient;
}


// Copy constructor
PtCallEvent::PtCallEvent(const PtCallEvent& rPtCallEvent)
:
PtEvent(rPtCallEvent)
{
}

// Destructor
PtCallEvent::~PtCallEvent()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
PtCallEvent&
PtCallEvent::operator=(const PtCallEvent& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

        PtEvent::operator=(rhs);

   return *this;
}

/* ============================ ACCESSORS ================================= */
// Return the call object associated with the event.
PtStatus PtCallEvent::getCall(PtCall& rCall) const
{
        PtCall call(mpClient, mCallId);
        rCall = call;

        return PT_SUCCESS;
}

/* ============================ INQUIRY =================================== */

PT_IMPLEMENT_CLASS_INFO(PtCallEvent, PtEvent)

/* //////////////////////////// PROTECTED ///////////////////////////////// */
// Protected constructor.

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
