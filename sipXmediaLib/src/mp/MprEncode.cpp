//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////

// SYSTEM INCLUDES
#include <assert.h>


#ifdef __pingtel_on_posix__
#include <sys/types.h>
#include <netinet/in.h>
#endif

#ifdef WIN32 /* [ */
#include <winsock2.h>
#endif /* WIN32 ] */

#include <string.h>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "mp/MpMisc.h"
#include "mp/MpBuf.h"
#include "mp/MprEncode.h"
#include "mp/MprToNet.h"
#include "mp/MpEncoderBase.h"
#include "mp/dmaTask.h"
#include "mp/MpMediaTask.h"
#include "mp/MpCodecFactory.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES

// CONSTANTS
static const int NO_WAIT = 0;

// STATIC VARIABLE INITIALIZATIONS
   // At 10 ms each, 10 seconds.  We will send an RTP packet to each active
   // destination at least this often, even when muted.
   const int MprEncode::RTP_KEEP_ALIVE_FRAME_INTERVAL = 1000;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprEncode::MprEncode(const UtlString& rName,
                           int samplesPerFrame, int samplesPerSec)
:  MpResource(rName, 1, 1, 1, 1, samplesPerFrame, samplesPerSec),
   mpPrimaryCodec(NULL),
   mpPacket1Buffer(NULL),
   mpPacket1Payload(NULL),
   mPacket1PayloadBytes(0),
   mActiveAudio1(FALSE),
   mMarkNext1(FALSE),
   mConsecutiveInactive1(0),
   mConsecutiveActive1(0),
   mConsecutiveUnsentFrames1(0),

   mpDtmfCodec(NULL),
   mpPacket2Buffer(NULL),
   mpPacket2Payload(NULL),
   mPacket2PayloadBytes(0),

   mCurrentTone(-1),
   mNumToneStops(-1),
   mTotalTime(0),
   mNewTone(0),

   mpSecondaryCodec(NULL),
   mpPacket3Buffer(NULL),
   mpPacket3Payload(NULL),
   mPacket3PayloadBytes(0),
   mActiveAudio3(FALSE),
   mMarkNext3(FALSE),
   mConsecutiveInactive3(0),
   mConsecutiveActive3(0),
   mConsecutiveUnsentFrames3(0),

   mpToNet(NULL)
{
   mPacket1PayloadUsed = 0;
}

// Destructor
MprEncode::~MprEncode()
{
   if (NULL != mpPacket1Buffer) {
      delete[] mpPacket1Buffer;
      mpPacket1Buffer = NULL;
   }
   if (NULL != mpPacket2Buffer) {
      delete[] mpPacket2Buffer;
      mpPacket2Buffer = NULL;
   }
   if (NULL != mpPacket3Buffer) {
      delete[] mpPacket3Buffer;
      mpPacket3Buffer = NULL;
   }
   if (NULL != mpPrimaryCodec) {
      delete mpPrimaryCodec;
      mpPrimaryCodec = NULL;
   }
   if (NULL != mpDtmfCodec) {
      delete mpDtmfCodec;
      mpDtmfCodec = NULL;
   }
   if (NULL != mpSecondaryCodec) {
      delete mpSecondaryCodec;
      mpSecondaryCodec = NULL;
   }
   mpToNet = NULL;
}

/* ============================ MANIPULATORS ============================== */

void MprEncode::setMyToNet(MprToNet* myToNet)
{
   mpToNet = myToNet;
}

OsStatus MprEncode::startTone(int toneId)
{
   MpFlowGraphMsg msg(START_TONE, this, NULL, NULL, toneId, 0);
   return postMessage(msg);
}

OsStatus MprEncode::stopTone(void)
{
   MpFlowGraphMsg msg(STOP_TONE, this, NULL, NULL, 0, 0);
   return postMessage(msg);
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

OsStatus MprEncode::deselectCodecs(void)
{
   MpFlowGraphMsg msg(DESELECT_CODECS, this, NULL, NULL, 0, 0);

   return postMessage(msg);
}

OsStatus MprEncode::selectCodecs(SdpCodec* pPrimary, SdpCodec* pDtmf,
   SdpCodec* pSecondary)
{
   OsStatus res = OS_SUCCESS;
   MpFlowGraphMsg msg(SELECT_CODECS, this, NULL, NULL, 3, 0);
   SdpCodec** newCodecs;

   newCodecs = new SdpCodec*[3];
   newCodecs[0] = newCodecs[1] = newCodecs[2] = NULL;
   if (NULL != pPrimary)   newCodecs[0] = new SdpCodec(*pPrimary);
   if (NULL != pDtmf)      newCodecs[1] = new SdpCodec(*pDtmf);
   if (NULL != pSecondary) newCodecs[2] = new SdpCodec(*pSecondary);
   msg.setPtr1(newCodecs);
   res = postMessage(msg);

   return res;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

int MprEncode::payloadByteLength(MpEncoderBase& rEncoder)
{
   int maxBitsPerPacket = rEncoder.getInfo()->getMaxPacketBits();
   int packetPayloadBytes = 0;

   packetPayloadBytes = (maxBitsPerPacket + 7) / 8;
/*
   osPrintf(
      "MprEncode::payloadByteLength: maxBitsPerPacket=%d, returning bytes=%d\n",
      maxBitsPerPacket, packetPayloadBytes);
*/
   return packetPayloadBytes;
}

OsStatus MprEncode::allocPacketBuffer(MpEncoderBase& rEncoder,
   unsigned char*& rpPacketBuffer, unsigned char*& rpPacketPayload,
   int& rPacketPayloadBytes, int& rPacketPayloadUsed)
{
   OsStatus ret = OS_SUCCESS;

   rPacketPayloadBytes = payloadByteLength(rEncoder);
   int packetBytes =
         MprToNet::RESERVED_RTP_PACKET_HEADER_BYTES + rPacketPayloadBytes;
   rpPacketBuffer = new unsigned char[packetBytes+26];
   if (NULL != rpPacketBuffer) {
      rpPacketPayload = rpPacketBuffer +
                              MprToNet::RESERVED_RTP_PACKET_HEADER_BYTES;
      memset(rpPacketBuffer, 0, packetBytes+26);
      memcpy(rpPacketBuffer+packetBytes, "DON'T TOUCH!!!!!!!!!!!!!!", 26);
   } else {
      ret = OS_NO_MEMORY;
      rpPacketPayload = NULL;
   }
   rPacketPayloadUsed = 0;
   return ret;
}

void MprEncode::handleDeselectCodecs(void)
{
   if (NULL != mpPrimaryCodec) {
      delete mpPrimaryCodec;
      mpPrimaryCodec = NULL;
      if (NULL != mpPacket1Buffer) {
         delete[] mpPacket1Buffer;
         mpPacket1Buffer = NULL;
         mpPacket1Payload = NULL;
         mPacket1PayloadBytes = 0;
      }
   }
   if (NULL != mpDtmfCodec) {
      delete mpDtmfCodec;
      mpDtmfCodec = NULL;
      if (NULL != mpPacket2Buffer) {
         delete[] mpPacket2Buffer;
         mpPacket2Buffer = NULL;
         mpPacket2Payload = NULL;
         mPacket2PayloadBytes = 0;
      }
   }
   if (NULL != mpSecondaryCodec) {
      delete mpSecondaryCodec;
      mpSecondaryCodec = NULL;
      if (NULL != mpPacket3Buffer) {
         delete[] mpPacket3Buffer;
         mpPacket3Buffer = NULL;
         mpPacket3Payload = NULL;
         mPacket3PayloadBytes = 0;
      }
   }
}

static int sbAllowAvtCodec = 1;
extern "C" {
   extern int allowAvt(int flag);
}
int allowAvt(int flag) {
   int save = sbAllowAvtCodec;
   sbAllowAvtCodec = (flag ? 1 : 0);
   return save;
}

void MprEncode::handleSelectCodecs(MpFlowGraphMsg& rMsg)
{
   SdpCodec** newCodecs;
   SdpCodec* pPrimary;
   SdpCodec* pDtmf;
   SdpCodec* pSecondary;
   MpEncoderBase* pNewEncoder;
   MpCodecFactory* pFactory = MpCodecFactory::getMpCodecFactory();
   SdpCodec::SdpCodecTypes ourCodec;
   OsStatus ret;
   int payload;

   newCodecs = (SdpCodec**) rMsg.getPtr1();
   pPrimary = newCodecs[0];
   pDtmf = newCodecs[1];
   pSecondary = newCodecs[2];

   handleDeselectCodecs();  // cleanup the old ones, if any

   if (OsSysLog::willLog(FAC_MP, PRI_DEBUG))
   {
      if (NULL != pPrimary) {
         OsSysLog::add(FAC_MP, PRI_DEBUG,
                       "MprEncode::handleSelectCodecs "
                       "pPrimary->getCodecType() = %d, "
                       "pPrimary->getCodecPayloadFormat() = %d",
                       pPrimary->getCodecType(),
                       pPrimary->getCodecPayloadFormat());
      } else {
         OsSysLog::add(FAC_MP, PRI_DEBUG,
                       "MprEncode::handleSelectCodecs "
                       "pPrimary == NULL");
      }
      if (sbAllowAvtCodec && NULL != pDtmf) {
         OsSysLog::add(FAC_MP, PRI_DEBUG,
                       "MprEncode::handleSelectCodecs "
                       "pDtmf->getCodecType() = %d, "
                       "pDtmf->getCodecPayloadFormat() = %d",
                       pDtmf->getCodecType(),
                       pDtmf->getCodecPayloadFormat());
      } else {
         OsSysLog::add(FAC_MP, PRI_DEBUG,
                       "MprEncode::handleSelectCodecs "
                       "sbAllowAvtCodec = %d, pDtmf = %p",
                       sbAllowAvtCodec, pDtmf);
      }
      if (NULL != pSecondary) {
         OsSysLog::add(FAC_MP, PRI_DEBUG,
                       "MprEncode::handleSelectCodecs "
                       "pSecondary->getCodecType() = %d, "
                       "pSecondary->getCodecPayloadFormat() = %d",
                       pSecondary->getCodecType(),
                       pSecondary->getCodecPayloadFormat());
      } else {
         OsSysLog::add(FAC_MP, PRI_DEBUG,
                       "MprEncode::handleSelectCodecs "
                       "pSecondary == NULL");
      }
   }

   if (NULL != pPrimary) {
      ourCodec = pPrimary->getCodecType();
      payload = pPrimary->getCodecPayloadFormat();
      ret = pFactory->createEncoder(ourCodec, payload, pNewEncoder);
      assert(OS_SUCCESS == ret);
      assert(NULL != pNewEncoder);
      pNewEncoder->initEncode();
      mpPrimaryCodec = pNewEncoder;
      mDoesVad1 = (pNewEncoder->getInfo())->doesVadCng();
      allocPacketBuffer(*mpPrimaryCodec, mpPacket1Buffer,
         mpPacket1Payload, mPacket1PayloadBytes, mPacket1PayloadUsed);
   }

   if (sbAllowAvtCodec) {
      if (NULL != pDtmf) {
         ourCodec = pDtmf->getCodecType();
         payload = pDtmf->getCodecPayloadFormat();
         ret = pFactory->createEncoder(ourCodec, payload, pNewEncoder);
         assert(OS_SUCCESS == ret);
         assert(NULL != pNewEncoder);
         pNewEncoder->initEncode();
         mpDtmfCodec = pNewEncoder;
         allocPacketBuffer(*mpDtmfCodec, mpPacket2Buffer,
            mpPacket2Payload, mPacket2PayloadBytes, mPacket2PayloadUsed);
      }
   }

   if (NULL != pSecondary) {
      ourCodec = pSecondary->getCodecType();
      payload = pSecondary->getCodecPayloadFormat();
      ret = pFactory->createEncoder(ourCodec, payload, pNewEncoder);
      assert(OS_SUCCESS == ret);
      assert(NULL != pNewEncoder);
      pNewEncoder->initEncode();
      mpSecondaryCodec = pNewEncoder;
      mDoesVad3 = (pNewEncoder->getInfo())->doesVadCng();
      allocPacketBuffer(*mpSecondaryCodec, mpPacket3Buffer,
         mpPacket3Payload, mPacket3PayloadBytes, mPacket3PayloadUsed);
   }

   // delete any SdpCodec objects that we did not keep pointers to.
   if (NULL != pPrimary)   delete pPrimary;
   if (NULL != pDtmf)      delete pDtmf;
   if (NULL != pSecondary) delete pSecondary;

   // free the array we were sent
   delete[] newCodecs;
}

void MprEncode::handleStartTone(int toneId)
{
   if (NULL == mpDtmfCodec) return;
   if ((mCurrentTone == -1) && (mNumToneStops < 1)) {
      mCurrentTone = lookupTone(toneId);
      if (-1 != mCurrentTone) {
         mNewTone = 1;
      }
   }
}

void MprEncode::handleStopTone(void)
{
   if ((mCurrentTone > -1) && (mNumToneStops < 1)) {
      mNumToneStops = TONE_STOP_PACKETS; // send TONE_STOP_PACKETS end packets
   }
}

// Handle messages for this resource.
UtlBoolean MprEncode::handleMessage(MpFlowGraphMsg& rMsg)
{
   if (rMsg.getMsg() == SELECT_CODECS)
   {
      handleSelectCodecs(rMsg);
      return TRUE;
   } else if (rMsg.getMsg() == DESELECT_CODECS) {
      handleDeselectCodecs();
      return TRUE;
   } else if (rMsg.getMsg() == START_TONE) {
      handleStartTone(rMsg.getInt1());
      return TRUE;
   } else if (rMsg.getMsg() == STOP_TONE) {
      handleStopTone();
      return TRUE;
   }
   else
      return MpResource::handleMessage(rMsg);
}

// Translate our tone ID into RFC2833 values.
int MprEncode::lookupTone(int toneId)
{
   int ret = -1;

   switch (toneId) {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
         ret = toneId - '0';
         break;
      case  0 :
      case  1 :
      case  2 :
      case  3 :
      case  4 :
      case  5 :
      case  6 :
      case  7 :
      case  8 :
      case  9 :
         ret = toneId;
         break;
      case 'a': case 'A':
      case 'b': case 'B':
      case 'c': case 'C':
      case 'd': case 'D':
         ret = ((toneId | ('a' ^ 'A')) - ('a' | ('a' ^ 'A'))) + 12;
         break;
      case '*':
         ret = 10;
         break;
      case '#':
         ret = 11;
         break;
   }
   return ret;
}

#ifdef DEBUG /* [ */
static int NumberOfEncodes = 0;
#endif /* DEBUG ] */

void MprEncode::doPrimaryCodec(MpBufPtr in, unsigned int startTs)
{
   int numSamplesIn;
   int numSamplesOut;
   Sample* pSamplesIn;
   int payloadBytesLeft;
   unsigned char* pDest;
   int bytesAdded; //$$$
   MpBufSpeech content = MP_SPEECH_UNKNOWN;
   OsStatus ret;
   UtlBoolean sendNow;

   if (NULL == mpPrimaryCodec) return;

   numSamplesIn = MpBuf_getNumSamples(in);
   pSamplesIn = MpBuf_getSamples(in);

   while (numSamplesIn > 0) {

      if (0 == mPacket1PayloadUsed) {
         mStartTimestamp1 = startTs;
         mActiveAudio1 = mDoesVad1;
      }

      if (!mActiveAudio1) {
         mActiveAudio1 = MpBuf_isActiveAudio(in);
      }

      payloadBytesLeft = mPacket1PayloadBytes - mPacket1PayloadUsed;
      // maxSamplesOut = payloadBytesLeft / bytesPerSample;

      // n = (numSamplesIn > maxSamplesOut) ? maxSamplesOut : numSamplesIn;
      pDest = mpPacket1Payload + mPacket1PayloadUsed;

      bytesAdded = 0;
      ret = mpPrimaryCodec->encode(pSamplesIn, numSamplesIn, numSamplesOut,
                        pDest, payloadBytesLeft, bytesAdded,
                        sendNow, content);
      mPacket1PayloadUsed += bytesAdded;
      assert (mPacket1PayloadBytes >= mPacket1PayloadUsed);

      // In case the encoder does silence suppression (e.g. G.729 Annex B)
      mMarkNext1 = mMarkNext1 | (0 == bytesAdded);

      pSamplesIn += numSamplesOut;
      numSamplesIn -= numSamplesOut;
      startTs += numSamplesOut;

      if (MP_SPEECH_ACTIVE == content) {
         mActiveAudio1 = TRUE;
      }

      if (sendNow || (mPacket1PayloadBytes == mPacket1PayloadUsed)) {
         if (mActiveAudio1) {
            mConsecutiveInactive1 = 0;
         } else {
            mConsecutiveInactive1++;
         }
         if ((mConsecutiveInactive1 < HANGOVER_PACKETS) ||
             (mConsecutiveUnsentFrames1 >= RTP_KEEP_ALIVE_FRAME_INTERVAL))
         {
            mpToNet->writeRtp(mpPrimaryCodec->getPayloadType(),
               mMarkNext1,
               mpPacket1Payload,
               mPacket1PayloadUsed,
               mStartTimestamp1,
               NULL);
            mMarkNext1 = FALSE;
            mConsecutiveUnsentFrames1 = 0;
         } else {
            mMarkNext1 = TRUE;
         }
         mPacket1PayloadUsed = 0;
      }
   }
}

void MprEncode::doDtmfCodec(unsigned int startTs, int samplesPerFrame,
   int samplesPerSecond)
{
   int numSampleTimes;
#ifdef _VXWORKS /* [ */
   extern volatile int* pOsTC;
#endif /* _VXWORKS ] */
#ifdef DEBUG_DTMF_SEND /* [ */
   int skipped;
#endif /* DEBUG_DTMF_SEND ] */

   if (-1 == mCurrentTone) return;
   if (NULL == mpDtmfCodec) return;

   if (mNewTone) {
      mStartTimestamp2 = startTs;
      mDtmfSampleInterval = samplesPerFrame * 2;
      mNumToneStops = -1;
#ifdef _VXWORKS /* [ */
      OsSysLog::add(FAC_MP, PRI_INFO, "MprEncode::doDtmfCodec - key down,"
         " key=%d, TS=0x%X, OsTC=0x%X\n", mNewTone, startTs, *pOsTC);
#endif /* _VXWORKS ] */
   }

   if (TONE_STOP_PACKETS == mNumToneStops) {
      mTotalTime = startTs - mStartTimestamp2;
   }

   if (mNumToneStops-- < 0) {
      if (mNewTone ||
          ((mLastDtmfSendTimestamp + mDtmfSampleInterval) <= startTs)) {

         numSampleTimes = (startTs + samplesPerFrame) - mStartTimestamp2;
         if (numSampleTimes > ((1<<16) - 1)) numSampleTimes = ((1<<16) - 1);

         mpPacket2Payload[0] = mCurrentTone;
         mpPacket2Payload[1] = 10; // -10 dBm0
         mpPacket2Payload[2] = (numSampleTimes >> 8) & 0xff; // Big Endian
         mpPacket2Payload[3] = numSampleTimes & 0xff; // Big Endian
         mpToNet->writeRtp(mpDtmfCodec->getPayloadType(),
            (0 != mNewTone),  // set marker on first packet
            mpPacket2Payload,
            4,
            mStartTimestamp2,
            NULL);
         mLastDtmfSendTimestamp = startTs;
         mNewTone = 0;
#ifdef DEBUG_DTMF_SEND /* [ */
         skipped = 0;
      } else {
         skipped = 1;
      }
      if (mNumToneStops > -20) {
         osPrintf("doDtmfCodec: %d + %d = %d, %d -- %s\n",
            mLastDtmfSendTimestamp, mDtmfSampleInterval,
            (mLastDtmfSendTimestamp + mDtmfSampleInterval),
            startTs, (skipped ? "skipped" : "sent"));
#endif /* DEBUG_DTMF_SEND ] */
      }
   } else {
      numSampleTimes = mTotalTime;
      if (numSampleTimes > ((1<<16) - 1)) numSampleTimes = ((1<<16) - 1);

#ifdef _VXWORKS /* [ */
      OsSysLog::add(FAC_MP, PRI_DEBUG, "MprEncode::doDtmfCodec - key up (%d),"
         " key=%d, TS=0x%X, OsTC=0x%X\n",
         mNumToneStops, mNewTone, startTs, *pOsTC);
#endif /* _VXWORKS ] */

      mpPacket2Payload[0] = mCurrentTone;
      mpPacket2Payload[1] = (1<<7) + 10; // -10 dBm0, with E bit
      mpPacket2Payload[2] = (numSampleTimes >> 8) & 0xff; // Big Endian
      mpPacket2Payload[3] = numSampleTimes & 0xff; // Big Endian
      mpToNet->writeRtp(mpDtmfCodec->getPayloadType(),
         FALSE,
         mpPacket2Payload,
         4,
         mStartTimestamp2,
         NULL);
      mLastDtmfSendTimestamp = startTs;
      if (1 > mNumToneStops) { // all done, ready to start next tone.
         mCurrentTone = -1;
         mNumToneStops = -1;
         mTotalTime = 0;
      }
   }
}

void MprEncode::doSecondaryCodec(MpBufPtr in, unsigned int startTs)
{
   assert(FALSE);
}

UtlBoolean MprEncode::doProcessFrame(MpBufPtr inBufs[],
                                    MpBufPtr outBufs[],
                                    int inBufsSize,
                                    int outBufsSize,
                                    UtlBoolean isEnabled,
                                    int samplesPerFrame,
                                    int samplesPerSecond)
{
   MpBufPtr in;
   unsigned int startTs;

   mConsecutiveUnsentFrames1++;
   mConsecutiveUnsentFrames3++;

   if (0 == inBufsSize) return FALSE;

   if (!isEnabled) return TRUE;

   in = *inBufs;

   startTs = (showFrameCount(1) * samplesPerFrame);

   if (NULL != mpPrimaryCodec) {
      doPrimaryCodec(in, startTs);
   }

   if (NULL != mpDtmfCodec) {
      doDtmfCodec(startTs, samplesPerFrame, samplesPerSecond);
   }

   if (NULL != mpSecondaryCodec) {
      doSecondaryCodec(in, startTs);
   }

   // mLastTimestamp = startTs;  // Unused?

   return TRUE;
}
