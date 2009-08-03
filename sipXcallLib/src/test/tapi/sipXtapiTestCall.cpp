//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
//////////////////////////////////////////////////////////////////////////////

#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include "sipXtapiTest.h"
#include "EventValidator.h"
#include "callbacks.h"

extern SIPX_INST g_hInst;
extern SIPX_INST g_hInst2;
extern SIPX_INST g_hInst3;
extern SIPX_INST g_hInst5;

extern bool g_bCallbackCalled;

extern SIPX_INST g_hInstInfo;
extern SIPX_CALL ghCallHangup;

void sipXtapiTestSuite::testCallMakeAPI()
{
    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestCallMakeAPI (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        SIPX_LINE hLine = SIPX_LINE_NULL;
        SIPX_CALL hCall = SIPX_CALL_NULL ;
        SIPX_RESULT rc ;

        rc = sipxLineAdd(g_hInst, "sip:bandreasen@pingtel.com", &hLine) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT(hLine) ;

        rc = sipxCallCreate(g_hInst, hLine, &hCall) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT(hCall != SIPX_CALL_NULL) ;

        rc = sipxCallDestroy(hCall) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT(hCall == SIPX_CALL_NULL) ;

        rc = sipxLineRemove(hLine) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;
    }

    OsTask::delay(TEST_DELAY) ;
    checkForLeaks();
}


void sipXtapiTestSuite::testCallGetID()
{
    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        SIPX_LINE hLine = SIPX_LINE_NULL ;
        SIPX_CALL hCall = SIPX_CALL_NULL ;

        printf("\ntestCallGetID (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        createCall(&hLine, &hCall) ;

        char cBuf[128] ;

        memset(cBuf, 0, sizeof(cBuf)) ;
        CPPUNIT_ASSERT_EQUAL(sipxCallGetID(hCall, cBuf, sizeof(cBuf)), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT(strlen(cBuf) > 0) ;

        memset(cBuf, ' ', sizeof(cBuf)) ;
        CPPUNIT_ASSERT_EQUAL(sipxCallGetID(hCall, cBuf, 4), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT(strlen(cBuf) == 3) ;
        CPPUNIT_ASSERT(cBuf[5] == ' ') ;

        destroyCall(hLine, hCall) ;
    }
    OsTask::delay(TEST_DELAY) ;

    checkForLeaks();
}

void sipXtapiTestSuite::testCallRapidCallAndHangup()
{
    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        SIPX_LINE hLine = SIPX_LINE_NULL ;
        SIPX_CALL hCall = SIPX_CALL_NULL ;

        printf("\ntestCallRapidCallAndHangup (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);


        CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst, "sip:bandreasen@pingtel.com", &hLine), SIPX_RESULT_SUCCESS) ;

         CPPUNIT_ASSERT_EQUAL(sipxCallCreate(g_hInst, hLine, &hCall), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxCallConnect(hCall, "sip:mike2@cheetah.pingtel.com"), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxCallDestroy(hCall), SIPX_RESULT_SUCCESS);

         CPPUNIT_ASSERT_EQUAL(sipxCallCreate(g_hInst, hLine, &hCall), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxCallConnect(hCall, "sip:mike2@cheetah.pingtel.com"), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxCallDestroy(hCall), SIPX_RESULT_SUCCESS);

        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS) ;
        OsTask::delay(CALL_DELAY*2);
    }
    OsTask::delay(TEST_DELAY) ;
}


void sipXtapiTestSuite::testCallGetRemoteID()
{
    bool bRC ;
    EventValidator validatorCalling("testCallGetRemoteID.calling") ;
    EventValidator validatorCalled("testCallGetRemoteID.called") ;

    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestCallGetRemoteID (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);
        SIPX_CALL hCall ;
        SIPX_LINE hLine ;
        SIPX_LINE hReceivingLine;

        validatorCalling.reset() ;
        validatorCalled.reset() ;

        // Setup Auto-answer call back
        resetAutoAnswerCallback() ;
        sipxEventListenerAdd(g_hInst2, AutoAnswerCallback, NULL) ;
        sipxEventListenerAdd(g_hInst2, UniversalEventValidatorCallback, &validatorCalled) ;
        sipxEventListenerAdd(g_hInst, UniversalEventValidatorCallback, &validatorCalling) ;

        sipxLineAdd(g_hInst2, "sip:foo@127.0.0.1:9100", &hReceivingLine, CONTACT_LOCAL);
        bRC = validatorCalled.waitForLineEvent(hReceivingLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        createCall(&hLine, &hCall) ;
        bRC = validatorCalling.waitForLineEvent(hLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        sipxCallConnect(hCall, "sip:foo@127.0.0.1:9100") ;


        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_DIALTONE, CALLSTATE_DIALTONE_UNKNOWN, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_REMOTE_OFFERING, CALLSTATE_REMOTE_OFFERING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_REMOTE_ALERTING, CALLSTATE_REMOTE_ALERTING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_NEWCALL, CALLSTATE_NEW_CALL_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_OFFERING, CALLSTATE_OFFERING_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_ALERTING, CALLSTATE_ALERTING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Test sipxCallGetRemoteID
        char cBuf[128] ;
        memset(cBuf, 0, sizeof(cBuf));
        CPPUNIT_ASSERT_EQUAL( sipxCallGetRemoteID(hCall, cBuf, sizeof(cBuf)), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT(strlen(cBuf) > 0) ;

        memset(cBuf, ' ', sizeof(cBuf)) ;
        CPPUNIT_ASSERT_EQUAL(sipxCallGetRemoteID(hCall, cBuf, 4), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT(strlen(cBuf) == 3) ;
        CPPUNIT_ASSERT(cBuf[5] == ' ') ;

        SIPX_CALL hDestroyedCall = hCall ;
        destroyCall(hCall) ;

        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hDestroyedCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE_HELD, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hDestroyedCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hDestroyedCall, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hDestroyedCall, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst, UniversalEventValidatorCallback, &validatorCalling), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, AutoAnswerCallback, NULL), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, UniversalEventValidatorCallback, &validatorCalled), SIPX_RESULT_SUCCESS) ;

        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hReceivingLine), SIPX_RESULT_SUCCESS);
    }

    OsTask::delay(TEST_DELAY) ;

    checkForLeaks();
}


void sipXtapiTestSuite::testCallGetLocalID()
{
    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        SIPX_LINE hLine = SIPX_LINE_NULL ;
        SIPX_CALL hCall = SIPX_CALL_NULL ;

        printf("\ntestCallGetLocalID (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        createCall(&hLine, &hCall) ;

        char cBuf[128] ;

        memset(cBuf, 0, sizeof(cBuf)) ;
        CPPUNIT_ASSERT_EQUAL(sipxCallGetLocalID(hCall, cBuf, sizeof(cBuf)), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT(strlen(cBuf) > 0) ;

        memset(cBuf, ' ', sizeof(cBuf)) ;
        CPPUNIT_ASSERT_EQUAL(sipxCallGetLocalID(hCall, cBuf, 4), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT(strlen(cBuf) == 3) ;
        CPPUNIT_ASSERT(cBuf[5] == ' ') ;

        destroyCall(hLine, hCall) ;
    }
    OsTask::delay(TEST_DELAY) ;

    checkForLeaks();
}


void sipXtapiTestSuite::createCall(SIPX_LINE* phLine, SIPX_CALL* phCall)
{
    CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst, "sip:bandreasen@pingtel.com", phLine), SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT(*phLine) ;

    createCall(*phLine, phCall) ;
}

void sipXtapiTestSuite::createCall(SIPX_LINE hLine, SIPX_CALL* phCall)
{
    CPPUNIT_ASSERT_EQUAL(sipxCallCreate(g_hInst, hLine, phCall), SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT(*phCall) ;
}


void sipXtapiTestSuite::destroyCall(SIPX_LINE& hLine, SIPX_CALL& hCall)
{
    destroyCall(hCall) ;
    OsTask::delay(250) ;
    CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS) ;
}

void sipXtapiTestSuite::destroyCall(SIPX_CALL& hCall)
{
    CPPUNIT_ASSERT_EQUAL(sipxCallDestroy(hCall), SIPX_RESULT_SUCCESS) ;
    CPPUNIT_ASSERT(hCall == SIPX_CALL_NULL) ;
}


// A calls B, B answers, A hangs up
void sipXtapiTestSuite::testCallBasic()
{
    bool bRC ;
    EventValidator validatorCalling("testCallBasic.calling") ;
    EventValidator validatorCalled("testCallBasic.called") ;

    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestCallBasic (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);
        SIPX_CALL hCall ;
        SIPX_LINE hLine ;
        SIPX_LINE hReceivingLine;

        validatorCalling.reset() ;
        validatorCalled.reset() ;

        // Setup Auto-answer call back
        resetAutoAnswerCallback() ;
        sipxEventListenerAdd(g_hInst2, AutoAnswerCallback, NULL) ;
        sipxEventListenerAdd(g_hInst2, UniversalEventValidatorCallback, &validatorCalled) ;
        sipxEventListenerAdd(g_hInst, UniversalEventValidatorCallback, &validatorCalling) ;

        sipxLineAdd(g_hInst2, "sip:foo@127.0.0.1:9100", &hReceivingLine, CONTACT_LOCAL);
        bRC = validatorCalled.waitForLineEvent(hReceivingLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        createCall(&hLine, &hCall) ;
        bRC = validatorCalling.waitForLineEvent(hLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        sipxCallConnect(hCall, "sip:foo@127.0.0.1:9100") ;


        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_DIALTONE, CALLSTATE_DIALTONE_UNKNOWN, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_REMOTE_OFFERING, CALLSTATE_REMOTE_OFFERING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_REMOTE_ALERTING, CALLSTATE_REMOTE_ALERTING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_NEWCALL, CALLSTATE_NEW_CALL_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_OFFERING, CALLSTATE_OFFERING_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_ALERTING, CALLSTATE_ALERTING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;

        int connectionId = -1;

        CPPUNIT_ASSERT_EQUAL(sipxCallGetConnectionId(hCall, connectionId), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT(connectionId != -1) ;

        SIPX_CALL hDestroyedCall = hCall ;
        destroyCall(hCall) ;

        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hDestroyedCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE_HELD, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hDestroyedCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hDestroyedCall, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hDestroyedCall, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst, UniversalEventValidatorCallback, &validatorCalling), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, AutoAnswerCallback, NULL), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, UniversalEventValidatorCallback, &validatorCalled), SIPX_RESULT_SUCCESS) ;

        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hReceivingLine), SIPX_RESULT_SUCCESS);
    }

    OsTask::delay(TEST_DELAY) ;

    checkForLeaks();
}



// A calls B, B answers, A mutes and unmutes repeatedly, then hangs up
void sipXtapiTestSuite::testCallMute()
{
    bool bRC ;
    EventValidator validatorCalling("testCallMute.calling") ;
    EventValidator validatorCalled("testCallMute.called") ;

    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestCallMute (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);
        SIPX_CALL hCall ;
        SIPX_LINE hLine ;
        SIPX_LINE hReceivingLine;

        validatorCalling.reset() ;
        validatorCalled.reset() ;

        // Setup Auto-answer call back
        resetAutoAnswerCallback() ;
        sipxEventListenerAdd(g_hInst2, AutoAnswerCallback, NULL) ;
        sipxEventListenerAdd(g_hInst2, UniversalEventValidatorCallback, &validatorCalled) ;
        sipxEventListenerAdd(g_hInst, UniversalEventValidatorCallback, &validatorCalling) ;

        sipxLineAdd(g_hInst2, "sip:foo@127.0.0.1:9100", &hReceivingLine, CONTACT_LOCAL);
        bRC = validatorCalled.waitForLineEvent(hReceivingLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        createCall(&hLine, &hCall) ;
        bRC = validatorCalling.waitForLineEvent(hLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        sipxCallConnect(hCall, "sip:foo@127.0.0.1:9100") ;


        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_DIALTONE, CALLSTATE_DIALTONE_UNKNOWN, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_REMOTE_OFFERING, CALLSTATE_REMOTE_OFFERING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_REMOTE_ALERTING, CALLSTATE_REMOTE_ALERTING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_NEWCALL, CALLSTATE_NEW_CALL_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_OFFERING, CALLSTATE_OFFERING_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_ALERTING, CALLSTATE_ALERTING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;

        int connectionId = -1;

        CPPUNIT_ASSERT_EQUAL(sipxCallGetConnectionId(hCall, connectionId), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT(connectionId != -1) ;

        // loop 10 times, muting and unmuting
        for (int i = 0; i < 50; i++)
        {
            sipxAudioMute(g_hInst, true);
            sipxAudioMute(g_hInst, false);
        }

        SIPX_CALL hDestroyedCall = hCall ;
        destroyCall(hCall) ;

        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hDestroyedCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE_HELD, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hDestroyedCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hDestroyedCall, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hDestroyedCall, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst, UniversalEventValidatorCallback, &validatorCalling), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, AutoAnswerCallback, NULL), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, UniversalEventValidatorCallback, &validatorCalled), SIPX_RESULT_SUCCESS) ;

        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hReceivingLine), SIPX_RESULT_SUCCESS);
    }

    OsTask::delay(TEST_DELAY) ;

    checkForLeaks();
}


// A calls B, B answers, B hangs up
void sipXtapiTestSuite::testCallBasic2()
{
    bool bRC ;
    EventValidator validatorCalling("testCallBasic2.calling") ;
    EventValidator validatorCalled("testCallBasic2.called") ;

    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestCallBasic2 (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);
        SIPX_CALL hCall ;
        SIPX_LINE hLine ;
        SIPX_LINE hReceivingLine;

        validatorCalling.reset() ;
        validatorCalled.reset() ;

        // Setup Auto-answer call back
        sipxEventListenerAdd(g_hInst2, AutoAnswerHangupCallback, NULL) ;
        sipxEventListenerAdd(g_hInst2, UniversalEventValidatorCallback, &validatorCalled) ;
        sipxEventListenerAdd(g_hInst, UniversalEventValidatorCallback, &validatorCalling) ;

        sipxLineAdd(g_hInst2, "sip:foo@127.0.0.1:9100", &hReceivingLine, CONTACT_LOCAL);

        bRC = validatorCalled.waitForLineEvent(hReceivingLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        createCall(&hLine, &hCall) ;

        bRC = validatorCalling.waitForLineEvent(hLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        sipxCallConnect(hCall, "sip:foo@127.0.0.1:9100") ;

        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_DIALTONE, CALLSTATE_DIALTONE_UNKNOWN, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_REMOTE_OFFERING, CALLSTATE_REMOTE_OFFERING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_REMOTE_ALERTING, CALLSTATE_REMOTE_ALERTING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerHangupCallbackLine, g_hAutoAnswerHangupCallbackCall, CALLSTATE_NEWCALL, CALLSTATE_NEW_CALL_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerHangupCallbackLine, g_hAutoAnswerHangupCallbackCall, CALLSTATE_OFFERING, CALLSTATE_OFFERING_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerHangupCallbackLine, g_hAutoAnswerHangupCallbackCall, CALLSTATE_ALERTING, CALLSTATE_ALERTING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerHangupCallbackLine, g_hAutoAnswerHangupCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerHangupCallbackLine, g_hAutoAnswerHangupCallbackCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Called side with automatically hang up
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerHangupCallbackLine, g_hAutoAnswerHangupCallbackCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerHangupCallbackLine, g_hAutoAnswerHangupCallbackCall, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerHangupCallbackLine, g_hAutoAnswerHangupCallbackCall, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Calling side should disconnect
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        SIPX_CALL hDestroyedCall = hCall ;
        destroyCall(hCall) ;

        // Finally, calling side is cleaned up
        bRC = validatorCalling.waitForCallEvent(hLine, hDestroyedCall, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst, UniversalEventValidatorCallback, &validatorCalling), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, AutoAnswerHangupCallback, NULL), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, UniversalEventValidatorCallback, &validatorCalled), SIPX_RESULT_SUCCESS) ;

        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hReceivingLine), SIPX_RESULT_SUCCESS);
    }

    OsTask::delay(TEST_DELAY) ;

    checkForLeaks() ;
}


// A calls B, B is busy
void sipXtapiTestSuite::testCallBusy()
{
    bool bRC ;
    EventValidator validatorCalling("testCallBusy.calling") ;
    EventValidator validatorCalled("testCallBusy.calling") ;

    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestCallBusy (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        SIPX_CALL hCall ;
        SIPX_LINE hLine ;
        SIPX_LINE hReceivingLine;

        validatorCalling.reset() ;
        validatorCalled.reset() ;

        // Setup Auto-answer call back
        sipxEventListenerAdd(g_hInst2, AutoRejectCallback, NULL) ;
        sipxEventListenerAdd(g_hInst2, UniversalEventValidatorCallback, &validatorCalled) ;
        sipxEventListenerAdd(g_hInst, UniversalEventValidatorCallback, &validatorCalling) ;

        sipxLineAdd(g_hInst2, "sip:foo@127.0.0.1:9100", &hReceivingLine, CONTACT_LOCAL);

        bRC = validatorCalled.waitForLineEvent(hReceivingLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        createCall(&hLine, &hCall) ;

        bRC = validatorCalling.waitForLineEvent(hLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        sipxCallConnect(hCall, "sip:foo@127.0.0.1:9100") ;

        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_DIALTONE, CALLSTATE_DIALTONE_UNKNOWN, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_REMOTE_OFFERING, CALLSTATE_REMOTE_OFFERING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_BUSY, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side (auto rejects)
        bRC = validatorCalled.waitForCallEvent(g_hAutoRejectCallbackLine, g_hAutoRejectCallbackCall, CALLSTATE_NEWCALL, CALLSTATE_NEW_CALL_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoRejectCallbackLine, g_hAutoRejectCallbackCall, CALLSTATE_OFFERING, CALLSTATE_OFFERING_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoRejectCallbackLine, g_hAutoRejectCallbackCall, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoRejectCallbackLine, g_hAutoRejectCallbackCall, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Calling side should disconnect
        SIPX_CALL hDestroyedCall = hCall ;
        destroyCall(hCall) ;

        // Finally, calling side is cleaned up
        bRC = validatorCalling.waitForCallEvent(hLine, hDestroyedCall, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst, UniversalEventValidatorCallback, &validatorCalling), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, AutoRejectCallback, NULL), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, UniversalEventValidatorCallback, &validatorCalled), SIPX_RESULT_SUCCESS) ;

        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hReceivingLine), SIPX_RESULT_SUCCESS);
    }

    OsTask::delay(TEST_DELAY) ;

    checkForLeaks() ;
}


// A calls B, B redirects to C
void sipXtapiTestSuite::testCallRedirect()
{
    bool bRC ;
    EventValidator validatorCalling("testCallRedirect.calling") ;
    EventValidator validatorRedirector("testCallRedirect.redirector") ;
    EventValidator validatorCalled("testCallRedirected.called") ;

    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestCallRedirect (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        SIPX_CALL hCall ;
        SIPX_LINE hLine ;
        SIPX_LINE hRedirectLine ;
        SIPX_LINE hCalledLine ;

        validatorCalling.reset() ;
        validatorRedirector.reset() ;
        validatorCalled.reset() ;

        // Setup callbacks
        resetAutoAnswerCallback() ;
        sipxEventListenerAdd(g_hInst,  UniversalEventValidatorCallback, &validatorCalling) ;
        sipxEventListenerAdd(g_hInst2, UniversalEventValidatorCallback, &validatorRedirector) ;
        sipxEventListenerAdd(g_hInst2, AutoRedirectCallback, NULL) ;
        sipxEventListenerAdd(g_hInst3, UniversalEventValidatorCallback, &validatorCalled) ;
        sipxEventListenerAdd(g_hInst3, AutoAnswerCallback, NULL) ;


        sipxLineAdd(g_hInst2, "sip:foo@127.0.0.1:9100", &hRedirectLine, CONTACT_LOCAL);
        sipxLineAdd(g_hInst3, "sip:foo@127.0.0.1:10000", &hCalledLine, CONTACT_LOCAL);

        bRC = validatorRedirector.waitForLineEvent(hRedirectLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForLineEvent(hCalledLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        createCall(&hLine, &hCall) ;
        bRC = validatorCalling.waitForLineEvent(hLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;


        sipxCallConnect(hCall, "sip:foo@127.0.0.1:9100") ;

        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_DIALTONE, CALLSTATE_DIALTONE_UNKNOWN, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_REMOTE_OFFERING, CALLSTATE_REMOTE_OFFERING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        /*
        // ALERTING is not being fired -- need to investigate
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_REMOTE_ALERTING, CALLSTATE_REMOTE_ALERTING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        */
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;

        SIPX_CALL hDestroyCall = hCall ;
        destroyCall(hDestroyCall) ;

        // Finally, calling side is cleaned up
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE_HELD, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Redirected UA
        bRC = validatorRedirector.waitForCallEvent(g_hAutoRedirectCallbackLine, g_hAutoRedirectCallbackCall, CALLSTATE_NEWCALL, CALLSTATE_NEW_CALL_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorRedirector.waitForCallEvent(g_hAutoRedirectCallbackLine, g_hAutoRedirectCallbackCall, CALLSTATE_OFFERING, CALLSTATE_OFFERING_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorRedirector.waitForCallEvent(g_hAutoRedirectCallbackLine, g_hAutoRedirectCallbackCall, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_REDIRECTED, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorRedirector.waitForCallEvent(g_hAutoRedirectCallbackLine, g_hAutoRedirectCallbackCall, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called UA
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_NEWCALL, CALLSTATE_NEW_CALL_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_OFFERING, CALLSTATE_OFFERING_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_ALERTING, CALLSTATE_ALERTING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;


        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst,  UniversalEventValidatorCallback, &validatorCalling), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, UniversalEventValidatorCallback, &validatorRedirector), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, AutoRedirectCallback, NULL), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst3, UniversalEventValidatorCallback, &validatorCalled), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst3, AutoAnswerCallback, NULL), SIPX_RESULT_SUCCESS);

        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hRedirectLine), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hCalledLine), SIPX_RESULT_SUCCESS) ;
    }
    OsTask::delay(TEST_DELAY) ;

    checkForLeaks();
}


// A calls B, B answers, A places call on hold, A takes call off hold, A hangs up
void sipXtapiTestSuite::testCallHold()
{
    bool bRC ;
    EventValidator validatorCalling("testCallHold.calling") ;
    EventValidator validatorCalled("testCallHold.called") ;

    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestCallHold (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);
        SIPX_CALL hCall ;
        SIPX_LINE hLine ;
        SIPX_LINE hCalledLine;

        validatorCalling.reset() ;
        validatorCalled.reset() ;

        // Setup Auto-answer call back
        resetAutoAnswerCallback() ;
        sipxEventListenerAdd(g_hInst, UniversalEventValidatorCallback, &validatorCalling) ;
        sipxEventListenerAdd(g_hInst2, UniversalEventValidatorCallback, &validatorCalled) ;
        sipxEventListenerAdd(g_hInst2, AutoAnswerCallback, NULL) ;

        // Add Line
        sipxLineAdd(g_hInst2, "sip:foo@127.0.0.1:9100", &hCalledLine, CONTACT_LOCAL);
        bRC = validatorCalled.waitForLineEvent(hCalledLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Create Call/Line
        createCall(&hLine, &hCall) ;
        bRC = validatorCalling.waitForLineEvent(hLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Place Call
        sipxCallConnect(hCall, "sip:foo@127.0.0.1:9100") ;

        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_DIALTONE, CALLSTATE_DIALTONE_UNKNOWN, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_REMOTE_OFFERING, CALLSTATE_REMOTE_OFFERING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_REMOTE_ALERTING, CALLSTATE_REMOTE_ALERTING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_NEWCALL, CALLSTATE_NEW_CALL_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_OFFERING, CALLSTATE_OFFERING_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_ALERTING, CALLSTATE_ALERTING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Hold
        CPPUNIT_ASSERT(sipxCallHold(hCall) == SIPX_RESULT_SUCCESS) ;

        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE_HELD, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_INACTIVE, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE_HELD, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, false) ;
        CPPUNIT_ASSERT(bRC) ;

        // Unhold
        CPPUNIT_ASSERT(sipxCallUnhold(hCall) == SIPX_RESULT_SUCCESS) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE_HELD, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Drop
        SIPX_CALL hDestroyedCall = hCall ;
        destroyCall(hCall) ;

        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hDestroyedCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE_HELD, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hDestroyedCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hDestroyedCall, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hDestroyedCall, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst, UniversalEventValidatorCallback, &validatorCalling), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, UniversalEventValidatorCallback, &validatorCalled), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, AutoAnswerCallback, NULL), SIPX_RESULT_SUCCESS) ;

        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hCalledLine), SIPX_RESULT_SUCCESS);
    }

    OsTask::delay(TEST_DELAY) ;

    checkForLeaks();
}



void sipXtapiTestSuite::testSendInfo()
{
    bool bRC ;
    EventValidator validatorCalling("testSendInfo.calling") ;
    EventValidator validatorCalled("testSendInfo.called") ;
    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestSendInfo (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        SIPX_CALL hCall ;
        SIPX_LINE hLine ;
        SIPX_LINE hReceivingLine ;
        SIPX_INFO hInfo;

        validatorCalling.reset() ;
        validatorCalled.reset() ;

        // Setup Auto-answer call back
        resetAutoAnswerCallback() ;
        sipxEventListenerAdd(g_hInst, UniversalEventValidatorCallback, &validatorCalling) ;
        sipxEventListenerAdd(g_hInst2, AutoAnswerCallback, NULL) ;
        sipxEventListenerAdd(g_hInst2, UniversalEventValidatorCallback, &validatorCalled) ;

        CPPUNIT_ASSERT_EQUAL(sipxConfigSetUserAgentName(g_hInst, "sipXtapiTest", false), SIPX_RESULT_SUCCESS) ;

        sipxLineAdd(g_hInst2, "sip:foo@127.0.0.1:9100", &hReceivingLine, CONTACT_LOCAL);
        bRC = validatorCalled.waitForLineEvent(hReceivingLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;


        createCall(&hLine, &hCall) ;
        bRC = validatorCalling.waitForLineEvent(hLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        sipxCallConnect(hCall, "sip:foo@127.0.0.1:9100") ;

       // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_DIALTONE, CALLSTATE_DIALTONE_UNKNOWN, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_REMOTE_OFFERING, CALLSTATE_REMOTE_OFFERING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_REMOTE_ALERTING, CALLSTATE_REMOTE_ALERTING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_NEWCALL, CALLSTATE_NEW_CALL_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_OFFERING, CALLSTATE_OFFERING_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_ALERTING, CALLSTATE_ALERTING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Send Info
        UtlString payload("abc") ;
        sipxCallSendInfo(&hInfo, hCall, "text/plain", payload, payload.length());
        bRC = validatorCalling.waitForInfoStatusEvent(hInfo, 0, 200, "OK") ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForInfoEvent(g_hAutoAnswerCallbackCall, g_hAutoAnswerCallbackLine, "sip:foo@127.0.0.1:9100", "sipXtapiTest", "text/plain", payload, payload.length()) ;
        CPPUNIT_ASSERT(bRC) ;

        // Send Another Info
        payload = "soylent green is people!" ;
        sipxCallSendInfo(&hInfo, hCall, "text/plain", payload, payload.length());
        bRC = validatorCalling.waitForInfoStatusEvent(hInfo, 0, 200, "OK") ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForInfoEvent(g_hAutoAnswerCallbackCall, g_hAutoAnswerCallbackLine, "sip:foo@127.0.0.1:9100", "sipXtapiTest", "text/plain", payload, payload.length()) ;
        CPPUNIT_ASSERT(bRC) ;

        SIPX_CALL hDestroyedCall = hCall ;
        destroyCall(hDestroyedCall) ;

        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE_HELD, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst, UniversalEventValidatorCallback, &validatorCalling), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, AutoAnswerCallback, NULL), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, UniversalEventValidatorCallback, &validatorCalled), SIPX_RESULT_SUCCESS) ;

        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hReceivingLine), SIPX_RESULT_SUCCESS);
    }
    OsTask::delay(TEST_DELAY) ;

    checkForLeaks();
}

void sipXtapiTestSuite::testSendInfoFailure()
{
    // TEST HACK: Disable INFO on the receiving end
    sipxConfigAllowMethod(g_hInst2, "INFO", false);

    bool bRC ;
    EventValidator validatorCalling("testSendInfoFailure.calling") ;
    EventValidator validatorCalled("testSendInfoFailure.called") ;
    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestSendInfoFailure (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        SIPX_CALL hCall ;
        SIPX_LINE hLine ;
        SIPX_LINE hReceivingLine ;
        SIPX_INFO hInfo;

        validatorCalling.reset() ;
        validatorCalled.reset() ;

        // Setup Auto-answer call back
        resetAutoAnswerCallback() ;
        sipxEventListenerAdd(g_hInst, UniversalEventValidatorCallback, &validatorCalling) ;
        sipxEventListenerAdd(g_hInst2, AutoAnswerCallback, NULL) ;
        sipxEventListenerAdd(g_hInst2, UniversalEventValidatorCallback, &validatorCalled) ;

        CPPUNIT_ASSERT_EQUAL(sipxConfigSetUserAgentName(g_hInst, "sipXtapiTest", false), SIPX_RESULT_SUCCESS) ;

        sipxLineAdd(g_hInst2, "sip:foo@127.0.0.1:9100", &hReceivingLine, CONTACT_LOCAL);
        bRC = validatorCalled.waitForLineEvent(hReceivingLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;


        createCall(&hLine, &hCall) ;
        bRC = validatorCalling.waitForLineEvent(hLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        sipxCallConnect(hCall, "sip:foo@127.0.0.1:9100") ;

       // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_DIALTONE, CALLSTATE_DIALTONE_UNKNOWN, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_REMOTE_OFFERING, CALLSTATE_REMOTE_OFFERING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_REMOTE_ALERTING, CALLSTATE_REMOTE_ALERTING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_NEWCALL, CALLSTATE_NEW_CALL_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_OFFERING, CALLSTATE_OFFERING_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_ALERTING, CALLSTATE_ALERTING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Send Info
        UtlString payload("the answer is 42") ;
        sipxCallSendInfo(&hInfo, hCall, "text/plain", payload, payload.length());
        bRC = validatorCalling.waitForInfoStatusEvent(hInfo, 2, 501, "Not Implemented") ;
        CPPUNIT_ASSERT(bRC) ;

        SIPX_CALL hDestroyedCall = hCall ;
        destroyCall(hDestroyedCall) ;

        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE_HELD, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst, UniversalEventValidatorCallback, &validatorCalling), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, AutoAnswerCallback, NULL), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, UniversalEventValidatorCallback, &validatorCalled), SIPX_RESULT_SUCCESS) ;

        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hReceivingLine), SIPX_RESULT_SUCCESS);
    }
    OsTask::delay(TEST_DELAY) ;

    // TEST HACK: Enable INFO on the receiving end
    sipxConfigAllowMethod(g_hInst2, "INFO", true);

    checkForLeaks();
}

void sipXtapiTestSuite::testSendInfoTimeout()
{
    // TEST HACK: now set the response code for testing
    SIPX_INSTANCE_DATA* pInst = (SIPX_INSTANCE_DATA*) g_hInst2;
    pInst->pMessageObserver->setTestResponseCode(408);

    EventValidator validatorCalling("testSendInfoTimeout.calling") ;
    EventValidator validatorCalled("testSendInfoTimeout.called") ;
    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestSendInfoTimeout (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        SIPX_CALL hCall ;
        SIPX_LINE hLine ;
        SIPX_LINE hReceivingLine ;
        SIPX_INFO hInfo;
        bool bRC ;

        validatorCalling.reset() ;
        validatorCalled.reset() ;

        // Setup Auto-answer call back
        resetAutoAnswerCallback() ;
        sipxEventListenerAdd(g_hInst, UniversalEventValidatorCallback, &validatorCalling) ;
        sipxEventListenerAdd(g_hInst2, AutoAnswerCallback, NULL) ;
        sipxEventListenerAdd(g_hInst2, UniversalEventValidatorCallback, &validatorCalled) ;

        CPPUNIT_ASSERT_EQUAL(sipxConfigSetUserAgentName(g_hInst, "sipXtapiTest", false), SIPX_RESULT_SUCCESS) ;

        sipxLineAdd(g_hInst2, "sip:foo@127.0.0.1:9100", &hReceivingLine, CONTACT_LOCAL);
        bRC = validatorCalled.waitForLineEvent(hReceivingLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;


        createCall(&hLine, &hCall) ;
        bRC = validatorCalling.waitForLineEvent(hLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        sipxCallConnect(hCall, "sip:foo@127.0.0.1:9100") ;

       // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_DIALTONE, CALLSTATE_DIALTONE_UNKNOWN, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_REMOTE_OFFERING, CALLSTATE_REMOTE_OFFERING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_REMOTE_ALERTING, CALLSTATE_REMOTE_ALERTING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_NEWCALL, CALLSTATE_NEW_CALL_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_OFFERING, CALLSTATE_OFFERING_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_ALERTING, CALLSTATE_ALERTING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Send Info
        UtlString payload("the answer is 42") ;
        sipxCallSendInfo(&hInfo, hCall, "text/plain", payload, payload.length());
        bRC = validatorCalling.waitForInfoStatusEvent(hInfo, 1, 408, "timed out") ;
        CPPUNIT_ASSERT(bRC) ;

        SIPX_CALL hDestroyedCall = hCall ;
        destroyCall(hDestroyedCall) ;

        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE_HELD, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst, UniversalEventValidatorCallback, &validatorCalling), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, AutoAnswerCallback, NULL), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, UniversalEventValidatorCallback, &validatorCalled), SIPX_RESULT_SUCCESS) ;

        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hReceivingLine), SIPX_RESULT_SUCCESS);
    }
    OsTask::delay(TEST_DELAY) ;

    // TEST HACK: Reset changes
    pInst->pMessageObserver->setTestResponseCode(0);

    checkForLeaks();
}


void sipXtapiTestSuite::testManualCallDialtone()
{
    SIPX_CALL hCall ;
    SIPX_LINE hLine ;

    // Setup
    sipxLineAdd(g_hInst, "sip:foo@10.1.1.120", &hLine) ;
    sipxCallCreate(g_hInst, hLine, &hCall) ;

    sipxCallPlayFileStart(hCall, "c:\\na_dialtone.wav", true, true, false) ;
    OsTask::delay(1000) ;
    sipxCallStartTone(hCall, ID_DTMF_0, true, false) ;
    OsTask::delay(1000) ;
    sipxCallStartTone(hCall, ID_DTMF_1, true, false) ;
    OsTask::delay(1000) ;
    sipxCallStartTone(hCall, ID_DTMF_2, true, false) ;
    OsTask::delay(1000) ;
    sipxCallPlayFileStop(hCall) ;
    OsTask::delay(5000) ;
    sipxCallPlayFileStart(hCall, "c:\\na_dialtone.wav", true, true, false) ;
    OsTask::delay(10000) ;
    sipxCallPlayFileStop(hCall) ;

    // Destory
    sipxCallDestroy(hCall) ;
    sipxLineRemove(hLine) ;
}
