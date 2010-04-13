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
#include "mp/MpStreamQueuePlayer.h"
#include "os/OsDefs.h"
#include "mp/StreamDefs.h"
#include "net/Url.h"

class MpStreamQueuePlayerTest : public CppUnit::TestCase
{

        CPPUNIT_TEST_SUITE(MpStreamQueuePlayerTest);
        CPPUNIT_TEST(testRawSynchronousSingle);
        CPPUNIT_TEST(testRawSynchronousMultiple);
        CPPUNIT_TEST(testRawSynchronousMultipleDelay);
        CPPUNIT_TEST(testRawSynchronousMultiplePreload);
        CPPUNIT_TEST(testRawSynchronousMultipleClear);
        CPPUNIT_TEST(testRawSynchronousMultipleReset);
        CPPUNIT_TEST(testRawSynchronousMultipleManyBad);
        CPPUNIT_TEST(testRawSynchronousMultipleMany);
        CPPUNIT_TEST(testRawSynchronousMultipleDelete);
        CPPUNIT_TEST(testRawSynchronousMultipleBadRaw);
        CPPUNIT_TEST(testRawSynchronousMultipleBadWav);
        CPPUNIT_TEST(testRawSynchronousManyBadRaw);
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp()
        {
            mCfg = MpTestConfig::getTestInstance();
        }

        void testRawSynchronousSingle()
        {
            char szUrl[128] ;
            int i ;

            osPrintf("testRawSynchronousSingle\n") ;

            for (i=0; i<TESTING_ATTEMPTS; i++)
            {
                sprintf(szUrl, "http://%s/nums/%d.raw", BASE_URL, i) ;
                osPrintf("Playing %s\n", szUrl) ;

                Url url(szUrl) ;

                MpStreamQueuePlayer* pPlayer = new MpStreamQueuePlayer(mCfg->getFlowGraph()->getMsgQ()) ;

                pPlayer->add(url, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW) ;
                pPlayer->play() ;
                pPlayer->wait() ;

                delete pPlayer ;
            }
        }

        void testRawSynchronousMultiple()
        {
            osPrintf("testRawSynchronousMultiple\n") ;

            for (int j=0; j<TESTING_ATTEMPTS; j++)
            {
                char szUrl[128] ;
                int i ;

                MpStreamQueuePlayer* pPlayer = new MpStreamQueuePlayer(mCfg->getFlowGraph()->getMsgQ()) ;

                for (i=0; i<4; i++)
                {
                    sprintf(szUrl, "http://%s/nums/%d.raw", BASE_URL, i) ;
                    osPrintf("Playing %s\n", szUrl) ;

                    Url url(szUrl) ;

                    pPlayer->add(url, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW) ;
                }

                pPlayer->play() ;
                pPlayer->wait() ;
                delete pPlayer ;
            }
        }

        void testRawSynchronousMultipleDelay()
        {
            osPrintf("testRawSynchronousMultipleDelay\n") ;

            for (int j=0; j<TESTING_ATTEMPTS; j++)
            {
                char szUrl[128] ;
                int i ;

                MpStreamQueuePlayer* pPlayer = new MpStreamQueuePlayer(mCfg->getFlowGraph()->getMsgQ()) ;

                for (i=0; i<4; i++)
                {
                    sprintf(szUrl, "http://%s/nums/%d.raw", BASE_URL, i) ;
                    osPrintf("Playing %s\n", szUrl) ;

                    Url url(szUrl) ;

                    pPlayer->add(url, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW) ;
                    pPlayer->play() ;
                    OsTask::delay(500) ;
                }

                pPlayer->wait() ;
                delete pPlayer ;
            }
        }


        void testRawSynchronousMultiplePreload()
        {
            osPrintf("testRawSynchronousMultiplePreload\n") ;

            for (int j=0; j<TESTING_ATTEMPTS; j++)
            {
                char szUrl[128] ;
                int i ;

                MpStreamQueuePlayer* pPlayer = new MpStreamQueuePlayer(mCfg->getFlowGraph()->getMsgQ()) ;

                for (i=0; i<2; i++)
                {
                    sprintf(szUrl, "http://%s/nums/%d.raw", BASE_URL, i) ;
                    osPrintf("Playing %s\n", szUrl) ;

                    Url url(szUrl) ;

                    pPlayer->add(url, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW) ;
                }

                pPlayer->play() ;

                for (i=2; i<5; i++)
                {
                    sprintf(szUrl, "http://%s/nums/%d.raw", BASE_URL, i) ;
                    osPrintf("Playing %s\n", szUrl) ;

                    Url url(szUrl) ;

                    pPlayer->add(url, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW) ;
                }

                pPlayer->wait() ;
                pPlayer->play() ;
                pPlayer->wait() ;

                delete pPlayer ;
            }
        }

        void testRawSynchronousMultipleClear()
        {
            osPrintf("testRawSynchronousMultipleClear\n") ;

            for (int j=0; j<TESTING_ATTEMPTS; j++)
            {
                char szUrl[128] ;
                int i ;

                MpStreamQueuePlayer* pPlayer = new MpStreamQueuePlayer(mCfg->getFlowGraph()->getMsgQ()) ;

                for (i=0; i<2; i++)
                {
                    sprintf(szUrl, "http://%s/nums/%d.raw", BASE_URL, i) ;
                    osPrintf("Playing %s\n", szUrl) ;

                    Url url(szUrl) ;

                    pPlayer->add(url, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW) ;
                }

                pPlayer->play() ;

                for (i=2; i<4; i++)
                {
                    sprintf(szUrl, "http://%s/nums/%d.raw", BASE_URL, i) ;
                    osPrintf("Playing %s\n", szUrl) ;

                    Url url(szUrl) ;

                    pPlayer->add(url, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW) ;
                }

                OsTask::delay(1000) ;

                pPlayer->clear() ;

                for (i=5; i<7; i++)
                {
                    sprintf(szUrl, "http://%s/nums/%d.raw", BASE_URL, i) ;
                    osPrintf("Playing %s\n", szUrl) ;

                    Url url(szUrl) ;

                    pPlayer->add(url, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW) ;
                }


                pPlayer->wait() ;
                pPlayer->play() ;
                pPlayer->wait() ;

                delete pPlayer ;
            }
        }


        void testRawSynchronousMultipleReset()
        {
            osPrintf("testRawSynchronousMultipleReset\n") ;

            for (int j=0; j<TESTING_ATTEMPTS; j++)
            {
                MyStreamQueueHistoryKeeper* pListener = new MyStreamQueueHistoryKeeper() ;
                char szUrl[128] ;

                MpStreamQueuePlayer* pPlayer = new MpStreamQueuePlayer(mCfg->getFlowGraph()->getMsgQ()) ;
                pPlayer->addListener(pListener) ;

                sprintf(szUrl, "http://%s/longstream.raw", BASE_URL) ;
                osPrintf("Playing %s\n", szUrl) ;

                Url url(szUrl) ;

                pPlayer->add(url, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW) ;
                pPlayer->play() ;

                int delay = abs(rand() % 3000) ;
                if (delay > 300)
                    OsTask::delay(delay) ;

                pPlayer->reset() ;

                sprintf(szUrl, "http://%s/nums/8.wav", BASE_URL) ;
                Url url2(szUrl);

                pPlayer->add(url2, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW) ;
                pPlayer->add(url2, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW) ;
                pPlayer->add(url2, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW) ;
                pPlayer->play() ;
                pPlayer->wait() ;

                pPlayer->add(url2, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW) ;
                pPlayer->play() ;
                pPlayer->wait() ;

                pPlayer->removeListener(pListener) ;
                delete pPlayer ;
                delete pListener ;
            }
        }


        void testRawSynchronousMultipleManyBad()
        {
            osPrintf("testRawSynchronousMultipleManyBad\n") ;

            for (int j=0; j<TESTING_ATTEMPTS; j++)
            {
                char szUrl[128] ;

                MpStreamQueuePlayer* pPlayer = new MpStreamQueuePlayer(mCfg->getFlowGraph()->getMsgQ()) ;

                sprintf(szUrl, "http://%s/nums/1.wav", BASE_URL) ;
                Url url1(szUrl);
                sprintf(szUrl, "http://%s/nums/2.wav", BASE_URL) ;
                Url url2(szUrl);
                sprintf(szUrl, "http://%s/nums/3.wav", BASE_URL) ;
                Url url3(szUrl);
                sprintf(szUrl, "http://%s/nums/4.wav", BASE_URL) ;
                Url url4(szUrl);
                sprintf(szUrl, "http://%s/nums/5.wav", BASE_URL) ;
                Url url5(szUrl);
                sprintf(szUrl, "http://%s/nums/doesnotexist.wav", BASE_URL) ;
                Url urlBad(szUrl);

                pPlayer->add(url1, STREAM_SOUND_LOCAL | STREAM_FORMAT_WAV) ;
                pPlayer->add(url2, STREAM_SOUND_LOCAL | STREAM_FORMAT_WAV) ;
                pPlayer->add(url3, STREAM_SOUND_LOCAL | STREAM_FORMAT_WAV) ;
                pPlayer->add(urlBad, STREAM_SOUND_LOCAL | STREAM_FORMAT_WAV) ;
                pPlayer->add(urlBad, STREAM_SOUND_LOCAL | STREAM_FORMAT_WAV) ;
                pPlayer->add(urlBad, STREAM_SOUND_LOCAL | STREAM_FORMAT_WAV) ;
                pPlayer->add(urlBad, STREAM_SOUND_LOCAL | STREAM_FORMAT_WAV) ;
                pPlayer->add(urlBad, STREAM_SOUND_LOCAL | STREAM_FORMAT_WAV) ;
                pPlayer->add(urlBad, STREAM_SOUND_LOCAL | STREAM_FORMAT_WAV) ;
                pPlayer->add(urlBad, STREAM_SOUND_LOCAL | STREAM_FORMAT_WAV) ;
                pPlayer->add(url4, STREAM_SOUND_LOCAL | STREAM_FORMAT_WAV) ;
                pPlayer->add(url5, STREAM_SOUND_LOCAL | STREAM_FORMAT_WAV) ;
                pPlayer->add(urlBad, STREAM_SOUND_LOCAL | STREAM_FORMAT_WAV) ;

                pPlayer->play() ;
                pPlayer->wait() ;

                delete pPlayer ;
            }
        }



        void testRawSynchronousMultipleMany()
        {
            osPrintf("testRawSynchronousMultipleMany\n") ;

            for (int j=0; j<TESTING_ATTEMPTS; j++)
            {
                MyStreamQueueHistoryKeeper* pListener = new MyStreamQueueHistoryKeeper() ;
                char szUrl[128] ;

                MpStreamQueuePlayer* pPlayer = new MpStreamQueuePlayer(mCfg->getFlowGraph()->getMsgQ()) ;
                pPlayer->addListener(pListener) ;

                sprintf(szUrl, "http://%s/nums/8.wav", BASE_URL) ;
                Url url2(szUrl);

                for (int i = 0; i< 5; i++)
                {
                    pPlayer->add(url2, STREAM_SOUND_LOCAL | STREAM_FORMAT_WAV) ;
                }
                pPlayer->play() ;

                sprintf(szUrl, "http://%s/nums/9.wav", BASE_URL) ;
                Url url3(szUrl);

                for (int i = 0; i< 5; i++)
                {
                    pPlayer->add(url3, STREAM_SOUND_LOCAL | STREAM_FORMAT_WAV) ;
                }

                pPlayer->wait() ;
                pPlayer->play() ;
                pPlayer->wait() ;

                pPlayer->removeListener(pListener) ;
                delete pPlayer ;
                delete pListener ;
            }
        }


        void testRawSynchronousMultipleDelete()
        {
            osPrintf("testRawSynchronousMultipleDelete\n") ;

            for (int j=0; j<TESTING_ATTEMPTS; j++)
            {
                char szUrl[128] ;

                osPrintf("\n\ntestRawSynchronousMultipleDelete (%d of %d): %s\n",
                         j+1, TESTING_ATTEMPTS, BASE_URL) ;

                MpStreamQueuePlayer* pPlayer = new MpStreamQueuePlayer(mCfg->getFlowGraph()->getMsgQ()) ;

                sprintf(szUrl, "http://%s/longstream.raw", BASE_URL) ;
                osPrintf("Playing %s\n", szUrl) ;

                Url url(szUrl) ;

                pPlayer->add(url, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW) ;
                pPlayer->play() ;
                int delay = abs(rand() % 3000) ;
                if ((delay > 32) && (delay < 2500))
                    OsTask::delay(delay) ;
                delete pPlayer ;
            }
        }

        void testRawSynchronousMultipleBadRaw()
        {
            osPrintf("testRawSynchronousMultipleBadRaw\n") ;

            for (int j=0; j<TESTING_ATTEMPTS; j++)
            {
                char szUrl[128] ;
                Url url ;

                MpStreamQueuePlayer* pPlayer = new MpStreamQueuePlayer(mCfg->getFlowGraph()->getMsgQ()) ;

                sprintf(szUrl, "http://%s/noexist.raw", BASE_URL) ;
                osPrintf("Playing %s\n", szUrl) ;
                Url url1(szUrl);
                pPlayer->add(url1, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW) ;
                pPlayer->play() ;
                pPlayer->wait() ;

                sprintf(szUrl, "http://%s/nums/1.raw", BASE_URL) ;
                osPrintf("Playing %s\n", szUrl) ;
                Url url2(szUrl);
                pPlayer->add(url2, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW) ;
                pPlayer->play() ;
                pPlayer->wait() ;

                sprintf(szUrl, "http://%s/noexist.raw", BASE_URL) ;
                osPrintf("Playing %s\n", szUrl) ;
                Url url3(szUrl);
                pPlayer->add(url3, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW) ;
                Url url3a(szUrl);
                osPrintf("Playing %s\n", szUrl) ;
                pPlayer->add(url3a, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW) ;

                sprintf(szUrl, "http://%s/nums/2.raw", BASE_URL) ;
                osPrintf("Playing %s\n", szUrl) ;
                Url url4(szUrl);
                pPlayer->add(url4, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW) ;
                pPlayer->play() ;
                pPlayer->wait() ;

                delete pPlayer ;
            }
        }


        void testRawSynchronousMultipleBadWav()
        {
            osPrintf("testRawSynchronousMultipleBadWav\n") ;

            for (int j=0; j<TESTING_ATTEMPTS; j++)
            {
                char szUrl[128] ;
                Url url ;

                MpStreamQueuePlayer* pPlayer = new MpStreamQueuePlayer(mCfg->getFlowGraph()->getMsgQ()) ;

                sprintf(szUrl, "http://%s/noexist.wav", BASE_URL) ;
                osPrintf("Playing %s\n", szUrl) ;
                Url url1(szUrl);
                pPlayer->add(url1, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW) ;
                pPlayer->play() ;
                pPlayer->wait() ;

                sprintf(szUrl, "http://%s/nums/1.wav", BASE_URL) ;
                osPrintf("Playing %s\n", szUrl) ;
                Url url2(szUrl);
                pPlayer->add(url2, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW) ;
                pPlayer->play() ;
                pPlayer->wait() ;

                sprintf(szUrl, "http://%s/noexist.wav", BASE_URL) ;
                osPrintf("Playing %s\n", szUrl) ;
                Url url3(szUrl);
                pPlayer->add(url3, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW) ;
                osPrintf("Playing %s\n", szUrl) ;
                Url url4(szUrl);
                pPlayer->add(url4, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW) ;

                sprintf(szUrl, "http://%s/nums/2.wav", BASE_URL) ;
                osPrintf("Playing %s\n", szUrl) ;
                Url url5(szUrl);
                pPlayer->add(url5, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW) ;
                pPlayer->play() ;
                pPlayer->wait() ;

                delete pPlayer ;
            }
        }


        void testRawSynchronousManyBadRaw()
        {
            osPrintf("testRawSynchronousManyBadRaw\n") ;

            for (int j=0; j<TESTING_ATTEMPTS; j++)
            {
                char szUrl[128] ;

                MpStreamQueuePlayer* pPlayer = new MpStreamQueuePlayer(mCfg->getFlowGraph()->getMsgQ()) ;

                sprintf(szUrl, "http://%s/noexist.raw", BASE_URL) ;
                osPrintf("Playing %s\n", szUrl) ;
                Url url(szUrl);
                pPlayer->add(url, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW) ;
                pPlayer->play() ;
                pPlayer->wait() ;

                delete pPlayer ;
            }
        }

private:

    MpTestConfig *mCfg;
};

CPPUNIT_TEST_SUITE_REGISTRATION(MpStreamQueuePlayerTest);
