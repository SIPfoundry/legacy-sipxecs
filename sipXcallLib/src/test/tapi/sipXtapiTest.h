//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
//////////////////////////////////////////////////////////////////////////////


#ifndef _SIPXTAPITEST_H
#define _SIPXTAPITEST_H

#ifdef _WIN32
#ifdef SIPX_TEST_FOR_MEMORY_LEAKS
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif
#endif

#include <cppunit/extensions/HelperMacros.h>

#include "utl/UtlSList.h"
#include "tapi/sipXtapi.h"
#include "tapi/sipXtapiEvents.h"
#include "tapi/sipXtapiInternal.h"

// Defines
#define CALL_DELAY      500     /**< Delay where we need to wait for something in ms */
#define STRESS_FACTOR   3       /**< How many times to repeat each test case */
#define TEST_DELAY      4000    /**< Time to delay between each test */

#ifdef _WIN32
  #define TEST_AUDIO              0
  #define TEST_LINE               0
  #define TEST_CALL               0
  #define TEST_CONF               0
  #define TEST_REG                0
  #define TEST_TRANSFER           1
  #define TEST_CONFIG             1
  #define TEST_PROBLEMATIC_CASES  0
#else
  #define TEST_AUDIO              0
  #define TEST_LINE               0
  #define TEST_CALL               0
  #define TEST_CONF               0
  #define TEST_REG                0
  #define TEST_TRANSFER           0
  #define TEST_CONFIG             0
  #define TEST_PROBLEMATIC_CASES  0
#endif /* _WIN32 */


bool basicCall_CallBack_Receive(SIPX_EVENT_CATEGORY category,
                                         void* pInfo,
                                         void* pUserData);
bool basicCall_CallBack_Receive3(SIPX_EVENT_CATEGORY category,
                                         void* pInfo,
                                         void* pUserData);

bool basicCall_CallBack_Place(SIPX_EVENT_CATEGORY category,
                                         void* pInfo,
                                         void* pUserData);

bool basicCall_CallBack_Receive3_hangup(SIPX_EVENT_CATEGORY category,
                                         void* pInfo,
                                         void* pUserData);

bool basicCall_CallBack_Receive3_busy(SIPX_EVENT_CATEGORY category,
                                         void* pInfo,
                                         void* pUserData);

class sipXtapiTestSuite : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(sipXtapiTestSuite) ;

    // CPPUNIT_TEST(testNothing) ;

#if TEST_AUDIO /* [ */
    CPPUNIT_TEST(testGainAPI) ;
    CPPUNIT_TEST(testMuteAPI) ;
    CPPUNIT_TEST(testVolumeAPI) ;
    CPPUNIT_TEST(testAudioSettings);
#endif /* TEST_AUDIO ] */

#if TEST_LINE /* [ */
    CPPUNIT_TEST(testLineAPI_Add) ;
    CPPUNIT_TEST(testLineAPI_Remove) ;
    CPPUNIT_TEST(testLineAPI_Credential) ;
    CPPUNIT_TEST(testLineAPI_Get) ;
    CPPUNIT_TEST(testLineAPI_GetURI) ;
    CPPUNIT_TEST(testLines) ;
    CPPUNIT_TEST(testLineEvents);
    CPPUNIT_TEST(testLineAliases);
#endif /* TEST_LINE ] */

#if TEST_CALL /* [ */
    CPPUNIT_TEST(testCallMakeAPI) ;
    CPPUNIT_TEST(testCallGetID) ;
    CPPUNIT_TEST(testCallGetRemoteID) ;
    CPPUNIT_TEST(testCallGetLocalID) ;
    CPPUNIT_TEST(testCallBasic) ;
    CPPUNIT_TEST(testCallBasic2) ;
#ifdef _WIN32
    CPPUNIT_TEST(testCallMute);
#endif
    CPPUNIT_TEST(testCallBusy) ;
    CPPUNIT_TEST(testCallHold) ;
    CPPUNIT_TEST(testCallRedirect);
    CPPUNIT_TEST(testSendInfo);
    CPPUNIT_TEST(testSendInfoTimeout);
    CPPUNIT_TEST(testSendInfoFailure);

    //
    // The following test cases allow you to manually test features and
    // verify that audio works.  You will need to modify the IP address
    // to work for you.
    //
    // CPPUNIT_TEST(testManualCallDialtone) ;
    // CPPUNIT_TEST(testManualPlayFileNoCall);
#endif /* TEST_CALL ] */

#if TEST_CONF /* [ */
    CPPUNIT_TEST(testConfBasic1) ;
    CPPUNIT_TEST(testConfBasic2) ;
    CPPUNIT_TEST(testConfBasic3) ;
    CPPUNIT_TEST(testConfBasic4) ;
    CPPUNIT_TEST(testConfBasic5) ;
    CPPUNIT_TEST(testConfBasic6) ;
    CPPUNIT_TEST(testConfHoldIndividual) ;
    CPPUNIT_TEST(testConfJoin) ;
#if 0
    // This test is not yet working.
    CPPUNIT_TEST(testConfJoin2) ;
#endif
    CPPUNIT_TEST(testConfHoldNoBridge) ;
    CPPUNIT_TEST(testConfHoldBridge) ;
    CPPUNIT_TEST(testConfReAdd) ;

    //
    // The following test cases allow you to manually test join/split and
    // verify audio -- you will need to modify the IP address to work
    // for you.
    //
    // CPPUNIT_TEST(testManualConfBridge) ;
    // CPPUNIT_TEST(testManualConfSplit) ;
    // CPPUNIT_TEST(testManualConfJoin) ;
#endif /* TEST_CONF ] */

#if TEST_REG /* [ */
    CPPUNIT_TEST(testRegistration);
    CPPUNIT_TEST(testReRegistration);
    CPPUNIT_TEST(testBadRegistrarRegistration);
#endif /* TEST_REG ] */

#if TEST_TRANSFER /* [ */
    CPPUNIT_TEST(testBlindTransferSuccess) ;
    CPPUNIT_TEST(testBlindTransferFailureBusy) ;
    CPPUNIT_TEST(testTransferSuccess) ;
    CPPUNIT_TEST(testTransferConferenceSuccess) ;
#endif /* TEST_TRANSFER ] */

#if TEST_CONFIG /* [ */
    CPPUNIT_TEST(testGetVersion) ;
    CPPUNIT_TEST(testSeqPortSelection) ;
    CPPUNIT_TEST(testAutoPortSelection) ;
    CPPUNIT_TEST(testSetCallback) ;
#ifdef _WIN32
#ifdef VOICE_ENGINE
    //CPPUNIT_TEST(testConfigOutOfBand) ;
#endif
#endif
    CPPUNIT_TEST(testTeardown);
    CPPUNIT_TEST(testConfigEnableStunSuccess);
    CPPUNIT_TEST(testConfigEnableStunFailure);
    CPPUNIT_TEST(testConfigLog) ;
#ifdef VOICE_ENGINE /* [ */
    CPPUNIT_TEST(testConfigCodecPreferences);
#endif /* VOICE_ENGINE ] */
#endif /* TEST_CONFIG ] */

#ifdef TEST_PROBLEMATIC_CASES /* [ */
    void testCallRapidCallAndHangup();
#endif /* TEST_PROBLEMATIC_CASES ] */

CPPUNIT_TEST_SUITE_END() ;

public:
    sipXtapiTestSuite();

    void setUp() ;
    void tearDown() ;

    void testNothing() ;

    void testGainAPI() ;
    void testMuteAPI() ;
    void testVolumeAPI() ;
    void testAudioSettings();

    void testLineAPI_Add() ;
    void testLineAPI_Remove() ;
    void testLineAPI_Credential() ;
    void testLineAPI_Get() ;
    void testLineAPI_GetURI() ;

    void testCallMakeAPI() ;
    void testCallGetID() ;
    void testCallGetRemoteID() ;
    void testCallGetLocalID() ;

    void testCallBasic() ;
    void testCallBasic2() ;
    void testCallBusy() ;
    void testCallHold() ;
    void testCallMute();
    void testCallRedirect();
    void testManualCallDialtone() ;

    void testConfBasic1() ;
    void testConfBasic2() ;
    void testConfBasic3() ;
    void testConfBasic4() ;
    void testConfBasic5() ;
    void testConfBasic6() ;
    void testConfJoin() ;
    void testConfJoin2() ;
    void testConfHoldIndividual() ;
    void testConfHoldNoBridge() ;
    void testConfHoldBridge() ;
    void testConfReAdd() ;
    void testManualConfSplit() ;
    void testManualConfJoin() ;
    void testManualConfBridge() ;

    void testLines() ;
    void testLineEvents();
    void testLineAliases();
    void testRegistration();
    void testBadRegistrarRegistration();
    void testReRegistration();

    void testBlindTransferSuccess() ;
    void testBlindTransferFailureBusy() ;
    void testTransferSuccess() ;
    void testTransferConferenceSuccess() ;

    void testGetVersion() ;
    void testSendInfo();
    void testSendInfoFailure();
    void testSendInfoTimeout();
    void testSetCallback();

    void testAutoPortSelection() ;
    void testSeqPortSelection() ;
    void testConfigLog() ;
    void testConfigOutOfBand() ;
    void testTeardown();
    void testConfigCodecPreferences();
    void testConfigEnableStunSuccess() ;
    void testConfigEnableStunFailure() ;

    void testCallRapidCallAndHangup();
protected:
    void createCall(SIPX_LINE hLine, SIPX_CALL* phCall) ;
    void destroyCall(SIPX_CALL& hCall) ;

    void createCall(SIPX_LINE* phLine, SIPX_CALL* phCall) ;
    void destroyCall(SIPX_LINE& hLine, SIPX_CALL& hCall) ;

    void checkForLeaks();
    void checkForCallLeaks(SIPX_INST hInst) ;
private:
#ifdef _WIN32
#ifdef SIPX_TEST_FOR_MEMORY_LEAKS
    _CrtMemState msBeforeTest, msAfterTest ;
#endif
#endif


} ;

#endif
