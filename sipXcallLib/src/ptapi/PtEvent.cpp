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
#include <stdlib.h>
#include <string.h>

// APPLICATION INCLUDES
#include "ptapi/PtEvent.h"
#include "ptapi/PtMultiCallMetaEvent.h"
#include "ptapi/PtSingleCallMetaEvent.h"

#include "tao/TaoClientTask.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Default Constructor
PtEvent::PtEvent(int eventId,
                                 int metaCode,
                                 int numOldCalls,
                                 const char* callId,
                                 TaoClientTask *pClient,
                                 int sipResponseCode,
                                 const char* sipResponseText,
                                 const char** pMetaEventCallIds,
                                 const char* newCallId,
                                 PtEventCause cause,
                                 int isLocal) :
mIsLocal(isLocal),
mEventId((PtEventId)eventId),
mEventCause(cause),
mMetaCode((PtMetaCode)metaCode),
mNumOldCalls(numOldCalls),
mpClient(pClient),
mSipResponseCode(sipResponseCode)
{

    if (sipResponseText)
                mSipResponseText = sipResponseText;

        if (callId)
                mCallId = callId;

        if (newCallId)
                mNewCallId = newCallId ;

        if (numOldCalls > 0 && pMetaEventCallIds != NULL)
        {
                for(int i = 0; i < numOldCalls; i++)
                {
         mOldCallIds[i] = strdup(pMetaEventCallIds[i]) ;
                }
        }
}

// Copy constructor
PtEvent::PtEvent(const PtEvent& rPtEvent)
{
        mMetaCode = rPtEvent.mMetaCode;
        mEventId = rPtEvent.mEventId;
        mEventCause = rPtEvent.mEventCause;
        mNumOldCalls = rPtEvent.mNumOldCalls;
        mpClient = rPtEvent.mpClient;
        mCallId = rPtEvent.mCallId;
        mNewCallId = rPtEvent.mNewCallId;
        mIsLocal = rPtEvent.mIsLocal;
        mSipResponseCode = rPtEvent.mSipResponseCode;
        mSipResponseText = rPtEvent.mSipResponseText;

        if (mNumOldCalls > 0)
        {
                for(int i = 0; i < mNumOldCalls; i++)
                {
         mOldCallIds[i] = strdup(rPtEvent.mOldCallIds[i]) ;
                }
        }
}

// Destructor
PtEvent::~PtEvent()
{
   for(int i = 0; i < mNumOldCalls; i++)
   {
      free(mOldCallIds[i]) ;
   }
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
PtEvent&
PtEvent::operator=(const PtEvent& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

        mMetaCode = rhs.mMetaCode;
        mEventId = rhs.mEventId;
        mEventCause = rhs.mEventCause;
        mNumOldCalls = rhs.mNumOldCalls;
        mpClient = rhs.mpClient;
        mCallId = rhs.mCallId;
        mNewCallId = rhs.mNewCallId;
        mIsLocal = rhs.mIsLocal;
        mSipResponseCode = rhs.mSipResponseCode;
        mSipResponseText = rhs.mSipResponseText;

        if (mNumOldCalls > 0)
        {
                for(int i = 0; i < mNumOldCalls; i++)
                {
                        mOldCallIds[i] = strdup(rhs.mOldCallIds[i]);
                }
        }
   return *this;
}

void PtEvent::setEventId(PtEventId eventId)
{
        mEventId = eventId;
}

void PtEvent::setMetaCode(PtMetaCode metaCode)
{
        mMetaCode = metaCode;
}

void PtEvent::setEventCallId(const char* callId)
{
        mCallId = callId;
}

void PtEvent::setEventSipResponseCode(int sipResponseCode)
{
        mSipResponseCode = sipResponseCode;
}

void PtEvent::setEventSipResponseText(const char* sipResponseText)
{
        mSipResponseText = sipResponseText;
}

void PtEvent::setEventNewCallId(const char* newCallId)
{
        mNewCallId = newCallId;
}


void PtEvent::setEventOldCallIds(int numOldCalls, UtlString* oldCallIds)
{
        mNumOldCalls = numOldCalls;
        if (mNumOldCalls > 0 && oldCallIds)
        {
                for(int i = 0; i < mNumOldCalls; i++)
                {
                        mOldCallIds[i] = strdup(oldCallIds[i].data());
                }
        }
}

void PtEvent::setEventCause(PtEventCause cause)
{
        mEventCause = cause;
}

void PtEvent::setEventLocal(int isLocal)
{
        mIsLocal = isLocal;
}

/* ============================ ACCESSORS ================================= */
// Return the event identifier.
PtStatus PtEvent::getId(PtEventId& rId)
{
        /* algorithms:
        PT_SUCCESS if success
        PT_PROVIDER_UNAVALAIBLE if provider unavailable
        */

        rId = mEventId;

        if (mEventId == PROVIDER_SHUTDOWN || mEventId == PROVIDER_OUT_OF_SERVICE)
                return PT_PROVIDER_UNAVAILABLE;
        else
                return PT_SUCCESS;
}


PtStatus PtEvent::getCause(PtEventCause& rCause)
{
        rCause = mEventCause;

        return PT_SUCCESS;
}

PtStatus PtEvent::getMetaCode(int& rMetaCode)
{
        rMetaCode = mMetaCode;
        return PT_SUCCESS;
}

PtStatus PtEvent::getMetaEvent(PtBoolean& rMetaEventExists, PtMetaEvent*& pEvent) const
{
        PtEventId eventId;
        rMetaEventExists = TRUE;

        switch (mMetaCode)
        {
                case META_CALL_STARTING:
                        eventId = CALL_META_CALL_STARTING_STARTED;
                        break;

                case META_CALL_PROGRESS:
                        eventId = SINGLECALL_META_PROGRESS_STARTED;
                        break;

                case META_CALL_ADDITIONAL_PARTY:
                        eventId = CALL_META_ADD_PARTY_STARTED;
                        break;

                case META_CALL_REMOVING_PARTY:
                        eventId = CALL_META_REMOVE_PARTY_STARTED;
                        break;

                case META_CALL_ENDING:
                        eventId = CALL_META_CALL_ENDING_STARTED;
                        break;

                case META_CALL_MERGING:
                        eventId = MULTICALL_META_MERGE_STARTED;
                        break;

                case META_CALL_TRANSFERRING:
                        eventId = MULTICALL_META_TRANSFER_STARTED;
                        break;

                case META_SNAPSHOT:
                        eventId = SINGLECALL_META_SNAPSHOT_STARTED;
                        break;

        case META_CALL_REPLACING:
            eventId = MULTICALL_META_REPLACE_STARTED;
            break;

                default:
                case META_UNKNOWN:
                case META_EVENT_NONE:
                        rMetaEventExists = FALSE;
                        return PT_SUCCESS;
        }

        if (eventId < SINGLECALL_META_PROGRESS_STARTED)
        {       // MultiCallMetaEvent
                pEvent = (PtMetaEvent*) new PtMultiCallMetaEvent(eventId,
                                                                                        mMetaCode,
                                                                                        mpClient,
                                                                                        mSipResponseCode,
                                                                                        mSipResponseText,
                                                                                        mCallId.data(),
                                                                                        mNewCallId.data(),
                                                                                        (const char**) mOldCallIds,
                                                                                        mNumOldCalls);
        }
        else
        {
                pEvent = (PtMetaEvent *) new PtSingleCallMetaEvent(eventId,
                                                                                                                        mMetaCode,
                                                                                                                        mpClient,
                                                                                                                        mSipResponseCode,
                                                                                                                        mSipResponseText,
                                                                                                                        mCallId.data());
        }

        return PT_SUCCESS;
}

PtStatus PtEvent::getSipResponseCode(int& responseCode, UtlString& responseText)
{
        responseCode = mSipResponseCode;
        responseText = mSipResponseText;
        return PT_SUCCESS;
}

/* ============================ INQUIRY =================================== */
int PtEvent::isLocal() const
{
   return mIsLocal;
}

PtBoolean PtEvent::isCallEvent(int eventId)
{
        int id1 = (int)CALL_ACTIVE;
        int id2 = (int)CALL_EVENT_TRANSMISSION_ENDED;

        int id3 = (int)MULTICALL_META_MERGE_STARTED;
        int id4 = (int)CALL_META_REMOVE_PARTY_ENDED;

        PtBoolean isEvent = ((eventId >= id1) && (eventId <= id2)) ||
                                                ((eventId >= id3) && (eventId <= id4));
        return isEvent;
}

PtBoolean PtEvent::isConnectionEvent(int eventId)
{
        int id1 = (int)CONNECTION_ALERTING;
        int id2 = (int)CONNECTION_UNKNOWN;

        int id3 = (int)CONNECTION_DIALING;
        int id4 = (int)CONNECTION_QUEUED;

        PtBoolean isEvent = ((eventId >= id1) && (eventId <= id2)) ||
                                                ((eventId >= id3) && (eventId <= id4));
        return isEvent;
}

PtBoolean PtEvent::isTerminalEvent(int eventId)
{
        return (eventId == TERMINAL_EVENT_TRANSMISSION_ENDED);
}

PtBoolean PtEvent::isTerminalComponentEvent(int eventId)
{
        int id1 = (int)PHONE_BUTTON_INFO_CHANGED;
        int id2 = (int)PHONE_RINGER_INFO_CHANGED;

        PtBoolean isEvent = ((eventId >= id1) && (eventId <= id2));
        return isEvent;
}

PtBoolean PtEvent::isTerminalConnectionEvent(int eventId)
{
        int id1 = (int)TERMINAL_CONNECTION_ACTIVE;
        int id2 = (int)TERMINAL_CONNECTION_UNKNOWN;

        int id3 = (int)TERMINAL_CONNECTION_BRIDGED;
        int id4 = (int)TERMINAL_CONNECTION_IDLE;

        PtBoolean isEvent = ((eventId >= id1) && (eventId <= id2)) ||
                                                ((eventId >= id3) && (eventId <= id4));
        return isEvent;
}

PT_IMPLEMENT_CLASS_INFO(PtEvent, PT_NO_PARENT_CLASS)

// To see if the 2 events are the same.
PtBoolean PtEvent::isSame(const PtEvent& rEvent)
{
        return  (mMetaCode == rEvent.mMetaCode &&
                                 mEventId == rEvent.mEventId &&
                                 mEventCause == rEvent.mEventCause);
}

PtBoolean PtEvent::isStateTransitionAllowed(int newState, int oldState)
{
        PtBoolean isAllowed = TRUE;

        if (newState == oldState)
                return FALSE;

        switch (oldState)
        {
        case CONNECTION_CREATED:
                if (newState == CONNECTION_NETWORK_ALERTING ||
                        newState == CONNECTION_DISCONNECTED)
                {
                        isAllowed = FALSE;
                }
                break;
        case CONNECTION_QUEUED:
        case CONNECTION_OFFERED:
                if (newState != CONNECTION_ALERTING &&
                        newState != CONNECTION_ESTABLISHED &&
                        newState != CONNECTION_DISCONNECTED &&
                        newState != CONNECTION_FAILED &&
                        newState != CONNECTION_UNKNOWN)
                {
                        isAllowed = FALSE;
                }
                break;
        case CONNECTION_ALERTING:
                if (newState != CONNECTION_ESTABLISHED &&
                        newState != CONNECTION_DISCONNECTED &&
                        newState != CONNECTION_FAILED &&
                        newState != CONNECTION_UNKNOWN)
                {
                        isAllowed = FALSE;
                }
                break;
        case CONNECTION_ESTABLISHED:
                if (newState != CONNECTION_DISCONNECTED &&
                        newState != CONNECTION_FAILED &&
                        newState != CONNECTION_UNKNOWN)
                {
                        isAllowed = FALSE;
                }
                break;
        case CONNECTION_FAILED:
                if (newState != CONNECTION_DISCONNECTED &&
                        newState != CONNECTION_UNKNOWN)
                {
                        isAllowed = FALSE;
                }
                break;
        case CONNECTION_DISCONNECTED:
                if (newState != CONNECTION_UNKNOWN)
                {
                        isAllowed = FALSE;
                }
                break;
    case CONNECTION_INITIATED:
                if (newState != CONNECTION_DIALING &&
                        newState != CONNECTION_ESTABLISHED &&
                        newState != CONNECTION_DISCONNECTED &&
                        newState != CONNECTION_FAILED &&
                        newState != CONNECTION_UNKNOWN)
                {
                        isAllowed = FALSE;
                }
                break;
    case CONNECTION_DIALING:
                if (newState != CONNECTION_ESTABLISHED &&
                        newState != CONNECTION_DISCONNECTED &&
                        newState != CONNECTION_FAILED &&
                        newState != CONNECTION_UNKNOWN)
                {
                        isAllowed = FALSE;
                }
                break;
    case CONNECTION_NETWORK_REACHED:
                if (newState != CONNECTION_NETWORK_ALERTING &&
                        newState != CONNECTION_ESTABLISHED &&
                        newState != CONNECTION_DISCONNECTED &&
                        newState != CONNECTION_FAILED &&
                        newState != CONNECTION_UNKNOWN)
                {
                        isAllowed = FALSE;
                }
                break;
    case CONNECTION_NETWORK_ALERTING:
                if (newState != CONNECTION_ESTABLISHED &&
                        newState != CONNECTION_DISCONNECTED &&
                        newState != CONNECTION_FAILED &&
                        newState != CONNECTION_UNKNOWN)
                {
                        isAllowed = FALSE;
                }
                break;
        default:
    case CONNECTION_UNKNOWN:
                break;
        }

        return isAllowed;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
// protected constructor.
//PtEvent::PtEvent(PtEventId eventId, PtEventCause cause)
//{
//      mEventId = eventId;
//      mEventCause = cause;
//}
/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
