//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
//////////////////////////////////////////////////////////////////////////////


#ifndef _EVENTRECORDER_H /* [ */
#define _EVENTRECORDER_H

#define MAX_EVENTS      1024    /**< Max number of events (something absurdly large) */

#include "tapi/sipXtapi.h"
#include "tapi/sipXtapiEvents.h"
#include "tapi/sipXtapiInternal.h"

class EventRecorder
{
protected:
    char* m_events[MAX_EVENTS] ;
    int   m_numEvents ;

    char* m_compareEvents[MAX_EVENTS] ;
    int   m_numCompareEvents ;


public:
    EventRecorder() ;
    ~EventRecorder() ;

    void clear() ;

    void addEvent(SIPX_LINE hLine, SIPX_CALLSTATE_EVENT eMajor, SIPX_CALLSTATE_CAUSE eMinor) ;
    void addCompareEvent(SIPX_LINE hLine, SIPX_CALLSTATE_EVENT eMajor, SIPX_CALLSTATE_CAUSE eMinor) ;

    void addEvent(SIPX_LINE hLine, SIPX_LINESTATE_EVENT event, SIPX_LINESTATE_CAUSE cause);
    void addCompareEvent(SIPX_LINE hLine, SIPX_LINESTATE_EVENT event, SIPX_LINESTATE_CAUSE cause);

    void addEvent(SIPX_INFO_INFO* pInfoInfo);
    void addCompareEvent(SIPX_INFO_INFO* pInfoInfo);

    void addEvent(SIPX_INFOSTATUS_INFO* pInfoStatus);
    void addCompareEvent(SIPX_INFOSTATUS_INFO* pInfoStatus);

    void addEvent(SIPX_CONFIG_INFO* pConfigInfo);
    void addCompareEvent(SIPX_CONFIG_INFO* pConfigInfo);

    void addMsgString(SIPX_LINE hLine, const char* szMsg);
    void addCompareMsgString(SIPX_LINE hLine, const char* szMsg);

    bool compare() ;
    bool compareNoOrder();
    void sortEvents();

protected:
    char* buildEventStr(char* array[], int nLength) ;
} ;

#endif /* ] _EVENTRECORDER_H */
