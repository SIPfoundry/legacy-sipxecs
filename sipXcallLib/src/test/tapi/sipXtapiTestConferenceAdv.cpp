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


void sipXtapiTestSuite::testConfHoldIndividual()
{
    bool bRC ;
    EventValidator validatorCalling("testConfHoldIndividual.calling") ;
    EventValidator validatorCalled1("testConfHoldIndividual.called1") ;
    EventValidator validatorCalled2("testConfHoldIndividual.called2") ;

    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestConfHoldIndividual (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        SIPX_CALL hCall1 ;
        SIPX_CALL hCall2 ;
        SIPX_CONF hConf ;
        SIPX_LINE hLine ;
        SIPX_LINE hReceivingLine1;
        SIPX_LINE hReceivingLine2;

        SIPX_CALL hCheckCalls[10] ;
        size_t nCheckCalls ;

        validatorCalling.reset() ;
        validatorCalled1.reset() ;
        validatorCalled2.reset() ;

        // Setup Auto-answer call back
        resetAutoAnswerCallback() ;
        resetAutoAnswerCallback2() ;
        sipxEventListenerAdd(g_hInst, UniversalEventValidatorCallback, &validatorCalling) ;
        sipxEventListenerAdd(g_hInst2, UniversalEventValidatorCallback, &validatorCalled1) ;
        sipxEventListenerAdd(g_hInst2, AutoAnswerCallback, NULL) ;
        sipxEventListenerAdd(g_hInst3, UniversalEventValidatorCallback, &validatorCalled2) ;
        sipxEventListenerAdd(g_hInst3, AutoAnswerCallback2, NULL) ;

        // Create Lines
        CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst, "sip:foo@127.0.0.1:8000", &hLine), SIPX_RESULT_SUCCESS) ;
        bRC = validatorCalling.waitForLineEvent(hLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst2, "sip:foo@127.0.0.1:9100", &hReceivingLine1, CONTACT_AUTO), SIPX_RESULT_SUCCESS) ;
        bRC = validatorCalled1.waitForLineEvent(hReceivingLine1, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst3, "sip:foo@127.0.0.1:10000", &hReceivingLine2, CONTACT_AUTO), SIPX_RESULT_SUCCESS) ;
        bRC = validatorCalled2.waitForLineEvent(hReceivingLine2, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Setup first leg
        CPPUNIT_ASSERT_EQUAL(sipxConferenceCreate(g_hInst, &hConf), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxConferenceAdd(hConf, hLine, "sip:blah@127.0.0.1:9100", &hCall1), SIPX_RESULT_SUCCESS) ;

        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_DIALTONE, CALLSTATE_DIALTONE_CONFERENCE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_REMOTE_OFFERING, CALLSTATE_REMOTE_OFFERING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_REMOTE_ALERTING, CALLSTATE_REMOTE_ALERTING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_NEWCALL, CALLSTATE_NEW_CALL_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_OFFERING, CALLSTATE_OFFERING_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_ALERTING, CALLSTATE_ALERTING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Make sure we don't have any unexpected events
        CPPUNIT_ASSERT(!validatorCalling.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled1.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled2.validateNoWaitingEvent()) ;

        // Validate Get Calls
        CPPUNIT_ASSERT_EQUAL(sipxConferenceGetCalls(hConf, hCheckCalls, 10, nCheckCalls), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT(nCheckCalls == 1) ;
        CPPUNIT_ASSERT(hCheckCalls[0] == hCall1) ;

        // Setup 2nd Leg
        CPPUNIT_ASSERT_EQUAL(sipxConferenceAdd(hConf, hLine, "sip:blah2@127.0.0.1:10000", &hCall2), SIPX_RESULT_SUCCESS) ;

        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_DIALTONE, CALLSTATE_DIALTONE_CONFERENCE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_REMOTE_OFFERING, CALLSTATE_REMOTE_OFFERING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_REMOTE_ALERTING, CALLSTATE_REMOTE_ALERTING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_NEWCALL, CALLSTATE_NEW_CALL_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_OFFERING, CALLSTATE_OFFERING_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_ALERTING, CALLSTATE_ALERTING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Make sure we don't have any unexpected events
        CPPUNIT_ASSERT(!validatorCalling.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled1.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled2.validateNoWaitingEvent()) ;

        // Validate Get Calls
        CPPUNIT_ASSERT_EQUAL(sipxConferenceGetCalls(hConf, hCheckCalls, 10, nCheckCalls), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT(nCheckCalls == 2) ;
        CPPUNIT_ASSERT(hCheckCalls[0] == hCall1) ;
        CPPUNIT_ASSERT(hCheckCalls[1] == hCall2) ;
        CPPUNIT_ASSERT(hCheckCalls[0] != hCheckCalls[1]) ;


        // Hold Call 1
        CPPUNIT_ASSERT_EQUAL(sipxCallHold(hCall1), SIPX_RESULT_SUCCESS) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE_HELD, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_INACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE_HELD, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Hold Call 2
        CPPUNIT_ASSERT_EQUAL(sipxCallHold(hCall2), SIPX_RESULT_SUCCESS) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE_HELD, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_INACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE_HELD, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Unhold call 1
        CPPUNIT_ASSERT_EQUAL(sipxCallUnhold(hCall1), SIPX_RESULT_SUCCESS) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;


        // Unhold call 2
        CPPUNIT_ASSERT_EQUAL(sipxCallUnhold(hCall2), SIPX_RESULT_SUCCESS) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;


        // Tear down conference
        CPPUNIT_ASSERT_EQUAL(sipxConferenceDestroy(hConf), SIPX_RESULT_SUCCESS) ;
        // Validate Calling Side

        // TODO: FIX PT-1
        //bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        //CPPUNIT_ASSERT(bRC) ;
        //bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        //CPPUNIT_ASSERT(bRC) ;

        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Sides
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Make sure we don't have any unexpected events
        CPPUNIT_ASSERT(!validatorCalling.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled1.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled2.validateNoWaitingEvent()) ;

        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst, UniversalEventValidatorCallback, &validatorCalling), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, UniversalEventValidatorCallback, &validatorCalled1), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, AutoAnswerCallback, NULL), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst3, UniversalEventValidatorCallback, &validatorCalled2), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst3, AutoAnswerCallback2, NULL), SIPX_RESULT_SUCCESS) ;

        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hReceivingLine1), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hReceivingLine2), SIPX_RESULT_SUCCESS);
    }

    OsTask::delay(TEST_DELAY) ;
    checkForLeaks();
}


// Setup conferencing using join (first party), hang up first party then second.
void sipXtapiTestSuite::testConfJoin()
{
    bool bRC ;
    EventValidator validatorCalling("testConfJoin.calling") ;
    EventValidator validatorCalled1("testConfJoin.called1") ;
    EventValidator validatorCalled2("testConfJoin.called2") ;

    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestConfJoin (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        SIPX_CALL hCall1 ;
        SIPX_CALL hCall2 ;
        SIPX_CONF hConf ;
        SIPX_LINE hLine ;
        SIPX_LINE hReceivingLine1;
        SIPX_LINE hReceivingLine2;

        SIPX_CALL hCheckCalls[10] ;
        size_t nCheckCalls ;

        validatorCalling.reset() ;
        validatorCalled1.reset() ;
        validatorCalled2.reset() ;

        // Setup Auto-answer call back
        resetAutoAnswerCallback() ;
        resetAutoAnswerCallback2() ;
        sipxEventListenerAdd(g_hInst, UniversalEventValidatorCallback, &validatorCalling) ;
        sipxEventListenerAdd(g_hInst2, UniversalEventValidatorCallback, &validatorCalled1) ;
        sipxEventListenerAdd(g_hInst2, AutoAnswerCallback, NULL) ;
        sipxEventListenerAdd(g_hInst3, UniversalEventValidatorCallback, &validatorCalled2) ;
        sipxEventListenerAdd(g_hInst3, AutoAnswerCallback2, NULL) ;

        // Create Lines
        CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst, "sip:foo@127.0.0.1:8000", &hLine), SIPX_RESULT_SUCCESS) ;
        bRC = validatorCalling.waitForLineEvent(hLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst2, "sip:foo@127.0.0.1:9100", &hReceivingLine1, CONTACT_AUTO), SIPX_RESULT_SUCCESS) ;
        bRC = validatorCalled1.waitForLineEvent(hReceivingLine1, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst3, "sip:foo@127.0.0.1:10000", &hReceivingLine2, CONTACT_AUTO), SIPX_RESULT_SUCCESS) ;
        bRC = validatorCalled2.waitForLineEvent(hReceivingLine2, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Setup first leg
        CPPUNIT_ASSERT_EQUAL(sipxCallCreate(g_hInst, hLine, &hCall1), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxCallConnect(hCall1, "sip:blah@127.0.0.1:9100"), SIPX_RESULT_SUCCESS) ;

        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_DIALTONE, CALLSTATE_DIALTONE_UNKNOWN, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_REMOTE_OFFERING, CALLSTATE_REMOTE_OFFERING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_REMOTE_ALERTING, CALLSTATE_REMOTE_ALERTING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_NEWCALL, CALLSTATE_NEW_CALL_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_OFFERING, CALLSTATE_OFFERING_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_ALERTING, CALLSTATE_ALERTING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Make sure we don't have any unexpected events
        CPPUNIT_ASSERT(!validatorCalling.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled1.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled2.validateNoWaitingEvent()) ;

        // Do the join
        CPPUNIT_ASSERT_EQUAL(sipxConferenceCreate(g_hInst, &hConf), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxConferenceJoin(hConf, hCall1), SIPX_RESULT_SUCCESS) ;

        // Validate Get Calls
        CPPUNIT_ASSERT_EQUAL(sipxConferenceGetCalls(hConf, hCheckCalls, 10, nCheckCalls), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT(nCheckCalls == 1) ;
        CPPUNIT_ASSERT(hCheckCalls[0] == hCall1) ;

        // Setup 2nd Leg
        CPPUNIT_ASSERT_EQUAL(sipxConferenceAdd(hConf, hLine, "sip:blah2@127.0.0.1:10000", &hCall2), SIPX_RESULT_SUCCESS) ;

        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_DIALTONE, CALLSTATE_DIALTONE_CONFERENCE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_REMOTE_OFFERING, CALLSTATE_REMOTE_OFFERING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_REMOTE_ALERTING, CALLSTATE_REMOTE_ALERTING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_NEWCALL, CALLSTATE_NEW_CALL_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_OFFERING, CALLSTATE_OFFERING_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_ALERTING, CALLSTATE_ALERTING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Make sure we don't have any unexpected events
        CPPUNIT_ASSERT(!validatorCalling.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled1.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled2.validateNoWaitingEvent()) ;

        // Validate Get Calls
        CPPUNIT_ASSERT_EQUAL(sipxConferenceGetCalls(hConf, hCheckCalls, 10, nCheckCalls), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT(nCheckCalls == 2) ;
        CPPUNIT_ASSERT(hCheckCalls[0] == hCall1) ;
        CPPUNIT_ASSERT(hCheckCalls[1] == hCall2) ;
        CPPUNIT_ASSERT(hCheckCalls[0] != hCheckCalls[1]) ;

        // Tear Down first leg
        CPPUNIT_ASSERT_EQUAL(sipxConferenceRemove(hConf, hCall1), SIPX_RESULT_SUCCESS) ;
        // TODO: FIX PT-1
        // bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        // CPPUNIT_ASSERT(bRC) ;

        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Sides
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Make sure we don't have any unexpected events
        CPPUNIT_ASSERT(!validatorCalling.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled1.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled2.validateNoWaitingEvent()) ;

        // Validate Get Calls
        CPPUNIT_ASSERT_EQUAL(sipxConferenceGetCalls(hConf, hCheckCalls, 10, nCheckCalls), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT(nCheckCalls == 1) ;
        CPPUNIT_ASSERT(hCheckCalls[0] == hCall2) ;

        // Tear Down 2nd leg
        CPPUNIT_ASSERT_EQUAL(sipxConferenceRemove(hConf, hCall2), SIPX_RESULT_SUCCESS) ;

        // TODO: FIX PT-1
        // bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        // CPPUNIT_ASSERT(bRC) ;

        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Sides
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Make sure we don't have any unexpected events
        CPPUNIT_ASSERT(!validatorCalling.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled1.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled2.validateNoWaitingEvent()) ;

        // Tear down conference
        CPPUNIT_ASSERT_EQUAL(sipxConferenceDestroy(hConf), SIPX_RESULT_SUCCESS) ;

        // Make sure we don't have any unexpected events
        CPPUNIT_ASSERT(!validatorCalling.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled1.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled2.validateNoWaitingEvent()) ;

        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst, UniversalEventValidatorCallback, &validatorCalling), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, UniversalEventValidatorCallback, &validatorCalled1), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, AutoAnswerCallback, NULL), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst3, UniversalEventValidatorCallback, &validatorCalled2), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst3, AutoAnswerCallback2, NULL), SIPX_RESULT_SUCCESS) ;

        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hReceivingLine1), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hReceivingLine2), SIPX_RESULT_SUCCESS) ;
    }

    OsTask::delay(TEST_DELAY*2) ;
    checkForLeaks() ;
}


// Set up conference using join.  Check that the right events are generated.
void sipXtapiTestSuite::testConfJoin2()
{
   bool bRC ;
   EventValidator validatorCalled("testConfJoin2.called") ;

   printf("*** Starting testConfJoin2 ***\n");

   for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
   {
      printf("\ntestConfJoin2 (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

      SIPX_CALL hCall1 ;
      SIPX_CALL hCall2 ;
      SIPX_CONF hConf ;
      SIPX_LINE hCalledLine ;
      SIPX_LINE hCallingLine1;
      SIPX_LINE hCallingLine2;

      validatorCalled.reset() ;

      // Set up validators.
      sipxEventListenerAdd(g_hInst, UniversalEventValidatorCallback, &validatorCalled) ;

      // Create Lines
      CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst, "sip:foo@127.0.0.1:8000", &hCalledLine), SIPX_RESULT_SUCCESS) ;
      bRC = validatorCalled.waitForLineEvent(hCalledLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
      CPPUNIT_ASSERT(bRC) ;

      CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst2, "sip:foo@127.0.0.1:9100", &hCallingLine1, CONTACT_AUTO), SIPX_RESULT_SUCCESS) ;
      CPPUNIT_ASSERT(bRC) ;

      CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst3, "sip:foo@127.0.0.1:10000", &hCallingLine2, CONTACT_AUTO), SIPX_RESULT_SUCCESS) ;
      CPPUNIT_ASSERT(bRC) ;

      // Set up first leg
      CPPUNIT_ASSERT_EQUAL(sipxCallCreate(g_hInst, hCallingLine1, &hCall1), SIPX_RESULT_SUCCESS) ;
      CPPUNIT_ASSERT_EQUAL(sipxCallConnect(hCall1, "sip:blah@127.0.0.1:8000"), SIPX_RESULT_SUCCESS) ;

      // Validate called side.
      bRC = validatorCalled.waitForCallEvent(hCallingLine1, hCall1, CALLSTATE_NEWCALL, CALLSTATE_NEW_CALL_NORMAL, true) ;
      CPPUNIT_ASSERT(bRC) ;
      bRC = validatorCalled.waitForCallEvent(hCallingLine1, hCall1, CALLSTATE_OFFERING, CALLSTATE_OFFERING_ACTIVE, true) ;
      CPPUNIT_ASSERT(bRC) ;
      CPPUNIT_ASSERT_EQUAL(sipxCallAccept(hCall1), SIPX_RESULT_SUCCESS) ;
      bRC = validatorCalled.waitForCallEvent(hCallingLine1, hCall1, CALLSTATE_ALERTING, CALLSTATE_ALERTING_NORMAL, true) ;
      CPPUNIT_ASSERT(bRC) ;
      CPPUNIT_ASSERT_EQUAL(sipxCallAnswer(hCall1), SIPX_RESULT_SUCCESS) ;
      bRC = validatorCalled.waitForCallEvent(hCallingLine1, hCall1, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
      CPPUNIT_ASSERT(bRC) ;
      bRC = validatorCalled.waitForCallEvent(hCallingLine1, hCall1, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
      CPPUNIT_ASSERT(bRC) ;

      // Make sure we don't have any unexpected events
      CPPUNIT_ASSERT(!validatorCalled.validateNoWaitingEvent()) ;

      // Do the join
      CPPUNIT_ASSERT_EQUAL(sipxConferenceCreate(g_hInst, &hConf), SIPX_RESULT_SUCCESS) ;

      CPPUNIT_ASSERT_EQUAL(sipxCallHold(hCall1), SIPX_RESULT_SUCCESS) ;
      bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_INACTIVE, true) ;
      CPPUNIT_ASSERT(bRC) ;
      CPPUNIT_ASSERT_EQUAL(sipxConferenceJoin(hConf, hCall1), SIPX_RESULT_SUCCESS) ;
      CPPUNIT_ASSERT_EQUAL(sipxCallUnhold(hCall1), SIPX_RESULT_SUCCESS) ;
      bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
      CPPUNIT_ASSERT(bRC) ;

      // Make sure we don't have any unexpected events
      CPPUNIT_ASSERT(!validatorCalled.validateNoWaitingEvent()) ;

      // Set up 2nd Leg
      CPPUNIT_ASSERT_EQUAL(sipxCallCreate(g_hInst, hCallingLine2, &hCall2), SIPX_RESULT_SUCCESS) ;
      CPPUNIT_ASSERT_EQUAL(sipxCallConnect(hCall2, "sip:blah@127.0.0.1:8000"), SIPX_RESULT_SUCCESS) ;

      // Validate called side.
      bRC = validatorCalled.waitForCallEvent(hCallingLine2, hCall2, CALLSTATE_NEWCALL, CALLSTATE_NEW_CALL_NORMAL, true) ;
      CPPUNIT_ASSERT(bRC) ;
      bRC = validatorCalled.waitForCallEvent(hCallingLine2, hCall2, CALLSTATE_OFFERING, CALLSTATE_OFFERING_ACTIVE, true) ;
      CPPUNIT_ASSERT(bRC) ;
      CPPUNIT_ASSERT_EQUAL(sipxCallAccept(hCall2), SIPX_RESULT_SUCCESS) ;
      bRC = validatorCalled.waitForCallEvent(hCallingLine2, hCall2, CALLSTATE_ALERTING, CALLSTATE_ALERTING_NORMAL, true) ;
      CPPUNIT_ASSERT(bRC) ;
      CPPUNIT_ASSERT_EQUAL(sipxCallAnswer(hCall2), SIPX_RESULT_SUCCESS) ;
      bRC = validatorCalled.waitForCallEvent(hCallingLine2, hCall2, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
      CPPUNIT_ASSERT(bRC) ;
      bRC = validatorCalled.waitForCallEvent(hCallingLine2, hCall2, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
      CPPUNIT_ASSERT(bRC) ;

      // Make sure we don't have any unexpected events
      CPPUNIT_ASSERT(!validatorCalled.validateNoWaitingEvent()) ;

      // Do the join
      CPPUNIT_ASSERT_EQUAL(sipxConferenceCreate(g_hInst, &hConf), SIPX_RESULT_SUCCESS) ;

      CPPUNIT_ASSERT_EQUAL(sipxCallHold(hCall2), SIPX_RESULT_SUCCESS) ;
      bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_INACTIVE, true) ;
      CPPUNIT_ASSERT(bRC) ;
      CPPUNIT_ASSERT_EQUAL(sipxConferenceJoin(hConf, hCall2), SIPX_RESULT_SUCCESS) ;
      CPPUNIT_ASSERT_EQUAL(sipxCallUnhold(hCall2), SIPX_RESULT_SUCCESS) ;
      bRC = validatorCalled.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
      CPPUNIT_ASSERT(bRC) ;

      // Make sure we don't have any unexpected events
      CPPUNIT_ASSERT(!validatorCalled.validateNoWaitingEvent()) ;

      // Tear down conference
      CPPUNIT_ASSERT_EQUAL(sipxCallDestroy(hCall1), SIPX_RESULT_SUCCESS) ;
      CPPUNIT_ASSERT_EQUAL(sipxCallDestroy(hCall2), SIPX_RESULT_SUCCESS) ;
      CPPUNIT_ASSERT_EQUAL(sipxConferenceDestroy(hConf), SIPX_RESULT_SUCCESS) ;

      CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst, UniversalEventValidatorCallback, &validatorCalled), SIPX_RESULT_SUCCESS) ;

      CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hCalledLine), SIPX_RESULT_SUCCESS) ;
      CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hCallingLine1), SIPX_RESULT_SUCCESS) ;
      CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hCallingLine2), SIPX_RESULT_SUCCESS) ;
   }

   OsTask::delay(TEST_DELAY*2) ;
   checkForLeaks() ;
}


void sipXtapiTestSuite::testConfHoldBridge()
{
    bool bRC ;
    EventValidator validatorCalling("testConfHoldBridge.calling") ;
    EventValidator validatorCalled1("testConfHoldBridge.called1") ;
    EventValidator validatorCalled2("testConfHoldBridge.called2") ;

    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestConfHoldBridge (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        SIPX_CALL hCall1 ;
        SIPX_CALL hCall2 ;
        SIPX_CONF hConf ;
        SIPX_LINE hLine ;
        SIPX_LINE hReceivingLine1;
        SIPX_LINE hReceivingLine2;

        SIPX_CALL hCheckCalls[10] ;
        size_t nCheckCalls ;

        validatorCalling.reset() ;
        validatorCalled1.reset() ;
        validatorCalled2.reset() ;

        // Setup Auto-answer call back
        resetAutoAnswerCallback() ;
        resetAutoAnswerCallback2() ;
        sipxEventListenerAdd(g_hInst,  UniversalEventValidatorCallback, &validatorCalling) ;
        sipxEventListenerAdd(g_hInst2, UniversalEventValidatorCallback, &validatorCalled1) ;
        sipxEventListenerAdd(g_hInst2, AutoAnswerCallback, NULL) ;
        sipxEventListenerAdd(g_hInst3, UniversalEventValidatorCallback, &validatorCalled2) ;
        sipxEventListenerAdd(g_hInst3, AutoAnswerCallback2, NULL) ;

        // Create Lines
        CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst, "sip:foo@127.0.0.1:8000", &hLine), SIPX_RESULT_SUCCESS) ;
        bRC = validatorCalling.waitForLineEvent(hLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst2, "sip:foo@127.0.0.1:9100", &hReceivingLine1, CONTACT_AUTO), SIPX_RESULT_SUCCESS) ;
        bRC = validatorCalled1.waitForLineEvent(hReceivingLine1, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst3, "sip:foo@127.0.0.1:10000", &hReceivingLine2, CONTACT_AUTO), SIPX_RESULT_SUCCESS) ;
        bRC = validatorCalled2.waitForLineEvent(hReceivingLine2, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Setup first leg
        CPPUNIT_ASSERT_EQUAL(sipxConferenceCreate(g_hInst, &hConf), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxConferenceAdd(hConf, hLine, "sip:blah@127.0.0.1:9100", &hCall1), SIPX_RESULT_SUCCESS) ;

        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_DIALTONE, CALLSTATE_DIALTONE_CONFERENCE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_REMOTE_OFFERING, CALLSTATE_REMOTE_OFFERING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_REMOTE_ALERTING, CALLSTATE_REMOTE_ALERTING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_NEWCALL, CALLSTATE_NEW_CALL_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_OFFERING, CALLSTATE_OFFERING_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_ALERTING, CALLSTATE_ALERTING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Make sure we don't have any unexpected events
        CPPUNIT_ASSERT(!validatorCalling.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled1.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled2.validateNoWaitingEvent()) ;

        // Validate Get Calls
        CPPUNIT_ASSERT_EQUAL(sipxConferenceGetCalls(hConf, hCheckCalls, 10, nCheckCalls), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT(nCheckCalls == 1) ;
        CPPUNIT_ASSERT(hCheckCalls[0] == hCall1) ;

        // Setup 2nd Leg
        CPPUNIT_ASSERT_EQUAL(sipxConferenceAdd(hConf, hLine, "sip:blah2@127.0.0.1:10000", &hCall2), SIPX_RESULT_SUCCESS) ;

        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_DIALTONE, CALLSTATE_DIALTONE_CONFERENCE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_REMOTE_OFFERING, CALLSTATE_REMOTE_OFFERING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_REMOTE_ALERTING, CALLSTATE_REMOTE_ALERTING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_NEWCALL, CALLSTATE_NEW_CALL_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_OFFERING, CALLSTATE_OFFERING_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_ALERTING, CALLSTATE_ALERTING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Make sure we don't have any unexpected events
        CPPUNIT_ASSERT(!validatorCalling.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled1.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled2.validateNoWaitingEvent()) ;

        // Validate Get Calls
        CPPUNIT_ASSERT_EQUAL(sipxConferenceGetCalls(hConf, hCheckCalls, 10, nCheckCalls), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT(nCheckCalls == 2) ;
        CPPUNIT_ASSERT(hCheckCalls[0] == hCall1) ;
        CPPUNIT_ASSERT(hCheckCalls[1] == hCall2) ;
        CPPUNIT_ASSERT(hCheckCalls[0] != hCheckCalls[1]) ;


        // Hold
        CPPUNIT_ASSERT_EQUAL(sipxConferenceHold(hConf, true), SIPX_RESULT_SUCCESS) ;

        // Validate Calling side
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE_HELD, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE_HELD, false) ;
        CPPUNIT_ASSERT(bRC) ;

        // Make sure we don't have any unexpected events
        CPPUNIT_ASSERT(!validatorCalling.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled1.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled2.validateNoWaitingEvent()) ;

        // Unhold
        CPPUNIT_ASSERT_EQUAL(sipxConferenceUnhold(hConf), SIPX_RESULT_SUCCESS) ;

        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, false) ;
        CPPUNIT_ASSERT(bRC) ;
        // Fix PT-6

        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, false) ;
        CPPUNIT_ASSERT(bRC) ;

        // Make sure we don't have any unexpected events
        CPPUNIT_ASSERT(!validatorCalling.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled1.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled2.validateNoWaitingEvent()) ;

        // Tear down conference
        CPPUNIT_ASSERT_EQUAL(sipxConferenceDestroy(hConf), SIPX_RESULT_SUCCESS) ;
        // Validate Calling Side

        // TODO: FIX PT-1
        // bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        // CPPUNIT_ASSERT(bRC) ;
        // bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        // CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Sides
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Make sure we don't have any unexpected events
        CPPUNIT_ASSERT(!validatorCalling.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled1.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled2.validateNoWaitingEvent()) ;

        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst, UniversalEventValidatorCallback, &validatorCalling), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, UniversalEventValidatorCallback, &validatorCalled1), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, AutoAnswerCallback, NULL), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst3, UniversalEventValidatorCallback, &validatorCalled2), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst3, AutoAnswerCallback2, NULL), SIPX_RESULT_SUCCESS) ;

        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hReceivingLine1), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hReceivingLine2), SIPX_RESULT_SUCCESS);
    }

    OsTask::delay(TEST_DELAY) ;
    checkForLeaks();
}


void sipXtapiTestSuite::testConfReAdd()
{
    bool bRC ;
    EventValidator validatorCalling("testConfReAdd.calling") ;
    EventValidator validatorCalled1("testConfReAdd.called1") ;
    EventValidator validatorCalled2("testConfReAdd.called2") ;

    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestConfReAdd (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        SIPX_CALL hCall1 ;
        SIPX_CALL hCall2 ;
        SIPX_CONF hConf ;
        SIPX_LINE hLine ;
        SIPX_LINE hReceivingLine1;
        SIPX_LINE hReceivingLine2;

        SIPX_CALL hCheckCalls[10] ;
        size_t nCheckCalls ;

        validatorCalling.reset() ;
        validatorCalled1.reset() ;
        validatorCalled2.reset() ;

        // Setup Auto-answer call back
        resetAutoAnswerCallback() ;
        resetAutoAnswerCallback2() ;

        sipxEventListenerAdd(g_hInst,  UniversalEventValidatorCallback, &validatorCalling) ;
        sipxEventListenerAdd(g_hInst2, AutoAnswerCallback, NULL) ;
        sipxEventListenerAdd(g_hInst2, UniversalEventValidatorCallback, &validatorCalled1) ;
        sipxEventListenerAdd(g_hInst3, AutoAnswerCallback2, NULL) ;
        sipxEventListenerAdd(g_hInst3, UniversalEventValidatorCallback, &validatorCalled2) ;

        // Create Lines
        CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst, "sip:foo@127.0.0.1:8000", &hLine), SIPX_RESULT_SUCCESS) ;
        bRC = validatorCalling.waitForLineEvent(hLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst2, "sip:foo@127.0.0.1:9100", &hReceivingLine1, CONTACT_AUTO), SIPX_RESULT_SUCCESS) ;
        bRC = validatorCalled1.waitForLineEvent(hReceivingLine1, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst3, "sip:foo@127.0.0.1:10000", &hReceivingLine2, CONTACT_AUTO), SIPX_RESULT_SUCCESS) ;
        bRC = validatorCalled2.waitForLineEvent(hReceivingLine2, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;


        CPPUNIT_ASSERT_EQUAL(sipxConferenceCreate(g_hInst, &hConf), SIPX_RESULT_SUCCESS) ;

        // Setup first leg
        CPPUNIT_ASSERT_EQUAL(sipxConferenceAdd(hConf, hLine, "sip:blah@127.0.0.1:9100", &hCall1), SIPX_RESULT_SUCCESS) ;

        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_DIALTONE, CALLSTATE_DIALTONE_CONFERENCE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_REMOTE_OFFERING, CALLSTATE_REMOTE_OFFERING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_REMOTE_ALERTING, CALLSTATE_REMOTE_ALERTING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_NEWCALL, CALLSTATE_NEW_CALL_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_OFFERING, CALLSTATE_OFFERING_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_ALERTING, CALLSTATE_ALERTING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Make sure we don't have any unexpected events
        CPPUNIT_ASSERT(!validatorCalling.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled1.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled2.validateNoWaitingEvent()) ;

        // Validate Get Calls
        CPPUNIT_ASSERT_EQUAL(sipxConferenceGetCalls(hConf, hCheckCalls, 10, nCheckCalls), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT(nCheckCalls == 1) ;
        CPPUNIT_ASSERT(hCheckCalls[0] == hCall1) ;

        // Setup 2nd Leg
        CPPUNIT_ASSERT_EQUAL(sipxConferenceAdd(hConf, hLine, "sip:blah2@127.0.0.1:10000", &hCall2), SIPX_RESULT_SUCCESS) ;

        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_DIALTONE, CALLSTATE_DIALTONE_CONFERENCE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_REMOTE_OFFERING, CALLSTATE_REMOTE_OFFERING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_REMOTE_ALERTING, CALLSTATE_REMOTE_ALERTING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_NEWCALL, CALLSTATE_NEW_CALL_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_OFFERING, CALLSTATE_OFFERING_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_ALERTING, CALLSTATE_ALERTING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Make sure we don't have any unexpected events
        CPPUNIT_ASSERT(!validatorCalling.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled1.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled2.validateNoWaitingEvent()) ;

        // Validate Get Calls
        CPPUNIT_ASSERT_EQUAL(sipxConferenceGetCalls(hConf, hCheckCalls, 10, nCheckCalls), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT(nCheckCalls == 2) ;
        CPPUNIT_ASSERT(hCheckCalls[0] == hCall1) ;
        CPPUNIT_ASSERT(hCheckCalls[1] == hCall2) ;
        CPPUNIT_ASSERT(hCheckCalls[0] != hCheckCalls[1]) ;

        // Tear Down first leg
        CPPUNIT_ASSERT_EQUAL(sipxConferenceRemove(hConf, hCall1), SIPX_RESULT_SUCCESS) ;

        // TODO: FIX PT-1
        // bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        // CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Sides
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Make sure we don't have any unexpected events
        CPPUNIT_ASSERT(!validatorCalling.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled1.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled2.validateNoWaitingEvent()) ;

        // Validate Get Calls
        CPPUNIT_ASSERT_EQUAL(sipxConferenceGetCalls(hConf, hCheckCalls, 10, nCheckCalls), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT(nCheckCalls == 1) ;
        CPPUNIT_ASSERT(hCheckCalls[0] == hCall2) ;

        resetAutoAnswerCallback() ;

        // Re-Add Leg 1
        CPPUNIT_ASSERT_EQUAL(sipxConferenceAdd(hConf, hLine, "sip:blah@127.0.0.1:9100", &hCall1), SIPX_RESULT_SUCCESS) ;

        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_DIALTONE, CALLSTATE_DIALTONE_CONFERENCE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_REMOTE_OFFERING, CALLSTATE_REMOTE_OFFERING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_REMOTE_ALERTING, CALLSTATE_REMOTE_ALERTING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_NEWCALL, CALLSTATE_NEW_CALL_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_OFFERING, CALLSTATE_OFFERING_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_ALERTING, CALLSTATE_ALERTING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Make sure we don't have any unexpected events
        CPPUNIT_ASSERT(!validatorCalling.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled1.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled2.validateNoWaitingEvent()) ;

        // Validate Get Calls
        CPPUNIT_ASSERT_EQUAL(sipxConferenceGetCalls(hConf, hCheckCalls, 10, nCheckCalls), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT(nCheckCalls == 2) ;
        CPPUNIT_ASSERT(hCheckCalls[0] == hCall2) ;
        CPPUNIT_ASSERT(hCheckCalls[1] == hCall1) ;
        CPPUNIT_ASSERT(hCheckCalls[0] != hCheckCalls[1]) ;

        // Tear Down 2nd leg
        CPPUNIT_ASSERT_EQUAL(sipxConferenceRemove(hConf, hCall2), SIPX_RESULT_SUCCESS) ;

        // TODO: FIX PT-1
        // bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        // CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Sides
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Make sure we don't have any unexpected events
        CPPUNIT_ASSERT(!validatorCalling.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled1.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled2.validateNoWaitingEvent()) ;

        resetAutoAnswerCallback2() ;

        // Re-Add 2nd Leg
        CPPUNIT_ASSERT_EQUAL(sipxConferenceAdd(hConf, hLine, "sip:blah2@127.0.0.1:10000", &hCall2), SIPX_RESULT_SUCCESS) ;

        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_DIALTONE, CALLSTATE_DIALTONE_CONFERENCE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_REMOTE_OFFERING, CALLSTATE_REMOTE_OFFERING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_REMOTE_ALERTING, CALLSTATE_REMOTE_ALERTING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_NEWCALL, CALLSTATE_NEW_CALL_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_OFFERING, CALLSTATE_OFFERING_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_ALERTING, CALLSTATE_ALERTING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Make sure we don't have any unexpected events
        CPPUNIT_ASSERT(!validatorCalling.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled1.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled2.validateNoWaitingEvent()) ;

        // Validate Get Calls
        CPPUNIT_ASSERT_EQUAL(sipxConferenceGetCalls(hConf, hCheckCalls, 10, nCheckCalls), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT(nCheckCalls == 2) ;
        CPPUNIT_ASSERT(hCheckCalls[0] == hCall1) ;
        CPPUNIT_ASSERT(hCheckCalls[1] == hCall2) ;
        CPPUNIT_ASSERT(hCheckCalls[0] != hCheckCalls[1]) ;


        // Tear Down first leg
        CPPUNIT_ASSERT_EQUAL(sipxConferenceRemove(hConf, hCall1), SIPX_RESULT_SUCCESS) ;

        // TODO: FIX PT-1
        // bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        // CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Sides
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Make sure we don't have any unexpected events
        CPPUNIT_ASSERT(!validatorCalling.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled1.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled2.validateNoWaitingEvent()) ;

        // Validate Get Calls
        CPPUNIT_ASSERT_EQUAL(sipxConferenceGetCalls(hConf, hCheckCalls, 10, nCheckCalls), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT(nCheckCalls == 1) ;
        CPPUNIT_ASSERT(hCheckCalls[0] == hCall2) ;

        resetAutoAnswerCallback() ;

        // Re-Add Leg 1
        CPPUNIT_ASSERT_EQUAL(sipxConferenceAdd(hConf, hLine, "sip:blah@127.0.0.1:9100", &hCall1), SIPX_RESULT_SUCCESS) ;

        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_DIALTONE, CALLSTATE_DIALTONE_CONFERENCE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_REMOTE_OFFERING, CALLSTATE_REMOTE_OFFERING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_REMOTE_ALERTING, CALLSTATE_REMOTE_ALERTING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_NEWCALL, CALLSTATE_NEW_CALL_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_OFFERING, CALLSTATE_OFFERING_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_ALERTING, CALLSTATE_ALERTING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Make sure we don't have any unexpected events
        CPPUNIT_ASSERT(!validatorCalling.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled1.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled2.validateNoWaitingEvent()) ;

        // Validate Get Calls
        CPPUNIT_ASSERT_EQUAL(sipxConferenceGetCalls(hConf, hCheckCalls, 10, nCheckCalls), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT(nCheckCalls == 2) ;
        CPPUNIT_ASSERT(hCheckCalls[0] == hCall2) ;
        CPPUNIT_ASSERT(hCheckCalls[1] == hCall1) ;
        CPPUNIT_ASSERT(hCheckCalls[0] != hCheckCalls[1]) ;

        // Tear Down 2nd leg
        CPPUNIT_ASSERT_EQUAL(sipxConferenceRemove(hConf, hCall2), SIPX_RESULT_SUCCESS) ;

        // TODO: FIX PT-1
        // bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        // CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Sides
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Make sure we don't have any unexpected events
        CPPUNIT_ASSERT(!validatorCalling.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled1.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled2.validateNoWaitingEvent()) ;

        resetAutoAnswerCallback2() ;

        // Re-Add 2nd Leg
        CPPUNIT_ASSERT_EQUAL(sipxConferenceAdd(hConf, hLine, "sip:blah2@127.0.0.1:10000", &hCall2), SIPX_RESULT_SUCCESS) ;

        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_DIALTONE, CALLSTATE_DIALTONE_CONFERENCE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_REMOTE_OFFERING, CALLSTATE_REMOTE_OFFERING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_REMOTE_ALERTING, CALLSTATE_REMOTE_ALERTING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_NEWCALL, CALLSTATE_NEW_CALL_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_OFFERING, CALLSTATE_OFFERING_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_ALERTING, CALLSTATE_ALERTING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Make sure we don't have any unexpected events
        CPPUNIT_ASSERT(!validatorCalling.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled1.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled2.validateNoWaitingEvent()) ;

        // Validate Get Calls
        CPPUNIT_ASSERT_EQUAL(sipxConferenceGetCalls(hConf, hCheckCalls, 10, nCheckCalls), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT(nCheckCalls == 2) ;
        CPPUNIT_ASSERT(hCheckCalls[0] == hCall1) ;
        CPPUNIT_ASSERT(hCheckCalls[1] == hCall2) ;
        CPPUNIT_ASSERT(hCheckCalls[0] != hCheckCalls[1]) ;


        // Tear Down first leg
        CPPUNIT_ASSERT_EQUAL(sipxConferenceRemove(hConf, hCall1), SIPX_RESULT_SUCCESS) ;

        // TODO: FIX PT-1
        // bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        // CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Sides
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Make sure we don't have any unexpected events
        CPPUNIT_ASSERT(!validatorCalling.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled1.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled2.validateNoWaitingEvent()) ;

        // Validate Get Calls
        CPPUNIT_ASSERT_EQUAL(sipxConferenceGetCalls(hConf, hCheckCalls, 10, nCheckCalls), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT(nCheckCalls == 1) ;
        CPPUNIT_ASSERT(hCheckCalls[0] == hCall2) ;

        resetAutoAnswerCallback() ;

        // Re-Add Leg 1
        CPPUNIT_ASSERT_EQUAL(sipxConferenceAdd(hConf, hLine, "sip:blah@127.0.0.1:9100", &hCall1), SIPX_RESULT_SUCCESS) ;

        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_DIALTONE, CALLSTATE_DIALTONE_CONFERENCE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_REMOTE_OFFERING, CALLSTATE_REMOTE_OFFERING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_REMOTE_ALERTING, CALLSTATE_REMOTE_ALERTING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_NEWCALL, CALLSTATE_NEW_CALL_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_OFFERING, CALLSTATE_OFFERING_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_ALERTING, CALLSTATE_ALERTING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Make sure we don't have any unexpected events
        CPPUNIT_ASSERT(!validatorCalling.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled1.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled2.validateNoWaitingEvent()) ;

        // Validate Get Calls
        CPPUNIT_ASSERT_EQUAL(sipxConferenceGetCalls(hConf, hCheckCalls, 10, nCheckCalls), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT(nCheckCalls == 2) ;
        CPPUNIT_ASSERT(hCheckCalls[0] == hCall2) ;
        CPPUNIT_ASSERT(hCheckCalls[1] == hCall1) ;
        CPPUNIT_ASSERT(hCheckCalls[0] != hCheckCalls[1]) ;

        // Tear Down 2nd leg
        CPPUNIT_ASSERT_EQUAL(sipxConferenceRemove(hConf, hCall2), SIPX_RESULT_SUCCESS) ;

        // TODO: FIX PT-1
        // bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        // CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Sides
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Make sure we don't have any unexpected events
        CPPUNIT_ASSERT(!validatorCalling.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled1.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled2.validateNoWaitingEvent()) ;

        resetAutoAnswerCallback2() ;

        // Re-Add 2nd Leg
        CPPUNIT_ASSERT_EQUAL(sipxConferenceAdd(hConf, hLine, "sip:blah2@127.0.0.1:10000", &hCall2), SIPX_RESULT_SUCCESS) ;

        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_DIALTONE, CALLSTATE_DIALTONE_CONFERENCE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_REMOTE_OFFERING, CALLSTATE_REMOTE_OFFERING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_REMOTE_ALERTING, CALLSTATE_REMOTE_ALERTING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_NEWCALL, CALLSTATE_NEW_CALL_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_OFFERING, CALLSTATE_OFFERING_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_ALERTING, CALLSTATE_ALERTING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Make sure we don't have any unexpected events
        CPPUNIT_ASSERT(!validatorCalling.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled1.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled2.validateNoWaitingEvent()) ;

        // Validate Get Calls
        CPPUNIT_ASSERT_EQUAL(sipxConferenceGetCalls(hConf, hCheckCalls, 10, nCheckCalls), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT(nCheckCalls == 2) ;
        CPPUNIT_ASSERT(hCheckCalls[0] == hCall1) ;
        CPPUNIT_ASSERT(hCheckCalls[1] == hCall2) ;
        CPPUNIT_ASSERT(hCheckCalls[0] != hCheckCalls[1]) ;

        // Tear down conference
        CPPUNIT_ASSERT_EQUAL(sipxConferenceDestroy(hConf), SIPX_RESULT_SUCCESS) ;

        // TODO: FIX PT-1
        //bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        //CPPUNIT_ASSERT(bRC) ;
        //bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        //CPPUNIT_ASSERT(bRC) ;

        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Sides
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Make sure we don't have any unexpected events
        CPPUNIT_ASSERT(!validatorCalling.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled1.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled2.validateNoWaitingEvent()) ;

        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst, UniversalEventValidatorCallback, &validatorCalling), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, UniversalEventValidatorCallback, &validatorCalled1), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, AutoAnswerCallback, NULL), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst3, UniversalEventValidatorCallback, &validatorCalled2), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst3, AutoAnswerCallback2, NULL), SIPX_RESULT_SUCCESS) ;

        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hReceivingLine1), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hReceivingLine2), SIPX_RESULT_SUCCESS) ;
    }

    OsTask::delay(TEST_DELAY) ;
    checkForLeaks() ;
}


void sipXtapiTestSuite::testConfHoldNoBridge()
{
    bool bRC ;
    EventValidator validatorCalling("testConfHoldNoBridge.calling") ;
    EventValidator validatorCalled1("testConfHoldNoBridge.called1") ;
    EventValidator validatorCalled2("testConfHoldNoBridge.called2") ;

    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestConfHoldNoBridge (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        SIPX_CALL hCall1 ;
        SIPX_CALL hCall2 ;
        SIPX_CONF hConf ;
        SIPX_LINE hLine ;
        SIPX_LINE hReceivingLine1;
        SIPX_LINE hReceivingLine2;

        SIPX_CALL hCheckCalls[10] ;
        size_t nCheckCalls ;

        validatorCalling.reset() ;
        validatorCalled1.reset() ;
        validatorCalled2.reset() ;

        // Setup Auto-answer call back
        resetAutoAnswerCallback() ;
        resetAutoAnswerCallback2() ;
        sipxEventListenerAdd(g_hInst, UniversalEventValidatorCallback, &validatorCalling) ;
        sipxEventListenerAdd(g_hInst2, UniversalEventValidatorCallback, &validatorCalled1) ;
        sipxEventListenerAdd(g_hInst2, AutoAnswerCallback, NULL) ;
        sipxEventListenerAdd(g_hInst3, UniversalEventValidatorCallback, &validatorCalled2) ;
        sipxEventListenerAdd(g_hInst3, AutoAnswerCallback2, NULL) ;

        // Create Lines
        CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst, "sip:foo@127.0.0.1:8000", &hLine), SIPX_RESULT_SUCCESS) ;
        bRC = validatorCalling.waitForLineEvent(hLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst2, "sip:foo@127.0.0.1:9100", &hReceivingLine1, CONTACT_AUTO), SIPX_RESULT_SUCCESS) ;
        bRC = validatorCalled1.waitForLineEvent(hReceivingLine1, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        CPPUNIT_ASSERT_EQUAL(sipxLineAdd(g_hInst3, "sip:foo@127.0.0.1:10000", &hReceivingLine2, CONTACT_AUTO), SIPX_RESULT_SUCCESS) ;
        bRC = validatorCalled2.waitForLineEvent(hReceivingLine2, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Setup first leg
        CPPUNIT_ASSERT_EQUAL(sipxConferenceCreate(g_hInst, &hConf), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxConferenceAdd(hConf, hLine, "sip:blah@127.0.0.1:9100", &hCall1), SIPX_RESULT_SUCCESS) ;

        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_DIALTONE, CALLSTATE_DIALTONE_CONFERENCE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_REMOTE_OFFERING, CALLSTATE_REMOTE_OFFERING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_REMOTE_ALERTING, CALLSTATE_REMOTE_ALERTING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_NEWCALL, CALLSTATE_NEW_CALL_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_OFFERING, CALLSTATE_OFFERING_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_ALERTING, CALLSTATE_ALERTING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Make sure we don't have any unexpected events
        CPPUNIT_ASSERT(!validatorCalling.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled1.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled2.validateNoWaitingEvent()) ;

        // Validate Get Calls
        CPPUNIT_ASSERT_EQUAL(sipxConferenceGetCalls(hConf, hCheckCalls, 10, nCheckCalls), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT(nCheckCalls == 1) ;
        CPPUNIT_ASSERT(hCheckCalls[0] == hCall1) ;

        // Setup 2nd Leg
        CPPUNIT_ASSERT_EQUAL(sipxConferenceAdd(hConf, hLine, "sip:blah2@127.0.0.1:10000", &hCall2), SIPX_RESULT_SUCCESS) ;

        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_DIALTONE, CALLSTATE_DIALTONE_CONFERENCE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_REMOTE_OFFERING, CALLSTATE_REMOTE_OFFERING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_REMOTE_ALERTING, CALLSTATE_REMOTE_ALERTING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Side
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_NEWCALL, CALLSTATE_NEW_CALL_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_OFFERING, CALLSTATE_OFFERING_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_ALERTING, CALLSTATE_ALERTING_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Make sure we don't have any unexpected events
        CPPUNIT_ASSERT(!validatorCalling.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled1.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled2.validateNoWaitingEvent()) ;

        // Validate Get Calls
        CPPUNIT_ASSERT_EQUAL(sipxConferenceGetCalls(hConf, hCheckCalls, 10, nCheckCalls), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT(nCheckCalls == 2) ;
        CPPUNIT_ASSERT(hCheckCalls[0] == hCall1) ;
        CPPUNIT_ASSERT(hCheckCalls[1] == hCall2) ;
        CPPUNIT_ASSERT(hCheckCalls[0] != hCheckCalls[1]) ;


        // Hold
        CPPUNIT_ASSERT_EQUAL(sipxConferenceHold(hConf, false), SIPX_RESULT_SUCCESS) ;

        // Validate Calling side
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE_HELD, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE_HELD, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_INACTIVE, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_INACTIVE, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, false) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Sides
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE_HELD, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE_HELD, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate no extra events.
        CPPUNIT_ASSERT(!validatorCalling.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled1.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled2.validateNoWaitingEvent()) ;

        // Unhold
        CPPUNIT_ASSERT_EQUAL(sipxConferenceUnhold(hConf), SIPX_RESULT_SUCCESS) ;

        // Validate Called Sides
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, false) ;
        CPPUNIT_ASSERT(bRC) ;

        // Make sure we don't have any unexpected events
        CPPUNIT_ASSERT(!validatorCalling.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled1.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled2.validateNoWaitingEvent()) ;

        // Tear down conference
        CPPUNIT_ASSERT_EQUAL(sipxConferenceDestroy(hConf), SIPX_RESULT_SUCCESS) ;

        // Validate Calling Side
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall1, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalling.waitForCallEvent(hLine, hCall2, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Called Sides
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, true) ;
        CPPUNIT_ASSERT(bRC) ;

        // Make sure we don't have any unexpected events
        CPPUNIT_ASSERT(!validatorCalling.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled1.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled2.validateNoWaitingEvent()) ;

        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst, UniversalEventValidatorCallback, &validatorCalling), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, UniversalEventValidatorCallback, &validatorCalled1), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst2, AutoAnswerCallback, NULL), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst3, UniversalEventValidatorCallback, &validatorCalled2), SIPX_RESULT_SUCCESS) ;
        CPPUNIT_ASSERT_EQUAL(sipxEventListenerRemove(g_hInst3, AutoAnswerCallback2, NULL), SIPX_RESULT_SUCCESS) ;

        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hLine), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hReceivingLine1), SIPX_RESULT_SUCCESS);
        CPPUNIT_ASSERT_EQUAL(sipxLineRemove(hReceivingLine2), SIPX_RESULT_SUCCESS);
    }

    OsTask::delay(TEST_DELAY) ;
    checkForLeaks();
}


void sipXtapiTestSuite::testManualConfSplit()
{
    SIPX_LINE hLine ;
    SIPX_CONF hConf ;
    SIPX_CALL hCall1 ;
    SIPX_CALL hCall2 ;

    // Create Lines
    sipxLineAdd(g_hInst5, "sip:foo@10.1.1.120", &hLine) ;

    // Setup first leg
    sipxConferenceCreate(g_hInst5, &hConf) ;
    sipxConferenceAdd(hConf, hLine, "sip:foo@10.1.1.164", &hCall1) ;
    sipxConferenceAdd(hConf, hLine, "sip:foo@10.1.1.93", &hCall2) ;

    printf("Waiting for answer...\n") ;
    OsTask::delay(7000) ;

    sipxCallHold(hCall1) ;

    printf("Waiting for hold (indigo)...\n") ;
    OsTask::delay(7000) ;

    sipxConferenceSplit(hConf, hCall1) ;

    printf("Waiting for split...\n") ;
    OsTask::delay(7000) ;

    sipxConferenceHold(hConf, false) ;
    printf("Waiting for conference hold...\n") ;
    OsTask::delay(7000) ;

    sipxCallUnhold(hCall1) ;

    printf("Check audio for call 1 (indigo)...\n") ;
    OsTask::delay(10000) ;

    sipxCallHold(hCall1) ;
    printf("Check audio for call 1 - hold (indigo)\n") ;
    OsTask::delay(7000) ;

    // Take conference off hold
    sipxConferenceUnhold(hConf) ;

    printf("Check audio for call 2 (mango)...\n") ;
    OsTask::delay(7000) ;

    sipxConferenceDestroy(hConf) ;
    sipxCallDestroy(hCall1) ;

    sipxLineRemove(hLine) ;
}


void sipXtapiTestSuite::testManualConfJoin()
{
    SIPX_LINE hLine ;
    SIPX_CONF hConf ;
    SIPX_CALL hCall1 ;
    SIPX_CALL hCall2 ;

    // Create Lines
    sipxLineAdd(g_hInst5, "sip:foo@10.1.1.120", &hLine) ;

    sipxCallCreate(g_hInst5, hLine, &hCall1) ;
    sipxCallConnect(hCall1, "sip:foo@10.1.1.93") ;
    sipxCallCreate(g_hInst5, hLine, &hCall2) ;
    sipxCallConnect(hCall2, "sip:foo@10.1.1.164") ;

    printf("Waiting for answer...\n") ;
    OsTask::delay(5000) ;

    sipxConferenceCreate(g_hInst5, &hConf) ;
    sipxConferenceJoin(hConf, hCall1) ;

    printf("Check Audio (mango)\n") ;
    OsTask::delay(5000) ;

    sipxCallHold(hCall2) ;
    OsTask::delay(1000) ;

    sipxConferenceJoin(hConf, hCall2) ;
    printf("Check No Audio Mango (indigo)\n") ;
    OsTask::delay(5000) ;

    sipxCallUnhold(hCall2) ;
    printf("Check Audio Mango (indigo)\n") ;
    OsTask::delay(30000) ;

    sipxConferenceDestroy(hConf) ;

    sipxLineRemove(hLine) ;
}


void sipXtapiTestSuite::testManualConfBridge()
{
    SIPX_CALL hCall1 ;
    SIPX_CALL hCall2 ;
    SIPX_CALL hCall3 ;
    SIPX_CONF hConf ;
    SIPX_LINE hLine ;

    sipxLineAdd(g_hInst5, "sip:foo@10.1.1.120", &hLine) ;

    sipxConferenceCreate(g_hInst5, &hConf) ;
    sipxConferenceAdd(hConf, hLine, "sip:foo@10.1.1.93", &hCall1) ;
    sipxConferenceAdd(hConf, hLine, "sip:foo@10.1.1.164", &hCall2) ;

    printf("Test Audio for local confernce\n") ;
    OsTask::delay(10000) ;

    sipxConferenceHold(hConf, true) ;

    sipxCallCreate(g_hInst5, hLine, &hCall3) ;
    sipxCallConnect(hCall3, "sip:foo@10.1.20.209") ;

    printf("Test Audio for other call and conference\n") ;
    OsTask::delay(45000) ;

    sipxConferenceDestroy(hConf) ;
    sipxCallDestroy(hCall3) ;

    sipxLineRemove(hLine) ;
}
