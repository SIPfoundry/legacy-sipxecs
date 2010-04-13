//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _MpConnection_h_
#define _MpConnection_h_

#include "rtcp/RtcpConfig.h"

// FORWARD DECLARATIONS
class MpCallFlowGraph;
class MpDecoderBase;
class MprDecode;
class MprDejitter;
class MprEncode;
class MprToNet;
class MprFromNet;
class OsSocket;
class OsNotification;
class SdpCodec;
class MprRecorder;

typedef struct GIPSNETEQ_inst NETEQ_inst;

// SYSTEM INCLUDES
// #include <...>

// APPLICATION INCLUDES
#include "mp/JB/jb_typedefs.h"
#include "mp/MprFromNet.h"
#include "mp/MprToNet.h"
#include "mp/MpResource.h"
#ifdef INCLUDE_RTCP /* [ */
#include "rtcp/IRTCPConnection.h"
#endif /* INCLUDE_RTCP ] */

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

typedef int MpConnectionID;

//:Connection container for the inbound and outbound network paths to a
// single remote party.
class MpConnection
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   friend class MpCallFlowGraph;

   enum {NUM_PAYLOAD_TYPES = (2<<7),
         MAX_ACTIVE_PAYLOAD_TYPES = 10,
   };

   enum PremiumSoundOptions
   {
      DisablePremiumSound,
      EnablePremiumSound
   };

/* ============================ CREATORS ================================== */

   MpConnection(MpCallFlowGraph* pParent, MpConnectionID myID,
                                 int samplesPerFrame, int samplesPerSec);
     //:Constructor

   virtual
   ~MpConnection();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   OsStatus disableIn(void);
   OsStatus disableOut(void);
   OsStatus disable(void); // Both in and out
     //:Disables the input path, output path, or both paths, of the connection.
     // Resources on the path(s) will also be disabled by these calls.
     // If the flow graph is not "started", this call takes effect
     // immediately.  Otherwise, the call takes effect at the start of the
     // next frame processing interval.
     //!retcode: OS_SUCCESS - for now, these methods always return success

   OsStatus enableIn(void);
   OsStatus enableOut(void);
   OsStatus enable(void); // Both in and out
     //:Enables the input path, output path, or both paths, of the connection.
     // Resources on the path(s) will also be enabled by these calls.
     // Resources may allocate needed data (e.g. output path reframe buffer)
     //  during this operation.
     // If the flow graph is not "started", this call takes effect
     // immediately.  Otherwise, the call takes effect at the start of the
     // next frame processing interval.
     //!retcode: OS_SUCCESS - for now, these methods always return success

   void startSendRtp(OsSocket& rRtpSocket, OsSocket& rRtcpSocket,
                  SdpCodec* pPrimary, SdpCodec* pDtmf, SdpCodec* pSecondary
                  );
     //:Starts sending RTP and RTCP packets.

   void startSendRtp(SdpCodec& rCodec,
                  OsSocket& rRtpSocket, OsSocket& rRtcpSocket);
     //:Starts sending RTP and RTCP packets.

   void stopSendRtp(void);
     //:Stops sending RTP and RTCP packets.

   void startReceiveRtp(SdpCodec* pCodecs[], int numCodecs,
                     OsSocket& rRtpSocket, OsSocket& rRtcpSocket);
     //:Starts receiving RTP and RTCP packets.

   void stopReceiveRtp(void);
     //:Stops receiving RTP and RTCP packets.

   OsStatus setBridgePort(int port);
     //:Save the port number that was assigned by the bridge.

   void startTone(int toneId);

   void stopTone(void);

#ifdef INCLUDE_RTCP /* [ */
// A new SSRC has been generated for the Session
   void     reassignSSRC(int iSSRC);
#endif /* INCLUDE_RTCP ] */

   void addPayloadType(int payloadId, MpDecoderBase* pDecoder);
   //:Add an RTP payload type to decoder instance mapping table

   void deletePayloadType(int payloadId);
   //:Remove an RTP payload type from decoder instance map

/* ============================ ACCESSORS ================================= */

   JB_inst* getJBinst(UtlBoolean optional = FALSE);
     //:Returns a pointer to the JB instance, creating it if necessary
     // If the instance has not been created, but the argument "optional" is
     // TRUE, then do not create it, just return NULL.

   MpResource* getSinkResource(void);
     //:Returns the resource to link to upstream resource's outPort.

   MpResource* getSourceResource(void);
     //:Returns the resource to link to downstream resource's inPort.

   int getBridgePort(void);
     //:Retrieve the port number that was assigned by the bridge.

#ifdef INCLUDE_RTCP /* [ */
    //:Retrieve the RTCP Connection interface associated with this MpConnection
   IRTCPConnection *getRTCPConnection(void);
#endif /* INCLUDE_RTCP ] */

   MpDecoderBase* mapPayloadType(int payloadType);

   void setPremiumSound(PremiumSoundOptions op);
     //:Disables or enables the GIPS premium sound.

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   UtlBoolean handleSetDtmfNotify(OsNotification* n);
     //:Handle the FLOWGRAPH_SET_DTMF_NOTIFY message.
     // Returns TRUE

   UtlBoolean setDtmfTerm(MprRecorder *pRecorder);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   MpConnection();
     //:Default constructor

   MpConnection(const MpConnection& rMpConnection);
     //:Copy constructor (not implemented for this type)

   MpConnection& operator=(const MpConnection& rhs);
     //:Assignment operator (not implemented for this type)

   MpCallFlowGraph*   mpFlowGraph;     // parent
   MprEncode*         mpEncode;        // the outbound components
   MprToNet*          mpToNet;
   MprFromNet*        mpFromNet;       // the inbound components
   MprDejitter*       mpDejitter;
   MprDecode*         mpDecode;
   MpConnectionID     mMyID;           // my ID within my parent
   int                mBridgePort;     // where we are connected on the bridge
   UtlBoolean          mInEnabled;      // current state of components
   UtlBoolean          mOutEnabled;
   UtlBoolean          mInRtpStarted;   // are we currently receiving
   UtlBoolean          mOutRtpStarted;  //                  sending
   JB_inst*           mpJB_inst;

   MpDecoderBase*     mpPayloadMap[NUM_PAYLOAD_TYPES];
   OsMutex            mLock;

#ifdef INCLUDE_RTCP /* [ */
// RTCP Connection Interface pointer
   IRTCPConnection *mpiRTCPConnection;

#endif /* INCLUDE_RTCP ] */
};

/* ============================ INLINE METHODS ============================ */
#ifdef INCLUDE_RTCP /* [ */
inline IRTCPConnection *MpConnection::getRTCPConnection(void)
{
    return(mpiRTCPConnection);

}

inline void MpConnection::reassignSSRC(int iSSRC)
{

//  Set the new SSRC
    mpToNet->setSSRC(iSSRC);

    return;

}
#endif /* INCLUDE_RTCP ] */

#endif  // _MpConnection_h_
