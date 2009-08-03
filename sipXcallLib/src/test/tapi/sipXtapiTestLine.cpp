//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
//////////////////////////////////////////////////////////////////////////////

#include <string.h>

#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include "sipXtapiTest.h"
#include "TestRegistrar.h"
#include "EventRecorder.h"
#include "EventValidator.h"
#include "callbacks.h"

extern SIPX_INST g_hInst;
extern EventRecorder g_recorder ;
extern EventRecorder g_lineRecorder;
EventRecorder g_lineRecorder2;

extern SIPX_INST g_hInst2;
extern EventRecorder g_recorder2 ;

extern SIPX_INST g_hInst3;
extern EventRecorder g_recorder3 ;

extern SIPX_INST g_hInst4;
extern EventRecorder g_recorder4;

extern SIPX_INST g_hInstInfo;

extern bool g_bCallbackCalled;

void sipXtapiTestSuite::testLineAPI_Add()
{
    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        SIPX_LINE    hLine = SIPX_LINE_NULL ;
        SIPX_LINE    hLine2 = SIPX_LINE_NULL ;

        printf("\ntestLineAPI_Add (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        // Add Line
        CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst, "sip:bandreasen@pingtel.com", &hLine), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT(hLine) ;

        // Re-Add Line
        CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst, "sip:bandreasen@pingtel.com", &hLine2), SIPX_RESULT_FAILURE) ;
        CPPUNIT_ASSERT(hLine2 == SIPX_LINE_NULL) ;

        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS) ;
    }

    OsTask::delay(TEST_DELAY) ;
    checkForLeaks() ;
}

void sipXtapiTestSuite::testLineAPI_Remove()
{
    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        SIPX_LINE    hLine = SIPX_LINE_NULL ;

        printf("\ntestLineAPI_Remove (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        // Add a line to remove
        CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst, "sip:removeme@pingtel.com", &hLine), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT(hLine) ;

        // Remove it and remove it again
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_FAILURE) ;

        // Remove something invalid
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(SIPX_LINE_NULL), SIPX_RESULT_INVALID_ARGS) ;
    }

    OsTask::delay(TEST_DELAY) ;
    checkForLeaks() ;
}

void sipXtapiTestSuite::testLineAPI_Credential()
{
    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        SIPX_LINE    hLine = SIPX_LINE_NULL ;

        printf("\ntestLineAPI_Credential (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        // Add a line to remove
        CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst, "sip:credential@pingtel.com", &hLine), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT(hLine) ;

        CPPUNIT_ASSERT_EQUAL(sipxLineAddCredential(hLine, "userID", "passwd", "pingtel.com"), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxLineAddCredential(hLine, "userID", "passwd", "pingtel2.com"), SIPX_RESULT_SUCCESS) ;

        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS) ;
    }

    OsTask::delay(TEST_DELAY) ;
    checkForLeaks() ;
}

void sipXtapiTestSuite::testLineAPI_Get()
{
    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        SIPX_LINE    hLine = SIPX_LINE_NULL ;
        SIPX_LINE hLines[32] ;
        size_t nLines ;
      size_t i;
        const char* szURI = "sip:removeme@pingtel.com" ;

        printf("\ntestLineAPI_Get (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        // First clear any lines
        CPPUNIT_ASSERT_EQUAL(sipxLineGet(g_hInst, hLines, 32, nLines), SIPX_RESULT_SUCCESS) ;
        for (i=0; i<nLines; i++)
        {
            CPPUNIT_ASSERT(hLines[i] != SIPX_LINE_NULL) ;
            CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLines[i]), SIPX_RESULT_SUCCESS) ;
        }

        // Make sure the list is empty
        nLines = 203431 ;
        CPPUNIT_ASSERT_EQUAL(sipxLineGet(g_hInst, hLines, 32, nLines), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(nLines, (size_t) 0) ;

        // Add and element and verify it is present
        CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst, szURI, &hLine), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT(hLine) ;

        CPPUNIT_ASSERT_EQUAL(sipxLineGet(g_hInst, hLines, 32, nLines), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(nLines, (size_t) 1) ;
        CPPUNIT_ASSERT(hLines[0] != SIPX_LINE_NULL) ;

        char cBuf[256] ;
        size_t nActual ;
        CPPUNIT_ASSERT_EQUAL(sipxLineGetURI(hLine, cBuf, sizeof(cBuf), nActual), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT(strcasecmp(cBuf, szURI) == 0) ;
        CPPUNIT_ASSERT_EQUAL(strlen(cBuf)+1, nActual) ;

      // Clean up
      CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLines[i]), SIPX_RESULT_SUCCESS) ;
    }

    OsTask::delay(TEST_DELAY) ;
    checkForLeaks() ;
}

void sipXtapiTestSuite::testLineAPI_GetURI()
{
    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        SIPX_LINE    hLine = SIPX_LINE_NULL ;
        const char* szURI = "sip:removeme@pingtel.com" ;

        printf("\ntestLineAPI_GetURI (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        // Add and element
        CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst, szURI, &hLine), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT(hLine) ;


        // Standard get
        char cBuf[256] ;
        size_t nActual ;
        CPPUNIT_ASSERT_EQUAL(sipxLineGetURI(hLine, cBuf, sizeof(cBuf), nActual), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT(strcasecmp(cBuf, szURI) == 0) ;
        CPPUNIT_ASSERT_EQUAL(strlen(cBuf)+1, nActual) ;

        // Ask for length
        CPPUNIT_ASSERT_EQUAL(sipxLineGetURI(hLine, NULL, sizeof(cBuf), nActual), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(nActual, strlen(szURI) + 1) ;

        // Small Buffer (doesn't stomp, etc)
        strcpy(cBuf, "1234567890") ;
        CPPUNIT_ASSERT_EQUAL(sipxLineGetURI(hLine, cBuf, 5, nActual), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT(strcasecmp(cBuf, "sip:") == 0) ;
        CPPUNIT_ASSERT(cBuf[5] == '6') ;

        // Clean up
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS) ;
    }

    OsTask::delay(TEST_DELAY) ;
    checkForLeaks() ;
}




bool linetest_verify_lineurl(SIPX_EVENT_CATEGORY event,
                             void* pInfo,
                             void* pUserData)
{
    if (event == EVENT_CATEGORY_LINESTATE)
    {
        SIPX_LINESTATE_INFO* pLineInfo = (SIPX_LINESTATE_INFO*) pInfo;
        SIPX_LINE hLine = pLineInfo->hLine;


        g_lineRecorder2.addEvent(pLineInfo->hLine, pLineInfo->event, pLineInfo->cause);

        char szBuffer[500] ;
        size_t nBuffer ;
        CPPUNIT_ASSERT_EQUAL(sipxLineGetURI(hLine, szBuffer, sizeof(szBuffer), nBuffer), SIPX_RESULT_SUCCESS) ;

        // printf("comparing %s to %s\n", pUserData, szBuffer) ;
        CPPUNIT_ASSERT(strcmp((char*) pUserData, szBuffer) == 0) ;
    }
    return true;
}


/**
 * Source Call:
 *   Line1: george@thejungle.com
 *   Line2: jane@thejungle.com
 *
 * Target Call:
 *   Line1: bob@127.0.0.1:9100
 *   Line2: jill@127.0.0.1:9100
 *
 * This test verifies that the line handles return with the sipX events are sane
 */
#define URL_GEORGE_AT_JUNGLE        "sip:george@thejungle.com"
#define URL_JANE_AT_JUNGLE          "sip:jane@thejungle.com"
#define URL_BOB_AT_LOCALHOST        "sip:bob@127.0.0.1:9100"
#define URL_JILL_AT_LOCALHOST       "sip:jill@127.0.0.1:9100"
#define URL_BOGUS_AT_LOCALHOST      "sip:bogus@127.0.0.1:900"
#define URL_EMPTY_STRING            ""
void sipXtapiTestSuite::testLines()
{
    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        SIPX_CALL hCall ;
        SIPX_LINE hLineGeorge ;
        SIPX_LINE hLineJane ;
        SIPX_LINE hLineBob ;
        SIPX_LINE hLineJill ;

        printf("\ntestLines (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst, URL_GEORGE_AT_JUNGLE, &hLineGeorge), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst, URL_JANE_AT_JUNGLE, &hLineJane), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst2, URL_BOB_AT_LOCALHOST, &hLineBob), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst2, URL_JILL_AT_LOCALHOST, &hLineJill), SIPX_RESULT_SUCCESS) ;

        /**
         * Make call as George to Jill
         */
        // Setup Auto-answer call back
        sipxEventListenerAdd(g_hInst2, basicCall_CallBack_Receive, (void*)URL_JILL_AT_LOCALHOST) ;

        createCall(hLineGeorge, &hCall) ;

        sipxCallConnect(hCall, URL_JILL_AT_LOCALHOST) ;
        OsTask::delay(CALL_DELAY*2) ;
        destroyCall(hCall) ;
        OsTask::delay(CALL_DELAY*4) ;

        // Remove George's line
        g_lineRecorder2.clear() ;

        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, basicCall_CallBack_Receive, (void*)URL_JILL_AT_LOCALHOST), SIPX_RESULT_SUCCESS) ;

        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLineGeorge), SIPX_RESULT_SUCCESS) ;
        OsTask::delay(CALL_DELAY*2);

        /**
         * Make call as Jane to Bob
         */
        sipxEventListenerAdd(g_hInst, linetest_verify_lineurl, (void*)URL_JANE_AT_JUNGLE) ;

        sipxEventListenerAdd(g_hInst2, basicCall_CallBack_Receive, (void*)URL_BOB_AT_LOCALHOST) ;
        createCall(hLineJane, &hCall) ;

        sipxCallConnect(hCall, URL_BOB_AT_LOCALHOST) ;
        OsTask::delay(CALL_DELAY*2) ;
        destroyCall(hCall) ;
        OsTask::delay(CALL_DELAY*4) ;

        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst, linetest_verify_lineurl, (void*)URL_JANE_AT_JUNGLE), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, basicCall_CallBack_Receive, (void*)URL_BOB_AT_LOCALHOST), SIPX_RESULT_SUCCESS) ;

        /**
         * Make call as Jane to BOGUS (line not defined)
         */
        sipxEventListenerAdd(g_hInst2, basicCall_CallBack_Receive, (void*)URL_EMPTY_STRING) ;
        createCall(hLineJane, &hCall) ;

        sipxCallConnect(hCall, URL_BOGUS_AT_LOCALHOST) ;
        OsTask::delay(22000) ;
        destroyCall(hCall) ;
        OsTask::delay(CALL_DELAY*4) ;

        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, basicCall_CallBack_Receive, (void*)URL_EMPTY_STRING), SIPX_RESULT_SUCCESS) ;

        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLineJane), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLineBob), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLineJill), SIPX_RESULT_SUCCESS) ;
    }

    OsTask::delay(TEST_DELAY) ;
    checkForLeaks();
}

bool lineCallback(SIPX_EVENT_CATEGORY event,
                  void* pInfo,
                  void* pUser)
{
    if (event == EVENT_CATEGORY_LINESTATE)
    {
        SIPX_LINESTATE_INFO* pLineInfo = (SIPX_LINESTATE_INFO*) pInfo;
        g_lineRecorder.addEvent(pLineInfo->hLine, pLineInfo->event, pLineInfo->cause);
    }
    return true;
}


void sipXtapiTestSuite::testLineEvents()
{
    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestLineEvents (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        g_lineRecorder.clear() ;

        CPPUNIT_ASSERT_EQUAL(sipxEventListenerAdd(g_hInst, lineCallback, NULL), SIPX_RESULT_SUCCESS);

        // TODO - we need to figure out how to better test registrar interaction.
        // It might require creating a test registrar that responds to REGISTER messages.


        SIPX_LINE hLine;
        CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst, "sip:115@hopefullythisisnotarealdomain.com", &hLine), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxLineRegister(hLine, true), SIPX_RESULT_SUCCESS) ;

        g_lineRecorder.addCompareEvent(hLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL);
        g_lineRecorder.addCompareEvent(hLine, LINESTATE_REGISTER_FAILED, LINESTATE_REGISTER_FAILED_COULD_NOT_CONNECT);

        OsTask::delay(500);

        CPPUNIT_ASSERT(g_lineRecorder.compare()) ;

        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst, lineCallback, NULL), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS) ;
    }

    OsTask::delay(TEST_DELAY) ;
    checkForLeaks();
}


void sipXtapiTestSuite::testLineAliases()
{
    bool bRC ;
    EventValidator validatorCalling("testLineAliases.calling") ;
    EventValidator validatorCalled("testLineAliases.called") ;

    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestLineAliases (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);
        SIPX_CALL hCall ;
        SIPX_LINE hLine ;
        SIPX_LINE hReceivingLine;
        SIPX_RESULT rc ;

        validatorCalling.reset() ;
        validatorCalled.reset() ;

        // Setup Auto-answer call back
        resetAutoAnswerCallback() ;
        sipxEventListenerAdd(g_hInst2, AutoAnswerCallback, NULL) ;
        sipxEventListenerAdd(g_hInst2, UniversalEventValidatorCallback, &validatorCalled) ;
        sipxEventListenerAdd(g_hInst, UniversalEventValidatorCallback, &validatorCalling) ;

        sipxLineAdd(g_hInst2, "sip:foo@127.0.0.1:9100", &hReceivingLine, CONTACT_AUTO);
        bRC = validatorCalled.waitForLineEvent(hReceivingLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        rc = sipxLineAddAlias(hReceivingLine, "sip:alias@127.0.0.1:9100") ;
        CPPUNIT_ASSERT(rc == SIPX_RESULT_SUCCESS) ;

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
        bRC = validatorCalled.waitForCallEvent(hReceivingLine, g_hAutoAnswerCallbackCall, CALLSTATE_NEWCALL, CALLSTATE_NEW_CALL_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(hReceivingLine, g_hAutoAnswerCallbackCall, CALLSTATE_OFFERING, CALLSTATE_OFFERING_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(hReceivingLine, g_hAutoAnswerCallbackCall, CALLSTATE_ALERTING, CALLSTATE_ALERTING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(hReceivingLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(hReceivingLine, g_hAutoAnswerCallbackCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
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
        bRC = validatorCalled.waitForCallEvent(hReceivingLine, g_hAutoAnswerCallbackCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(hReceivingLine, g_hAutoAnswerCallbackCall, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(hReceivingLine, g_hAutoAnswerCallbackCall, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;


        validatorCalling.reset() ;
        validatorCalled.reset() ;
        resetAutoAnswerCallback() ;
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS);

        createCall(&hLine, &hCall) ;
        bRC = validatorCalling.waitForLineEvent(hLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        sipxCallConnect(hCall, "sip:alias@127.0.0.1:9100") ;

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
        bRC = validatorCalled.waitForCallEvent(hReceivingLine, g_hAutoAnswerCallbackCall, CALLSTATE_NEWCALL, CALLSTATE_NEW_CALL_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(hReceivingLine, g_hAutoAnswerCallbackCall, CALLSTATE_OFFERING, CALLSTATE_OFFERING_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(hReceivingLine, g_hAutoAnswerCallbackCall, CALLSTATE_ALERTING, CALLSTATE_ALERTING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(hReceivingLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(hReceivingLine, g_hAutoAnswerCallbackCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;

        connectionId = -1;

        CPPUNIT_ASSERT_EQUAL(sipxCallGetConnectionId(hCall, connectionId), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT(connectionId != -1) ;

        hDestroyedCall = hCall ;
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
        bRC = validatorCalled.waitForCallEvent(hReceivingLine, g_hAutoAnswerCallbackCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(hReceivingLine, g_hAutoAnswerCallbackCall, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled.waitForCallEvent(hReceivingLine, g_hAutoAnswerCallbackCall, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, true) ;
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


void sipXtapiTestSuite::testRegistration()
{
    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        TestRegistrar testRegistrar;
        SIPX_RESULT rc ;

        testRegistrar.init();
        printf("\ntestRegistration (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);
        g_lineRecorder.clear() ;

        rc = sipxEventListenerAdd(g_hInst4, lineCallback, NULL) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;

        rc = sipxConfigSetOutboundProxy(g_hInst4, "127.0.0.1:5070") ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;

        SIPX_LINE hLine;
        rc =sipxLineAdd(g_hInst4, "sip:anon@127.0.0.1:12070", &hLine) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;
        rc =sipxLineRegister(hLine, true) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;

        g_lineRecorder.addCompareEvent(hLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL);
        g_lineRecorder.addCompareEvent(hLine, LINESTATE_REGISTERING, LINESTATE_REGISTERING_NORMAL);
        g_lineRecorder.addCompareEvent(hLine, LINESTATE_REGISTERED, LINESTATE_REGISTERED_NORMAL);

        OsTask::delay(1000);

        CPPUNIT_ASSERT(g_lineRecorder.compare()) ;
        g_lineRecorder.clear() ;

        // try to register, for a user that needs credentials,
        // but without credentials
        rc = sipxLineRemove(hLine);
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;

        g_lineRecorder.addCompareEvent(hLine, LINESTATE_UNREGISTERING, LINESTATE_UNREGISTERING_NORMAL);
        g_lineRecorder.addCompareEvent(hLine, LINESTATE_UNREGISTERED, LINESTATE_UNREGISTERED_NORMAL);
        OsTask::delay(1000);
        CPPUNIT_ASSERT(g_lineRecorder.compare()) ;

        g_lineRecorder.clear() ;

        rc = sipxLineAdd(g_hInst4, "sip:mike@127.0.0.1:12070", &hLine) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;
        rc = sipxLineRegister(hLine, true) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;

        g_lineRecorder.addCompareEvent(hLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL);
        g_lineRecorder.addCompareEvent(hLine, LINESTATE_REGISTERING, LINESTATE_REGISTERING_NORMAL);
        g_lineRecorder.addCompareEvent(hLine, LINESTATE_REGISTER_FAILED, LINESTATE_REGISTER_FAILED_NOT_AUTHORIZED);

        OsTask::delay(1000);

        CPPUNIT_ASSERT(g_lineRecorder.compare()) ;

        g_lineRecorder.clear() ;

        rc = sipxLineAddCredential(hLine, "mike", "1234", "TestRegistrar") ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;
        rc = sipxLineRegister(hLine, true) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;

        g_lineRecorder.addCompareEvent(hLine, LINESTATE_REGISTERING, LINESTATE_REGISTERING_NORMAL);
        g_lineRecorder.addCompareEvent(hLine, LINESTATE_REGISTERED, LINESTATE_REGISTERED_NORMAL);

        OsTask::delay(1000);

        CPPUNIT_ASSERT(g_lineRecorder.compare()) ;

        rc = sipxLineRemove(hLine) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;

        g_lineRecorder.addCompareEvent(hLine, LINESTATE_UNREGISTERING, LINESTATE_UNREGISTERING_NORMAL);
        g_lineRecorder.addCompareEvent(hLine, LINESTATE_UNREGISTERED, LINESTATE_UNREGISTERED_NORMAL);
        OsTask::delay(1000);
        CPPUNIT_ASSERT(g_lineRecorder.compare()) ;
        rc = sipxEventListenerRemove(g_hInst4, lineCallback, NULL) ;
        CPPUNIT_ASSERT_EQUAL(rc, SIPX_RESULT_SUCCESS) ;

    }

    OsTask::delay(TEST_DELAY) ;
    checkForLeaks() ;
}


void sipXtapiTestSuite::testBadRegistrarRegistration()
{
    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        TestRegistrar testRegistrar;

        testRegistrar.init();

        printf("\ntestBadRegistrarRegistration (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);
        g_lineRecorder.clear() ;

        CPPUNIT_ASSERT_EQUAL(sipxEventListenerAdd(g_hInst4, lineCallback, NULL), SIPX_RESULT_SUCCESS);

        CPPUNIT_ASSERT_EQUAL(sipxConfigSetOutboundProxy(g_hInst4, "127.0.0.1:5070"), SIPX_RESULT_SUCCESS);

        // receiving a 480 response
        SIPX_LINE hLine;
        CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst4, "sip:xyzzy480@127.0.0.1:12070", &hLine), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxLineRegister(hLine, true), SIPX_RESULT_SUCCESS) ;

        g_lineRecorder.addCompareEvent(hLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL);
        g_lineRecorder.addCompareEvent(hLine, LINESTATE_REGISTERING, LINESTATE_REGISTERING_NORMAL);
        g_lineRecorder.addCompareEvent(hLine, LINESTATE_REGISTER_FAILED, LINESTATE_CAUSE_UNKNOWN);
        OsTask::delay(1000);
        CPPUNIT_ASSERT(g_lineRecorder.compare()) ;
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS) ;
        g_lineRecorder.clear() ;

        // receiving a 503 response
        CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst4, "sip:xyzzy503@127.0.0.1:12070", &hLine), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxLineRegister(hLine, true), SIPX_RESULT_SUCCESS) ;
        g_lineRecorder.addCompareEvent(hLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL);
        g_lineRecorder.addCompareEvent(hLine, LINESTATE_REGISTERING, LINESTATE_REGISTERING_NORMAL);
        g_lineRecorder.addCompareEvent(hLine, LINESTATE_REGISTER_FAILED, LINESTATE_CAUSE_UNKNOWN);
        OsTask::delay(1000);
        CPPUNIT_ASSERT(g_lineRecorder.compare()) ;
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS) ;
        g_lineRecorder.clear() ;

        // receiving a 600 response
        CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst4, "sip:xyzzy600@127.0.0.1:12070", &hLine), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxLineRegister(hLine, true), SIPX_RESULT_SUCCESS) ;
        g_lineRecorder.addCompareEvent(hLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL);
        g_lineRecorder.addCompareEvent(hLine, LINESTATE_REGISTERING, LINESTATE_REGISTERING_NORMAL);
        g_lineRecorder.addCompareEvent(hLine, LINESTATE_REGISTER_FAILED, LINESTATE_CAUSE_UNKNOWN);
        OsTask::delay(1000);
        CPPUNIT_ASSERT(g_lineRecorder.compare()) ;
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS) ;
        g_lineRecorder.clear() ;

        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst4, lineCallback, NULL), SIPX_RESULT_SUCCESS);
    }

    OsTask::delay(TEST_DELAY) ;
    checkForLeaks() ;
}

void sipXtapiTestSuite::testReRegistration()
{
    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        TestRegistrar testRegistrar;

        testRegistrar.init();

        printf("\ntestReRegistration (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);
        g_lineRecorder.clear() ;

        CPPUNIT_ASSERT_EQUAL(sipxEventListenerAdd(g_hInst4, lineCallback, NULL), SIPX_RESULT_SUCCESS);

        CPPUNIT_ASSERT_EQUAL(sipxConfigSetOutboundProxy(g_hInst4, "127.0.0.1:5070"), SIPX_RESULT_SUCCESS);

        SIPX_LINE hLine;
        CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst4, "sip:mike@127.0.0.1:12070", &hLine), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxConfigSetRegisterExpiration(g_hInst4, 10), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(sipxLineAddCredential(hLine, "mike", "1234", "TestRegistrar"), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxLineRegister(hLine, true), SIPX_RESULT_SUCCESS) ;

        g_lineRecorder.addCompareEvent(hLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL);
        g_lineRecorder.addCompareEvent(hLine, LINESTATE_REGISTERING, LINESTATE_REGISTERING_NORMAL);
        g_lineRecorder.addCompareEvent(hLine, LINESTATE_REGISTERED, LINESTATE_REGISTERED_NORMAL);
        g_lineRecorder.addCompareEvent(hLine, LINESTATE_REGISTERING, LINESTATE_REGISTERING_NORMAL);
        g_lineRecorder.addCompareEvent(hLine, LINESTATE_REGISTERED, LINESTATE_REGISTERED_NORMAL);
        g_lineRecorder.addCompareEvent(hLine, LINESTATE_REGISTERING, LINESTATE_REGISTERING_NORMAL);
        g_lineRecorder.addCompareEvent(hLine, LINESTATE_REGISTERED, LINESTATE_REGISTERED_NORMAL);

        printf("\nWaiting for 12 seconds! (for the re-registration test)");
        OsTask::delay(12000);
        CPPUNIT_ASSERT(g_lineRecorder.compare()) ;

        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS) ;
        OsTask::delay(1000);
        g_lineRecorder.clear() ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst4, lineCallback, NULL), SIPX_RESULT_SUCCESS);


    }

    OsTask::delay(TEST_DELAY) ;
    checkForLeaks() ;
}
