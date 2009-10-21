//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _VoiceEngineMediaInterface_h_
#define _VoiceEngineMediaInterface_h_

// SYSTEM INCLUDES
//#include <>

// APPLICATION INCLUDES
#include <os/OsStatus.h>
#include <os/OsDefs.h>
#include <os/OsMutex.h>
#include <os/OsStunDatagramSocket.h>
#include <net/QoS.h>
#include <net/SdpCodecFactory.h>
#include "include/VoiceEngineFactoryImpl.h"
#include "mi/CpMediaInterface.h"


/**
 * Type for storing a "window object handle" - in Windows,
 * the application should cast their HWND to a SIPX_WINDOW_HANDLE.
 */
typedef void* SIPXVE_WINDOW_HANDLE;

/**
 * Enum for specifying the type of display object
 * to be used for displaying video
 */
typedef enum SIPXVE_VIDEO_DISPLAY_TYPE
{
    SIPXVE_WINDOW_HANDLE_TYPE,     /**< A handle to the window for
                                        the remote video display */
    SIPXVE_DIRECT_SHOW_FILTER      /**< A DirectShow render filter object for
                                        handling the remote video display */
} SIPXVE_VIDEO_DISPLAY_TYPE;

typedef struct SIPXVE_VIDEO_DISPLAY
{
      int cbSize;
      SIPXVE_VIDEO_DISPLAY_TYPE type;
      union
      {
        SIPXVE_WINDOW_HANDLE handle;
        void* filter;
      };
} SIPXVE_VIDEO_DISPLAY;


// DEFINES
#define GIPS_CODEC_ID_IPCMWB      "IPCMWB"
#define GIPS_CODEC_ID_ISAC        "ISAC"
#define GIPS_CODEC_ID_EG711U      "EG711U"
#define GIPS_CODEC_ID_EG711A      "EG711A"
#define GIPS_CODEC_ID_PCMA        "PCMA"
#define GIPS_CODEC_ID_PCMU        "PCMU"
#define GIPS_CODEC_ID_ILBC        "iLBC"
#define GIPS_CODEC_ID_G729        "G729"
#define GIPS_CODEC_ID_GSM         "GSM"
#define GIPS_CODEC_ID_G723        "G723"
#define GIPS_CODEC_ID_VP71_CIF    "VP71-CIF"
#define GIPS_CODEC_ID_VP71_QCIF   "VP71-QCIF"
#define GIPS_CODEC_ID_VP71_SQCIF  "VP71-SQCIF"
#define GIPS_CODEC_ID_IYUV_CIF    "IYUV-CIF"
#define GIPS_CODEC_ID_IYUV_QCIF   "IYUV-QCIF"
#define GIPS_CODEC_ID_IYUV_SQCIF  "IYUV-SQCIF"
#define GIPS_CODEC_ID_I420_CIF    "I420-CIF"
#define GIPS_CODEC_ID_I420_QCIF   "I420-QCIF"
#define GIPS_CODEC_ID_I420_SQCIF  "I420-SQCIF"
#define GIPS_CODEC_ID_RGB24_CIF   "RGB24-CIF"
#define GIPS_CODEC_ID_RGB24_QCIF  "RGB24-QCIF"
#define GIPS_CODEC_ID_RGB24_SQCIF "RGB24-SQCIF"
#define GIPS_CODEC_ID_TELEPHONE   "audio/telephone-event"

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
class VoiceEngineFactoryImpl;
class VoiceEngineMediaConnection;
class GipsVoiceEngineLib ;
#ifdef VIDEO
class GipsVideoEngineWindows ;
#endif
class VoiceEngineNetTask ;

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class VoiceEngineMediaInterface : public CpMediaInterface
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   VoiceEngineMediaInterface(VoiceEngineFactoryImpl* pFactoryImpl,
                             const char* publicAddress = NULL,
                             const char* localAddress = NULL,
                             int numCodecs = 0,
                             SdpCodec* sdpCodecArray[] = NULL,
                             const char* pLocale = "",
                             int expeditedIpTos = QOS_LAYER3_LOW_DELAY_IP_TOS,
                             const char* szStunServer = NULL,
                             int stunOptions = STUN_OPTION_NORMAL,
                             int iStunKeepAlivePeriodSecs = 28,
                             UtlBoolean bDTMFInband=TRUE);

   virtual
   ~VoiceEngineMediaInterface();
     //:Destructor

   /**
    * public interface for destroying this media interface
    */
   virtual void release();

/* ============================ MANIPULATORS ============================== */

   virtual OsStatus createConnection(int& connectionId,
                                     const char* szLocalAddress,
                                     void* videoWindowHandle) ;

   virtual int getNumCodecs(int connectionId);

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

   virtual OsStatus setVideoWindowDisplay(const void* hWnd);
   virtual const void* getVideoWindowDisplay();


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

   virtual OsStatus startDtmf(char toneId, UtlBoolean local, UtlBoolean remote);
   virtual OsStatus stopDtmf();

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
   virtual OsStatus pauseAudio();
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

   //virtual OsStatus enableSrtp(

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

   GipsVoiceEngineLib* const getVoiceEnginePtr();

/* ============================ ACCESSORS ================================= */

   virtual void setPremiumSound(UtlBoolean enabled);

   virtual int getCodecCPUCost();
      //:Calculate the current cost for our sending/receiving codecs

   virtual int getCodecCPULimit();
      //:Calculate the worst cost for our sending/receiving codecs

   virtual OsMsgQ* getMsgQ();
     //:Returns the flowgraph's message queue

   virtual OsStatus getPrimaryCodec(int connectionId,
                                    UtlString& audioCodec,
                                    UtlString& videoCodec,
                                    int* audioPayloadType,
                                    int* videoPayloadType);
     //:Returns the codec used in RTP session

    static UtlBoolean getCodecNameByType(SdpCodec::SdpCodecTypes codeType, UtlString& codecName);
     //:Match up codec types with VoiceEngine codec names


/* ============================ INQUIRY =================================== */

   virtual UtlBoolean isSendingRtpAudio(int connectionId);
   virtual UtlBoolean isSendingRtpVideo(int connectionId);

   virtual UtlBoolean isReceivingRtpAudio(int connectionId);
   virtual UtlBoolean isReceivingRtpVideo(int connectionId);

   virtual UtlBoolean isDestinationSet(int connectionId);

   OsStatus muteMicrophone(const bool bMute);

   virtual UtlBoolean canAddParty() ;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    void startVideoSupport(int connectionId) ;
    void endVideoSupport(int connectionId) ;
    UtlBoolean containsVideoCodec(int numCodecs, SdpCodec* codecs[]) ;
    void doVideo(int connectionId) ;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    VoiceEngineMediaConnection* getMediaConnection(int connecctionId);
    VoiceEngineMediaConnection* removeMediaConnection(int connecctionId);
    void restartRtpSendVideo(int connectionId);

    UtlBoolean getCodecTypeByName(const UtlString& codecName, SdpCodec::SdpCodecTypes& codecType);
    UtlBoolean getVoiceEngineCodec(const SdpCodec& pCodec, GIPSVE_CodecInst& cInst);
#ifdef VIDEO
    UtlBoolean getVideoEngineCodec(const SdpCodec& pCodec, GIPSVideo_CodecInst& codecInfo);
    const bool isDisplayValid(const SIPXVE_VIDEO_DISPLAY* const pDisplay);
#endif

    UtlString mRtpReceiveHostAddress ;
    UtlString mLocalAddress ;
    MpCallFlowGraph* mpFlowGraph ;
    UtlBoolean mRingToneFromFile ;
    SdpCodecFactory mSupportedCodecs ;
    UtlSList mMediaConnections ;
    int mExpeditedIpTos ;
    UtlString mStunServer ;
    int mStunOptions ;
    int mStunRefreshPeriodSecs ;
    ContactType mContactType;
    UtlBoolean mbFocus ;
    UtlBoolean mbDTMFOutOfBand;
    GipsVoiceEngineLib* mpVoiceEngine ;
    SdpCodec* mPrimaryVideoCodec;
#ifdef VIDEO
    GipsVideoEngineWindows* mpVideoEngine ;
#endif
    VoiceEngineNetTask* pNetTask ;
    UtlBoolean mbLocalMute ;
    OsMutex mVoiceEngineGuard ;
    SIPXVE_VIDEO_DISPLAY* mpDisplay;
    bool mbVideoStarted;
    void startRtpReceiveVideo(int channelId);
    bool mbVideoChannelEstablishedReceiving;
    bool mbVideoChannelEstablishedForSend;
};

/* ============================ INLINE METHODS ============================ */


#endif  // _VoiceEngineMediaInterface_h_
