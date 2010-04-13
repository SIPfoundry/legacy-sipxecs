//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _CpPhoneMediaInterface_h_
#define _CpPhoneMediaInterface_h_

// SYSTEM INCLUDES
//#include <>

// APPLICATION INCLUDES
#include <os/OsStatus.h>
#include <os/OsDefs.h>
#include <net/QoS.h>
#include <net/SdpCodecFactory.h>
#include "mi/CpMediaInterface.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MpCallFlowGraph;
class SdpCodec;
class OsDatagramSocket;
class CpPhoneMediaConnection;

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class CpPhoneMediaInterface : public CpMediaInterface
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum OutputAudioDevice
   {
      UNKNOWN = 0x0,
      HANDSET = 0x1,
      SPEAKER = 0x2,
      HEADSET = 0x4
   };

/* ============================ CREATORS ================================== */

   CpPhoneMediaInterface(CpMediaInterfaceFactoryImpl* pFactoryImpl,
                         const char* publicAddress = NULL,
                         const char* localAddress = NULL,
                         int numCodecs = 0,
                         SdpCodec* sdpCodecArray[] = NULL,
                         const char* pLocale = "",
                         int expeditedIpTos = QOS_LAYER3_LOW_DELAY_IP_TOS,
                         const char* szStunServer = NULL,
                         int iStunKeepAlivePeriodSecs = 28);
     //:Default constructor

  protected:
   virtual
   ~CpPhoneMediaInterface();
     //:Destructor
  public:

   /**
    * public interface for destroying this media interface
    */
   void release();

/* ============================ MANIPULATORS ============================== */

   CpPhoneMediaInterface& operator=(const CpPhoneMediaInterface& rhs);
     //:Assignment operator


   virtual OsStatus createConnection(int& connectionId,
                                     const char* szLocalAddress,
                                     void* videoWindowHandle);

   virtual OsStatus getCapabilities(int connectionId,
                                    UtlString& rtpHostAddress,
                                    int& rtpAudioPort,
                                    int& rtcpAudioPort,
                                    int& rtpVideoPort,
                                    int& rtcpVideoPort,
                                    SdpCodecFactory& supportedCodecs,
                                    SdpSrtpParameters& srtpParams);

   virtual OsStatus setConnectionDestination(int connectionId,
                                             const char* rtpHostAddress,
                                             int rtpAudioPort,
                                             int rtcpAudioPort,
                                             int rtpVideoPort,
                                             int rtcpVideoPort);


   virtual OsStatus addAlternateDestinations(int connectionId,
                                             unsigned char cPriority,
                                             const char* rtpHostAddress,
                                             int port,
                                             bool bRtp) ;

   virtual OsStatus startRtpSend(int connectionId,
                                 int numCodecs,
                                 SdpCodec* sendCodec[],
                                 SdpSrtpParameters& srtpParams);
   virtual OsStatus startRtpReceive(int connectionId,
                                    int numCodecs,
                                    SdpCodec* sendCodec[],
                                    SdpSrtpParameters& srtpParams);
   virtual OsStatus stopRtpSend(int connectionId);
   virtual OsStatus stopRtpReceive(int connectionId);

   virtual OsStatus deleteConnection(int connectionId);

   virtual OsStatus startTone(int toneId, UtlBoolean local, UtlBoolean remote);
   virtual OsStatus stopTone();

   virtual OsStatus playAudio(const char* url,
                              UtlBoolean repeat,
                              UtlBoolean local,
                              UtlBoolean remote);
   virtual OsStatus playBuffer(char* buf,
                               unsigned long bufSize,
                               int type,
                              UtlBoolean repeat,
                              UtlBoolean local,
                              UtlBoolean remote,
                              OsProtectedEvent* event = NULL);
   virtual OsStatus stopAudio();

   virtual OsStatus createPlayer(MpStreamPlayer** ppPlayer,
                                 const char* szStream,
                                 int flags,
                                 OsMsgQ *pMsgQ = NULL,
                                 const char* szTarget = NULL) ;
   virtual OsStatus destroyPlayer(MpStreamPlayer* pPlayer);
   virtual OsStatus createPlaylistPlayer(MpStreamPlaylistPlayer**
                                         ppPlayer,
                                         OsMsgQ *pMsgQ = NULL,
                                         const char* szTarget = NULL);
   virtual OsStatus destroyPlaylistPlayer(MpStreamPlaylistPlayer* pPlayer);
   virtual OsStatus createQueuePlayer(MpStreamQueuePlayer** ppPlayer,
                                      OsMsgQ *pMsgQ = NULL,
                                      const char* szTarget = NULL);
   virtual OsStatus destroyQueuePlayer(MpStreamQueuePlayer* pPlayer);

   virtual OsStatus giveFocus();
   virtual OsStatus defocus();

   virtual void setCodecCPULimit(int iLimit);
     //:Limits the available codecs to only those within the designated
     //:limit.

   virtual void addToneListener(OsNotification *pListener, int connectionId);

   virtual void removeToneListener(int connectionId);

   virtual OsStatus stopRecording();
   virtual OsStatus ezRecord(int ms,
           int silenceLength,
           const char* fileName,
           double& duration,
           int& dtmfterm,
           OsProtectedEvent* ev = NULL);

   virtual void setContactType(int connectionId, ContactType eType) ;
     //: Set the contact type for this Phone media interface.
     //  It is important to set the contact type BEFORE creating the
     //  connection -- setting after the connection has been created
     //  is essentially a NOP.

/* ============================ ACCESSORS ================================= */

   virtual void setPremiumSound(UtlBoolean enabled);
   virtual int getCodecCPUCost();
      //:Calculate the current cost for our sending/receiving codecs

   virtual int getCodecCPULimit();
      //:Calculate the worst cost for our sending/receiving codecs

   virtual OsMsgQ* getMsgQ();
     //:Returns the flowgraph's message queue

   virtual OsStatus getVideoQuality(int& quality);
   virtual OsStatus getVideoBitRate(int& bitRate);
   virtual OsStatus getVideoFrameRate(int& frameRate);

   virtual OsStatus getPrimaryCodec(int connectionId, UtlString& audioCodec, UtlString& videoCodec, int *payloadType, int* videoPayloadType);
     //:Returns primary codec for the connection
   virtual OsStatus setVideoWindowDisplay(const void* hWnd);
   virtual const void* getVideoWindowDisplay();

   virtual OsStatus setVideoQuality(int quality);
   virtual OsStatus setVideoParameters(int bitRate, int frameRate);


/* ============================ INQUIRY =================================== */

   virtual UtlBoolean isSendingRtpAudio(int connectionId);
   virtual UtlBoolean isReceivingRtpAudio(int connectionId);
   virtual UtlBoolean isSendingRtpVideo(int connectionId);
   virtual UtlBoolean isReceivingRtpVideo(int connectionId);
   virtual UtlBoolean isDestinationSet(int connectionId);
   virtual UtlBoolean canAddParty() ;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    CpPhoneMediaConnection* getMediaConnection(int connecctionId);
    CpPhoneMediaConnection* removeMediaConnection(int connecctionId);
    OsStatus doDeleteConnection(CpPhoneMediaConnection* mediaConnection);

   UtlString mRtpReceiveHostAddress; // Advertized as place to send RTP/RTCP
   UtlString mLocalAddress; // On which ports are bound
   MpCallFlowGraph* mpFlowGraph;
   UtlBoolean mRingToneFromFile;
   SdpCodecFactory mSupportedCodecs;
   UtlDList mMediaConnections;
   int mExpeditedIpTos;
   UtlString mStunServer ;
   int mStunRefreshPeriodSecs ;

   // Disabled copy constructor
   CpPhoneMediaInterface(const CpPhoneMediaInterface& rCpPhoneMediaInterface);

};

/* ============================ INLINE METHODS ============================ */
#endif  // _CpPhoneMediaInterface_h_
