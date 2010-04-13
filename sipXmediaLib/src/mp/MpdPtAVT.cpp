//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// APPLICATION INCLUDES
#ifdef __pingtel_on_posix__ /* [ */
#include <sys/types.h>
#include <netinet/in.h>
#endif /* __pingtel_on_posix__ ] */
#include "mp/MpConnection.h"
#include "mp/MpdPtAVT.h"
#include "mp/MprDejitter.h"
#include "mp/NetInTask.h"
#include "mp/MprRecorder.h"
#include "mp/JB/JB_API.h"
#include "os/OsNotification.h"
#ifdef _VXWORKS /* [ */
#include <inetlib.h>
#endif /* _VXWORKS ] */

struct avtPacket {
   struct rtpHeader rh;
   UCHAR  key;
   UCHAR  dB;
   short  samplesSwapped;
};

static int debugCtr = 0;

const MpCodecInfo MpdPtAVT::smCodecInfo(
         SdpCodec::SDP_CODEC_TONES, "Pingtel_1.0", false,
         8000, 0, 1, 0, 6400, 128, 128, 128, 160, TRUE);

MpdPtAVT::MpdPtAVT(int payloadType)
   : MpDecoderBase(payloadType, &smCodecInfo),
     mpJBState(NULL),
     mCurrentToneKey(-1),
     mPrevToneSignature(0),
     mCurrentToneSignature(0),
     mToneDuration(0),
     mpNotify(NULL),
     mpRecorder(NULL)
{
   OsSysLog::add(FAC_MP, PRI_INFO, "MpdPtAVT(%p)::MpdPtAVT(%d)\n",
      this, payloadType);
}

MpdPtAVT::~MpdPtAVT()
{
   freeDecode();
}

OsStatus MpdPtAVT::initDecode(MpConnection* pConnection)
{
   int res = 0;
   debugCtr = 0;

   if (NULL != mpJBState) {
      return OS_SUCCESS;  // we already did this, no need to do it again.
   }

   //Get JB pointer
   mpJBState = pConnection->getJBinst(TRUE);

   if (NULL != mpJBState) {
      int payloadType = getPayloadType();

      res = JB_initCodepoint(mpJBState,
         (char*) ("audio/telephone-event"), 8000, payloadType);

      OsSysLog::add(FAC_MP, PRI_DEBUG, "%sMpdAVT: registered with JB (pt=%d), res=%d\n",
         ((0==res) ? "" : " ***** "), payloadType, res);
   } else {
      OsSysLog::add(FAC_MP, PRI_DEBUG, "MpdAVT: NOT registering with JB\n");
   }

   return (0 == res) ? OS_SUCCESS : OS_UNSPECIFIED;
}

OsStatus MpdPtAVT::freeDecode(void)
{
   return OS_SUCCESS;
}

void dumpRawAvtPacket(struct avtPacket* pAvt, void* pThis)
{
   UCHAR vpxcc;
   UCHAR mpt;
   USHORT seq;
   UINT timestamp;
   UINT ssrc;

   UCHAR  key;
   UCHAR  dB;
   short  duration;


   vpxcc = pAvt->rh.vpxcc;
   mpt = pAvt->rh.mpt;
   seq = pAvt->rh.seq;
   seq = ntohs(seq);
   timestamp = pAvt->rh.timestamp;
   timestamp = ntohl(timestamp);
   ssrc = pAvt->rh.ssrc;
   ssrc = ntohl(ssrc);
   key = pAvt->key;
   dB = pAvt->dB;
   duration = pAvt->samplesSwapped;
   duration = ntohs(duration);

   OsSysLog::add(FAC_MP, PRI_INFO,
      " MpdPtAVT(%p): Raw packet: %02x %02x %6d %08x %08x %2d %02x %5d\n",
      pThis, vpxcc, mpt, seq, timestamp, ssrc, key, dB, duration);
}


int MpdPtAVT::decodeIn(MpBufPtr pPacket)
{
   struct avtPacket* pAvt;
   unsigned int samples;
   unsigned int ts;

   pAvt = (struct avtPacket*) MpBuf_getStorage(pPacket);

   dumpRawAvtPacket(pAvt, this);

   ts = pAvt->rh.timestamp;

   if (-1 != mCurrentToneKey) { // if previous tone still active
      if (mCurrentToneSignature != ts) { // and we have not seen this
         if (0 != mToneDuration) { // and its duration > 0
            OsSysLog::add(FAC_MP, PRI_INFO,
               "++++ MpdPtAVT(%p) SYNTHESIZING KEYUP for old key (%d)"
               " duration=%d ++++\n", this,
               mCurrentToneKey, mToneDuration);
            signalKeyUp(pPacket);
         }
      }
   }

   // Key Down (start of tone)
   if ((0x80 == (0x80 & (pAvt->rh.mpt))) && (ts != mCurrentToneSignature)) {
     // start bit marked
      OsSysLog::add(FAC_MP, PRI_INFO, "++++ MpdPtAVT(%p) RECEIVED KEYDOWN"
         " (marker bit set), duration=%d, TSs: old=0x%08x, new=0x%08x,"
         " delta=%d; mCurrentToneKey=%d ++++",
         this, mToneDuration, ntohl(mPrevToneSignature), ntohl(ts),
         ntohl(ts) - ntohl(mPrevToneSignature), mCurrentToneKey);
      signalKeyDown(pPacket);
      samples = pAvt->samplesSwapped;
      mToneDuration = (ntohs(samples) & 0xffff);
   } else if ((mPrevToneSignature != ts) && (-1 == mCurrentToneKey)) {
     // key up interpreted as key down if no previous start tone received
      OsSysLog::add(FAC_MP, PRI_INFO, "++++ MpdPtAVT(%p) RECEIVED KEYDOWN"
         " (lost packets?) duration=%d; TSs: old=0x%08x, new=0x%08x,"
         " delta=%d; ++++\n",
         this, mToneDuration, ntohl(mPrevToneSignature), ntohl(ts),
         ntohl(ts) - ntohl(mPrevToneSignature));
      signalKeyDown(pPacket);
      samples = pAvt->samplesSwapped;
      mToneDuration = (ntohs(samples) & 0xffff);
   }
   else
   {
      samples = pAvt->samplesSwapped;
      mToneDuration = (ntohs(samples) & 0xffff);
      if (mToneDuration && (0x80 != (0x80 & (pAvt->dB))))
      {
         OsSysLog::add(FAC_MP, PRI_INFO, "++++ MpdPtAVT(%p) RECEIVED packet, not KEYDOWN, set duration to zero"
              " duration=%d; TSs: old=0x%08x, new=0x%08x,"
              " delta=%d; ++++\n",
              this, mToneDuration, ntohl(mPrevToneSignature), ntohl(ts),
              ntohl(ts) - ntohl(mPrevToneSignature));
	      mToneDuration = 0;
      }
   }

   // Key Up (end of tone)
   if (0x80 == (0x80 & (pAvt->dB))) {
      OsSysLog::add(FAC_MP, PRI_INFO, "++++ MpdPtAVT(%p) RECEIVED KEYUP"
      " duration=%d, TS=0x%08x ++++\n", this, mToneDuration, ntohl(ts));
      signalKeyUp(pPacket);
   }

   return MpBuf_getContentLen(pPacket);
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

UtlBoolean MpdPtAVT::handleSetDtmfNotify(OsNotification* pNotify)
{
   OsSysLog::add(FAC_MP, PRI_DEBUG, "MpdPtAVT::handleSetDtmfNotify setting mpNotify = %p",
                 pNotify);
   mpNotify = pNotify;
   return TRUE;
}

UtlBoolean MpdPtAVT::setDtmfTerm(MprRecorder* pRecorder)
{
   mpRecorder = pRecorder;
   return TRUE;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

void MpdPtAVT::signalKeyDown(MpBufPtr pPacket)
{
   struct avtPacket* pAvt;
   unsigned int ts;
   OsStatus ret;

   pAvt = (struct avtPacket*) MpBuf_getStorage(pPacket);

   ts = pAvt->rh.timestamp;
   OsSysLog::add(FAC_MP, PRI_INFO, "MpdPtAVT(%p) Start Rcv Tone key=%d"
      " dB=%d TS=0x%08x\n", this, pAvt->key, pAvt->dB, ntohl(ts));
   OsSysLog::add(FAC_MP, PRI_INFO,
                 "MpdPtAVT::signalKeyDown mpRecorder = %p, mpNotify = %p",
                 mpRecorder, mpNotify);
   if (mpRecorder)
         mpRecorder->termDtmf(pAvt->key);

   if (NULL != mpNotify) {
      ret = mpNotify->signal((pAvt->key) << 16 | (mToneDuration & 0xffff));
         if (OS_SUCCESS != ret) {
            if (OS_ALREADY_SIGNALED == ret) {
               OsSysLog::add(FAC_MP, PRI_ERR,
                  "MpdPtAVT(%p) Signal Start returned OS_ALREADY_SIGNALED",
                  this);
            } else {
               OsSysLog::add(FAC_MP, PRI_ERR,
                  "MpdPtAVT(%p) Signal Start returned %d", this, ret);
            }
         }
   }
   mCurrentToneKey = pAvt->key;
   mCurrentToneSignature = ts;
   mToneDuration = 0;
}


void MpdPtAVT::signalKeyUp(MpBufPtr pPacket)
{
   struct avtPacket* pAvt;
   unsigned int samples;
   unsigned int ts;
   OsStatus ret;

   pAvt = (struct avtPacket*) MpBuf_getStorage(pPacket);
   ts = pAvt->rh.timestamp;
   samples = pAvt->samplesSwapped;
   samples = ntohs(samples);

   if ((-1) != mCurrentToneKey) {
      OsSysLog::add(FAC_MP, PRI_INFO, "MpdPtAVT(%p) Stop Rcv Tone key=%d"
         " dB=%d TS=0x%08x+%d last key=%d\n", this, pAvt->key, pAvt->dB,
         ntohl(mCurrentToneSignature), mToneDuration, mCurrentToneKey);
      mPrevToneSignature = mCurrentToneSignature;
      if (NULL != mpNotify) {
         ret = mpNotify->signal(0x80000000 |
                       (0x3fff0000 & (mCurrentToneKey << 16)) |
                       (mToneDuration & 0xffff));
         if (OS_SUCCESS != ret) {
            if (OS_ALREADY_SIGNALED == ret) {
               OsSysLog::add(FAC_MP, PRI_ERR,
                  "MpdPtAVT(%p) Signal Stop returned OS_ALREADY_SIGNALED",
                  this);
            } else {
               OsSysLog::add(FAC_MP, PRI_ERR,
                  "MpdPtAVT(%p) Signal Stop returned %d", this, ret);
            }
         }
      }
      if (mpRecorder) {
        mpRecorder->termDtmf(-1);
      }
   }
   mCurrentToneKey = -1;
   mCurrentToneSignature = 0;
   mToneDuration = 0;
}
