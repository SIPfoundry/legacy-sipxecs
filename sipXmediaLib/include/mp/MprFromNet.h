//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _MprFromNet_h_
#define _MprFromNet_h_

#include "rtcp/RtcpConfig.h"

// SYSTEM INCLUDES
#ifdef _WIN32 /* [ */
#include <winsock.h>
#elif defined(__pingtel_on_posix__)
#include <sys/types.h>
#endif /* _WIN32 ] */

// APPLICATION INCLUDES

class MprDejitter;
class MpConnection;

// #include "os/OsMsgQ.h"
#include "os/OsDefs.h"
#include "os/OsSocket.h"
#include "mp/MpResource.h"
#include "mp/NetInTask.h"
#ifdef INCLUDE_RTCP /* [ */
#include "rtcp/IRTPDispatch.h"
#include "rtcp/INetDispatch.h"
#endif /* INCLUDE_RTCP ] */

// DEFINES
// MACROS
#undef TESTING_ODD_LENGTH_PACKETS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// typedef struct rtpSession *rtpHandle;
// typedef struct rtcpSession *rtcpHandle;

// FORWARD DECLARATIONS

//:The "From Network" media processing resource
class MprFromNet : public MpResource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   MprFromNet(const UtlString& rName, MpConnection* pConn,
                                 int samplesPerFrame, int samplesPerSec);
     //:Constructor

   virtual
   ~MprFromNet();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   OsStatus setSockets(OsSocket& rRtpSocket, OsSocket& rRtcpSocket);
     //:Sends a SET_SOCKETS message to this resource to set the inbound
     //:RTP and RTCP sockets.
     // returns OS_SUCCESS, unless unable to queue message.

   OsStatus resetSockets();
     //:Sends a RESET_SOCKETS message to this resource to deregister the
     //:inbound RTP and RTCP sockets.
     // returns OS_SUCCESS, unless unable to queue message.

   OsStatus pushPacket(MpBufPtr buf, int rtpOrRtcp, struct in_addr* I, int P);
     //:Take in a buffer from the NetIn task

   void setMyDejitter(MprDejitter* newDJ);
     //:Inform this object of its sibling dejitter object.

   void setDestIp(OsSocket& newDest);
     //:Inform this object of its sibling ToNet's destination

/* ============================ ACCESSORS ================================= */

#ifdef INCLUDE_RTCP /* [ */
// These accessors were added by DMG to allow a Connection to access and modify
// rtp and rtcp stream informations
   void setDispatchers(IRTPDispatch *piRTPDispatch, INetDispatch *piRTCPDispatch);

#else /* INCLUDE_RTCP ] [ */
   OsStatus getRtcpStats(MprRtcpStats& stats);
     //:retrieve the RR info needed to complete an RTCP packet
#endif /* INCLUDE_RTCP ] */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   OsMutex          mMutex;
   UtlBoolean        mRegistered;
   MprDejitter*     mpDejitter;
   MpConnection*    mpConnection;
#ifdef INCLUDE_RTCP /* [ */
   INetDispatch*    mpiRTCPDispatch;
   IRTPDispatch*    mpiRTPDispatch;
#else /* INCLUDE_RTCP ] [ */
   rtpHandle        mInRtpHandle;
   int              mRtcpCount;
#endif /* INCLUDE_RTCP ] */
#ifdef TESTING_ODD_LENGTH_PACKETS /* [ */
   static int       sPacketPad;

   static int setPacketPad(int value);
#endif /* TESTING_ODD_LENGTH_PACKETS ] */
   u_long mPrevIP;
   int mPrevPort;
   int mNumPushed;
   int mNumWarnings;

   int mPrefSsrc;  // current "preferred SSRC"
   UtlBoolean mPrefSsrcValid;
   u_long mRtpDestIp;    // where this connection is sending to
   int mRtpDestPort;

   int mNumNonPrefPackets;
   int mRtpRtcpMatchSsrc;
   UtlBoolean mRtpRtcpMatchSsrcValid;
   int mRtpDestMatchIpOnlySsrc;
   UtlBoolean mRtpDestMatchIpOnlySsrcValid;
   int mRtpOtherSsrc;
   UtlBoolean mRtpOtherSsrcValid;
   static const int SSRC_SWITCH_MISMATCH_COUNT;


   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                                    MpBufPtr outBufs[],
                                    int inBufsSize,
                                    int outBufsSize,
                                    UtlBoolean isEnabled,
                                    int samplesPerFrame=80,
                                    int samplesPerSecond=8000);

   MprFromNet(const MprFromNet& rMprFromNet);
     //:Copy constructor (not implemented for this class)

   MprFromNet& operator=(const MprFromNet& rhs);
     //:Assignment operator (not implemented for this class)

   OsStatus rtcpStats(struct rtpHeader *h);
     //:Update the RR info for the current incoming packet

   MprDejitter* getMyDejitter(void);

   int adjustBufferForRtp(MpBufPtr buf);
     //:Adjust buffer header based on RTP packet header.

   int extractSsrc(MpBufPtr buf);
     //:Extract and return SSRC from RTP packet header.

   int setPrefSsrc(int newSsrc);
   int getPrefSsrc(void);


};

/* ============================ INLINE METHODS ============================ */
#ifdef INCLUDE_RTCP /* [ */
inline  void  MprFromNet::setDispatchers(IRTPDispatch *piRTPDispatch, INetDispatch *piRTCPDispatch)
{
// Set the dispatch pointers for both RTP and RTCP
   mpiRTPDispatch   = piRTPDispatch;
   mpiRTCPDispatch  = piRTCPDispatch;
}

#endif /* INCLUDE_RTCP ] */

#endif  // _MprFromNet_h_
