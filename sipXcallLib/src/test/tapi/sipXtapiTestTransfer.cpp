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


void sipXtapiTestSuite::testBlindTransferSuccess()
{
    EventValidator validatorTransferController("testBlindTransferSuccess.validatorTransferController") ;
    EventValidator validatorTransferee("testBlindTransferSuccess.validatorTransferee") ;
    EventValidator validatorTransferTarget("testBlindTransferSuccess.validatorTransferTarget") ;

    SIPX_CALL hTransferee ;
    SIPX_LINE hLine ;
    SIPX_LINE hReceivingLine1 ;
    SIPX_LINE hReceivingLine2 ;

    SIPX_RESULT sipxRC ;
    bool        bRC ;

    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestBlindTransferSuccess (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        validatorTransferController.reset() ;
        validatorTransferee.reset() ;
        validatorTransferTarget.reset() ;

        resetAutoAnswerCallback() ;
        resetAutoAnswerCallback2() ;

        sipxRC = sipxEventListenerAdd(g_hInst, UniversalEventValidatorCallback, &validatorTransferController) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerAdd(g_hInst2, UniversalEventValidatorCallback, &validatorTransferee) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerAdd(g_hInst2, AutoAnswerCallback, NULL) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerAdd(g_hInst3, UniversalEventValidatorCallback, &validatorTransferTarget) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerAdd(g_hInst3, AutoAnswerCallback2, NULL) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        // Create Lines
        sipxRC = sipxLineAdd(g_hInst, "sip:foo@127.0.0.1:8000", &hLine) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        bRC = validatorTransferController.waitForLineEvent(hLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;

        sipxRC = sipxLineAdd(g_hInst2, "sip:foo@127.0.0.1:9100", &hReceivingLine1, CONTACT_AUTO) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        bRC = validatorTransferee.waitForLineEvent(hReceivingLine1, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;

        sipxRC = sipxLineAdd(g_hInst3, "sip:foo@127.0.0.1:10000", &hReceivingLine2, CONTACT_AUTO) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        bRC = validatorTransferTarget.waitForLineEvent(hReceivingLine2, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;

        // Setup call to transfer source (transferee)
        sipxRC = sipxCallCreate(g_hInst, hLine, &hTransferee) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxCallConnect(hTransferee, "foo:foo@127.0.0.1:9100") ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        validatorTransferController.addMarker("Validate Calling Side: Transferee") ;
        bRC = validatorTransferController.waitForCallEvent(hLine, hTransferee, CALLSTATE_DIALTONE, CALLSTATE_DIALTONE_UNKNOWN) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferController.waitForCallEvent(hLine, hTransferee, CALLSTATE_REMOTE_OFFERING, CALLSTATE_REMOTE_OFFERING_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferController.waitForCallEvent(hLine, hTransferee, CALLSTATE_REMOTE_ALERTING, CALLSTATE_REMOTE_ALERTING_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferController.waitForCallEvent(hLine, hTransferee, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferController.waitForCallEvent(hLine, hTransferee, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START) ;
        CPPUNIT_ASSERT(bRC) ;

        validatorTransferee.addMarker("Validated Called Side: Transferee") ;
        bRC = validatorTransferee.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_NEWCALL, CALLSTATE_NEW_CALL_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_OFFERING, CALLSTATE_OFFERING_ACTIVE) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_ALERTING, CALLSTATE_ALERTING_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START) ;
        CPPUNIT_ASSERT(bRC) ;

        // Make sure we don't have any unexpected events
        CPPUNIT_ASSERT(!validatorTransferController.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorTransferee.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorTransferTarget.validateNoWaitingEvent()) ;

        sipxRC = sipxCallBlindTransfer(hTransferee, "sip:foo@127.0.0.1:10000") ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        // Validate Calling Side: Transfer Target
        bRC = validatorTransferController.waitForCallEvent(hLine, hTransferee, CALLSTATE_TRANSFER, CALLSTATE_TRANSFER_INITIATED) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferController.waitForCallEvent(hLine, hTransferee, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE_HELD) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferController.waitForCallEvent(hLine, hTransferee, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_INACTIVE) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferController.waitForCallEvent(hLine, hTransferee, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferController.waitForCallEvent(hLine, hTransferee, CALLSTATE_TRANSFER, CALLSTATE_TRANSFER_ACCEPTED) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferController.waitForCallEvent(hLine, hTransferee, CALLSTATE_TRANSFER, CALLSTATE_TRANSFER_RINGING) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferController.waitForCallEvent(hLine, hTransferee, CALLSTATE_TRANSFER, CALLSTATE_TRANSFER_SUCCESS) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferController.waitForCallEvent(hLine, hTransferee, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate Calling Side: Transferee
        bRC = validatorTransferee.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE_HELD) ;
        CPPUNIT_ASSERT(bRC) ;
        // BUG:: Why is line 0????
        bRC = validatorTransferee.waitForCallEvent(hReceivingLine1, g_hAutoAnswerCallbackCallOther, CALLSTATE_NEWCALL, CALLSTATE_NEW_CALL_TRANSFER) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForCallEvent(hReceivingLine1, g_hAutoAnswerCallbackCallOther, CALLSTATE_REMOTE_OFFERING, CALLSTATE_REMOTE_OFFERING_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForCallEvent(hReceivingLine1, g_hAutoAnswerCallbackCallOther, CALLSTATE_REMOTE_ALERTING, CALLSTATE_REMOTE_ALERTING_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForCallEvent(hReceivingLine1, g_hAutoAnswerCallbackCallOther, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForCallEvent(hReceivingLine1, g_hAutoAnswerCallbackCallOther, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;


        // Validated Called Side: Transfer Target
        bRC = validatorTransferTarget.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_NEWCALL, CALLSTATE_NEW_CALL_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferTarget.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_OFFERING, CALLSTATE_OFFERING_ACTIVE) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferTarget.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_ALERTING, CALLSTATE_ALERTING_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferTarget.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferTarget.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START) ;
        CPPUNIT_ASSERT(bRC) ;

        // Drop Original Call
        SIPX_CALL hDestroy = hTransferee ;
        sipxRC = sipxCallDestroy(hDestroy) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        bRC = validatorTransferController.waitForCallEvent(hLine, hTransferee, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;

        // Drop Calls
        hDestroy = g_hAutoAnswerCallbackCallOther ;
        sipxRC = sipxCallDestroy(hDestroy) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        bRC = validatorTransferee.waitForCallEvent(hReceivingLine1, g_hAutoAnswerCallbackCallOther, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE_HELD) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForCallEvent(hReceivingLine1, g_hAutoAnswerCallbackCallOther, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForCallEvent(hReceivingLine1, g_hAutoAnswerCallbackCallOther, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForCallEvent(hReceivingLine1, g_hAutoAnswerCallbackCallOther, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;

        bRC = validatorTransferTarget.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferTarget.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferTarget.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;

        CPPUNIT_ASSERT(!validatorTransferController.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorTransferee.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorTransferTarget.validateNoWaitingEvent()) ;

        // Remove Listeners
        sipxRC = sipxEventListenerRemove(g_hInst, UniversalEventValidatorCallback, &validatorTransferController) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerRemove(g_hInst2, UniversalEventValidatorCallback, &validatorTransferee) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerRemove(g_hInst2, AutoAnswerCallback, NULL) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerRemove(g_hInst3, UniversalEventValidatorCallback, &validatorTransferTarget) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerRemove(g_hInst3, AutoAnswerCallback2, NULL) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        sipxRC = sipxLineRemove(hLine) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxLineRemove(hReceivingLine1) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxLineRemove(hReceivingLine2) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
    }

    OsTask::delay(TEST_DELAY) ;
    checkForLeaks() ;
}


void sipXtapiTestSuite::testBlindTransferFailureBusy()
{
    EventValidator validatorTransferController("testBlindTransferFailureBusy.validatorTransferController") ;
    EventValidator validatorTransferee("testBlindTransferFailureBusy.validatorTransferee") ;
    EventValidator validatorTransferTarget("testBlindTransferFailureBusy.validatorTransferTarget") ;

    SIPX_CALL hTransferee ;
    SIPX_LINE hLine ;
    SIPX_LINE hReceivingLine1 ;
    SIPX_LINE hReceivingLine2 ;

    SIPX_RESULT sipxRC ;
    bool        bRC ;

    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        printf("\ntestBlindTransferFailureBusy (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        validatorTransferController.reset() ;
        validatorTransferee.reset() ;
        validatorTransferTarget.reset() ;

        resetAutoAnswerCallback() ;
        resetAutoAnswerCallback2() ;
        sipxRC = sipxEventListenerAdd(g_hInst, UniversalEventValidatorCallback, &validatorTransferController) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerAdd(g_hInst2, UniversalEventValidatorCallback, &validatorTransferee) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerAdd(g_hInst2, AutoAnswerCallback, NULL) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerAdd(g_hInst3, UniversalEventValidatorCallback, &validatorTransferTarget) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerAdd(g_hInst3, AutoRejectCallback, NULL) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        // Create Lines
        sipxRC = sipxLineAdd(g_hInst, "sip:foo@127.0.0.1:8000", &hLine) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        bRC = validatorTransferController.waitForLineEvent(hLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;

        sipxRC = sipxLineAdd(g_hInst2, "sip:foo@127.0.0.1:9100", &hReceivingLine1, CONTACT_AUTO) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        bRC = validatorTransferee.waitForLineEvent(hReceivingLine1, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;

        sipxRC = sipxLineAdd(g_hInst3, "sip:foo@127.0.0.1:10000", &hReceivingLine2, CONTACT_AUTO) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        bRC = validatorTransferTarget.waitForLineEvent(hReceivingLine2, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;

        // Setup call to transfer source (transferee)
        sipxRC = sipxCallCreate(g_hInst, hLine, &hTransferee) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxCallConnect(hTransferee, "blah:foo@127.0.0.1:9100") ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        // Validate Calling Side: Transferee
        bRC = validatorTransferController.waitForCallEvent(hLine, hTransferee, CALLSTATE_DIALTONE, CALLSTATE_DIALTONE_UNKNOWN) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferController.waitForCallEvent(hLine, hTransferee, CALLSTATE_REMOTE_OFFERING, CALLSTATE_REMOTE_OFFERING_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferController.waitForCallEvent(hLine, hTransferee, CALLSTATE_REMOTE_ALERTING, CALLSTATE_REMOTE_ALERTING_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferController.waitForCallEvent(hLine, hTransferee, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferController.waitForCallEvent(hLine, hTransferee, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validated Called Side: Transferee
        bRC = validatorTransferee.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_NEWCALL, CALLSTATE_NEW_CALL_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_OFFERING, CALLSTATE_OFFERING_ACTIVE) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_ALERTING, CALLSTATE_ALERTING_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START) ;
        CPPUNIT_ASSERT(bRC) ;

        // Make sure we don't have any unexpected events
        CPPUNIT_ASSERT(!validatorTransferController.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorTransferee.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorTransferTarget.validateNoWaitingEvent()) ;

        sipxRC = sipxCallBlindTransfer(hTransferee, "sip:foo@127.0.0.1:10000") ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        // Validate Calling Side: Transfer Target
        bRC = validatorTransferController.waitForCallEvent(hLine, hTransferee, CALLSTATE_TRANSFER, CALLSTATE_TRANSFER_INITIATED) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferController.waitForCallEvent(hLine, hTransferee, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE_HELD) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferController.waitForCallEvent(hLine, hTransferee, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_INACTIVE) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferController.waitForCallEvent(hLine, hTransferee, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferController.waitForCallEvent(hLine, hTransferee, CALLSTATE_TRANSFER, CALLSTATE_TRANSFER_ACCEPTED) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferController.waitForCallEvent(hLine, hTransferee, CALLSTATE_TRANSFER, CALLSTATE_TRANSFER_FAILURE) ;
        CPPUNIT_ASSERT(bRC) ;


        // Validate Calling Side: Transferee
        bRC = validatorTransferee.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE_HELD) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCallOther, CALLSTATE_NEWCALL, CALLSTATE_NEW_CALL_TRANSFER) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCallOther, CALLSTATE_REMOTE_OFFERING, CALLSTATE_REMOTE_OFFERING_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCallOther, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_BUSY) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCallOther, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;


        // Validated Called Side: Transfer Target
        bRC = validatorTransferTarget.waitForCallEvent(g_hAutoRejectCallbackLine, g_hAutoRejectCallbackCall, CALLSTATE_NEWCALL, CALLSTATE_NEW_CALL_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferTarget.waitForCallEvent(g_hAutoRejectCallbackLine, g_hAutoRejectCallbackCall, CALLSTATE_OFFERING, CALLSTATE_OFFERING_ACTIVE) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferTarget.waitForCallEvent(g_hAutoRejectCallbackLine, g_hAutoRejectCallbackCall, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferTarget.waitForCallEvent(g_hAutoRejectCallbackLine, g_hAutoRejectCallbackCall, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;


        CPPUNIT_ASSERT(!validatorTransferController.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorTransferee.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorTransferTarget.validateNoWaitingEvent()) ;

        // Drop Original Call
        SIPX_CALL hDestroy = hTransferee ;
        sipxRC = sipxCallDestroy(hDestroy) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        bRC = validatorTransferController.waitForCallEvent(hLine, hTransferee, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferController.waitForCallEvent(hLine, hTransferee, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;

        bRC = validatorTransferee.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorTransferee.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;

        CPPUNIT_ASSERT(!validatorTransferController.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorTransferee.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorTransferTarget.validateNoWaitingEvent()) ;

        // Remove Listeners
        sipxRC = sipxEventListenerRemove(g_hInst, UniversalEventValidatorCallback, &validatorTransferController) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerRemove(g_hInst2, UniversalEventValidatorCallback, &validatorTransferee) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerRemove(g_hInst2, AutoAnswerCallback, NULL) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerRemove(g_hInst3, UniversalEventValidatorCallback, &validatorTransferTarget) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerRemove(g_hInst3, AutoRejectCallback, NULL) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        sipxRC = sipxLineRemove(hLine) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxLineRemove(hReceivingLine1) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxLineRemove(hReceivingLine2) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
    }

    OsTask::delay(TEST_DELAY) ;
    checkForLeaks() ;
}



/**
 * This tests a basic transfer between two outbound calls.  The first call
 * (hCallSource) is transferred to the second outbound call (hSourceTarget).
 *
 */
void sipXtapiTestSuite::testTransferSuccess()
{
    EventValidator validatorSource("testTransferSuccess.source") ;
    EventValidator validatorCalled1("testTransferSuccess.validatorCalled1") ;
    EventValidator validatorCalled2("testTransferSuccess.validatorCalled2") ;

    SIPX_RESULT sipxRC ;
    bool        bRC ;

    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        SIPX_LINE hLine ;           // Local Line definition
        SIPX_CALL hCallSource ;     // Transferee
        SIPX_CALL hCallTarget ;     // Tranfer Target
        SIPX_LINE hReceivingLine1;  // Other side of hCallSource
        SIPX_LINE hReceivingLine2;  // Other side of hCallTarget

        printf("\ntestTransferSuccess (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        validatorSource.reset() ;
        validatorCalled1.reset() ;
        validatorCalled2.reset() ;

        //
        // Setup Listeners
        //
        resetAutoAnswerCallback() ;
        resetAutoAnswerCallback2() ;
        sipxRC = sipxEventListenerAdd(g_hInst, UniversalEventValidatorCallback, &validatorSource) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerAdd(g_hInst2, UniversalEventValidatorCallback, &validatorCalled1) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerAdd(g_hInst2, AutoAnswerCallback, NULL) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerAdd(g_hInst3, UniversalEventValidatorCallback, &validatorCalled2) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerAdd(g_hInst3, AutoAnswerCallback2, NULL) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        // Create Lines
        sipxRC = sipxLineAdd(g_hInst, "sip:foo@127.0.0.1:8000", &hLine) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        bRC = validatorSource.waitForLineEvent(hLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;

        sipxRC = sipxLineAdd(g_hInst2, "sip:foo@127.0.0.1:9100", &hReceivingLine1, CONTACT_AUTO) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        bRC = validatorCalled1.waitForLineEvent(hReceivingLine1, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;

        sipxRC = sipxLineAdd(g_hInst3, "sip:foo@127.0.0.1:10000", &hReceivingLine2, CONTACT_AUTO) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        bRC = validatorCalled2.waitForLineEvent(hReceivingLine2, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;

        // Setup call to transfer source (transferee)
        sipxRC = sipxCallCreate(g_hInst, hLine, &hCallSource) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxCallConnect(hCallSource, "blah:foo@127.0.0.1:9100") ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        // Validate Calling Side: Transferee
        bRC = validatorSource.waitForCallEvent(hLine, hCallSource, CALLSTATE_DIALTONE, CALLSTATE_DIALTONE_UNKNOWN) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallSource, CALLSTATE_REMOTE_OFFERING, CALLSTATE_REMOTE_OFFERING_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallSource, CALLSTATE_REMOTE_ALERTING, CALLSTATE_REMOTE_ALERTING_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallSource, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallSource, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validated Called Side: Transferee
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_NEWCALL, CALLSTATE_NEW_CALL_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_OFFERING, CALLSTATE_OFFERING_ACTIVE) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_ALERTING, CALLSTATE_ALERTING_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START) ;
        CPPUNIT_ASSERT(bRC) ;

        // Make sure we don't have any unexpected events
        CPPUNIT_ASSERT(!validatorSource.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled1.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled2.validateNoWaitingEvent()) ;

        // Setup call to transfer target (Transfer Target)
        sipxRC = sipxCallCreate(g_hInst, hLine, &hCallTarget) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxCallConnect(hCallTarget, "sip:foo@127.0.0.1:10000") ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        // Validate Calling Side: Transfer Target
        bRC = validatorSource.waitForCallEvent(hLine, hCallTarget, CALLSTATE_DIALTONE, CALLSTATE_DIALTONE_UNKNOWN) ;
        CPPUNIT_ASSERT(bRC) ;

        // BUG, the active event can be fire before ringing as focus is switched to this call.

        bRC = validatorSource.waitForCallEvent(hLine, hCallTarget, CALLSTATE_REMOTE_OFFERING, CALLSTATE_REMOTE_OFFERING_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallTarget, CALLSTATE_REMOTE_ALERTING, CALLSTATE_REMOTE_ALERTING_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallTarget, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallTarget, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validated Called Side: Transfer Target
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_NEWCALL, CALLSTATE_NEW_CALL_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_OFFERING, CALLSTATE_OFFERING_ACTIVE) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_ALERTING, CALLSTATE_ALERTING_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START) ;
        CPPUNIT_ASSERT(bRC) ;

        // Make sure we don't have any unexpected events
        CPPUNIT_ASSERT(!validatorSource.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled1.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled2.validateNoWaitingEvent()) ;

        // Kick off transfer
        sipxRC = sipxCallTransfer(hCallSource, hCallTarget) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        validatorSource.setMaxLookhead(8) ;

        // Validate Calling Side
        bRC = validatorSource.waitForCallEvent(hLine, hCallSource, CALLSTATE_TRANSFER, CALLSTATE_TRANSFER_INITIATED, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallSource, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE_HELD, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallSource, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallSource, CALLSTATE_TRANSFER, CALLSTATE_TRANSFER_ACCEPTED, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallSource, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallSource, CALLSTATE_TRANSFER, CALLSTATE_TRANSFER_SUCCESS, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallTarget, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE_HELD, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallTarget, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_INACTIVE, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallTarget, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallTarget, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate remote Transferee Side (called1)
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE_HELD, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCallOther, CALLSTATE_NEWCALL, CALLSTATE_NEW_CALL_TRANSFER, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCallOther, CALLSTATE_REMOTE_OFFERING, CALLSTATE_REMOTE_OFFERING_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCallOther, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCallOther, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;


        // Validate remote Transfer Target Side (called2)
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE_HELD, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2Other, CALLSTATE_NEWCALL, CALLSTATE_NEW_CALL_TRANSFERRED, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2Other, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2Other, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;

        // Make sure we don't have any unexpected events
        CPPUNIT_ASSERT(!validatorSource.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled1.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled2.validateNoWaitingEvent()) ;

        // Destroy calls.
        SIPX_CALL hDestroy = hCallSource ;
        sipxRC = sipxCallDestroy(hDestroy) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        hDestroy = hCallTarget ;
        sipxRC = sipxCallDestroy(hDestroy) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        // Validate destroy
        bRC = validatorSource.waitForCallEvent(hLine, hCallSource, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallTarget, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;

        // Drop other calls
        hDestroy = g_hAutoAnswerCallbackCallOther ;
        sipxRC = sipxCallDestroy(hDestroy) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        hDestroy = g_hAutoAnswerCallbackCall2Other ;
        sipxRC = sipxCallDestroy(hDestroy) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCallOther, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE_HELD) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCallOther, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCallOther, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCallOther, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;

        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2Other, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2Other, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2Other, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;

        // Remove Listeners
        sipxRC = sipxEventListenerRemove(g_hInst, UniversalEventValidatorCallback, &validatorSource) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerRemove(g_hInst2, UniversalEventValidatorCallback, &validatorCalled1) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerRemove(g_hInst2, AutoAnswerCallback, NULL) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerRemove(g_hInst3, UniversalEventValidatorCallback, &validatorCalled2) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerRemove(g_hInst3, AutoAnswerCallback2, NULL) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        sipxRC = sipxLineRemove(hLine) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxLineRemove(hReceivingLine1) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxLineRemove(hReceivingLine2) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

    }

    OsTask::delay(TEST_DELAY) ;
    checkForLeaks() ;
}

/**
 * This test creates a conference by creating two outbound call legs.
 * Afterwards, it removes itself from the conference by transferring
 * the first party called to the second party called.
 */
void sipXtapiTestSuite::testTransferConferenceSuccess()
{
    EventValidator validatorSource("testTransferConferenceSuccess.source") ;
    EventValidator validatorCalled1("testTransferConferenceSuccess.validatorCalled1") ;
    EventValidator validatorCalled2("testTransferConferenceSuccess.validatorCalled2") ;

    SIPX_RESULT sipxRC ;
    bool        bRC ;

    for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
    {
        SIPX_LINE hLine ;           // Local Line definition
        SIPX_CONF hConf ;           // Local Conference
        SIPX_CALL hCallSource ;     // Transferee
        SIPX_CALL hCallTarget ;     // Tranfer Target
        SIPX_LINE hReceivingLine1;  // Other side of hCallSource
        SIPX_LINE hReceivingLine2;  // Other side of hCallTarget

        printf("\ntestTransferConferenceSuccess (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

        validatorSource.reset() ;
        validatorCalled1.reset() ;
        validatorCalled2.reset() ;

        //
        // Setup Listeners
        //
        resetAutoAnswerCallback() ;
        resetAutoAnswerCallback2() ;
        sipxRC = sipxEventListenerAdd(g_hInst, UniversalEventValidatorCallback, &validatorSource) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerAdd(g_hInst2, UniversalEventValidatorCallback, &validatorCalled1) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerAdd(g_hInst2, AutoAnswerCallback, NULL) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerAdd(g_hInst3, UniversalEventValidatorCallback, &validatorCalled2) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerAdd(g_hInst3, AutoAnswerCallback2, NULL) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        // Create Lines
        sipxRC = sipxLineAdd(g_hInst, "sip:foo@127.0.0.1:8000", &hLine) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        bRC = validatorSource.waitForLineEvent(hLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;

        sipxRC = sipxLineAdd(g_hInst2, "sip:foo@127.0.0.1:9100", &hReceivingLine1, CONTACT_AUTO) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        bRC = validatorCalled1.waitForLineEvent(hReceivingLine1, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;

        sipxRC = sipxLineAdd(g_hInst3, "sip:foo@127.0.0.1:10000", &hReceivingLine2, CONTACT_AUTO) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        bRC = validatorCalled2.waitForLineEvent(hReceivingLine2, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;

        // Create Conference
        sipxRC = sipxConferenceCreate(g_hInst, &hConf) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        // Setup call to transfer source (transferee)
        sipxRC = sipxConferenceAdd(hConf, hLine, "sip:foo@127.0.0.1:9100", &hCallSource) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        // Validate Calling Side: Transferee
        bRC = validatorSource.waitForCallEvent(hLine, hCallSource, CALLSTATE_DIALTONE, CALLSTATE_DIALTONE_CONFERENCE) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallSource, CALLSTATE_REMOTE_OFFERING, CALLSTATE_REMOTE_OFFERING_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallSource, CALLSTATE_REMOTE_ALERTING, CALLSTATE_REMOTE_ALERTING_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallSource, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallSource, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validated Called Side: Transferee
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_NEWCALL, CALLSTATE_NEW_CALL_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_OFFERING, CALLSTATE_OFFERING_ACTIVE) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_ALERTING, CALLSTATE_ALERTING_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START) ;
        CPPUNIT_ASSERT(bRC) ;

        // Make sure we don't have any unexpected events
        CPPUNIT_ASSERT(!validatorSource.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled1.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled2.validateNoWaitingEvent()) ;

        // Setup call to transfer target (Transfer Target)
        sipxRC = sipxConferenceAdd(hConf, hLine, "sip:foo@127.0.0.1:10000", &hCallTarget) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        // Validate Calling Side: Transfer Target
        bRC = validatorSource.waitForCallEvent(hLine, hCallTarget, CALLSTATE_DIALTONE, CALLSTATE_DIALTONE_CONFERENCE) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallTarget, CALLSTATE_REMOTE_OFFERING, CALLSTATE_REMOTE_OFFERING_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallTarget, CALLSTATE_REMOTE_ALERTING, CALLSTATE_REMOTE_ALERTING_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallTarget, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallTarget, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validated Called Side: Transfer Target
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_NEWCALL, CALLSTATE_NEW_CALL_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_OFFERING, CALLSTATE_OFFERING_ACTIVE) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_ALERTING, CALLSTATE_ALERTING_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START) ;
        CPPUNIT_ASSERT(bRC) ;

        // Make sure we don't have any unexpected events
        CPPUNIT_ASSERT(!validatorSource.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled1.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled2.validateNoWaitingEvent()) ;

        // Kick off transfer
        sipxRC = sipxCallTransfer(hCallSource, hCallTarget) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        // Validate Calling Side
        validatorSource.setMaxLookhead(8) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallSource, CALLSTATE_TRANSFER, CALLSTATE_TRANSFER_INITIATED, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallSource, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE_HELD, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallSource, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallSource, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallSource, CALLSTATE_TRANSFER, CALLSTATE_TRANSFER_ACCEPTED, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallSource, CALLSTATE_TRANSFER, CALLSTATE_TRANSFER_SUCCESS, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallSource, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallTarget, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE_HELD, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallTarget, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_INACTIVE, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallTarget, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallTarget, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorSource.waitForCallEvent(hLine, hCallTarget, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;

        // Validate remote Transferee Side (called1)
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE_HELD, false) ;
        CPPUNIT_ASSERT(bRC) ;
        // BUG: Line handle should not be 0??
        bRC = validatorCalled1.waitForCallEvent(0, g_hAutoAnswerCallbackCallOther, CALLSTATE_NEWCALL, CALLSTATE_NEW_CALL_TRANSFER, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(0, g_hAutoAnswerCallbackCallOther, CALLSTATE_REMOTE_OFFERING, CALLSTATE_REMOTE_OFFERING_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(0, g_hAutoAnswerCallbackCallOther, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(0, g_hAutoAnswerCallbackCallOther, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(g_hAutoAnswerCallbackLine, g_hAutoAnswerCallbackCall, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;


        // Validate remote Transfer Target Side (called2)
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE_HELD, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2Other, CALLSTATE_NEWCALL, CALLSTATE_NEW_CALL_TRANSFERRED, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2Other, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2Other, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_START, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL, false) ;
        CPPUNIT_ASSERT(bRC) ;

        // Make sure we don't have any unexpected events
        CPPUNIT_ASSERT(!validatorSource.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled1.validateNoWaitingEvent()) ;
        CPPUNIT_ASSERT(!validatorCalled2.validateNoWaitingEvent()) ;

        // Destroy calls.
        sipxRC = sipxConferenceDestroy(hConf) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        // Drop other calls
        SIPX_CALL hDestroy = g_hAutoAnswerCallbackCallOther ;
        sipxRC = sipxCallDestroy(hDestroy) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        hDestroy = g_hAutoAnswerCallbackCall2Other ;
        sipxRC = sipxCallDestroy(hDestroy) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        bRC = validatorCalled1.waitForCallEvent(0, g_hAutoAnswerCallbackCallOther, CALLSTATE_CONNECTED, CALLSTATE_CONNECTED_ACTIVE_HELD) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(0, g_hAutoAnswerCallbackCallOther, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(0, g_hAutoAnswerCallbackCallOther, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled1.waitForCallEvent(0, g_hAutoAnswerCallbackCallOther, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;

        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2Other, CALLSTATE_AUDIO_EVENT, CALLSTATE_AUDIO_STOP) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2Other, CALLSTATE_DISCONNECTED, CALLSTATE_DISCONNECTED_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;
        bRC = validatorCalled2.waitForCallEvent(g_hAutoAnswerCallbackLine2, g_hAutoAnswerCallbackCall2Other, CALLSTATE_DESTROYED, CALLSTATE_DESTROYED_NORMAL) ;
        CPPUNIT_ASSERT(bRC) ;

        // Remove Listeners
        sipxRC = sipxEventListenerRemove(g_hInst, UniversalEventValidatorCallback, &validatorSource) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerRemove(g_hInst2, UniversalEventValidatorCallback, &validatorCalled1) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerRemove(g_hInst2, AutoAnswerCallback, NULL) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerRemove(g_hInst3, UniversalEventValidatorCallback, &validatorCalled2) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxEventListenerRemove(g_hInst3, AutoAnswerCallback2, NULL) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

        sipxRC = sipxLineRemove(hLine) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxLineRemove(hReceivingLine1) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;
        sipxRC = sipxLineRemove(hReceivingLine2) ;
        CPPUNIT_ASSERT(sipxRC == SIPX_RESULT_SUCCESS) ;

    }

    OsTask::delay(TEST_DELAY) ;
    checkForLeaks() ;
}
