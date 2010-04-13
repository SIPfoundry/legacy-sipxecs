//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#include "EventRecorder.h"

EventRecorder::EventRecorder()
{
    int i ;

    // Init events
    for (i=0;i<MAX_EVENTS;i++)
    {
        m_events[i] = NULL ;
    }
    m_numEvents = 0 ;

    // Init compare events
    for (i=0;i<MAX_EVENTS;i++)
    {
        m_compareEvents[i] = NULL ;
    }
    m_numCompareEvents = 0 ;
}


EventRecorder::~EventRecorder()
{
    clear() ;
}


void EventRecorder::clear()
{
    int i ;

    // Clear events
    for (i=0; i<MAX_EVENTS; i++)
    {
        if (m_events[i])
        {
            free(m_events[i]) ;
            m_events[i] = NULL ;
        }
    }
    m_numEvents = 0 ;

    // Clear compare events
    for (i=0; i<MAX_EVENTS; i++)
    {
        if (m_compareEvents[i])
        {
            free(m_compareEvents[i]) ;
            m_compareEvents[i] = NULL ;
        }
    }
    m_numCompareEvents = 0 ;

}


void EventRecorder::addEvent(SIPX_LINE hLine, SIPX_CALLSTATE_EVENT eMajor, SIPX_CALLSTATE_CAUSE eMinor)
{
    char szBuffer[256] ;
    char szBuffer2[256];

    sipxCallEventToString((SIPX_CALLSTATE_MAJOR)(int)eMajor,
        (SIPX_CALLSTATE_MINOR)(int)eMinor,
        szBuffer, sizeof(szBuffer));
    sprintf(szBuffer2, "hLine-%u: %s", hLine, szBuffer);
    m_events[m_numEvents++] = strdup(szBuffer2) ;
}

void EventRecorder::addCompareEvent(SIPX_LINE hLine, SIPX_CALLSTATE_EVENT eMajor, SIPX_CALLSTATE_CAUSE eMinor)
{
    char szBuffer[256] ;
    char szBuffer2[256];

    sipxCallEventToString((SIPX_CALLSTATE_MAJOR)(int)eMajor,
        (SIPX_CALLSTATE_MINOR)(int)eMinor,
        szBuffer, sizeof(szBuffer));
    sprintf(szBuffer2, "hLine-%u: %s", hLine, szBuffer);
    m_compareEvents[m_numCompareEvents++] = strdup(szBuffer2) ;
}

void EventRecorder::addEvent(SIPX_LINE hLine, SIPX_LINESTATE_EVENT event, SIPX_LINESTATE_CAUSE cause)
{
    char szBuffer[256] ;
    char szBuffer2[256] ;
    sipxLineEventToString((SIPX_LINE_EVENT_TYPE_MAJOR)(int)event,
                          (SIPX_LINE_EVENT_TYPE_MINOR)(int)cause,
                          szBuffer, sizeof(szBuffer));
    sprintf(szBuffer2, "hLine-%u: %s", hLine, szBuffer);
    m_events[m_numEvents++] = strdup(szBuffer2) ;
}

void EventRecorder::addCompareEvent(SIPX_LINE hLine, SIPX_LINESTATE_EVENT event, SIPX_LINESTATE_CAUSE cause)
{
    char szBuffer[256] ;
    char szBuffer2[256] ;
    sipxLineEventToString((SIPX_LINE_EVENT_TYPE_MAJOR)(int)event, (SIPX_LINE_EVENT_TYPE_MINOR)(int)cause,szBuffer, sizeof(szBuffer));
    sprintf(szBuffer2, "hLine-%u: %s", hLine, szBuffer);
    m_compareEvents[m_numCompareEvents++] = strdup(szBuffer2) ;
}

void EventRecorder::addEvent(SIPX_INFO_INFO* pInfoInfo)
{
    char szBuffer[1024] ;

    sprintf(szBuffer, "INFO MSG: %s, %zu", pInfoInfo->pContent, pInfoInfo->nContentLength);
    m_events[m_numEvents++] = strdup(szBuffer) ;
    return;
}

void EventRecorder::addCompareEvent(SIPX_INFO_INFO* pInfoInfo)
{
    char szBuffer[1024] ;

    sprintf(szBuffer, "INFO MSG: %s, %zu", pInfoInfo->pContent, pInfoInfo->nContentLength);
    m_compareEvents[m_numCompareEvents++] = strdup(szBuffer) ;
    return;
}

void EventRecorder::addEvent(SIPX_INFOSTATUS_INFO* pInfoStatus)
{
    char szBuffer[128] ;

    sprintf(szBuffer, "INFO STATUS: %d", pInfoStatus->responseCode);
    m_events[m_numEvents++] = strdup(szBuffer) ;
    return;
}

void EventRecorder::addCompareEvent(SIPX_INFOSTATUS_INFO* pInfoStatus)
{
    char szBuffer[128] ;

    sprintf(szBuffer, "INFO STATUS: %d", pInfoStatus->responseCode);
    m_compareEvents[m_numCompareEvents++] = strdup(szBuffer) ;
    return;
}

void EventRecorder::addEvent(SIPX_CONFIG_INFO* pConfigInfo)
{
    char cBuffer[128] ;

    sipxConfigEventToString(pConfigInfo->event, cBuffer, sizeof(cBuffer)) ;
    m_events[m_numEvents++] = strdup(cBuffer) ;
}

void EventRecorder::addCompareEvent(SIPX_CONFIG_INFO* pConfigInfo)
{
    char cBuffer[128] ;

    sipxConfigEventToString(pConfigInfo->event, cBuffer, sizeof(cBuffer)) ;
    m_compareEvents[m_numCompareEvents++] = strdup(cBuffer) ;
}

void EventRecorder::addMsgString(SIPX_LINE hLine, const char* szMsg)
{
    char szBuffer[256] ;

    sprintf(szBuffer, "hLine-%u: %s", hLine, szMsg);
    m_events[m_numEvents++] = strdup(szBuffer);
}

void EventRecorder::addCompareMsgString(SIPX_LINE hLine, const char* szMsg)
{
    char szBuffer[256] ;

    sprintf(szBuffer, "hLine-%u: %s", hLine, szMsg);
    m_compareEvents[m_numCompareEvents++] = strdup(szBuffer);
}

bool EventRecorder::compare()
{
    bool bMatch = false ;

    char* szRecorded = buildEventStr(m_events, m_numEvents) ;
    char* szCompare = buildEventStr(m_compareEvents, m_numCompareEvents) ;

    if (strcmp(szRecorded, szCompare) == 0)
    {
        bMatch = true ;
    }
    else
    {
        printf("\nverifyEvent Failed\n") ;
        printf("Recorded:\n%s\n", szRecorded) ;
        printf("Expected:\n%s\n", szCompare) ;
    }

    free(szRecorded) ;
    free(szCompare) ;

    return bMatch ;
}


bool EventRecorder::compareNoOrder()
{
    sortEvents();
    return compare();
}

char* EventRecorder::buildEventStr(char* array[], int nLength)
{
    char* szRC = NULL ;
    int iEvents = nLength ;
    int iSize, i ;

    // Pass 1: calc size
    iSize = 1 ; // For null
    for (i=0; i<iEvents; i++)
    {
        iSize += strlen(array[i]) + 2 ;  // "\t\n"
    }

    // Pass 2: Build String
    szRC = (char*) calloc(iSize, 1) ;
    for (i=0; i<iEvents; i++)
    {
        strcat(szRC, "\t") ;
        strcat(szRC, array[i]) ;
        if ((i+1) < iEvents)
        {
            strcat(szRC, "\n") ;
        }
    }

    return szRC ;
}


static int eventSortCompare(const void *str1, const void *str2)
{
    return strcmp(*(const char**)str1, *(const char **)str2);
}

void EventRecorder::sortEvents()
{
    qsort(m_events, m_numEvents, sizeof(char *), eventSortCompare);
    qsort(m_compareEvents, m_numCompareEvents, sizeof(char *), eventSortCompare);
}
