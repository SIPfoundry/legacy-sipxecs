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
#include "ptapi/PtSingleCallMetaEvent.h"

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
PtSingleCallMetaEvent::PtSingleCallMetaEvent(PtEventId eventId,
                                                                                int metaCode,
                                                                                TaoClientTask *pClient,
                                                                                int sipResponseCode,
                                                                                const char* sipResponseText,
                                                                                const char* callId)
:       PtMetaEvent(eventId, metaCode, 0, callId, pClient, sipResponseCode, sipResponseText)
{
}


PtSingleCallMetaEvent::PtSingleCallMetaEvent(const PtSingleCallMetaEvent& rPtSingleCallMetaEvent)
:       PtMetaEvent(rPtSingleCallMetaEvent.mEventId,
                                rPtSingleCallMetaEvent.mMetaCode,
                                rPtSingleCallMetaEvent.mNumOldCalls,
                                rPtSingleCallMetaEvent.mCallId,
                                rPtSingleCallMetaEvent.mpClient,
                                rPtSingleCallMetaEvent.mSipResponseCode,
                                rPtSingleCallMetaEvent.mSipResponseText)
{
}


PtSingleCallMetaEvent::~PtSingleCallMetaEvent()
{
}

/* ============================ MANIPULATORS ============================== */

PtSingleCallMetaEvent& PtSingleCallMetaEvent::operator=(const PtSingleCallMetaEvent& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

        mEventId = rhs.mEventId;
        mMetaCode = rhs.mMetaCode;
        mCallId = rhs.mCallId;
        mpClient = rhs.mpClient;

   return *this;
}

/* ============================ ACCESSORS ================================= */

PtStatus PtSingleCallMetaEvent::getCall(PtCall& rCall) const
{
        PtCall call(mpClient, mCallId);
        rCall = call;

        return PT_SUCCESS;
}

/* ============================ INQUIRY =================================== */

PT_IMPLEMENT_CLASS_INFO(PtSingleCallMetaEvent, PtMetaEvent)

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
