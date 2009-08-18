//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _MprToNet_h_
#define _MprToNet_h_

#include "rtcp/RtcpConfig.h"

// Defining this option includes debug code for introducing sudden changes
// to several fields of the outgoing RTP headers (SSRC, seq#, timestamp).
#define ENABLE_PACKET_HACKING
#undef ENABLE_PACKET_HACKING

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsSocket.h"
#include "mp/MpFlowGraphMsg.h"
#include "mp/MpResource.h"
#include "mp/NetInTask.h"
#include "mp/MprFromNet.h"
#ifdef INCLUDE_RTCP /* [ */
#include "rtcp/ISetSenderStatistics.h"
#endif /* INCLUDE_RTCP ] */

// DEFINES
#define NETWORK_MTU 1500
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:The "To Network" media processing resource
class MprToNet : public MpResource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum { RESERVED_RTP_PACKET_HEADER_BYTES = 76};
             // 76 =    4 for encryption prefix +
             //        12 for basic packet header +
             //      15*4 for CSRC list

#ifdef ENABLE_PACKET_HACKING /* [ */
   static int sDebug1;
   static int sDebug2;
   static int sDebug3;
   static int sDebug4;
   static int sDebug5;
#endif /* ENABLE_PACKET_HACKING ] */

/* ============================ CREATORS ================================== */

   MprToNet(const UtlString& rName, int samplesPerFrame, int samplesPerSec);
     //:Constructor

   virtual
   ~MprToNet();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   OsStatus setSockets(OsSocket& rRtpSocket, OsSocket& rRtcpSocket);
     //:Sends a SET_SOCKETS message to this resource to set the outbound
     //:RTP and RTCP sockets.
     // Returns the result of attempting to queue the message to this
     // resource.

   OsStatus resetSockets();
     //:Sends a RESET_SOCKETS message to this resource to stop sending
     //:RTP and RTCP sockets.
     // returns OS_SUCCESS, unless unable to queue message.

   OsStatus setRtpPal(MprFromNet* pal);
   // Connect us to our corresponding FromNet, for RTCP stats.

   OsStatus sendRtcpPacket(void); // Time to send an RTCP message

   int writeRtp(int payloadType, UtlBoolean markerState,
      unsigned char* payloadData, int payloadOctets, unsigned int timestamp,
      void* csrcList);

/* ============================ ACCESSORS ================================= */

// These accessors were added by DMG to allow a Connection to access and modify
// rtp and rtcp stream information
   void   setSSRC(int iSSRC);
#ifdef INCLUDE_RTCP /* [ */
   void   setRTPAccumulator(ISetSenderStatistics *piRTPAccumulator);
#endif /* INCLUDE_RTCP ] */
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   enum AddlMsgTypes
   {
      SET_SOCKETS = MpFlowGraphMsg::RESOURCE_SPECIFIC_START,
      RESET_SOCKETS
   };

#ifdef DEBUG /* [ */
   unsigned int mRtpSampleCounter;
#endif /* DEBUG ] */
   MprFromNet*  mpFromNetPal;
   int          mRtcpPackets;
   int          mRtcpFrameCount;
   int          mRtcpFrameLimit;

   // RTP State
   unsigned int mTimestampDelta;
   unsigned int mSeqNum;
   unsigned int mSSRC;
   OsSocket*    mpRtpSocket;
   OsSocket*    mpRtcpSocket;
   int          mNumRtpWriteErrors;
   int          mNumRtcpWriteErrors;

#ifdef ENABLE_PACKET_HACKING /* [ */
   void adjustRtpPacket(struct rtpHeader* p);
#endif /* ENABLE_PACKET_HACKING ] */

#ifdef INCLUDE_RTCP /* [ */
// Allow outbound RTP stream to accumulate RTP packet statistics
   ISetSenderStatistics *mpiRTPAccumulator;
#endif /* INCLUDE_RTCP ] */

   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                                    MpBufPtr outBufs[],
                                    int inBufsSize,
                                    int outBufsSize,
                                    UtlBoolean isEnabled,
                                    int samplesPerFrame=80,
                                    int samplesPerSecond=8000);

   virtual UtlBoolean handleMessage(MpFlowGraphMsg& rMsg);
     //:Handle messages for this resource.

   MprToNet(const MprToNet& rMprToNet);
     //:Copy constructor (not implemented for this class)

   MprToNet& operator=(const MprToNet& rhs);
     //:Assignment operator (not implemented for this class)

   void sentRtcpPacket(void);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprToNet_h_
