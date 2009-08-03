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
#if !defined(_WIN32)
#  include <stddef.h>
#endif
#include <assert.h>
// APPLICATION INCLUDES
#include "tapi/sipXtapiEvents.h"
#include "tapi/sipXtapiInternal.h"
#include "tapi/SipXHandleMap.h"
#include "utl/UtlSList.h"
#include "utl/UtlSListIterator.h"
#include "utl/UtlVoidPtr.h"
#include "os/OsMutex.h"
#include "utl/UtlString.h"
#include "os/OsLock.h"
#include "net/Url.h"
#include "utl/UtlHashMap.h"
#include "utl/UtlString.h"
#include "net/SipSession.h"
#include "cp/CallManager.h"

// DEFINES
#ifdef WIN32
#define SNPRINTF _snprintf
#else
#define SNPRINTF snprintf
#endif

// #define DEBUG_SIPXTAPI_EVENTS
//#define DUMP_CALLS              1

// GLOBAL VARIABLES
UtlSList*   g_pListeners = new UtlSList();
OsMutex*    g_pListenerLock = new OsMutex(OsMutex::Q_FIFO) ;
UtlSList*   g_pLineListeners = new UtlSList();
OsMutex*    g_pLineListenerLock = new OsMutex(OsMutex::Q_FIFO) ;
UtlSList*   g_pEventListeners = new UtlSList();
OsMutex*    g_pEventListenerLock = new OsMutex(OsMutex::Q_FIFO) ;
UtlSList*   g_pDeadLineList = new UtlSList();


// EXTERNAL VARIABLES
extern SipXHandleMap* gpCallHandleMap ;   // sipXtapiInternal.cpp

// EXTERNAL FUNCTIONS
// STRUCTURES

// FUNCTION DECLARATIONS
static const char* MajorLineEventToString(SIPX_LINE_EVENT_TYPE_MAJOR eMajor);
static const char* MinorLineEventToString(SIPX_LINE_EVENT_TYPE_MINOR eMinor);

/* ============================ FUNCTIONS ================================= */

static const char* convertEventCategoryToString(SIPX_EVENT_CATEGORY category)
{
    const char* str = "Unknown" ;

    switch (category)
    {
        case EVENT_CATEGORY_CALLSTATE:
            str = "EVENT_CATEGORY_CALLSTATE" ;
            break ;
        case EVENT_CATEGORY_LINESTATE:
            str = "EVENT_CATEGORY_LINESTATE" ;
            break ;
        case EVENT_CATEGORY_INFO_STATUS:
            str = "EVENT_CATEGORY_INFO_STATUS" ;
            break ;
        case EVENT_CATEGORY_INFO:
            str = "EVENT_CATEGORY_INFO" ;
            break ;
        case EVENT_CATEGORY_SUB_STATUS:
            str = "EVENT_CATEGORY_SUB_STATUS" ;
            break ;
        case EVENT_CATEGORY_NOTIFY:
            str = "EVENT_CATEGORY_NOTIFY" ;
            break ;
        case EVENT_CATEGORY_CONFIG:
            str = "EVENT_CATEGORY_CONFIG" ;
            break ;
    }

    return str ;
}

static const char* MajorEventToString(SIPX_CALLSTATE_MAJOR eMajor)
{
    const char* str = "Unknown" ;
    switch (eMajor)
    {
        case CALLSTATE_UNKNOWN:
            str = "UNKNOWN" ;
            break ;
        case NEWCALL:
                str = "NEWCALL" ;
                break ;
        case DIALTONE:
                str = "DIALTONE" ;
                break ;
        case REMOTE_OFFERING:
                str = "REMOTE_OFFERING" ;
                break ;
        case REMOTE_ALERTING:
                str = "REMOTE_ALERTING" ;
                break ;
        case CONNECTED:
                str = "CONNECTED" ;
                break ;
        case DISCONNECTED:
                str = "DISCONNECTED" ;
                break ;
        case OFFERING:
                str = "OFFERING" ;
                break ;
        case ALERTING:
                str = "ALERTING" ;
                break ;
        case DESTROYED:
                str = "DESTROYED" ;
                break;
        case AUDIO_EVENT:
                str = "AUDIO_EVENT" ;
                break ;
        case TRANSFER:
                str = "TRANSFER" ;
                break ;
    }
   return str;
}

static const char* MinorEventToString(SIPX_CALLSTATE_MINOR eMinor)
{
    const char* str = "Unknown" ;
    switch (eMinor)
    {
        case NEW_CALL_NORMAL:
                str = "NEW_CALL_NORMAL" ;
                break ;
        case NEW_CALL_TRANSFERRED:
                str = "NEW_CALL_TRANSFERRED" ;
                break ;
        case NEW_CALL_TRANSFER:
                str = "NEW_CALL_TRANSFER" ;
                break ;
        case DIALTONE_UNKNOWN:
                str = "DIALTONE_UNKNOWN" ;
                break ;
        case DIALTONE_CONFERENCE:
                str = "DIALTONE_CONFERENCE" ;
                break ;
        case REMOTE_OFFERING_NORMAL:
                str = "REMOTE_OFFERING_NORMAL" ;
                break ;
        case REMOTE_ALERTING_NORMAL:
                str = "REMOTE_ALERTING_NORMAL" ;
                break ;
        case REMOTE_ALERTING_MEDIA:
                str = "REMOTE_ALERTING_MEDIA" ;
                break ;
        case CONNECTED_ACTIVE:
                str = "CONNECTED_ACTIVE" ;
                break ;
        case CONNECTED_ACTIVE_HELD:
                str = "CONNECTED_ACTIVE_HELD" ;
                break ;
        case CONNECTED_INACTIVE:
                str = "CONNECTED_INACTIVE" ;
                break ;
        case DISCONNECTED_BADADDRESS:
                str = "DISCONNECTED_BADADDRESS" ;
                break ;
        case DISCONNECTED_BUSY:
                str = "DISCONNECTED_BUSY" ;
                break ;
        case DISCONNECTED_NORMAL:
                str = "DISCONNECTED_NORMAL" ;
                break ;
        case DISCONNECTED_RESOURCES:
                str = "DISCONNECTED_RESOURCES" ;
                break ;
        case DISCONNECTED_NETWORK:
                str = "DISCONNECTED_NETWORK" ;
                break ;
        case DISCONNECTED_REDIRECTED:
                str = "DISCONNECTED_REDIRECTED" ;
                break ;
        case DISCONNECTED_NO_RESPONSE:
                str = "DISCONNECTED_NO_RESPONSE" ;
                break ;
        case DISCONNECTED_AUTH:
                str = "DISCONNECTED_AUTH" ;
                break ;
        case DISCONNECTED_UNKNOWN:
                str = "DISCONNECTED_UNKNOWN" ;
                break ;
        case OFFERING_ACTIVE:
                str = "OFFERING_ACTIVE" ;
                break ;
        case ALERTING_NORMAL:
                str = "ALERTING_NORMAL" ;
                break ;
        case DESTROYED_NORMAL:
                str = "DESTROYED_NORMAL" ;
                break ;
        case AUDIO_START:
                str = "AUDIO_START";
                break;
        case AUDIO_STOP:
                str = "AUDIO_STOP";
                break;
        case TRANSFER_INITIATED:
                str = "TRANSFER_INITIATED";
                break;
        case TRANSFER_ACCEPTED:
                str = "TRANSFER_ACCEPTED";
                break;
        case TRANSFER_TRYING:
                str = "TRANSFER_TRYING";
                break;
        case TRANSFER_RINGING:
                str = "TRANSFER_RINGING";
                break;
        case TRANSFER_SUCCESS:
                str = "TRANSFER_SUCCESS";
                break;
        case TRANSFER_FAILURE:
                str = "TRANSFER_FAILURE";
                break;
    }
    return str;
}


static const char* convertInfoStatusEventToString(SIPX_INFOSTATUS_EVENT event)
{
    const char* str = "Unknown" ;

    switch (event)
    {
        case INFOSTATUS_UNKNOWN:
            str = "INFOSTATUS_UNKNOWN" ;
            break ;
        case INFOSTATUS_RESPONSE:
            str = "INFOSTATUS_RESPONSE" ;
            break ;
        case INFOSTATUS_NETWORK_ERROR:
            str = "INFOSTATUS_NETWORK_ERROR" ;
            break;
    }

    return str ;
}

static const char* convertMessageStatusToString(SIPX_MESSAGE_STATUS status)
{
    const char* str = "Unknown" ;

    switch (status)
    {
        case SIPX_MESSAGE_OK:
            str = "SIPX_MESSAGE_OK" ;
            break ;
        case SIPX_MESSAGE_FAILURE:
            str = "SIPX_MESSAGE_FAILURE" ;
            break ;
        case SIPX_MESSAGE_SERVER_FAILURE:
            str = "SIPX_MESSAGE_SERVER_FAILURE" ;
            break ;
        case SIPX_MESSAGE_GLOBAL_FAILURE:
            str = "SIPX_MESSAGE_GLOBAL_FAILURE" ;
            break ;
    }

    return str ;
}


static const char* convertConfigEventToString(SIPX_CONFIG_EVENT event)
{
    const char* str = "Unknown" ;

    switch (event)
    {
        case CONFIG_UNKNOWN:
            str = "CONFIG_UNKNOWN" ;
            break ;
        case CONFIG_STUN_SUCCESS:
            str = "CONFIG_STUN_SUCCESS" ;
            break ;
        case CONFIG_STUN_FAILURE:
            str = "CONFIG_STUN_FAILURE" ;
            break ;
    }

    return str ;
}

const char* convertSubscriptionStateToString(SIPX_SUBSCRIPTION_STATE state)
{
    const char* str = "Unknown" ;

    switch (state)
    {
        case SIPX_SUBSCRIPTION_PENDING:
            str = "SIPX_SUBSCRIPTION_PENDING" ;
            break ;
        case SIPX_SUBSCRIPTION_ACTIVE:
            str = "SIPX_SUBSCRIPTION_ACTIVE" ;
            break ;
        case SIPX_SUBSCRIPTION_FAILED:
            str = "SIPX_SUBSCRIPTION_FAILED" ;
            break ;
        case SIPX_SUBSCRIPTION_EXPIRED:
            str = "SIPX_SUBSCRIPTION_EXPIRED" ;
            break ;
    }

    return str ;
}

const char* convertSubscriptionCauseToString(SIPX_SUBSCRIPTION_CAUSE cause)
{
    const char* str = "Unknown" ;

    switch (cause)
    {
        case SUBSCRIPTION_CAUSE_UNKNOWN:
            str = "SUBSCRIPTION_CAUSE_UNKNOWN" ;
            break ;
        case SUBSCRIPTION_CAUSE_NORMAL:
            str = "SUBSCRIPTION_CAUSE_NORMAL" ;
            break ;
    }

    return str ;
}


SIPXTAPI_API char* sipxEventToString(const SIPX_EVENT_CATEGORY category,
                                     const void* pEvent,
                                     char*  szBuffer,
                                     size_t nBuffer)
{
    switch (category)
    {
        case EVENT_CATEGORY_CALLSTATE:
            {
                SIPX_CALLSTATE_INFO* pCallEvent = (SIPX_CALLSTATE_INFO*)pEvent;
                SNPRINTF(szBuffer, nBuffer, "%s::%s::%s",
                        convertEventCategoryToString(category),
                        MajorEventToString((SIPX_CALLSTATE_MAJOR) pCallEvent->event),
                        MinorEventToString((SIPX_CALLSTATE_MINOR) pCallEvent->cause)) ;
            }
            break;
        case EVENT_CATEGORY_LINESTATE:
            {
                SIPX_LINESTATE_INFO* pLineEvent = (SIPX_LINESTATE_INFO*)pEvent;
                SNPRINTF(szBuffer, nBuffer, "%s::%s::%s",
                        convertEventCategoryToString(category),
                        MajorLineEventToString((SIPX_LINE_EVENT_TYPE_MAJOR) pLineEvent->event),
                        MinorLineEventToString((SIPX_LINE_EVENT_TYPE_MINOR) pLineEvent->cause)) ;
            }
            break;
        case EVENT_CATEGORY_INFO_STATUS:
            {
                SIPX_INFOSTATUS_INFO* pInfoEvent = (SIPX_INFOSTATUS_INFO*)pEvent;
                SNPRINTF(szBuffer, nBuffer, "%s::%s::%s",
                        convertEventCategoryToString(category),
                        convertInfoStatusEventToString(pInfoEvent->event),
                        convertMessageStatusToString(pInfoEvent->status)) ;

            }
            break;
        case EVENT_CATEGORY_INFO:
            {
                SNPRINTF(szBuffer, nBuffer, "%s",
                        convertEventCategoryToString(category)) ;
            }
            break ;
        case EVENT_CATEGORY_SUB_STATUS:
            {
                SIPX_SUBSTATUS_INFO* pStatusInfo = (SIPX_SUBSTATUS_INFO*) pEvent ;
                SNPRINTF(szBuffer, nBuffer, "%s::%s::%s",
                        convertEventCategoryToString(category),
                        convertSubscriptionStateToString(pStatusInfo->state),
                        convertSubscriptionCauseToString(pStatusInfo->cause)) ;
            }
            break ;
        case EVENT_CATEGORY_NOTIFY:
            {
                SNPRINTF(szBuffer, nBuffer, "%s",
                        convertEventCategoryToString(category)) ;
            }
            break ;
        case EVENT_CATEGORY_CONFIG:
            {
                SIPX_CONFIG_INFO* pConfigEvent = (SIPX_CONFIG_INFO*)pEvent;
                SNPRINTF(szBuffer, nBuffer, "%s::%s",
                        convertEventCategoryToString(category),
                        convertConfigEventToString(pConfigEvent->event)) ;
            }
            break ;
        default:
            break;
    }
    return szBuffer ;
}

SIPXTAPI_API char* sipxCallEventToString(SIPX_CALLSTATE_MAJOR eMajor,
                                         SIPX_CALLSTATE_MINOR eMinor,
                                         char*  szBuffer,
                                         size_t nBuffer)
{
    assert(szBuffer != NULL) ;

    if (szBuffer)
    {
        SNPRINTF(szBuffer, nBuffer, "%s::%s",  MajorEventToString(eMajor), MinorEventToString(eMinor)) ;
    }

    return szBuffer ;
}

SIPXTAPI_API char* sipxLineEventToString(SIPX_LINE_EVENT_TYPE_MAJOR lineTypeMajor,
                                         SIPX_LINE_EVENT_TYPE_MINOR lineTypeMinor,
                                         char*  szBuffer,
                                         size_t nBuffer)
{
#ifdef WIN32
   _snprintf(szBuffer, nBuffer, "%s::%s", MajorLineEventToString(lineTypeMajor), MinorLineEventToString(lineTypeMinor));
#else
   snprintf(szBuffer, nBuffer, "%s::%s", MajorLineEventToString(lineTypeMajor), MinorLineEventToString(lineTypeMinor));
#endif

    return szBuffer;
}

void ReportCallback(SIPX_CALL hCall,
                                        SIPX_LINE hLine,
                                        SIPX_CALLSTATE_MAJOR eMajor,
                                        SIPX_CALLSTATE_MINOR eMinor,
                                        void* pUserData)
{
    SIPX_INSTANCE_DATA* pInst ;
    UtlString callId ;
    UtlString remoteAddress ;
    UtlString lineId ;
    static size_t nCnt = 0 ;

    if (sipxCallGetCommonData(hCall, &pInst, &callId, &remoteAddress, &lineId))
    {
        printf("<event i=%p, h=%04u, c=%4zu, M=%25s, m=%25s, a=%s, c=%s l=%s/>\n",
                pInst,
                hCall,
                ++nCnt,
                MajorEventToString(eMajor),
                MinorEventToString(eMinor),
                remoteAddress.data(),
                callId.data(),
                lineId.data()) ;
    }
}


void sipxFireCallEvent(const void* pSrc,
                       const char* szCallId,
                       SipSession* pSession,
                       const char* szRemoteAddress,
                       SIPX_CALLSTATE_MAJOR major,
                       SIPX_CALLSTATE_MINOR minor,
                       void* pEventData)
{
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
                  "sipxFireCallEvent "
                  "pSrc=%p callId=%s pSession=%p, "
                  "szRemoteAddress=%s major=%d minor=%d",
                  pSrc, szCallId, pSession, szRemoteAddress, major, minor);

    SIPX_CALL hCall = SIPX_CALL_NULL;

    {   // Scope for listener/event locks
        OsLock lock(*g_pListenerLock) ;
        OsLock eventLock(*g_pEventListenerLock) ;

        SIPX_CALL_DATA* pCallData = NULL;
        SIPX_LINE hLine = SIPX_LINE_NULL ;
        UtlVoidPtr* ptr = NULL;

        SIPX_INSTANCE_DATA* pInst = NULL;
        UtlString callId ;
        UtlString remoteAddress ;
        UtlString lineId ;
        SIPX_CALL hAssociatedCall = SIPX_CALL_NULL ;

        // If this is an NEW inbound call (first we are hearing of it), then create
        // a call handle/data structure for it.
        if (major == NEWCALL)
        {
            pCallData = new SIPX_CALL_DATA;
            memset((void*) pCallData, 0, sizeof(SIPX_CALL_DATA));

            pCallData->callId = new UtlString(szCallId) ;
            pCallData->remoteAddress = new UtlString(szRemoteAddress) ;
            pCallData->pMutex = new OsRWMutex(OsRWMutex::Q_FIFO) ;

            Url urlFrom;
            pSession->getFromUrl(urlFrom) ;

            pCallData->lineURI = new UtlString(urlFrom.toString()) ;
            pCallData->pInst = findSessionByCallManager(pSrc) ;

            hCall = gpCallHandleMap->allocHandle(pCallData) ;
            pInst = pCallData->pInst ;

            if (pEventData)     // for NEW CALL
            {
                char* szOriginalCallId = (char*) pEventData ;
                hAssociatedCall = sipxCallLookupHandle(UtlString(szOriginalCallId), pSrc) ;

                // Make sure we remove the call instead of allowing a drop.  When acting
                // as a transfer target, we are performing surgery on a CpPeerCall.  We
                // want to remove the call leg -- not drop the entire call.
                if ((hAssociatedCall) /* && (minor == CALLSTATE_NEW_CALL_TRANSFERRED)*/)
                {
                    sipxCallSetRemoveInsteadofDrop(hAssociatedCall) ;

                    /*
                    SIPX_CONF hConf = sipxCallGetConf(hAssociatedCall) ;
                    if (hConf)
                    {
                        sipxAddCallHandleToConf(hCall, hConf) ;
                    }
                    */
                }
            }

            // Increment call count
            pInst->pLock->acquire() ;
            pInst->nCalls++ ;
            pInst->pLock->release() ;

            callId = szCallId ;
            remoteAddress = szRemoteAddress ;
            lineId = urlFrom.toString() ;
        }   // end NEW CALL

        else    // call should exist
        {
            if (REMOTE_OFFERING == major)
            {
                if (pEventData)
                {
                    char* szOriginalCallId = (char*) pEventData ;
                    hAssociatedCall = sipxCallLookupHandle(UtlString(szOriginalCallId), pSrc) ;
                }
            }

            hCall = sipxCallLookupHandle(szCallId, pSrc);
            if (SIPX_CALL_NULL == hCall)
            {
                bool showWarning = TRUE;

                // Check for ghost trying to disconnect
                if (DISCONNECTED == major)
                {
                    if (pEventData)
                    {
                        // Assume event data means ghost, report that a tapi listener couldn't find the call
                        bool *pbReport = (bool *)pEventData;
                        *pbReport = TRUE;
                        OsSysLog::add(FAC_SIPXTAPI, PRI_DEBUG,
                                      "sipxFireCallEvent - "
                                      "No call found for (ghost) szCallId=%s, pSrc=%p",
                                      szCallId, pSrc);
                        showWarning = FALSE;
                    }
                }
                if (showWarning == TRUE)
                {
                    OsSysLog::add(FAC_SIPXTAPI, PRI_WARNING,
                                  "sipxFireCallEvent - "
                                  "No call found for szCallId=%s, pSrc=%p",
                                  szCallId, pSrc);
                }

#ifdef DUMP_CALLS
                sipxDumpCalls();
#endif
            }
            // Call exists from here on.
            else if (!sipxCallGetCommonData(hCall, &pInst, &callId, &remoteAddress, &lineId))
            {
                // When unable to get the common data, the call has been torn
                // down.  Since we are not holding a global call lock, it is
                // possible for the call to be torn down between getting the
                // handle lock and the common data.  At this point, there is
                // no need to continue processing.
                hCall = SIPX_CALL_NULL ;
            }
        }

        // Filter duplicate events
        UtlBoolean bDuplicateEvent = FALSE ;
        SIPX_CALLSTATE_EVENT lastEvent ;
        SIPX_CALLSTATE_CAUSE lastCause ;
        SIPX_INTERNAL_CALLSTATE state ;
        if (hCall != SIPX_CALL_NULL &&
            sipxCallGetState(hCall, lastEvent, lastCause, state))
        {
            if ((lastEvent == major) && (lastCause == minor))
            {
                bDuplicateEvent = TRUE ;
            }
        }


        // Only proceed if this isn't a duplicate event and we have a valid
        // call handle.
        if (!bDuplicateEvent && hCall != SIPX_CALL_NULL)
        {
            // Find Line
            UtlString requestUri;
            pSession->getRemoteRequestUri(requestUri);
            hLine = sipxLineLookupHandle(pInst, lineId.data(), requestUri.data()) ;

            OsSysLog::add(FAC_SIPXTAPI, PRI_DEBUG,
                          "sipxFireCallEvent "
                          "Line id = %s, hLine=%d, Request Uri = %s\n",
                          lineId.data(), hLine, requestUri.data());
            if (0 == hLine)
            {
                // no line exists for the lineId
                // log it
                OsSysLog::add(FAC_SIPXTAPI, PRI_WARNING,
                              "sipxFireCallEvent - "
                              "unknown line id = %s\n",
                              lineId.data());
            }

            // Fill in remote address
            if (szRemoteAddress)
            {
                pCallData = sipxCallLookup(hCall, SIPX_LOCK_WRITE) ;
                if (pCallData)
                {
                    if (pCallData->remoteAddress)
                    {
                        delete pCallData->remoteAddress;
                    }
                    pCallData->remoteAddress = new UtlString(szRemoteAddress) ;
                    sipxCallReleaseLock(pCallData, SIPX_LOCK_WRITE) ;
                }
            }


            // Report events to subscribe listeners
            UtlSListIterator itor(*g_pListeners) ;
            while ((ptr = (UtlVoidPtr*) itor()) != NULL)
            {
                CALL_LISTENER_DATA *pData = (CALL_LISTENER_DATA*) ptr->getValue() ;
                if (pData->pInst->pCallManager == pSrc)
                {
                    pData->pCallbackProc(hCall, hLine, major, minor, pData->pUserData) ;
                }
            }


            UtlSListIterator eventListenerItor(*g_pEventListeners);
            while ((ptr = (UtlVoidPtr*) eventListenerItor()) != NULL)
            {
                EVENT_LISTENER_DATA *pData = (EVENT_LISTENER_DATA*) ptr->getValue();
                if (pData->pInst->pCallManager == pSrc)
                {
                    SIPX_CALLSTATE_INFO callInfo;

                    memset((void*) &callInfo, 0, sizeof(SIPX_CALLSTATE_INFO));
                    callInfo.event = (SIPX_CALLSTATE_EVENT)(int)major;
                    callInfo.cause = (SIPX_CALLSTATE_CAUSE)(int)minor;
                    callInfo.hCall = hCall;
                    callInfo.hLine = hLine;
                    callInfo.hAssociatedCall = hAssociatedCall ;
                    callInfo.nSize = sizeof(SIPX_CALLSTATE_INFO);

                    if (szRemoteAddress) {
                        callInfo.remoteAddress = szRemoteAddress;
                    }

                    if (minor == CALLSTATE_AUDIO_START)
                    {
                        // Copy codec information into callInfo
                        memcpy((void*)&callInfo.codecs, pEventData, sizeof(SIPX_CODEC_INFO));
                    }

                    // callback signature:  typedef bool (*SIPX_EVENT_CALLBACK_PROC)(SIPX_EVENT_CATEGORY category, void* pInfo, void* pUserData);

                    pData->pCallbackProc(EVENT_CATEGORY_CALLSTATE, &callInfo, pData->pUserData);
                }
            }
            sipxCallSetState(hCall, (SIPX_CALLSTATE_EVENT) major, (SIPX_CALLSTATE_CAUSE) minor) ;
#ifdef DEBUG_SIPXTAPI_EVENTS
            ReportCallback(hCall, hLine, major, minor, NULL) ;
#endif

            // If this is a DESTROY message, free up resources after all listeners
            // have been notified.
            if (DESTROYED == major)
            {
                OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
                    "sipxFireCallEvent Free up the call object hCall=%u",
                    hCall);
                sipxCallObjectFree(hCall);
            }
        }
    }

    if (   (DISCONNECTED == major)
        && ( (sipxCallGetConf(hCall) != 0)
           || sipxCallIsRemoveInsteadOfDropSet(hCall)))
    {
        sipxFireCallEvent(pSrc, szCallId, pSession, szRemoteAddress,
                       DESTROYED,
                       DESTROYED_NORMAL,
                       pEventData) ;
    }
}


SIPXTAPI_API SIPX_RESULT sipxEventListenerAdd(const SIPX_INST hInst,
                                             SIPX_EVENT_CALLBACK_PROC pCallbackProc,
                                             void *pUserData)
{
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxEventListenerAdd hInst=%p pCallbackProc=%p pUserData=%p",
        hInst, pCallbackProc, pUserData);

    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS;
    OsLock lock(*g_pListenerLock) ;
    OsLock eventLock(*g_pEventListenerLock) ;
    OsLock lock2(*g_pLineListenerLock);

    if (hInst && pCallbackProc)
    {
        EVENT_LISTENER_DATA *pData = new EVENT_LISTENER_DATA;
        pData->pCallbackProc = pCallbackProc;
        pData->pUserData = pUserData;
        pData->pInst = (SIPX_INSTANCE_DATA*) hInst;

        g_pEventListeners->append(new UtlVoidPtr(pData));

        rc = SIPX_RESULT_SUCCESS;
    }

    return rc;
}

SIPXTAPI_API SIPX_RESULT sipxEventListenerRemove(const SIPX_INST hInst,
                                            SIPX_EVENT_CALLBACK_PROC pCallbackProc,
                                            void* pUserData)
{
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxEventListenerRemove hInst=%p pCallbackProc=%p pUserData=%p",
        hInst, pCallbackProc, pUserData);

    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS ;

    OsLock lock(*g_pListenerLock) ;
    OsLock eventLock(*g_pEventListenerLock) ;
    OsLock lock2(*g_pLineListenerLock);

    UtlVoidPtr* ptr ;

    if (hInst && pCallbackProc)
    {
        UtlSListIterator itor(*g_pEventListeners) ;
        while ((ptr = (UtlVoidPtr*) itor()) != NULL)
        {
            EVENT_LISTENER_DATA *pData = (EVENT_LISTENER_DATA*) ptr->getValue() ;
            if ((pData->pCallbackProc == pCallbackProc) &&
                (pData->pUserData == pUserData) &&
                (pData->pInst == (SIPX_INSTANCE_DATA*) hInst))
            {
                g_pEventListeners->removeReference(ptr) ;
                delete pData ;
                delete ptr ;

                rc = SIPX_RESULT_SUCCESS ;
                break ;
            }
        }
    }

    return rc ;
}

SIPXTAPI_API SIPX_RESULT sipxListenerAdd(const SIPX_INST hInst,
                                         CALLBACKPROC pCallbackProc,
                                         void* pUserData)
{
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxListenerAdd hInst=%p pCallbackProc=%p pUserData=%p",
        hInst, pCallbackProc, pUserData);

    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS ;

    OsLock lock(*g_pListenerLock) ;

    if (hInst && pCallbackProc)
    {
        CALL_LISTENER_DATA *pData = new CALL_LISTENER_DATA ;
        pData->pCallbackProc = pCallbackProc ;
        pData->pUserData = pUserData ;
        pData->pInst = (SIPX_INSTANCE_DATA*) hInst ;

        g_pListeners->append(new UtlVoidPtr(pData)) ;

        rc = SIPX_RESULT_SUCCESS ;
    }

    return rc;

}


SIPXTAPI_API SIPX_RESULT sipxListenerRemove(const SIPX_INST hInst,
                                            CALLBACKPROC pCallbackProc,
                                            void* pUserData)
{
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxListenerRemove hInst=%p pCallbackProc=%p pUserData=%p",
        hInst, pCallbackProc, pUserData);

    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS ;

    OsLock lock(*g_pListenerLock) ;

    UtlVoidPtr* ptr ;

    if (hInst && pCallbackProc)
    {
        UtlSListIterator itor(*g_pListeners) ;
        while ((ptr = (UtlVoidPtr*) itor()) != NULL)
        {
            CALL_LISTENER_DATA *pData = (CALL_LISTENER_DATA*) ptr->getValue() ;
            if ((pData->pCallbackProc == pCallbackProc) &&
                (pData->pUserData == pUserData) &&
                (pData->pInst == (SIPX_INSTANCE_DATA*) hInst))
            {
                            g_pListeners->removeReference(ptr) ;
                            delete pData ;
                            delete ptr ;

                rc = SIPX_RESULT_SUCCESS ;
                break ;
            }
        }
    }

    return rc ;
}


void sipxDumpListeners()
{
    OsLock lock(*g_pListenerLock) ;

    UtlVoidPtr* ptr ;

    printf("Dumping sipXtapi Listener List:\n") ;

    int count = 0 ;
    UtlSListIterator itor(*g_pListeners) ;
    while ((ptr = (UtlVoidPtr*) itor()) != NULL)
    {
        CALL_LISTENER_DATA *pData = (CALL_LISTENER_DATA*) ptr->getValue() ;

        printf("\tListener %02d: inst=%p proc=%p, data=%p\n",
                count, pData->pInst, pData->pCallbackProc, pData->pUserData) ;
        count++ ;
    }
}


static const char* MajorLineEventToString(SIPX_LINE_EVENT_TYPE_MAJOR eMajor)
{
    const char* str = "Unknown" ;

    switch (eMajor)
    {
        case SIPX_LINE_EVENT_REGISTERING:
            str = "REGISTERING";
            break;
        case SIPX_LINE_EVENT_REGISTERED:
                str = "REGISTERED";
                break ;
        case SIPX_LINE_EVENT_UNREGISTERING:
                str = "UNREGISTERING";
                break ;
        case SIPX_LINE_EVENT_UNREGISTERED:
                str = "UNREGISTERED";
                break ;
        case SIPX_LINE_EVENT_REGISTER_FAILED:
                str = "REGISTER_FAILED";
                break ;
        case SIPX_LINE_EVENT_PROVISIONED:
                str = "PROVISIONED";
                break ;
        case SIPX_LINE_EVENT_UNREGISTER_FAILED:
            str = "UNREGISTER_FAILED";
            break;
    }
    return str;
}


static const char* MinorLineEventToString(SIPX_LINE_EVENT_TYPE_MINOR eMinor)
{
    const char* str = "Unknown" ;

    switch (eMinor)
    {

        case LINE_EVENT_REGISTERING_NORMAL                      :
            str = "REGISTERING_NORMAL";
            break;
        case LINE_EVENT_REGISTERED_NORMAL                       :
            str = "REGISTERED_NORMAL";
            break;
        case LINE_EVENT_UNREGISTERING_NORMAL                    :
            str = "UNREGISTERING_NORMAL";
            break;
        case LINE_EVENT_UNREGISTERED_NORMAL                     :
            str = "UNREGISTERED_NORMAL";
            break;
        case LINE_EVENT_REGISTER_FAILED_COULD_NOT_CONNECT       :
            str = "COULD NOT CONNECT";
            break;
        case LINE_EVENT_REGISTER_FAILED_NOT_AUTHORIZED          :
            str = "NOT AUTHORIZED";
            break;
        case LINE_EVENT_REGISTER_FAILED_TIMEOUT                 :
            str = "TIMEOUT FAILURE";
            break;
        case LINE_EVENT_UNREGISTER_FAILED_COULD_NOT_CONNECT     :
            str = "COULD NOT CONNECT";
            break;
        case LINE_EVENT_UNREGISTER_FAILED_NOT_AUTHORIZED        :
            str = "NOT AUTHORIZED";
            break;
        case LINE_EVENT_UNREGISTER_FAILED_TIMEOUT               :
            str = "TIMEOUT FAILURE";
            break;
        case LINE_EVENT_PROVISIONED_NORMAL                      :
            str = "PROVISIONED_NORMAL";
            break;
        default:
            break;
    }
    return str;
}

SIPXTAPI_API SIPX_RESULT sipxLineListenerAdd(const SIPX_INST hInst,
                                         LINECALLBACKPROC pCallbackProc,
                                         void* pUserData)
{
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxLineListenerAdd hInst=%p pCallbackProc=%p pUserData=%p",
        hInst, pCallbackProc, pUserData);

   SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS ;

    OsLock lock(*g_pLineListenerLock) ;

    if (hInst && pCallbackProc)
    {
        LINE_LISTENER_DATA *pData = new LINE_LISTENER_DATA ;
        pData->pCallbackProc = pCallbackProc ;
        pData->pUserData = pUserData ;
        pData->pInst = (SIPX_INSTANCE_DATA*) hInst ;

        g_pLineListeners->append(new UtlVoidPtr(pData)) ;

        rc = SIPX_RESULT_SUCCESS ;
    }

    return rc;
}

SIPXTAPI_API SIPX_RESULT sipxLineListenerRemove(const SIPX_INST hInst,
                                            LINECALLBACKPROC pCallbackProc,
                                            void* pUserData)
{
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxLineListenerRemove hInst=%p pCallbackProc=%p pUserData=%p",
        hInst, pCallbackProc, pUserData);

    SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS ;

    OsLock lock(*g_pLineListenerLock) ;

    UtlVoidPtr* ptr ;

    if (hInst && pCallbackProc)
    {
        UtlSListIterator itor(*g_pLineListeners) ;
        while ((ptr = (UtlVoidPtr*) itor()) != NULL)
        {
            LINE_LISTENER_DATA *pData = (LINE_LISTENER_DATA*) ptr->getValue() ;
            if ((pData->pCallbackProc == pCallbackProc) &&
                    (pData->pUserData == pUserData) &&
                    (pData->pInst == (SIPX_INSTANCE_DATA*) hInst))
            {
                g_pLineListeners->removeReference(ptr) ;
                delete pData ;
                delete ptr ;

                rc = SIPX_RESULT_SUCCESS ;
                break ;
            }
        }
    }

    return rc ;
}


SIPXTAPI_API char* sipxConfigEventToString(SIPX_CONFIG_EVENT event,
                                           char* szBuffer,
                                           size_t nBuffer)
{
    switch (event)
    {
        case CONFIG_UNKNOWN:
            SNPRINTF(szBuffer, nBuffer, "CONFIG_UNKNOWN") ;
            break ;
        case CONFIG_STUN_SUCCESS:
            SNPRINTF(szBuffer, nBuffer, "CONFIG_STUN_SUCCESS") ;
            break ;
        case CONFIG_STUN_FAILURE:
            SNPRINTF(szBuffer, nBuffer, "CONFIG_STUN_FAILURE") ;
            break ;
        default:
            SNPRINTF(szBuffer, nBuffer, "ERROR -- UNKNOWN EVENT") ;
            assert(FALSE) ;
            break ;
    }

    return szBuffer;
}


void sipxFireLineEvent(const void* pSrc,
                       const char* szLineIdentifier,
                       SIPX_LINE_EVENT_TYPE_MAJOR major,
                       SIPX_LINE_EVENT_TYPE_MINOR minor)
{
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxFireLineEvent pSrc=%p szLineIdentifier=%s major=%d",
        pSrc, szLineIdentifier, major);

    OsLock lock(*g_pLineListenerLock);
    SIPX_LINE_DATA* pLineData = NULL;
    SIPX_LINE hLine = SIPX_LINE_NULL ;

    hLine = sipxLineLookupHandleByURI(findSessionByCallManager(pSrc), szLineIdentifier);
    pLineData = sipxLineLookup(hLine, SIPX_LOCK_READ) ;
    if (pLineData)
    {
        UtlVoidPtr* ptr;
        // Report events to subscribe listeners
        UtlSListIterator itor(*g_pLineListeners) ;
        while ((ptr = (UtlVoidPtr*) itor()) != NULL)
        {
            LINE_LISTENER_DATA *pData = (LINE_LISTENER_DATA*) ptr->getValue() ;
            if (pData->pInst->pRefreshManager == pSrc)
            {
                pLineData->pInst = pData->pInst ;
                pData->pCallbackProc(hLine, (SIPX_LINE_EVENT_TYPE_MAJOR)(int)major, pData->pUserData) ;
            }
        }
        UtlSListIterator eventListenerItor(*g_pEventListeners);
        while ((ptr = (UtlVoidPtr*) eventListenerItor()) != NULL)
        {
            EVENT_LISTENER_DATA *pData = (EVENT_LISTENER_DATA*) ptr->getValue();
            if (pData->pInst->pRefreshManager == pSrc)
            {
                SIPX_LINESTATE_INFO lineInfo;

                memset((void*) &lineInfo, 0, sizeof(SIPX_LINESTATE_INFO));
                lineInfo.event = (SIPX_LINESTATE_EVENT)(int)major;
                lineInfo.cause = (SIPX_LINESTATE_CAUSE)(int)minor;
                lineInfo.hLine = hLine;
                lineInfo.nSize = sizeof(SIPX_LINESTATE_INFO);

                // callback signature:  typedef bool (*SIPX_EVENT_CALLBACK_PROC)(SIPX_EVENT_CATEGORY category, void* pInfo, void* pUserData);
                pData->pCallbackProc(EVENT_CATEGORY_LINESTATE, &lineInfo, pData->pUserData);
            }
        }
        sipxLineReleaseLock(pLineData, SIPX_LOCK_READ) ;
    }

    if (SIPX_LINE_EVENT_UNREGISTERED == major)
    {
        sipxLineObjectFree(hLine);
    }
    if (SIPX_LINE_EVENT_UNREGISTER_FAILED == major &&
        LINE_EVENT_UNREGISTER_FAILED_COULD_NOT_CONNECT == minor)
    {
        g_pDeadLineList->insert(new UtlInt(hLine));
    }
}

void sipxFireEvent(const void* pSrc,
                   SIPX_EVENT_CATEGORY category,
                   void* pInfo)
{
    OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
        "sipxFireEvent pSrc=%p category=%d pInfo=%p",
        pSrc, category, pInfo);

    UtlSListIterator eventListenerItor(*g_pEventListeners);
    UtlVoidPtr* ptr;

    while ((ptr = (UtlVoidPtr*) eventListenerItor()) != NULL)
    {
        EVENT_LISTENER_DATA *pData = (EVENT_LISTENER_DATA*) ptr->getValue();
        if (pData->pInst->pCallManager == pSrc ||
            pData->pInst->pRefreshManager == pSrc ||
            pData->pInst->pLineManager == pSrc ||
            pData->pInst->pMessageObserver == pSrc)
        {
            pData->pCallbackProc(category, pInfo, pData->pUserData);
        }
    }
}
