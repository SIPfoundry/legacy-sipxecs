//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#include "rtcp/RtcpConfig.h"

#undef WANT_RTCP_LOG_MESSAGES

// SYSTEM INCLUDES
#include <assert.h>

#ifdef __pingtel_on_posix__
#include <sys/types.h>
#include <netinet/in.h>
#endif

#ifdef _VXWORKS
#include <inetlib.h>
#endif

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsMsgQ.h"
#include "mp/MpMisc.h"
#include "mp/MpBuf.h"
#include "mp/MpConnection.h"
// #include "mp/NetInTask.h"
#include "mp/MprFromNet.h"
#include "mp/MprDejitter.h"
#include "mp/MpBufferMsg.h"
#ifdef INCLUDE_RTCP /* [ */
#include "rtcp/RTPHeader.h"
#endif /* INCLUDE_RTCP ] */
#include "os/OsEvent.h"
#include "os/OsMutex.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES

// CONSTANTS

// STATIC VARIABLE INITIALIZATIONS
const int MprFromNet::SSRC_SWITCH_MISMATCH_COUNT = 8;

#ifdef TESTING_ODD_LENGTH_PACKETS /* [ */
int MprFromNet::sPacketPad = 0;
#endif /* TESTING_ODD_LENGTH_PACKETS ] */

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprFromNet::MprFromNet(const UtlString& rName,
               MpConnection* pConn, int samplesPerFrame, int samplesPerSec)
:  MpResource(rName, 0, 0, 1, 1, samplesPerFrame, samplesPerSec),
   mMutex(OsMutex::Q_PRIORITY|OsMutex::INVERSION_SAFE),
   mRegistered(FALSE),
   mpDejitter(NULL),
   mpConnection(pConn),
#ifdef INCLUDE_RTCP /* [ */
   mpiRTCPDispatch(NULL),
   mpiRTPDispatch(NULL),
#else /* INCLUDE_RTCP ] [ */
   mRtcpCount(0),
#endif /* INCLUDE_RTCP ] */
   mPrevIP(0),
   mPrevPort(-1),
   mNumPushed(0),
   mNumWarnings(0),
   mPrefSsrc(0),
   mPrefSsrcValid(FALSE),
   mRtpDestIp(0),
   mRtpDestPort(0),
   mNumNonPrefPackets(0),
   mRtpRtcpMatchSsrc(0),
   mRtpRtcpMatchSsrcValid(FALSE),
   mRtpDestMatchIpOnlySsrc(0),
   mRtpDestMatchIpOnlySsrcValid(FALSE),
   mRtpOtherSsrc(0),
   mRtpOtherSsrcValid(FALSE)
{
#ifndef INCLUDE_RTCP /* [ */
   mInRtpHandle  = StartRtpSession(NULL, RTP_DIR_IN, (char) -1);
#endif /* INCLUDE_RTCP ] */
}

// Destructor
MprFromNet::~MprFromNet()
{
    resetSockets();

#ifdef INCLUDE_RTCP /* [ */
//  Release the references held to the RTP and RTCP Dispatchers used for
//  routing packets to the RTCP component
    if(mpiRTPDispatch)
        mpiRTPDispatch->Release();
    if(mpiRTCPDispatch)
        mpiRTCPDispatch->Release();
#else /* INCLUDE_RTCP ] [ */
   if (NULL != mInRtpHandle)  FinishRtpSession(mInRtpHandle);
   mInRtpHandle  = NULL;
#endif /* INCLUDE_RTCP ] */


}

/* ============================ MANIPULATORS ============================== */

// Handles a SET_SOCKETS message sent to this resource, to set the inbound
// RTP and RTCP sockets.
// Returns the result of attempting to queue the message to the NetInTask
OsStatus MprFromNet::setSockets(OsSocket& rRtpSocket, OsSocket& rRtcpSocket)
{
   OsStatus res;
   OsEvent notify;

   mMutex.acquire();

   resetSockets();
   res = addNetInputSources(&rRtpSocket, &rRtcpSocket, this, &notify);
   assert(res == OS_SUCCESS);
   notify.wait();
   mRegistered = TRUE;

   mMutex.release();
   return OS_SUCCESS;
}

// Handles a RESET_SOCKETS message sent to this resource, to deregister
// the inbound RTP and RTCP sockets.
// Returns the result of attempting to queue the message to the NetInTask
OsStatus MprFromNet::resetSockets(void)
{
   mMutex.acquire();
   if (mRegistered) {
      OsStatus res;
      OsEvent notify;

      mRegistered = FALSE;
      res = removeNetInputSources(this, &notify);
      if (res == OS_SUCCESS) {
         notify.wait();
      } else {
         assert(FALSE);
      }
   }
   mMutex.release();
   return OS_SUCCESS;
}

#ifdef TESTING_ODD_LENGTH_PACKETS /* [ */
int MprFromNet::setPacketPad(int value)
{
   int ret = sPacketPad;
   sPacketPad = value;
   return ret;
}
#endif /* TESTING_ODD_LENGTH_PACKETS ] */

#ifndef INCLUDE_RTCP /* [ */
OsStatus MprFromNet::getRtcpStats(MprRtcpStats& stats)
{
   stats.ssrc = mInRtpHandle->ssrc;
   stats.seqNumCycles = mInRtpHandle->cycles;
   stats.highSeqNum = mInRtpHandle->seq;
   return OS_SUCCESS;
}

OsStatus MprFromNet::rtcpStats(struct rtpHeader* rtpH)
{
   if (mInRtpHandle->ssrc != rtpH->ssrc) {
      const char* name = getName();
      static int firstfew = 0;

      if (firstfew++ < 20)
      {
         osPrintf("%s: New SSRC (0x%08X) detected\n", name, rtpH->ssrc);
      }
      mInRtpHandle->ssrc = rtpH->ssrc;
      mInRtpHandle->seq = rtpH->seq - 1;
      mInRtpHandle->cycles = 0;
      mRtcpCount = 0;
   }
   if (mInRtpHandle->seq < rtpH->seq) {
      mInRtpHandle->seq = rtpH->seq;
   } else {
      if ((mInRtpHandle->seq > 0xc000) && (rtpH->seq < 0x4000)) {
         mInRtpHandle->cycles++;
      }
   }
   return OS_SUCCESS;
}
#endif /* INCLUDE_RTCP ] */

int MprFromNet::adjustBufferForRtp(MpBufPtr buf)
{
   struct rtpHeader* pRtpH;
   int version;
   int padded;
   int ccs;
   int padBytes;
   int packetLength;
   int offset;
   int payloadType;
   int xBit;

   pRtpH = (struct rtpHeader*) MpBuf_getStorage(buf);
   version = (pRtpH->vpxcc & 0xC0);

   if (version != 0x80) {
      // 0x80 means V=2, the only RTP spec we support.  Other values are
      // probably indicative of bad packets.  Ignore them by returning a
      // bogus payload type.
      return 255;
   }

   payloadType = (pRtpH->mpt & 0x7f);
   padded = (pRtpH->vpxcc) & 0x20;
   xBit   = (pRtpH->vpxcc) & 0x10;
   ccs    = (pRtpH->vpxcc) & 0x0f;
   packetLength = MpBuf_getContentLen(buf); /* RTP packet length */
   if (padded) {
      padBytes = *(((char *) pRtpH) + (packetLength - 1));
      if (0 != ((~3) & padBytes)) {
         padBytes = 0;
      }
   } else {
      padBytes = 0;
   }
   packetLength -= padBytes;

   pRtpH->vpxcc &= ~0x20;
   offset = sizeof(struct rtpHeader) + (4 * ccs);
   if (0 != xBit) { // Check for RTP Header eXtension
      int xLen; // number of 32-bit words after extension header
      short* pXhdr; // pointer to extension header, after CSRC list
      pXhdr = (short*) (((intptr_t)pRtpH) + sizeof(struct rtpHeader) + offset + 2);
      xLen = ntohs(*pXhdr);
      offset += (sizeof(int) * (xLen + 1));
   }
   MpBuf_setOffset(buf, offset);
#ifdef TESTING_ODD_LENGTH_PACKETS /* [ */
   MpBuf_setNumSamples(buf, (packetLength - offset) + sPacketPad);
#else /* TESTING_ODD_LENGTH_PACKETS ] [ */
   MpBuf_setNumSamples(buf, (packetLength - offset));
#endif /* TESTING_ODD_LENGTH_PACKETS ] */
   MpBuf_setContentLen(buf, packetLength);
   return payloadType;
}

/**************************************************************************
 *            HACK   HACK   HACK   $$$
 *
 * Here is a temporary workaround for a memory leak in the RTCP code.
 **************************************************************************/

static volatile int DoForwardRtcp = 0;
static volatile int RtcpDiscards = 0;

#ifdef _VXWORKS /* [ */

static volatile int TotalRtcpDiscards = 0;

extern "C" {
extern int SFR(); // Show Forward RTCP status
extern int NFR(); // Do Not Forward RTCP
extern int FR();  // Do Forward RTCP
}

static int ForwardRtcp(int YesNo)
{
   int save = DoForwardRtcp;
   DoForwardRtcp = YesNo;
   TotalRtcpDiscards += RtcpDiscards;
   printf("Discarded %d RTCP packets (%d total)\n",
      RtcpDiscards, TotalRtcpDiscards);
   RtcpDiscards = 0;
   return save;
}

int SFR() {return ForwardRtcp(DoForwardRtcp);}
int NFR() {return ForwardRtcp(0);}
int FR() {return ForwardRtcp(1);}

#endif /* _VXWORKS ] */

/**************************************************************************/

// Take in a buffer from the NetIn task
OsStatus MprFromNet::pushPacket(MpBufPtr buf,
                      int rtpOrRtcp, struct in_addr* fromIP, int fromPort)
{
    OsStatus ret = OS_SUCCESS;
    int      payloadType;
    int      thisSsrc;

#ifdef INCLUDE_RTCP /* [ */
    CRTPHeader   oRTPHeader;
#endif /* INCLUDE_RTCP ] */

    mNumPushed++;
    if (0 == (mNumPushed & ((1<<11)-1))) mNumWarnings = 0; // every 2048

    if (MpBufferMsg::AUD_RTP_RECV == rtpOrRtcp) {

        MpBuf_setFormat(buf, MP_FMT_RTPPKT);
        payloadType = adjustBufferForRtp(buf);

        if (NULL == mpConnection->mapPayloadType(payloadType)) {
            // just ignore it!
            MpBuf_delRef(buf);
            return ret;
        }

        thisSsrc = extractSsrc(buf);
        if (!mPrefSsrcValid) {
            setPrefSsrc(thisSsrc);
        }

        if (thisSsrc == getPrefSsrc()) {
            mNumNonPrefPackets = 0;
        } else {
            if (mNumWarnings++ < 20) {
                UtlString Old(""), New("");
                struct in_addr t;
                t.s_addr = mRtpDestIp;
                OsSocket::inet_ntoa_pt(t, Old);
                OsSocket::inet_ntoa_pt(*fromIP, New);
                osPrintf("   pushPacket: Pref:0x%X, rtpDest=%s:%d,\n"
                    "       this:0x%X (src=%s:%d)\n",
                    getPrefSsrc(), Old.data(), mRtpDestPort,
                    thisSsrc, New.data(), fromPort);
            }
            if ((fromIP->s_addr == mRtpDestIp) && (fromPort == mRtpDestPort)) {
                setPrefSsrc(thisSsrc);
            } else if (mRtpRtcpMatchSsrcValid &&
                               (thisSsrc == mRtpRtcpMatchSsrc)) {
                setPrefSsrc(thisSsrc);
            } else {
                mNumNonPrefPackets++;
                if (fromIP->s_addr == mRtpDestIp) {
                    mRtpDestMatchIpOnlySsrc = thisSsrc;
                    mRtpDestMatchIpOnlySsrcValid = TRUE;
                } else {
                    mRtpOtherSsrc = thisSsrc;
                    mRtpOtherSsrcValid = TRUE;
                }
                if (SSRC_SWITCH_MISMATCH_COUNT <= mNumNonPrefPackets) {
                    setPrefSsrc(mRtpDestMatchIpOnlySsrcValid ?
                        mRtpDestMatchIpOnlySsrc : mRtpOtherSsrc);
                }
            }
            MpBuf_delRef(buf);
            return ret;
        }

        if ((mPrevIP != fromIP->s_addr) || (mPrevPort != fromPort)) {
            if (mNumWarnings++ < 20) {
                UtlString Old(""), New("");
                struct in_addr t;
                t.s_addr = mPrevIP;
                OsSocket::inet_ntoa_pt(t, Old);
                OsSocket::inet_ntoa_pt(*fromIP, New);
/*
                osPrintf("MprFromNet(%d): SrcIP changed"
                    " from '%s:%d' to '%s:%d'\n", mNumPushed, Old.data(),
                    mPrevPort, New.data(), fromPort);
*/
            }
            mPrevIP = fromIP->s_addr;
            mPrevPort = fromPort;
        }

#ifdef INCLUDE_RTCP /* [ */
//      bump the reference count so the buffer cannot unexpectedly
//      go away before being processed by RTCP
        MpBuf_addRef(buf);
#else /* INCLUDE_RTCP ] [ */
        rtcpStats((struct rtpHeader*) MpBuf_getStorage(buf));
#endif /* INCLUDE_RTCP ] */

        ret = getMyDejitter()->pushPacket(buf);

#ifdef INCLUDE_RTCP /* [ */
        // This is the logic that forwards RTP packets to the RTCP subsystem
        // for Receiver Report calculations.

        // Set RTP Header Received Timestamp
        {
            unsigned long t = (unsigned long)MpBuf_getOsTC(buf);
            double x;
            x = ((((double) t) * 8000.) / 3686400.);
            t = (unsigned long) x;
            oRTPHeader.SetRecvTimestamp(t);
        }

        // Parse the packet stream into an RTP header
        oRTPHeader.ParseRTPHeader((unsigned char *)MpBuf_getStorage(buf));

        // Dispatch packet to RTCP Render object
        mpiRTPDispatch->ForwardRTPHeader((IRTPHeader *)&oRTPHeader);

        // release our reference to the RTP buffer
        MpBuf_delRef(buf);
#endif /* INCLUDE_RTCP ] */

    } else {  // RTCP packet
#ifdef DUMP_RTCP_PACKETS /* [ */
        const char*       name;

        name = getName();
        osPrintf("%s: RTCP packet received, length = %d\n",
                                    name, MpBuf_getNumSamples(buf));
#endif /* DUMP_RTCP_PACKETS ] */

        MpBuf_setFormat(buf, MP_FMT_RTCPPKT);

#ifdef INCLUDE_RTCP /* [ */
//      Dispatch the RTCP data packet to the RTCP Source object registered
/**************************************************************************
 *            HACK   HACK   HACK   $$$
 *
 * Here is a temporary workaround for a memory leak in the RTCP code.
 **************************************************************************/
        if (DoForwardRtcp) {
            mpiRTCPDispatch->ProcessPacket(
                      (unsigned char *)MpBuf_getStorage(buf),
                      (unsigned long)MpBuf_getContentLen(buf));
        } else {
            RtcpDiscards++;
        }
#endif /* INCLUDE_RTCP ] */
        // release our [the only] reference to the RTCP buffer
        MpBuf_delRef(buf);

    }
    return ret;
}

void MprFromNet::setMyDejitter(MprDejitter* pDJ)
{
   mpDejitter = pDJ;
}

void MprFromNet::setDestIp(OsSocket& newDest)
{
   struct in_addr t;
   newDest.getRemoteHostIp(&t, &mRtpDestPort);
   mRtpDestIp = t.s_addr;
   {
      int a, b, c, d;
      a = (mRtpDestIp >>  0) & 0xff;
      b = (mRtpDestIp >>  8) & 0xff;
      c = (mRtpDestIp >> 16) & 0xff;
      d = (mRtpDestIp >> 24) & 0xff;
/*
      osPrintf("MprFromNet::setDestIp: DestIP=0x%08lX, Dest = %d.%d.%d.%d:%d\n",
         mRtpDestIp, a, b, c, d, mRtpDestPort);
*/
   }
}

/* ============================ ACCESSORS ================================= */

MprDejitter* MprFromNet::getMyDejitter(void)
{
   assert(NULL != mpDejitter);
   return mpDejitter;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

int MprFromNet::extractSsrc(MpBufPtr buf)
{
   rtpHeader* pR = (struct rtpHeader*) MpBuf_getStorage(buf);
   return pR->ssrc;
}

int MprFromNet::getPrefSsrc()
{
   return mPrefSsrc;
}

int MprFromNet::setPrefSsrc(int newSsrc)
{
   // osPrintf("MprFromNet::setPrefSsrc(0x%X) -- was 0x%X\n", newSsrc, mPrefSsrc);
   mPrefSsrc = newSsrc;
   mPrefSsrcValid = TRUE;
   mNumNonPrefPackets = 0;
   mRtpRtcpMatchSsrcValid = FALSE;
   mRtpDestMatchIpOnlySsrcValid = FALSE;
   mRtpOtherSsrcValid = FALSE;
   return 0;
}

UtlBoolean MprFromNet::doProcessFrame(MpBufPtr inBufs[],
                                    MpBufPtr outBufs[],
                                    int inBufsSize,
                                    int outBufsSize,
                                    UtlBoolean isEnabled,
                                    int samplesPerFrame,
                                    int samplesPerSecond)
{
   return TRUE;
}

/* ============================ FUNCTIONS ================================= */
