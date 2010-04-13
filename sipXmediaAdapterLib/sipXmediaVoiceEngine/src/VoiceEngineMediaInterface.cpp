//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>

// for Windows, we need the COM version defs
#ifdef _WIN32
    #define _OLE32_
    #define _WIN32_DCOM
    #define _WIN32_WINNT 0x0400
#endif

#ifdef __pingtel_on_posix__
#define OutputDebugString(x)
#endif

// APPLICATION INCLUDES
#include <utl/UtlSListIterator.h>
#include <os/OsProtectEventMgr.h>
#include <net/SdpCodec.h>
#include <net/SdpBody.h>
#ifdef _WIN32
#include "GipsVoiceEngineLib.h"
#else
#include "GipsVoiceEngineLibLinux.h"
#endif
#include "include/VoiceEngineNetTask.h"
#include "include/VoiceEngineDatagramSocket.h"
#include "include/VoiceEngineMediaInterface.h"
#ifdef VIDEO
    #include "GipsVideoEngine.h"
#ifdef _WIN32
    #include <windows.h>
    #include "GipsVideoEngineWindows.h"
#endif
#endif

#include "net/SipContactDb.h"

//#define USE_GLOBAL_VOICE_ENGINE

#if defined(_VXWORKS)
#   include <socket.h>
#   include <netinet/ip.h>
#elif defined(__pingtel_on_posix__)
#   include <netinet/in.h>
#   include <netinet/tcp.h>
#   include <sys/types.h>
#   include <sys/socket.h>
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define MINIMUM_DTMF_LENGTH 60
#define MAX_RTP_PORTS       1000

// STATIC VARIABLE INITIALIZATIONS

class VoiceEngineMediaConnection : public UtlInt
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
    VoiceEngineMediaConnection(int connectionId = -1)
        : UtlInt(connectionId)
    {
        mpRtpAudioSocket = NULL;
        mpRtcpAudioSocket = NULL;
        mRtpAudioSendHostPort = 0;
        mRtcpAudioSendHostPort = 0;
        mRtpAudioReceivePort = 0;
        mRtcpAudioReceivePort = 0;
        mpAudioSocketAdapter = NULL ;

#ifdef VIDEO
        mVideoConnectionId = -1;
        mpRtpVideoSocket = NULL;
        mpRtcpVideoSocket = NULL;
        mRtpVideoSendHostPort = 0;
        mRtcpVideoSendHostPort = 0;
        mRtpVideoReceivePort = 0;
        mRtcpVideoReceivePort = 0;
        mpVideoSocketAdapter = NULL ;
        mpPrimaryVideoCodec = NULL;
        mRtpVideoPayloadType = 0;
#endif
        mRtpPayloadType = 0;
        mDestinationSet = FALSE;
        mRtpSendingAudio = FALSE;
        mRtpSendingVideo = FALSE;
        mRtpReceivingAudio = FALSE;
        mRtpReceivingVideo = FALSE;
        mpCodecFactory = NULL;
        mpPrimaryCodec = NULL;
        meContactType = AUTO ;
    };

    virtual ~VoiceEngineMediaConnection()
    {
        if(mpRtpAudioSocket)
        {
            mpRtpAudioSocket->close();
            delete mpRtpAudioSocket;
            mpRtpAudioSocket = NULL;
        }

        if(mpRtcpAudioSocket)
        {
            mpRtcpAudioSocket->close();
            delete mpRtcpAudioSocket;
            mpRtcpAudioSocket = NULL;
        }

#ifdef VIDEO
        if(mpRtpVideoSocket)
        {
            mpRtpVideoSocket->close();
            delete mpRtpVideoSocket;
            mpRtpVideoSocket = NULL;
        }

        if(mpRtcpVideoSocket)
        {
            mpRtcpVideoSocket->close();
            delete mpRtcpVideoSocket;
            mpRtcpVideoSocket = NULL;
        }

        if (mpVideoSocketAdapter)
        {
            delete mpVideoSocketAdapter ;
            mpVideoSocketAdapter = NULL ;
        }
#endif
        if(mpCodecFactory)
        {
            delete mpCodecFactory;
            mpCodecFactory = NULL;
        }

        if (mpAudioSocketAdapter)
        {
            delete mpAudioSocketAdapter ;
            mpAudioSocketAdapter = NULL ;
        }
    }

    VoiceEngineDatagramSocket* mpRtpAudioSocket;
    VoiceEngineDatagramSocket* mpRtcpAudioSocket;
    VoiceEngineSocketAdapter* mpAudioSocketAdapter ;
    int mRtpAudioSendHostPort;
    int mRtcpAudioSendHostPort;
    int mRtpAudioReceivePort;
    int mRtcpAudioReceivePort;
    int mRtpPayloadType;

#ifdef VIDEO
    VoiceEngineDatagramSocket* mpRtpVideoSocket;
    VoiceEngineDatagramSocket* mpRtcpVideoSocket;
    VoiceEngineSocketAdapter* mpVideoSocketAdapter ;
    int mVideoConnectionId ;
    int mRtpVideoSendHostPort ;
    int mRtcpVideoSendHostPort ;
    int mRtpVideoReceivePort ;
    int mRtcpVideoReceivePort ;
    SdpCodec* mpPrimaryVideoCodec;
    int mRtpVideoPayloadType;
#endif

    UtlBoolean mDestinationSet;
    UtlBoolean mRtpSendingAudio;
    UtlBoolean mRtpReceivingAudio;
    UtlBoolean mRtpSendingVideo;
    UtlBoolean mRtpReceivingVideo;
    SdpCodecFactory* mpCodecFactory;
    SdpCodec* mpPrimaryCodec;
    ContactType meContactType ;
    UtlString mRtpSendHostAddress;
};

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
VoiceEngineMediaInterface::VoiceEngineMediaInterface(VoiceEngineFactoryImpl* pFactoryImpl,
                                                     const char* publicAddress,
                                                     const char* localAddress,
                                                     int numCodecs,
                                                     SdpCodec* sdpCodecArray[],
                                                     const char* locale,
                                                     int expeditedIpTos,
                                                     const char* szStunServer,
                                                     int stunOptions,
                                                     int iStunKeepAlivePeriodSecs,
                                                     UtlBoolean bDTMFOutOfBand)
    : CpMediaInterface(pFactoryImpl)
    , mVoiceEngineGuard(OsMutex::Q_FIFO)
    , mSupportedCodecs(numCodecs, sdpCodecArray),
      mpDisplay(NULL),
      mbVideoStarted(false),
      mPrimaryVideoCodec(NULL),
      mContactType(AUTO),
      mbVideoChannelEstablishedReceiving(false),
      mbVideoChannelEstablishedForSend(false)
{
#ifdef USE_GLOBAL_VOICE_ENGINE /* [ */
#ifdef VIDEO /* [ */
    mpVideoEngine = pFactoryImpl->getVideoEnginePointer();
#endif /* VIDEO ] */

#else /* ! USE_GLOBAL_VOICE_ENGINE */
    int rc ;
    int lastError ;

    // Initialize VoiceEngine
    mpVoiceEngine = GetNewVoiceEngineLib() ;
    rc = mpVoiceEngine->GIPSVE_Init() ;
    assert(rc == 0);
    lastError = mpVoiceEngine->GIPSVE_GetLastError();
    assert(lastError == 0) ;

    // Initialize GIPS Debug Tracing
#ifdef GIPS_TRACE /* [ */
    rc = mpVoiceEngine->GIPSVE_SetTrace(2);
    assert(rc == 0);
    lastError = mpVoiceEngine->GIPSVE_GetLastError();
    assert(lastError == 0) ;
#endif /* GIPS_TRACE ] */

    // Initialize Video
#ifdef VIDEO
    mpVideoEngine = GetNewVideoEngine();
    rc = mpVideoEngine->GIPSVideo_Init(mpVoiceEngine) ;
    OutputDebugString("GIPSVideo_Init\n");
#ifdef GIPS_TRACE /* [ */
        rc = mpVideoEngine->GIPSVideo_SetTrace(2);
#endif
    lastError = mpVideoEngine->GIPSVideo_GetLastError() ;
    //assert(lastError == 0) ;
#endif

#endif /* USE_GLOBAL_VOICE_ENGINE ] */

    mbFocus = FALSE ;

    int rc3 = mpVoiceEngine->GIPSVE_SetAGCStatus(0);     // make sure Automatic Gain Control is turned off
    assert(rc3 == 0);

    pNetTask = VoiceEngineNetTask::getVoiceEngineNetTask() ;

    mStunOptions = stunOptions ;
    mStunServer = szStunServer ;
    mStunRefreshPeriodSecs = iStunKeepAlivePeriodSecs ;
    mbDTMFOutOfBand = bDTMFOutOfBand;
    mbLocalMute = FALSE ;

    if(localAddress && *localAddress)
    {
        mRtpReceiveHostAddress = localAddress;
        mLocalAddress = localAddress;
    }
    else
    {
        OsSocket::getHostIp(&mLocalAddress);
    }

    if(sdpCodecArray && numCodecs > 0)
    {
        // Assign any unset payload types
        mSupportedCodecs.bindPayloadTypes();
    }
}


// Destructor
VoiceEngineMediaInterface::~VoiceEngineMediaInterface()
{
    OsLock lock(mVoiceEngineGuard) ;

    VoiceEngineMediaConnection* pMediaConnection = NULL;

    UtlSListIterator iterator(mMediaConnections);
    while ((pMediaConnection = (VoiceEngineMediaConnection*) iterator()))
    {
        deleteConnection(pMediaConnection->getValue()) ;
    }

#ifndef USE_GLOBAL_VOICE_ENGINE /* [ */
    mpVoiceEngine->GIPSVE_Terminate();
    delete mpVoiceEngine ;
    mpVoiceEngine = NULL;

#ifdef VIDEO
    mpVideoEngine->GIPSVideo_Terminate();
    OutputDebugString("GIPSVideo_Terminate\n");
    delete mpVideoEngine;
    mpVideoEngine = NULL;
#endif
#endif /* USE_GLOBAL_VOICE_ENGINE ] */
}


void VoiceEngineMediaInterface::release()
{
    // remove this pointer from the factory implementation
    if (mpFactoryImpl)
    {
        ((VoiceEngineFactoryImpl*) mpFactoryImpl)->removeInterface(this);
    }
    delete this;
}


/* ============================ MANIPULATORS ============================== */

OsStatus VoiceEngineMediaInterface::createConnection(int& connectionId,
                                                     const char* szLocalAddress,
                                                     void* videoDisplay)
{
    OsLock lock(mVoiceEngineGuard) ;
    int rc;
    int localAudioPort ;

#ifdef _WIN32
#ifdef VIDEO
    #include <objbase.h>
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED) ;
#endif
#endif

    /*
     * Create Audio Channel
     */
    mpFactoryImpl->getNextRtpPort(localAudioPort);

    connectionId = mpVoiceEngine->GIPSVE_CreateChannel() ;
    int gips_err = mpVoiceEngine->GIPSVE_GetLastError();
    assert(connectionId >= 0) ;



    VoiceEngineMediaConnection* pMediaConnection = new VoiceEngineMediaConnection() ;
    pMediaConnection->setValue(connectionId);
    mpDisplay = (SIPXVE_VIDEO_DISPLAY*)videoDisplay;

    // If the connection Id is already used -- this is an error
    assert(mMediaConnections.find(&UtlInt(connectionId)) == NULL) ;
    mMediaConnections.insert(pMediaConnection) ;

    VoiceEngineDatagramSocket* rtpAudioSocket = new VoiceEngineDatagramSocket(
            mpVoiceEngine, NULL, connectionId, -1,
            TYPE_AUDIO_RTP, 0, NULL, localAudioPort, mLocalAddress.data(),
            mStunServer.length() != 0, mStunServer, mStunOptions, mStunRefreshPeriodSecs);
    VoiceEngineDatagramSocket* rtcpAudioSocket = new VoiceEngineDatagramSocket(
            mpVoiceEngine, NULL, connectionId, -1,
            TYPE_AUDIO_RTCP, 0, NULL, localAudioPort + 1, mLocalAddress.data(),
            mStunServer.length() != 0, mStunServer, mStunOptions, mStunRefreshPeriodSecs);

    pMediaConnection->mpRtpAudioSocket = rtpAudioSocket;
    pMediaConnection->mpRtcpAudioSocket = rtcpAudioSocket;
    pMediaConnection->mRtpAudioReceivePort = rtpAudioSocket->getLocalHostPort() ;
    pMediaConnection->mRtcpAudioReceivePort = rtcpAudioSocket->getLocalHostPort() ;
    pMediaConnection->mpAudioSocketAdapter = new
            VoiceEngineSocketAdapter(pMediaConnection->mpRtpAudioSocket, pMediaConnection->mpRtcpAudioSocket) ;
    rc = mpVoiceEngine->GIPSVE_SetSendTransport(connectionId, *pMediaConnection->mpAudioSocketAdapter) ;
    assert(rc == 0);

    pMediaConnection->mpCodecFactory = new SdpCodecFactory(mSupportedCodecs);
    pMediaConnection->mpCodecFactory->bindPayloadTypes();

    UtlBoolean bAEC;
    mpFactoryImpl->isAudioAECEnabled(bAEC);
    if (bAEC)
    {
        int irc = 0;
        // Mode 1: echo cancellation enable, mode2: automatic
        if (0 == mpVoiceEngine->GIPSVE_SetECStatus(1))
        {
            // Type 0: echo canvcellation, type 1: echo suppression
            irc = mpVoiceEngine->GIPSVE_SetECType(0);
            assert(irc == 0);
        }
    }
    else
    {
        int rc = mpVoiceEngine->GIPSVE_SetECStatus(0);
        assert(rc == 0);
    }

    if (((VoiceEngineFactoryImpl*) mpFactoryImpl)->isMuted())
    {
        muteMicrophone(true);
    }

    return OS_SUCCESS ;
}


int VoiceEngineMediaInterface::getNumCodecs(int connectionId)
{
    OsLock lock(mVoiceEngineGuard) ;
    int iCodecs = 0;
    VoiceEngineMediaConnection*
        pMediaConn = getMediaConnection(connectionId);

    if (pMediaConn && pMediaConn->mpCodecFactory)
    {
        iCodecs = pMediaConn->mpCodecFactory->getCodecCount();
    }

    return iCodecs;
}


OsStatus VoiceEngineMediaInterface::getCapabilities(int connectionId,
                                                    UtlString& rtpHostAddress,
                                                    int& rtpAudioPort,
                                                    int& rtcpAudioPort,
                                                    int& rtpVideoPort,
                                                    int& rtcpVideoPort,
                                                    SdpCodecFactory& supportedCodecs,
                                                    SdpSrtpParameters& srtpParameters)
{
    OsLock lock(mVoiceEngineGuard) ;
#ifdef _WIN32
#ifdef VIDEO
    CoInitializeEx(NULL, 0);
#endif
#endif

    OsStatus rc = OS_FAILED ;
    UtlString ignored ;
    VoiceEngineMediaConnection* pMediaConn = getMediaConnection(connectionId);
    rtpAudioPort = 0 ;
    rtcpAudioPort = 0 ;
#ifdef VIDEO
    rtpVideoPort = 0 ;
    rtcpVideoPort = 0 ;
#endif

    if (pMediaConn)
    {
#ifdef VIDEO
        if (isDisplayValid(mpDisplay) && mbVideoStarted == false)
        {
            startVideoSupport(connectionId) ;
            mbVideoStarted = true;
        }
#endif

        if (    (pMediaConn->meContactType == AUTO) ||
                (pMediaConn->meContactType == NAT_MAPPED))
        {
            // Audio RTP
            if (pMediaConn->mpRtpAudioSocket)
            {
                // The "rtpHostAddress" is used for the rtp stream -- others
                // are ignored.  They *SHOULD* be the same as the first.
                // Possible exceptions: STUN worked for the first, but not the
                // others.  Not sure how to handle/recover from that case.
                if (mContactType == AUTO || mContactType == NAT_MAPPED)
                {
                    if (!pMediaConn->mpRtpAudioSocket->getExternalIp(&rtpHostAddress, &rtpAudioPort))
                    {
                        rtpAudioPort = pMediaConn->mRtpAudioReceivePort ;
                        rtpHostAddress = mRtpReceiveHostAddress ;
                    }
                }
                else if (mContactType == LOCAL)
                {
                     rtpHostAddress = pMediaConn->mpRtpAudioSocket->getLocalIp();
                     rtpAudioPort = pMediaConn->mpRtpAudioSocket->getLocalHostPort();
                     if (rtpAudioPort <= 0)
                     {
                         rtpAudioPort = pMediaConn->mRtpAudioReceivePort ;
                         rtpHostAddress = mRtpReceiveHostAddress ;
                     }
                }
                else
                {
                  assert(0);
                }

            }

            // Audio RTCP
            if (pMediaConn->mpRtcpAudioSocket)
            {
                if (mContactType == AUTO || mContactType == NAT_MAPPED)
                {
                    if (!pMediaConn->mpRtcpAudioSocket->getExternalIp(&ignored, &rtcpAudioPort))
                    {
                        rtcpAudioPort = pMediaConn->mRtcpAudioReceivePort ;
                    }
                    else
                    {
                        // External address should match that of Audio RTP
                        assert(ignored.compareTo(rtpHostAddress) == 0) ;
                    }
                }
                else if (mContactType == LOCAL)
                {
                    ignored = pMediaConn->mpRtcpAudioSocket->getLocalIp();
                    rtcpAudioPort = pMediaConn->mpRtcpAudioSocket->getLocalHostPort();
                    if (rtcpAudioPort <= 0)
                    {
                        rtcpAudioPort = pMediaConn->mRtcpAudioReceivePort ;
                    }
                }
                else
                {
                    assert(0);
                }
            }
#ifdef VIDEO
            // Video RTP
            if (pMediaConn->mpRtpVideoSocket)
            {
                if (mContactType == AUTO || mContactType == NAT_MAPPED)
                {
                    if (!pMediaConn->mpRtpVideoSocket->getExternalIp(&ignored, &rtpVideoPort))
                    {
                        rtpVideoPort = pMediaConn->mRtpVideoReceivePort ;
                    }
                    else
                    {
                        // External address should match that of Audio RTP
                        assert(ignored.compareTo(rtpHostAddress) == 0) ;
                    }
                }
                else if (mContactType == LOCAL)
                {
                     rtpHostAddress = pMediaConn->mpRtpVideoSocket->getLocalIp();
                     rtpVideoPort = pMediaConn->mpRtpVideoSocket->getLocalHostPort();
                     if (rtpVideoPort <= 0)
                     {
                         rtpVideoPort = pMediaConn->mRtpVideoReceivePort ;
                         rtpHostAddress = mRtpReceiveHostAddress ;
                     }
                }
                else
                {
                    assert(0);
                }
            }

            // Video RTCP
            if (pMediaConn->mpRtcpVideoSocket)
            {
                if (mContactType == AUTO || mContactType == NAT_MAPPED)
                {
                    if (!pMediaConn->mpRtcpVideoSocket->getExternalIp(&ignored, &rtcpVideoPort))
                    {
                        rtcpVideoPort = pMediaConn->mRtcpVideoReceivePort ;
                    }
                    else
                    {
                        // External address should match that of Audio RTP
                        assert(ignored.compareTo(rtpHostAddress) == 0) ;
                    }
                }
                else if  (mContactType == LOCAL)
                {
                    ignored = pMediaConn->mpRtcpVideoSocket->getLocalIp();
                    rtcpVideoPort = pMediaConn->mpRtcpVideoSocket->getLocalHostPort();
                    if (rtcpVideoPort <= 0)
                    {
                        rtcpVideoPort = pMediaConn->mRtcpVideoReceivePort ;
                    }
                }
                else
                {
                    assert(0);
                }
            }
#endif
        }
        else
        {
            rtpHostAddress = mRtpReceiveHostAddress ;
            rtpAudioPort = pMediaConn->mRtpAudioReceivePort ;
            rtcpAudioPort = pMediaConn->mRtcpAudioReceivePort ;
#ifdef VIDEO
            rtpVideoPort = pMediaConn->mRtpVideoReceivePort ;
            rtcpVideoPort = pMediaConn->mRtcpVideoReceivePort ;
#endif
        }

        // Codecs
        supportedCodecs = *(pMediaConn->mpCodecFactory);
        supportedCodecs.bindPayloadTypes();

        // Set up srtpParameters for testing
        srtpParameters.cipherType = AES_CM_128_HMAC_SHA1_80;
        //srtpParameters.securityLevel = ENCRYPTION_AND_AUTHENTICATION;
        srtpParameters.securityLevel = SRTP_OFF;
        strcpy((char*)srtpParameters.masterKey, "GoingDownToTheCadillacRanch!!!!");

        // VIDEO: Need to strip out Video codecs if pMediaConn->mpRtpVideoSocket is NULL

        rc = OS_SUCCESS ;
    }

    return rc ;
}


VoiceEngineMediaConnection* VoiceEngineMediaInterface::getMediaConnection(int connectionId)
{
   UtlInt matchConnectionId(connectionId) ;

   return ((VoiceEngineMediaConnection*) mMediaConnections.find(&matchConnectionId)) ;
}

OsStatus VoiceEngineMediaInterface::setConnectionDestination(int connectionId,
                                                             const char* remoteRtpHostAddress,
                                                             int remoteAudioRtpPort,
                                                             int remoteAudioRtcpPort,
                                                             int remoteVideoRtpPort,
                                                             int remoteVideoRtcpPort)
{
    OsLock lock(mVoiceEngineGuard) ;
    VoiceEngineMediaConnection* pMediaConnection = getMediaConnection(connectionId);
    if (pMediaConnection)
    {
        /*
         * Common setup
         */
        pMediaConnection->mDestinationSet = TRUE;
        pMediaConnection->mRtpSendHostAddress = remoteRtpHostAddress;

        /*
         * Audio setup
         */
        pMediaConnection->mRtpAudioSendHostPort = remoteAudioRtpPort;
        pMediaConnection->mpRtpAudioSocket->doConnect(remoteAudioRtpPort, remoteRtpHostAddress, TRUE);
        if ((remoteAudioRtcpPort > 0) && pMediaConnection->mpRtcpAudioSocket)
        {
            pMediaConnection->mRtcpAudioSendHostPort = remoteAudioRtcpPort;
            pMediaConnection->mpRtcpAudioSocket->doConnect(remoteAudioRtcpPort, remoteRtpHostAddress, TRUE);
            int rc = mpVoiceEngine->GIPSVE_EnableRTCP(connectionId, TRUE) ;
            assert(rc == 0) ;
        }
        else
        {
            pMediaConnection->mRtcpAudioSendHostPort = 0 ;
            int rc = mpVoiceEngine->GIPSVE_EnableRTCP(connectionId, FALSE) ;
            assert(rc == 0) ;
        }

        /*
         * Video Setup
         */

#ifdef VIDEO
        if (pMediaConnection->mpRtpVideoSocket)
        {
            pMediaConnection->mRtpVideoSendHostPort = remoteVideoRtpPort ;
            pMediaConnection->mpRtpVideoSocket->doConnect(remoteVideoRtpPort, remoteRtpHostAddress, TRUE) ;
            if ((remoteVideoRtcpPort > 0) && pMediaConnection->mpRtcpVideoSocket)
            {
                pMediaConnection->mRtcpVideoSendHostPort = remoteVideoRtcpPort ;
                pMediaConnection->mpRtcpVideoSocket->doConnect(remoteVideoRtcpPort, remoteRtpHostAddress, TRUE);
                // Calling EnableRTCP twice under Video Engine appears to fail
                // assert(rc == 0) ;
            }
            else
            {
                pMediaConnection->mRtcpVideoSendHostPort = 0 ;
                // Calling EnableRTCP twice under Video Engine appears to fail
                // assert(rc == 0) ;
            }
        }
        else
        {
            pMediaConnection->mRtpVideoSendHostPort = 0 ;
            pMediaConnection->mRtcpVideoSendHostPort = 0 ;
        }
#endif
    }

    return OS_SUCCESS ;
}

OsStatus VoiceEngineMediaInterface::addAlternateDestinations(int connectionId,
                                                             unsigned char cPriority,
                                                             const char* rtpHostAddress,
                                                             int port,
                                                             bool bRtp)
{
    OsLock lock(mVoiceEngineGuard) ;
    OsStatus returnCode = OS_NOT_FOUND;
    VoiceEngineMediaConnection* mediaConnection = getMediaConnection(connectionId);
    if (mediaConnection)
    {
        if (bRtp)
        {
            if (mediaConnection->mpRtpAudioSocket)
            {
                mediaConnection->mpRtpAudioSocket->addAlternateDestination(
                        rtpHostAddress, port,
                        cPriority) ;
                returnCode = OS_SUCCESS;
            }
        }
        else
        {
            if (mediaConnection->mpRtcpAudioSocket)
            {
                mediaConnection->mpRtcpAudioSocket->addAlternateDestination(
                        rtpHostAddress, port,
                        cPriority) ;
                returnCode = OS_SUCCESS;
            }
        }
    }

    return returnCode ;
}

OsStatus VoiceEngineMediaInterface::startRtpSend(int connectionId,
                                                 int numCodecs,
                                                 SdpCodec* sendCodecs[],
                                                 SdpSrtpParameters& srtpParams)
{
    OsLock lock(mVoiceEngineGuard) ;
    int i, rc;
    SdpCodec* primaryCodec = NULL;
    SdpCodec* primaryVideoCodec = NULL;
    SdpCodec* dtmfCodec = NULL;
    GIPSVE_CodecInst codecInfo;
#ifdef VIDEO
    GIPSVideo_CodecInst vcodecInfo;
    memset((void*)&vcodecInfo, 0, sizeof(vcodecInfo));
#endif
    UtlString mimeType;

#ifdef _WIN32
#ifdef VIDEO
    CoInitializeEx(NULL, 0);
#endif
#endif

    VoiceEngineMediaConnection* pMediaConnection = getMediaConnection(connectionId);
    if (pMediaConnection && !pMediaConnection->mRtpSendingAudio && !pMediaConnection->mRtpSendingVideo)
    {
        for (i=0; i<numCodecs; i++) {
            sendCodecs[i]->getMediaType(mimeType);
            if (SdpCodec::SDP_CODEC_TONES == sendCodecs[i]->getValue())
            {
                if (NULL == dtmfCodec)
                {
                    dtmfCodec = sendCodecs[i];
                }
            }
            else if (NULL == primaryCodec && mimeType.compareTo("audio") == 0)
            {
                primaryCodec = sendCodecs[i];
                pMediaConnection->mpPrimaryCodec = primaryCodec;
            }
#ifdef VIDEO
            else if (NULL == primaryVideoCodec && mimeType.compareTo("video") == 0)
            {
                primaryVideoCodec = sendCodecs[i];
                mPrimaryVideoCodec = primaryVideoCodec;
                pMediaConnection->mpPrimaryVideoCodec = primaryVideoCodec;
            }
#endif
        }

        rc = mpVoiceEngine->GIPSVE_SetSendPort(connectionId, pMediaConnection->mRtpAudioSendHostPort) ;
        assert(rc == 0);
        rc = mpVoiceEngine->GIPSVE_SetSendIP(connectionId, (char*) pMediaConnection->mRtpSendHostAddress.data()) ;
        assert(rc == 0);


        if (primaryCodec && getVoiceEngineCodec(*primaryCodec, codecInfo))
        {
            OsSysLog::add(FAC_MP, PRI_DEBUG,
                          "startRtpSend: using GIPS codec %s for id %d, payload %d",
                          codecInfo.plname, primaryCodec->getCodecType(),
                          primaryCodec->getCodecPayloadFormat());
            pMediaConnection->mRtpPayloadType = primaryCodec->getCodecPayloadFormat();
            // Forcing VoiceEngine to use our dynamically allocated payload types
            codecInfo.pltype = primaryCodec->getCodecPayloadFormat();
            if ((rc=mpVoiceEngine->GIPSVE_SetSendCodec(connectionId, &codecInfo)) == -1)
            {
                i = mpVoiceEngine->GIPSVE_GetLastError();
                OsSysLog::add(FAC_MP, PRI_DEBUG,
                              "startRtpSend: SetSendCodec failed with code %d", i);
                assert(0);
            }
            if (dtmfCodec)
            {
                rc = mpVoiceEngine->GIPSVE_SetDTMFPayloadType(connectionId, dtmfCodec->getCodecPayloadFormat());
                assert(rc == 0);
            }
            pMediaConnection->mRtpSendingAudio = TRUE ;
        }
#ifdef VIDEO
        if (mbVideoChannelEstablishedForSend)
        {
            restartRtpSendVideo(pMediaConnection->mVideoConnectionId);
        }
        else
        {
            if (primaryVideoCodec && getVideoEngineCodec(*primaryVideoCodec, vcodecInfo))
            {
                pMediaConnection->mRtpVideoPayloadType = primaryVideoCodec->getCodecPayloadFormat();
                //vcodecInfo.pltype = primaryVideoCodec->getCodecPayloadFormat();
                if (primaryVideoCodec->getVideoFormat() == SDP_VIDEO_FORMAT_CIF)
                {
                    vcodecInfo.height = 288;
                    vcodecInfo.width = 352;
                }
                else if (primaryVideoCodec->getVideoFormat() == SDP_VIDEO_FORMAT_QCIF)
                {
                    vcodecInfo.height = 144;
                    vcodecInfo.width = 176;
                }
                else if (primaryVideoCodec->getVideoFormat() == SDP_VIDEO_FORMAT_SQCIF)
                {
                    vcodecInfo.height = 96;
                    vcodecInfo.width = 128;
                }

                mPrimaryVideoCodec = primaryVideoCodec;

                if (pMediaConnection->mpRtpVideoSocket)
                {
                    pNetTask->addInputSource(pMediaConnection->mpRtpVideoSocket) ;
                }

                if (pMediaConnection->mpRtcpVideoSocket)
                {
                    pNetTask->addInputSource(pMediaConnection->mpRtcpVideoSocket) ;
                }

                if (isDisplayValid(mpDisplay))
                {
                    rc = pMediaConnection->mVideoConnectionId = mpVideoEngine->GIPSVideo_CreateChannel(connectionId) ;
                    if (rc)
                    {
                        int check = mpVideoEngine->GIPSVideo_GetLastError() ;
                        assert(0) ;
                    }
                    else
                    {
                        mbVideoChannelEstablishedForSend = true;
                    }
                    OutputDebugString("GIPSVideo_CreateChannel\n");
                }

                if (isDisplayValid(mpDisplay) && pMediaConnection->mpRtpVideoSocket)
                {
                    pMediaConnection->mpRtpVideoSocket->setVideoChannel(pMediaConnection->mVideoConnectionId);
                }
                if (isDisplayValid(mpDisplay) && pMediaConnection->mpRtcpVideoSocket)
                {
                    pMediaConnection->mpRtcpVideoSocket->setVideoChannel(pMediaConnection->mVideoConnectionId);
                }

                if (isDisplayValid(mpDisplay))
                {
                    rc = mpVideoEngine->GIPSVideo_SetSendTransport(pMediaConnection->mVideoConnectionId, *pMediaConnection->mpVideoSocketAdapter) ;
                    assert(rc == 0);
                    OutputDebugString("GIPSVideo_SetSendTransport\n");
                }
                rc = mpVoiceEngine->GIPSVE_EnableRTCP(connectionId, TRUE) ;

                startRtpReceiveVideo(connectionId);

                if (isDisplayValid(mpDisplay))
                {
                    // get the first capture device
                    char szDevice[256];
                    rc = mpVideoEngine->GIPSVideo_GetCaptureDevice(0, szDevice, sizeof(szDevice));

                    rc = mpVideoEngine->GIPSVideo_SetCaptureDevice(szDevice, sizeof(szDevice));
                    OutputDebugString("GIPSVideo_SetCaptureDevice\n");
                    int check = mpVideoEngine->GIPSVideo_GetLastError() ;
                    if (rc == 0)
                    {

                        // TEMP FIX - hard coding the codec to VP71 - QCIF
                        rc = mpVideoEngine->GIPSVideo_GetCodec(0, &vcodecInfo);
                        check = mpVideoEngine->GIPSVideo_GetLastError();
                        assert (rc == 0);

                        mpFactoryImpl->getVideoQuality(vcodecInfo.quality);
                        mpFactoryImpl->getVideoBitRate(vcodecInfo.bitRate);
                        mpFactoryImpl->getVideoFrameRate(vcodecInfo.frameRate);

                        if ((rc=mpVideoEngine->GIPSVideo_SetSendCodec(&vcodecInfo)) == -1)
                        {
                            i = mpVideoEngine->GIPSVideo_GetLastError();
                            OsSysLog::add(FAC_MP, PRI_DEBUG,
                                            "startRtpSend: SetSendCodec for video failed with code %d", i);
                            assert(0);
                        }
                        OutputDebugString("GIPSVideo_SetSendCodec\n");

                #ifdef _WIN32
                        SIPXVE_VIDEO_DISPLAY* pDisplay = NULL;

                        pDisplay = (SIPXVE_VIDEO_DISPLAY*)((VoiceEngineFactoryImpl*)mpFactoryImpl)->getVideoPreviewDisplay();

                        if (pDisplay && pDisplay->type == SIPXVE_WINDOW_HANDLE_TYPE)
                        {
                            rc = mpVideoEngine->GIPSVideo_AddLocalRenderer((HWND) pDisplay->handle);
                            check = mpVideoEngine->GIPSVideo_GetLastError() ;
                            assert(rc == 0);
                            //assert(check == 0) ;
                            OutputDebugString("GIPSVideo_AddLocalRenderer\n");
                        }
                        else if (pDisplay && pDisplay->type == SIPXVE_DIRECT_SHOW_FILTER)
                        {
                            rc = mpVideoEngine->GIPSVideo_AddLocalRenderer((IBaseFilter*) pDisplay->filter);
                            check = mpVideoEngine->GIPSVideo_GetLastError() ;
                            assert(rc == 0);
                            //assert(check == 0) ;
                            OutputDebugString("GIPSVideo_AddLocalRenderer\n");
                        }

                        int rc = mpVideoEngine->GIPSVideo_EnableRTCP(pMediaConnection->mVideoConnectionId, TRUE) ;
                        OutputDebugString("GIPSVideo_EnableRTCP\n");


                #endif

                        if (primaryVideoCodec)
                        {
                            pMediaConnection->mRtpVideoPayloadType = primaryVideoCodec->getCodecPayloadFormat();

                        }
	                    rc = mpVideoEngine->GIPSVideo_StartSend(pMediaConnection->mVideoConnectionId) ;
                        assert(rc == 0) ;
                        OutputDebugString("GIPSVideo_StartSend\n");

                        pMediaConnection->mRtpSendingVideo = TRUE ;
                    }
                }
            }
        }
#endif
        /* for testing - please do not remove until SRTP has been verified */
        if (srtpParams.securityLevel & SRTP_SEND)
        {
            int retVal = mpVoiceEngine->GIPSVE_EnableSRTPSend(connectionId, srtpParams.cipherType, 30,
                                                              AUTH_HMAC_SHA1, 16, 4, srtpParams.securityLevel&SRTP_SECURITY_MASK,
                                                              srtpParams.masterKey);
            int err = mpVoiceEngine->GIPSVE_GetLastError();
            assert(retVal==0);
#ifdef VIDEO
            retVal = mpVideoEngine->GIPSVideo_EnableSRTPSend(connectionId, srtpParams.cipherType, 30,
                                                             AUTH_HMAC_SHA1, 16, 4, srtpParams.securityLevel&SRTP_SECURITY_MASK,
                                                             srtpParams.masterKey);
            err = mpVideoEngine->GIPSVideo_GetLastError();
            assert(retVal==0);
#endif /* VIDEO */
        }

        rc = mpVoiceEngine->GIPSVE_StartSend(connectionId) ;

        assert(rc == 0);

        rc = mpVoiceEngine->GIPSVE_AddToConference(connectionId, true) ;
        assert(rc == 0);
    }

    return OS_SUCCESS ;
}

void VoiceEngineMediaInterface::restartRtpSendVideo(int connectionId)
{
#ifdef VIDEO
    startRtpReceiveVideo(connectionId);

	int rc = mpVideoEngine->GIPSVideo_StartSend(connectionId) ;
    OutputDebugString("GIPSVideo_StartSend\n");
    int check = mpVideoEngine->GIPSVideo_GetLastError();
    assert(rc == 0) ;

    VoiceEngineMediaConnection* pMediaConnection = getMediaConnection(connectionId) ;
    pMediaConnection->mRtpSendingVideo = true;

    rc = mpVideoEngine->GIPSVideo_StartRender(connectionId) ;
    OutputDebugString("GIPSVideo_StartRender\n");
    check = mpVideoEngine->GIPSVideo_GetLastError();
    assert(rc == 0) ;
#endif /* VIDEO */
}

OsStatus VoiceEngineMediaInterface::startRtpReceive(int connectionId,
                                                    int numCodecs,
                                                    SdpCodec* receiveCodecs[],
                                                    SdpSrtpParameters& srtpParams)
{
    OsLock lock(mVoiceEngineGuard) ;
#ifdef _WIN32
#ifdef VIDEO
    CoInitializeEx(NULL, 0);
#endif
#endif

    int rc;
    int i;
    SdpCodec* primaryCodec = NULL;
    SdpCodec* dtmfCodec = NULL;
    GIPSVE_CodecInst codecInfo;
    UtlString mimeType;

    VoiceEngineMediaConnection* pMediaConnection = getMediaConnection(connectionId) ;
    if (pMediaConnection && !pMediaConnection->mRtpReceivingAudio && !pMediaConnection->mRtpReceivingVideo)
    {
        pMediaConnection->mpPrimaryCodec = NULL;
        if (pMediaConnection->mpCodecFactory)
        {
            pMediaConnection->mpCodecFactory->copyPayloadTypes(numCodecs,
                                                               receiveCodecs);
        }
        for (i=0; i<numCodecs; i++)
        {
            receiveCodecs[i]->getMediaType(mimeType);
            if (SdpCodec::SDP_CODEC_TONES == receiveCodecs[i]->getValue())
            {
                if (NULL == dtmfCodec)
                {
                    dtmfCodec = receiveCodecs[i];
                    rc = mpVoiceEngine->GIPSVE_SetDTMFPayloadType(connectionId,
                                                                  dtmfCodec->getCodecPayloadFormat());
                    assert(rc == 0);
                }
            }
            else if (NULL == primaryCodec && mimeType.compareTo("audio") == 0)
            {
                primaryCodec = receiveCodecs[i];
                if (getVoiceEngineCodec(*primaryCodec, codecInfo))
                {
                    // Stop playout here just in case we had a previous giveFocus call. That
                    // will make the SetRecPayloadType call fail. Playout will be resumed
                    // further down.
                    rc = mpVoiceEngine->GIPSVE_StopPlayout(connectionId);
                    assert(rc == 0);

                    if (primaryCodec)
                    {
                        codecInfo.pltype = primaryCodec->getCodecPayloadFormat();
                    }

                    rc = mpVoiceEngine->GIPSVE_SetRecPayloadType(connectionId, &codecInfo) ;
                    assert(rc == 0);
                }
                pMediaConnection->mRtpReceivingAudio = TRUE ;
            }
            else if (NULL == primaryCodec && mimeType.compareTo("video") == 0)
            {
                mPrimaryVideoCodec = receiveCodecs[i];
            }
        }

        if (pMediaConnection->mpRtpAudioSocket)
        {
            pNetTask->addInputSource(pMediaConnection->mpRtpAudioSocket) ;
        }

        if (pMediaConnection->mpRtcpAudioSocket)
        {
            pNetTask->addInputSource(pMediaConnection->mpRtcpAudioSocket) ;
        }

        if (mbFocus)
        {
#ifdef SIPXTAPI_USE_GIPS_TRANSPORT
		    if (pMediaConnection)
		    {
                int rc = mpVoiceEngine->GIPSVE_SetRecPort(connectionId, pMediaConnection->mRtpAudioReceivePort + 10);
                assert(rc == 0);
                int i = mpVoiceEngine->GIPSVE_GetLastError();
		    }

            // This call only has meaning using internal transport
            rc = mpVoiceEngine->GIPSVE_StartListen(connectionId) ;
            assert(rc == 0);
#endif

            if (srtpParams.securityLevel & SRTP_RECEIVE)
            {
                int retVal = mpVoiceEngine->GIPSVE_EnableSRTPReceive(connectionId, srtpParams.cipherType, 30, AUTH_HMAC_SHA1,
                                                                     16, 4, srtpParams.securityLevel&SRTP_SECURITY_MASK,
                                                                     srtpParams.masterKey);
                int err = mpVoiceEngine->GIPSVE_GetLastError();
                assert(retVal == 0);
#ifdef VIDEO
                retVal = mpVideoEngine->GIPSVideo_EnableSRTPReceive(connectionId, srtpParams.cipherType, 30, AUTH_HMAC_SHA1,
                                                                    16, 4, srtpParams.securityLevel&SRTP_SECURITY_MASK,
                                                                    srtpParams.masterKey);
                err = mpVideoEngine->GIPSVideo_GetLastError();
                assert(retVal == 0);
#endif /* VIDEO */
            }

            rc = mpVoiceEngine->GIPSVE_StartPlayout(connectionId) ;
            OutputDebugString("GIPSVideo_StartPlayout\n");
            assert(rc == 0) ;
        }
    }

    return OS_SUCCESS ;
}

void VoiceEngineMediaInterface::startRtpReceiveVideo(int connectionId)
{
#ifdef VIDEO
    OsLock lock(mVoiceEngineGuard) ;
    int rc;
    int i;
    SdpCodec* primaryVideoCodec = NULL;
    GIPSVideo_CodecInst vcodecInfo;

    VoiceEngineMediaConnection* pMediaConnection = getMediaConnection(connectionId) ;

    primaryVideoCodec = mPrimaryVideoCodec;
    if (getVideoEngineCodec(*primaryVideoCodec, vcodecInfo))
    {
        //vcodecInfo.pltype = primaryVideoCodec->getCodecPayloadFormat();
        if (primaryVideoCodec->getVideoFormat() == SDP_VIDEO_FORMAT_CIF)
        {
            vcodecInfo.height = 288;
            vcodecInfo.width = 352;
        }
        else if (primaryVideoCodec->getVideoFormat() == SDP_VIDEO_FORMAT_QCIF)
        {
            vcodecInfo.height = 144;
            vcodecInfo.width = 176;
        }
        else if (primaryVideoCodec->getVideoFormat() == SDP_VIDEO_FORMAT_SQCIF)
        {
            vcodecInfo.height = 96;
            vcodecInfo.width = 128;
        }
    }

    // TEMP FIX - hard coding the codec to VP71 - QCIF
    if (isDisplayValid(mpDisplay))
    {
        if (!mbVideoChannelEstablishedReceiving)
        {
            rc = mpVideoEngine->GIPSVideo_GetCodec(0, &vcodecInfo);
            int check = mpVideoEngine->GIPSVideo_GetLastError();
            assert (rc == 0);
            //assert (check == 0);

            mpFactoryImpl->getVideoQuality(vcodecInfo.quality);
            mpFactoryImpl->getVideoBitRate(vcodecInfo.bitRate);
            mpFactoryImpl->getVideoFrameRate(vcodecInfo.frameRate);

	        rc = mpVideoEngine->GIPSVideo_SetReceiveCodec(pMediaConnection->mVideoConnectionId, &vcodecInfo);
            check = mpVideoEngine->GIPSVideo_GetLastError() ;
            //assert(check == 0) ;
            assert(rc == 0) ;

            OutputDebugString("GIPSVideo_SetReceiveCodec\n");
            if (mpDisplay->type == SIPXVE_WINDOW_HANDLE_TYPE)
            {
                rc = mpVideoEngine->GIPSVideo_AddRemoteRenderer(pMediaConnection->mVideoConnectionId,
                        (HWND) mpDisplay->handle);
                assert(rc == 0) ;
                OutputDebugString("GIPSVideo_AddRemoteRenderer\n");
            }
            else if (mpDisplay->type == SIPXVE_DIRECT_SHOW_FILTER)
            {
#if defined (_WIN32) && defined (VIDEO)

                rc = mpVideoEngine->GIPSVideo_AddRemoteRenderer(pMediaConnection->mVideoConnectionId,
                        (IBaseFilter*) mpDisplay->filter);
                assert(rc == 0) ;
                OutputDebugString("GIPSVideo_AddRemoteRenderer\n");
#endif
            }
            rc = mpVideoEngine->GIPSVideo_StartRender(pMediaConnection->mVideoConnectionId) ;
            assert(rc == 0) ;
            OutputDebugString("GIPSVideo_StartRender\n");

            mbVideoChannelEstablishedReceiving = true;
        }

        pMediaConnection->mRtpReceivingVideo = TRUE ;
    }

#endif
}

OsStatus VoiceEngineMediaInterface::stopRtpSend(int connectionId)
{
    OsLock lock(mVoiceEngineGuard) ;
    int iRC ;

    VoiceEngineMediaConnection* pMediaConnection = getMediaConnection(connectionId) ;

    if (pMediaConnection && pMediaConnection->mRtpSendingAudio)
    {
        pMediaConnection->mRtpSendingAudio = FALSE ;

        iRC = mpVoiceEngine->GIPSVE_StopSend(connectionId) ;
        assert(iRC == 0) ;

        iRC = mpVoiceEngine->GIPSVE_AddToConference(connectionId, false) ;
        assert(iRC == 0) ;
    }

#ifdef VIDEO
    if (pMediaConnection && pMediaConnection->mRtpSendingVideo)
    {
        pMediaConnection->mRtpSendingVideo = FALSE ;

        if (pMediaConnection->mpRtpVideoSocket)
        {
            iRC = mpVideoEngine->GIPSVideo_StopSend(pMediaConnection->mVideoConnectionId) ;
            assert(iRC == 0) ;
            OutputDebugString("GIPSVideo_StopSend\n");
        }
    }
#endif

    return OS_SUCCESS ;
}

OsStatus VoiceEngineMediaInterface::stopRtpReceive(int connectionId)
{
    OsLock lock(mVoiceEngineGuard) ;
    int iRC ;

    VoiceEngineMediaConnection* pMediaConnection = getMediaConnection(connectionId) ;
    if (pMediaConnection)
    {
        if (pMediaConnection->mRtpReceivingAudio)
        {
            pMediaConnection->mRtpReceivingAudio = FALSE ;

            iRC = mpVoiceEngine->GIPSVE_StopListen(connectionId) ;
            assert(iRC == 0);
            iRC = mpVoiceEngine->GIPSVE_StopPlayout(connectionId) ;
            assert(iRC == 0) ;
        }
#ifdef VIDEO
        if (pMediaConnection->mRtpReceivingVideo)
        {
            pMediaConnection->mRtpReceivingVideo = FALSE ;
            if (pMediaConnection->mpRtpVideoSocket)
            {
                iRC = mpVideoEngine->GIPSVideo_StopRender(pMediaConnection->mVideoConnectionId) ;
                // not an assertion for now.  As it is now, we may have called a phone
                // which does not support video, so StopRender should fail in that case
                //assert(iRC == 0) ;
                OutputDebugString("GIPSVideo_StopRender\n");
            }
        }
#endif
    }
    return OS_SUCCESS ;
}


OsStatus VoiceEngineMediaInterface::deleteConnection(int connectionId)
{
    OsLock lock(mVoiceEngineGuard) ;
    OsStatus rc = OS_NOT_FOUND ;
    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    VoiceEngineNetTask* pNetTask = NULL;
    OsTime maxEventTime(20, 0);

    pNetTask = VoiceEngineNetTask::getVoiceEngineNetTask();

    VoiceEngineMediaConnection* pMediaConnection = getMediaConnection(connectionId) ;
    if (pMediaConnection)
    {
        stopRtpSend(connectionId) ;
        stopRtpReceive(connectionId) ;

        if (pNetTask)
        {
            // Audio RTP
            OsProtectedEvent* rtpSocketRemoveEvent = eventMgr->alloc();
            pNetTask->removeInputSource(pMediaConnection->mpRtpAudioSocket, rtpSocketRemoveEvent) ;
            if (rtpSocketRemoveEvent->wait(0, maxEventTime) != OS_SUCCESS)
            {
                OsSysLog::add(FAC_MP, PRI_ERR,
                        " *** VoiceEngineMediaInterface: failed to wait for audio rtp socket release") ;
            }
            eventMgr->release(rtpSocketRemoveEvent);
            mpFactoryImpl->releaseRtpPort(pMediaConnection->mRtpAudioReceivePort) ;

            // Audio RTCP
            if (pMediaConnection->mpRtcpAudioSocket)
            {
                OsProtectedEvent* rtcpSocketRemoveEvent = eventMgr->alloc();
                pNetTask->removeInputSource(pMediaConnection->mpRtcpAudioSocket, rtcpSocketRemoveEvent) ;
                if (rtcpSocketRemoveEvent->wait(0, maxEventTime) != OS_SUCCESS)
                {
                    OsSysLog::add(FAC_MP, PRI_ERR,
                            " *** VoiceEngineMediaInterface: failed to wait for audio rtpc socket release") ;
                }
                eventMgr->release(rtcpSocketRemoveEvent);
            }
#ifdef VIDEO
            // Audio RTP
            if (pMediaConnection->mpRtpVideoSocket)
            {
                OsProtectedEvent* rtpVideoSocketRemoveEvent = eventMgr->alloc();
                pNetTask->removeInputSource(pMediaConnection->mpRtpVideoSocket, rtpVideoSocketRemoveEvent) ;
                if (rtpVideoSocketRemoveEvent->wait(0, maxEventTime) != OS_SUCCESS)
                {
                    OsSysLog::add(FAC_MP, PRI_ERR,
                            " *** VoiceEngineMediaInterface: failed to wait for video rtp socket release") ;
                }
                eventMgr->release(rtpVideoSocketRemoveEvent);
                mpFactoryImpl->releaseRtpPort(pMediaConnection->mRtpVideoReceivePort) ;
            }

            // Audio RTCP
            if (pMediaConnection->mpRtcpVideoSocket)
            {
                OsProtectedEvent* rtcpVideoSocketRemoveEvent = eventMgr->alloc();
                pNetTask->removeInputSource(pMediaConnection->mpRtcpVideoSocket, rtcpVideoSocketRemoveEvent) ;
                if (rtcpVideoSocketRemoveEvent->wait(0, maxEventTime) != OS_SUCCESS)
                {
                    OsSysLog::add(FAC_MP, PRI_ERR,
                            " *** VoiceEngineMediaInterface: failed to wait for video rtpc socket release") ;
                }
                eventMgr->release(rtcpVideoSocketRemoveEvent);
            }
#endif
        }
        // Remove Audio Channel

        mMediaConnections.remove(&UtlInt(connectionId)) ;
        int iRC = mpVoiceEngine->GIPSVE_DeleteChannel(connectionId) ;
        assert(iRC == 0);

        // Remove Video Channel
#ifdef VIDEO
        if (pMediaConnection->mVideoConnectionId >= 0)
        {
            iRC = mpVideoEngine->GIPSVideo_DeleteChannel(pMediaConnection->mVideoConnectionId) ;
            assert(iRC == 0);
            OutputDebugString("GIPSVideo_DeleteChannel\n");
        }
#endif
        delete pMediaConnection ;
        rc = OS_SUCCESS ;
    }

    return rc ;
}


OsStatus VoiceEngineMediaInterface::startDtmf(char toneId,
                                          UtlBoolean local,
                                          UtlBoolean remote)
{
    return OS_NOT_SUPPORTED ;
}

OsStatus VoiceEngineMediaInterface::stopDtmf()
{
    return OS_NOT_SUPPORTED ;
}

OsStatus VoiceEngineMediaInterface::playAudio(const char* url,
                                          UtlBoolean repeat,
                                          UtlBoolean local,
                                          UtlBoolean remote)
{
    OsLock lock(mVoiceEngineGuard) ;
    assert(url);
    int iRC;

    OsStatus rc = OS_FAILED;
    VoiceEngineMediaConnection* pMediaConnection = NULL;
    int fileFormat;
    char sBuffer[5];

    FILE *fp;
    if ((fp=fopen(url,"rb")) != NULL)
    {
        // Determine if file is PCM or WAV
        memset(sBuffer, 0, 5);
        fgets(sBuffer, 5, fp);
        if (strcmp(sBuffer, "RIFF") == 0)
        {
            fileFormat = FILE_FORMAT_WAV_FILE;
        }
        else
        {
            fileFormat = FILE_FORMAT_PCM_FILE;
        }
        fclose(fp);

        pMediaConnection = (VoiceEngineMediaConnection *)mMediaConnections.first();
        if (pMediaConnection)
        {
            int connectionId = pMediaConnection->getValue();
            iRC = 0 ;

            // If remote is set then play file as if it came from microphone
            if (remote &&
                (iRC=mpVoiceEngine->GIPSVE_PlayPCMAsMicrophone(connectionId, (char*)url, (repeat==TRUE) ? true : false, true, fileFormat)) == 0)
            {
                rc = OS_SUCCESS;
            }
            assert(iRC == 0);

        }

        // If local is set then play it locally
        if (local)
        {
            if (mpFactoryImpl)
            {
                ((VoiceEngineFactoryImpl*) mpFactoryImpl)->startPlayFile(url, (repeat==TRUE) ? true : false) ;
            }
        }
    }
    else
    {
        rc = OS_FAILED;
    }


    return rc;
}

OsStatus VoiceEngineMediaInterface::playBuffer(char* buf,
                                           unsigned long bufSize,
                                           int type,
                                           UtlBoolean repeat,
                                           UtlBoolean local,
                                           UtlBoolean remote,
                                           OsProtectedEvent* event)
{
    return OS_NOT_SUPPORTED ;
}

OsStatus VoiceEngineMediaInterface::pauseAudio()
{
    return OS_NOT_SUPPORTED ;
}

OsStatus VoiceEngineMediaInterface::stopAudio()
{
    OsLock lock(mVoiceEngineGuard) ;
    OsStatus rc = OS_FAILED;
    VoiceEngineMediaConnection* pMediaConnection = NULL;

    pMediaConnection = (VoiceEngineMediaConnection *)mMediaConnections.first();

    if (pMediaConnection)
    {
        int connectionId = pMediaConnection->getValue();

        if (mpVoiceEngine->GIPSVE_IsPlayingFileAsMicrophone(connectionId) == 1)
        {
            if (mpVoiceEngine->GIPSVE_StopPlayingFileAsMicrophone(connectionId) == -1)
            {
                OsSysLog::add(FAC_MP, PRI_ERR,
                             "stopAudio: GIPSVE_StopPlayingFileAsMicrophone on channel %d failed with error %d",
                             connectionId,
                             mpVoiceEngine->GIPSVE_GetLastError());
                assert(0);
            }
            else
            {
                rc = OS_SUCCESS;
            }
        }
    }

    if (mpFactoryImpl)
    {
        ((VoiceEngineFactoryImpl*) mpFactoryImpl)->stopPlayFile() ;
    }

    return rc;
}


OsStatus VoiceEngineMediaInterface::createPlayer(MpStreamPlayer** ppPlayer,
                                             const char* szStream,
                                             int flags,
                                             OsMsgQ *pMsgQ,
                                             const char* szTarget)
{
    return OS_NOT_SUPPORTED ;
}


OsStatus VoiceEngineMediaInterface::destroyPlayer(MpStreamPlayer* pPlayer)
{
    return OS_NOT_SUPPORTED ;
}


OsStatus VoiceEngineMediaInterface::createPlaylistPlayer(MpStreamPlaylistPlayer** ppPlayer,
                                                     OsMsgQ *pMsgQ,
                                                     const char* szTarget)
{
    return OS_NOT_SUPPORTED ;
}


OsStatus VoiceEngineMediaInterface::destroyPlaylistPlayer(MpStreamPlaylistPlayer* pPlayer)
{
    return OS_NOT_SUPPORTED ;
}


OsStatus VoiceEngineMediaInterface::createQueuePlayer(MpStreamQueuePlayer** ppPlayer,
                                                  OsMsgQ *pMsgQ,
                                                  const char* szTarget)
{
    return OS_NOT_SUPPORTED ;
}


OsStatus VoiceEngineMediaInterface::destroyQueuePlayer(MpStreamQueuePlayer* pPlayer)
{
    return OS_NOT_SUPPORTED ;
}


OsStatus VoiceEngineMediaInterface::startTone(int toneId,
                                              UtlBoolean local,
                                              UtlBoolean remote)
{
    OsLock lock(mVoiceEngineGuard) ;
    OsStatus rc = OS_FAILED;
    VoiceEngineMediaConnection* pMediaConnection = NULL ;
    int err = 0;

    int n = mMediaConnections.entries();
    pMediaConnection = (VoiceEngineMediaConnection *)mMediaConnections.first();

    if (local && !remote)
    {
        if (mpFactoryImpl)
        {
            ((VoiceEngineFactoryImpl*) mpFactoryImpl)->playTone(toneId) ;
        }
    }


    if (pMediaConnection != NULL)
    {
        if (remote)
        {
            int connectionId = pMediaConnection->getValue();

            // Try out-of-band first if enabled
            if (mbDTMFOutOfBand)
            {
                if (mpVoiceEngine->GIPSVE_SendDTMF(connectionId, toneId, 0) == 0)
                {
                    rc = OS_SUCCESS;
                }
                else
                {
                    OsSysLog::add(FAC_MP, PRI_ERR,
                              "startTone: out-of-band SendDTMF with event nr %d returned error %d",
                              toneId, err);
                    assert(0);
                }
            }

            // Then send inband DTMF
            if (mpVoiceEngine->GIPSVE_SendDTMF(connectionId, toneId, 1) == 0)
            {
                rc = OS_SUCCESS;
            }
            else
            {
                // Don't log error for inband flash hook
                if (toneId != 16)
                {
                    err = mpVoiceEngine->GIPSVE_GetLastError();
                    OsSysLog::add(FAC_MP, PRI_ERR,
                              "startTone: inband SendDTMF with event nr %d returned error %d",
                              toneId, err);
                    assert(0);
                    rc = OS_FAILED;
                }
            }
        }
    }

    return rc;
}


OsStatus VoiceEngineMediaInterface::stopTone()
{
    return OS_NOT_SUPPORTED ;

}

OsStatus VoiceEngineMediaInterface::giveFocus()
{
    OsLock lock(mVoiceEngineGuard) ;
#ifdef _WIN32
#ifdef VIDEO
    CoInitializeEx(NULL, 0);
#endif
#endif

    mbFocus = TRUE ;
    OsStatus rc = OS_FAILED;
    VoiceEngineMediaConnection* pMediaConnection = NULL ;

    UtlSListIterator iterator(mMediaConnections);
    while ((pMediaConnection = (VoiceEngineMediaConnection*) iterator()))    {
        int iRC ;
        iRC = mpVoiceEngine->GIPSVE_StartPlayout(pMediaConnection->getValue()) ;
        assert(iRC == 0);
        if (iRC == 0)
        {
            rc = OS_SUCCESS;
        }
#ifdef VIDEO
        if (pMediaConnection->mRtpReceivingVideo)
        {
            iRC = mpVideoEngine->GIPSVideo_StartRender(pMediaConnection->mVideoConnectionId) ;
            assert(iRC ==0);
            OutputDebugString("GIPSVideo_StartRender (giveFocus)\n");
        }
#endif
    }

    if (mbLocalMute)
    {
        mbLocalMute = false ;
        if (!((VoiceEngineFactoryImpl*) mpFactoryImpl)->isMuted())
        {
            muteMicrophone(false);
        }
    }


    return rc;
}


OsStatus VoiceEngineMediaInterface::defocus()
{
    OsLock lock(mVoiceEngineGuard) ;
    int rc;
    mbFocus = FALSE ;
    VoiceEngineMediaConnection* pMediaConnection = NULL ;

    UtlSListIterator iterator(mMediaConnections);
    while ((pMediaConnection = (VoiceEngineMediaConnection*) iterator()))
    {
        rc = mpVoiceEngine->GIPSVE_StopPlayout(pMediaConnection->getValue()) ;
        assert(rc ==0);
#ifdef VIDEO
        if (pMediaConnection->mRtpReceivingVideo)
        {
            rc = mpVideoEngine->GIPSVideo_StopRender(pMediaConnection->mVideoConnectionId) ;
            assert(rc ==0);
            OutputDebugString("GIPSVideo_StopRender (defocus)\n");
        }
#endif
    }

    mbLocalMute = true ;
    muteMicrophone(true);

    return OS_SUCCESS;
}


// Limits the available codecs to only those within the designated limit.
void VoiceEngineMediaInterface::setCodecCPULimit(int iLimit)
{

}

OsStatus VoiceEngineMediaInterface::stopRecording()
{
    return OS_SUCCESS ;
}


OsStatus VoiceEngineMediaInterface::ezRecord(int ms,
                                         int silenceLength,
                                         const char* fileName,
                                         double& duration,
                                         int& dtmfterm,
                                         OsProtectedEvent* ev)
{
    return OS_SUCCESS ;
}

void VoiceEngineMediaInterface::addToneListener(OsNotification *pListener, int connectionId)
{
}

void VoiceEngineMediaInterface::removeToneListener(int connectionId)
{
}

void  VoiceEngineMediaInterface::setContactType(int connectionId, ContactType eType)
{
    mContactType = eType;
}


/* ============================ ACCESSORS ================================= */

void VoiceEngineMediaInterface::setPremiumSound(UtlBoolean enabled)
{
}


// Calculate the current cost for our sending/receiving codecs
int VoiceEngineMediaInterface::getCodecCPUCost()
{
   int iCost = SdpCodec::SDP_CODEC_CPU_LOW ;

   return iCost ;
}


// Calculate the worst case cost for our sending/receiving codecs
int VoiceEngineMediaInterface::getCodecCPULimit()
{
   int iCost = SdpCodec::SDP_CODEC_CPU_LOW ;

   return iCost ;
}


// Returns the flowgraph's message queue
OsMsgQ* VoiceEngineMediaInterface::getMsgQ()
{
    return NULL;
}


OsStatus VoiceEngineMediaInterface::getPrimaryCodec(int connectionId,
                                                    UtlString& audioCodec,
                                                    UtlString& videoCodec,
                                                    int* audioPayloadType,
                                                    int* videoPayloadType)
{
    OsLock lock(mVoiceEngineGuard) ;
    OsStatus rc = OS_FAILED;

    VoiceEngineMediaConnection* pMediaConnection = getMediaConnection(connectionId);
    if (pMediaConnection && pMediaConnection->mpPrimaryCodec)
    {
        if (getCodecNameByType(pMediaConnection->mpPrimaryCodec->getCodecType(), audioCodec))
        {
            if (NULL != audioPayloadType)
            {
                *audioPayloadType = pMediaConnection->mRtpPayloadType;
            }

            rc = OS_SUCCESS;
        }
#ifdef VIDEO
        if (videoPayloadType)
        {
            *videoPayloadType = -1;
        }
        videoCodec = "";
        if (pMediaConnection->mpPrimaryVideoCodec)
        {
            if (getCodecNameByType(pMediaConnection->mpPrimaryVideoCodec->getCodecType(), videoCodec))
            {
                if (NULL != videoPayloadType)
                {
                    *videoPayloadType = pMediaConnection->mRtpVideoPayloadType;
                }

                rc = OS_SUCCESS;
            }
        }
#endif
    }

    return rc;
}

/* ============================ INQUIRY =================================== */
UtlBoolean VoiceEngineMediaInterface::isSendingRtpAudio(int connectionId)
{
    OsLock lock(mVoiceEngineGuard) ;
    UtlBoolean bSending = FALSE ;

    VoiceEngineMediaConnection* pMediaConnection = getMediaConnection(connectionId);
    if (pMediaConnection)
    {
        bSending = pMediaConnection->mRtpSendingAudio ;
    }

    return bSending ;
}


UtlBoolean VoiceEngineMediaInterface::isSendingRtpVideo(int connectionId)
{
    OsLock lock(mVoiceEngineGuard) ;
    UtlBoolean bSending = FALSE ;

    VoiceEngineMediaConnection* pMediaConnection = getMediaConnection(connectionId);
    if (pMediaConnection)
    {
        bSending = pMediaConnection->mRtpSendingVideo ;
    }
    return bSending ;
}
UtlBoolean VoiceEngineMediaInterface::isReceivingRtpAudio(int connectionId)
{
    OsLock lock(mVoiceEngineGuard) ;
    UtlBoolean bReceiving = FALSE ;

    VoiceEngineMediaConnection* pMediaConnection = getMediaConnection(connectionId);
    if (pMediaConnection)
    {
        bReceiving = pMediaConnection->mRtpReceivingAudio ;
    }


    return bReceiving ;
}

UtlBoolean VoiceEngineMediaInterface::isReceivingRtpVideo(int connectionId)
{
    OsLock lock(mVoiceEngineGuard) ;
    UtlBoolean bReceiving = FALSE ;

    VoiceEngineMediaConnection* pMediaConnection = getMediaConnection(connectionId);
    if (pMediaConnection)
    {
        bReceiving = pMediaConnection->mRtpReceivingVideo ;
    }


    return bReceiving ;
}

UtlBoolean VoiceEngineMediaInterface::isDestinationSet(int connectionId)
{
    return TRUE ;
}


UtlBoolean VoiceEngineMediaInterface::canAddParty()
{
    OsLock lock(mVoiceEngineGuard) ;
    int iLoad = mpVoiceEngine->GIPSVE_GetCPULoad() ;

    assert((iLoad >=0) && (iLoad <=10)) ;

    return (iLoad < 69) ;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

void VoiceEngineMediaInterface::startVideoSupport(int connectionId)
{
#ifdef VIDEO
    OsLock lock(mVoiceEngineGuard) ;
    int localVideoPort ;

#ifdef _WIN32
#ifdef VIDEO
    CoInitializeEx(NULL, 0);
#endif
#endif



    VoiceEngineMediaConnection* pMediaConnection = getMediaConnection(connectionId);
    if (pMediaConnection && (pMediaConnection->mpRtpVideoSocket == NULL))
    {

        mpFactoryImpl->getNextRtpPort(localVideoPort);


        VoiceEngineDatagramSocket* rtpVideoSocket = new VoiceEngineDatagramSocket(
                mpVoiceEngine, mpVideoEngine, connectionId, pMediaConnection->mVideoConnectionId,
                TYPE_VIDEO_RTP, 0, NULL, localVideoPort, mLocalAddress.data(),
                mStunServer.length() != 0, mStunServer, mStunOptions, mStunRefreshPeriodSecs);

        VoiceEngineDatagramSocket* rtcpVideoSocket = new VoiceEngineDatagramSocket(
                mpVoiceEngine, mpVideoEngine, connectionId, pMediaConnection->mVideoConnectionId,
                TYPE_VIDEO_RTCP, 0, NULL, localVideoPort+1, mLocalAddress.data(),
                mStunServer.length() != 0, mStunServer, mStunOptions, mStunRefreshPeriodSecs);

        pMediaConnection->mpRtpVideoSocket = rtpVideoSocket;
        pMediaConnection->mpRtcpVideoSocket = rtcpVideoSocket;
        pMediaConnection->mRtpVideoReceivePort = rtpVideoSocket->getLocalHostPort() ;
        pMediaConnection->mRtcpVideoReceivePort = rtcpVideoSocket->getLocalHostPort() ;
        pMediaConnection->mpVideoSocketAdapter = new
                VoiceEngineSocketAdapter(pMediaConnection->mpRtpVideoSocket, pMediaConnection->mpRtcpVideoSocket) ;

    }
#endif
}

void VoiceEngineMediaInterface::endVideoSupport(int connectionId)
{

}


UtlBoolean VoiceEngineMediaInterface::containsVideoCodec(int numCodecs, SdpCodec* codecs[])
{
    UtlBoolean bFound = false ;

    for (int i=0; i<numCodecs; i++)
    {
        UtlString mimeType ;
        codecs[i]->getMediaType(mimeType) ;
        if (mimeType.compareTo(MIME_TYPE_VIDEO, UtlString::ignoreCase) == 0)
        {
            bFound = true ;
            break ;
        }
    }

    return bFound ;
}


/* //////////////////////////// PRIVATE /////////////////////////////////// */

UtlBoolean VoiceEngineMediaInterface::getCodecNameByType(SdpCodec::SdpCodecTypes codecType, UtlString& codecName)
{
    UtlBoolean matchFound = FALSE;

    codecName = "";

    switch (codecType)
    {
    case SdpCodec::SDP_CODEC_G729A:
        codecName = GIPS_CODEC_ID_G729;
        break;
    case SdpCodec::SDP_CODEC_TONES:
        codecName = GIPS_CODEC_ID_TELEPHONE;
        break;
    case SdpCodec::SDP_CODEC_GIPS_PCMA:
        codecName = GIPS_CODEC_ID_PCMA;
        break;
    case SdpCodec::SDP_CODEC_GIPS_PCMU:
        codecName = GIPS_CODEC_ID_PCMU;
        break;
    case SdpCodec::SDP_CODEC_GIPS_IPCMA:
        codecName = GIPS_CODEC_ID_EG711A;
        break;
    case SdpCodec::SDP_CODEC_GIPS_IPCMU:
        codecName = GIPS_CODEC_ID_EG711U;
        break;
    case SdpCodec::SDP_CODEC_GIPS_IPCMWB:
        codecName = GIPS_CODEC_ID_IPCMWB;
        break;
    case SdpCodec::SDP_CODEC_GIPS_ILBC:
        codecName = GIPS_CODEC_ID_ILBC;
        break;
    case SdpCodec::SDP_CODEC_GIPS_ISAC:
        codecName = GIPS_CODEC_ID_ISAC;
        break;
    case SdpCodec::SDP_CODEC_GSM:
        codecName = GIPS_CODEC_ID_GSM;
        break;
    case SdpCodec::SDP_CODEC_G723:
        codecName = GIPS_CODEC_ID_G723;
        break;
    case SdpCodec::SDP_CODEC_VP71_CIF:
        codecName = GIPS_CODEC_ID_VP71_CIF;
        break;
    case SdpCodec::SDP_CODEC_IYUV_CIF:
        codecName = GIPS_CODEC_ID_IYUV_CIF;
        break;
    case SdpCodec::SDP_CODEC_I420_CIF:
        codecName = GIPS_CODEC_ID_I420_CIF;
        break;
    case SdpCodec::SDP_CODEC_RGB24_CIF:
        codecName = GIPS_CODEC_ID_RGB24_CIF;
        break;
    case SdpCodec::SDP_CODEC_VP71_QCIF:
        codecName = GIPS_CODEC_ID_VP71_QCIF;
        break;
    case SdpCodec::SDP_CODEC_IYUV_QCIF:
        codecName = GIPS_CODEC_ID_IYUV_QCIF;
        break;
    case SdpCodec::SDP_CODEC_I420_QCIF:
        codecName = GIPS_CODEC_ID_I420_QCIF;
        break;
    case SdpCodec::SDP_CODEC_RGB24_QCIF:
        codecName = GIPS_CODEC_ID_RGB24_QCIF;
        break;
    case SdpCodec::SDP_CODEC_VP71_SQCIF:
        codecName = GIPS_CODEC_ID_VP71_SQCIF;
        break;
    case SdpCodec::SDP_CODEC_IYUV_SQCIF:
        codecName = GIPS_CODEC_ID_IYUV_SQCIF;
        break;
    case SdpCodec::SDP_CODEC_I420_SQCIF:
        codecName = GIPS_CODEC_ID_I420_SQCIF;
        break;
    case SdpCodec::SDP_CODEC_RGB24_SQCIF:
        codecName = GIPS_CODEC_ID_RGB24_SQCIF;
        break;
    default:
        OsSysLog::add(FAC_MP, PRI_DEBUG,
                      "getCodecNameByType: unsupported codec %d",
                      codecType);
    }

    if (codecName != "")
    {
        matchFound = TRUE;
    }

    return matchFound;
}

UtlBoolean VoiceEngineMediaInterface::getCodecTypeByName(const UtlString& codecName, SdpCodec::SdpCodecTypes& codecType)
{
    UtlBoolean matchFound = FALSE;

    codecType = SdpCodec::SDP_CODEC_UNKNOWN;

    if (codecName == GIPS_CODEC_ID_G729)
    {
        codecType = SdpCodec::SDP_CODEC_G729A;
    }
    else if (codecName == GIPS_CODEC_ID_TELEPHONE)
    {
        codecType = SdpCodec::SDP_CODEC_TONES;
    }
    else if (codecName == GIPS_CODEC_ID_PCMA)
    {
        codecType = SdpCodec::SDP_CODEC_GIPS_PCMA;
    }
    else if (codecName == GIPS_CODEC_ID_PCMU)
    {
        codecType = SdpCodec::SDP_CODEC_GIPS_PCMU;
    }
    else if (codecName == GIPS_CODEC_ID_EG711A)
    {
        codecType = SdpCodec::SDP_CODEC_GIPS_IPCMA;
    }
    else if (codecName == GIPS_CODEC_ID_EG711U)
    {
        codecType = SdpCodec::SDP_CODEC_GIPS_IPCMU;
    }
    else if (codecName == GIPS_CODEC_ID_IPCMWB)
    {
        codecType = SdpCodec::SDP_CODEC_GIPS_IPCMWB;
    }
    else if (codecName == GIPS_CODEC_ID_ILBC)
    {
        codecType = SdpCodec::SDP_CODEC_GIPS_ILBC;
    }
    else if (codecName == GIPS_CODEC_ID_ISAC)
    {
        codecType = SdpCodec::SDP_CODEC_GIPS_ISAC;
    }
    else if (codecName == GIPS_CODEC_ID_GSM)
    {
        codecType = SdpCodec::SDP_CODEC_GSM;
    }
    else if (codecName == GIPS_CODEC_ID_G723)
    {
        codecType = SdpCodec::SDP_CODEC_G723;
    }
    else if (codecName == GIPS_CODEC_ID_VP71_CIF)
    {
        codecType = SdpCodec::SDP_CODEC_VP71_CIF;
    }
    else if (codecName == GIPS_CODEC_ID_IYUV_CIF)
    {
        codecType = SdpCodec::SDP_CODEC_IYUV_CIF;
    }
    else if (codecName == GIPS_CODEC_ID_I420_CIF)
    {
        codecType = SdpCodec::SDP_CODEC_I420_CIF;
    }
    else if (codecName == GIPS_CODEC_ID_RGB24_CIF)
    {
        codecType = SdpCodec::SDP_CODEC_RGB24_CIF;
    }
    else if (codecName == GIPS_CODEC_ID_VP71_QCIF)
    {
        codecType = SdpCodec::SDP_CODEC_VP71_QCIF;
    }
    else if (codecName == GIPS_CODEC_ID_IYUV_QCIF)
    {
        codecType = SdpCodec::SDP_CODEC_IYUV_QCIF;
    }
    else if (codecName == GIPS_CODEC_ID_I420_QCIF)
    {
        codecType = SdpCodec::SDP_CODEC_I420_QCIF;
    }
    else if (codecName == GIPS_CODEC_ID_RGB24_QCIF)
    {
        codecType = SdpCodec::SDP_CODEC_RGB24_QCIF;
    }
    else if (codecName == GIPS_CODEC_ID_VP71_SQCIF)
    {
        codecType = SdpCodec::SDP_CODEC_VP71_SQCIF;
    }
    else if (codecName == GIPS_CODEC_ID_IYUV_SQCIF)
    {
        codecType = SdpCodec::SDP_CODEC_IYUV_SQCIF;
    }
    else if (codecName == GIPS_CODEC_ID_I420_SQCIF)
    {
        codecType = SdpCodec::SDP_CODEC_I420_SQCIF;
    }
    else if (codecName == GIPS_CODEC_ID_RGB24_SQCIF)
    {
        codecType = SdpCodec::SDP_CODEC_RGB24_SQCIF;
    }

    if (codecType != SdpCodec::SDP_CODEC_UNKNOWN)
    {
        matchFound = TRUE;
    }

    return matchFound;
}

UtlBoolean VoiceEngineMediaInterface::getVoiceEngineCodec(const SdpCodec& pCodec, GIPSVE_CodecInst& codecInfo)
{
    OsLock lock(mVoiceEngineGuard) ;
    UtlString codecName;
    UtlBoolean matchFound = FALSE;
    int iCodecs;

    if (getCodecNameByType(pCodec.getCodecType(), codecName))
    {
        if ((iCodecs=mpVoiceEngine->GIPSVE_GetNofCodecs()) != -1)
        {
            for (int i=0; i<iCodecs; ++i)
            {
                if (mpVoiceEngine->GIPSVE_GetCodec(i, &codecInfo) == 0)
                {
                    if (strcmp(codecName.data(), codecInfo.plname) == 0)
                    {
                        matchFound = TRUE;
                        break;
                    }
                }
            }
        }
    }
    return matchFound;
}

#ifdef VIDEO
UtlBoolean VoiceEngineMediaInterface::getVideoEngineCodec(const SdpCodec& pCodec, GIPSVideo_CodecInst& codecInfo)
{
    OsLock lock(mVoiceEngineGuard) ;
    UtlString codecName;
    UtlBoolean matchFound = FALSE;
    int iCodecs;

    if (getCodecNameByType(pCodec.getCodecType(), codecName))
    {
        if ((iCodecs=mpVideoEngine->GIPSVideo_GetNofCodecs()) != -1)
        {
            for (int i=0; i<iCodecs; ++i)
            {
                memset((void*)&codecInfo, 0, sizeof(codecInfo));
                if (mpVideoEngine->GIPSVideo_GetCodec(i, &codecInfo) == 0)
                {
                    if (strncmp(codecName.data(), codecInfo.plname, 4) == 0)
                    {
                        matchFound = TRUE;
                        break;
                    }
                }
            }
        }
    }
    return matchFound;
}
#endif

GipsVoiceEngineLib* const VoiceEngineMediaInterface::getVoiceEnginePtr()
{
    OsLock lock(mVoiceEngineGuard) ;
    return mpVoiceEngine;
}


OsStatus VoiceEngineMediaInterface::muteMicrophone(const bool bMute)
{
    OsLock lock(mVoiceEngineGuard) ;
    OsStatus rc = OS_SUCCESS;
    VoiceEngineMediaConnection* pMediaConnection = NULL ;

    UtlSListIterator iterator(mMediaConnections);
    while ((pMediaConnection = (VoiceEngineMediaConnection*) iterator()))
    {
        if (0 != mpVoiceEngine->GIPSVE_MuteMic(pMediaConnection->getValue(), bMute))
        {
            assert(0);
            rc = OS_FAILED;
        }

    }
    return rc;
}

OsStatus VoiceEngineMediaInterface::setVideoWindowDisplay(const void* pDisplay)
{
    OsStatus rc = OS_SUCCESS;

    mpDisplay = (SIPXVE_VIDEO_DISPLAY*)pDisplay;
    return rc;
}

const void* VoiceEngineMediaInterface::getVideoWindowDisplay()
{
    return mpDisplay;
}

#ifdef VIDEO
const bool VoiceEngineMediaInterface::isDisplayValid(const SIPXVE_VIDEO_DISPLAY* const pDisplay)
{
    bool bRet = false;
    if (pDisplay && pDisplay->handle)
    {
        bRet = true;
    }
    return bRet;
}
#endif
/* ============================ FUNCTIONS ================================= */
