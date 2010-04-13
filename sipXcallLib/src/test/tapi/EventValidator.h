//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _EVENTVALIDATOR_H /* [ */
#define _EVENTVALIDATOR_H

#include "tapi/sipXtapi.h"
#include "tapi/sipXtapiEvents.h"
#include "tapi/sipXtapiInternal.h"
#include "os/OsBSem.h"
#include "os/OsMutex.h"
#include "os/OsLock.h"
#include "utl/UtlSList.h"

#define DEFAULT_TIMEOUT         -1
#define MAX_EVENT_CATEGORIES    16  // room for growth
#ifndef MIN
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif

typedef enum
{
    EVENT_VALIDATOR_ERROR_NONE,
    EVENT_VALIDATOR_ERROR_TIMEOUT,
    EVENT_VALIDATOR_ERROR_MISMATCH,
    EVENT_VALIDATOR_ERROR_UNEXPECTED
} EVENT_VALIDATOR_ERROR_TYPE ;

class EventValidator
{
protected:
    bool m_filterCategories[MAX_EVENT_CATEGORIES] ;
    int  m_iDefaultTimeoutInSecs ;
    int  m_iMaxLookAhead ;

    UtlSList   m_unprocessedEvents ;
    UtlSList   m_processedEvents ;
    UtlString* m_pUnfoundEvent ;
    UtlString  m_title ;

    OsBSem   m_semUnprocessed ;
    OsMutex  m_mutLists ;
    EVENT_VALIDATOR_ERROR_TYPE m_eErrorType ;

public:
    EventValidator(const char* szTitle = "Unknown") ;

    ~EventValidator() ;

    void ignoreEventCategory(SIPX_EVENT_CATEGORY category) ;
    bool isIgnoredCateogry(SIPX_EVENT_CATEGORY category) ;
    void setDefaultTimeout(int iTimeoutInSecs) ;
    void setMaxLookhead(int iMaxLookAhead) ;
    void reset() ;
    const char* getTitle();

    bool waitForCallEvent(SIPX_LINE hLine,
                          SIPX_CALL hCall,
                          SIPX_CALLSTATE_EVENT event,
                          SIPX_CALLSTATE_CAUSE cause,
                          bool bStrictOrderMatch = true,
                          int iTimeoutInSecs = DEFAULT_TIMEOUT) ;


    bool waitForMessage(SIPX_LINE hLine,
                        const char* szMsg,
                        bool bStrictOrderMatch = true,
                        int iTimeoutInSecs = DEFAULT_TIMEOUT) ;

    bool waitForLineEvent(SIPX_LINE hLine,
                          SIPX_LINESTATE_EVENT event,
                          SIPX_LINESTATE_CAUSE cause,
                          bool bStrictOrderMatch = true,
                          int iTimeoutInSecs = DEFAULT_TIMEOUT) ;


    bool waitForInfoStatusEvent(SIPX_INFO hInfo,
                                int status,
                                int responseCode,
                                const char* szResponseText,
                                bool bStrictOrderMatch = true,
                                int iTimeoutInSecs = DEFAULT_TIMEOUT) ;

    bool waitForInfoEvent(SIPX_CALL hCall,
                          SIPX_LINE hLine,
                          const char* szFromURL,
                          const char* szUserAgent,
                          const char* szContentType,
                          const char* szContent,
                          int nContentLength,
                          bool bStrictOrderMatch = true,
                          int iTimeoutInSecs = DEFAULT_TIMEOUT) ;


    bool waitForConfigEvent(SIPX_CONFIG_EVENT event,
                            bool bStrictOrderMatch = true,
                            int iTimeoutInSecs = DEFAULT_TIMEOUT) ;


    bool hasUnprocessedEvents() ;

    bool validateNoWaitingEvent() ;

    void report() ;

    void addEvent(SIPX_EVENT_CATEGORY category, void* pInfo) ;

    void addMessage(SIPX_LINE hLine, const char* szMsg) ;

    void addMarker(const char* szMarkerText) ;

protected:
    UtlString* allocCallStateEntry(SIPX_CALL hCall,
                                   SIPX_LINE hLine,
                                   SIPX_CALLSTATE_EVENT event,
                                   SIPX_CALLSTATE_CAUSE cause) ;
    UtlString* allocLineStateEntry(SIPX_LINE hLine,
                                   SIPX_LINESTATE_EVENT event,
                                   SIPX_LINESTATE_CAUSE cause) ;

    UtlString* allocMessageEvent(SIPX_LINE hLine,
                                 const char* szMessage) ;

    UtlString* allocInfoStatusEvent(SIPX_INFO hInfo,
                                    int status,
                                    int responseCode,
                                    const char* szResponseText)  ;

    UtlString* allocInfoEvent(SIPX_CALL hCall,
                              SIPX_LINE hLine,
                              const char* szFromURL,
                              const char* szUserAgent,
                              const char* szContentType,
                              const char* szContent,
                              int nContentLength) ;

    UtlString* allocConfigEvent(SIPX_CONFIG_EVENT hEvent) ;


    bool findEvent(const char* szEvent, int nMaxLookAhead, int &nActualLookAhead) ;


    bool waitForEvent(const char* szEvent, bool bStrictOrderMatch, int iTimeoutInSecs) ;



} ;


#endif /* _EVENTVALIDATOR_H ] */
