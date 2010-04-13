//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include "sipXtapiTest.h"
#include "EventValidator.h"
#include "EventRecorder.h"

extern SIPX_INST g_hInst;
extern EventRecorder g_recorder ;
extern EventRecorder g_lineRecorder;
extern EventRecorder g_lineRecorder2;

extern SIPX_INST g_hInst2;
extern EventRecorder g_recorder2 ;

extern SIPX_INST g_hInst3;
extern EventRecorder g_recorder3 ;

extern bool g_bCallbackCalled;

extern SIPX_CALL ghCallHangup;

SIPX_CALL g_hAutoAnswerCallbackCall ;
SIPX_CALL g_hAutoAnswerCallbackCallOther ;
SIPX_LINE g_hAutoAnswerCallbackLine ;
SIPX_CALL g_hAutoAnswerCallbackCall2 ;
SIPX_CALL g_hAutoAnswerCallbackCall2Other ;
SIPX_LINE g_hAutoAnswerCallbackLine2 ;
SIPX_CALL g_hAutoAnswerHangupCallbackCall ;
SIPX_LINE g_hAutoAnswerHangupCallbackLine ;
SIPX_CALL g_hAutoRejectCallbackCall ;
SIPX_LINE g_hAutoRejectCallbackLine ;
SIPX_CALL g_hAutoRedirectCallbackCall ;
SIPX_LINE g_hAutoRedirectCallbackLine ;


bool UniversalEventValidatorCallback(SIPX_EVENT_CATEGORY category,
                                     void* pInfo,
                                     void* pUserData)
{
    EventValidator* pValidator = (EventValidator*) pUserData ;
    assert(pValidator) ;
    pValidator->addEvent(category, pInfo) ;

    // Uncomment for debugging purposes...
    // char cBuffer[1000];
    // printf("%s: %s\n", pValidator->getTitle(), sipxEventToString(category, pInfo, cBuffer, 1000)) ;

    return true ;
}

void resetAutoAnswerCallback()
{
    g_hAutoAnswerCallbackCall = 0 ;
    g_hAutoAnswerCallbackCallOther = 0 ;
}


bool AutoAnswerCallback(SIPX_EVENT_CATEGORY category,
                        void* pInfo,
                        void* pUserData)
{
    if (category == EVENT_CATEGORY_CALLSTATE)
    {
        SIPX_CALLSTATE_INFO* pCallInfo = (SIPX_CALLSTATE_INFO*) pInfo;

        if (g_hAutoAnswerCallbackCall == 0)
        {
            g_hAutoAnswerCallbackCall = pCallInfo->hCall;
        }
        else if (g_hAutoAnswerCallbackCall != pCallInfo->hCall)
        {
            g_hAutoAnswerCallbackCallOther = pCallInfo->hCall ;
        }
        g_hAutoAnswerCallbackLine = pCallInfo->hLine;


        // If we have user data verify the line url against it
        if (pUserData)
        {
            char szBuffer[500] ;
            size_t nBuffer ;

            if (strlen((const char*) pUserData))
            {
                if (pCallInfo->hLine)  // hLine can be 0, and therefore, sipxLineGetURI should fail)
                {
                    CPPUNIT_ASSERT_EQUAL(sipxLineGetURI(pCallInfo->hLine, szBuffer, sizeof(szBuffer), nBuffer), SIPX_RESULT_SUCCESS) ;
                }
            }
        }

        switch(pCallInfo->event)
        {
            case CALLSTATE_OFFERING:
                sipxCallAccept(pCallInfo->hCall) ;
                break ;
            case CALLSTATE_ALERTING:
                sipxCallAnswer(pCallInfo->hCall) ;
                break ;
            case CALLSTATE_DISCONNECTED:
                {
                    SIPX_CALL hDestroy = pCallInfo->hCall ;
                    sipxCallDestroy(hDestroy) ;
                }
                break ;
            default:
                break ;
        }
    }

    return true;
}


void resetAutoAnswerCallback2()
{
    g_hAutoAnswerCallbackCall2 = 0 ;
    g_hAutoAnswerCallbackCall2Other = 0 ;
}

bool AutoAnswerCallback2(SIPX_EVENT_CATEGORY category,
                         void* pInfo,
                         void* pUserData)
{

    if (category == EVENT_CATEGORY_CALLSTATE)
    {
        SIPX_CALLSTATE_INFO* pCallInfo = (SIPX_CALLSTATE_INFO*) pInfo;

        if (g_hAutoAnswerCallbackCall2 == 0)
        {
            g_hAutoAnswerCallbackCall2 = pCallInfo->hCall;
        }
        else if (g_hAutoAnswerCallbackCall2 != pCallInfo->hCall)
        {
            g_hAutoAnswerCallbackCall2Other = pCallInfo->hCall;
        }
        g_hAutoAnswerCallbackLine2 = pCallInfo->hLine;



        // If we have user data verify the line url against it
        if (pUserData)
        {
            char szBuffer[500] ;
            size_t nBuffer ;

            if (strlen((const char*) pUserData))
            {
                if (pCallInfo->hLine)  // hLine can be 0, and therefore, sipxLineGetURI should fail)
                {
                    CPPUNIT_ASSERT_EQUAL(sipxLineGetURI(pCallInfo->hLine, szBuffer, sizeof(szBuffer), nBuffer), SIPX_RESULT_SUCCESS) ;
                }
            }
        }

        switch(pCallInfo->event)
        {
            case CALLSTATE_OFFERING:
                sipxCallAccept(pCallInfo->hCall) ;
                break ;
            case CALLSTATE_ALERTING:
                sipxCallAnswer(pCallInfo->hCall) ;
                break ;
            case CALLSTATE_DISCONNECTED:
                {
                    SIPX_CALL hDestroy = pCallInfo->hCall ;
                    sipxCallDestroy(hDestroy) ;
                }
                break ;
            case CALLSTATE_DESTROYED:
                {
                    int i ;
                    i = 2;
                }
                break ;
            default:
                break ;
        }
    }

    return true;
}


bool AutoAnswerHangupCallback(SIPX_EVENT_CATEGORY category,
                              void* pInfo,
                              void* pUserData)
{
    if (category == EVENT_CATEGORY_CALLSTATE)
    {
        SIPX_CALLSTATE_INFO* pCallInfo = (SIPX_CALLSTATE_INFO*) pInfo;
        g_hAutoAnswerHangupCallbackCall = pCallInfo->hCall;
        g_hAutoAnswerHangupCallbackLine = pCallInfo->hLine;


        // If we have user data verify the line url against it
        if (pUserData)
        {
            char szBuffer[500] ;
            size_t nBuffer ;

            if (strlen((const char*) pUserData))
            {
                if (g_hAutoAnswerHangupCallbackLine)  // hLine can be 0, and therefore, sipxLineGetURI should fail)
                {
                    CPPUNIT_ASSERT_EQUAL(sipxLineGetURI(g_hAutoAnswerHangupCallbackLine, szBuffer, sizeof(szBuffer), nBuffer), SIPX_RESULT_SUCCESS) ;
                }
            }
        }

        switch(pCallInfo->event)
        {
            case CALLSTATE_OFFERING:
                sipxCallAccept(g_hAutoAnswerHangupCallbackCall) ;
                break ;
            case CALLSTATE_ALERTING:
                {
                    sipxCallAnswer(g_hAutoAnswerHangupCallbackCall) ;
                    OsTask::delay(CALL_DELAY);
                    SIPX_CALL hDestroy = g_hAutoAnswerHangupCallbackCall ;
                    sipxCallDestroy(hDestroy) ;
                }
                break ;
            default:
                break ;
        }
    }

    return true;
}


bool AutoRejectCallback(SIPX_EVENT_CATEGORY category,
                        void* pInfo,
                        void* pUserData)
{
    if (category == EVENT_CATEGORY_CALLSTATE)
    {
        SIPX_CALLSTATE_INFO* pCallInfo = (SIPX_CALLSTATE_INFO*) pInfo;
        g_hAutoRejectCallbackCall = pCallInfo->hCall;
        g_hAutoRejectCallbackLine = pCallInfo->hLine;


        // If we have user data verify the line url against it
        if (pUserData)
        {
            char szBuffer[500] ;
            size_t nBuffer ;

            if (strlen((const char*) pUserData))
            {
                if (g_hAutoRejectCallbackLine)  // hLine can be 0, and therefore, sipxLineGetURI should fail)
                {
                    CPPUNIT_ASSERT_EQUAL(sipxLineGetURI(g_hAutoRejectCallbackLine, szBuffer, sizeof(szBuffer), nBuffer), SIPX_RESULT_SUCCESS) ;
                }
            }
        }

        switch(pCallInfo->event)
        {
            case CALLSTATE_OFFERING:
                sipxCallReject(g_hAutoRejectCallbackCall) ;
                break ;
            case CALLSTATE_DISCONNECTED:
                {
                    SIPX_CALL hDestroy = g_hAutoRejectCallbackCall ;
                    sipxCallDestroy(hDestroy) ;
                }
                break ;
            default:
                break ;
        }
    }

    return true;
}


bool AutoRedirectCallback(SIPX_EVENT_CATEGORY category,
                          void* pInfo,
                          void* pUserData)
{
    if (category == EVENT_CATEGORY_CALLSTATE)
    {
        SIPX_CALLSTATE_INFO* pCallInfo = (SIPX_CALLSTATE_INFO*) pInfo;
        g_hAutoRedirectCallbackCall = pCallInfo->hCall;
        g_hAutoRedirectCallbackLine = pCallInfo->hLine;


        // If we have user data verify the line url against it
        if (pUserData)
        {
            char szBuffer[500] ;
            size_t nBuffer ;

            if (strlen((const char*) pUserData))
            {
                if (g_hAutoRedirectCallbackLine)  // hLine can be 0, and therefore, sipxLineGetURI should fail)
                {
                    CPPUNIT_ASSERT_EQUAL(sipxLineGetURI(g_hAutoRedirectCallbackLine, szBuffer, sizeof(szBuffer), nBuffer), SIPX_RESULT_SUCCESS) ;
                }
            }
        }

        switch(pCallInfo->event)
        {
            case CALLSTATE_OFFERING:
                sipxCallRedirect(g_hAutoRedirectCallbackCall, "sip:foo@127.0.0.1:10000");
                break ;
            case CALLSTATE_DISCONNECTED:
                {
                    SIPX_CALL hDestroy = g_hAutoRedirectCallbackCall ;
                    sipxCallDestroy(hDestroy) ;
                }
                break ;
            default:
                break ;
        }
    }

    return true;
}


bool basicCall_CallBack_Place(SIPX_EVENT_CATEGORY category,
                              void* pInfo,
                              void* pUserData)
{
    if (category == EVENT_CATEGORY_CALLSTATE)
    {
        SIPX_CALLSTATE_INFO* pCallInfo = (SIPX_CALLSTATE_INFO*)pInfo;
        g_recorder.addEvent(pCallInfo->hLine, pCallInfo->event, pCallInfo->cause) ;
    }
    return true;
}


bool basicCall_CallBack_Receive(SIPX_EVENT_CATEGORY category,
                                         void* pInfo,
                                         void* pUserData)
{

    if (category == EVENT_CATEGORY_CALLSTATE)
    {
        SIPX_CALLSTATE_INFO* pCallInfo = (SIPX_CALLSTATE_INFO*)pInfo;
        SIPX_LINE hLine = pCallInfo->hLine;
        SIPX_CALL hCall = pCallInfo->hCall;

        g_recorder2.addEvent(hLine, pCallInfo->event, pCallInfo->cause) ;

        // If we have user data verify the line url against it
        if (pUserData)
        {
            char szBuffer[500] ;
            size_t nBuffer ;

            if (strlen((const char*) pUserData))
            {
                if (hLine)  // hLine can be 0, and therefore, sipxLineGetURI should fail)
                {
                    CPPUNIT_ASSERT_EQUAL(sipxLineGetURI(hLine, szBuffer, sizeof(szBuffer), nBuffer), SIPX_RESULT_SUCCESS) ;

                    // an event can come in with what appears to be the "wrong" user data.
                    // for instance, if a listener is removed and a new listener is added immediately with new user data.
                    // printf("comparing %s to %s\n", pUserData, szBuffer) ;
                    //CPPUNIT_ASSERT(strcmp((char*) pUserData, szBuffer) == 0) ;
                }
            }
            //else
            //{
                //CPPUNIT_ASSERT_EQUAL(sipxLineGetURI(hLine, szBuffer, sizeof(szBuffer), nBuffer), SIPX_RESULT_FAILURE) ;
            //}
        }

        switch(pCallInfo->event)
        {
            case CALLSTATE_OFFERING:
                sipxCallAccept(hCall) ;
                break ;
            case CALLSTATE_ALERTING:
                sipxCallAnswer(hCall) ;
                break ;
            case CALLSTATE_DISCONNECTED:
                sipxCallDestroy(hCall) ;
                break ;
            default:
                break ;
        }
    }

    return true;
}


bool basicCall_CallBack_Receive3(SIPX_EVENT_CATEGORY category,
                              void* pInfo,
                              void* pUserData)
{
    if (category == EVENT_CATEGORY_CALLSTATE)
    {
        SIPX_CALLSTATE_INFO* pCallInfo = (SIPX_CALLSTATE_INFO*) pInfo;
        SIPX_LINE hLine = pCallInfo->hLine;
        SIPX_CALL hCall = pCallInfo->hCall;
        g_recorder3.addEvent(hLine, pCallInfo->event, pCallInfo->cause) ;

        // If we have user data verify the line url against it
        if (pUserData)
        {
            char szBuffer[500] ;
            size_t nBuffer ;

            if (strlen((const char*) pUserData))
            {
                CPPUNIT_ASSERT_EQUAL(sipxLineGetURI(hLine, szBuffer, sizeof(szBuffer), nBuffer), SIPX_RESULT_SUCCESS) ;

                // printf("comparing %s to %s\n", pUserData, szBuffer) ;
                CPPUNIT_ASSERT(strcmp((char*) pUserData, szBuffer) == 0) ;
            }
            else
            {
                CPPUNIT_ASSERT_EQUAL(sipxLineGetURI(hLine, szBuffer, sizeof(szBuffer), nBuffer), SIPX_RESULT_FAILURE) ;
            }
        }

        switch(pCallInfo->event)
        {
            case CALLSTATE_OFFERING:
                sipxCallAccept(hCall) ;
                break ;
            case CALLSTATE_ALERTING:
                sipxCallAnswer(hCall) ;
                break ;
            case CALLSTATE_DISCONNECTED:
                sipxCallDestroy(hCall) ;
                break ;
            default:
                break ;
        }
    }
    return true;
}



bool basicCall_CallBack_Receive3_hangup(SIPX_EVENT_CATEGORY category,
                              void* pInfo,
                              void* pUserData)
{
    if (category == EVENT_CATEGORY_CALLSTATE)
    {
        SIPX_CALLSTATE_INFO* pCallInfo = (SIPX_CALLSTATE_INFO*) pInfo;
        SIPX_LINE hLine = pCallInfo->hLine;
        SIPX_CALL hCall = pCallInfo->hCall;
        g_recorder3.addEvent(hLine, pCallInfo->event, pCallInfo->cause) ;

        // If we have user data verify the line url against it
        if (pUserData)
        {
            char szBuffer[500] ;
            size_t nBuffer ;

            if (strlen((const char*) pUserData))
            {
                CPPUNIT_ASSERT_EQUAL(sipxLineGetURI(hLine, szBuffer, sizeof(szBuffer), nBuffer), SIPX_RESULT_SUCCESS) ;

                // printf("comparing %s to %s\n", pUserData, szBuffer) ;
                CPPUNIT_ASSERT(strcmp((char*) pUserData, szBuffer) == 0) ;
            }
            else
            {
                CPPUNIT_ASSERT_EQUAL(sipxLineGetURI(hLine, szBuffer, sizeof(szBuffer), nBuffer), SIPX_RESULT_FAILURE) ;
            }
        }

        switch(pCallInfo->event)
        {
            case CALLSTATE_OFFERING:
                sipxCallAccept(hCall) ;
                break ;
            case CALLSTATE_ALERTING:
                sipxCallAnswer(hCall) ;
                break ;
            case CALLSTATE_CONNECTED:
                ghCallHangup = hCall;
                break ;
            case CALLSTATE_DISCONNECTED:
                break ;
            default:
                break ;
        }
    }
    return true;
}


bool basicCall_CallBack_Receive3_busy(SIPX_EVENT_CATEGORY category,
                              void* pInfo,
                              void* pUserData)
{
    if (category == EVENT_CATEGORY_CALLSTATE)
    {
        SIPX_CALLSTATE_INFO* pCallInfo = (SIPX_CALLSTATE_INFO*) pInfo;
        SIPX_LINE hLine = pCallInfo->hLine;
        SIPX_CALL hCall = pCallInfo->hCall;
        g_recorder3.addEvent(hLine, pCallInfo->event, pCallInfo->cause) ;

        // If we have user data verify the line url against it
        if (pUserData)
        {
            char szBuffer[500] ;
            size_t nBuffer ;

            if (strlen((const char*) pUserData))
            {
                CPPUNIT_ASSERT_EQUAL(sipxLineGetURI(hLine, szBuffer, sizeof(szBuffer), nBuffer), SIPX_RESULT_SUCCESS) ;

                // printf("comparing %s to %s\n", pUserData, szBuffer) ;
                CPPUNIT_ASSERT(strcmp((char*) pUserData, szBuffer) == 0) ;
            }
            else
            {
                CPPUNIT_ASSERT_EQUAL(sipxLineGetURI(hLine, szBuffer, sizeof(szBuffer), nBuffer), SIPX_RESULT_FAILURE) ;
            }
        }

        switch(pCallInfo->event)
        {
            case CALLSTATE_OFFERING:
                sipxCallReject(hCall) ;
                sipxCallDestroy(hCall) ;
                break ;
            default:
                break ;
        }
    }
    return true;
}

bool basicCall_CallBack_Receive_Hangup(SIPX_EVENT_CATEGORY category,
                              void* pInfo,
                              void* pUserData)
{
    if (category == EVENT_CATEGORY_CALLSTATE)
    {
        SIPX_CALLSTATE_INFO* pCallInfo = (SIPX_CALLSTATE_INFO*) pInfo;
        SIPX_LINE hLine = pCallInfo->hLine;
        SIPX_CALL hCall = pCallInfo->hCall;
        g_recorder2.addEvent(hLine, pCallInfo->event, pCallInfo->cause) ;

        switch(pCallInfo->event)
        {
            case CALLSTATE_OFFERING:
                sipxCallAccept(hCall) ;
                break ;
            case CALLSTATE_ALERTING:
                sipxCallAnswer(hCall) ;
                OsTask::delay(CALL_DELAY);
                sipxCallDestroy(hCall) ;
                break ;
            case CALLSTATE_DISCONNECTED:
                break ;
            default:
                break ;
        }
    }
    return true;
}

bool basicCall_CallBack_Receive_Reject(SIPX_EVENT_CATEGORY category,
                              void* pInfo,
                              void* pUserData)
{
    if (category == EVENT_CATEGORY_CALLSTATE)
    {
        SIPX_CALLSTATE_INFO* pCallInfo = (SIPX_CALLSTATE_INFO*) pInfo;
        SIPX_LINE hLine = pCallInfo->hLine;
        SIPX_CALL hCall = pCallInfo->hCall;
        g_recorder2.addEvent(hLine, pCallInfo->event, pCallInfo->cause) ;


        switch(pCallInfo->event)
        {
            case CALLSTATE_OFFERING:
                sipxCallReject(hCall) ;
                sipxCallDestroy(hCall) ;
                break ;
            case CALLSTATE_ALERTING:
                break ;
            case CALLSTATE_DISCONNECTED:
                break ;
            default:
                break ;
        }
    }
    return true;
}


bool basicCall_CallBack_Redirect(SIPX_EVENT_CATEGORY category,
                              void* pInfo,
                              void* pUserData)
{
    if (category == EVENT_CATEGORY_CALLSTATE)
    {
        SIPX_CALLSTATE_INFO* pCallInfo = (SIPX_CALLSTATE_INFO*) pInfo;
        SIPX_CALL hCall = pCallInfo->hCall;

        switch(pCallInfo->event)
        {
            case CALLSTATE_OFFERING:
                sipxCallRedirect(hCall, "sip:foo@127.0.0.1:10000");
				sipxCallDestroy(hCall);
                break ;
            case CALLSTATE_ALERTING:
                break ;
            case CALLSTATE_DISCONNECTED:
                break ;
            default:
                break ;
        }
    }
    return true;
}
