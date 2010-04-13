//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include <cp/CallManager.h>
#include <ps/PsMsg.h>
#include <ps/PsHookswTask.h>
#include <net/SipUserAgent.h>
#include <cp/CpTestSupport.h>
#include <net/SipMessage.h>
#include <net/SipLineMgr.h>
#include <net/SipRefreshMgr.h>
#include <mi/CpMediaInterfaceFactoryFactory.h>

#ifdef _WIN32
  #define _CRTDBG_MAP_ALLOC
  #include <crtdbg.h>

_CrtMemState MemStateBegin;
_CrtMemState MemStateEnd;
_CrtMemState MemStateDiff;
#endif

#define BROKEN_INITTEST

#define NUM_OF_RUNS 10
/**
 * Unittest for CallManager
 */
class CallManagerTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(CallManagerTest);

#ifdef BROKEN_UNITTEST
    CPPUNIT_TEST(testOnOffHook);
    CPPUNIT_TEST(testPickupCall);
    CPPUNIT_TEST(testSimpleTeardown);
    CPPUNIT_TEST(testUATeardown);
    CPPUNIT_TEST(testLineMgrUATeardown);
    CPPUNIT_TEST(testRefreshMgrUATeardown);
#endif
    CPPUNIT_TEST_SUITE_END();

public:

    void testOnOffHook()
    {
        PsMsg *keyMsg;
        SipUserAgent *ua = CpTestSupport::newSipUserAgent();
        ua->start();
        CallManager *callmgr = CpTestSupport::newCallManager(ua);
        callmgr->start();

                keyMsg = new PsMsg(PsMsg::HOOKSW_STATE, NULL, PsHookswTask::OFF_HOOK, 0);
                callmgr->postMessage(*keyMsg);
                delete keyMsg;

                keyMsg = new PsMsg(PsMsg::BUTTON_DOWN, NULL, 0, '1');
                callmgr->postMessage(*keyMsg);
        delete keyMsg;

                keyMsg = new PsMsg(PsMsg::BUTTON_UP, NULL, 0, '1');
                callmgr->postMessage(*keyMsg);
        delete keyMsg;

                keyMsg = new PsMsg(PsMsg::BUTTON_DOWN, NULL, 0, '0');
                callmgr->postMessage(*keyMsg);
                delete keyMsg;

                keyMsg = new PsMsg(PsMsg::BUTTON_UP, NULL, 0, '0');
                callmgr->postMessage(*keyMsg);
                delete keyMsg;

                keyMsg = new PsMsg(PsMsg::BUTTON_DOWN, NULL, 0, '0');
                callmgr->postMessage(*keyMsg);
                delete keyMsg;

                keyMsg = new PsMsg(PsMsg::BUTTON_UP, NULL, 0, '0');
                callmgr->postMessage(*keyMsg);
                delete keyMsg;

                keyMsg = new PsMsg(PsMsg::BUTTON_DOWN, NULL, 0, '4');
                callmgr->postMessage(*keyMsg);
                delete keyMsg;

                keyMsg = new PsMsg(PsMsg::BUTTON_UP, NULL, 0, '4');
                callmgr->postMessage(*keyMsg);
                delete keyMsg;

        delete callmgr;
        delete ua;
    }


    void testPickupCall()
    {
        PsMsg *keyMsg;
        SipUserAgent *ua = CpTestSupport::newSipUserAgent();
        ua->start();
        CallManager *callmgr = CpTestSupport::newCallManager(ua);
        callmgr->start();

                // Wait a little and pick up the hook assuming it is ringing
                OsTask::delay(30000);
                printf("Picking up ringing phone\n");

                keyMsg = new PsMsg(PsMsg::HOOKSW_STATE, NULL, PsHookswTask::OFF_HOOK, 0);
                callmgr->postMessage(*keyMsg);
                delete keyMsg;

        delete callmgr;
        delete ua;
    }

    void testSimpleTeardown()
    {
       KNOWN_FATAL_BUG("XECS-259", "fails in mutex calls");

#ifdef _WIN32
        _CrtMemCheckpoint(&MemStateBegin);
#endif
        for (int i=0; i<NUM_OF_RUNS; ++i)
        {
            CallManager *pCallManager =
               new CallManager(FALSE,
                               NULL, //LineMgr
                               TRUE, // early media in 180 ringing
                               NULL, // CodecFactory
                               9000, // rtp start
                               9002, // rtp end
                               "sip:153@pingtel.com",
                               "sip:153@pingtel.com",
                               NULL, //SipUserAgent
                               0, // sipSessionReinviteTimer
                               NULL, // mgcpStackTask
                               NULL, // defaultCallExtension
                               Connection::RING, // availableBehavior
                               NULL, // unconditionalForwardUrl
                               -1, // forwardOnNoAnswerSeconds
                               NULL, // forwardOnNoAnswerUrl
                               Connection::BUSY, // busyBehavior
                               NULL, // sipForwardOnBusyUrl
                               NULL, // speedNums
                               CallManager::SIP_CALL, // phonesetOutgoingCallProtocol
                               4, // numDialPlanDigits
                               CallManager::NEAR_END_HOLD, // holdType
                               5000, // offeringDelay
                               "", // pLocal
                               CP_MAXIMUM_RINGING_EXPIRE_SECONDS, //inviteExpireSeconds
                               QOS_LAYER3_LOW_DELAY_IP_TOS, // expeditedIpTos
                               10, //maxCalls
                               sipXmediaFactoryFactory(NULL)); //pMediaFactory
#if 0
            printf("Starting CallManager\n");
#endif
            pCallManager->start();

            pCallManager->requestShutdown();

#if 0
            printf("Deleting CallManager\n");
#endif
            delete pCallManager;
        }

        for (int i=0; i<NUM_OF_RUNS; ++i)
        {
            sipxDestroyMediaFactoryFactory() ;
        }

#ifdef _WIN32
        _CrtMemCheckpoint(&MemStateEnd);
        if (_CrtMemDifference(&MemStateDiff, &MemStateBegin, &MemStateEnd))
        {
            _CrtMemDumpStatistics(&MemStateDiff);
        }
#endif
    }

    void testUATeardown()
    {
        for (int i=0; i<NUM_OF_RUNS; ++i)
        {
            SipUserAgent* sipUA = new SipUserAgent( 5090
                                                    ,5090
                                                    ,5091
                                                    ,NULL     // default publicAddress
                                                    ,NULL     // default defaultUser
                                                    ,"127.0.0.1" // default defaultSipAddress
                                                    ,NULL     // default sipProxyServers
                                                    ,NULL     // default sipDirectoryServers
                                                    ,NULL     // default sipRegistryServers
                                                    ,NULL     // default authenicateRealm
                                                    ,NULL     // default authenticateDb
                                                    ,NULL     // default authorizeUserIds
                                                    ,NULL     // default authorizePasswords
                                                    ,NULL //lineMgr
                                                   );

            sipUA->start();

            CallManager *pCallManager =
               new CallManager(FALSE,
                               NULL, //LineMgr
                               TRUE, // early media in 180 ringing
                               NULL, // CodecFactory
                               9000, // rtp start
                               9002, // rtp end
                               "sip:153@pingtel.com",
                               "sip:153@pingtel.com",
                               sipUA, //SipUserAgent
                               0, // sipSessionReinviteTimer
                               NULL, // mgcpStackTask
                               NULL, // defaultCallExtension
                               Connection::RING, // availableBehavior
                               NULL, // unconditionalForwardUrl
                               -1, // forwardOnNoAnswerSeconds
                               NULL, // forwardOnNoAnswerUrl
                               Connection::BUSY, // busyBehavior
                               NULL, // sipForwardOnBusyUrl
                               NULL, // speedNums
                               CallManager::SIP_CALL, // phonesetOutgoingCallProtocol
                               4, // numDialPlanDigits
                               CallManager::NEAR_END_HOLD, // holdType
                               5000, // offeringDelay
                               "", // pLocal
                               CP_MAXIMUM_RINGING_EXPIRE_SECONDS, //inviteExpireSeconds
                               QOS_LAYER3_LOW_DELAY_IP_TOS, // expeditedIpTos
                               10, //maxCalls
                               sipXmediaFactoryFactory(NULL)); //pMediaFactory
#if 0
            printf("Starting CallManager\n");
#endif
            pCallManager->start();

            sipUA->shutdown(TRUE);
            pCallManager->requestShutdown();

#if 0
            printf("Deleting CallManager\n");
#endif
            delete pCallManager;

        }

        for (int i=0; i<NUM_OF_RUNS; ++i)
        {
            sipxDestroyMediaFactoryFactory() ;
        }
    }

    void testLineMgrUATeardown()
    {
        for (int i=0; i<NUM_OF_RUNS; ++i)
        {
            SipLineMgr*    lineMgr = new SipLineMgr();
            lineMgr->StartLineMgr();
            SipUserAgent* sipUA = new SipUserAgent( 5090
                                                    ,5090
                                                    ,5091
                                                    ,NULL     // default publicAddress
                                                    ,NULL     // default defaultUser
                                                    ,"127.0.0.1" // default defaultSipAddress
                                                    ,NULL     // default sipProxyServers
                                                    ,NULL     // default sipDirectoryServers
                                                    ,NULL     // default sipRegistryServers
                                                    ,NULL     // default authenicateRealm
                                                    ,NULL     // default authenticateDb
                                                    ,NULL     // default authorizeUserIds
                                                    ,NULL     // default authorizePasswords
                                                    ,lineMgr
                                                   );

            sipUA->start();
            CallManager *pCallManager =
               new CallManager(FALSE,
                               NULL, //LineMgr
                               TRUE, // early media in 180 ringing
                               NULL, // CodecFactory
                               9000, // rtp start
                               9002, // rtp end
                               "sip:153@pingtel.com",
                               "sip:153@pingtel.com",
                               sipUA, //SipUserAgent
                               0, // sipSessionReinviteTimer
                               NULL, // mgcpStackTask
                               NULL, // defaultCallExtension
                               Connection::RING, // availableBehavior
                               NULL, // unconditionalForwardUrl
                               -1, // forwardOnNoAnswerSeconds
                               NULL, // forwardOnNoAnswerUrl
                               Connection::BUSY, // busyBehavior
                               NULL, // sipForwardOnBusyUrl
                               NULL, // speedNums
                               CallManager::SIP_CALL, // phonesetOutgoingCallProtocol
                               4, // numDialPlanDigits
                               CallManager::NEAR_END_HOLD, // holdType
                               5000, // offeringDelay
                               "", // pLocal
                               CP_MAXIMUM_RINGING_EXPIRE_SECONDS, //inviteExpireSeconds
                               QOS_LAYER3_LOW_DELAY_IP_TOS, // expeditedIpTos
                               10, //maxCalls
                               sipXmediaFactoryFactory(NULL)); //pMediaFactory
#if 0
            printf("Starting CallManager\n");
#endif
            pCallManager->start();

            lineMgr->requestShutdown();
            sipUA->shutdown(TRUE);
            pCallManager->requestShutdown();

#if 0
            printf("Deleting CallManager\n");
#endif

            // Delete lineMgr *after* CallManager - this seems to fix the problem
            // that SipClient->run() encounters a NULL socket.
            delete pCallManager;
            delete lineMgr;
        }

        for (int i=0; i<NUM_OF_RUNS; ++i)
        {
            sipxDestroyMediaFactoryFactory() ;
        }
    }

    void testRefreshMgrUATeardown()
    {
        for (int i=0; i<NUM_OF_RUNS; ++i)
        {
            SipLineMgr*    lineMgr = new SipLineMgr();
            SipRefreshMgr* refreshMgr = new SipRefreshMgr();

            lineMgr->StartLineMgr();
            lineMgr->initializeRefreshMgr( refreshMgr );

            SipUserAgent* sipUA = new SipUserAgent( 5090
                                                    ,5090
                                                    ,5091
                                                    ,NULL     // default publicAddress
                                                    ,NULL     // default defaultUser
                                                    ,"127.0.0.1" // default defaultSipAddress
                                                    ,NULL     // default sipProxyServers
                                                    ,NULL     // default sipDirectoryServers
                                                    ,NULL     // default sipRegistryServers
                                                    ,NULL     // default authenicateRealm
                                                    ,NULL     // default authenticateDb
                                                    ,NULL     // default authorizeUserIds
                                                    ,NULL     // default authorizePasswords
                                                    ,lineMgr
                                                   );

            sipUA->start();
            refreshMgr->init(sipUA);


            CallManager *pCallManager =
               new CallManager(FALSE,
                               NULL, //LineMgr
                               TRUE, // early media in 180 ringing
                               NULL, // CodecFactory
                               9000, // rtp start
                               9002, // rtp end
                               "sip:153@pingtel.com",
                               "sip:153@pingtel.com",
                               sipUA, //SipUserAgent
                               0, // sipSessionReinviteTimer
                               NULL, // mgcpStackTask
                               NULL, // defaultCallExtension
                               Connection::RING, // availableBehavior
                               NULL, // unconditionalForwardUrl
                               -1, // forwardOnNoAnswerSeconds
                               NULL, // forwardOnNoAnswerUrl
                               Connection::BUSY, // busyBehavior
                               NULL, // sipForwardOnBusyUrl
                               NULL, // speedNums
                               CallManager::SIP_CALL, // phonesetOutgoingCallProtocol
                               4, // numDialPlanDigits
                               CallManager::NEAR_END_HOLD, // holdType
                               5000, // offeringDelay
                               "", // pLocal
                               CP_MAXIMUM_RINGING_EXPIRE_SECONDS, //inviteExpireSeconds
                               QOS_LAYER3_LOW_DELAY_IP_TOS, // expeditedIpTos
                               10, //maxCalls
                               sipXmediaFactoryFactory(NULL)); //pMediaFactory
#if 0
            printf("Starting CallManager\n");
#endif
            pCallManager->start();

            lineMgr->requestShutdown();
            refreshMgr->requestShutdown();
            sipUA->shutdown(TRUE);
            pCallManager->requestShutdown();

#if 0
            printf("Deleting CallManager\n");
#endif

            delete pCallManager;
            delete refreshMgr;
            delete lineMgr;
        }

        for (int i=0; i<NUM_OF_RUNS; ++i)
        {
            sipxDestroyMediaFactoryFactory() ;
        }
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(CallManagerTest);
