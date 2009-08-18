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
#undef DROP_SOME_PACKETS

// SYSTEM INCLUDES
#include <assert.h>

#ifdef _VXWORKS /* [ */
#include <iosLib.h>
#include <netinet/in.h>
#endif /* _VXWORKS ] */

#ifdef WIN32 /* [ */
#include <winsock2.h>
#include <io.h>
#endif /* WIN32 ] */

#ifdef __pingtel_on_posix__ /* [ */
#include <sys/types.h>
#include <netinet/in.h>
#endif /* __pingtel_on_posix__ ] */

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "mp/MpMisc.h"
#include "mp/MpBuf.h"
#include "mp/MprToNet.h"
#include "mp/NetInTask.h"
#include "mp/MprFromNet.h"
#include "mp/dmaTask.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
extern volatile int* pOsTC;

// CONSTANTS
static const int NO_WAIT = 0;

// STATIC VARIABLE INITIALIZATIONS

#ifdef ENABLE_PACKET_HACKING /* [ */
int MprToNet::sDebug1 = 0;
int MprToNet::sDebug2 = 0;
int MprToNet::sDebug3 = 0;
int MprToNet::sDebug4 = 0;
int MprToNet::sDebug5 = 0;
#endif /* ENABLE_PACKET_HACKING ] */

static int doPadRtp = 1;

#ifdef _VXWORKS /* [ */

extern "C" {
extern int PadRtp();
extern int PR();
extern int NoPadRtp();
extern int NPR();
}

static int DoPadRtp(int flag)
{
   int save = doPadRtp;
   doPadRtp = flag;
   return save;
}

int PadRtp() {return DoPadRtp(1);}
int PR() {return DoPadRtp(1);}
int NoPadRtp() {return DoPadRtp(0);}
int NPR() {return DoPadRtp(0);}
#endif /* _VXWORKS ] */

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprToNet::MprToNet(const UtlString& rName,
                           int samplesPerFrame, int samplesPerSec)
:  MpResource(rName, 1, 1, 0, 0, samplesPerFrame, samplesPerSec),
#ifdef DEBUG /* [ */
   mRtpSampleCounter(0),
#endif /* DEBUG ] */
   mpFromNetPal(NULL),
   mRtcpPackets(0),
   mRtcpFrameCount(0),
   mRtcpFrameLimit(500),
   mSSRC(0),
   mpRtpSocket(NULL),
   mpRtcpSocket(NULL),
   mNumRtpWriteErrors(0),
   mNumRtcpWriteErrors(0)
#ifdef INCLUDE_RTCP /* [ */
   , mpiRTPAccumulator(NULL)
#endif /* INCLUDE_RTCP ] */

{
}

// Destructor
MprToNet::~MprToNet()
{
#ifdef INCLUDE_RTCP /* [ */

//  Release the reference held to the RTP Accumulator interface used to
//  RTP outbound stream statistics
    if(mpiRTPAccumulator)
        mpiRTPAccumulator->Release();
#endif /* INCLUDE_RTCP ] */
    if (0 != mNumRtpWriteErrors) {
        osPrintf("MprToNet: %d network write errors on RTP socket!\n",
            mNumRtpWriteErrors);
    }
    if (0 != mNumRtcpWriteErrors) {
        osPrintf("MprToNet: %d network write errors on RTCP socket!\n",
            mNumRtcpWriteErrors);
    }
}

/* ============================ MANIPULATORS ============================== */

// Sends a SET_SOCKETS message to this resource to set the outbound
// RTP and RTCP sockets.
// Returns the result of attempting to queue the message to this resource.
OsStatus MprToNet::setSockets(OsSocket& rRtpSocket, OsSocket& rRtcpSocket)
{
   MpFlowGraphMsg msg(SET_SOCKETS, this, &rRtpSocket, &rRtcpSocket, 0, 0);
   OsStatus       res;

   res = postMessage(msg);
   return res;
}

// Sends a RESET_SOCKETS message to this resource to clear the outbound
// RTP and RTCP sockets.
// Returns the result of attempting to queue the message to this resource.
OsStatus MprToNet::resetSockets(void)
{
   MpFlowGraphMsg msg(RESET_SOCKETS, this, NULL, NULL, 0, 0);
   OsStatus       res;

   res = postMessage(msg);
   return res;
}

void MprToNet::setSSRC(int iSSRC)
{
   mSSRC = htonl(iSSRC);
   mSeqNum = rand_timer32() | 0xfc00;
#ifndef INCLUDE_RTCP /* [ */
   mTimestampDelta = rand_timer32();
#endif /* INCLUDE_RTCP ] */
}

#ifdef INCLUDE_RTCP /* [ */
void MprToNet::setRTPAccumulator(ISetSenderStatistics *piRTPAccumulator)
{
   mpiRTPAccumulator = piRTPAccumulator;
   mTimestampDelta = rand_timer32();
   mpiRTPAccumulator->SetRTPTimestamp(mTimestampDelta);
}

#endif /* INCLUDE_RTCP ] */

#ifdef DEBUG /* [ */
static int NumberOfPuts = 0;
#endif /* DEBUG ] */

#ifdef DEBUG /* [ */
int NumberOfRtpWrites = 0;
#endif /* DEBUG ] */

#define RTP_BAD_DIRECTION 0x12300001

struct __rtpPacket {
        struct rtpHeader s;
        char b[1];
};
typedef struct __rtpPacket rtpPacket;
typedef struct __rtpPacket *rtpPacketPtr;

#undef DEBUG_TIMESTAMP
#ifdef DEBUG_TIMESTAMP /* [ */
static int startTimeStamp = 0;

UINT hack_timer32(void)
{
   return startTimeStamp;
}

int setTimeStamp(int x) {
   int save = hack_timer32();
   startTimeStamp = x;
   return save;
}

#define rand_timer32 hack_timer32
#endif /* DEBUG_TIMESTAMP ] */

#ifdef DROP_SOME_PACKETS /* [ */
static int dropLimit = 1<<30;
static int dropCount = 0;

extern "C" {
extern int dropEvery(int limit);
}

int dropEvery(int limit)
{
   int save = dropLimit;
   if (limit < 0) limit = 1<<30;
   if (limit > 0) {
      dropLimit = limit;
      dropCount = 0;
   }
   return save;
}
#endif /* DROP_SOME_PACKETS ] */

int MprToNet::writeRtp(int payloadType, UtlBoolean markerState,
   unsigned char* payloadData, int payloadOctets, unsigned int timestamp,
   void* csrcList)
{
        struct rtpHeader* ph;
        int pad, l;
        int sendret;
        int len;

        ph = (struct rtpHeader*) (payloadData - sizeof(struct rtpHeader));//$$$
               //$$$ Line above will need to adjust for CSRC list someday...
        len = payloadOctets;
        Lprintf("Enter rW: %d, %d\n", len, timestamp, 0,0,0,0);

        if (NULL == mpRtpSocket) {return 0;}
        mSeqNum++;

#ifdef DEBUG /* [ */
        if (NumberOfRtpWrites++ < 10)
            Zprintf("rW: %d, %d, %d\n", len, ts, h->mpt, 0,0,0);
#endif /* DEBUG ] */

        ph->vpxcc = 2<<6; // h->vpxcc;
        ph->mpt = (payloadType & 0x7f) | (markerState ? 0x80 : 0);
        ph->seq = htons(0xffff&mSeqNum);
        ph->timestamp = htonl(mTimestampDelta + timestamp);
        ph->ssrc = mSSRC;

#ifdef ENABLE_PACKET_HACKING /* [ */
        adjustRtpPacket(ph);
#endif /* ENABLE_PACKET_HACKING ] */

        pad = doPadRtp ? ((4 - (3 & len)) & 3) : 0;

        switch (pad) {
        case 3:
                payloadData[len+1] = 0;         /* fall through */
        case 2:
                payloadData[len] = 0;           /* fall through */
        case 1:
                payloadData[len+pad-1] = pad;
                ph->vpxcc |= (1<<5);
                Lprintf("writeRtp: adding %d pad bytes\n", pad, 0,0,0,0,0);
                                                  /* fall through */
        case 0:
                break;
        }

#ifdef INCLUDE_RTCP /* [ */
// Update the Accumulated statistics kept for an inbound RTP packet.
// These statistics comprise a Sender Report that is sent out periodically
// to the originating site
        mpiRTPAccumulator->IncrementCounts(len);
#endif /* INCLUDE_RTCP ] */

        l = sizeof(struct rtpHeader) + len + pad;

#ifdef DROP_SOME_PACKETS /* [ */
        if (dropCount++ == dropLimit) {
            dropCount = 0;
            sendret = l;
        } else {
            sendret = mpRtpSocket->write((char *) ph, l);
        }
#else /* DROP_SOME_PACKETS ] [*/
        sendret = mpRtpSocket->write((char *) ph, l);
#endif /* DROP_SOME_PACKETS ] */

        if (l != sendret) {
            if (5 > mNumRtpWriteErrors++) {
                Zprintf("Exit rW: send(0x%p 0x%p %d) returned %d,"
                    " errno=%d (at %d)\n",
                    mpRtpSocket, ph, l, sendret, errno, *pOsTC);
            }

            switch (errno) {
            /* insert other benign errno values here */
            case 0:
            case 55: // Network disconnected, continue and hope it comes back
                break;
            default:
                // close(fd);  MAYBE: mpRtpSocket->close() ?
                // mpRtpSocket = NULL;
                break;
            }

        }
        return (l == sendret) ? len : sendret;
}

#ifdef ENABLE_PACKET_HACKING /* [ */
extern "C" {
extern int sTRA(int a, int b, int c);
extern int setToRtpAdjustment(int a, int b, int c);
};

int setToRtpAdjustment(int ssrc, int seq, int ts)
{
   MprToNet::sDebug1 = ssrc;
   MprToNet::sDebug2 = seq;
   MprToNet::sDebug3 = ts;
   return 0;
}

int sTRA(int a, int b, int c) {return setToRtpAdjustment(a,b,c);}
#endif /* ENABLE_PACKET_HACKING ] */

/* ============================ ACCESSORS ================================= */

OsStatus MprToNet::setRtpPal(MprFromNet* pal)
{
   mpFromNetPal = pal;
   return OS_SUCCESS;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

#ifdef ENABLE_PACKET_HACKING /* [ */
void MprToNet::adjustRtpPacket(struct rtpHeader* rp)
{
   struct rtpHeader rh;

   memcpy((char *) &rh, (char *) rp, sizeof(struct rtpHeader));
   // rh.vpxcc = rh.vpxcc;
   // rh.mpt = rh.mpt;
   rh.ssrc = ntohl(rh.ssrc) - sDebug1;
   rh.seq = ntohs(rh.seq) - sDebug2;
   rh.timestamp = ntohl(rh.timestamp) - sDebug3;
   rh.ssrc = htonl(rh.ssrc);
   rh.seq = htons(rh.seq);
   rh.timestamp = htonl(rh.timestamp);
   memcpy((char *) rp, (char *) &rh, sizeof(struct rtpHeader));
}
#endif /* ENABLE_PACKET_HACKING ] */

UtlBoolean MprToNet::doProcessFrame(MpBufPtr inBufs[],
                                    MpBufPtr outBufs[],
                                    int inBufsSize,
                                    int outBufsSize,
                                    UtlBoolean isEnabled,
                                    int samplesPerFrame,
                                    int samplesPerSecond)
{
   return TRUE;
}

// Handle messages for this resource.
UtlBoolean MprToNet::handleMessage(MpFlowGraphMsg& rMsg)
{

   if (rMsg.getMsg() == SET_SOCKETS)
   {
      mpRtpSocket  = (OsSocket*) rMsg.getPtr1();
      mpRtcpSocket = (OsSocket*) rMsg.getPtr2();
      return TRUE;
   }
   if (rMsg.getMsg() == RESET_SOCKETS)
   {
      mpRtpSocket  = NULL;
      mpRtcpSocket = NULL;
      return TRUE;
   }
   else
      return MpResource::handleMessage(rMsg);
}

/* ============================ FUNCTIONS ================================= */
