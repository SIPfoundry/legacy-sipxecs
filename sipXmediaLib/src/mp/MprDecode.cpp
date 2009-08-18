//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#define DEBUG_DECODING
#undef DEBUG_DECODING

// SYSTEM INCLUDES
#include <assert.h>

#ifdef WIN32 /* [ */
#include <winsock2.h>
#endif /* WIN32 ] */

#if defined(_VXWORKS) || defined(__pingtel_on_posix__) /* [ */
#include <sys/types.h>
#include <netinet/in.h>
#endif /* _VXWORKS || __pingtel_on_posix__ ] */

#include <string.h>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsSysLog.h"
#include "os/OsLock.h"
#include "mp/MpMisc.h"
#include "mp/MpBuf.h"
#include "mp/MpConnection.h"
#include "mp/MprDecode.h"
#include "mp/MprDejitter.h"
#include "mp/MpDecoderBase.h"
#include "mp/NetInTask.h"
#include "mp/dmaTask.h"
#include "mp/MpMediaTask.h"
#include "mp/MpCodecFactory.h"
#include "mp/JB/JB_API.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES

// CONSTANTS

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprDecode::MprDecode(const UtlString& rName, MpConnection* pConn,
                           int samplesPerFrame, int samplesPerSec)
:  MpResource(rName, 1, 1, 1, 1, samplesPerFrame, samplesPerSec),
   mPreloading(1),
   mpMyDJ(NULL),
   mLock(OsMutex::Q_PRIORITY|OsMutex::INVERSION_SAFE),
   mpCurrentCodecs(NULL),
   mNumCurrentCodecs(0),
   mpPrevCodecs(NULL),
   mNumPrevCodecs(0),
   mpConnection(pConn),
   mNumMarkerNotices(0),
   mFrameLastMarkerNotice(0),
   mFrameCounter(0),
   mpNotify(NULL),
   mpRecorder(NULL)
{

   // no work required
}

// Destructor
MprDecode::~MprDecode()
{
   // Clean up decoder object
   int i;
#if 0
   osPrintf("|~MprDecode(0x%p): calling handleDeselectCodecs (%d to free)\n",
      this, mNumCurrentCodecs);
#endif
   // Release our codecs (if any), and the array of pointers to them
   handleDeselectCodecs();
#if 0
   osPrintf("|~MprDecode(0x%p): deleting %d decoders, and array at 0x%p\n",
      this, mNumPrevCodecs, mpPrevCodecs);
#endif
   // Delete the list of codecs used in the past.
   {
      OsLock lock(mLock);
      if (mNumPrevCodecs > 0) {
         for (i=0; i<mNumPrevCodecs; i++) {
            mpPrevCodecs[i]->freeDecode();
            delete mpPrevCodecs[i];
         }
         delete[] mpPrevCodecs;
      }
   }
}

/* ============================ MANIPULATORS ============================== */

OsStatus MprDecode::selectCodecs(SdpCodec* codecs[], int numCodecs)
{
   OsStatus ret = OS_SUCCESS;
   SdpCodec** codecArray;
   int i;
   MpFlowGraphMsg msg(SELECT_CODECS, this, NULL, NULL, 0, 0);

   codecArray = new SdpCodec*[numCodecs];

   for (i=0; i<numCodecs; i++) {
      codecArray[i] = new SdpCodec(*codecs[i]);
   }

   msg.setPtr1(codecArray);
   msg.setInt1(numCodecs);
   ret = postMessage(msg);

   return ret;
}

OsStatus MprDecode::deselectCodec()
{
   MpFlowGraphMsg msg(DESELECT_CODECS, this, NULL, NULL, 0, 0);
   OsStatus ret = OS_SUCCESS;

//   osPrintf("MprDecode::deselectCodec\n");
   ret = postMessage(msg);

   return ret;
}

void MprDecode::setMyDejitter(MprDejitter* pDJ)
{
   mpMyDJ = pDJ;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

UtlBoolean MprDecode::handleSetDtmfNotify(OsNotification* pNotify)
{
   MpDecoderBase** pMDB;
   UtlBoolean ret = TRUE;
   int i;
   OsLock lock(mLock);

   // Apply the notifier to all current codecs.
   pMDB = mpCurrentCodecs;
   for (i=0; i<mNumCurrentCodecs; i++)
   {
      if ((*pMDB)->getInfo()->isSignalingCodec())
      {
         (*pMDB)->handleSetDtmfNotify(pNotify);
      }
      pMDB++;
   }

   // Save the notifier to apply to future codecs.
   mpNotify = pNotify;

   return ret;
}

UtlBoolean MprDecode::setDtmfTerm(MprRecorder *pRecorder)
{
   MpDecoderBase** pMDB;
   UtlBoolean ret = TRUE;
   int i;
   OsLock lock(mLock);

   // Apply the recorder to all current codecs.
   pMDB = mpCurrentCodecs;
   for (i=0; i<mNumCurrentCodecs; i++)
   {
      if ((*pMDB)->getInfo()->isSignalingCodec())
      {
         (*pMDB)->setDtmfTerm(pRecorder);
      }
      pMDB++;
   }

   // Save the recorder to apply to future codecs.
   mpRecorder = pRecorder;

   return ret;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

MprDejitter* MprDecode::getMyDejitter(void)
{
   assert(NULL != mpMyDJ);
   return mpMyDJ;
}

#ifdef DEBUG /* [ */
static void showRtpPacket(MpBufPtr rtp)
{
   struct rtpHeader rh, *rp;
   int len;

   rp = (struct rtpHeader *) (MpBuf_getStorage(rtp));
   memcpy((char *) &rh, (char *) rp, sizeof(struct rtpHeader));
   rh.vpxcc = rp->vpxcc;
   rh.mpt = rp->mpt;
   rh.seq = ntohs(rp->seq);
   rh.timestamp = ntohl(rp->timestamp);
   rh.ssrc = ntohl(rp->ssrc);
   len = MpBuf_getNumSamples(rtp) - sizeof(struct rtpHeader);
   Zprintf("RcvRTP: %02X, %02X, %d, %d, %08X, and %d bytes of data\n",
      rh.vpxcc, rh.mpt, rh.seq, rh.timestamp, rh.ssrc, len);
}
#endif /* DEBUG ] */

void MprDecode::pushIntoJitterBuffer(MpBufPtr pPacket, int packetLen)
{
   int res;

   JB_inst* pJBState = mpConnection->getJBinst();
   unsigned char* pHeader = (unsigned char*)MpBuf_getStorage(pPacket);

   res = JB_RecIn(pJBState, pHeader, packetLen, 0);
   if (0 != res) {
      osPrintf(
         "\n\n *** JB_RecIn(0x%p, 0x%p, %d) returned %d\n",
         pJBState, pHeader, packetLen, res);
      osPrintf(" pt=%d, Ts=%d, Seq=%d (%2X %2X)\n\n",
         MprDejitter::getPayloadType(pPacket),
         MprDejitter::getTimestamp(pPacket), MprDejitter::getSeqNum(pPacket),
         *pHeader, *(pHeader+1));
   }
}

UtlBoolean MprDecode::doProcessFrame(MpBufPtr inBufs[],
                                    MpBufPtr outBufs[],
                                    int inBufsSize,
                                    int outBufsSize,
                                    UtlBoolean isEnabled,
                                    int samplesPerFrame,
                                    int samplesPerSecond)
{
#ifdef DEBUG_DECODING /* [ */
static int numFramesForWarnings = 0;
static int numWarnings = 0;
#endif /* DEBUG_DECODING ] */

   MpBufPtr rtp;
   MpBufPtr out;

#ifdef DEBUG_DECODING /* [ */
   numFramesForWarnings++;
#endif /* DEBUG_DECODING ] */

   MpDecoderBase* pCurDec;
   Sample* pSamples = NULL;

   mFrameCounter++;

   if (0 == outBufsSize) return FALSE;

   if (!isEnabled) {
      mPreloading = 1;
      *outBufs = MpBuf_getFgSilence();
      return TRUE;
   }

   {
      MprDejitter* pDej = getMyDejitter();
      int packetLen;
      int pt;

      while (NULL != (rtp = pDej->pullPacket())) {
         pt = MprDejitter::getPayloadType(rtp);
         pCurDec = mpConnection->mapPayloadType(pt);
         if (NULL != pCurDec) {
            unsigned char* pRtpH;
            pRtpH = ((unsigned char*) MpBuf_getStorage(rtp)) + 1;
            if (0x80 == (0x80 & *pRtpH)) {
               if ((mFrameLastMarkerNotice + MARKER_WAIT_FRAMES) <
                         mFrameCounter) {
                  mNumMarkerNotices = 0;
               }
               if (mNumMarkerNotices++ < MAX_MARKER_NOTICES)
               {
                  // osPrintf("MprDecode: RTP marker bit ON\n");
                  mFrameLastMarkerNotice = mFrameCounter;
               }
            }
            packetLen = pCurDec->decodeIn(rtp);
            if (packetLen > 0) {
               pushIntoJitterBuffer(rtp, packetLen);
            }
         }
         MpBuf_delRef(rtp);
      }
   }

   out = MpBuf_getBuf(MpMisc.UcbPool, samplesPerFrame, 0, MP_FMT_T12);
   if (out)
   {
      pSamples = MpBuf_getSamples(out);
      memset(pSamples, 0, samplesPerFrame * sizeof(Sample));
      MpBuf_setSpeech(out, MP_SPEECH_SILENT);
   }
   JB_inst* pJBState = mpConnection->getJBinst();
   if (pJBState) {
      // This should be a JB_something or other.  However the only
      // current choices is a short or long equivalant and this needs
      // to be a plain old int:
      int outLen;
      int res;
      res = JB_RecOut(pJBState, pSamples, &outLen);
      MpBuf_setSpeech(out, MP_SPEECH_UNKNOWN);
   }

   *outBufs = out;
   Nprintf("Decode_doPF: returning 0x%p\n", out, 0,0,0,0,0);
   return TRUE;
}

// Handle messages for this resource.
UtlBoolean MprDecode::handleMessage(MpFlowGraphMsg& rMsg)
{
   UtlBoolean ret = FALSE;

   switch (rMsg.getMsg()) {
   case DESELECT_CODECS:
      handleDeselectCodecs();
      ret = TRUE;
      break;
   case SELECT_CODECS:
      handleSelectCodecs((SdpCodec**) rMsg.getPtr1(), rMsg.getInt1());
      ret = TRUE;
      break;
   default:
      ret = MpResource::handleMessage(rMsg);
      break;
   }
   return ret;
}

UtlBoolean MprDecode::handleSelectCodecs(SdpCodec* pCodecs[], int numCodecs)
{
   int i;
   SdpCodec* pCodec;
   int payload;
   SdpCodec::SdpCodecTypes ourCodec;
   SdpCodec::SdpCodecTypes oldSdpType = SdpCodec::SDP_CODEC_UNKNOWN;
   OsStatus ret;
   MpDecoderBase* pNewDecoder;
   MpDecoderBase* pOldDecoder;
   MpCodecFactory* pFactory = MpCodecFactory::getMpCodecFactory();
   int allReusable = 1;
   int canReuse;
#if 0
   osPrintf("MprDecode::handleSelectCodecs(%d codec%s):\n",
      numCodecs, ((1 == numCodecs) ? "" : "s"));
#endif
   if (OsSysLog::willLog(FAC_MP, PRI_DEBUG))
   {
      for (i=0; i<numCodecs; i++) {
         pCodec = pCodecs[i];
         OsSysLog::add(FAC_MP, PRI_DEBUG,
                       "MprDecode::handleSelectCodecs "
                       "pCodecs[%d]->getCodecType() = %d, "
                       "pCodecs[%d]->getCodecPayloadFormat() = %d",
                       i, pCodec->getCodecType(),
                       i, pCodec->getCodecPayloadFormat());
            }
   }

   // Check to see if all codecs in pCodecs can be handled by codecs
   // in mpCurrentCodecs.
   for (i=0; i<numCodecs; i++) {
      pCodec = pCodecs[i];
      ourCodec = pCodec->getCodecType();
      payload = pCodec->getCodecPayloadFormat();
#if 0
      osPrintf("  #%d: New=0x%p/i:%d/x:%d, ",
         i, ourCodec, payload);
#endif
      pOldDecoder = mpConnection->mapPayloadType(payload);
      if (NULL != pOldDecoder) {
         oldSdpType = pOldDecoder->getInfo()->getCodecType();
#if 0
         osPrintf("  Old=0x%p/i:%d", oldSdpType);
#endif
         canReuse = (ourCodec == oldSdpType)
            || ((SdpCodec::SDP_CODEC_G729AB == ourCodec)
                            && (SdpCodec::SDP_CODEC_G729A == oldSdpType))
            || ((SdpCodec::SDP_CODEC_G729A == ourCodec)
                            && (SdpCodec::SDP_CODEC_G729AB == oldSdpType));
      } else {
         // osPrintf("  no Old");
         canReuse = 0;
      }
      allReusable &= canReuse;
#if 0
      osPrintf(" i:%d/x:%d (%sreusable%s)\n", ourCodec, payload,
         (canReuse ? "" : "not "),
         (canReuse && (ourCodec != oldSdpType) ? "[*]" : ""));
#endif
   }

   // If the new list is not a subset of the old list, we have to copy
   // pCodecs into mpCurrentCodecs.
   if (!allReusable) {
      // Lock the m*Codecs members.
      OsLock lock(mLock);

      // Delete the current codecs.
      handleDeselectCodecs();

      mNumCurrentCodecs = numCodecs;
      mpCurrentCodecs = new MpDecoderBase*[numCodecs];

      for (i=0; i<numCodecs; i++) {
         pCodec = pCodecs[i];
         ourCodec = pCodec->getCodecType();
         payload = pCodec->getCodecPayloadFormat();
         ret = pFactory->createDecoder(ourCodec, payload, pNewDecoder);
         assert(OS_SUCCESS == ret);
         assert(NULL != pNewDecoder);
         pNewDecoder->initDecode(mpConnection);
         // Set up the DTMF notifier and media recorder, if any.
         if (pNewDecoder->getInfo()->isSignalingCodec())
         {
            pNewDecoder->handleSetDtmfNotify(mpNotify);
            pNewDecoder->setDtmfTerm(mpRecorder);
         }
         // Add this codec to mpConnection's payload type decoding table.
         mpConnection->addPayloadType(payload, pNewDecoder);
         mpCurrentCodecs[i] = pNewDecoder;
      }

      // Go back and add any signaling codecs to Jitter Buffer.
      for (i=0; i<numCodecs; i++) {
         if (mpCurrentCodecs[i]->getInfo()->isSignalingCodec()) {
            mpCurrentCodecs[i]->initDecode(mpConnection);
         }
      }
   }

   // Delete the list pCodecs.
   for (i=0; i<numCodecs; i++) {
      delete pCodecs[i];
   }
   delete[] pCodecs;
   return TRUE;
}

UtlBoolean MprDecode::handleDeselectCodec(MpDecoderBase* pDecoder)
{
   int payload;

   if (NULL != pDecoder) {
      payload = pDecoder->getPayloadType();
      mpConnection->deletePayloadType(payload);
   }
   return TRUE;
}

UtlBoolean MprDecode::handleDeselectCodecs()
{
   int i;
   MpDecoderBase** pCurrentCodecs;
   MpDecoderBase** pPrevCodecs;
   int newN;
   OsLock lock(mLock);

   if (0 < mNumCurrentCodecs) {

      newN = mNumCurrentCodecs + mNumPrevCodecs;
      pPrevCodecs = new MpDecoderBase*[newN];
#if 0
      osPrintf("|handleDeselectCodecs(0x%p): (0x%p,%d) -> (0x%p,%d) (+%d)\n",
         this, mpPrevCodecs, mNumPrevCodecs, pPrevCodecs,
         newN, mNumCurrentCodecs);
#endif
      if (mNumPrevCodecs > 0) {
         for (i=0; i<mNumPrevCodecs; i++) {
            pPrevCodecs[i] = mpPrevCodecs[i];
         }
         delete[] mpPrevCodecs;
      }

      i = mNumCurrentCodecs;
      mNumCurrentCodecs = 0;
      pCurrentCodecs = mpCurrentCodecs;
      mpCurrentCodecs = NULL;
      while (i>0) {
         i--;
         handleDeselectCodec(pCurrentCodecs[i]);
         pPrevCodecs[i+mNumPrevCodecs] = pCurrentCodecs[i];
         pCurrentCodecs[i] = NULL;
      }
      delete[] pCurrentCodecs;
      mpPrevCodecs = pPrevCodecs;
      mNumPrevCodecs = newN;
   }
   return TRUE;
}

/* ============================ FUNCTIONS ================================= */
