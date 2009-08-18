//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#include "mp/MpAudioUtils.h"
#include "rtcp/RtcpConfig.h"

#ifndef __pingtel_on_posix__
//#define DOING_ECHO_SUPPRESSION
#endif

// SYSTEM INCLUDES
#include <assert.h>

#ifdef WIN32 /* [ */
#include <io.h>
#include <fcntl.h>
#endif /* WIN32 ] */

#ifdef __pingtel_on_posix__
#include <unistd.h>
#include <fcntl.h>
#endif

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsWriteLock.h"
#include "os/OsEvent.h"
#include "net/SdpCodec.h"
#include "os/OsProtectEventMgr.h"
#include "os/OsProtectEvent.h"
#include "os/OsFS.h"
#include "mp/MpConnection.h"
#include "mp/MpCallFlowGraph.h"
#include "mp/MpMediaTask.h"
#include "mp/MpStreamMsg.h"
#include "mp/MprBridge.h"
#include "mp/MprFromStream.h"
#include "mp/MprFromFile.h"
#include "mp/MprFromMic.h"


//#ifdef DOING_ECHO_SUPPRESSION /* [ */
//#include "mp/MprEchoSuppress.h"
//#endif /* DOING_ECHO_SUPPRESSION ] */

#include "mp/MprMixer.h"
#include "mp/MprSplitter.h"
#include "mp/MprToSpkr.h"
#include "mp/MprToneGen.h"

#include "mp/MprDecode.h"
#include "mp/MprEncode.h"
#include "mp/MprToNet.h"
#include "mp/MprFromNet.h"
#include "mp/MprDejitter.h"
#include "mp/MprRecorder.h"
#include "mp/MpTypes.h"
#include "mp/MpAudioUtils.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define KHz8000
#undef  KHz32000
#ifdef KHz8000 /* [ */
const int MpCallFlowGraph::DEF_SAMPLES_PER_FRAME = 80;
const int MpCallFlowGraph::DEF_SAMPLES_PER_SEC   = 8000;
#endif /* KHz8000 ] */
#ifdef KHz32000 /* [ */
const int MpCallFlowGraph::DEF_SAMPLES_PER_FRAME = 320;
const int MpCallFlowGraph::DEF_SAMPLES_PER_SEC   = 32000;
#endif /* KHz32000 ] */

// STATIC VARIABLE INITIALIZATIONS
UtlBoolean MpCallFlowGraph::sbSendInBandDTMF = true ;
UtlBoolean MpCallFlowGraph::sbEnableAEC = false ;

#define INSERT_RECORDERS // splices recorders into flowgraph
#undef INSERT_RECORDERS

#ifdef INSERT_RECORDERS /* [ */
static int WantRecorders = 1;
int wantRecorders(int flag) {
   int save = WantRecorders;
   WantRecorders = !flag;
   return save;
}
int wR() {return wantRecorders(0);}
int nwR() {return wantRecorders(1);}
#endif /* INSERT_RECORDERS ] */

#ifndef O_BINARY
#define O_BINARY 0      // O_BINARY is needed for WIN32 not for VxWorks or Linux
#endif

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MpCallFlowGraph::MpCallFlowGraph(const char* locale,
                                 int samplesPerFrame, int samplesPerSec)
: MpFlowGraphBase(samplesPerFrame, samplesPerSec),
  mConnTableLock(OsBSem::Q_FIFO, OsBSem::FULL),
  mToneIsGlobal(FALSE),
#ifdef INCLUDE_RTCP /* [ */
  mulEventInterest(LOCAL_SSRC_COLLISION | REMOTE_SSRC_COLLISION),
#endif /* INCLUDE_RTCP ] */
  mPremiumSoundEnabled(TRUE)
{
   UtlBoolean    boolRes;
   MpMediaTask* pMediaTask;
   OsStatus     res;
   int          i;

   for (i=0; i<MAX_CONNECTIONS; i++) mpConnections[i] = NULL;
   for (i=0; i<MAX_RECORDERS; i++) mpRecorders[i] = NULL;

   // create the resources and add them to the flow graph
   mpBridge           = new MprBridge("Bridge",
                                 samplesPerFrame, samplesPerSec);
   mpFromFile         = new MprFromFile("FromFile",
                                 samplesPerFrame, samplesPerSec);
   mpFromStream       = new MprFromStream("FromStream",
                                 samplesPerFrame, samplesPerSec);
   mpFromMic          = new MprFromMic("FromMic",
                                 samplesPerFrame, samplesPerSec);
#ifdef DOING_ECHO_SUPPRESSION /* [ */
   mpEchoSuppress     = new MprEchoSuppress("EchoSuppress",
                                 samplesPerFrame, samplesPerSec);
#endif /* DOING_ECHO_SUPPRESSION ] */
   mpTFsMicMixer      = new MprMixer("TFsMicMixer", 2,
                                 samplesPerFrame, samplesPerSec);
   mpTFsBridgeMixer   = new MprMixer("TFsBridgeMixer", 2,
                                 samplesPerFrame, samplesPerSec);
   mpToneFileSplitter = new MprSplitter("ToneFileSplitter", 2,
                                 samplesPerFrame, samplesPerSec);
   mpToSpkr           = new MprToSpkr("ToSpkr",
                                 samplesPerFrame, samplesPerSec);
   mpToneGen          = new MprToneGen("ToneGen",
                                 samplesPerFrame, samplesPerSec,
                                 locale);

#ifdef DOING_ECHO_SUPPRESSION /* [ */
   mpEchoSuppress->setSpkrPal(mpToSpkr);
#endif /* DOING_ECHO_SUPPRESSION ] */

   res = addResource(*mpBridge);            assert(res == OS_SUCCESS);
   res = addResource(*mpFromStream);        assert(res == OS_SUCCESS);
   res = addResource(*mpFromFile);          assert(res == OS_SUCCESS);
   res = addResource(*mpFromMic);           assert(res == OS_SUCCESS);
#ifdef DOING_ECHO_SUPPRESSION /* [ */
   res = addResource(*mpEchoSuppress);      assert(res == OS_SUCCESS);
#endif /* DOING_ECHO_SUPPRESSION ] */
   res = addResource(*mpTFsMicMixer);       assert(res == OS_SUCCESS);
   res = addResource(*mpTFsBridgeMixer);    assert(res == OS_SUCCESS);
   res = addResource(*mpToneFileSplitter);  assert(res == OS_SUCCESS);
   res = addResource(*mpToSpkr);            assert(res == OS_SUCCESS);
   res = addResource(*mpToneGen);           assert(res == OS_SUCCESS);

   // create the connections between the resources
   //////////////////////////////////////////////////////////////////////////
#ifdef DOING_ECHO_SUPPRESSION /* [ */
   // connect FromMic -> EchoSuppress -> TFsMicMixer -> Bridge
   res = addLink(*mpFromMic, 0, *mpEchoSuppress, 0);
   assert(res == OS_SUCCESS);
#ifdef  FLOWGRAPH_DOES_RESAMPLING /* [ */
   res = addLink(*mpFromMic, 1, *mpEchoSuppress, 1);
   assert(res == OS_SUCCESS);
#endif /* FLOWGRAPH_DOES_RESAMPLING ] */

   res = addLink(*mpEchoSuppress, 0, *mpTFsMicMixer, 1);
   assert(res == OS_SUCCESS);
#else /* DOING_ECHO_SUPPRESSION ] [ */
   // connect FromMic -> TFsMicMixer -> Bridge
   res = addLink(*mpFromMic, 0, *mpTFsMicMixer, 1);
   assert(res == OS_SUCCESS);
#endif /* DOING_ECHO_SUPPRESSION ] */

   res = addLink(*mpTFsMicMixer, 0, *mpBridge, 0);
   assert(res == OS_SUCCESS);

   //////////////////////////////////////////////////////////////////////////
   // connect Bridge -> TFsBridgeMixer

   res = addLink(*mpBridge, 0, *mpTFsBridgeMixer, 1);
   assert(res == OS_SUCCESS);

   //////////////////////////////////////////////////////////////////////////
   // connect ToneGen -> FromStream -> FromFile -> Splitter -> TFsBridgeMixer -> ToSpkr
   //                                                       -> Mixer

   res = addLink(*mpToneGen, 0, *mpFromStream, 0);
   assert(res == OS_SUCCESS);

   res = addLink(*mpFromStream, 0, *mpFromFile, 0);
   assert(res == OS_SUCCESS);

   res = addLink(*mpFromFile, 0, *mpToneFileSplitter, 0);
   assert(res == OS_SUCCESS);

   res = addLink(*mpToneFileSplitter, 0, *mpTFsBridgeMixer, 0);
   assert(res == OS_SUCCESS);

   res = addLink(*mpToneFileSplitter, 1, *mpTFsMicMixer, 0);
   assert(res == OS_SUCCESS);

   res = addLink(*mpTFsBridgeMixer, 0, *mpToSpkr, 0);
   assert(res == OS_SUCCESS);

   //////////////////////////////////////////////////////////////////////////
   // enable the flow graph (and all of the resources within it)
   res = enable();
   assert(res == OS_SUCCESS);

   // disable the tone generator
   boolRes = mpToneGen->disable();      assert(boolRes);
   mToneGenDefocused = FALSE;

   // disable the from stream
   boolRes = mpFromStream->disable();   assert(boolRes);

   // disable the from file
   boolRes = mpFromFile->disable();     assert(boolRes);

   // disable the FromMic, EchoSup, and ToSpkr -- we cannot have focus yet...
   boolRes = mpFromMic->disable();                assert(boolRes);
#ifdef DOING_ECHO_SUPPRESSION /* [ */
   boolRes = mpEchoSuppress->disable();           assert(boolRes);
#endif /* DOING_ECHO_SUPPRESSION ] */
   boolRes = mpToSpkr->disable();                 assert(boolRes);

   // The next group of settings turns the mixers into 2-to-1 multiplexors.
   // When disabled, mixers default to passing input 0 to output, and with
   // this setup, when enabled, they pass input 1 to output.
   boolRes = mpTFsMicMixer->setWeight(0, 0);      assert(boolRes);
   boolRes = mpTFsMicMixer->setWeight(1, 1);      assert(boolRes);

   boolRes = mpTFsBridgeMixer->setWeight(0, 0);   assert(boolRes);
   boolRes = mpTFsBridgeMixer->setWeight(1, 1);   assert(boolRes);

#ifdef INCLUDE_RTCP /* [ */
   // All the Media Resource seemed to have been started successfully.
   // Let's now create an RTCP Session so that we may be prepared to
   // report on the RTP connections that shall eventually be associated
   // with this flow graph

   // Let's get the  RTCP Control interface
   IRTCPControl *piRTCPControl = CRTCManager::getRTCPControl();
   assert(piRTCPControl);

   // Create an RTCP Session for this Flow Graph.  Pass the SSRC ID to be
   // used to identify our audio source uniquely within this RTP/RTCP Session.
   mpiRTCPSession = piRTCPControl->CreateSession(rand_timer32());

   // Subscribe for Events associated with this Session
   piRTCPControl->Advise((IRTCPNotify *)this);

   // Release Reference to RTCP Control Interface
   piRTCPControl->Release();
#endif /* INCLUDE_RTCP ] */

////////////////////////////////////////////////////////////////////////////
//
//  NOTE:  The following should be a runtime decision, not a compile time
//         decision... watch for it in an upcoming version... soon, I hope.
//  But, that needs to be coordinated with changes in ToSpkr and FromMic,
//  and some recorders should be skipped on Win/32.
//
//  A couple more bits of unfinished business:  The destructor should
//  clean up recorders and open record files, if any.
//
////////////////////////////////////////////////////////////////////////////
#ifdef INSERT_RECORDERS /* [ */
   ////////////////////////////////////////////////////////////////////
 if (WantRecorders) {
   mpRecorders[RECORDER_MIC] = new MprRecorder("RecordMic",
                                 samplesPerFrame, samplesPerSec);
   res = insertResourceAfter(*(mpRecorders[RECORDER_MIC]), *mpFromMic, 0);
   assert(res == OS_SUCCESS);
   mpRecorders[RECORDER_MIC32K] = new MprRecorder("RecordMicH",
                                 samplesPerFrame, samplesPerSec);
   res = insertResourceAfter(*(mpRecorders[RECORDER_MIC32K]), *mpFromMic, 1);
   assert(res == OS_SUCCESS);
#ifdef DOING_ECHO_SUPPRESSION /* [ */
   mpRecorders[RECORDER_ECHO_OUT] =
      new MprRecorder("RecordEchoOut", samplesPerFrame, samplesPerSec);
   res = insertResourceAfter(*(mpRecorders[RECORDER_ECHO_OUT]),
                                                    *mpEchoSuppress, 0);
   assert(res == OS_SUCCESS);

   mpRecorders[RECORDER_ECHO_IN8] =
      new MprRecorder("RecordEchoIn8", samplesPerFrame, samplesPerSec);
   res = insertResourceAfter(*(mpRecorders[RECORDER_ECHO_IN8]),
                                                    *mpEchoSuppress, 1);
   assert(res == OS_SUCCESS);

   mpRecorders[RECORDER_ECHO_IN32] =
      new MprRecorder("RecordEchoIn32", samplesPerFrame, samplesPerSec);
   res = insertResourceAfter(*(mpRecorders[RECORDER_ECHO_IN32]),
                                                    *mpEchoSuppress, 2);
   assert(res == OS_SUCCESS);
#endif /* DOING_ECHO_SUPPRESSION ] */
   mpRecorders[RECORDER_SPKR32K] = new MprRecorder("RecordSpkrH",
                                 samplesPerFrame, samplesPerSec);
   res = insertResourceAfter(*(mpRecorders[RECORDER_SPKR32K]), *mpToSpkr, 1);
   assert(res == OS_SUCCESS);
   ////////////////////////////////////////////////////////////////////
 }
#endif /* INSERT_RECORDERS ] */

///////////////////////////////////////////////////////////////////////////////////
//  Moved the following recorder out of the ifdef, to make it permanently available,
//  for media server use.
//
   mpRecorders[RECORDER_SPKR] = new MprRecorder("RecordSpkr",
                                 samplesPerFrame, samplesPerSec);
   res = insertResourceBefore(*(mpRecorders[RECORDER_SPKR]), *mpTFsBridgeMixer, 1);
   assert(res == OS_SUCCESS);
///////////////////////////////////////////////////////////////////////////////////


   // ask the media processing task to manage the new flow graph
   pMediaTask = MpMediaTask::getMediaTask(0);
   res = pMediaTask->manageFlowGraph(*this);
   assert(res == OS_SUCCESS);

   // start the flow graph
   res = pMediaTask->startFlowGraph(*this);
   assert(res == OS_SUCCESS);

   Zprintf("mpBridge=0x%p, " "mpConnection=0x%p, " "mpFromFile=0x%p\n"
       "mpFromMic=0x%p, " "mpTFsMicMixer=0x%p" "mpTFsBridgeMixer=0x%p\n",
      mpBridge, mpConnections[0], mpFromFile,
      mpFromMic, mpTFsMicMixer, mpTFsBridgeMixer);

#ifdef DOING_ECHO_SUPPRESSION /* [ */
   Zprintf("mpTFsMicMixer=0x%p, " "mpTFsBridgeMixer=0x%p\n"
      "mpToneFileSplitter=0x%p, " "mpToSpkr=0x%p, " "mpToneGen=0x%p\n"
      "mpEchoSuppress=0x%p\n",
      mpTFsMicMixer, mpTFsBridgeMixer, mpToneFileSplitter,
      mpToSpkr, mpToneGen, mpEchoSuppress);
#else /* DOING_ECHO_SUPPRESSION ] [ */
   Zprintf("mpTFsMicMixer=0x%p, " "mpTFsBridgeMixer=0x%p\n"
      "mpToneFileSplitter=0x%p, " "mpToSpkr=0x%p, " "mpToneGen=0x%p\n",
      mpTFsMicMixer, mpTFsBridgeMixer, mpToneFileSplitter,
      mpToSpkr, mpToneGen, 0);
#endif /* DOING_ECHO_SUPPRESSION ] */
}

// Destructor
MpCallFlowGraph::~MpCallFlowGraph()
{
   MpMediaTask* pMediaTask;
   OsStatus     res;
   int          i;

#ifdef INCLUDE_RTCP /* [ */
   // Let's terminate the RTCP Session in preparation for call teardown

   // Let's get the  RTCP Control interface
   IRTCPControl *piRTCPControl = CRTCManager::getRTCPControl();
   assert(piRTCPControl);

   // Unsubscribe for Events associated with this Session
   piRTCPControl->Unadvise((IRTCPNotify *)this);

   // Terminate the RTCP Session
   piRTCPControl->TerminateSession(mpiRTCPSession);

   // Release Reference to RTCP Control Interface
   piRTCPControl->Release();
#endif /* INCLUDE_RTCP ] */

   // unmanage the flow graph
   pMediaTask = MpMediaTask::getMediaTask(0);
   res = pMediaTask->unmanageFlowGraph(*this);
   assert(res == OS_SUCCESS);

   // wait until the flow graph is unmanaged.
   while (pMediaTask->isManagedFlowGraph(this))
      OsTask::delay(20);   // wait 20 msecs before checking again

   // $$$ I believe that we should just be able to delete the flow graph
   // $$$ at this point, but for now let's get rid of all the connections
   // $$$ and resources first.

   // remove the links between the resources
   res = removeLink(*mpBridge, 0);           assert(res == OS_SUCCESS);
   res = removeLink(*mpFromMic, 0);          assert(res == OS_SUCCESS);
#ifdef DOING_ECHO_SUPPRESSION /* [ */
   res = removeLink(*mpEchoSuppress, 0);     assert(res == OS_SUCCESS);
#endif /* DOING_ECHO_SUPPRESSION ] */
   res = removeLink(*mpTFsMicMixer, 0);      assert(res == OS_SUCCESS);
   res = removeLink(*mpTFsBridgeMixer, 0);   assert(res == OS_SUCCESS);
   res = removeLink(*mpToneGen, 0);          assert(res == OS_SUCCESS);
   res = removeLink(*mpFromStream, 0);       assert(res == OS_SUCCESS);
   res = removeLink(*mpFromFile, 0);         assert(res == OS_SUCCESS);
   res = removeLink(*mpToneFileSplitter, 0); assert(res == OS_SUCCESS);
   res = removeLink(*mpToneFileSplitter, 1); assert(res == OS_SUCCESS);

   // now remove (and destroy) the resources
   res = removeResource(*mpFromMic);
   assert(res == OS_SUCCESS);
   delete mpFromMic;
   mpFromMic = 0;

#ifdef DOING_ECHO_SUPPRESSION /* [ */
   res = removeResource(*mpEchoSuppress);
   assert(res == OS_SUCCESS);
   delete mpEchoSuppress;
#endif /* DOING_ECHO_SUPPRESSION ] */

   res = removeResource(*mpTFsMicMixer);
   assert(res == OS_SUCCESS);
   delete mpTFsMicMixer;

   res = removeResource(*mpTFsBridgeMixer);
   assert(res == OS_SUCCESS);
   delete mpTFsBridgeMixer;

   res = removeResource(*mpToneFileSplitter);
   assert(res == OS_SUCCESS);
   delete mpToneFileSplitter;

   res = removeResource(*mpToSpkr);
   assert(res == OS_SUCCESS);
   delete mpToSpkr;

   res = removeResource(*mpToneGen);
   assert(res == OS_SUCCESS);
   delete mpToneGen;

   res = removeResource(*mpFromStream);
   assert(res == OS_SUCCESS);
   delete mpFromStream;

   res = removeResource(*mpFromFile);
   assert(res == OS_SUCCESS);
   delete mpFromFile;

   for (i=0; i<MAX_RECORDERS; i++) {
      if (NULL != mpRecorders[i]) {
         res = removeResource(*mpRecorders[i]);
         assert(res == OS_SUCCESS);
         delete mpRecorders[i];
         mpRecorders[i] = NULL;
      }
   }

// For now, we just let the base class destructor mop up this mess...
#ifdef IS_THIS_TOO_MUCH /* [ */
// int          i;
// MpResource*  pBridgeSource;
// int          bridgePort;
   for (i=1; i<MAX_CONNECTIONS; i++) {
      if (NULL != mpConnections[i]) {
         pBridgeSource = mpConnections[i]->getSourceResource();
         printf("removing link from bridge input (connID=%d)\n", i);
         pBridgeSource->resourceInfo(pBridgeSource, i);
         res = removeLink(*pBridgeSource, 0);
         assert(OS_SUCCESS == res);
         bridgePort = mpConnections[i]->getBridgePort();
         printf("removing link from Bridge:%d\n", bridgePort);
         res = removeLink(*mpBridge, bridgePort);
         assert(OS_SUCCESS == res);
         delete mpConnections[i];
         mpConnections[i] = NULL;
      }
   }

#endif /* IS_THIS_TOO_MUCH ] */
   res = removeResource(*mpBridge);
   assert(res == OS_SUCCESS);
   delete mpBridge;
}

/* ============================ MANIPULATORS ============================== */

// Notification that this flow graph has just been granted the focus.
// Enable our microphone and speaker resources
OsStatus MpCallFlowGraph::gainFocus(void)
{
   UtlBoolean    boolRes;
#ifdef DOING_ECHO_SUPPRESSION /* [ */
   // enable the FromMic, EchoSup, and ToSpkr -- we have focus
   boolRes = mpFromMic->enable();       assert(boolRes);
   if (sbEnableAEC)
   {
      boolRes = mpEchoSuppress->enable();  assert(boolRes);
   }
   boolRes = mpToSpkr->enable();        assert(boolRes);
#else /* DOING_ECHO_SUPPRESSION ] [ */
   // enable the FromMic and ToSpkr -- we have focus
   boolRes = mpFromMic->enable();       assert(boolRes);
   boolRes = mpToSpkr->enable();        assert(boolRes);
#endif /* DOING_ECHO_SUPPRESSION ] */

   // Re-enable the tone as it is now being heard
   if(mToneGenDefocused)
   {
      mpToneGen->enable();
      mToneGenDefocused = FALSE;
   }

#ifdef DOING_ECHO_SUPPRESSION /* [ */
   if (!mpTFsMicMixer->isEnabled())  {
      boolRes = mpEchoSuppress->disable();  assert(boolRes);
   }
#endif /* DOING_ECHO_SUPPRESSION ] */

   Nprintf("MpBFG::gainFocus(0x%p)\n", this, 0,0,0,0,0);
   return OS_SUCCESS;
}

// Notification that this flow graph has just lost the focus.
// Disable our microphone and speaker resources
OsStatus MpCallFlowGraph::loseFocus(void)
{
   UtlBoolean    boolRes;

#ifdef DOING_ECHO_SUPPRESSION /* [ */
   // disable the FromMic, EchoSuppress and ToSpkr --
   //                                    we no longer have the focus.
#else /* DOING_ECHO_SUPPRESSION ] [ */
   // disable the FromMic and ToSpkr -- we no longer have the focus.
#endif /* DOING_ECHO_SUPPRESSION ] */
   boolRes = mpFromMic->disable();       assert(boolRes);
#ifdef DOING_ECHO_SUPPRESSION /* [ */
   boolRes = mpEchoSuppress->disable();  assert(boolRes);
#endif /* DOING_ECHO_SUPPRESSION ] */
   boolRes = mpToSpkr->disable();        assert(boolRes);

   // If the tone gen is not needed while we are out of focus disable it
   // as it is using resources while it is not being heard.
   if(mpToneGen->isEnabled() &&
      mpTFsBridgeMixer->isEnabled()) // Local tone only
      // Should also disable when remote tone and no connections
      // || (!mp???Mixer->isEnabled() && noConnections))
   {
      // osPrintf("Defocusing tone generator\n");
      mpToneGen->disable();
      mToneGenDefocused = TRUE;
   }

   Nprintf("MpBFG::loseFocus(0x%p)\n", this, 0,0,0,0,0);
   return OS_SUCCESS;
}

// Start playing the indicated tone.
void MpCallFlowGraph::startTone(int toneId, int toneOptions)
{
   UtlBoolean boolRes;
   OsStatus  res;
   int i;
   MpFlowGraphMsg msg(MpFlowGraphMsg::FLOWGRAPH_START_TONE, NULL,
                   NULL, NULL, toneOptions, 0);

   res = mpToneGen->startTone(toneId);        assert(res == OS_SUCCESS);

   res = postMessage(msg);

   // Only play locally if requested
   if (toneOptions & TONE_TO_SPKR)
   {
      boolRes = mpTFsBridgeMixer->disable();   assert(boolRes);
   }

   if (toneOptions & TONE_TO_NET) { // "mToneIsGlobal"
      // Notify outbound leg of all connections that we are playing a tone
      for (i=0; i<MAX_CONNECTIONS; i++) {
         if (NULL != mpConnections[i]) mpConnections[i]->startTone(toneId);
      }
   }
   // mpToneGen->enable();
}

// Stop playing the tone (applies to all tone destinations).
void MpCallFlowGraph::stopTone(void)
{
   OsStatus  res;
   int i;
   MpFlowGraphMsg msg(MpFlowGraphMsg::FLOWGRAPH_STOP_TONE, NULL,
                   NULL, NULL, 0, 0);

   // mpToneGen->disable();

   res = mpToneGen->stopTone();         assert(res == OS_SUCCESS);
   res = postMessage(msg);

   // Shut off the tone generator input to the Mixer resources
   // boolRes = mpTFsBridgeMixer->enable();   assert(boolRes);

   if (mToneIsGlobal) {
      // boolRes = mpTFsMicMixer->enable();      assert(boolRes);
      // Notify outbound leg of all connections that we are playing a tone
      for (i=0; i<MAX_CONNECTIONS; i++) {
         if (NULL != mpConnections[i]) mpConnections[i]->stopTone();
      }
   }
}

int MpCallFlowGraph::closeRecorders(void)
{
   int ret = 0;
   int i;

   if (NULL == this) {
      MpMediaTask* pMT = MpMediaTask::getMediaTask(0);
      MpCallFlowGraph* pIF = (MpCallFlowGraph*) pMT->getFocus();
      if (NULL != pIF) return pIF->closeRecorders();
      return 0;
   }
   for (i=0; i<MAX_RECORDERS; i++) {
      if (mpRecorders[i]) {
         mpRecorders[i]->closeRecorder();
         ret++;
      }
   }
   return ret;
}




////////////////////////////////////////////////////////////////////////////
//
// A simple method for starting the recorders on the hard phone
// INSERT_RECORDERS must be defined for this to work
//
// bitmask of recorderBitmask is as follows:
//  0000 0000 0000 0000 0000 0000 0000 0000
//  ^^^^ ^^^^ ^^^^ ^^^^ ^^^^^^^^^ ^^^^ ^^^^
//                  |              ||| ||||
//                  |              ||| |||+------- mic
//                  |              ||| ||+-------- echo out
//                  |              ||| |+--------- speaker
//                  |              ||| +---------- mic 32k sampling
//                  |              ||+------------ speaker 32k sampling
//                  |              |+------------- echo in 8K
//                  |              +-------------- echo in 32K
//                  |
//                  +----------------------------- not used
//
////////////////////////////////////////////////////////////////////////////

#define MAXUNIXPATH 64

OsStatus MpCallFlowGraph::Record(int ms,
      const char* playFilename, //if NULL, defaults to previous string
      const char* baseName,     //if NULL, defaults to previous string
      const char* endName,      //if NULL, defaults to previous string
      int recorderMask)
{
   static int  playIndex = 0;
   static int  saved_ms = 0;
   static char saved_playFilename[MAXUNIXPATH] = "";
   static char saved_baseName[MAXUNIXPATH] = "";
   static char saved_endName[MAXUNIXPATH] = "";
   OsStatus    res;

   if (NULL == this) {
      MpMediaTask* pMT = MpMediaTask::getMediaTask(0);
      MpCallFlowGraph* pIF = (MpCallFlowGraph*) pMT->getFocus();
      if (NULL != pIF) {
         return pIF->Record(ms, playFilename, baseName, endName, recorderMask);
      }
      return OS_INVALID;
   }

   if (ms == 0)
      ms = saved_ms;

   if (playFilename == NULL)
      playFilename = saved_playFilename;

   if (baseName == NULL)
      baseName = saved_baseName;

   if (endName == NULL)
      endName = saved_endName;


   char *created_micNamePtr      = new char[MAXUNIXPATH];
   char *created_echoOutNamePtr  = new char[MAXUNIXPATH];
   char *created_spkrNamePtr     = new char[MAXUNIXPATH];
   char *created_mic32NamePtr    = new char[MAXUNIXPATH];
   char *created_spkr32NamePtr   = new char[MAXUNIXPATH];
   char *created_echoIn8NamePtr  = new char[MAXUNIXPATH];
   char *created_echoIn32NamePtr = new char[MAXUNIXPATH];

   if (recorderMask & 1)
      sprintf(created_micNamePtr,
                        "%sm%d_%s_8k.raw", baseName, playIndex, endName);
   else {
      delete [] created_micNamePtr;
      created_micNamePtr = NULL;
   }

   if (recorderMask & 2)
      sprintf(created_echoOutNamePtr,
                        "%so%d_%s_8k.raw", baseName, playIndex, endName);
   else {
      delete [] created_echoOutNamePtr;
      created_echoOutNamePtr = NULL;
   }

   if (recorderMask & 4)
      sprintf(created_spkrNamePtr,
                        "%ss%d_%s_8k.raw", baseName, playIndex,  endName);
   else {
      delete [] created_spkrNamePtr;
      created_spkrNamePtr = NULL;
   }

   if (recorderMask & 8)
      sprintf(created_mic32NamePtr,
                        "%sm%d_%s_32k.raw", baseName, playIndex, endName);
   else {
      delete [] created_mic32NamePtr;
      created_mic32NamePtr = NULL;
   }

   if (recorderMask & 16)
      sprintf(created_spkr32NamePtr,
                        "%ss%d_%s_32k.raw", baseName, playIndex, endName);
   else {
      delete [] created_spkr32NamePtr;
      created_spkr32NamePtr = NULL;
   }

   if (recorderMask & 32)
      sprintf(created_echoIn8NamePtr,
                        "%se%d_%s_8k.raw", baseName, playIndex, endName);
   else {
      delete [] created_echoIn8NamePtr;
      created_echoIn8NamePtr = NULL;
   }

   if (recorderMask & 64)
      sprintf(created_echoIn32NamePtr,
                        "%se%d_%s_32k.raw", baseName, playIndex, endName);
   else {
      delete [] created_echoIn32NamePtr;
      created_echoIn32NamePtr = NULL;
   }

   res = record(ms, 999999, created_micNamePtr, created_echoOutNamePtr,
              created_spkrNamePtr, created_mic32NamePtr, created_spkr32NamePtr,
              created_echoIn8NamePtr, created_echoIn32NamePtr,
              playFilename, 0, 0, NULL);
   playIndex++;

   strcpy(saved_playFilename,playFilename);
   strcpy(saved_baseName,baseName);
   strcpy(saved_endName,endName);

   return res;
}


OsStatus MpCallFlowGraph::mediaRecord(int ms,
                                   int silenceLength,
                                   const char* fileName,
                                   double& duration,
                                   int& dtmfTerm,
                                   MprRecorder::RecordFileFormat format,
                                   OsProtectedEvent* recordEvent)
{
  if (!recordEvent)   // behaves like ezRecord
    return ezRecord(ms,
                    silenceLength,
                    fileName,
                    duration,
                    dtmfTerm,
                    format);

  // nonblocking version
   if (dtmfTerm)
   {
     for (int i=0; i<MAX_CONNECTIONS; i++)
     {
         if (NULL != mpConnections[i])
         {
           mpConnections[i]->setDtmfTerm(mpRecorders[RECORDER_SPKR]);
         }
     }
   }

  return record(ms, silenceLength, NULL, NULL, fileName,
                 NULL, NULL, NULL, NULL, NULL, 0, 0, recordEvent, format);

}

OsStatus MpCallFlowGraph::ezRecord(int ms,
                                   int silenceLength,
                                   const char* fileName,
                                   double& duration,
                                   int& dtmfTerm,
                                   MprRecorder::RecordFileFormat format)
{
   OsStatus ret = OS_WAIT_TIMEOUT;
   MprRecorderStats rs;
   OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
   OsProtectedEvent* recordEvent = eventMgr->alloc();
   recordEvent->setUserData(&rs);

   int timeoutSecs = (ms/1000 + 1);
   OsTime maxEventTime(timeoutSecs, 0);


   record(ms, silenceLength, NULL, NULL, fileName,
                 NULL, NULL, NULL, NULL, NULL, 0, 0, recordEvent,format);

   if (dtmfTerm)
   {
     for (int i=0; i<MAX_CONNECTIONS; i++)
     {
         if (NULL != mpConnections[i])
         {
           mpConnections[i]->setDtmfTerm(mpRecorders[RECORDER_SPKR]);
         }
     }
   }

   // Wait until the call sets the number of connections
   while(recordEvent->wait(0, maxEventTime) == OS_SUCCESS)
   {
      void* info;
      recordEvent->getUserData(info);
      if (info)
      {
         rs = *((MprRecorderStats *)info);
         duration = rs.mDuration;
         dtmfTerm = rs.mDtmfTerm;
         if (rs.mFinalStatus != MprRecorder::RECORDING)
         {
           ret = OS_SUCCESS;
           break;
         }
         else
            recordEvent->reset();
      }

   }

  closeRecorders();
  // If the event has already been signalled, clean up
  if(OS_ALREADY_SIGNALED == recordEvent->signal(0))
  {
     eventMgr->release(recordEvent);
  }
  return ret;
}

OsStatus MpCallFlowGraph::record(int ms, int silenceLength, const char* micName,
   const char* echoOutName, const char* spkrName,
   const char* mic32Name, const char* spkr32Name,
   const char* echoIn8Name, const char* echoIn32Name,
   const char* playName, int toneOptions,
   int repeat, OsNotification* completion,
   MprRecorder::RecordFileFormat format)
{
   if (NULL == this) {
      MpMediaTask* pMT = MpMediaTask::getMediaTask(0);
      MpCallFlowGraph* pIF = (MpCallFlowGraph*) pMT->getFocus();
      if (NULL != pIF) {
         return pIF-> record(ms, silenceLength, micName, echoOutName, spkrName,
            mic32Name, spkr32Name, echoIn8Name, echoIn32Name,
            playName, toneOptions, repeat, completion);
      }
      return OS_INVALID;
   }

   if (NULL != micName) {
      setupRecorder(RECORDER_MIC, micName,
                    ms, silenceLength, completion, format);
   }
   if (NULL != echoOutName) {
      setupRecorder(RECORDER_ECHO_OUT, echoOutName,
                    ms, silenceLength, completion, format);
   }
   if (NULL != spkrName) {
      setupRecorder(RECORDER_SPKR, spkrName,
                    ms, silenceLength, completion, format);
   }
   if (NULL != mic32Name) {
      setupRecorder(RECORDER_MIC32K, mic32Name,
                    ms, silenceLength, completion, format);
   }
   if (NULL != spkr32Name) {
      setupRecorder(RECORDER_SPKR32K,spkr32Name,
                    ms, silenceLength, completion, format);
   }
   if (NULL != echoIn8Name) {
      setupRecorder(RECORDER_ECHO_IN8, echoIn8Name,
                    ms, silenceLength, completion, format);
   }
   if (NULL != echoIn32Name) {
      setupRecorder(RECORDER_ECHO_IN32,
                    echoIn32Name, ms, silenceLength, completion, format);
   }
   return startRecording(playName, repeat, toneOptions, completion);
}

OsStatus MpCallFlowGraph::startRecording(const char* audioFileName,
                  UtlBoolean repeat, int toneOptions, OsNotification* event)


{
   OsStatus  res = OS_SUCCESS;
   MpFlowGraphMsg msg(MpFlowGraphMsg::FLOWGRAPH_START_RECORD, NULL,
                   NULL, NULL, toneOptions, START_PLAY_NONE);

   if (NULL != audioFileName) {
      res = mpFromFile->playFile(audioFileName, repeat, event);
      if (res == OS_SUCCESS) {
         msg.setInt1(toneOptions);
         msg.setInt2(START_PLAY_FILE);
      }
   }

   res = postMessage(msg);
   return(res);
}


// Setup recording on one recorder
UtlBoolean MpCallFlowGraph::setupRecorder(RecorderChoice which,
                  const char* audioFileName, int timeMS,
                  int silenceLength, OsNotification* event,
                  MprRecorder::RecordFileFormat format)

{
   int file = -1;
   OsStatus  res = OS_INVALID_ARGUMENT;

   if (NULL == mpRecorders[which]) {
      return res;
   }

   if (NULL != audioFileName) {
        file = open(audioFileName, O_BINARY | O_CREAT | O_RDWR, 0600);
   }

   if (-1 < file) {
      if (format == MprRecorder::WAV_PCM_16)
      {
          writeWAVHeader(file);
      }

      res = mpRecorders[which]->setup(file, format, timeMS, silenceLength, (OsEvent*)event);
   }
   return (file != -1);
}

// Start playing the indicated audio file.
OsStatus MpCallFlowGraph::playFile(const char* audioFileName, UtlBoolean repeat,
                                int toneOptions, OsNotification* event)
{
   OsStatus  res;

   res = mpFromFile->playFile(audioFileName, repeat, event);

   if (res == OS_SUCCESS)
   {
      MpFlowGraphMsg msg(MpFlowGraphMsg::FLOWGRAPH_START_PLAY, NULL,
                      NULL, NULL, toneOptions, 0);
      res = postMessage(msg);
   }
   return(res);
}

// Start playing the indicated audio buffer.
OsStatus MpCallFlowGraph::playBuffer(char* audioBuf,
                                     unsigned long bufSize,
                                     int type,
                                     UtlBoolean repeat,
                                     int toneOptions,
                                     OsProtectedEvent* event)
{
   OsStatus  res;

   res = mpFromFile->playBuffer(audioBuf, bufSize, type, repeat, event);

   if (res == OS_SUCCESS)
   {
      MpFlowGraphMsg msg(MpFlowGraphMsg::FLOWGRAPH_START_PLAY, NULL,
                      NULL, NULL, toneOptions, 0);
      res = postMessage(msg);
   }
   return(res);
}

// Stop playing the audio file.
void MpCallFlowGraph::stopFile(UtlBoolean closeFile)
{
   OsStatus  res;
   MpFlowGraphMsg msg(MpFlowGraphMsg::FLOWGRAPH_STOP_PLAY, NULL,
                   NULL, NULL, 0, 0);

   // mpFromFile->disable();

   res = mpFromFile->stopFile();       assert(res == OS_SUCCESS);
   res = postMessage(msg);

   // Shut off the tone generator/play sound input to the Mixer resource
   // boolRes = mpTFsMicMixer->enable();      assert(boolRes);
   // boolRes = mpTFsBridgeMixer->enable();   assert(boolRes);
}

MpConnectionID MpCallFlowGraph::createConnection()
{
   int            i;
   MpConnectionID found = -1;
   int            bridgePort;
   MpResource*    pBridgeSink;
   MpResource*    pBridgeSource;
   MpConnection*  pConnection;

   mConnTableLock.acquire();
   for (i=1; i<MAX_CONNECTIONS; i++) {
      if (NULL == mpConnections[i]) {
         mpConnections[i] = (MpConnection*) -1;
         found = i;
         i = MAX_CONNECTIONS;
      }
   }

   if (found < 0) {
      mConnTableLock.release();
      return -1;
   }

   mpConnections[found] = new MpConnection(this, found,
                 getSamplesPerFrame(), getSamplesPerSec());

   pConnection = mpConnections[found];

   bridgePort = mpBridge->connectPort(found);

   if (bridgePort < 0) {
      delete pConnection;
      mpConnections[found] = NULL;
      mConnTableLock.release();
      return -1;
   }

   mConnTableLock.release();

   Zprintf("bridgePort = %d\n", bridgePort, 0,0,0,0,0);

   pConnection->setBridgePort(bridgePort);
   pBridgeSink = pConnection->getSinkResource();
   pBridgeSource = pConnection->getSourceResource();

   OsStatus stat = addLink(*mpBridge, bridgePort, *pBridgeSink, 0);
   assert(OS_SUCCESS == stat);
   stat = addLink(*pBridgeSource, 0, *mpBridge, bridgePort);
   assert(OS_SUCCESS == stat);

   return found;
}

OsStatus MpCallFlowGraph::deleteConnection(MpConnectionID connID)
{
   OsWriteLock    lock(mRWMutex);

   UtlBoolean      handled;
   OsStatus       ret;

//   osPrintf("deleteConnection(%d)\n", connID);
   assert((0 < connID) && (connID < MAX_CONNECTIONS));

   if ((NULL == mpConnections[connID]) ||
      (((MpConnection*) -1) == mpConnections[connID]))
         return OS_INVALID_ARGUMENT;

   MpFlowGraphMsg msg(MpFlowGraphMsg::FLOWGRAPH_REMOVE_CONNECTION, NULL,
                      NULL, NULL, connID);

   if (isStarted()) {
      // postPone(1000); // testing...
      ret = postMessage(msg);
      // if (OS_SUCCESS == ret) {
         // synchronize();
      // }
      return ret;
   }

   handled = handleMessage(msg);
   if (handled)
      return OS_SUCCESS;
   else
      return OS_UNSPECIFIED;
}

void MpCallFlowGraph::setPremiumSound(MpConnection::PremiumSoundOptions op)
{
   OsStatus  res;
   MpFlowGraphMsg msg(MpFlowGraphMsg::FLOWGRAPH_SET_PREMIUM_SOUND,
      NULL, NULL, NULL, op);

   res = postMessage(msg);
}

#define DEBUG_GIPS_PREMIUM_SOUND
#undef  DEBUG_GIPS_PREMIUM_SOUND

void MpCallFlowGraph::disablePremiumSound(void)
{
#ifdef _VXWORKS /* [ */
#ifdef DEBUG_GIPS_PREMIUM_SOUND /* [ */
   if (NULL == this) {
      MpMediaTask* pMT = MpMediaTask::getMediaTask(0);
      MpCallFlowGraph* pIF = (MpCallFlowGraph*) pMT->getFocus();
      if (NULL != pIF) pIF->disablePremiumSound();
      return;
   }
#endif /* DEBUG_GIPS_PREMIUM_SOUND ] */
#endif /* _VXWORKS ] */
   setPremiumSound(MpConnection::DisablePremiumSound);
}

void MpCallFlowGraph::enablePremiumSound(void)
{
#ifdef _VXWORKS /* [ */
#ifdef DEBUG_GIPS_PREMIUM_SOUND /* [ */
   if (NULL == this) {
      MpMediaTask* pMT = MpMediaTask::getMediaTask(0);
      MpCallFlowGraph* pIF = (MpCallFlowGraph*) pMT->getFocus();
      if (NULL != pIF) pIF->enablePremiumSound();
      return;
   }
#endif /* DEBUG_GIPS_PREMIUM_SOUND ] */
#endif /* _VXWORKS ] */
   setPremiumSound(MpConnection::EnablePremiumSound);
}

// Start sending RTP and RTCP packets.
void MpCallFlowGraph::startSendRtp(OsSocket& rRtpSocket,
                                    OsSocket& rRtcpSocket,
                                    MpConnectionID connID,
                                    SdpCodec* pPrimaryCodec,
                                    SdpCodec* pDtmfCodec,
                                    SdpCodec* pSecondaryCodec)
{
   mpConnections[connID]->startSendRtp(rRtpSocket, rRtcpSocket,
      pPrimaryCodec, pDtmfCodec, pSecondaryCodec);
}

// (old style call...)
void MpCallFlowGraph::startSendRtp(SdpCodec& rPrimaryCodec,
                                    OsSocket& rRtpSocket,
                                    OsSocket& rRtcpSocket,
                                    MpConnectionID connID)
{
   startSendRtp(rRtpSocket, rRtcpSocket, connID, &rPrimaryCodec, NULL, NULL);
}

// Stop sending RTP and RTCP packets.
void MpCallFlowGraph::stopSendRtp(MpConnectionID connID)
{
   // postPone(40); // testing...
   mpConnections[connID]->stopSendRtp();
}

// Start receiving RTP and RTCP packets.
#ifdef OLD_WAY /* [ */
void MpCallFlowGraph::startReceiveRtp(SdpCodec& rCodec,
                                       OsSocket& rRtpSocket,
                                       OsSocket& rRtcpSocket,
                                       MpConnectionID connID)
{
   SdpCodec* pCodecs[1];

   pCodecs[0] = &rCodec;
   startReceiveRtp(pCodecs, 1, rRtpSocket, rRtcpSocket, connID);
}
#endif /* OLD_WAY ] */

void MpCallFlowGraph::startReceiveRtp(SdpCodec* pCodecs[],
                                       int numCodecs,
                                       OsSocket& rRtpSocket,
                                       OsSocket& rRtcpSocket,
                                       MpConnectionID connID)
{
   mpConnections[connID]->
            startReceiveRtp(pCodecs, numCodecs, rRtpSocket, rRtcpSocket);
}

// Stop receiving RTP and RTCP packets.
void MpCallFlowGraph::stopReceiveRtp(MpConnectionID connID)
{
   mpConnections[connID]->stopReceiveRtp();
}

OsStatus MpCallFlowGraph::addToneListener(OsNotification* pNotify,
                                             MpConnectionID connectionId)
{
   MpFlowGraphMsg msg(MpFlowGraphMsg::FLOWGRAPH_SET_DTMF_NOTIFY,
      NULL, pNotify, NULL, connectionId);

   return postMessage(msg);
}

OsStatus MpCallFlowGraph::removeToneListener(MpConnectionID connectionId)
{
   MpFlowGraphMsg msg(MpFlowGraphMsg::FLOWGRAPH_SET_DTMF_NOTIFY,
      NULL, NULL, NULL, connectionId);

   return postMessage(msg);
}


// Enables/Disable the transmission of inband DTMF audio
UtlBoolean MpCallFlowGraph::setInbandDTMF(UtlBoolean bEnable)
{
   UtlBoolean bSave = sbSendInBandDTMF;
   sbSendInBandDTMF = bEnable;
   return bSave ;
}


// Enables/Disable Acoustic Echo Cancellation
UtlBoolean MpCallFlowGraph::setEnableAEC(UtlBoolean bEnable)
{
   UtlBoolean bSave = sbEnableAEC;
   sbEnableAEC = bEnable;
   return bSave ;
}

#ifdef INCLUDE_RTCP /* [ */

/* ======================== CALLBACK METHODS ============================= */

/**
 *
 * Method Name:  LocalSSRCCollision()
 *
 *
 * Inputs:      IRTCPConnection *piRTCPConnection - Interface to associated
 *                                                   RTCP Connection
 *              IRTCPSession    *piRTCPSession    - Interface to associated
 *                                                   RTCP Session
 *
 * Outputs:     None
 *
 * Returns:     None
 *
 * Description: The LocalSSRCCollision() event method shall inform the
 *              recipient of a collision between the local SSRC and one
 *              used by one of the remote participants.
 *              .
 *
 * Usage Notes:
 *
 */
void MpCallFlowGraph::LocalSSRCCollision(IRTCPConnection  *piRTCPConnection,
                                         IRTCPSession     *piRTCPSession)
{

//  Ignore those events that are for a session other than ours
    if(mpiRTCPSession != piRTCPSession)
    {
//      Release Interface References
        piRTCPConnection->Release();
        piRTCPSession->Release();
        return;
    }

// We have a collision with our local SSRC.  We will remedy this by
// generating a new SSRC
    mpiRTCPSession->ReassignSSRC(rand_timer32(),
                         (unsigned char *)"LOCAL SSRC COLLISION");

// We must inform all connections associated with this session to change their
// SSRC
    mConnTableLock.acquire();
    for (int iConnection = 1; iConnection < MAX_CONNECTIONS; iConnection++)
    {
      if (mpConnections[iConnection]->getRTCPConnection())
      {
//       Set the new SSRC
         mpConnections[iConnection]->
                          reassignSSRC((int)mpiRTCPSession->GetSSRC());
         break;
      }
   }
   mConnTableLock.release();

// Release Interface References
   piRTCPConnection->Release();
   piRTCPSession->Release();

   return;
}


/**
 *
 * Method Name:  RemoteSSRCCollision()
 *
 *
 * Inputs:      IRTCPConnection *piRTCPConnection - Interface to associated
 *                                                   RTCP Connection
 *              IRTCPSession    *piRTCPSession    - Interface to associated
 *                                                   RTCP Session
 *
 * Outputs:     None
 *
 * Returns:     None
 *
 * Description: The RemoteSSRCCollision() event method shall inform the
 *              recipient of a collision between two remote participants.
 *              .
 *
 * Usage Notes:
 *
 */
void MpCallFlowGraph::RemoteSSRCCollision(IRTCPConnection  *piRTCPConnection,
                                          IRTCPSession     *piRTCPSession)
{

//  Ignore those events that are for a session other than ours
    if(mpiRTCPSession != piRTCPSession)
    {
//      Release Interface References
        piRTCPConnection->Release();
        piRTCPSession->Release();
        return;
    }

// According to standards, we are supposed to ignore remote sites that
// have colliding SSRC IDS.
    mConnTableLock.acquire();
    for (int iConnection = 1; iConnection < MAX_CONNECTIONS; iConnection++)
    {
      if (mpConnections[iConnection]->getRTCPConnection() == piRTCPConnection)
      {
// We are supposed to ignore the media of the latter of two terminals
// whose SSRC collides
         mpConnections[iConnection]->stopReceiveRtp();
         break;
      }
   }
   mConnTableLock.release();

// Release Interface References
   piRTCPConnection->Release();
   piRTCPSession->Release();


}
#endif /* INCLUDE_RTCP ] */

/* ============================ ACCESSORS ================================= */

void MpCallFlowGraph::synchronize(const char* tag, int val1)
{
   OsTask* val2 = OsTask::getCurrentTask();
   if (val2 != MpMediaTask::getMediaTask(0)) {
      OsEvent event;
      MpFlowGraphMsg msg(MpFlowGraphMsg::FLOWGRAPH_SYNCHRONIZE,
         NULL, NULL, (void*) tag, val1, (intptr_t) val2);
      OsStatus  res;

      msg.setPtr1(&event);
      res = postMessage(msg);
      // if (NULL == tag) osPrintf("MpCallFlowGraph::synchronize()\n");
      event.wait();
   } else {
      osPrintf("Note: synchronize called from within Media Task\n");
   }
}

/* ============================ INQUIRY =================================== */

// Returns TRUE if the indicated codec is supported.
UtlBoolean MpCallFlowGraph::isCodecSupported(SdpCodec& rCodec)
{
   // $$$ For now always return TRUE
   return TRUE;
}

// Returns TRUE if the GIPS premium sound is enabled
UtlBoolean MpCallFlowGraph::isPremiumSoundEnabled(void)
{
   return mPremiumSoundEnabled;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
UtlBoolean MpCallFlowGraph::writeWAVHeader(int handle)
{
    UtlBoolean retCode = FALSE;
    char tmpbuf[80];
    short bitsPerSample = 16;

    short sampleSize = sizeof(Sample);
    uint16_t compressionCode = 1; //PCM = 2 byte value
    uint16_t numChannels = 1;  // 2 byte value for Endian conversion
    uint32_t samplesPerSecond = 8000; // 4 byte value for Endian conversion
    uint32_t averageSamplePerSec = samplesPerSecond*sampleSize; // 4 byte value
    uint16_t blockAlign = sampleSize*numChannels;  // 2 byte value
    size_t bytesWritten = 0;

    //write RIFF & length
    //8 bytes written
    strcpy(tmpbuf,MpWaveFileFormat);
    uint32_t length = 0;  // 4 byte value for Endian conversion
    bytesWritten += write(handle,tmpbuf, strlen(tmpbuf));
    bytesWritten += write(handle, (char*)&length, sizeof(length)); //filled in on close

    //write WAVE
    //4 bytes written
    strcpy(tmpbuf,"WAVE");
    bytesWritten += write(handle,tmpbuf, strlen(tmpbuf));

    //write fmt & length
    //8 bytes written
    strcpy(tmpbuf,"fmt ");
    length = 16;
    bytesWritten += write(handle,tmpbuf,strlen(tmpbuf));
    length = htolel(length);
    bytesWritten += write(handle, (char*)&length,sizeof(length)); //filled in on close

    //now write each piece of the format
    //16 bytes written
    compressionCode = htoles(compressionCode);
    bytesWritten += write(handle, (char*)&compressionCode, sizeof(compressionCode));
    numChannels = htoles(numChannels);
    bytesWritten += write(handle, (char*)&numChannels, sizeof(numChannels));
    samplesPerSecond = htolel(samplesPerSecond);
    bytesWritten += write(handle, (char*)&samplesPerSecond, sizeof(samplesPerSecond));
    averageSamplePerSec = htolel(averageSamplePerSec);
    bytesWritten += write(handle, (char*)&averageSamplePerSec, sizeof(averageSamplePerSec));
    blockAlign = htoles(blockAlign);
    bytesWritten += write(handle, (char*)&blockAlign, sizeof(blockAlign));
    bitsPerSample = htoles(bitsPerSample);
    bytesWritten += write(handle, (char*)&bitsPerSample, sizeof(bitsPerSample));


    //write data and length
    strcpy(tmpbuf,"data");
    length = 0;
    bytesWritten += write(handle,tmpbuf,strlen(tmpbuf));
    length = htolel(length);
    bytesWritten += write(handle, (char*)&length,sizeof(length)); //filled in on close

    //total length at this point should be 48 bytes
    if (bytesWritten == 44)
        retCode = TRUE;

    return retCode;

}


// Handles an incoming message for the flow graph.
// Returns TRUE if the message was handled, otherwise FALSE.
UtlBoolean MpCallFlowGraph::handleMessage(OsMsg& rMsg)
{
   UtlBoolean retCode;

   retCode = FALSE;

   if (rMsg.getMsgType() == OsMsg::STREAMING_MSG)
   {
      //
      // Handle Streaming Messages
      //
      MpStreamMsg* pMsg = (MpStreamMsg*) &rMsg ;
      switch (pMsg->getMsg())
      {
         case MpStreamMsg::STREAM_REALIZE_URL:
            retCode = handleStreamRealizeUrl(*pMsg) ;
            break;
         case MpStreamMsg::STREAM_REALIZE_BUFFER:
            retCode = handleStreamRealizeBuffer(*pMsg) ;
            break;
         case MpStreamMsg::STREAM_PREFETCH:
            retCode = handleStreamPrefetch(*pMsg) ;
            break;
         case MpStreamMsg::STREAM_PLAY:
            retCode = handleStreamPlay(*pMsg) ;
            break;
         case MpStreamMsg::STREAM_REWIND:
            retCode = handleStreamRewind(*pMsg) ;
            break;
         case MpStreamMsg::STREAM_PAUSE:
            retCode = handleStreamPause(*pMsg) ;
            break;
         case MpStreamMsg::STREAM_STOP:
            retCode = handleStreamStop(*pMsg) ;
            break;
         case MpStreamMsg::STREAM_DESTROY:
            retCode = handleStreamDestroy(*pMsg) ;
            break;
         default:
            break;
      }
   }
   else
   {
      MpFlowGraphMsg* pMsg = (MpFlowGraphMsg*) &rMsg ;
      //
      // Handle Normal Flow Graph Messages
      //
      switch (pMsg->getMsg())
      {
      case MpFlowGraphMsg::FLOWGRAPH_REMOVE_CONNECTION:
         retCode = handleRemoveConnection(*pMsg);
         break;
      case MpFlowGraphMsg::FLOWGRAPH_START_PLAY:
         retCode = handleStartPlay(*pMsg);
         break;
      case MpFlowGraphMsg::FLOWGRAPH_START_RECORD:
         retCode = handleStartRecord(*pMsg);
         break;
      case MpFlowGraphMsg::FLOWGRAPH_STOP_RECORD:
         // osPrintf("\n++++++ recording stopped\n");
         // retCode = handleStopRecord(rMsg);
         break;
      case MpFlowGraphMsg::FLOWGRAPH_START_TONE:
         retCode = handleStartTone(*pMsg);
         break;
      case MpFlowGraphMsg::FLOWGRAPH_STOP_PLAY:
      case MpFlowGraphMsg::FLOWGRAPH_STOP_TONE:
         retCode = handleStopToneOrPlay();
         break;
      case MpFlowGraphMsg::FLOWGRAPH_SYNCHRONIZE:
         retCode = handleSynchronize(*pMsg);
         break;
      case MpFlowGraphMsg::FLOWGRAPH_SET_PREMIUM_SOUND:
         retCode = handleSetPremiumSound(*pMsg);
         break;
      case MpFlowGraphMsg::FLOWGRAPH_SET_DTMF_NOTIFY:
         retCode = handleSetDtmfNotify(*pMsg);
         break;
      default:
         retCode = MpFlowGraphBase::handleMessage(*pMsg);
         break;
      }
   }

   return retCode;
}

// Handle the FLOWGRAPH_REMOVE_CONNECTION message.
// Returns TRUE if the message was handled, otherwise FALSE.
UtlBoolean MpCallFlowGraph::handleRemoveConnection(MpFlowGraphMsg& rMsg)
{
   MpConnectionID connID = rMsg.getInt1();
   MpConnection* pConnection;
   UtlBoolean    res;

   mpBridge->disconnectPort(connID);
   mConnTableLock.acquire();
   pConnection = mpConnections[connID];
   mpConnections[connID] = NULL;
   mConnTableLock.release();

   if ((NULL == pConnection) || (((MpConnection*) -1) == pConnection))
      return TRUE;

   // remove the links between the resources
   res = handleRemoveLink((MpResource*) pConnection->mpEncode, 0);
   assert(res);
   res = handleRemoveLink((MpResource*) pConnection->mpFromNet, 0);
   assert(res);
   res = handleRemoveLink((MpResource*) pConnection->mpDejitter, 0);
   assert(res);

   // now remove (and destroy) the resources
   res = handleRemoveResource((MpResource*) pConnection->mpDecode);
   assert(res);
   delete pConnection->mpDecode;

   res = handleRemoveResource((MpResource*) pConnection->mpDejitter);
   assert(res);
   delete pConnection->mpDejitter;

   res = handleRemoveResource((MpResource*) pConnection->mpEncode);
   assert(res);
   delete pConnection->mpEncode;

   res = handleRemoveResource((MpResource*) pConnection->mpFromNet);
   assert(res);
   delete pConnection->mpFromNet;

   res = handleRemoveResource((MpResource*) pConnection->mpToNet);
   assert(res);
   delete pConnection->mpToNet;

   delete pConnection;
   return TRUE;
}

UtlBoolean MpCallFlowGraph::handleStartPlay(MpFlowGraphMsg& rMsg)
{
   UtlBoolean boolRes;
   int toneOptions = rMsg.getInt1();

   boolRes = mpFromFile->enable();          assert(boolRes);

   // Play locally, always
   boolRes = mpTFsBridgeMixer->disable();   assert(boolRes);

   if (toneOptions & TONE_TO_NET)
   {
      // Play the file audio through the Mixer resource,
      // shutting off the other audio input
      boolRes = mpTFsMicMixer->disable();   assert(boolRes);
   }
#ifdef DOING_ECHO_SUPPRESSION /* [ */
   boolRes = mpEchoSuppress->disable();  assert(boolRes);
#endif /* DOING_ECHO_SUPPRESSION ] */
   return TRUE;
}

UtlBoolean MpCallFlowGraph::handleStartTone(MpFlowGraphMsg& rMsg)
{
   UtlBoolean boolRes;
   int toneOptions = rMsg.getInt1();

   // boolRes = mpToneGen->enable();          assert(boolRes);

   // Only play locally if requested
   if (toneOptions & TONE_TO_SPKR)
   {
      boolRes = mpTFsBridgeMixer->disable();   assert(boolRes);
   }

   mToneIsGlobal = (toneOptions & TONE_TO_NET);
   if (mToneIsGlobal)
   {
      // Play the file audio through the Mixer resource,
      // shutting off the other audio input
      boolRes = mpTFsMicMixer->disable();   assert(boolRes);

      // We may be asked NOT to send inband DTMF to the remote
      // party.  This is accomplished by setting the tone gen
      // weight to zero.  At which point, the mixer will send
      // silence.
      if (!sbSendInBandDTMF)
      {
         boolRes = mpTFsMicMixer->setWeight(0, 1);      assert(boolRes);
      }
   }
#ifdef DOING_ECHO_SUPPRESSION /* [ */
   boolRes = mpEchoSuppress->disable();  assert(boolRes);
#endif /* DOING_ECHO_SUPPRESSION ] */
   return TRUE;
}

UtlBoolean MpCallFlowGraph::handleStartRecord(MpFlowGraphMsg& rMsg)
{
   int i;
   int startPlayer = rMsg.getInt2();

   if (START_PLAY_FILE == startPlayer) handleStartPlay(rMsg);
   for (i=0; i<MAX_RECORDERS; i++) {
      if (NULL != mpRecorders[i]) {
         mpRecorders[i]->begin();
      }
   }
   return TRUE;
}

UtlBoolean MpCallFlowGraph::handleStopToneOrPlay()
{
   UtlBoolean boolRes;

#ifdef DOING_ECHO_SUPPRESSION /* [ */
   MpMediaTask* pMediaTask;

   pMediaTask = MpMediaTask::getMediaTask(0);
#endif /* DOING_ECHO_SUPPRESSION ] */

   // Shut off the tone generator input to the Mixer resources
   boolRes = mpTFsBridgeMixer->enable();     assert(boolRes);
   boolRes = mpTFsMicMixer->enable();        assert(boolRes);

   // The weight of the tone gen / from file resource may have
   // be changed to zero if we were requested NOT to send inband
   // DTMF.  This code resets that weight.
   if (!sbSendInBandDTMF)
   {
      boolRes = mpTFsMicMixer->setWeight(1, 1); assert(boolRes);
   }
#ifdef DOING_ECHO_SUPPRESSION /* [ */
   if (sbEnableAEC && (this == pMediaTask->getFocus()))
   {
      boolRes = mpEchoSuppress->enable();  assert(boolRes);
   }
#endif /* DOING_ECHO_SUPPRESSION ] */
   return TRUE;
}

#ifdef DEBUG_POSTPONE /* [ */
   void MpCallFlowGraph::postPone(int ms)
   {
      MpFlowGraphMsg msg(MpFlowGraphMsg::FLOWGRAPH_SYNCHRONIZE,
         NULL, NULL, NULL, ms, 0);
      OsStatus  res;

      res = postMessage(msg);
      // osPrintf("MpCallFlowGraph::postPone(%d)\n", ms);
   }
#endif /* DEBUG_POSTPONE ] */

UtlBoolean MpCallFlowGraph::handleSynchronize(MpFlowGraphMsg& rMsg)
{
   OsNotification* pSync = (OsNotification*) rMsg.getPtr1();

   intptr_t val1  = rMsg.getInt1();

   if (0 != pSync) {
      pSync->signal(val1);

#ifdef DEBUG_POSTPONE /* [ */
   } else {
      // just delay (postPone()), for debugging race conditions...
      OsTask::delay(rMsg.getInt1());
#endif /* DEBUG_POSTPONE ] */
   }
   return TRUE;
}

UtlBoolean MpCallFlowGraph::handleSetPremiumSound(MpFlowGraphMsg& rMsg)
{
   UtlBoolean save = mPremiumSoundEnabled;
   MpConnection::PremiumSoundOptions op =
                        (MpConnection::PremiumSoundOptions) rMsg.getInt1();
   int i;

   mPremiumSoundEnabled = (op == MpConnection::EnablePremiumSound);
   if (save != mPremiumSoundEnabled) {
/*
      osPrintf("MpCallFlowGraph(%p): Premium sound %sABLED\n",
         this, mPremiumSoundEnabled ? "EN" : "DIS");
*/

      for (i=0; i<MAX_CONNECTIONS; i++) {
         if (NULL != mpConnections[i]) {
            mpConnections[i]->setPremiumSound(op);
         }
      }
   }
   return TRUE;
}

UtlBoolean MpCallFlowGraph::handleSetDtmfNotify(MpFlowGraphMsg& rMsg)
{
   OsNotification* pNotify = (OsNotification*) rMsg.getPtr1();
   MpConnectionID  connId  = rMsg.getInt1();

   return mpConnections[connId]->handleSetDtmfNotify(pNotify);
}


UtlBoolean MpCallFlowGraph::handleStreamRealizeUrl(MpStreamMsg& rMsg)
{
   int flags = rMsg.getInt1() ;
   Url* pUrl = (Url*) rMsg.getInt2() ;
   OsNotification* pNotifyHandle = (OsNotification*) rMsg.getPtr1() ;
   OsNotification* pNotifyEvents = (OsNotification*) rMsg.getPtr2() ;

   StreamHandle handle = NULL ;

   mpFromStream->realize(*pUrl, flags, handle, pNotifyEvents) ;
   delete pUrl ;

   pNotifyHandle->signal((intptr_t)handle) ;

   return TRUE ;
}


UtlBoolean MpCallFlowGraph::handleStreamRealizeBuffer(MpStreamMsg& rMsg)
{
   int flags = rMsg.getInt1() ;
   UtlString* pBuffer = (UtlString*) rMsg.getInt2() ;
   OsNotification* pNotifyHandle = (OsNotification*) rMsg.getPtr1() ;
   OsNotification* pNotifyEvents = (OsNotification*) rMsg.getPtr2() ;

   StreamHandle handle = NULL ;

   mpFromStream->realize(pBuffer, flags, handle, pNotifyEvents) ;

   pNotifyHandle->signal((intptr_t)handle) ;

   return TRUE ;
}



UtlBoolean MpCallFlowGraph::handleStreamPrefetch(MpStreamMsg& rMsg)
{
   StreamHandle handle = rMsg.getHandle() ;

   mpFromStream->prefetch(handle) ;

   return TRUE ;
}


UtlBoolean MpCallFlowGraph::handleStreamRewind(MpStreamMsg& rMsg)
{
   StreamHandle handle = rMsg.getHandle() ;

   mpFromStream->rewind(handle) ;

   return TRUE ;
}



UtlBoolean MpCallFlowGraph::handleStreamPlay(MpStreamMsg& rMsg)
{
   UtlBoolean boolRes ;
   StreamHandle handle = rMsg.getHandle() ;
   int iFlags ;

   if (mpFromStream->getFlags(handle, iFlags) == OS_SUCCESS)
   {
      // Should we play locally?
      if (iFlags & STREAM_SOUND_LOCAL)
      {
         boolRes = mpTFsBridgeMixer->disable();
         assert(boolRes);
      }
      else
      {
         boolRes = mpTFsBridgeMixer->enable();
         assert(boolRes);
      }

      // Should we play remotely?
      if (iFlags & STREAM_SOUND_REMOTE)
      {
         boolRes = mpTFsMicMixer->disable();
         assert(boolRes);
      }
      else
      {
         boolRes = mpTFsMicMixer->enable();
         assert(boolRes);
      }

      mpFromStream->play(handle) ;
      mpFromStream->enable() ;
   }

   return TRUE ;
}


UtlBoolean MpCallFlowGraph::handleStreamPause(MpStreamMsg& rMsg)
{
   StreamHandle handle = rMsg.getHandle() ;

   mpFromStream->pause(handle) ;

   return TRUE ;
}

UtlBoolean MpCallFlowGraph::handleStreamStop(MpStreamMsg& rMsg)
{
   UtlBoolean boolRes;
   StreamHandle handle = rMsg.getHandle() ;
   int iFlags ;

   // now lets do the enabling of devices we disabled
   // earlier in the handleStreamPlay method
   mpFromStream->stop(handle) ;

   if (mpFromStream->getFlags(handle, iFlags) == OS_SUCCESS)
   {
      // did we play locally?
      if (iFlags & STREAM_SOUND_LOCAL)
      {
         boolRes = mpTFsBridgeMixer->enable();
         assert(boolRes);
      }

      // did we play remotely?
      if (iFlags & STREAM_SOUND_REMOTE)
      {
         boolRes = mpTFsMicMixer->enable();
         assert(boolRes);
      }
   }

   return TRUE ;
}

UtlBoolean MpCallFlowGraph::handleStreamDestroy(MpStreamMsg& rMsg)
{
   StreamHandle handle = rMsg.getHandle() ;

   mpFromStream->destroy(handle) ;

   return TRUE ;
}

/* ============================ FUNCTIONS ================================= */
