//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _MprDejitter_h_
#define _MprDejitter_h_

#include "mp/MpMisc.h"

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "mp/MpResource.h"
#include "mp/MprFromNet.h"

// DEFINES
#define DEBUGGING_LATENCY
#undef DEBUGGING_LATENCY

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MpConnection;

//:The "Dejitter" media processing resource
class MprDejitter : public MpResource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

#ifdef DEBUGGING_LATENCY /* [ */
   enum { MAX_RTP_PACKETS = 64};  // MUST BE A POWER OF 2, AND SHOULD BE >3
        // 20 Apr 2001 (HZM): Increased from 16 to 64 for debugging purposes.
#else /* DEBUGGING_LATENCY ] [ */
   enum { MAX_RTP_PACKETS = 16};  // MUST BE A POWER OF 2, AND SHOULD BE >3
#endif /* DEBUGGING_LATENCY ] */

   enum { GET_ALL = 1 }; // get all packets, ignoring timestamps.  For NetEQ

/* ============================ CREATORS ================================== */

   MprDejitter(const UtlString& rName, MpConnection* pConn,
      int samplesPerFrame, int samplesPerSec);
     //:Constructor

   virtual
   ~MprDejitter();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   OsStatus pushPacket(MpBufPtr pRtp);
     //:Add a buffer containing an incoming RTP packet to the dejitter pool

   MpBufPtr pullPacket(void);
     //:Submit all RTP packets to the Jitter Buffer.

   OsStatus getPacketsInfo(int& nPackets,
                           unsigned int& lowTimestamp);
     //:Return status info on current backlog.

   void dumpState();

/* ============================ ACCESSORS ================================= */
public:
   static unsigned short getSeqNum(MpBufPtr pRtp);
   static unsigned int getTimestamp(MpBufPtr pRtp);
   static unsigned int getPayloadType(MpBufPtr pRtp);

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   MpBufPtr      mpPackets[MAX_RTP_PACKETS];
   OsBSem        mRtpLock;
   int           mNumPackets;
   int           mNumDiscarded;

#ifdef DEJITTER_DEBUG /* [ */
   // These are only used if DEJITTER_DEBUG is defined, but I am
   // leaving them in all the time so that changing that definition
   // does not require recompiling more things...
   int           mPullCount;
   int           mLatencyMax;
   int           mLatencyMin;
   int           mPrevNumPackets;
   int           mPrevPullTime;
#endif /* DEJITTER_DEBUG ] */

   /* end of Dejitter handling variables */

   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                                    MpBufPtr outBufs[],
                                    int inBufsSize,
                                    int outBufsSize,
                                    UtlBoolean isEnabled,
                                    int samplesPerFrame=80,
                                    int samplesPerSecond=8000);

   MprDejitter(const MprDejitter& rMprDejitter);
     //:Copy constructor (not implemented for this class)

   MprDejitter& operator=(const MprDejitter& rhs);
     //:Assignment operator (not implemented for this class)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprDejitter_h_
