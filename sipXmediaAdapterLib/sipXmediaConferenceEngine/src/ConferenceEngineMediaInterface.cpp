//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////


// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "ConferenceEngine.h"
#include <utl/UtlSListIterator.h>
#include <os/OsDatagramSocket.h>
#include <os/OsProtectEventMgr.h>
#include <net/SdpCodec.h>
#include <net/SipContactDb.h>
#include "include/ConferenceEngineNetTask.h"
#include "include/ConferenceEngineDatagramSocket.h"
#include "include/ConferenceEngineMediaInterface.h"


#if defined(__pingtel_on_posix__)
#   include <netinet/in.h>
#   include <netinet/tcp.h>
#   include <sys/types.h>
#   include <sys/socket.h>
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define MINIMUM_DTMF_LENGTH 60
#define MAX_RTP_PORTS 1000


// STATIC VARIABLE INITIALIZATIONS

class ConferenceEngineMediaConnection : public UtlInt
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
    ConferenceEngineMediaConnection(int connectionId = -1)
        : UtlInt(connectionId)
    {
        mpRtpSocket = NULL;
        mpRtcpSocket = NULL;
        mRtpSendHostPort = 0;
        mRtcpSendHostPort = 0;
        mRtpReceivePort = 0;
        mRtcpReceivePort = 0;
        mRtpPayloadType = 0;
        mDestinationSet = FALSE;
        mRtpSending = FALSE;
        mRtpReceiving = FALSE;
        mpCodecFactory = NULL;
        mpPrimaryCodec = NULL;
        meContactType = AUTO ;
        mpSocketAdapter = NULL;
        mpDTMFNotify = NULL;
        mpPlayNotify = NULL;
    };

    virtual ~ConferenceEngineMediaConnection()
    {
        if(mpRtpSocket)
        {
            mpRtpSocket->close();
            delete mpRtpSocket;
            mpRtpSocket = NULL;
        }

        if(mpRtcpSocket)
        {
            mpRtcpSocket->close();
            delete mpRtcpSocket;
            mpRtcpSocket = NULL;
        }

        if(mpCodecFactory)
        {
            delete mpCodecFactory;
            mpCodecFactory = NULL;
        }

        delete mpSocketAdapter ;  ;
    }

    ConferenceEngineDatagramSocket* mpRtpSocket;
    ConferenceEngineDatagramSocket* mpRtcpSocket;
    ConferenceEngineSocketAdapter* mpSocketAdapter ;
    UtlString mRtpSendHostAddress;
    int mRtpSendHostPort;
    int mRtcpSendHostPort;
    int mRtpReceivePort;
    int mRtcpReceivePort;
    int mRtpPayloadType;
    UtlBoolean mDestinationSet;
    UtlBoolean mRtpSending;
    UtlBoolean mRtpReceiving;
    SdpCodecFactory* mpCodecFactory;
    SdpCodec* mpPrimaryCodec;
    ContactType meContactType ;
    OsNotification* mpDTMFNotify;  // Object to signal on key-down/key-up events
    OsNotification* mpPlayNotify;  // Object to signal on play done event
};

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
ConferenceEngineMediaInterface::ConferenceEngineMediaInterface(ConferenceEngineFactoryImpl* pFactoryImpl,
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
    : mLock(OsBSem::Q_PRIORITY, OsBSem::FULL)
    , CpMediaInterface(pFactoryImpl)
    , mSupportedCodecs(numCodecs, sdpCodecArray)
{
    OsSysLog::add(FAC_MP, PRI_DEBUG,
                  "ConferenceEngineMediaInterface::ConferenceEngineMediaInterface creating a new ConferenceEngineMediaInterface ...");
    OsSysLog::add(FAC_MP, PRI_DEBUG,
                  "ConferenceEngineMediaInterface::ConferenceEngineMediaInterface public addr = %s, local addr = %s, numCodecs = %d",
                  publicAddress, localAddress, numCodecs);

    mpConferenceEngine = &NewConfEngine() ;

    // Initialize the ConferenceEngine object
    int rc = mpConferenceEngine->GIPSConf_Init() ;
    if (rc != 0)
    {
        OsSysLog::add(FAC_MP, PRI_ERR,
                      "ConferenceEngineMediaInterface::ConferenceEngineMediaInterface GIPS ConferenceEngine failed to initialize. error = %d.",
                      mpConferenceEngine->GIPSConf_GetLastError());
    }
    assert(rc == 0);

    // Add the event handler to the ConferenceEngine
    rc = mpConferenceEngine->GIPSConf_SetEventHandler(this, mEventHandler) ;
    if (rc != 0)
    {
        OsSysLog::add(FAC_MP, PRI_ERR,
                      "ConferenceEngineMediaInterface::ConferenceEngineMediaInterface GIPSConf_SetEventHandler failed with error = %d.",
                      mpConferenceEngine->GIPSConf_GetLastError());
    }
    assert(rc == 0);

    mpNetTask = ConferenceEngineNetTask::getConferenceEngineNetTask() ;

    mStunOptions = stunOptions ;
    mStunServer = szStunServer ;
    mStunRefreshPeriodSecs = iStunKeepAlivePeriodSecs ;

    mFocus = FALSE;
    mDTMFOutOfBand = bDTMFOutOfBand;

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

    pFactoryImpl->getSystemVolume(mDefaultVolume);
}


// Destructor
ConferenceEngineMediaInterface::~ConferenceEngineMediaInterface()
{
    mLock.acquire();

    ConferenceEngineMediaConnection* pMediaConnection = NULL;

    UtlSListIterator iterator(mMediaConnections);
    while ((pMediaConnection = (ConferenceEngineMediaConnection*) iterator()))
    {
        deleteConnection(pMediaConnection->getValue()) ;
    }

    mpConferenceEngine->GIPSConf_Terminate();
    delete mpConferenceEngine ;
    mpConferenceEngine = NULL;

    mLock.release();
}

void ConferenceEngineMediaInterface::release()
{
    // remove this pointer from the factory implementation
    if (mpFactoryImpl)
    {
        ((ConferenceEngineFactoryImpl*) mpFactoryImpl)->removeMediaInterface(this);
    }

    delete this;
}


/* ============================ MANIPULATORS ============================== */

OsStatus ConferenceEngineMediaInterface::createConnection(int& connectionId, void* videoDisplay)
{
    mLock.acquire();

    int localPort;
    mpFactoryImpl->getNextRtpPort(localPort);

    connectionId = mpConferenceEngine->GIPSConf_CreateChannel();
    if (connectionId < 0)
    {
        OsSysLog::add(FAC_MP, PRI_ERR,
                      "ConferenceEngineMediaInterface::createConnection CreateChannel failed. error = %d.",
                      mpConferenceEngine->GIPSConf_GetLastError());
    }
    assert(connectionId >= 0) ;

    // Create a new  ConferenceEngineMediaConnection object
    ConferenceEngineMediaConnection* pMediaConnection = new ConferenceEngineMediaConnection(connectionId);

    UtlInt container(connectionId);
    if (mMediaConnections.find(&container) == NULL)
    {
        mMediaConnections.insert(pMediaConnection);
    }
    else
    {
        OsSysLog::add(FAC_MP, PRI_ERR,
                      "ConferenceEngineMediaInterface::createConnection MediaConnection %d has already existed",
                      connectionId);

        // Remove the old one and insert the new one
        mMediaConnections.destroy(&container);

        mMediaConnections.insert(pMediaConnection);
    }

    ConferenceEngineDatagramSocket* rtpSocket = new ConferenceEngineDatagramSocket(
            mpConferenceEngine, connectionId, TYPE_RTP, 0, NULL,
            localPort, mLocalAddress.data(), mStunServer.length() != 0,
            mStunServer, mStunRefreshPeriodSecs);

    ConferenceEngineDatagramSocket* rtcpSocket = new ConferenceEngineDatagramSocket(
            mpConferenceEngine, connectionId, TYPE_RTCP, 0, NULL,
            localPort == 0 ? 0 : localPort + 1, mLocalAddress.data(),
            mStunServer.length() != 0, mStunServer, mStunRefreshPeriodSecs);

    pMediaConnection->mpRtpSocket = rtpSocket;
    pMediaConnection->mpRtcpSocket = rtcpSocket;
    pMediaConnection->mRtpReceivePort = rtpSocket->getLocalHostPort() ;
    pMediaConnection->mRtcpReceivePort = rtcpSocket->getLocalHostPort() ;
    pMediaConnection->mpSocketAdapter = new
            ConferenceEngineSocketAdapter(pMediaConnection->mpRtpSocket, pMediaConnection->mpRtcpSocket) ;
    pMediaConnection->mpCodecFactory = new SdpCodecFactory(mSupportedCodecs);
    pMediaConnection->mpCodecFactory->bindPayloadTypes();

    int rc = mpConferenceEngine->GIPSConf_SetSendTransport(connectionId, *pMediaConnection->mpSocketAdapter) ;
    if (rc != 0)
    {
        OsSysLog::add(FAC_MP, PRI_ERR,
                      "ConferenceEngineMediaInterface::createConnection GIPSConf_SetSendTransport failed with error = %d.",
                      mpConferenceEngine->GIPSConf_GetLastError());
    }
    assert(rc == 0);

    UtlBoolean bAEC;
    mpFactoryImpl->isAudioAECEnabled(bAEC);
    if (bAEC)
    {
        rc = mpConferenceEngine->GIPSConf_SetAGCStatus(connectionId, 1);
    }
    else
    {
        rc = mpConferenceEngine->GIPSConf_SetAGCStatus(connectionId, 0);
    }

    if (rc != 0)
    {
        OsSysLog::add(FAC_MP, PRI_ERR,
                      "ConferenceEngineMediaInterface::createConnection GIPSConf_SetAGCStatus failed with error = %d.",
                      mpConferenceEngine->GIPSConf_GetLastError());
    }

    OsSysLog::add(FAC_MP, PRI_DEBUG,
                  "ConferenceEngineMediaInterface::createConnection a new connection is created with id = %d on port %d",
                  connectionId, localPort);

    mLock.release();
    return OS_SUCCESS ;
}


int ConferenceEngineMediaInterface::getNumCodecs(int connectionId)
{
    mLock.acquire();

    int iCodecs = 0;
    ConferenceEngineMediaConnection*
        pMediaConn = getMediaConnection(connectionId);

    if (pMediaConn && pMediaConn->mpCodecFactory)
    {
        iCodecs = pMediaConn->mpCodecFactory->getCodecCount();
    }

    mLock.release();
    return iCodecs;
}


OsStatus ConferenceEngineMediaInterface::getCapabilities(int connectionId,
                                                         UtlString& rtpHostAddress,
                                                         int& rtpPort,
                                                         int& rtcpPort,
                                                         int& rtpVideoPort,
                                                         int& rtcpVideoPort,
                                                         SdpCodecFactory& supportedCodecs,
                                                         SdpSrtpParameters& srtpParams)
{
    mLock.acquire();

    ConferenceEngineMediaConnection* pMediaConn = getMediaConnection(connectionId);
    if (pMediaConn)
    {
        bool bSet = FALSE ;

        if ((pMediaConn->meContactType == AUTO) || (pMediaConn->meContactType == NAT_MAPPED))
        {
            if (pMediaConn->mpRtpSocket->getExternalIp(&rtpHostAddress, &rtpPort) &&
                pMediaConn->mpRtcpSocket->getExternalIp(NULL, &rtcpPort))
            {
                bSet = TRUE ;
            }
        }

        if (!bSet)
        {
            rtpHostAddress = mRtpReceiveHostAddress.data();
            rtpPort = pMediaConn->mRtpReceivePort;
            rtcpPort = pMediaConn->mRtcpReceivePort ;
        }

        supportedCodecs = *(pMediaConn->mpCodecFactory);
        supportedCodecs.bindPayloadTypes();
    }

    mLock.release();
    return OS_SUCCESS ;
}


ConferenceEngineMediaConnection* ConferenceEngineMediaInterface::getMediaConnection(int connectionId)
{
   UtlInt matchConnectionId(connectionId) ;

   return ((ConferenceEngineMediaConnection*) mMediaConnections.find(&matchConnectionId)) ;
}


OsStatus ConferenceEngineMediaInterface::setConnectionDestination(int connectionId,
                                                                  const char* remoteRtpHostAddress,
                                                                  int remoteRtpPort,
                                                                  int remoteRtcpPort,
                                                                  int remoteVideoRtpPort,
                                                                  int remoteVideoRtcpPort)
{
    mLock.acquire();

    ConferenceEngineMediaConnection* pMediaConnection = getMediaConnection(connectionId);
    if (pMediaConnection)
    {
        pMediaConnection->mDestinationSet = TRUE;
        pMediaConnection->mRtpSendHostAddress = remoteRtpHostAddress;
        pMediaConnection->mRtpSendHostPort = remoteRtpPort;
        pMediaConnection->mRtcpSendHostPort = remoteRtcpPort;

        pMediaConnection->mpRtpSocket->doConnect(remoteRtpPort, remoteRtpHostAddress, TRUE);

        int rc = 0;
        if (remoteRtcpPort > 0)
        {
            pMediaConnection->mpRtcpSocket->doConnect(remoteRtcpPort, remoteRtpHostAddress, TRUE);
            rc = mpConferenceEngine->GIPSConf_EnableRTCP(connectionId, TRUE) ;
        }
        else
        {
            rc = mpConferenceEngine->GIPSConf_EnableRTCP(connectionId, FALSE) ;
        }

        if (rc != 0)
        {
            OsSysLog::add(FAC_MP, PRI_ERR,
                          "ConferenceEngineMediaInterface::setConnectionDestination GIPSConf_EnableRTCP failed with error = %d.",
                          mpConferenceEngine->GIPSConf_GetLastError());
        }
        assert(rc == 0) ;
    }

    mLock.release();
    return OS_SUCCESS ;
}


OsStatus ConferenceEngineMediaInterface::addAlternateDestinations(int connectionId,
                                                                  unsigned char cPriority,
                                                                  const char* rtpHostAddress,
                                                                  int port,
                                                                  bool bRtp)
{
    mLock.acquire();

    OsStatus returnCode = OS_NOT_FOUND;
    ConferenceEngineMediaConnection* mediaConnection = getMediaConnection(connectionId);
    if (mediaConnection)
    {
        if (bRtp)
        {
            if (mediaConnection->mpRtpSocket)
            {
                mediaConnection->mpRtpSocket->addAlternateDestination(rtpHostAddress,
                                                                      port,
                                                                      cPriority) ;
                returnCode = OS_SUCCESS;
            }
        }
        else
        {
            if (mediaConnection->mpRtcpSocket)
            {
                mediaConnection->mpRtcpSocket->addAlternateDestination(rtpHostAddress,
                                                                       port,
                                                                       cPriority) ;
                returnCode = OS_SUCCESS;
            }
        }
    }

    mLock.release();
    return returnCode ;
}


OsStatus ConferenceEngineMediaInterface::startRtpSend(int connectionId,
                                                      int numCodecs,
                                                      SdpCodec* sendCodecs[],
                                                      SdpSrtpParameters& srtpParams)
{
    mLock.acquire();

    OsStatus status = OS_FAILED;
    int i, rc;
    SdpCodec* primaryCodec = NULL;
    GIPS_CodecInst codecInfo;

    ConferenceEngineMediaConnection* pMediaConnection = getMediaConnection(connectionId);
    if (pMediaConnection && !pMediaConnection->mRtpSending)
    {
        rc = mpConferenceEngine->GIPSConf_SetSendPort(connectionId, pMediaConnection->mRtpSendHostPort) ;
        if (rc != 0)
        {
            OsSysLog::add(FAC_MP, PRI_ERR,
                          "ConferenceEngineMediaInterface::startRtpSend GIPSConf_SetSendPort failed with error = %d.",
                          mpConferenceEngine->GIPSConf_GetLastError());
        }
        assert(rc == 0);

        rc = mpConferenceEngine->GIPSConf_SetSendIP(connectionId, (char*) pMediaConnection->mRtpSendHostAddress.data()) ;
        if (rc != 0)
        {
            OsSysLog::add(FAC_MP, PRI_ERR,
                          "ConferenceEngineMediaInterface::startRtpSend GIPSConf_SetSendIP failed with error = %d.",
                          mpConferenceEngine->GIPSConf_GetLastError());
        }
        assert(rc == 0);

        bool bFoundCodec;
        for (i = 0; i < numCodecs; i++)
        {
            primaryCodec = sendCodecs[i];
            pMediaConnection->mpPrimaryCodec = primaryCodec;

            if (primaryCodec && getConferenceEngineCodec(*primaryCodec, codecInfo))
            {
                OsSysLog::add(FAC_MP, PRI_DEBUG,
                              "ConferenceEngineMediaInterface::startRtpSend: using GIPS codec %s for id %d, payload %d",
                              codecInfo.plname, primaryCodec->getCodecType(),
                              primaryCodec->getCodecPayloadFormat());
                pMediaConnection->mRtpPayloadType = primaryCodec->getCodecPayloadFormat();

                // Forcing ConferenceEngine to use our dynamically allocated payload types
                codecInfo.pltype = primaryCodec->getCodecPayloadFormat();

                rc = mpConferenceEngine->GIPSConf_SetSendCodec(connectionId, &codecInfo);
                if (rc != 0)
                {
                    OsSysLog::add(FAC_MP, PRI_DEBUG,
                                  "ConferenceEngineMediaInterface::startRtpSend: GIPSConf_SetSendCodec failed with error %d",
                                   mpConferenceEngine->GIPSConf_GetLastError());
                }
                assert(rc == 0);

                bFoundCodec = true;
            }

            break;
        }

        if (bFoundCodec)
        {
            rc = mpConferenceEngine->GIPSConf_StartSendToParticipant(connectionId) ;
            if (rc != 0)
            {
                OsSysLog::add(FAC_MP, PRI_ERR,
                              "ConferenceEngineMediaInterface::startRtpSend GIPSConf_StartSendToParticipant failed with error = %d.",
                              mpConferenceEngine->GIPSConf_GetLastError());
            }
            assert(rc == 0);

            pMediaConnection->mRtpSending = TRUE ;
            rc = mpConferenceEngine->GIPSConf_StartPlayoutToMeeting(connectionId) ;
            if (rc != 0)
            {
                OsSysLog::add(FAC_MP, PRI_ERR,
                              "ConferenceEngineMediaInterface::startRtpSend GIPSConf_StartPlayoutToMeeting failed with error = %d.",
                              mpConferenceEngine->GIPSConf_GetLastError());
            }
            assert(rc == 0);

            status = OS_SUCCESS;
        }
        else
        {
            OsSysLog::add(FAC_MP, PRI_ERR,
                          "ConferenceEngineMediaInterface::startRtpSend No match codec is found.");
        }
    }

    mLock.release();
    return status;
}


OsStatus ConferenceEngineMediaInterface::startRtpReceive(int connectionId,
                                                         int numCodecs,
                                                         SdpCodec* receiveCodecs[],
                                                         SdpSrtpParameters& srtpParams)
{
    mLock.acquire();

    int rc;
    int i;
    SdpCodec* primaryCodec = NULL;
    GIPS_CodecInst codecInfo;

    ConferenceEngineMediaConnection* pMediaConnection = getMediaConnection(connectionId) ;
    if (pMediaConnection && !pMediaConnection->mRtpReceiving)
    {
        pMediaConnection->mpPrimaryCodec = NULL;
        if (pMediaConnection->mpCodecFactory)
        {
            pMediaConnection->mpCodecFactory->copyPayloadTypes(numCodecs, receiveCodecs);
        }

        for (i = 0; i < numCodecs; i++)
        {
            if (NULL == primaryCodec)
            {
                primaryCodec = receiveCodecs[i];
                if (getConferenceEngineCodec(*primaryCodec, codecInfo))
                {
                    // Stop playout here just in case we had a previous giveFocus call. That
                    // will make the SetRecPayloadType call fail. Playout will be resumed
                    // further down.
                    rc = mpConferenceEngine->GIPSConf_StopPlayoutToMeeting(connectionId);
                    if (rc != 0)
                    {
                        OsSysLog::add(FAC_MP, PRI_ERR,
                                      "ConferenceEngineMediaInterface::startRtpReceive GIPSConf_StopPlayoutToMeeting failed with error = %d.",
                                      mpConferenceEngine->GIPSConf_GetLastError());
                    }
                    assert(rc == 0);

                    if (primaryCodec)
                    {
                        codecInfo.pltype = primaryCodec->getCodecPayloadFormat();
                    }

                    rc = mpConferenceEngine->GIPSConf_SetRecPayloadType(connectionId, &codecInfo) ;
                    if (rc != 0)
                    {
                        OsSysLog::add(FAC_MP, PRI_ERR,
                                      "ConferenceEngineMediaInterface::startRtpReceive GIPSConf_SetRecPayloadType failed with error = %d.",
                                      mpConferenceEngine->GIPSConf_GetLastError());
                    }
                    assert(rc == 0);
                }
            }
        }

        pMediaConnection->mRtpReceiving = TRUE ;
        if (pMediaConnection->mpRtpSocket)
        {
            mpNetTask->addInputSource(pMediaConnection->mpRtpSocket) ;
        }

        if (pMediaConnection->mpRtcpSocket)
        {
            mpNetTask->addInputSource(pMediaConnection->mpRtcpSocket) ;
        }

        rc = mpConferenceEngine->GIPSConf_SetRecPort(connectionId, pMediaConnection->mRtpReceivePort + 10);
        if (rc != 0)
        {
            OsSysLog::add(FAC_MP, PRI_ERR,
                          "ConferenceEngineMediaInterface::startRtpReceive GIPSConf_SetRecPort failed with error = %d.",
                          mpConferenceEngine->GIPSConf_GetLastError());
        }
        assert(rc == 0);

#if 0
        // This call only has meaning using internal transport
        rc = mpConferenceEngine->GIPSConf_StartListenToParticipant(connectionId) ;
        if (rc != 0)
        {
            OsSysLog::add(FAC_MP, PRI_ERR,
                          "ConferenceEngineMediaInterface::startRtpReceive GIPSConf_StartListenToParticipant failed with error = %d.",
                          mpConferenceEngine->GIPSConf_GetLastError());
        }
        assert(rc == 0);

        rc = mpConferenceEngine->GIPSConf_StartPlayoutToMeeting(connectionId) ;
        if (rc != 0)
        {
            OsSysLog::add(FAC_MP, PRI_ERR,
                          "ConferenceEngineMediaInterface::startRtpReceive GIPSConf_StartPlayoutToMeeting failed with error = %d.",
                          mpConferenceEngine->GIPSConf_GetLastError());
        }
        assert(rc == 0) ;
#endif
    }

    mLock.release();
    return OS_SUCCESS ;
}


OsStatus ConferenceEngineMediaInterface::stopRtpSend(int connectionId)
{
    mLock.acquire();

    int iRC ;
    ConferenceEngineMediaConnection* pMediaConnection = getMediaConnection(connectionId) ;
    if (pMediaConnection && pMediaConnection->mRtpSending)
    {
        pMediaConnection->mRtpSending = FALSE ;

        iRC = mpConferenceEngine->GIPSConf_StopPlayoutToMeeting(connectionId) ;
        if (iRC != 0)
        {
            OsSysLog::add(FAC_MP, PRI_ERR,
                          "ConferenceEngineMediaInterface::stopRtpSend GIPSConf_StopPlayoutToMeeting failed with error = %d.",
                          mpConferenceEngine->GIPSConf_GetLastError());
        }
        assert(iRC == 0) ;

        iRC = mpConferenceEngine->GIPSConf_StopSendToParticipant(connectionId) ;
        if (iRC != 0)
        {
            OsSysLog::add(FAC_MP, PRI_ERR,
                          "ConferenceEngineMediaInterface::stopRtpSend GIPSConf_StopSendToParticipant failed with error = %d.",
                          mpConferenceEngine->GIPSConf_GetLastError());
        }
        assert(iRC == 0) ;
    }

    mLock.release();
    return OS_SUCCESS ;
}

OsStatus ConferenceEngineMediaInterface::stopRtpReceive(int connectionId)
{
    mLock.acquire();
    int iRC ;

    ConferenceEngineMediaConnection* pMediaConnection = getMediaConnection(connectionId) ;
    if (pMediaConnection && pMediaConnection->mRtpReceiving)
    {
        pMediaConnection->mRtpReceiving = FALSE ;

        iRC = mpConferenceEngine->GIPSConf_StopPlayoutToMeeting(connectionId) ;
        if (iRC != 0)
        {
            OsSysLog::add(FAC_MP, PRI_ERR,
                          "ConferenceEngineMediaInterface::stopRtpReceive GIPSConf_StopPlayoutToMeeting failed with error = %d.",
                          mpConferenceEngine->GIPSConf_GetLastError());
        }
        assert(iRC == 0) ;

        iRC = mpConferenceEngine->GIPSConf_StopListenToParticipant(connectionId) ;
        if (iRC != 0)
        {
            OsSysLog::add(FAC_MP, PRI_ERR,
                          "ConferenceEngineMediaInterface::stopRtpReceive GIPSConf_StopListenToParticipant failed with error = %d.",
                          mpConferenceEngine->GIPSConf_GetLastError());
        }
        assert(iRC == 0);
    }

    mLock.release();
    return OS_SUCCESS ;
}


OsStatus ConferenceEngineMediaInterface::deleteConnection(int connectionId)
{
    mLock.acquire();
    OsStatus rc = OS_NOT_FOUND ;

    ConferenceEngineMediaConnection* pMediaConnection = getMediaConnection(connectionId) ;
    if (pMediaConnection)
    {
        stopRtpSend(connectionId) ;
        stopRtpReceive(connectionId) ;

        OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
        OsProtectedEvent* rtpSockeRemoveEvent = eventMgr->alloc();
        OsProtectedEvent* rtcpSockeRemoveEvent = eventMgr->alloc();
        OsTime maxEventTime(20, 0);

        ConferenceEngineNetTask* pNetTask = NULL;

        pNetTask = ConferenceEngineNetTask::getConferenceEngineNetTask();

        if (pNetTask)
        {
            pNetTask->removeInputSource(pMediaConnection->mpRtpSocket, rtpSockeRemoveEvent) ;
            pNetTask->removeInputSource(pMediaConnection->mpRtcpSocket, rtcpSockeRemoveEvent) ;
            // Wait until the call sets the number of connections
            rtpSockeRemoveEvent->wait(0, maxEventTime);
            rtcpSockeRemoveEvent->wait(0, maxEventTime);
        }

        mpFactoryImpl->releaseRtpPort(pMediaConnection->mRtcpReceivePort) ;

        UtlInt container(connectionId);
        mMediaConnections.destroy(&container);

        int iRC = mpConferenceEngine->GIPSConf_DeleteChannel(connectionId) ;
        assert(iRC == 0);

        eventMgr->release(rtpSockeRemoveEvent);
        eventMgr->release(rtcpSockeRemoveEvent);
        delete pMediaConnection;

        rc = OS_SUCCESS;
    }

    mLock.release();
    return rc ;
}


OsStatus ConferenceEngineMediaInterface::playAudio(const char* url,
                                                   UtlBoolean repeat,
                                                   UtlBoolean local,
                                                   UtlBoolean remote)
{
    mLock.acquire();

    assert(url);

    ConferenceEngineMediaConnection* pMediaConnection = (ConferenceEngineMediaConnection *) mMediaConnections.first();
    if (pMediaConnection)
    {
        // Set up a notification for play done event
        pMediaConnection->mpPlayNotify = NULL;
    }

    // According to gips/ConferenceEngine/interface/ConferenceEngine.h, "url"
    // must be a file name.
    int iRC = mpConferenceEngine->GIPSConf_StartPlayFileToMeeting(url, true);
    if (iRC != 0)
    {
        OsSysLog::add(FAC_MP, PRI_ERR,
                      "ConferenceEngineMediaInterface::playAudio GIPSConf_StartPlayFileToMeeting('%s', true) failed with error = %d.",
                      url,
                      mpConferenceEngine->GIPSConf_GetLastError());
    }
    assert(iRC == 0);

    mLock.release();
    return OS_SUCCESS;
}


OsStatus ConferenceEngineMediaInterface::playAudioForIndividual(int connectionId,
                                                                const char* url,
                                                                OsNotification* event)
{
    mLock.acquire();

    assert(url);

    int iRC;

    ConferenceEngineMediaConnection* pMediaConnection = getMediaConnection(connectionId) ;
    if (pMediaConnection)
    {
        // Set up a notification for play done event
        pMediaConnection->mpPlayNotify = event;

        iRC = mpConferenceEngine->GIPSConf_StartPlayFileToChannel(connectionId, url);
        if (iRC != 0)
        {
            OsSysLog::add(FAC_MP, PRI_ERR,
                          "ConferenceEngineMediaInterface::playAudioForIndividual GIPSConf_StartPlayFileToChannel failed with error = %d.",
                          mpConferenceEngine->GIPSConf_GetLastError());
        }
        assert(iRC == 0);
    }

    mLock.release();
    return OS_SUCCESS;
}


OsStatus ConferenceEngineMediaInterface::playBuffer(char* buf,
                                                    unsigned long bufSize,
                                                    int type,
                                                    UtlBoolean repeat,
                                                    UtlBoolean local,
                                                    UtlBoolean remote,
                                                    OsNotification* event)
{
    OsSysLog::add(FAC_MP, PRI_ERR,
                  "ConferenceEngineMediaInterface::stopDtmf is not supported.");

    return OS_NOT_SUPPORTED ;
}


OsStatus ConferenceEngineMediaInterface::pauseAudio()
{
    OsSysLog::add(FAC_MP, PRI_ERR,
                  "ConferenceEngineMediaInterface::stopDtmf is not supported.");

    return OS_NOT_SUPPORTED ;
}


OsStatus ConferenceEngineMediaInterface::stopAudio()
{
    mLock.acquire();

    ConferenceEngineMediaConnection* pMediaConnection = (ConferenceEngineMediaConnection *) mMediaConnections.first();
    if (pMediaConnection)
    {
        // Set up a notification for play done event
        pMediaConnection->mpPlayNotify = NULL;
    }

    int iRC = mpConferenceEngine->GIPSConf_StopPlayFileToMeeting();
    if (iRC != 0)
    {
        OsSysLog::add(FAC_MP, PRI_ERR,
                      "ConferenceEngineMediaInterface::stopAudio GIPSConf_StopPlayFileToMeeting failed with error = %d.",
                      mpConferenceEngine->GIPSConf_GetLastError());
    }
    assert(iRC == 0);

    mLock.release();
    return OS_SUCCESS;
}


OsStatus ConferenceEngineMediaInterface::stopAudioForIndividual(int connectionId)
{
    mLock.acquire();

    int iRC;

    ConferenceEngineMediaConnection* pMediaConnection = getMediaConnection(connectionId) ;
    if (pMediaConnection)
    {
        // Set up a notification for play done event
        pMediaConnection->mpPlayNotify = NULL;

        iRC = mpConferenceEngine->GIPSConf_StopPlayFileToChannel(connectionId);
        if (iRC != 0)
        {
            OsSysLog::add(FAC_MP, PRI_ERR,
                          "ConferenceEngineMediaInterface::stopAudioForIndividual GIPSConf_StopPlayFileToChannel failed with error = %d.",
                          mpConferenceEngine->GIPSConf_GetLastError());
        }
        assert(iRC == 0);
    }

    mLock.release();
    return OS_SUCCESS;
}


OsStatus ConferenceEngineMediaInterface::createPlayer(MpStreamPlayer** ppPlayer,
                                                      const char* szStream,
                                                      int flags,
                                                      OsMsgQ *pMsgQ,
                                                      const char* szTarget)
{
    OsSysLog::add(FAC_MP, PRI_ERR,
                  "ConferenceEngineMediaInterface::createPlayer is not supported.");

    return OS_NOT_SUPPORTED ;
}


OsStatus ConferenceEngineMediaInterface::destroyPlayer(MpStreamPlayer* pPlayer)
{
    OsSysLog::add(FAC_MP, PRI_ERR,
                  "ConferenceEngineMediaInterface::destroyPlayer is not supported.");

    return OS_NOT_SUPPORTED ;
}


OsStatus ConferenceEngineMediaInterface::createPlaylistPlayer(MpStreamPlaylistPlayer** ppPlayer,
                                                              OsMsgQ *pMsgQ,
                                                              const char* szTarget)
{
    OsSysLog::add(FAC_MP, PRI_ERR,
                  "ConferenceEngineMediaInterface::createPlaylistPlayer is not supported.");

    return OS_NOT_SUPPORTED ;
}


OsStatus ConferenceEngineMediaInterface::destroyPlaylistPlayer(MpStreamPlaylistPlayer* pPlayer)
{
    OsSysLog::add(FAC_MP, PRI_ERR,
                  "ConferenceEngineMediaInterface::destroyPlaylistPlayer is not supported.");

    return OS_NOT_SUPPORTED ;
}


OsStatus ConferenceEngineMediaInterface::createQueuePlayer(MpStreamQueuePlayer** ppPlayer,
                                                           OsMsgQ *pMsgQ,
                                                           const char* szTarget)
{
    OsSysLog::add(FAC_MP, PRI_ERR,
                  "ConferenceEngineMediaInterface::createQueuePlayer is not supported.");

    return OS_NOT_SUPPORTED ;
}


OsStatus ConferenceEngineMediaInterface::destroyQueuePlayer(MpStreamQueuePlayer* pPlayer)
{
    OsSysLog::add(FAC_MP, PRI_ERR,
                  "ConferenceEngineMediaInterface::destroyQueuePlayer is not supported.");

    return OS_NOT_SUPPORTED ;
}


OsStatus ConferenceEngineMediaInterface::startTone(int toneId,
                                                   UtlBoolean local,
                                                   UtlBoolean remote)
{
    OsSysLog::add(FAC_MP, PRI_ERR,
                  "ConferenceEngineMediaInterface::startTone is not supported.");

    return OS_NOT_SUPPORTED;
}


OsStatus ConferenceEngineMediaInterface::stopTone()
{
    OsSysLog::add(FAC_MP, PRI_ERR,
                  "ConferenceEngineMediaInterface::stopTone is not supported.");

    return OS_NOT_SUPPORTED ;
}

OsStatus ConferenceEngineMediaInterface::giveFocus()
{
    mLock.acquire();
    mFocus = TRUE;

    return OS_SUCCESS;
}


OsStatus ConferenceEngineMediaInterface::defocus()
{
    mLock.acquire();
    mFocus = FALSE;

    return OS_SUCCESS;
}


void ConferenceEngineMediaInterface::setCodecCPULimit(int iLimit)
{
    OsSysLog::add(FAC_MP, PRI_ERR,
                  "ConferenceEngineMediaInterface::setCodecCPULimit to %d", iLimit);

    ConferenceEngineMediaConnection* mediaConnection = NULL;
    UtlSListIterator connectionIterator(mMediaConnections);
    while ((mediaConnection = (ConferenceEngineMediaConnection*) connectionIterator()))
    {
        mediaConnection->mpCodecFactory->setCodecCPULimit(iLimit) ;
    }
}


OsStatus ConferenceEngineMediaInterface::stopRecording()
{
    int rc = mpConferenceEngine->GIPSConf_StopRecordingMeeting();
    if (rc != 0)
    {
        OsSysLog::add(FAC_MP, PRI_ERR,
                      "ConferenceEngineMediaInterface::stopRecording: GIPSConf_StopRecordingMeeting failed with error %d",
                       mpConferenceEngine->GIPSConf_GetLastError());
    }
    assert(rc == 0);

    return OS_SUCCESS ;
}


OsStatus ConferenceEngineMediaInterface::ezRecord(int ms,
                                                  int silenceLength,
                                                  const char* fileName,
                                                  double& duration,
                                                  int& dtmfterm,
                                                  OsProtectedEvent* ev)
{
    int rc = mpConferenceEngine->GIPSConf_StartRecordingMeeting(fileName);
    if (rc != 0)
    {
        OsSysLog::add(FAC_MP, PRI_ERR,
                      "ConferenceEngineMediaInterface::ezRecord: GIPSConf_StartRecordingMeeting failed with error %d",
                       mpConferenceEngine->GIPSConf_GetLastError());
    }
    assert(rc == 0);

    return OS_SUCCESS ;
}

void ConferenceEngineMediaInterface::addToneListener(OsNotification *pListener, int connectionId)
{
    mLock.acquire();

    ConferenceEngineMediaConnection* pMediaConnection = getMediaConnection(connectionId) ;
    if (pMediaConnection)
    {
        pMediaConnection->mpDTMFNotify = pListener;
    }

    mLock.release();
}

void ConferenceEngineMediaInterface::removeToneListener(int connectionId)
{
   mLock.acquire();

    ConferenceEngineMediaConnection* pMediaConnection = getMediaConnection(connectionId) ;
    if (pMediaConnection)
    {
        pMediaConnection->mpDTMFNotify = NULL;
    }

    mLock.release();
}

void  ConferenceEngineMediaInterface::setContactType(int connectionId, ContactType eType)
{
    mLock.acquire();

    ConferenceEngineMediaConnection* pMediaConnection = getMediaConnection(connectionId) ;
    if (pMediaConnection)
    {
        pMediaConnection->meContactType = eType;
    }

    mLock.release();
}


/* ============================ ACCESSORS ================================= */

void ConferenceEngineMediaInterface::setPremiumSound(UtlBoolean enabled)
{
    OsSysLog::add(FAC_MP, PRI_ERR,
                  "ConferenceEngineMediaInterface::setPremiumSound is not supported.");
}


// Calculate the current cost for our sending/receiving codecs
int ConferenceEngineMediaInterface::getCodecCPUCost()
{
   int iCost = SdpCodec::SDP_CODEC_CPU_LOW ;

   return iCost ;
}


// Calculate the worst case cost for our sending/receiving codecs
int ConferenceEngineMediaInterface::getCodecCPULimit()
{
   int iCost = SdpCodec::SDP_CODEC_CPU_LOW ;

   return iCost ;
}


// Returns the flowgraph's message queue
OsMsgQ* ConferenceEngineMediaInterface::getMsgQ()
{
    return NULL;
}


OsStatus ConferenceEngineMediaInterface::getPrimaryCodec(int connectionId,
                                                         UtlString& codec,
                                                         UtlString& videoCodec,
                                                         int* payloadType,
                                                         int* videoPayloadType)
{
    mLock.acquire();
    OsStatus rc = OS_FAILED;

    ConferenceEngineMediaConnection* pMediaConnection = getMediaConnection(connectionId);
    if (pMediaConnection && pMediaConnection->mpPrimaryCodec)
    {
        if (getCodecNameByType(pMediaConnection->mpPrimaryCodec->getCodecType(), codec))
        {
            if (NULL != payloadType)
            {
                *payloadType = pMediaConnection->mRtpPayloadType;
            }
            rc = OS_SUCCESS;
        }
    }

    mLock.release();
    return rc;
}

/* ============================ INQUIRY =================================== */
UtlBoolean ConferenceEngineMediaInterface::isSendingRtpAudio(int connectionId)
{
    mLock.acquire();
    UtlBoolean bSending = FALSE ;

    ConferenceEngineMediaConnection* pMediaConnection = getMediaConnection(connectionId);
    if (pMediaConnection)
    {
        bSending = pMediaConnection->mRtpSending ;
    }

    mLock.release();
    return bSending ;
}

UtlBoolean ConferenceEngineMediaInterface::isReceivingRtpAudio(int connectionId)
{
    mLock.acquire();
    UtlBoolean bReceiving = FALSE ;

    ConferenceEngineMediaConnection* pMediaConnection = getMediaConnection(connectionId);
    if (pMediaConnection)
    {
        bReceiving = pMediaConnection->mRtpReceiving ;
    }


    mLock.release();
    return bReceiving ;
}

UtlBoolean ConferenceEngineMediaInterface::isSendingRtpVideo(int connectionId)
{
    return FALSE;
}

UtlBoolean ConferenceEngineMediaInterface::isReceivingRtpVideo(int connectionId)
{
    return FALSE;
}

UtlBoolean ConferenceEngineMediaInterface::isDestinationSet(int connectionId)
{
    mLock.acquire();
    UtlBoolean bDestination = FALSE ;

    ConferenceEngineMediaConnection* pMediaConnection = getMediaConnection(connectionId);
    if (pMediaConnection)
    {
        bDestination = pMediaConnection->mDestinationSet ;
    }


    mLock.release();
    return bDestination ;
}


UtlBoolean ConferenceEngineMediaInterface::canAddParty()
{
    return TRUE;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

UtlBoolean ConferenceEngineMediaInterface::getCodecNameByType(SdpCodec::SdpCodecTypes codecType, UtlString& codecName)
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
    default:
        OsSysLog::add(FAC_MP, PRI_WARNING,
                      "ConferenceEngineMediaInterface::getCodecNameByType unsupported type %d.",
                      codecType);

    }

    if (codecName != "")
    {
        matchFound = TRUE;
    }

    return matchFound;
}


UtlBoolean ConferenceEngineMediaInterface::getCodecTypeByName(const UtlString& codecName, SdpCodec::SdpCodecTypes& codecType)
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

    if (codecType != SdpCodec::SDP_CODEC_UNKNOWN)
    {
        matchFound = TRUE;
    }

    return matchFound;
}


UtlBoolean ConferenceEngineMediaInterface::getConferenceEngineCodec(const SdpCodec& pCodec, GIPS_CodecInst& codecInfo)
{
    mLock.acquire();
    UtlString codecName;
    UtlBoolean matchFound = FALSE;
    int iCodecs;

    if (getCodecNameByType(pCodec.getCodecType(), codecName))
    {
        if ((iCodecs=mpConferenceEngine->GIPSConf_GetNofCodecs()) != -1)
        {
            for (int i=0; i<iCodecs; ++i)
            {
                if (mpConferenceEngine->GIPSConf_GetCodec(i, &codecInfo) == 0)
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

    mLock.release();
    return matchFound;
}


ConferenceEngine* const ConferenceEngineMediaInterface::getConferenceEnginePtr()
{
    return mpConferenceEngine;
}


OsStatus ConferenceEngineMediaInterface::mute(int connectionId)
{
    mLock.acquire();

    int iRC;

    ConferenceEngineMediaConnection* pMediaConnection = getMediaConnection(connectionId) ;
    if (pMediaConnection)
    {
        iRC = mpConferenceEngine->GIPSConf_StopPlayoutToMeeting(connectionId);
        if (iRC != 0)
        {
            OsSysLog::add(FAC_MP, PRI_ERR,
                          "ConferenceEngineMediaInterface::mute GIPSConf_StopPlayoutToMeeting failed with error = %d.",
                          mpConferenceEngine->GIPSConf_GetLastError());
        }
        assert(iRC == 0);
    }

    mLock.release();
    return OS_SUCCESS;
}


OsStatus ConferenceEngineMediaInterface::unmute(int connectionId)
{
    mLock.acquire();

    int iRC;

    ConferenceEngineMediaConnection* pMediaConnection = getMediaConnection(connectionId) ;
    if (pMediaConnection)
    {
        iRC = mpConferenceEngine->GIPSConf_StartPlayoutToMeeting(connectionId);
        if (iRC != 0)
        {
            OsSysLog::add(FAC_MP, PRI_ERR,
                          "ConferenceEngineMediaInterface::unmute GIPSConf_StartPlayoutToMeeting failed with error = %d.",
                          mpConferenceEngine->GIPSConf_GetLastError());
        }
        assert(iRC == 0);
    }

    mLock.release();
    return OS_SUCCESS;
}


OsStatus ConferenceEngineMediaInterface::setDTMF(int connectionId)
{
    mLock.acquire();

    int rc;
    if (mDTMFOutOfBand)
    {
        rc = mpConferenceEngine->GIPSConf_OutbandDTMFdetection(connectionId, true);
        if (rc != 0)
        {
            OsSysLog::add(FAC_MP, PRI_ERR,
                          "ConferenceEngineMediaInterface::setDTMF GIPSConf_OutbandDTMFdetection failed with error = %d.",
                          mpConferenceEngine->GIPSConf_GetLastError());
        }
    }
    else
    {
        rc = mpConferenceEngine->GIPSConf_InbandDTMFdetection(connectionId, true);
        if (rc != 0)
        {
            OsSysLog::add(FAC_MP, PRI_ERR,
                          "ConferenceEngineMediaInterface::setDTMF GIPSConf_InbandDTMFdetection failed with error = %d.",
                          mpConferenceEngine->GIPSConf_GetLastError());
        }
    }

    assert(rc == 0);

    mLock.release();
    return OS_SUCCESS;
}


OsStatus ConferenceEngineMediaInterface::setVolume(int connectionId)
{
    mLock.acquire();

    int iRC;

    ConferenceEngineMediaConnection* pMediaConnection = getMediaConnection(connectionId) ;
    if (pMediaConnection)
    {
        iRC = mpConferenceEngine->GIPSConf_SetVolume(connectionId, mDefaultVolume);
        if (iRC != 0)
        {
            OsSysLog::add(FAC_MP, PRI_ERR,
                          "ConferenceEngineMediaInterface::setVolume GIPSConf_SetVolume failed with error = %d.",
                          mpConferenceEngine->GIPSConf_GetLastError());
        }
        assert(iRC == 0);
    }

    mLock.release();
    return OS_SUCCESS;
}


OsStatus ConferenceEngineMediaInterface::setVideoWindowDisplay(const void* pDisplay)
{
    OsStatus rc = OS_NOT_SUPPORTED;

    return rc;
}


const void* ConferenceEngineMediaInterface::getVideoWindowDisplay()
{
    return NULL;
}


OsNotification* ConferenceEngineMediaInterface::getDTMFNotifier(int connectionId)
{
    mLock.acquire();

    OsNotification* notifier = NULL;

    ConferenceEngineMediaConnection* pMediaConnection = getMediaConnection(connectionId) ;
    if (pMediaConnection)
    {
        notifier =  pMediaConnection->mpDTMFNotify;
    }

    mLock.release();
    return notifier;
}


OsNotification* ConferenceEngineMediaInterface::getPlayNotifier(int connectionId)
{
    mLock.acquire();

    OsNotification* notifier = NULL;

    ConferenceEngineMediaConnection* pMediaConnection = getMediaConnection(connectionId) ;
    if (pMediaConnection)
    {
        notifier =  pMediaConnection->mpPlayNotify;
    }

    mLock.release();
    return notifier;
}


/* ============================ FUNCTIONS ================================= */
