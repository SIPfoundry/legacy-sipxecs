//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _ConferenceEngineMediaInterface_h_
#define _ConferenceEngineMediaInterface_h_

// SYSTEM INCLUDES
//#include <>

// APPLICATION INCLUDES
#include <os/OsStatus.h>
#include <os/OsDefs.h>
#include <os/OsBSem.h>
#include <os/OsNotification.h>
#include <os/OsStunDatagramSocket.h>
#include <net/QoS.h>
#include <net/SdpCodecFactory.h>
#include "mi/CpMediaInterface.h"
#include "include/ConferenceEngineFactoryImpl.h"
#include "include/ConferenceEngineEventHandler.h"


// DEFINES
#define GIPS_CODEC_ID_IPCMWB    "IPCMWB"
#define GIPS_CODEC_ID_ISAC      "ISAC"
#define GIPS_CODEC_ID_EG711U    "EG711U"
#define GIPS_CODEC_ID_EG711A    "EG711A"
#define GIPS_CODEC_ID_PCMA      "PCMA"
#define GIPS_CODEC_ID_PCMU      "PCMU"
#define GIPS_CODEC_ID_ILBC      "iLBC"
#define GIPS_CODEC_ID_G729      "G729"
#define GIPS_CODEC_ID_TELEPHONE "audio/telephone-event"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SdpCodec;
class OsDatagramSocket;
class ConferenceEngineFactoryImpl;
class ConferenceEngineMediaConnection;
class ConferenceEngine;
class ConferenceEngineNetTask;

/**
 * The ConferenceEngineMediaInterface is an object derived from CpMediaInterface
 * class. It implements all the interafces defined in CpMediaInterface. One
 * ConferenceEngineMediaInterface is created for each call and one connection is
 * created for each conference participant.
 *
 */

class ConferenceEngineMediaInterface : public CpMediaInterface
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

    /// Constructor
    ConferenceEngineMediaInterface(ConferenceEngineFactoryImpl* pFactoryImpl,
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

    /// Destructor
    virtual ~ConferenceEngineMediaInterface();

    /// public interface for destroying this media interface
    virtual void release();

/* ============================ MANIPULATORS ============================== */

    /// Create a media connection in the ConferenceEngine subsystem
    virtual OsStatus createConnection(int& connectionId, void* videoWindowHandle);

    /// Get the number of codec for a given connection
    int getNumCodecs(int connectionId);

    /// Get the port, address, and codec capabilities for the specified connection
    virtual OsStatus getCapabilities(int connectionId,
                                     UtlString& rtpHostAddress,
                                     int& rtpAudioPort,
                                     int& rtcpAudioPort,
                                     int& rtpVideoPort,
                                     int& rtcpVideoPort,
                                     SdpCodecFactory& supportedCodecs,
                                     SdpSrtpParameters& srtpParams);

    /// Set the connection destination (target) for the ConferenceEngineMediaInterface
    virtual OsStatus setConnectionDestination(int connectionId,
                                              const char* rtpHostAddress,
                                              int rtpAudioPort,
                                              int rtcpAudioPort,
                                              int rtpVideoPort,
                                              int rtcpVideoPort);

    /// Add an alternate connection destination for the ConferenceEngineMediaInterface
    virtual OsStatus addAlternateDestinations(int connectionId,
                                              unsigned char cPriority,
                                              const char* rtpHostAddress,
                                              int port,
                                              bool bRtp) ;

    /// Start sending RTP using the specified codec list
    virtual OsStatus startRtpSend(int connectionId,
                                  int numCodecs,
                                  SdpCodec* sendCodec[],
                                  SdpSrtpParameters& srtpParams);

    /// Start receiving RTP using the specified codec list
    virtual OsStatus startRtpReceive(int connectionId,
                                     int numCodecs,
                                     SdpCodec* sendCodec[],
                                     SdpSrtpParameters& srtpParams);
    /**
     * Note: startRtpReceive() must be called before startRtpSend().
     *
     */

    /// Stop sending RTP (and RTCP) data for the specified connection
    virtual OsStatus stopRtpSend(int connectionId);

    /// Stop receiving RTP (and RTCP) data for the specified connection
    virtual OsStatus stopRtpReceive(int connectionId);

    /// Delete the specified connection and free up any resources associated with that connection
    virtual OsStatus deleteConnection(int connectionId);

    /// Unsupported
    virtual OsStatus startTone(int toneId, UtlBoolean local, UtlBoolean remote);

    /// Unsupported
    virtual OsStatus stopTone();

    /// Play the specified audio URL to all the participants on the call
    virtual OsStatus playAudio(const char* url,
                               UtlBoolean repeat,
                               UtlBoolean local,
                               UtlBoolean remote);

    /// Play the specified audio URL to a given connection
    virtual OsStatus playAudioForIndividual(int connectionId,
                                            const char* url,
                                            OsNotification* event = NULL);

    /// Unsupported
    virtual OsStatus playBuffer(char* buf,
                                unsigned long bufSize,
                                int type,
                                UtlBoolean repeat,
                                UtlBoolean local,
                                UtlBoolean remote,
                                OsProtectedEvent* event = NULL);

    /// Unsupported
    virtual OsStatus pauseAudio();

    /// Stop playing any URLs for all the participants
    virtual OsStatus stopAudio();

    /// Stop playing any URLs for individual
    virtual OsStatus stopAudioForIndividual(int connectionId);

    /// Unsupported
    virtual OsStatus createPlayer(MpStreamPlayer** ppPlayer,
                                  const char* szStream,
                                  int flags,
                                  OsMsgQ *pMsgQ = NULL,
                                  const char* szTarget = NULL) ;


    /// Unsupported
    virtual OsStatus destroyPlayer(MpStreamPlayer* pPlayer);

    /// Unsupported
    virtual OsStatus createPlaylistPlayer(MpStreamPlaylistPlayer**
                                          ppPlayer,
                                          OsMsgQ *pMsgQ = NULL,
                                          const char* szTarget = NULL);

    /// Unsupported
    virtual OsStatus destroyPlaylistPlayer(MpStreamPlaylistPlayer* pPlayer);

    /// Unsupported
    virtual OsStatus createQueuePlayer(MpStreamQueuePlayer** ppPlayer,
                                       OsMsgQ *pMsgQ = NULL,
                                       const char* szTarget = NULL);

    /// Unsupported
    virtual OsStatus destroyQueuePlayer(MpStreamQueuePlayer* pPlayer);

    /// Unsupported
    virtual OsStatus giveFocus();

    /// Unsupported
    virtual OsStatus defocus();

    /// Unsupported
    virtual void setCodecCPULimit(int iLimit);

    /**
     * Add a listener event to this call that will receive callback
     * or queued event notifications upon receipt of DTMF tone events
     * (RFC 2833).
     */
    virtual void addToneListener(OsNotification *pListener, int connectionId);

    /// Remove the specified DTMF listener from this call.
    virtual void removeToneListener(int connectionId);

    /// Stop recording for this call.
    virtual OsStatus stopRecording();

    /// Start recording audio for this call.
    virtual OsStatus ezRecord(int ms,
                              int silenceLength,
                              const char* fileName,
                              double& duration,
                              int& dtmfterm,
                              OsProtectedEvent* ev = NULL);

    /// Set the preferred contact type for this media connection
    virtual void setContactType(int connectionId, ContactType eType) ;

    /// Return the pointer of ConferenceEngine object
    ConferenceEngine* const getConferenceEnginePtr();

    /// Mute a specific connection
    virtual OsStatus mute(int connectionId);

    /// Unmute the connection
    virtual OsStatus unmute(int connectionId);

/* ============================ ACCESSORS ================================= */

    /// Unsupported
    virtual void setPremiumSound(UtlBoolean enabled);

    /// Unsupported
    virtual int getCodecCPUCost();

    /// Unsupported
    virtual int getCodecCPULimit();

    /// Unsupported
    virtual OsMsgQ* getMsgQ();

    /// Returns the primary codec for the connection
    virtual OsStatus getPrimaryCodec(int connectionId,
                                     UtlString& audioCodec,
                                     UtlString& videoCodec,
                                     int* audiopPayloadType,
                                     int* videoPayloadType);

    /// Return the name of codec based on the type
    static UtlBoolean getCodecNameByType(SdpCodec::SdpCodecTypes codeType, UtlString& codecName);

    /// Unsupported
    virtual OsStatus setVideoWindowDisplay(const void* hWnd);

    /// Unsupported
    virtual const void* getVideoWindowDisplay();

    /// Get the DTMF notifier
    OsNotification* getDTMFNotifier(int connectionId);

    /// Get the play notifier
    OsNotification* getPlayNotifier(int connectionId);

/* ============================ INQUIRY =================================== */

    /// Query whether the specified media connection is enabled for sending RTP
    virtual UtlBoolean isSendingRtpAudio(int connectionId);

    /// Query whether the specified media connection is enabled for receiving RTP
    virtual UtlBoolean isReceivingRtpAudio(int connectionId);

    /// Query whether the specified media connection is enabled for sending RTP
    virtual UtlBoolean isSendingRtpVideo(int connectionId);

    /// Query whether the specified media connection is enabled for receiving RTP
    virtual UtlBoolean isReceivingRtpVideo(int connectionId);

    /// Query whether the specified media connection has a destination set for sending RTP
    virtual UtlBoolean isDestinationSet(int connectionId);

    /// Query whether a new party can be added to this media interfaces
    virtual UtlBoolean canAddParty() ;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    /// Set the DTMF mode
    OsStatus setDTMF(int connectionId);

    /// Set the volume on a specific connection
    virtual OsStatus setVolume(int connectionId);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    /// Get the ConferenceEngineMediaConnection for a given connection Id
    ConferenceEngineMediaConnection* getMediaConnection(int connecctionId);

    UtlBoolean getCodecTypeByName(const UtlString& codecName, SdpCodec::SdpCodecTypes& codecType);
    UtlBoolean getConferenceEngineCodec(const SdpCodec& pCodec, GIPS_CodecInst& cInst);

    ConferenceEngine* mpConferenceEngine ;
    ConferenceEngineEventHandler mEventHandler;

    UtlSList mMediaConnections ;

    UtlString mRtpReceiveHostAddress ;
    UtlString mLocalAddress ;
    UtlBoolean mRingToneFromFile ;
    SdpCodecFactory mSupportedCodecs ;
    int mExpeditedIpTos ;

    UtlString mStunServer ;
    int mStunOptions ;
    int mStunRefreshPeriodSecs ;

    ConferenceEngineNetTask* mpNetTask ;

    UtlBoolean mDTMFOutOfBand;
    UtlBoolean mFocus;
    float mDefaultVolume;

    OsBSem mLock;

};

/* ============================ INLINE METHODS ============================ */


#endif  // _ConferenceEngineMediaInterface_h_
