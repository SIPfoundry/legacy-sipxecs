//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#include "rtcp/RtcpConfig.h"

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "mp/MpMediaTask.h"
#include "mp/MpConnection.h"
#include "mp/MpFlowGraphBase.h"
#include "mp/MpCallFlowGraph.h"
#include "mp/MprEncode.h"
#include "mp/MprToNet.h"
#include "mp/MprFromNet.h"
#include "mp/MprDejitter.h"
#include "mp/MprDecode.h"
#include "mp/JB/JB_API.h"
#include "net/SdpCodec.h"
#include "os/OsLock.h"
#include "os/OsTask.h"
#ifdef INCLUDE_RTCP /* [ */
#include "rtcp/INetDispatch.h"
#include "rtcp/IRTPDispatch.h"
#include "rtcp/ISetSenderStatistics.h"
#else
#include "os/OsDateTime.h"
#endif /* INCLUDE_RTCP ] */
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MpConnection::MpConnection(MpCallFlowGraph* pParent, MpConnectionID myID,
                                 int samplesPerFrame, int samplesPerSec)
             : mLock(OsMutex::Q_PRIORITY|OsMutex::INVERSION_SAFE)
#ifdef INCLUDE_RTCP /* [ */
             , mpiRTCPConnection(NULL)
#endif /* INCLUDE_RTCP ] */
{

   OsStatus     res;
   char         name[50];
   int          i;

#ifdef INCLUDE_RTCP /* [ */
// Get the Session Interface associated with this connection's flow graph
   IRTCPSession *piRTCPSession = pParent->getRTCPSessionPtr();

// Let's create an RTCP Connection to accompany the MP Connection just created.

   mpiRTCPConnection = piRTCPSession->CreateRTCPConnection();

   assert(mpiRTCPConnection != NULL);

// Let's use the Connection interface to acquire the constituent interfaces
// required for dispatching RTP and RTCP packets received from the network as
// well as the statistics interface tabulating RTP packets going to the network.
   INetDispatch         *piRTCPDispatch;
   IRTPDispatch         *piRTPDispatch;
   ISetSenderStatistics *piRTPAccumulator;

   mpiRTCPConnection-> GetDispatchInterfaces(&piRTCPDispatch,
      &piRTPDispatch, &piRTPAccumulator);
#endif /* INCLUDE_RTCP ] */


   sprintf(name, "Decode-%d", myID);
   mpDecode    = new MprDecode(name, this, samplesPerFrame, samplesPerSec);
   sprintf(name, "Dejitter-%d", myID);
   mpDejitter  = new MprDejitter(name, this, samplesPerFrame, samplesPerSec);
   sprintf(name, "Encode-%d", myID);
   mpEncode    = new MprEncode(name, samplesPerFrame, samplesPerSec);
   sprintf(name, "FromNet-%d", myID);
   mpFromNet   = new MprFromNet(name, this, samplesPerFrame, samplesPerSec);
   sprintf(name, "ToNet-%d", myID);
   mpToNet     = new MprToNet(name, samplesPerFrame, samplesPerSec);
   mpFlowGraph = pParent;
   mMyID       = myID;
   mBridgePort = -1;
   mpJB_inst   = NULL;

 //memset((char*)mpPayloadMap, 0, (NUM_PAYLOAD_TYPES*sizeof(MpDecoderBase*)));
   for (i=0; i<NUM_PAYLOAD_TYPES; i++) {
      mpPayloadMap[i] = NULL;
   }

#ifdef INCLUDE_RTCP /* [ */

// The MprFromNet object needs the RTP and RTCP Dispatch interfaces of the
// associated RTCP connection so that RTP and RTCP packets may be forwarded
// to the correct location.
   mpFromNet->setDispatchers(piRTPDispatch, piRTCPDispatch);

// Set the Statistics interface to be used by the RTP stream to increment
// packet and octet statistics
   mpToNet->setRTPAccumulator(piRTPAccumulator);

// The RTP Stream associated with the MprToNet object must have its SSRC ID
// set to the value generated from the Session.
   mpToNet->setSSRC(piRTCPSession->GetSSRC());
#else /* INCLUDE_RTCP ] [ */
   {
      OsDateTime date;
      OsTime now;
      int ssrc;
      OsDateTime::getCurTime(date);
      date.cvtToTimeSinceEpoch(now);
      ssrc = now.seconds() ^ now.usecs();
      mpToNet->setSSRC(ssrc);
   }
#endif /* INCLUDE_RTCP ] */


   res = pParent->addResource(*mpDecode);      assert(res == OS_SUCCESS);
   res = pParent->addResource(*mpDejitter);    assert(res == OS_SUCCESS);
   res = pParent->addResource(*mpEncode);      assert(res == OS_SUCCESS);
   res = pParent->addResource(*mpFromNet);     assert(res == OS_SUCCESS);
   res = pParent->addResource(*mpToNet);       assert(res == OS_SUCCESS);


   //////////////////////////////////////////////////////////////////////////
   // connect FromNet -> Dejitter -> Decode
   res = pParent->addLink(*mpFromNet, 0, *mpDejitter, 0);
   assert(res == OS_SUCCESS);

   res = pParent->addLink(*mpDejitter, 0, *mpDecode, 0);
   assert(res == OS_SUCCESS);

   //////////////////////////////////////////////////////////////////////////
   // connect Encode -> ToNet
   res = pParent->addLink(*mpEncode, 0, *mpToNet, 0);
   assert(res == OS_SUCCESS);

   //////////////////////////////////////////////////////////////////////////
   mpEncode->setMyToNet(mpToNet);
   mpDecode->setMyDejitter(mpDejitter);
   mpFromNet->setMyDejitter(mpDejitter);
   mpToNet->setRtpPal(mpFromNet);

   pParent->synchronize("new Connection, before enable(), %dx%X\n");
   enable();
   pParent->synchronize("new Connection, after enable(), %dx%X\n");

}

// Destructor
MpConnection::~MpConnection()
{
#ifdef INCLUDE_RTCP /* [ */
// Get the Session Interface associated with this connection's flow graph
   IRTCPSession *piRTCPSession = mpFlowGraph->getRTCPSessionPtr();

// Let's free our RTCP Connection
   piRTCPSession->TerminateRTCPConnection(mpiRTCPConnection);
#endif /* INCLUDE_RTCP ] */
   if (NULL != mpJB_inst) {
      JB_free(mpJB_inst);
      mpJB_inst = NULL;
   }
}

/* ============================ MANIPULATORS ============================== */

// Disables the input path, output path, or both paths, of the connection.
// Resources on the path(s) will also be disabled by these calls.
// If the flow graph is not "started", this call takes effect
// immediately.  Otherwise, the call takes effect at the start of the
// next frame processing interval.
//!retcode: OS_SUCCESS - for now, these methods always return success

OsStatus MpConnection::disableIn(void) {
   mpDecode->disable();
   mpDejitter->disable();
   mpFromNet->disable();
   mInEnabled = FALSE;
   return OS_SUCCESS;
}

OsStatus MpConnection::disableOut(void) {
   mpEncode->disable();
   mpToNet->disable();
   mOutEnabled = FALSE;
   return OS_SUCCESS;
}

OsStatus MpConnection::disable(void) {
   MpConnection::disableIn();
   MpConnection::disableOut();
   return OS_SUCCESS;
}

// Enables the input path, output path, or both paths, of the connection.
// Resources on the path(s) will also be enabled by these calls.
// Resources may allocate needed data (e.g. output path reframe buffer)
//  during this operation.
// If the flow graph is not "started", this call takes effect
// immediately.  Otherwise, the call takes effect at the start of the
// next frame processing interval.
//!retcode: OS_SUCCESS - for now, these methods always return success

OsStatus MpConnection::enableIn(void) {
   mpDecode->enable();
   mpDejitter->enable();
   mpFromNet->enable();
   mInEnabled = TRUE;
   return OS_SUCCESS;
}

OsStatus MpConnection::enableOut(void) {
   mpEncode->enable();
   mpToNet->enable();
   mOutEnabled = TRUE;
   return OS_SUCCESS;
}

OsStatus MpConnection::enable(void) {
   MpConnection::enableIn();
   MpConnection::enableOut();
   return OS_SUCCESS;
}

// Start sending RTP and RTCP packets.

void MpConnection::startSendRtp(OsSocket& rRtpSocket,
               OsSocket& rRtcpSocket,
               SdpCodec* pPrimaryCodec,
               SdpCodec* pDtmfCodec,
               SdpCodec* pSecondaryCodec)
{
   mpToNet->setSockets(rRtpSocket, rRtcpSocket);
   mpFromNet->setDestIp(rRtpSocket);

#ifdef INCLUDE_RTCP /* [ */
// Associate the RTCP socket to be used by the RTCP Render portion of the
// connection to write reports to the network
   mpiRTCPConnection->StartRenderer(rRtcpSocket);
#endif /* INCLUDE_RTCP ] */

   // This should be ok to set directly as long as we do not switch mid stream
   // Eventually this needs to be a message
#if 0
   osPrintf("MpConnection::startSendRtp setting send codecs:\n");

   if (NULL != pPrimaryCodec) {
      osPrintf("  Primary audio: codec=%d, payload type=%d\n",
          pPrimaryCodec->getCodecType(),
          pPrimaryCodec->getCodecPayloadFormat());
   } else {
      osPrintf("  Primary audio: NONE\n");
   }
   if (NULL != pDtmfCodec) {
      osPrintf("  DTMF Tones: codec=%d, payload type=%d\n",
          pDtmfCodec->getCodecType(), pDtmfCodec->getCodecPayloadFormat());
   } else {
      osPrintf("  DTMF Tones: NONE\n");
   }
   if (NULL != pSecondaryCodec) {
      osPrintf("  Secondary audio: codec=%d, payload type=%d\n",
          pSecondaryCodec->getCodecType(),
          pSecondaryCodec->getCodecPayloadFormat());
   } else {
      osPrintf("  Secondary audio: NONE\n");
   }
#endif
   mpEncode->selectCodecs(pPrimaryCodec, pDtmfCodec, pSecondaryCodec);
   mOutRtpStarted = TRUE;
   mpFlowGraph->synchronize();
   mpEncode->enable();
}

void MpConnection::startSendRtp(SdpCodec& rCodec,
                                    OsSocket& rRtpSocket,
                                    OsSocket& rRtcpSocket)
{
   startSendRtp(rRtpSocket, rRtcpSocket, &rCodec, NULL, NULL);
}

// Stop sending RTP and RTCP packets.
void MpConnection::stopSendRtp()
{
#ifdef INCLUDE_RTCP /* [ */
// Terminate the RTCP Connection which shall include stopping the RTCP
// Render so that no additional reports are emitted
   mpiRTCPConnection->StopRenderer();
#endif /* INCLUDE_RTCP ] */

   mpToNet->resetSockets(); //$$$ race condition? how long before the
                            // sockets are deleted???

//   osPrintf("MpConnection::stopSendRtp resetting send codec\n");
   mpEncode->deselectCodecs();
   mOutRtpStarted = FALSE;
   mpFlowGraph->synchronize();
   mpEncode->disable();
}

// Start receiving RTP and RTCP packets.

void MpConnection::startReceiveRtp(SdpCodec* pCodecs[], int numCodecs,
                                       OsSocket& rRtpSocket,
                                       OsSocket& rRtcpSocket)
{
   mpDecode->selectCodecs(pCodecs, numCodecs);
   mpFlowGraph->synchronize();
   mpFromNet->setSockets(rRtpSocket, rRtcpSocket);
   mInRtpStarted = TRUE;
   mpFlowGraph->synchronize();
   mpDecode->enable();
}

// Stop receiving RTP and RTCP packets.
void MpConnection::stopReceiveRtp()
{
   JB_inst* pJB_inst;

   mpFromNet->resetSockets();
   mpFlowGraph->synchronize();
   mInRtpStarted = FALSE;
   mpFlowGraph->synchronize();

   mpDecode->deselectCodec();
   mpFlowGraph->synchronize();

   pJB_inst = getJBinst(TRUE);  // get NULL if not allocated
   mpJB_inst = NULL;
   mpFlowGraph->synchronize();

   if (NULL != pJB_inst) {
      JB_free(pJB_inst);
   }
   mpDecode->disable();
}

OsStatus MpConnection::setBridgePort(int port)
{
   if (-1 != mBridgePort) return OS_BUSY;
   mBridgePort = port;
   return OS_SUCCESS;
}

void MpConnection::startTone(int toneId)
{
   mpEncode->startTone(toneId);
}

void MpConnection::stopTone(void)
{
   mpEncode->stopTone();
}

void MpConnection::addPayloadType(int payloadType, MpDecoderBase* decoder)
{
   OsLock lock(mLock);

   // Check that payloadType is valid.
   if ((payloadType < 0) || (payloadType >= NUM_PAYLOAD_TYPES))
   {
      OsSysLog::add(FAC_MP, PRI_ERR,
                    "MpConnection::addPayloadType Attempting to add an invalid payload type %d", payloadType);
   }
   // Check to see if we already have a decoder for this paylod type.
   else if (!(NULL == mpPayloadMap[payloadType]))
   {
      // This condition probably indicates that the sender of SDP specified
      // two decoders for the same payload type number.
      OsSysLog::add(FAC_MP, PRI_ERR,
                    "MpConnection::addPayloadType Attempting to add a second decoder for payload type %d",
                    payloadType);
   }
   else
   {
      mpPayloadMap[payloadType] = decoder;
   }
}

void MpConnection::deletePayloadType(int payloadType)
{
   OsLock lock(mLock);

   // Check that payloadType is valid.
   if ((payloadType < 0) || (payloadType >= NUM_PAYLOAD_TYPES))
   {
      OsSysLog::add(FAC_MP, PRI_ERR,
                    "MpConnection::deletePayloadType Attempting to delete an invalid payload type %d", payloadType);
   }
   // Check to see if this entry has already been deleted.
   else if (NULL == mpPayloadMap[payloadType])
   {
      // Either this payload type was doubly-added (and reported by
      // addPayloadType) or we've hit the race condtion in XMR-29.
      OsSysLog::add(FAC_MP, PRI_ERR,
                    "MpConnection::deletePayloadType Attempting to delete again payload type %d",
                    payloadType);
      OsSysLog::add(FAC_MP, PRI_ERR,
                    "MpConnection::deletePayloadType If there is no message from MpConnection::addPayloadType above, see XMR-29");
   }
   else
   {
      mpPayloadMap[payloadType] = NULL;
   }
}

void MpConnection::setPremiumSound(PremiumSoundOptions op)
{
#ifdef HAVE_GIPS /* [ */
   int NetEqOp = NETEQ_PLAYOUT_MODE_OFF;

   // this must only be called in the context of the Media Task
   assert(OsTask::getCurrentTask() == MpMediaTask::getMediaTask(0));

   if (EnablePremiumSound == op) {
      NetEqOp = NETEQ_PLAYOUT_MODE_ON;
   }
   if (NULL != mpJB_inst) {
#ifndef __pingtel_on_posix__
      NETEQ_GIPS_10MS16B_SetPlayoutMode(mpJB_inst, NetEqOp);
#endif
/*
      osPrintf("MpConnection::setPremiumSound: %sabling Premium Sound on #%d\n",
         (EnablePremiumSound == op) ? "En" : "Dis", mMyID);
*/
   }
#endif /* HAVE_GIPS ] */
}

/* ============================ ACCESSORS ================================= */

//:Returns a pointer to the Jitter Buffer instance, creating it if necessary
// If the instance has not been created, but the argument "optional" is
// TRUE, then do not create it, just return NULL.

JB_inst* MpConnection::getJBinst(UtlBoolean optional) {

   if ((NULL == mpJB_inst) && (!optional)) {
      int res;
      res = JB_create(&mpJB_inst);
/*
      osPrintf("MpConnection::getJBinst: JB_create=>0x%p\n",
         mpJB_inst);
*/

      assert(NULL != mpJB_inst);

      //Here it is hard coded to use 8000 Hz sampling frequency
      //This number is only relevant until any packet has arrived
      //When packet arrives the codec determines the output samp.freq.

      res |= JB_init(mpJB_inst, 8000);

      if (0 != res) { //just in case
         osPrintf("MpConnection::getJBinst: Jitter Buffer init failure!\n");
         if (NULL != mpJB_inst) {
            JB_free(mpJB_inst);
            mpJB_inst = NULL;
         }
      }
      if (NULL != mpJB_inst) {
         UtlBoolean on = mpFlowGraph->isPremiumSoundEnabled();
/*
         osPrintf("MpConnection::getJBinst: %sabling Premium Sound on #%d\n",
            on ? "En" : "Dis", mMyID);
*/
         setPremiumSound(on ? EnablePremiumSound : DisablePremiumSound);
      }
   }
   return(mpJB_inst);
}

//Returns the resource to link to upstream resource's outPort.
MpResource* MpConnection::getSinkResource() {
   return mpEncode;
}

//Returns the resource to link to downstream resource's inPort.
MpResource* MpConnection::getSourceResource() {
   return mpDecode;
}

//Retrieves the port number that was assigned by the bridge.
int MpConnection::getBridgePort() {
   return mBridgePort;
}

MpDecoderBase* MpConnection::mapPayloadType(int payloadType)
{
   OsLock lock(mLock);

   if ((payloadType < 0) || (payloadType >= NUM_PAYLOAD_TYPES))
   {
      OsSysLog::add(FAC_MP, PRI_ERR,
                    "MpConnection::mapPayloadType Attempting to map an invalid payload type %d", payloadType);
      return NULL;
   }
   else
   {
      return mpPayloadMap[payloadType];
   }
}


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

UtlBoolean MpConnection::handleSetDtmfNotify(OsNotification* pNotify)
{
   return mpDecode->handleSetDtmfNotify(pNotify);
}

UtlBoolean MpConnection::setDtmfTerm(MprRecorder *pRecorders)
{
   return mpDecode->setDtmfTerm(pRecorders);
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
