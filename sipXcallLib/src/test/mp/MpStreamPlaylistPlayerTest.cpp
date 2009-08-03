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
#include "mp/MpStreamPlaylistPlayer.h"
#include "os/OsDefs.h"
#include "mp/StreamDefs.h"
#include "net/Url.h"

class MpStreamPlaylistPlayerTest : public CppUnit::TestCase
{

        CPPUNIT_TEST_SUITE(MpStreamPlaylistPlayerTest);
        CPPUNIT_TEST(testRawSynchronousNonCache);
        CPPUNIT_TEST(testRawSynchronousReset);
        CPPUNIT_TEST(testRawSynchronousCache);
        CPPUNIT_TEST(testRawASynchronousNonCache);
        CPPUNIT_TEST(testRawASynchronousCache);
        CPPUNIT_TEST(testRawASynchronousAbortNonCache);
        CPPUNIT_TEST(testRawASynchronousPauseNonCache);
        CPPUNIT_TEST(testRawSynchronousMixedData);
        CPPUNIT_TEST_SUITE_END();

        void testRawSynchronousNonCache()
        {
            PlayerState state ;
            OsStatus status ;
            char szUrl[128] ;
            int i ;

            osPrintf("testRawSynchronousNonCache\n") ;

            for (int j=0; j<4; j++)
            {
                MpStreamPlaylistPlayer* pPlayer = new MpStreamPlaylistPlayer(mCfg->getFlowGraph()->getMsgQ()) ;
                for (i=0; i<4; i++)
                {
                    sprintf(szUrl, "Playlist +http://%s/nums/%d.raw", BASE_URL, i) ;
                    osPrintf("Playing %s\n", szUrl) ;

                    Url url(szUrl) ;
                    pPlayer->add(url, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW) ;
                }

                MyPlayerListenerPoller* pPoller = new MyPlayerListenerPoller() ;
                status = pPlayer->addListener(pPoller, (void*) i) ;
                CPPUNIT_ASSERT(status == OS_SUCCESS) ;

                MyPlayerListenerHistoryKeeper* pVerifier = new MyPlayerListenerHistoryKeeper() ;
                status = pPlayer->addListener(pVerifier, (void*) i) ;
                CPPUNIT_ASSERT(status == OS_SUCCESS) ;

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


        void testRawSynchronousReset()
        {
            PlayerState state ;
            OsStatus status ;
            char szUrl[128] ;
            int i ;

            osPrintf("testRawSynchronousReset\n") ;

            MpStreamPlaylistPlayer* pPlayer = new MpStreamPlaylistPlayer(mCfg->getFlowGraph()->getMsgQ()) ;

            for (int j=0; j<4; j++)
            {
                for (i=0; i<4; i++)
                {
                    sprintf(szUrl, "Playlist +http://%s/nums/%d.raw", BASE_URL, i) ;
                    osPrintf("Playing %s\n", szUrl) ;

                    Url url(szUrl) ;
                    pPlayer->add(url, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW | STREAM_HINT_CACHE) ;
                }

                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerUnrealized) ;

                MyPlayerListenerPoller* pPoller = new MyPlayerListenerPoller() ;
                status = pPlayer->addListener(pPoller, (void*) i) ;
                CPPUNIT_ASSERT(status == OS_SUCCESS) ;

                MyPlayerListenerHistoryKeeper* pVerifier = new MyPlayerListenerHistoryKeeper() ;
                status = pPlayer->addListener(pVerifier, (void*) i) ;
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

                status = pPlayer->reset() ;          CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                status = pPlayer->getState(state) ;  CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerUnrealized) ;


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

                delete pPoller ;
                delete pVerifier ;
            }
            delete pPlayer ;
        }



        void testRawSynchronousCache()
        {
            PlayerState state ;
            OsStatus status ;
            char szUrl[128] ;
            int i ;

            osPrintf("testRawSynchronousCache\n") ;

            for (int j=0; j<4; j++)
            {
                MpStreamPlaylistPlayer* pPlayer = new MpStreamPlaylistPlayer(mCfg->getFlowGraph()->getMsgQ()) ;
                for (i=0; i<4; i++)
                {
                    sprintf(szUrl, "Playlist +http://%s/nums/%d.raw", BASE_URL, i) ;
                    osPrintf("Playing %s\n", szUrl) ;

                    Url url(szUrl) ;
                    pPlayer->add(url, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW | STREAM_HINT_CACHE) ;
                }

                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerUnrealized) ;

                MyPlayerListenerPoller* pPoller = new MyPlayerListenerPoller() ;
                status = pPlayer->addListener(pPoller, (void*) i) ;
                CPPUNIT_ASSERT(status == OS_SUCCESS) ;

                MyPlayerListenerHistoryKeeper* pVerifier = new MyPlayerListenerHistoryKeeper() ;
                status = pPlayer->addListener(pVerifier, (void*) i) ;
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


        void testRawASynchronousNonCache()
        {
            UtlBoolean bRC ;
            PlayerState state ;
            OsStatus status ;
            char szUrl[128] ;
            int i ;

            osPrintf("testRawASynchronousNonCache\n") ;

            for (int j=0; j<4; j++)
            {
                MpStreamPlaylistPlayer* pPlayer = new MpStreamPlaylistPlayer(mCfg->getFlowGraph()->getMsgQ()) ;
                for (i=0; i<4; i++)
                {
                    sprintf(szUrl, "Playlist +http://%s/nums/%d.raw", BASE_URL, i) ;
                    osPrintf("Playing %s\n", szUrl) ;

                    Url url(szUrl) ;
                    pPlayer->add(url, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW) ;
                }

                MyPlayerListenerPoller* pPoller = new MyPlayerListenerPoller() ;
                status = pPlayer->addListener(pPoller, (void*) i) ;
                CPPUNIT_ASSERT(status == OS_SUCCESS) ;

                MyPlayerListenerHistoryKeeper* pVerifier = new MyPlayerListenerHistoryKeeper() ;
                status = pPlayer->addListener(pVerifier, (void*) i) ;
                CPPUNIT_ASSERT(status == OS_SUCCESS) ;

                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerUnrealized) ;

                status = pPlayer->realize(FALSE) ;  CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                bRC = pPoller->waitForState(PlayerRealized);    CPPUNIT_ASSERT(bRC) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerRealized) ;

                status = pPlayer->prefetch(FALSE) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                bRC = pPoller->waitForState(PlayerPrefetched);    CPPUNIT_ASSERT(bRC) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerPrefetched) ;

                status = pPlayer->play(FALSE) ;     CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                bRC = pPoller->waitForState(PlayerPlaying);    CPPUNIT_ASSERT(bRC) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerPlaying) ;
                bRC = pPoller->waitForState(PlayerStopped);    CPPUNIT_ASSERT(bRC) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerStopped) ;

                status = pPlayer->stop() ;          CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                bRC = pPoller->waitForState(PlayerStopped);    CPPUNIT_ASSERT(bRC) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
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


        void testRawASynchronousCache()
        {
            UtlBoolean bRC ;
            PlayerState state ;
            OsStatus status ;
            char szUrl[128] ;
            int i ;

            osPrintf("testRawASynchronousCache\n") ;

            for (int j=0; j<4; j++)
            {
                MpStreamPlaylistPlayer* pPlayer = new MpStreamPlaylistPlayer(mCfg->getFlowGraph()->getMsgQ()) ;
                for (i=0; i<4; i++)
                {
                    sprintf(szUrl, "Playlist +http://%s/nums/%d.raw", BASE_URL, i) ;
                    osPrintf("Playing %s\n", szUrl) ;

                    Url url(szUrl) ;
                    pPlayer->add(url, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW | STREAM_HINT_CACHE) ;
                }

                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerUnrealized) ;

                MyPlayerListenerPoller* pPoller = new MyPlayerListenerPoller() ;
                status = pPlayer->addListener(pPoller, (void*) i) ;
                CPPUNIT_ASSERT(status == OS_SUCCESS) ;

                MyPlayerListenerHistoryKeeper* pVerifier = new MyPlayerListenerHistoryKeeper() ;
                status = pPlayer->addListener(pVerifier, (void*) i) ;
                CPPUNIT_ASSERT(status == OS_SUCCESS) ;

                status = pPlayer->realize() ;       CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                bRC = pPoller->waitForState(PlayerRealized);    CPPUNIT_ASSERT(bRC) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerRealized) ;

                status = pPlayer->prefetch() ;      CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                bRC = pPoller->waitForState(PlayerPrefetched);    CPPUNIT_ASSERT(bRC) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerPrefetched) ;

                status = pPlayer->play() ;          CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                bRC = pPoller->waitForState(PlayerPlaying);    CPPUNIT_ASSERT(bRC) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                bRC = pPoller->waitForState(PlayerStopped);    CPPUNIT_ASSERT(bRC) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerStopped) ;

                pPoller->clearState() ;

                status = pPlayer->rewind() ;        CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                bRC = pPoller->waitForState(PlayerPrefetched);    CPPUNIT_ASSERT(bRC) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerPrefetched) ;

                status = pPlayer->play() ;          CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                bRC = pPoller->waitForState(PlayerPlaying);    CPPUNIT_ASSERT(bRC) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                bRC = pPoller->waitForState(PlayerStopped);    CPPUNIT_ASSERT(bRC) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerStopped) ;

                status = pPlayer->stop() ;          CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                bRC = pPoller->waitForState(PlayerStopped);    CPPUNIT_ASSERT(bRC) ;
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


        void testRawASynchronousAbortNonCache()
        {
            UtlBoolean bRC ;
            PlayerState state ;
            OsStatus status ;
            char szUrl[128] ;
            int i ;

            osPrintf("testRawASynchronousAbortNonCache\n") ;

            for (int j=0; j<4; j++)
            {
                MpStreamPlaylistPlayer* pPlayer = new MpStreamPlaylistPlayer(mCfg->getFlowGraph()->getMsgQ()) ;
                for (i=0; i<2; i++)
                {
                    sprintf(szUrl, "Playlist +http://%s/longstream.raw", BASE_URL) ;
                    osPrintf("Playing %s\n", szUrl) ;

                    Url url(szUrl) ;
                    pPlayer->add(url, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW) ;
                }

                MyPlayerListenerPoller* pPoller = new MyPlayerListenerPoller() ;
                status = pPlayer->addListener(pPoller, (void*) i) ;
                CPPUNIT_ASSERT(status == OS_SUCCESS) ;

                MyPlayerListenerHistoryKeeper* pVerifier = new MyPlayerListenerHistoryKeeper() ;
                status = pPlayer->addListener(pVerifier, (void*) i) ;
                CPPUNIT_ASSERT(status == OS_SUCCESS) ;

                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerUnrealized) ;

                status = pPlayer->realize(FALSE) ;  CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                bRC = pPoller->waitForState(PlayerRealized);    CPPUNIT_ASSERT(bRC ) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerRealized) ;

                status = pPlayer->prefetch(FALSE) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                bRC = pPoller->waitForState(PlayerPrefetched);    CPPUNIT_ASSERT(bRC ) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerPrefetched) ;

                status = pPlayer->play(FALSE) ;     CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                bRC = pPoller->waitForState(PlayerPlaying);    CPPUNIT_ASSERT(bRC ) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerPlaying) ;

                OsTask::delay(2000) ;

                status = pPlayer->stop() ;          CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                bRC = pPoller->waitForState(PlayerStopped);    CPPUNIT_ASSERT(bRC ) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
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


        void testRawASynchronousPauseNonCache()
        {
            UtlBoolean bRC ;
            PlayerState state ;
            OsStatus status ;
            char szUrl[128] ;
            int i ;

            osPrintf("testRawASynchronousPauseNonCache\n") ;

            for (int j=0; j<4; j++)
            {
                MpStreamPlaylistPlayer* pPlayer = new MpStreamPlaylistPlayer(mCfg->getFlowGraph()->getMsgQ()) ;
                for (i=0; i<2; i++)
                {
                    sprintf(szUrl, "Playlist +http://%s/longstream.raw", BASE_URL) ;
                    osPrintf("Playing %s\n", szUrl) ;

                    Url url(szUrl) ;
                    pPlayer->add(url, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW) ;
                }

                MyPlayerListenerPoller* pPoller = new MyPlayerListenerPoller() ;
                status = pPlayer->addListener(pPoller, (void*) i) ;
                CPPUNIT_ASSERT(status == OS_SUCCESS) ;

                MyPlayerListenerHistoryKeeper* pVerifier = new MyPlayerListenerHistoryKeeper() ;
                status = pPlayer->addListener(pVerifier, (void*) i) ;
                CPPUNIT_ASSERT(status == OS_SUCCESS) ;

                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerUnrealized) ;

                status = pPlayer->realize(FALSE) ;  CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                bRC = pPoller->waitForState(PlayerRealized);    CPPUNIT_ASSERT(bRC ) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerRealized) ;

                status = pPlayer->prefetch(FALSE) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                bRC = pPoller->waitForState(PlayerPrefetched);    CPPUNIT_ASSERT(bRC ) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerPrefetched) ;

                status = pPlayer->play(FALSE) ;     CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                bRC = pPoller->waitForState(PlayerPlaying);    CPPUNIT_ASSERT(bRC ) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerPlaying) ;

                OsTask::delay(2000) ;

                status = pPlayer->pause() ;          CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                bRC = pPoller->waitForState(PlayerPaused);    CPPUNIT_ASSERT(bRC ) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerPaused) ;

                OsTask::delay(1000) ;
                pPoller->clearState() ;

                status = pPlayer->play(FALSE) ;     CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                bRC = pPoller->waitForState(PlayerPlaying);    CPPUNIT_ASSERT(bRC ) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerPlaying) ;

                OsTask::delay(2000) ;

                status = pPlayer->stop() ;          CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                bRC = pPoller->waitForState(PlayerStopped);    CPPUNIT_ASSERT(bRC ) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerStopped) ;

                int expectedHistory[] =
                    {
                        PlayerRealized,
                        PlayerPrefetched,
                        PlayerPlaying,
                        PlayerPaused,
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


        void testRawSynchronousMixedData()
        {
            UtlBoolean bRC ;
            PlayerState state ;
            OsStatus status ;
            char szUrl[128] ;

            osPrintf("testRawSynchronousMixedData\n") ;

            for (int j=0; j<4; j++)
            {
                MpStreamPlaylistPlayer* pPlayer = new MpStreamPlaylistPlayer(mCfg->getFlowGraph()->getMsgQ()) ;
#if 0 /* [ */
                sprintf(szUrl, "http://%s/nums/0.raw", BASE_URL) ;
                pPlayer->add(Url(szUrl), STREAM_SOUND_LOCAL) ;
#endif /* ] */

                sprintf(szUrl, "http://64.55.140.162/wakeup-music/chillout/3.mp3") ;
                Url url(szUrl);
                pPlayer->add(url, STREAM_SOUND_LOCAL) ;

                MyPlayerListenerPoller* pPoller = new MyPlayerListenerPoller() ;
                status = pPlayer->addListener(pPoller, (void*) j) ;
                CPPUNIT_ASSERT(status == OS_SUCCESS) ;

                MyPlayerListenerHistoryKeeper* pVerifier = new MyPlayerListenerHistoryKeeper() ;
                status = pPlayer->addListener(pVerifier, (void*) j) ;
                CPPUNIT_ASSERT(status == OS_SUCCESS) ;

                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerUnrealized) ;

                status = pPlayer->realize(FALSE) ;  CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                bRC = pPoller->waitForState(PlayerRealized);    CPPUNIT_ASSERT(bRC ) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerRealized) ;

                status = pPlayer->prefetch(FALSE) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                bRC = pPoller->waitForState(PlayerPrefetched);    CPPUNIT_ASSERT(bRC ) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerPrefetched) ;

                status = pPlayer->play(TRUE) ;     CPPUNIT_ASSERT(status == OS_SUCCESS) ;
#if 0 /* [ */
                bRC = pPoller->waitForState(PlayerPlaying);    CPPUNIT_ASSERT(bRC ) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerPlaying) ;
#endif /* ] */
#if 0 /* [ */
                OsTask::delay(2000) ;

                status = pPlayer->pause() ;          CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                bRC = pPoller->waitForState(PlayerPaused);    CPPUNIT_ASSERT(bRC ) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerPaused) ;

                OsTask::delay(1000) ;
                pPoller->clearState() ;

                status = pPlayer->play(FALSE) ;     CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                bRC = pPoller->waitForState(PlayerPlaying);    CPPUNIT_ASSERT(bRC ) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerPlaying) ;

                OsTask::delay(2000) ;

                status = pPlayer->stop() ;          CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                bRC = pPoller->waitForState(PlayerStopped);    CPPUNIT_ASSERT(bRC ) ;
                status = pPlayer->getState(state) ; CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                CPPUNIT_ASSERT(state == PlayerStopped) ;

                int expectedHistory[] =
                    {
                        PlayerRealized,
                        PlayerPrefetched,
                        PlayerPlaying,
                        PlayerPaused,
                        PlayerPlaying,
                        PlayerStopped,
                        -1,
                    } ;

                UtlBoolean bMatches = pVerifier->matchesHistory((void*) j, expectedHistory) ;
                CPPUNIT_ASSERT(bMatches) ;
#endif /* ] */

                status = pPlayer->removeListener(pPoller) ;     CPPUNIT_ASSERT(status == OS_SUCCESS) ;
                status = pPlayer->removeListener(pVerifier) ;   CPPUNIT_ASSERT(status == OS_SUCCESS) ;

                status = pPlayer->removeListener(pPoller) ;     CPPUNIT_ASSERT(status == OS_NOT_FOUND) ;
                status = pPlayer->removeListener(pVerifier) ;   CPPUNIT_ASSERT(status == OS_NOT_FOUND) ;

                delete pPlayer ;
                delete pPoller ;
                delete pVerifier ;
            }
        }
    private:

        MpTestConfig *mCfg;

};

CPPUNIT_TEST_SUITE_REGISTRATION(MpStreamPlaylistPlayerTest);
