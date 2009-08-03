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
#include "os/OsDefs.h"
#include "mp/MpStreamQueuePlayer.h"
#include "mp/MpStreamPlayer.h"
#include "mp/StreamDefs.h"
#include "mp/MpMisc.h"


class CallManagerPlayerTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(CallManagerPlayerTest);
    CPPUNIT_TEST(testQueuePlayerSimple);
    CPPUNIT_TEST(testBasicPlayerRaw);
    CPPUNIT_TEST(testBasicPlayerRaw);
    CPPUNIT_TEST(testBasicPlayerWav);
    CPPUNIT_TEST(testQueuePlayerSuperDrop);
    CPPUNIT_TEST(testSuperDrop);
    CPPUNIT_TEST(testSuperCreateDestory);
    CPPUNIT_TEST(testQueuePlayerCallGoesAwayFirst);
    CPPUNIT_TEST(testQueuePlayerDropCallMiddle);
    CPPUNIT_TEST(testQueuePlayerManyDropCallRandom);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp()
    {
        mCfg = MpTestConfig::getTestInstance();
    }

    void testQueuePlayerSimple()
    {
       osPrintf("testQueuePlayerSimple\n") ;

       char szUrl[128] ;
       MpStreamQueuePlayer* pQPlayer ;
       MpStreamPlayer* pPlayer =  NULL ;

       for (int j=0; j<TESTING_ATTEMPTS; j++)
       {
          UtlString callId ;

          mCfg->getCallManager()->createCall(&callId) ;
          mCfg->getCallManager()->unholdAllTerminalConnections(callId) ;

          mCfg->getCallManager()->createPlayer(MpPlayer::STREAM_QUEUE_PLAYER, callId, NULL, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW, &pPlayer) ;
          pQPlayer = (MpStreamQueuePlayer*) pPlayer ;
          for (int i=3; i>0; i--)
          {
             sprintf(szUrl, "http://%s/nums/%d.raw", BASE_URL, i) ;
             Url url(szUrl) ;

             pQPlayer->add(url, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW) ;
          }
          pQPlayer->play() ;
          pQPlayer->wait() ;

          mCfg->getCallManager()->destroyPlayer(MpPlayer::STREAM_QUEUE_PLAYER, callId, pPlayer) ;
          mCfg->getCallManager()->drop(callId) ;
       }
    }

    void debug(const char* msg )
    {
       osPrintf("\n******************************\n");
       osPrintf("%s\n", msg);
       osPrintf("******************************\n");
    }

    void testBasicPlayerRaw()
    {
       debug("testBasicPlayerRaw") ;

       char szUrl[128] ;
       MpStreamPlayer* pPlayer =  NULL ;

       for (int j=0; j<3; j++)
       {
          sprintf(szUrl, "http://%s/nums/1.raw", BASE_URL) ;

          Url url(szUrl) ;

          // debug("NEW TEST ATTEMPT ******************************" );
          UtlString callId ;

          mCfg->getCallManager()->createCall(&callId) ;
          mCfg->getCallManager()->unholdAllTerminalConnections(callId) ;

           debug("before createPlayer");
          mCfg->getCallManager()->createPlayer(MpPlayer::STREAM_PLAYER, callId, szUrl,
                            STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW  , &pPlayer) ;
           debug("after createPlayer");

          pPlayer->setLoopCount(-1);
          // debug("before realize");
          pPlayer->realize() ;
          // debug("before prefetch");
          pPlayer->prefetch() ;


          // debug("before Playing" );
          pPlayer->play(false) ;
          osPrintf("before sleeping for 20 seconds");
          OsTask::delay(1000*20) ;
          // debug("before stop");
          pPlayer->stop() ;
          pPlayer->rewind() ;

          mCfg->getCallManager()->destroyPlayer(MpPlayer::STREAM_PLAYER, callId, pPlayer) ;
          mCfg->getCallManager()->drop(callId) ;
       }
    }



    void testBasicPlayerWav()
    {
       debug("testBasicPlayerWav\n") ;

       char szUrl[128] ;
       MpStreamPlayer* pPlayer =  NULL ;

       for (int j=0; j<3; j++)
       {
          sprintf(szUrl, "http://%s/nums/1.wav", BASE_URL) ;

          Url url(szUrl) ;

          // debug("NEW TEST ATTEMPT ******************************" );
          UtlString callId ;

          mCfg->getCallManager()->createCall(&callId) ;
          mCfg->getCallManager()->unholdAllTerminalConnections(callId) ;

          debug("before createPlayer");
          mCfg->getCallManager()->createPlayer(MpPlayer::STREAM_PLAYER, callId, szUrl,
                            STREAM_SOUND_LOCAL | STREAM_FORMAT_WAV  , &pPlayer) ;
          debug("after createPlayer");
          pPlayer->setLoopCount(-1);
          // debug("before realize");
          pPlayer->realize() ;
          // debug("before prefetch");
          pPlayer->prefetch() ;


          // debug("before Playing" );
          pPlayer->play(false) ;
          osPrintf("before sleeping for 20 seconds");
          OsTask::delay(1000*20) ;
            debug("before stop");
          pPlayer->stop() ;
            debug("after stop");

          pPlayer->rewind() ;
           debug("before destroy player");
          mCfg->getCallManager()->destroyPlayer(MpPlayer::STREAM_PLAYER, callId, pPlayer) ;
           debug("after destroy player");
          mCfg->getCallManager()->drop(callId) ;
       }
    }



    void testQueuePlayerSuperDrop()
    {
       char szUrl[128] ;
       MpStreamQueuePlayer* pQPlayer ;
       MpStreamPlayer* pPlayer =  NULL ;
       int j ;

       osPrintf("testQueuePlayerSuperDrop.1\n") ;

       for (j=0; j<TESTING_ATTEMPTS; j++)
       {
          UtlString callId ;

          mCfg->getCallManager()->createCall(&callId) ;
          mCfg->getCallManager()->unholdAllTerminalConnections(callId) ;
          mCfg->getCallManager()->createPlayer(MpPlayer::STREAM_QUEUE_PLAYER, callId, NULL, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW, &pPlayer) ;
          pQPlayer = (MpStreamQueuePlayer*) pPlayer ;
          sprintf(szUrl, "http://%s/nums/%d.raw", BASE_URL, 1) ;
          Url url(szUrl) ;
          pQPlayer->add(url, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW) ;
          pQPlayer->play() ;
          pQPlayer->add(url, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW) ;

          mCfg->getCallManager()->drop(callId) ;
          mCfg->getCallManager()->destroyPlayer(MpPlayer::STREAM_QUEUE_PLAYER, callId, pPlayer) ;

          OsTask::delay(DELAY_BEWTEEN_CALLS) ;   // Delay so that we don't run out of flowgraphs
       }

       osPrintf("testQueuePlayerSuperDrop.2\n") ;

       for (j=0; j<TESTING_ATTEMPTS; j++)
       {
          UtlString callId ;

          mCfg->getCallManager()->createCall(&callId) ;
          mCfg->getCallManager()->unholdAllTerminalConnections(callId) ;
          mCfg->getCallManager()->createPlayer(MpPlayer::STREAM_QUEUE_PLAYER, callId, NULL, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW, &pPlayer) ;
          pQPlayer = (MpStreamQueuePlayer*) pPlayer ;
          sprintf(szUrl, "http://%s/nums/%d.raw", BASE_URL, 1) ;
          Url url(szUrl) ;
          pQPlayer->add(url, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW) ;
          pQPlayer->play() ;
          pQPlayer->add(url, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW) ;

          mCfg->getCallManager()->destroyPlayer(MpPlayer::STREAM_QUEUE_PLAYER, callId, pPlayer) ;
          mCfg->getCallManager()->drop(callId) ;

          OsTask::delay(DELAY_BEWTEEN_CALLS) ;      // Delay so that we don't run out of flowgraphs
       }

       osPrintf("testQueuePlayerSuperDrop.3\n") ;

       for (j=0; j<TESTING_ATTEMPTS; j++)
       {
          UtlString callId ;

          mCfg->getCallManager()->createCall(&callId) ;
          mCfg->getCallManager()->unholdAllTerminalConnections(callId) ;

          mCfg->getCallManager()->createPlayer(MpPlayer::STREAM_QUEUE_PLAYER, callId, NULL, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW, &pPlayer) ;
          pQPlayer = (MpStreamQueuePlayer*) pPlayer ;
          sprintf(szUrl, "http://%s/nums/%d.raw", BASE_URL, 1) ;
          Url url(szUrl) ;
          pQPlayer->add(url, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW) ;
          pQPlayer->play() ;
          pQPlayer->add(url, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW) ;

          mCfg->getCallManager()->drop(callId) ;
          pQPlayer->add(url, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW) ;
          pQPlayer->play() ;
          pQPlayer->add(url, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW) ;
          pQPlayer->play() ;

          mCfg->getCallManager()->drop(callId) ;
          mCfg->getCallManager()->destroyPlayer(MpPlayer::STREAM_QUEUE_PLAYER, callId, pPlayer) ;

          OsTask::delay(DELAY_BEWTEEN_CALLS) ;      // Delay so that we don't run out of flowgraphs
       }

       osPrintf("testQueuePlayerSuperDrop.4\n") ;

       for (j=0; j<TESTING_ATTEMPTS; j++)
       {
          UtlString callId ;

          mCfg->getCallManager()->createCall(&callId) ;
          mCfg->getCallManager()->unholdAllTerminalConnections(callId) ;

          mCfg->getCallManager()->createPlayer(MpPlayer::STREAM_QUEUE_PLAYER, callId, NULL, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW, &pPlayer) ;
          pQPlayer = (MpStreamQueuePlayer*) pPlayer ;
          sprintf(szUrl, "http://%s/nums/%d.raw", BASE_URL, 1) ;
          Url url(szUrl) ;
          pQPlayer->add(url, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW) ;
          pQPlayer->play() ;
          pQPlayer->add(url, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW) ;

          mCfg->getCallManager()->drop(callId) ;
          pQPlayer->add(url, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW) ;
          pQPlayer->play() ;
          pQPlayer->add(url, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW) ;
          pQPlayer->play() ;

          mCfg->getCallManager()->destroyPlayer(MpPlayer::STREAM_QUEUE_PLAYER, callId, pPlayer) ;
          mCfg->getCallManager()->drop(callId) ;

          OsTask::delay(DELAY_BEWTEEN_CALLS) ;      // Delay so that we don't run out of flowgraphs
       }
    }


    void testSuperDrop()
    {
       char szUrl[128] ;
       MpStreamPlayer* pPlayer =  NULL ;
       int j ;

       sprintf(szUrl, "http://%s/nums/%d.raw", BASE_URL, 1) ;

       for (j=0; j<TESTING_ATTEMPTS; j++)
       {
          UtlString callId ;

          mCfg->getCallManager()->createCall(&callId) ;
          mCfg->getCallManager()->unholdAllTerminalConnections(callId) ;
          mCfg->getCallManager()->createPlayer(MpPlayer::STREAM_PLAYER, callId, szUrl, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW, &pPlayer) ;


          pPlayer->realize() ;
          pPlayer->prefetch() ;
          pPlayer->play() ;

          mCfg->getCallManager()->destroyPlayer(MpPlayer::STREAM_PLAYER, callId, pPlayer) ;
          mCfg->getCallManager()->drop(callId) ;

          OsTask::delay(DELAY_BEWTEEN_CALLS) ;   // Delay so that we don't run out of flowgraphs
       }

       for (j=0; j<TESTING_ATTEMPTS; j++)
       {
          UtlString callId ;

          mCfg->getCallManager()->createCall(&callId) ;
          mCfg->getCallManager()->unholdAllTerminalConnections(callId) ;
          mCfg->getCallManager()->createPlayer(MpPlayer::STREAM_PLAYER, callId, szUrl, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW, &pPlayer) ;

          pPlayer->realize(FALSE) ;

          mCfg->getCallManager()->destroyPlayer(MpPlayer::STREAM_PLAYER, callId, pPlayer) ;
          mCfg->getCallManager()->drop(callId) ;

          OsTask::delay(DELAY_BEWTEEN_CALLS) ;   // Delay so that we don't run out of flowgraphs
       }

       for (j=0; j<TESTING_ATTEMPTS; j++)
       {
          UtlString callId ;

          mCfg->getCallManager()->createCall(&callId) ;
          mCfg->getCallManager()->unholdAllTerminalConnections(callId) ;
          mCfg->getCallManager()->createPlayer(MpPlayer::STREAM_PLAYER, callId, szUrl, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW, &pPlayer) ;

          pPlayer->realize() ;
          pPlayer->prefetch(FALSE) ;

          mCfg->getCallManager()->destroyPlayer(MpPlayer::STREAM_PLAYER, callId, pPlayer) ;
          mCfg->getCallManager()->drop(callId) ;

          OsTask::delay(DELAY_BEWTEEN_CALLS) ;   // Delay so that we don't run out of flowgraphs
       }


       for (j=0; j<TESTING_ATTEMPTS; j++)
       {
          UtlString callId ;

          mCfg->getCallManager()->createCall(&callId) ;
          mCfg->getCallManager()->unholdAllTerminalConnections(callId) ;
          mCfg->getCallManager()->createPlayer(MpPlayer::STREAM_PLAYER, callId, szUrl, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW, &pPlayer) ;

          pPlayer->realize() ;
          pPlayer->prefetch() ;
          pPlayer->play(FALSE) ;

          mCfg->getCallManager()->destroyPlayer(MpPlayer::STREAM_PLAYER, callId, pPlayer) ;
          mCfg->getCallManager()->drop(callId) ;

          OsTask::delay(DELAY_BEWTEEN_CALLS) ;   // Delay so that we don't run out of flowgraphs
       }

       for (j=0; j<TESTING_ATTEMPTS; j++)
       {
          UtlString callId ;

          mCfg->getCallManager()->createCall(&callId) ;
          mCfg->getCallManager()->unholdAllTerminalConnections(callId) ;
          mCfg->getCallManager()->createPlayer(MpPlayer::STREAM_PLAYER, callId, szUrl, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW, &pPlayer) ;

          pPlayer->setLoopCount(-1) ;
          pPlayer->realize() ;
          pPlayer->prefetch() ;
          pPlayer->play(FALSE) ;

          mCfg->getCallManager()->destroyPlayer(MpPlayer::STREAM_PLAYER, callId, pPlayer) ;
          mCfg->getCallManager()->drop(callId) ;

          OsTask::delay(DELAY_BEWTEEN_CALLS) ;   // Delay so that we don't run out of flowgraphs
       }
    }


    void testSuperCreateDestory()
    {
       char szUrl[128] ;
       MpStreamPlayer* pPlayer =  NULL ;
       int j ;

       sprintf(szUrl, "http://%s/nums/%d.raw", BASE_URL, 1) ;

       for (j=0; j<TESTING_ATTEMPTS; j++)
       {
          UtlString callId ;

          mCfg->getCallManager()->createCall(&callId) ;
          mCfg->getCallManager()->unholdAllTerminalConnections(callId) ;

          for (int l=0; l<25; l++)
          {
             mCfg->getCallManager()->createPlayer(MpPlayer::STREAM_PLAYER, callId, szUrl, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW, &pPlayer) ;

             pPlayer->setLoopCount(-1) ;
             pPlayer->realize(FALSE) ;
             pPlayer->prefetch(FALSE) ;
             pPlayer->play(FALSE) ;

             OsTask::delay(abs(rand() % 5000)) ;

             mCfg->getCallManager()->destroyPlayer(MpPlayer::STREAM_PLAYER, callId, pPlayer) ;
          }

          mCfg->getCallManager()->drop(callId) ;

          OsTask::delay(DELAY_BEWTEEN_CALLS) ;   // Delay so that we don't run out of flowgraphs
       }
    }

    void testQueuePlayerCallGoesAwayFirst()
    {
       char szUrl[128] ;
       MpStreamQueuePlayer* pQPlayer ;
       MpStreamPlayer* pPlayer =  NULL ;

       osPrintf("testQueuePlayerCallGoesAwayFirst\n") ;

       for (int j=0; j<TESTING_ATTEMPTS; j++)
       {
          UtlString callId ;

          mCfg->getCallManager()->createCall(&callId) ;
          mCfg->getCallManager()->unholdAllTerminalConnections(callId) ;

          mCfg->getCallManager()->createPlayer(MpPlayer::STREAM_QUEUE_PLAYER, callId, NULL, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW, &pPlayer) ;
          pQPlayer = (MpStreamQueuePlayer*) pPlayer ;
          for (int i=3; i>0; i--)
          {
             sprintf(szUrl, "http://%s/nums/%d.raw", BASE_URL, i) ;
             Url url(szUrl) ;

             pQPlayer->add(url, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW) ;
          }
          pQPlayer->play() ;
          pQPlayer->wait() ;

          mCfg->getCallManager()->drop(callId) ;
          osPrintf("Delaying for 25 seconds...") ;
          OsTask::delay(1000*25) ;
          mCfg->getCallManager()->destroyPlayer(MpPlayer::STREAM_QUEUE_PLAYER, callId, pPlayer) ;
       }
    }


    void testQueuePlayerDropCallMiddle()
    {
       char szUrl[128] ;
       MpStreamQueuePlayer* pQPlayer ;
       MpStreamPlayer* pPlayer =  NULL ;

       osPrintf("testQueuePlayerDropCallMiddle\n") ;

       for (int j=0; j<TESTING_ATTEMPTS; j++)
       {
          pPlayer = NULL ;
          UtlString callId ;

          mCfg->getCallManager()->createCall(&callId) ;
          mCfg->getCallManager()->unholdAllTerminalConnections(callId) ;

          mCfg->getCallManager()->createPlayer(MpPlayer::STREAM_QUEUE_PLAYER, callId, NULL, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW, &pPlayer) ;
          pQPlayer = (MpStreamQueuePlayer*) pPlayer ;
          sprintf(szUrl, "http://%s/longstream.raw", BASE_URL) ;
          Url url(szUrl) ;

          pQPlayer->add(url, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW) ;
          pQPlayer->play() ;
          OsTask::delay(1500) ;
          mCfg->getCallManager()->drop(callId) ;
          pQPlayer->wait() ;
          mCfg->getCallManager()->destroyPlayer(MpPlayer::STREAM_QUEUE_PLAYER, callId, pPlayer) ;
          OsTask::delay(500) ;
       }
    }


    void testQueuePlayerManyDropCallRandom()
    {
       char szUrl[128] ;
       MpStreamQueuePlayer* pQPlayer ;
       MpStreamPlayer* pPlayer =  NULL ;
       int i ;

       osPrintf("testQueuePlayerManyDropCallRandom\n") ;

       for (int j=0; j<TESTING_ATTEMPTS; j++)
       {
          pPlayer = NULL ;
          UtlString callId ;

          mCfg->getCallManager()->createCall(&callId) ;
          mCfg->getCallManager()->unholdAllTerminalConnections(callId) ;

          mCfg->getCallManager()->createPlayer(MpPlayer::STREAM_QUEUE_PLAYER, callId, NULL, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW, &pPlayer) ;
          pQPlayer = (MpStreamQueuePlayer*) pPlayer ;
          for (i=10; i>0; i--)
          {
             sprintf(szUrl, "http://%s/nums/%d.raw", BASE_URL, i) ;
             Url url(szUrl) ;

             pQPlayer->add(url, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW) ;
          }


          pQPlayer->play() ;

          int delay = abs(rand() % 5000) ;
          OsTask::delay(delay) ;
          mCfg->getCallManager()->drop(callId) ;

          for (i=0; i<8; i++)
          {
             switch (abs(rand() % 8))
             {
                case 0:
                   pQPlayer->play() ;
                   break ;
                case 1:
                   pQPlayer->wait() ;
                   break ;
                case 2:
                   pQPlayer->reset() ;
                   break ;
                case 3:
                   pQPlayer->clear() ;
                   break ;
                default:
                   {
                      // It's not clear what this sprintf() should do, since
                      // none of these URLs seem to be valid.
                      // (See the top of include/test/mp/MpTestConfig.h.)
                      sprintf(szUrl, "http://%s/nums/%d.raw", BASE_URL, i) ;
                      Url url(szUrl) ;
                      pQPlayer->add(url, STREAM_SOUND_LOCAL | STREAM_FORMAT_RAW);
                   }
                   break ;
             }
          }

          mCfg->getCallManager()->destroyPlayer(MpPlayer::STREAM_QUEUE_PLAYER, callId, pPlayer) ;
          OsTask::delay(500) ;
       }
    }

private:

    MpTestConfig *mCfg;
};


CPPUNIT_TEST_SUITE_REGISTRATION(CallManagerPlayerTest);
