//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////

//The averaged latency of packets in dejitter buffer is calculated in method
//PullPacket( ) for the purpose of dejitter buffer
//backlog control (or called jitter control) by the decoder in down stream.
//The decoder will look at the latency at certain frequency to make
//the decision. -Brian Puh
//

#define DEJITTER_DEBUG
#undef  DEJITTER_DEBUG

// SYSTEM INCLUDES
#include <assert.h>
#if defined(_VXWORKS) || defined(__pingtel_on_posix__)
#include <sys/types.h>
#include <netinet/in.h>
#endif
#ifdef WIN32
#include <winsock2.h>
#endif

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "mp/MpMisc.h"
#include "mp/MpBuf.h"
#include "mp/MpConnection.h"
#include "mp/MprDejitter.h"
#include "mp/MprFromNet.h"
#include "mp/NetInTask.h"
/* for dejitter handling purpose */
#include "mp/dmaTask.h"
#include "mp/MpMediaTask.h"

//===========================================================
// Clock tick: for packet detaining period calculation
extern volatile int* pOsTC;
//===========================================================

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES

// CONSTANTS

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

unsigned short MprDejitter::getSeqNum(MpBufPtr pRtp)
{
   assert(NULL != pRtp);
   return ntohs(((struct rtpHeader*) MpBuf_getStorage(pRtp))->seq);
}

unsigned int MprDejitter::getTimestamp(MpBufPtr pRtp)
{
   assert(NULL != pRtp);
   return ntohl(((struct rtpHeader*) MpBuf_getStorage(pRtp))->timestamp);
}

unsigned int MprDejitter::getPayloadType(MpBufPtr pRtp)
{
   assert(NULL != pRtp);
   return (0x7f & (((struct rtpHeader*) MpBuf_getStorage(pRtp))->mpt));
}

/* ============================ CREATORS ================================== */

// Constructor
MprDejitter::MprDejitter(const UtlString& rName, MpConnection* pConn,
                           int samplesPerFrame, int samplesPerSec)
:  MpResource(rName, 1, 1, 1, 1, samplesPerFrame, samplesPerSec),
   mRtpLock(OsBSem::Q_FIFO, OsBSem::FULL),
   mNumPackets(0),
   mNumDiscarded(0)
   /* for Dejitter handling */
#ifdef DEJITTER_DEBUG /* [ */
   , mPullCount(0),
   mLatencyMax(0x80000000),
   mLatencyMin(0x7FFFFFFF)
#endif /* DEJITTER_DEBUG ] */
{
   memset(mpPackets, 0, MAX_RTP_PACKETS * sizeof(MpBufPtr));
}

// Destructor
MprDejitter::~MprDejitter()
{
   int i;

   mRtpLock.acquire();
   for (i=0; i<MAX_RTP_PACKETS; i++) {
      MpBuf_delRef(mpPackets[i]);
      mpPackets[i] = NULL;
   }
   mRtpLock.release();
}
/* ============================ MANIPULATORS ============================== */

//Add a buffer containing an incoming RTP packet to the dejitter pool
OsStatus MprDejitter::pushPacket(MpBufPtr pRtp)
{
   int index;

   MpBuf_touch(pRtp);
   mRtpLock.acquire();
   index = getSeqNum(pRtp) % MAX_RTP_PACKETS;
   if (NULL != mpPackets[index]) {
      mNumDiscarded++;
#ifdef MP_STREAM_DEBUG /* [ */
      if (mNumDiscarded < 40) {
         osPrintf("Dej: discard#%d Seq: %d -> %d at 0x%X\n",
            mNumDiscarded,
            getSeqNum(mpPackets[index]),
            getSeqNum(pRtp), *pOsTC);
      }
#endif /* MP_STREAM_DEBUG ] */
      MpBuf_delRef(mpPackets[index]);
      mpPackets[index] = NULL;
      mNumPackets--;
   }
   mpPackets[index] = pRtp;
   mNumPackets++;
   mRtpLock.release();

   return OS_SUCCESS;
}

// Get a pointer to the next RTP packet, or NULL if none is available.

MpBufPtr MprDejitter::pullPacket(void)
{
   MpBufPtr found = NULL;
   MpBufPtr cur;
   int curSeq;
   int first = -1;
   int firstSeq = 0;            // dummy value
   int i;

   mRtpLock.acquire();

   // Find smallest seq number
   for (i=0; i<MAX_RTP_PACKETS; i++)
   {
       cur = mpPackets[i];
       if (cur != NULL)
       {
           curSeq = getSeqNum(mpPackets[i]);
           if (first == -1 || curSeq < firstSeq)
           {
               first = i;
               firstSeq = curSeq;
           }
       }
   }

   if (-1 != first)
   {
      found = mpPackets[first];
      mpPackets[first] = NULL;
      mNumPackets--;
      MpBuf_touch(found);
   }

   mRtpLock.release();

   return found;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

UtlBoolean MprDejitter::doProcessFrame(MpBufPtr inBufs[],
                                    MpBufPtr outBufs[],
                                    int inBufsSize,
                                    int outBufsSize,
                                    UtlBoolean isEnabled,
                                    int samplesPerFrame,
                                    int samplesPerSecond)
{
   UtlBoolean ret = FALSE;

   if (!isEnabled) return TRUE;

   if ((1 != inBufsSize) || (1 != outBufsSize))
      ret = FALSE;
   else
   {
      *outBufs = *inBufs;
      *inBufs = NULL;
      ret = TRUE;
   }
   return ret;
}

/* ============================ FUNCTIONS ================================= */
#ifdef DEJITTER_DEBUG /* [ */
int dejitterdebug(int Flag)
{
    int save = iShowDejitterInfoFlag;
    if( Flag != 0) {
        Flag = 1;
    }
    iShowDejitterInfoFlag = Flag;
    return (save);
}
int ShowDejitterInfo(int iFlag) {
   return dejitterdebug(iFlag);
}
#endif /* DEJITTER_DEBUG ] */
