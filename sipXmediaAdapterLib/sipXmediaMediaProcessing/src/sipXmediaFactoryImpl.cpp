//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// SYSTEM INCLUDES
#include <stdlib.h>

// APPLICATION INCLUDES
#include "os/OsConfigDb.h"
#include "include/sipXmediaFactoryImpl.h"
#include "include/CpPhoneMediaInterface.h"
#include "net/SdpCodec.h"
#include "mp/MpMediaTask.h"
#include "mp/MpMisc.h"
#include "mp/MpCodec.h"
#include "mp/MpCallFlowGraph.h"
#include "mp/dmaTask.h"
#include "net/SdpCodecFactory.h"
#include "mi/CpMediaInterfaceFactoryFactory.h"

#ifdef INCLUDE_RTCP /* [ */
#include "rtcp/RTCManager.h"
#endif /* INCLUDE_RTCP ] */


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// GLOBAL FUNCTION

#define CONFIG_PHONESET_SEND_INBAND_DTMF  "PHONESET_SEND_INBAND_DTMF"
#define MAX_MANAGED_FLOW_GRAPHS           16

// STATIC VARIABLE INITIALIZATIONS
int sipXmediaFactoryImpl::miInstanceCount=0;

static CpMediaInterfaceFactory* spFactory = NULL;
static int siInstanceCount=0;

extern "C" CpMediaInterfaceFactory* sipXmediaFactoryFactory(OsConfigDb* pConfigDb)
{
    // TODO: Add locking

    if (spFactory == NULL)
    {
        spFactory = new CpMediaInterfaceFactory();
        spFactory->setFactoryImplementation(new sipXmediaFactoryImpl(pConfigDb));
    }
    siInstanceCount++ ;

    // Assert some sane value
    assert(siInstanceCount < 11) ;
    return spFactory;
}

extern "C" void sipxDestroyMediaFactoryFactory()
{
    // TODO: Add locking

    siInstanceCount-- ;
    assert(siInstanceCount >= 0) ;
    if (siInstanceCount == 0)
    {
        if (spFactory)
        {
            delete spFactory ;
            spFactory = NULL ;
        }
    }
}


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
sipXmediaFactoryImpl::sipXmediaFactoryImpl(OsConfigDb* pConfigDb)
{
    int maxFlowGraph = -1 ;
    UtlString strInBandDTMF ;

    if (pConfigDb)
    {
        pConfigDb->get("PHONESET_MAX_ACTIVE_CALLS_ALLOWED", maxFlowGraph) ;
        pConfigDb->get(CONFIG_PHONESET_SEND_INBAND_DTMF, strInBandDTMF) ;
        strInBandDTMF.toUpper() ;

        OsSysLog::add(FAC_MP, PRI_DEBUG,
                      "sipXmediaFactoryImpl::sipXmediaFactoryImpl maxFlowGraph = %d",
                      maxFlowGraph);
    }

    // Max Flow graphs
    if (maxFlowGraph <=0 )
    {
        maxFlowGraph = MAX_MANAGED_FLOW_GRAPHS;
    }
    if (miInstanceCount == 0)
    {
        mpStartUp(8000, 80, 16*maxFlowGraph, pConfigDb);
    }

    // Should we send inband DTMF by default?
    if (strInBandDTMF.compareTo("DISABLE") == 0)
    {
        MpCallFlowGraph::setInbandDTMF(false) ;
    }
    else
    {
        MpCallFlowGraph::setInbandDTMF(true) ;
    }

    // init the media processing task
    mpMediaTask = MpMediaTask::getMediaTask(maxFlowGraph);

#ifdef INCLUDE_RTCP /* [ */
    mpiRTCPControl = CRTCManager::getRTCPControl();
#endif /* INCLUDE_RTCP ] */

    if (miInstanceCount == 0)
    {
        mpStartTasks();
    }

    miGain = 7 ;
    ++miInstanceCount;
}


// Destructor
sipXmediaFactoryImpl::~sipXmediaFactoryImpl()
{
    // TODO: Shutdown
    --miInstanceCount;
    if (miInstanceCount == 0)
    {
        // Temporarily comment out this function because it causes the program hung.
        mpStopTasks();
        mpShutdown();
    }
}

/* ============================ MANIPULATORS ============================== */

CpMediaInterface* sipXmediaFactoryImpl::createMediaInterface( const char* publicAddress,
                                                              const char* localAddress,
                                                              int numCodecs,
                                                              SdpCodec* sdpCodecArray[],
                                                              const char* locale,
                                                              int expeditedIpTos,
                                                              const char* szStunServer,
                                                              int stunOptions,
                                                              int iStunKeepAliveSecs
                                                            )
{
    return new CpPhoneMediaInterface(this, publicAddress, localAddress,
            numCodecs, sdpCodecArray, locale, expeditedIpTos, szStunServer,
            iStunKeepAliveSecs) ;
}


OsStatus sipXmediaFactoryImpl::setSpeakerVolume(int iVolume)
{
    OsStatus rc = OS_SUCCESS ;
//#ifdef WIN32
    MpCodec_setVolume(iVolume) ;
//#endif

    return rc ;
}

OsStatus sipXmediaFactoryImpl::setSpeakerDevice(const UtlString& device)
{
    OsStatus rc = OS_SUCCESS ;
#ifdef WIN32
    DmaTask::setCallDevice(device.data()) ;
#endif
    return rc ;
}

OsStatus sipXmediaFactoryImpl::setMicrophoneGain(int iGain)
{
    OsStatus rc ;

    miGain = iGain ;
#   ifdef WIN32
    rc = MpCodec_setGain(miGain) ;
#   else
    rc = OS_FAILED;
#   endif
    return rc ;
}

OsStatus sipXmediaFactoryImpl::setMicrophoneDevice(const UtlString& device)
{
    OsStatus rc = OS_SUCCESS ;
#ifdef WIN32
    DmaTask::setInputDevice(device.data()) ;
#endif
    return rc ;
}

OsStatus sipXmediaFactoryImpl::muteMicrophone(UtlBoolean bMute)
{
//#ifdef WIN32
    if (bMute)
    {
        MpCodec_setGain(0) ;
    }
    else
    {
        MpCodec_setGain(miGain) ;
    }
//#endif
    return OS_SUCCESS ;
}

OsStatus sipXmediaFactoryImpl::enableAudioAEC(UtlBoolean bEnable)
{
    return OS_SUCCESS;
}

OsStatus sipXmediaFactoryImpl::enableOutOfBandDTMF(UtlBoolean bEnable)
{
    return OS_SUCCESS;
}

OsStatus sipXmediaFactoryImpl::buildCodecFactory(SdpCodecFactory *pFactory,
                                                 const UtlString& sPreferences,
                                                 const UtlString& sVideoPreferences,
                                                 int* iRejected)
{
    OsStatus rc = OS_FAILED;

    int numCodecs = 0;
    UtlString codecName;
    UtlString codecList;

    *iRejected = 0;

#ifdef HAVE_GIPS /* [ */
    numCodecs = 6;
    SdpCodec::SdpCodecTypes codecs[6];

    codecs[0] = SdpCodec::SDP_CODEC_GIPS_PCMU;
    codecs[1] = SdpCodec::SDP_CODEC_GIPS_PCMA;
    codecs[2] = SdpCodec::SDP_CODEC_GIPS_IPCMU;
    codecs[3] = SdpCodec::SDP_CODEC_GIPS_IPCMA;
    codecs[4] = SdpCodec::SDP_CODEC_GIPS_IPCMWB;
    codecs[5] = SdpCodec::SDP_CODEC_TONES;

#else /* HAVE_GIPS ] [ */
    numCodecs = 3;
    SdpCodec::SdpCodecTypes codecs[3];

    codecs[0] = SdpCodec::SDP_CODEC_GIPS_PCMU;
    codecs[1] = SdpCodec::SDP_CODEC_GIPS_PCMA;
    codecs[2] = SdpCodec::SDP_CODEC_TONES;
#endif /* HAVE_GIPS ] */

    if (pFactory)
    {
        pFactory->clearCodecs();

        // add preferred codecs first
        if (sPreferences.length() > 0)
        {
            UtlString references = sPreferences;
            *iRejected = pFactory->buildSdpCodecFactory(references);
            OsSysLog::add(FAC_MP, PRI_DEBUG,
                          "sipXmediaFactoryImpl::buildCodecFactory: sReferences = %s with NumReject %d",
                           references.data(), *iRejected);

            // Now pick preferences out of all available codecs
            SdpCodec** codecsArray = NULL;
            pFactory->getCodecs(numCodecs, codecsArray);

            UtlString preferences;
            for (int i = 0; i < numCodecs; i++)
            {
                if (getCodecNameByType(codecsArray[i]->getCodecType(), codecName) == OS_SUCCESS)
                {
                    preferences = preferences + " " + codecName;
                }
            }

            pFactory->clearCodecs();
            *iRejected = pFactory->buildSdpCodecFactory(preferences);
            OsSysLog::add(FAC_MP, PRI_DEBUG,
                          "sipXmediaFactoryImpl::buildCodecFactory: supported codecs = %s with NumReject %d",
                          preferences.data(), *iRejected);

            // Free up the codecs and the array
            for (int i = 0; i < numCodecs; i++)
            {
                delete codecsArray[i];
                codecsArray[i] = NULL;
            }
            delete[] codecsArray;
            codecsArray = NULL;

            rc = OS_SUCCESS;
        }
        else
        {
            // Build up the supported codecs
            *iRejected = pFactory->buildSdpCodecFactory(numCodecs, codecs);
            rc = OS_SUCCESS;
        }
    }

    return rc;
}


OsStatus sipXmediaFactoryImpl::updateVideoPreviewWindow(void* displayContext)
{
    return OS_NOT_SUPPORTED ;
}


/* ============================ ACCESSORS ================================= */

OsStatus sipXmediaFactoryImpl::getSpeakerVolume(int& iVolume) const
{
    OsStatus rc = OS_SUCCESS ;

//#ifdef WIN32
    iVolume = MpCodec_getVolume() ;
//#endif
    return rc ;
}

OsStatus sipXmediaFactoryImpl::getSpeakerDevice(UtlString& device) const
{
    OsStatus rc = OS_SUCCESS ;

#ifdef WIN32
    device = DmaTask::getCallDevice() ;
#endif
    return rc ;
}


OsStatus sipXmediaFactoryImpl::getMicrophoneGain(int& iGain) const
{
    OsStatus rc = OS_SUCCESS ;

#ifdef WIN32
    iGain = MpCodec_getGain() ;
#endif
    return rc ;
}


OsStatus sipXmediaFactoryImpl::getMicrophoneDevice(UtlString& device) const
{
    OsStatus rc = OS_SUCCESS ;

#ifdef WIN32
    device = DmaTask::getMicDevice() ;
#endif
    return rc ;
}


OsStatus sipXmediaFactoryImpl::isAudioAECEnabled(UtlBoolean& bEnabled) const
{
    bEnabled = false;
    return OS_SUCCESS;
}


OsStatus sipXmediaFactoryImpl::isOutOfBandDTMFEnabled(UtlBoolean& bEnabled) const
{
    bEnabled = false;
    return OS_SUCCESS;
}


OsStatus sipXmediaFactoryImpl::getNumOfCodecs(int& iCodecs) const
{
    iCodecs = 3;
    return OS_SUCCESS;
}


OsStatus sipXmediaFactoryImpl::getCodec(int iCodec, UtlString& codec, int &bandWidth) const
{
    OsStatus rc = OS_SUCCESS;

    switch (iCodec)
    {
#ifdef HAVE_GIPS /* [ */
    case 0: codec = (const char*) SdpCodec::SDP_CODEC_GIPS_PCMU;
        break;
    case 1: codec = (const char*) SdpCodec::SDP_CODEC_GIPS_PCMA;
        break;
#else /* HAVE_GIPS ] [ */
    case 0: codec = (const char*) SdpCodec::SDP_CODEC_GIPS_PCMU;
        break;
    case 1: codec = (const char*) SdpCodec::SDP_CODEC_GIPS_PCMA;
        break;
#endif /* HAVE_GIPS ] */
    case 2: codec = (const char*) SdpCodec::SDP_CODEC_TONES;
        break;
    default: rc = OS_FAILED;
    }

    return rc;
}

OsStatus sipXmediaFactoryImpl::setVideoPreviewDisplay(void* pDisplay)
{
    return OS_NOT_YET_IMPLEMENTED;
}

OsStatus sipXmediaFactoryImpl::setVideoQuality(int quality)
{
    return OS_NOT_YET_IMPLEMENTED;
}

OsStatus sipXmediaFactoryImpl::setVideoParameters(int bitRate, int frameRate)
{
    return OS_NOT_YET_IMPLEMENTED;
}

OsStatus sipXmediaFactoryImpl::getVideoQuality(int& quality) const
{
    return OS_NOT_YET_IMPLEMENTED;
}

OsStatus sipXmediaFactoryImpl::getVideoBitRate(int& bitRate) const
{
    return OS_NOT_YET_IMPLEMENTED;
}

OsStatus sipXmediaFactoryImpl::getVideoFrameRate(int& frameRate) const
{
    return OS_NOT_YET_IMPLEMENTED;
}

OsStatus sipXmediaFactoryImpl::getCodecNameByType(SdpCodec::SdpCodecTypes type, UtlString& codecName) const
{
    OsStatus rc = OS_FAILED;

    codecName = "";

    switch (type)
    {
    case SdpCodec::SDP_CODEC_TONES:
        codecName = GIPS_CODEC_ID_TELEPHONE;
        break;
    case SdpCodec::SDP_CODEC_G729A:
        codecName = GIPS_CODEC_ID_G729;
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
                      "sipXmediaFactoryImpl::getCodecNameByType unsupported type %d.",
                      type);

    }

    if (codecName != "")
    {
        rc = OS_SUCCESS;
    }

    return rc;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
