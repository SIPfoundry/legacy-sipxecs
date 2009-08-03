//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#include "EventValidator.h"
#include "tapi/sipXtapi.h"
#include "tapi/sipXtapiEvents.h"
#include "tapi/sipXtapiInternal.h"
#include "os/OsBSem.h"
#include "os/OsMutex.h"
#include "os/OsLock.h"
#include "utl/UtlSList.h"

EventValidator::EventValidator(const char* szTitle)
    : m_semUnprocessed(OsBSem::Q_FIFO, OsBSem::EMPTY)
    , m_mutLists(OsMutex::Q_FIFO)
{
    m_title = szTitle ;
    m_pUnfoundEvent = NULL ;
    reset() ;
}


EventValidator::~EventValidator()
{
    reset() ;
}


void EventValidator::ignoreEventCategory(SIPX_EVENT_CATEGORY category)
{
    assert((category >= 0) && (category < MAX_EVENT_CATEGORIES)) ;

    m_filterCategories[category] = true ;
}


bool EventValidator::isIgnoredCateogry(SIPX_EVENT_CATEGORY category)
{
    assert((category >= 0) && (category < MAX_EVENT_CATEGORIES)) ;

    return m_filterCategories[category] ;

}


void EventValidator::setDefaultTimeout(int iTimeoutInSecs)
{
    assert(iTimeoutInSecs >= 0) ;
    m_iDefaultTimeoutInSecs = iTimeoutInSecs ;
}


void EventValidator::setMaxLookhead(int iMaxLookAhead)
{
    assert(iMaxLookAhead >= 0) ;
    m_iMaxLookAhead = iMaxLookAhead ;
}


void EventValidator::reset()
{
    OsLock lock(m_mutLists) ;

    m_iDefaultTimeoutInSecs = 45 ;
    m_iMaxLookAhead = 4 ;

    for (int i=0; i<MAX_EVENT_CATEGORIES; i++)
    {
        m_filterCategories[i] = false ;
    }

    if (m_pUnfoundEvent)
    {
        delete m_pUnfoundEvent ;
    }

    while (!m_unprocessedEvents.isEmpty())
    {
        UtlString* pString = (UtlString*) m_unprocessedEvents.at(0) ;
        m_unprocessedEvents.removeAt(0) ;
        delete pString ;
    }

    while (!m_processedEvents.isEmpty())
    {
        UtlString* pString = (UtlString*) m_processedEvents.at(0) ;
        m_processedEvents.removeAt(0) ;
        delete pString ;
    }

    m_eErrorType = EVENT_VALIDATOR_ERROR_NONE ;
}


bool EventValidator::waitForCallEvent(SIPX_LINE hLine,
                                      SIPX_CALL hCall,
                                      SIPX_CALLSTATE_EVENT event,
                                      SIPX_CALLSTATE_CAUSE cause,
                                      bool bStrictOrderMatch,
                                      int iTimeoutInSecs)
{
    bool bFound = true ;

    if (!isIgnoredCateogry(EVENT_CATEGORY_CALLSTATE))
    {

        UtlString* pString = allocCallStateEntry(hCall,
                hLine,
                event,
                cause) ;

        bFound = waitForEvent(pString->data(), bStrictOrderMatch, iTimeoutInSecs) ;
        delete pString ;
    }

    if (!bFound)
    {

        // Wait a second for any additional events to pour in -- useful for
        // debugging.
        OsTask::delay(1000) ;
        report() ;
    }

    return bFound ;
}


bool EventValidator::waitForMessage(SIPX_LINE hLine,
                                    const char* szMsg,
                                    bool bStrictOrderMatch,
                                    int iTimeoutInSecs)
{
    bool bFound ;

    UtlString* pString = allocMessageEvent(hLine, szMsg) ;
    bFound = waitForEvent(pString->data(), bStrictOrderMatch, iTimeoutInSecs) ;

    delete pString ;

    if (!bFound)
    {
        // Wait a second for any additional events to pour in -- useful for
        // debugging.
        OsTask::delay(1000) ;

        report() ;
    }


    return bFound ;
}


bool EventValidator::waitForLineEvent(SIPX_LINE hLine,
                                      SIPX_LINESTATE_EVENT event,
                                      SIPX_LINESTATE_CAUSE cause,
                                      bool bStrictOrderMatch,
                                      int iTimeoutInSecs)
{
    bool bFound = true ;

    if (!isIgnoredCateogry(EVENT_CATEGORY_LINESTATE))
    {

        UtlString* pString = allocLineStateEntry(hLine,
                event,
                cause) ;
        bFound = waitForEvent(pString->data(), bStrictOrderMatch, iTimeoutInSecs) ;

        delete pString ;
    }

    if (!bFound)
    {
        // Wait a second for any additional events to pour in -- useful for
        // debugging.
        OsTask::delay(1000) ;

        report() ;
    }


    return bFound ;

}


bool EventValidator::waitForInfoStatusEvent(SIPX_INFO hInfo,
                                            int status,
                                            int responseCode,
                                            const char* szResponseText,
                                            bool bStrictOrderMatch,
                                            int iTimeoutInSecs)
{
    bool bFound = true ;

    if (!isIgnoredCateogry(EVENT_CATEGORY_INFO_STATUS))
    {

        UtlString* pString = allocInfoStatusEvent(hInfo,
                status,
                responseCode,
                szResponseText) ;
        bFound = waitForEvent(pString->data(), bStrictOrderMatch, iTimeoutInSecs) ;

        delete pString ;
    }

    if (!bFound)
    {
        // Wait a second for any additional events to pour in -- useful for
        // debugging.
        OsTask::delay(1000) ;

        report() ;
    }


    return bFound ;
}


bool EventValidator::waitForInfoEvent(SIPX_CALL hCall,
                                      SIPX_LINE hLine,
                                      const char* szFromURL,
                                      const char* szUserAgent,
                                      const char* szContentType,
                                      const char* szContent,
                                      int nContentLength,
                                      bool bStrictOrderMatch,
                                      int iTimeoutInSecs)
{
    bool bFound = true ;

    if (!isIgnoredCateogry(EVENT_CATEGORY_INFO))
    {
        UtlString* pString = allocInfoEvent(hCall,
                hLine,
                szFromURL,
                szUserAgent,
                szContentType,
                szContent,
                nContentLength) ;
        bFound = waitForEvent(pString->data(), bStrictOrderMatch, iTimeoutInSecs) ;

        delete pString ;
    }

    if (!bFound)
    {
        // Wait a second for any additional events to pour in -- useful for
        // debugging.
        OsTask::delay(1000) ;

        report() ;
    }

    return bFound ;
}


bool EventValidator::waitForConfigEvent(SIPX_CONFIG_EVENT event,
                                        bool bStrictOrderMatch,
                                        int iTimeoutInSecs)
{
    bool bFound = true ;

    if (!isIgnoredCateogry(EVENT_CATEGORY_CONFIG))
    {
        UtlString* pString = allocConfigEvent(event) ;
        bFound = waitForEvent(pString->data(), bStrictOrderMatch, iTimeoutInSecs) ;

        delete pString ;
    }

    if (!bFound)
    {
        // Wait a second for any additional events to pour in -- useful for
        // debugging.
        OsTask::delay(1000) ;

        report() ;
    }

    return bFound ;
}


bool EventValidator::hasUnprocessedEvents()
{
    bool bRC = false ;

    OsLock lock(m_mutLists) ;

    bRC = !m_unprocessedEvents.isEmpty() ;


    return bRC ;
}

bool EventValidator::validateNoWaitingEvent()
{
    bool bWaitingEvents = hasUnprocessedEvents() ;
    if (bWaitingEvents)
    {
        m_eErrorType = EVENT_VALIDATOR_ERROR_UNEXPECTED ;
        report() ;
    }
    return bWaitingEvents ;
}


const char* EventValidator::getTitle()
{
    return m_title.data();
}


void EventValidator::report()
{
    int nElements, i ;
    OsLock lock(m_mutLists) ;

    printf("\r\n") ;
    printf("%s EventValidator Error: ", m_title.data()) ;
    switch (m_eErrorType)
    {
        case EVENT_VALIDATOR_ERROR_NONE:
            printf("NONE\r\n") ;
            break ;
        case EVENT_VALIDATOR_ERROR_TIMEOUT:
            printf("EVENT TIMEOUT\r\n") ;
            break ;
        case EVENT_VALIDATOR_ERROR_MISMATCH:
            printf("EVENT MISMATCH\r\n") ;
            break ;
        case EVENT_VALIDATOR_ERROR_UNEXPECTED:
            printf("EVENT UNEXPECTED\r\n") ;
            break ;
    }

    nElements = m_processedEvents.entries() ;
    for (i=0; i<nElements; i++)
    {
        UtlString* pString = (UtlString*) m_processedEvents.at(i) ;
        printf("[OK] %s\r\n", pString->data()) ;
    }

    if (m_pUnfoundEvent)
    {
        printf("[!!] %s\r\n", m_pUnfoundEvent->data()) ;
    }

    nElements = m_unprocessedEvents.entries() ;
    for (i=0; i<nElements; i++)
    {
        UtlString* pString = (UtlString*) m_unprocessedEvents.at(i) ;
        printf("[??] %s\r\n", pString->data()) ;
    }

    printf("\r\n") ;
}


void EventValidator::addEvent(SIPX_EVENT_CATEGORY category, void* pInfo)
{
    assert((category >= 0) && (category < MAX_EVENT_CATEGORIES)) ;

    if (!isIgnoredCateogry(category))
    {
        OsLock lock(m_mutLists) ;

        switch (category)
        {
            case EVENT_CATEGORY_CALLSTATE:
                {
                    SIPX_CALLSTATE_INFO* pStateInfo = (SIPX_CALLSTATE_INFO*) pInfo ;

                    UtlString* pString = allocCallStateEntry(pStateInfo->hCall,
                            pStateInfo->hLine,
                            pStateInfo->event,
                            pStateInfo->cause) ;

                    m_unprocessedEvents.append(pString) ;
                    m_semUnprocessed.release() ;
                }
                break ;
            case EVENT_CATEGORY_LINESTATE:
                {
                    SIPX_LINESTATE_INFO* pStateInfo = (SIPX_LINESTATE_INFO*) pInfo ;

                    UtlString* pString = allocLineStateEntry(pStateInfo->hLine,
                            pStateInfo->event,
                            pStateInfo->cause) ;

                    m_unprocessedEvents.append(pString) ;
                    m_semUnprocessed.release() ;
                }
                break ;
            case EVENT_CATEGORY_INFO_STATUS:
                {
                    SIPX_INFOSTATUS_INFO* pStateInfo = (SIPX_INFOSTATUS_INFO*) pInfo ;

                    UtlString* pString = allocInfoStatusEvent(pStateInfo->hInfo,
                            pStateInfo->status,
                            pStateInfo->responseCode,
                            pStateInfo->szResponseText) ;

                    m_unprocessedEvents.append(pString) ;
                    m_semUnprocessed.release() ;
                }
                break ;
            case EVENT_CATEGORY_INFO:
                {
                    SIPX_INFO_INFO* pStateInfo = (SIPX_INFO_INFO*) pInfo ;

                    UtlString* pString = allocInfoEvent(pStateInfo->hCall,
                            pStateInfo->hLine,
                            pStateInfo->szFromURL,
                            pStateInfo->szUserAgent,
                            pStateInfo->szContentType,
                            pStateInfo->pContent,
                            pStateInfo->nContentLength) ;

                    m_unprocessedEvents.append(pString) ;
                    m_semUnprocessed.release() ;
                }
                break ;
            case EVENT_CATEGORY_CONFIG:
                {
                    SIPX_CONFIG_INFO* pStateInfo = (SIPX_CONFIG_INFO*) pInfo ;

                    UtlString* pString = allocConfigEvent(pStateInfo->event) ;

                    m_unprocessedEvents.append(pString) ;
                    m_semUnprocessed.release() ;
                }
                break ;
            // TODO: Add support for EVENT_CATEGORY_SUB_STATUS
            // TODO: Add support for EVENT_CATEGORY_NOTIFY
        }
    }
}


void EventValidator::addMessage(SIPX_LINE hLine, const char* szMsg)
{
    OsLock lock(m_mutLists) ;
    m_unprocessedEvents.append(allocMessageEvent(hLine, szMsg)) ;
    m_semUnprocessed.release() ;
}

void EventValidator::addMarker(const char* szMarkerText)
{
    OsLock lock(m_mutLists) ;
    UtlString marker ;

    marker.append("@@@ ") ;
    marker.append(szMarkerText) ;
    marker.append(" @@@") ;

    m_processedEvents.append(new UtlString(marker)) ;
}


UtlString* EventValidator::allocCallStateEntry(SIPX_CALL hCall,
                                               SIPX_LINE hLine,
                                               SIPX_CALLSTATE_EVENT event,
                                               SIPX_CALLSTATE_CAUSE cause)
{
    char szBuffer[256] ;
    char szBuffer2[256];


    sipxCallEventToString(
            (SIPX_CALLSTATE_MAJOR)(int) event,
            (SIPX_CALLSTATE_MINOR)(int)cause,
            szBuffer,
            sizeof(szBuffer));

    sprintf(szBuffer2, "<CALL> hLine=%u, hCall=%u: %s",
            hLine,
            hCall,
            szBuffer);

    return new UtlString(szBuffer2) ;
}


UtlString* EventValidator::allocLineStateEntry(SIPX_LINE hLine,
                                              SIPX_LINESTATE_EVENT event,
                                              SIPX_LINESTATE_CAUSE cause)
{
    char szBuffer[256] ;
    char szBuffer2[256] ;

    sipxLineEventToString((SIPX_LINE_EVENT_TYPE_MAJOR)(int) event,
            (SIPX_LINE_EVENT_TYPE_MINOR)(int) cause,
            szBuffer,
            sizeof(szBuffer)) ;
    sprintf(szBuffer2, "<LINE> hLine=%u: %s", hLine, szBuffer);

    return new UtlString(szBuffer2) ;

}


UtlString* EventValidator::allocMessageEvent(SIPX_LINE hLine, const char* szMessage)
{
    char szBuffer[1024] ;

    sprintf(szBuffer, "<MSG> hLine=%d: %s",
            (int) hLine,
            szMessage ? szMessage : "") ;

    return new UtlString(szBuffer) ;

}


UtlString* EventValidator::allocInfoStatusEvent(SIPX_INFO hInfo, int status, int responseCode, const char* szResponseText)
{
    char szBuffer[1024] ;

    sprintf(szBuffer, "<INFO STATUS> hInfo=%" PRIdPTR ": status=%d, responseCode=%d, responseText=%s",
            hInfo,
            status,
            responseCode,
            szResponseText ? szResponseText : "") ;

    return new UtlString(szBuffer) ;
}

UtlString* EventValidator::allocInfoEvent(SIPX_CALL hCall,
                                          SIPX_LINE hLine,
                                          const char* szFromURL,
                                          const char* szUserAgent,
                                          const char* szContentType,
                                          const char* szContent,
                                          int nContentLength)
{
    char szBuffer[1024] ;

    // TODO:: Need way to validate from url -- not API to get it w/ tag today from
    // the calling side.  Stripping here.
    UtlString from(szFromURL ? szFromURL : "") ;
    ssize_t tagIndex = from.index(';') ;
    if (tagIndex >= 0)
    {
        from.remove(tagIndex) ;
    }

    sprintf(szBuffer, "<INFO> hCall=%u, hLine=%u, from=%s, szUserAgent=%s, type=%s, content=%s, len=%d",
            hCall,
            hLine,
            from.data(),
            szUserAgent ? szUserAgent : "",
            szContentType ? szContentType : "",
            szContent ? szContent : "",
            nContentLength) ;

    return new UtlString(szBuffer) ;
}


UtlString* EventValidator::allocConfigEvent(SIPX_CONFIG_EVENT hEvent)
{
    char szBuffer[1024] ;
    char szBuffer2[1024] ;

    sprintf(szBuffer, "<CONFIG> event=%s",
            sipxConfigEventToString(hEvent, szBuffer2, sizeof(szBuffer2))) ;

    return new UtlString(szBuffer) ;

}


bool EventValidator::findEvent(const char* szEvent, int nMaxLookAhead, int &nActualLookAhead)
{
    bool bFound = false ;

    m_mutLists.acquire() ;

    int nEntries = m_unprocessedEvents.entries() ;
    if (nEntries > 0)
    {
        // Figure out the max and actual look ahead capabilities
        nActualLookAhead = MIN(nEntries, nMaxLookAhead) ;

        // Try to find message
        for (int i=0; i<nActualLookAhead; i++)
        {
            UtlString* pString = (UtlString*) m_unprocessedEvents.at(i) ;
            assert(pString != NULL) ;
            if (pString->compareTo(szEvent) == 0)
            {
                m_unprocessedEvents.removeAt(i) ;
                m_processedEvents.append(pString) ;
                bFound = true ;
                break ;
            }
        }
    }
    else
    {
        nActualLookAhead = 0;
    }

    m_mutLists.release() ;

    return bFound ;
}


bool EventValidator::waitForEvent(const char* szEvent, bool bStrictOrderMatch, int iTimeoutInSecs)
{
    bool bFound = false ;
    OsTime waitTime(iTimeoutInSecs == DEFAULT_TIMEOUT ? m_iDefaultTimeoutInSecs : iTimeoutInSecs, 0) ;
    int nMaxLookAhead = bStrictOrderMatch ? 1 : m_iMaxLookAhead ;
    int nActualLookAhead ;
    bool bTimedOut = false ;

    bFound = findEvent(szEvent, nMaxLookAhead, nActualLookAhead) ;
    while (!bFound && !bTimedOut && (nActualLookAhead < nMaxLookAhead))
    {
        if (m_semUnprocessed.acquire(waitTime) != OS_SUCCESS)
        {
            bTimedOut = true ;
        }
        bFound = findEvent(szEvent, nMaxLookAhead, nActualLookAhead) ;
    }

    if (!bFound)
    {
        m_pUnfoundEvent = new UtlString(szEvent) ;
    }

    if (!bFound)
    {
        if (bTimedOut)
        {
            m_eErrorType = EVENT_VALIDATOR_ERROR_TIMEOUT ;
        }
        else
        {
            m_eErrorType = EVENT_VALIDATOR_ERROR_MISMATCH ;
        }
    }


    // A Bit of a hack: In some of the unit tests, we have a race between the event matcher
    // and autoXXXX handlers.
    OsTask::delay(20) ;

    return bFound ;
}
