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
#include "ptapi/PtMetaEvent.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Default Constructor
PtMetaEvent::PtMetaEvent(PtEventId eventId,
                                int metaCode,
                                int numOldCalls,
                                const char* callId,
                                TaoClientTask *pClient,
                                int sipResponseCode,
                                const char* sipResponseText,
                                const char** oldCallIds,
                                const char* newCallId)
:       PtEvent(eventId, metaCode, numOldCalls, callId, pClient, sipResponseCode, sipResponseText, oldCallIds, newCallId)
{
}

// Copy constructor
PtMetaEvent::PtMetaEvent(const PtMetaEvent& rPtMetaEvent)
:       PtEvent(rPtMetaEvent.mEventId,
                        rPtMetaEvent.mMetaCode,
                        rPtMetaEvent.mNumOldCalls,
                        rPtMetaEvent.mCallId,
                        rPtMetaEvent.mpClient,
                        rPtMetaEvent.mSipResponseCode,
                        rPtMetaEvent.mSipResponseText,
                        (const char**)  rPtMetaEvent.mOldCallIds,
                        rPtMetaEvent.mNewCallId)
{
}

// Destructor
PtMetaEvent::~PtMetaEvent()
{
        // nothing need to be done right now.
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
PtMetaEvent&
PtMetaEvent::operator=(const PtMetaEvent& rhs)
{
   PtEvent::operator=(rhs);

   return *this ;
}

/* ============================ ACCESSORS ================================= */
// Return the event identifier.

/* ============================ INQUIRY =================================== */

PT_IMPLEMENT_CLASS_INFO(PtMetaEvent, PtEvent)


/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
