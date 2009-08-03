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
#include "ptapi/PtMultiCallMetaEvent.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */
PtMultiCallMetaEvent::PtMultiCallMetaEvent(PtEventId eventId)
:       PtMetaEvent(eventId)
{
}


PtMultiCallMetaEvent::PtMultiCallMetaEvent(PtEventId eventId,
                                                                                int metaCode,
                                                                                TaoClientTask *pClient,
                                                                                int sipResponseCode,
                                                                                const char* sipResponseText,
                                                                                const char* callId,
                                                                                const char* newCallId,
                                                                                const char** oldCallIds,
                                                                                int numOldCalls)
:       PtMetaEvent(eventId, metaCode, numOldCalls, callId, pClient, sipResponseCode, sipResponseText, oldCallIds, newCallId)
{
}

PtMultiCallMetaEvent::PtMultiCallMetaEvent(const PtMultiCallMetaEvent& rPtMultiCallMetaEvent)
:       PtMetaEvent(rPtMultiCallMetaEvent.mEventId,
                                rPtMultiCallMetaEvent.mMetaCode,
                                rPtMultiCallMetaEvent.mNumOldCalls,
                                rPtMultiCallMetaEvent.mCallId,
                                rPtMultiCallMetaEvent.mpClient,
                                rPtMultiCallMetaEvent.mSipResponseCode,
                                rPtMultiCallMetaEvent.mSipResponseText,
                                (const char**) rPtMultiCallMetaEvent.mOldCallIds,
                                rPtMultiCallMetaEvent.mNewCallId)
{
}


PtMultiCallMetaEvent::~PtMultiCallMetaEvent()
{
}

/* ============================ MANIPULATORS ============================== */

PtMultiCallMetaEvent& PtMultiCallMetaEvent::operator=(const PtMultiCallMetaEvent& rhs)
{
   return (PtMultiCallMetaEvent&) PtMetaEvent::operator=(rhs);
}

/* ============================ ACCESSORS ================================= */

PtStatus PtMultiCallMetaEvent::getNewCall(PtCall& rCall) const
{
        rCall = PtCall(PtMetaEvent::mpClient, mNewCallId.data());
        return PT_SUCCESS;
}

PtStatus PtMultiCallMetaEvent::getOldCalls(PtCall rCalls[], int size, int& rNumItems) const
{
        rNumItems = mNumOldCalls;
        if (rNumItems > size)
                rNumItems = size;
        for (int i = 0; i < rNumItems; i++)
        {
                rCalls[i] = PtCall(PtMetaEvent::mpClient, mOldCallIds[i]);
        }

        return PT_SUCCESS;
}

PtStatus PtMultiCallMetaEvent::numOldCalls(int& count) const
{
        count = mNumOldCalls;

        return PT_SUCCESS;
}

/* ============================ INQUIRY =================================== */

PT_IMPLEMENT_CLASS_INFO(PtMultiCallMetaEvent, PtMetaEvent)

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
