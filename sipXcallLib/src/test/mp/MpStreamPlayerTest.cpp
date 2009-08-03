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

#include "test/mp/MpTestConfig.h"
#include "test/mp/MyPlayerListenerPoller.h"
#include "test/mp/MyPlayerListenerHistoryKeeper.h"
#include "test/mp/MyStreamQueueHistoryKeeper.h"

#include "mp/MpMisc.h"
#include "mp/MpStreamPlayer.h"
#include "mp/MpStreamQueuePlayer.h"
#include "os/OsDefs.h"
#include "mp/StreamDefs.h"
#include "net/Url.h"

class MpStreamPlayerTest : public CppUnit::TestCase
{
        CPPUNIT_TEST_SUITE(MpStreamPlayerTest);
        CPPUNIT_TEST(testRawSynchronousNonCache);
        CPPUNIT_TEST(testRawSynchronousCache);
        CPPUNIT_TEST(testRawSynchronousListener);

        CPPUNIT_TEST(testRawAsynchronousNonCache);
        CPPUNIT_TEST(testRawAsynchronousCache);
        CPPUNIT_TEST(testRawAsynchronousAbortNonCache);
        CPPUNIT_TEST(testRawAsynchronousPauseNonCache);
        CPPUNIT_TEST(testRawAsynchronousAbortCache);
        CPPUNIT_TEST(testWavSynchronousNonCache);
        CPPUNIT_TEST(testRawASynchronousLoop);
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp()
        {
            mCfg = MpTestConfig::getTestInstance();
        }


        void testRawSynchronousNonCache()
        {
            PlayerState state ;
            OsStatus status ;
            char szUrl[128] ;
            int i ;

            osPrintf("testRawSynchronousNonCache\n") ;

            //
            // Without Caching; play 0, 1, 2, 3
            //
            for (i=0; i<4; i++)
            {
                sprintf(szUrl, "http://%s/nums/%d.raw", BASE_URL, i) ;
                osPrintf("Playing %s\n", szUrl) ;

                Url url(szUrl) ;

                MpStreamPlayer* pPlayer = new MpStreamPlayer(mCfg->getFlowGraph()->getMsgQ(), url, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerUnrealized) ;

                status = pPlayer->realize() ;       CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerRealized) ;

                status = pPlayer->prefetch() ;      CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerPrefetched) ;

                status = pPlayer->play() ;          CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerStopped) ;

                status = pPlayer->stop() ;          CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerStopped) ;

                delete pPlayer ;
            }
        }

        void testRawSynchronousCache()
        {
            osPrintf("testRawSynchronousCache\n") ;

            PlayerState state ;
            OsStatus status ;
            char szUrl[128] ;
            int i ;

            //
            // With Caching, play 0, 0, 1, 1, 2, 2, 3, 3
            //
            for (i=0; i<4; i++)
            {
                sprintf(szUrl, "http://%s/nums/%d.raw", BASE_URL, i) ;
                osPrintf("Playing %s\n", szUrl) ;

                Url url(szUrl) ;

                MpStreamPlayer* pPlayer = new MpStreamPlayer(mCfg->getFlowGraph()->getMsgQ(), url, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW | STREAM_HINT_CACHE) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerUnrealized) ;

                status = pPlayer->realize() ;       CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerRealized) ;

                status = pPlayer->prefetch() ;      CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerPrefetched) ;

                status = pPlayer->play() ;          CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerStopped) ;

                status = pPlayer->rewind() ;        CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerPrefetched) ;

                status = pPlayer->play() ;          CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerStopped) ;

                status = pPlayer->stop() ;          CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerStopped) ;

                delete pPlayer ;
            }
        }


        void testRawSynchronousListener()
        {
            PlayerState state ;
            OsStatus status ;
            char szUrl[128] ;
            int i ;

            osPrintf("testRawSynchronousListener\n") ;

            //
            // With Caching and listener, play 0, 0, 1, 1, 2, 2, 3, 3
            //
            for (i=0; i<4; i++)
            {
                sprintf(szUrl, "http://%s/nums/%d.raw", BASE_URL, i) ;
                osPrintf("Playing %s\n", szUrl) ;

                Url url(szUrl) ;

                MpStreamPlayer* pPlayer = new MpStreamPlayer(mCfg->getFlowGraph()->getMsgQ(), url, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW | STREAM_HINT_CACHE) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerUnrealized) ;

                MyPlayerListenerHistoryKeeper* pListener = new MyPlayerListenerHistoryKeeper() ;
                status = pPlayer->addListener(pListener, (void*) i) ;
                CPPUNIT_ASSERT(status == OS_SUCCESS) ;

                status = pPlayer->realize() ;       CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerRealized) ;

                status = pPlayer->prefetch() ;      CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerPrefetched) ;

                status = pPlayer->play() ;          CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerStopped) ;

                status = pPlayer->rewind() ;        CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerPrefetched) ;

                status = pPlayer->play() ;          CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerStopped) ;

                status = pPlayer->stop() ;          CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerStopped) ;

                int expectedHistory[] =
                    {
                        PlayerRealized,
                        PlayerPrefetched,
                        PlayerPlaying,
                        PlayerStopped,
                        PlayerPrefetched,
                        PlayerPlaying,
                        PlayerStopped,
                        -1,
                    } ;

                UtlBoolean bMatches = pListener->matchesHistory((void*) i, expectedHistory) ;
                CPPUNIT_ASSERT(bMatches) ;

                status = pPlayer->removeListener(pListener) ;   CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                status = pPlayer->removeListener(pListener) ;   CPPUNIT_ASSERT(status == OS_NOT_FOUND) ;

                delete pPlayer ;
                delete pListener ;
            }
        }


        void testRawAsynchronousNonCache()
        {
            PlayerState state ;
            UtlBoolean bRC ;
            OsStatus status ;
            char szUrl[128] ;
            int i ;

            osPrintf("testRawAsynchronousNonCache\n") ;

            //
            // Without Caching; play 0, 1, 2, 3
            //
            for (i=0; i<4; i++)
            {
                sprintf(szUrl, "http://%s/nums/%d.raw", BASE_URL, i) ;
                osPrintf("Playing %s\n", szUrl) ;

                Url url(szUrl) ;

                MpStreamPlayer* pPlayer = new MpStreamPlayer(mCfg->getFlowGraph()->getMsgQ(), url, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerUnrealized) ;

                MyPlayerListenerPoller* pPoller = new MyPlayerListenerPoller() ;
                status = pPlayer->addListener(pPoller, (void*) i) ;
                CPPUNIT_ASSERT(status == OS_SUCCESS) ;

                MyPlayerListenerHistoryKeeper* pVerifier = new MyPlayerListenerHistoryKeeper() ;
                status = pPlayer->addListener(pVerifier, (void*) i) ;
                CPPUNIT_ASSERT(status == OS_SUCCESS) ;

                status = pPlayer->realize(FALSE) ;              CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                bRC = pPoller->waitForState(PlayerRealized);    CPPUNIT_ASSERT(bRC ) ;
                status = pPlayer->getState(state) ;             CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerRealized) ;

                status = pPlayer->prefetch(FALSE) ;             CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                bRC = pPoller->waitForState(PlayerPrefetched);  CPPUNIT_ASSERT(bRC) ;
                status = pPlayer->getState(state) ;             CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerPrefetched) ;

                status = pPlayer->play(FALSE) ;                 CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                bRC = pPoller->waitForState(PlayerPlaying);     CPPUNIT_ASSERT(bRC) ;
                status = pPlayer->getState(state) ;             CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerPlaying) ;
                bRC = pPoller->waitForState(PlayerStopped);     CPPUNIT_ASSERT(bRC) ;
                status = pPlayer->getState(state) ;             CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerStopped) ;

                status = pPlayer->stop() ;                      CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                status = pPlayer->getState(state) ;             CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerStopped) ;

                int expectedHistory[] =
                    {
                        PlayerRealized,
                        PlayerPrefetched,
                        PlayerPlaying,
                        PlayerStopped,
                        -1,
                    } ;

                UtlBoolean bMatches = pVerifier->matchesHistory((void*) i, expectedHistory) ;
                CPPUNIT_ASSERT(bMatches) ;

                status = pPlayer->removeListener(pPoller) ;     CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                status = pPlayer->removeListener(pVerifier) ;   CPPUNIT_ASSERT(status == OS_SUCCESS) ;

                status = pPlayer->removeListener(pPoller) ;     CPPUNIT_ASSERT(status == OS_NOT_FOUND) ;
                status = pPlayer->removeListener(pVerifier) ;   CPPUNIT_ASSERT(status == OS_NOT_FOUND) ;

                delete pPlayer ;
                delete pPoller ;
                delete pVerifier ;
            }
        }

        void testRawAsynchronousCache()
        {
            PlayerState state ;
            UtlBoolean bRC ;
            OsStatus status ;
            char szUrl[128] ;
            int i ;

            osPrintf("testRawAsynchronousCache\n") ;

            //
            // With Caching; play 0, 0, 1, 1, 2, 2, 3, 3
            //
            for (i=0; i<4; i++)
            {
                sprintf(szUrl, "http://%s/nums/%d.raw", BASE_URL, i) ;
                osPrintf("Playing %s\n", szUrl) ;

                Url url(szUrl) ;

                MpStreamPlayer* pPlayer = new MpStreamPlayer(mCfg->getFlowGraph()->getMsgQ(), url, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW | STREAM_HINT_CACHE) ;
                status = pPlayer->getState(state) ;             CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerUnrealized) ;

                MyPlayerListenerPoller* pPoller = new MyPlayerListenerPoller() ;
                status = pPlayer->addListener(pPoller, (void*) i) ;
                CPPUNIT_ASSERT(status == OS_SUCCESS) ;

                MyPlayerListenerHistoryKeeper* pVerifier = new MyPlayerListenerHistoryKeeper() ;
                status = pPlayer->addListener(pVerifier, (void*) i) ;
                CPPUNIT_ASSERT(status == OS_SUCCESS) ;

                status = pPlayer->realize(FALSE) ;              CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                bRC = pPoller->waitForState(PlayerRealized);    CPPUNIT_ASSERT(bRC ) ;
                status = pPlayer->getState(state) ;             CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerRealized) ;

                status = pPlayer->prefetch(FALSE) ;             CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                bRC = pPoller->waitForState(PlayerPrefetched);  CPPUNIT_ASSERT(bRC) ;
                status = pPlayer->getState(state) ;             CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerPrefetched) ;

                status = pPlayer->play(FALSE) ;                 CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                bRC = pPoller->waitForState(PlayerPlaying);     CPPUNIT_ASSERT(bRC) ;
                // The state could be playingStopped already (RACE)
                status = pPlayer->getState(state) ;             CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerPlaying) ;
                bRC = pPoller->waitForState(PlayerStopped);     CPPUNIT_ASSERT(bRC) ;
                status = pPlayer->getState(state) ;             CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerStopped) ;

                pPoller->clearState() ;

                status = pPlayer->rewind(FALSE) ;               CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                bRC = pPoller->waitForState(PlayerPrefetched);  CPPUNIT_ASSERT(bRC) ;
                status = pPlayer->getState(state) ;             CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerPrefetched) ;

                status = pPlayer->play(FALSE) ;                 CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                bRC = pPoller->waitForState(PlayerPlaying);     CPPUNIT_ASSERT(bRC) ;
                // The state could be playingStopped already (RACE)
                status = pPlayer->getState(state) ;             CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerPlaying) ;
                bRC = pPoller->waitForState(PlayerStopped);     CPPUNIT_ASSERT(bRC) ;
                status = pPlayer->getState(state) ;             CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerStopped) ;

                status = pPlayer->stop() ;                      CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                status = pPlayer->getState(state) ;             CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerStopped) ;

                int expectedHistory[] =
                    {
                        PlayerRealized,
                        PlayerPrefetched,
                        PlayerPlaying,
                        PlayerStopped,
                        PlayerPrefetched,
                        PlayerPlaying,
                        PlayerStopped,
                        -1,
                    } ;

                UtlBoolean bMatches = pVerifier->matchesHistory((void*) i, expectedHistory) ;
                CPPUNIT_ASSERT(bMatches) ;

                status = pPlayer->removeListener(pPoller) ;     CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                status = pPlayer->removeListener(pVerifier) ;   CPPUNIT_ASSERT(status == OS_SUCCESS) ;

                status = pPlayer->removeListener(pPoller) ;     CPPUNIT_ASSERT(status == OS_NOT_FOUND) ;
                status = pPlayer->removeListener(pVerifier) ;   CPPUNIT_ASSERT(status == OS_NOT_FOUND) ;

                delete pPlayer ;
                delete pPoller ;
                delete pVerifier ;
            }
        }

        void testRawAsynchronousAbortNonCache()
        {
            PlayerState state ;
            UtlBoolean bRC ;
            OsStatus status ;
            char szUrl[128] ;
            int i ;

            osPrintf("testRawAsynchronousAbortNonCache\n") ;

            //
            // Abort while playing...  No Cache
            //
            for (i=0; i<4; i++)
            {
                sprintf(szUrl, "http://%s/longstream.raw", BASE_URL) ;
                osPrintf("Playing %s\n", szUrl) ;

                Url url(szUrl) ;

                MpStreamPlayer* pPlayer = new MpStreamPlayer(mCfg->getFlowGraph()->getMsgQ(), url, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerUnrealized) ;

                MyPlayerListenerPoller* pPoller = new MyPlayerListenerPoller() ;
                status = pPlayer->addListener(pPoller, (void*) i) ;
                CPPUNIT_ASSERT(status == OS_SUCCESS) ;

                MyPlayerListenerHistoryKeeper* pVerifier = new MyPlayerListenerHistoryKeeper() ;
                status = pPlayer->addListener(pVerifier, (void*) i) ;
                CPPUNIT_ASSERT(status == OS_SUCCESS) ;

                status = pPlayer->realize(FALSE) ;              CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                bRC = pPoller->waitForState(PlayerRealized);    CPPUNIT_ASSERT(bRC ) ;
                status = pPlayer->getState(state) ;             CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerRealized) ;

                status = pPlayer->prefetch(FALSE) ;             CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                bRC = pPoller->waitForState(PlayerPrefetched);  CPPUNIT_ASSERT(bRC) ;
                status = pPlayer->getState(state) ;             CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerPrefetched) ;

                status = pPlayer->play(FALSE) ;                 CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                bRC = pPoller->waitForState(PlayerPlaying);     CPPUNIT_ASSERT(bRC) ;
                status = pPlayer->getState(state) ;             CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerPlaying) ;

                // Wait a bit
                OsTask::delay(2000) ;

                status = pPlayer->stop() ;                      CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                bRC = pPoller->waitForState(PlayerAborted);     CPPUNIT_ASSERT(bRC) ;
                status = pPlayer->getState(state) ;             CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerAborted) ;

                int expectedHistory[] =
                    {
                        PlayerRealized,
                        PlayerPrefetched,
                        PlayerPlaying,
                        PlayerAborted,
                        -1,
                    } ;

                UtlBoolean bMatches = pVerifier->matchesHistory((void*) i, expectedHistory) ;
                CPPUNIT_ASSERT(bMatches) ;

                status = pPlayer->removeListener(pPoller) ;     CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                status = pPlayer->removeListener(pVerifier) ;   CPPUNIT_ASSERT(status == OS_SUCCESS) ;

                status = pPlayer->removeListener(pPoller) ;     CPPUNIT_ASSERT(status == OS_NOT_FOUND) ;
                status = pPlayer->removeListener(pVerifier) ;   CPPUNIT_ASSERT(status == OS_NOT_FOUND) ;

                delete pPlayer ;
                delete pPoller ;
                delete pVerifier ;
            }
        }


        void testRawAsynchronousPauseNonCache()
        {
            PlayerState state ;
            UtlBoolean bRC ;
            OsStatus status ;
            char szUrl[128] ;
            int i ;

            osPrintf("testRawAsynchronousPauseNonCache\n") ;

            //
            // Abort while playing...  No Cache
            //
            for (i=0; i<4; i++)
            {
                sprintf(szUrl, "http://%s/longstream.raw", BASE_URL) ;
                osPrintf("Playing %s\n", szUrl) ;

                Url url(szUrl) ;

                MpStreamPlayer* pPlayer = new MpStreamPlayer(mCfg->getFlowGraph()->getMsgQ(), url, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerUnrealized) ;

                MyPlayerListenerPoller* pPoller = new MyPlayerListenerPoller() ;
                status = pPlayer->addListener(pPoller, (void*) i) ;
                CPPUNIT_ASSERT(status == OS_SUCCESS) ;

                MyPlayerListenerHistoryKeeper* pVerifier = new MyPlayerListenerHistoryKeeper() ;
                status = pPlayer->addListener(pVerifier, (void*) i) ;
                CPPUNIT_ASSERT(status == OS_SUCCESS) ;

                status = pPlayer->realize(FALSE) ;              CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                bRC = pPoller->waitForState(PlayerRealized);    CPPUNIT_ASSERT(bRC ) ;
                status = pPlayer->getState(state) ;             CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerRealized) ;

                status = pPlayer->prefetch(FALSE) ;             CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                bRC = pPoller->waitForState(PlayerPrefetched);  CPPUNIT_ASSERT(bRC) ;
                status = pPlayer->getState(state) ;             CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerPrefetched) ;

                status = pPlayer->play(FALSE) ;                 CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                bRC = pPoller->waitForState(PlayerPlaying);     CPPUNIT_ASSERT(bRC) ;
                status = pPlayer->getState(state) ;             CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerPlaying) ;

                // Wait a bit
                OsTask::delay(2000) ;

                status = pPlayer->pause() ;
                bRC = pPoller->waitForState(PlayerPaused);      CPPUNIT_ASSERT(bRC) ;
                status = pPlayer->getState(state) ;             CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerPaused) ;

                pPoller->clearState() ;

                // Wait a bit more
                OsTask::delay(2000) ;

                status = pPlayer->play(FALSE) ;                 CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                bRC = pPoller->waitForState(PlayerPlaying);     CPPUNIT_ASSERT(bRC) ;
                status = pPlayer->getState(state) ;             CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerPlaying) ;

                // Wait a bit more
                OsTask::delay(2000) ;

                status = pPlayer->stop() ;                      CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                bRC = pPoller->waitForState(PlayerAborted);     CPPUNIT_ASSERT(bRC) ;
                status = pPlayer->getState(state) ;             CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerAborted) ;

                int expectedHistory[] =
                    {
                        PlayerRealized,
                        PlayerPrefetched,
                        PlayerPlaying,
                        PlayerPaused,
                        PlayerPlaying,
                        PlayerAborted,
                        -1,
                    } ;

                UtlBoolean bMatches = pVerifier->matchesHistory((void*) i, expectedHistory) ;
                CPPUNIT_ASSERT(bMatches) ;

                status = pPlayer->removeListener(pPoller) ;     CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                status = pPlayer->removeListener(pVerifier) ;   CPPUNIT_ASSERT(status == OS_SUCCESS) ;

                status = pPlayer->removeListener(pPoller) ;     CPPUNIT_ASSERT(status == OS_NOT_FOUND) ;
                status = pPlayer->removeListener(pVerifier) ;   CPPUNIT_ASSERT(status == OS_NOT_FOUND) ;

                delete pPlayer ;
                delete pPoller ;
                delete pVerifier ;
            }
        }


        void testRawAsynchronousAbortCache()
        {
            PlayerState state ;
            UtlBoolean bRC ;
            OsStatus status ;
            char szUrl[128] ;
            int i ;

            osPrintf("testRawAsynchronousAbortCache\n") ;

            //
            // Abort while playing...  Cache
            //
            for (i=0; i<4; i++)
            {
                sprintf(szUrl, "http://%s/longstream.raw", BASE_URL) ;
                osPrintf("Playing %s\n", szUrl) ;

                Url url(szUrl) ;

                MpStreamPlayer* pPlayer = new MpStreamPlayer(mCfg->getFlowGraph()->getMsgQ(), url, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW | STREAM_HINT_CACHE) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerUnrealized) ;

                MyPlayerListenerPoller* pPoller = new MyPlayerListenerPoller() ;
                status = pPlayer->addListener(pPoller, (void*) i) ;
                CPPUNIT_ASSERT(status == OS_SUCCESS) ;

                MyPlayerListenerHistoryKeeper* pVerifier = new MyPlayerListenerHistoryKeeper() ;
                status = pPlayer->addListener(pVerifier, (void*) i) ;
                CPPUNIT_ASSERT(status == OS_SUCCESS) ;

                status = pPlayer->realize(FALSE) ;              CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                bRC = pPoller->waitForState(PlayerRealized);    CPPUNIT_ASSERT(bRC ) ;
                status = pPlayer->getState(state) ;             CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerRealized) ;

                status = pPlayer->prefetch(FALSE) ;             CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                bRC = pPoller->waitForState(PlayerPrefetched);  CPPUNIT_ASSERT(bRC) ;
                status = pPlayer->getState(state) ;             CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerPrefetched) ;

                status = pPlayer->play(FALSE) ;                 CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                bRC = pPoller->waitForState(PlayerPlaying);     CPPUNIT_ASSERT(bRC) ;
                status = pPlayer->getState(state) ;             CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerPlaying) ;

                // Wait a bit
                OsTask::delay(2000) ;

                status = pPlayer->stop() ;                      CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                bRC = pPoller->waitForState(PlayerAborted);     CPPUNIT_ASSERT(bRC) ;
                status = pPlayer->getState(state) ;             CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerAborted) ;

                int expectedHistory[] =
                    {
                        PlayerRealized,
                        PlayerPrefetched,
                        PlayerPlaying,
                        PlayerAborted,
                        -1,
                    } ;

                UtlBoolean bMatches = pVerifier->matchesHistory((void*) i, expectedHistory) ;
                CPPUNIT_ASSERT(bMatches) ;

                status = pPlayer->removeListener(pPoller) ;     CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                status = pPlayer->removeListener(pVerifier) ;   CPPUNIT_ASSERT(status == OS_SUCCESS) ;

                status = pPlayer->removeListener(pPoller) ;     CPPUNIT_ASSERT(status == OS_NOT_FOUND) ;
                status = pPlayer->removeListener(pVerifier) ;   CPPUNIT_ASSERT(status == OS_NOT_FOUND) ;

                delete pPlayer ;
                delete pPoller ;
                delete pVerifier ;
            }
        }

        void testWavSynchronousNonCache()
        {
            PlayerState state ;
            OsStatus status ;
            char szUrl[128] ;
            int i ;

            osPrintf("testSynchronousNonCache\n") ;

            //
            // Without Caching; play 0, 1, 2, 3
            //
            for (i=0; i<4; i++)
            {
                sprintf(szUrl, "http://%s/nums/%d.wav", BASE_URL, i) ;
                osPrintf("Playing %s\n", szUrl) ;

                Url url(szUrl) ;

                MpStreamPlayer* pPlayer = new MpStreamPlayer(mCfg->getFlowGraph()->getMsgQ(), url, STREAM_SOUND_LOCAL | STREAM_FORMAT_WAV) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerUnrealized) ;

                status = pPlayer->realize() ;       CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerRealized) ;

                status = pPlayer->prefetch() ;      CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerPrefetched) ;

                status = pPlayer->play() ;          CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerStopped) ;

                status = pPlayer->stop() ;          CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerStopped) ;

                delete pPlayer ;
            }

            osPrintf("exiting testSynchronousNonCache\n") ;
        }

        void testRawASynchronousLoop()
        {
            PlayerState state ;
            OsStatus status ;
            char szUrl[128] ;
            int i ;

            osPrintf("testRawASynchronousLoop\n") ;

            for (i=0; i<TESTING_ATTEMPTS; i++)
            {
                // Set up to fetch [i].wav.
                sprintf(szUrl, "http://%s/nums/%d.wav", BASE_URL, i) ;
                osPrintf("Playing %s\n", szUrl) ;

                Url url(szUrl) ;

                MpStreamPlayer* pPlayer = new MpStreamPlayer(
                    mCfg->getFlowGraph()->getMsgQ(),
                    url,
                    STREAM_SOUND_LOCAL | STREAM_FORMAT_WAV ) ;

                pPlayer->setLoopCount(-1);

                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerUnrealized) ;

                status = pPlayer->realize() ;       CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerRealized) ;

                status = pPlayer->prefetch() ;      CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerPrefetched) ;

                status = pPlayer->play(FALSE) ;     CPPUNIT_ASSERT(status == OS_SUCCESS) ;

                int delay = abs(rand() % 7000) ;
                OsTask::delay(3000 + delay) ;

                status = pPlayer->stop();           CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerAborted || state == PlayerFailed) ;

                delete pPlayer ;
            }
        }

    private:

        MpTestConfig *mCfg;

};

CPPUNIT_TEST_SUITE_REGISTRATION(MpStreamPlayerTest);
