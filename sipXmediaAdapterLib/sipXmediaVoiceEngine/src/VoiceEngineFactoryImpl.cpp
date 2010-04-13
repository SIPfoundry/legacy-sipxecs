// $Id$
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <stdlib.h>
#include <assert.h>

// APPLICATION INCLUDES
#include "include/VoiceEngineFactoryImpl.h"
#include "os/OsConfigDb.h"
#include "mi/CpMediaInterfaceFactoryFactory.h"
#include <net/SdpCodec.h>
#include <net/SdpCodecFactory.h>
#include <utl/UtlSListIterator.h>
#include <os/OsSysLog.h>


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define CONFIG_PHONESET_SEND_INBAND_DTMF  "PHONESET_SEND_INBAND_DTMF"
#define MAX_MANAGED_FLOW_GRAPHS           16
#define MAX_AUDIO_DEVICES                 16
// STATIC VARIABLE INITIALIZATIONS

static CpMediaInterfaceFactory* spFactory = NULL;
static int siInstanceCount=0;

extern "C" CpMediaInterfaceFactory* sipXmediaFactoryFactory(OsConfigDb* pConfigDb)
{
    // TODO: Add locking

    if (spFactory == NULL)
    {
        spFactory = new CpMediaInterfaceFactory();
        spFactory->setFactoryImplementation(new VoiceEngineFactoryImpl(pConfigDb));
    }
    siInstanceCount++ ;

    // Assert some sane value
    assert(siInstanceCount < 10) ;
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
VoiceEngineFactoryImpl::VoiceEngineFactoryImpl(OsConfigDb* pConfigDb) :
    mMediaLib(GetGipsVoiceEngineLib()),
#ifdef VIDEO
    mVideoLib(GetGipsVideoEngine()),
#endif
    mCurrentWaveInDevice(0),
    mCurrentWaveOutDevice(0),
    mVideoQuality(2),
    mVideoBitRate(300),
    mVideoFrameRate(30),
    mbMute(false),
    mRtpPortLock(OsMutex::Q_FIFO)
{
    int maxFlowGraph = -1 ;
    int rc ;
    int lastError ;
    UtlString strOOBBandDTMF ;
    mbDTMFOutOfBand = TRUE;
    mpPreviewWindowDisplay = NULL ;

    if (pConfigDb)
    {
        pConfigDb->get("PHONESET_MAX_ACTIVE_CALLS_ALLOWED", maxFlowGraph) ;
        pConfigDb->get("CONFIG_PHONESET_SEND_OOB_DTMF", strOOBBandDTMF) ;
        strOOBBandDTMF.toUpper() ;

        if (strOOBBandDTMF.compareTo("DISABLE") == 0)
        {
            mbDTMFOutOfBand = FALSE;
        }
        else
        {
            mbDTMFOutOfBand = TRUE;
        }
    }

    // Max Flow graphs
    if (maxFlowGraph <=0 )
    {
        maxFlowGraph = MAX_MANAGED_FLOW_GRAPHS;
    }

    if (siInstanceCount == 0)
    {
        // start up the media sub system
#ifdef SIPXTAPI_VOICE_ENGINE_DLL
        char* szKey = "YOUR KEY GOES HERE";
        rc = mMediaLib.GIPSVE_Authenticate(szKey, strlen(szKey));
        assert(rc == 0);
        lastError = mMediaLib.GIPSVE_GetLastError();
        //assert(lastError == 0);
#endif
        rc = mMediaLib.GIPSVE_Init() ;
        assert(rc == 0);
        lastError = mMediaLib.GIPSVE_GetLastError();
        //assert(lastError == 0);
#ifdef GIPS_TRACE
        mMediaLib.GIPSVE_SetTrace(2);
#endif
        // Disable Automatic Gain Control by default
        rc = mMediaLib.GIPSVE_SetAGCStatus(0);
        assert(rc == 0);

        mConnectionId = mMediaLib.GIPSVE_CreateChannel() ;
        rc = mMediaLib.GIPSVE_StartPlayout(mConnectionId) ;
        assert(rc == 0);
        lastError = mMediaLib.GIPSVE_GetLastError();
        //assert(lastError == 0);

#ifdef VIDEO
        int rc3 = mVideoLib.GIPSVideo_Init(&mMediaLib) ;
        // assert(rc3 == 0) ;

        #ifdef GIPS_TRACE
            mVideoLib.GIPSVideo_SetTrace(2);
        #endif
#endif
    }
}


// Destructor
VoiceEngineFactoryImpl::~VoiceEngineFactoryImpl()
{
    int rc;

#ifdef VIDEO
    rc = mVideoLib.GIPSVideo_Terminate() ;
    assert(rc == 0);
#endif
    rc = mMediaLib.GIPSVE_Terminate();
    assert(rc == 0);

    if (siInstanceCount == 0)
    {
        mMediaLib.GIPSVE_DeleteChannel(mConnectionId) ;
        rc = mMediaLib.GIPSVE_Terminate();
        assert(rc == 0);
    }
    if (mpPreviewWindowDisplay)
    {
        delete mpPreviewWindowDisplay;
    }
}

/* ============================ MANIPULATORS ============================== */

CpMediaInterface* VoiceEngineFactoryImpl::createMediaInterface( const char* publicAddress,
                                                                const char* localAddress,
                                                                int numCodecs,
                                                                SdpCodec* sdpCodecArray[],
                                                                const char* locale,
                                                                int expeditedIpTos,
                                                                const char* szStunServer,
                                                                int iStunKeepAliveSecs,
                                                                int stunOptions
                                                               )
{
    VoiceEngineMediaInterface* pMediaInterface =
        new VoiceEngineMediaInterface(this, publicAddress, localAddress,
                numCodecs, sdpCodecArray, locale, expeditedIpTos,
                szStunServer, stunOptions, iStunKeepAliveSecs, mbDTMFOutOfBand) ;

    // store it in our internal list, as an int
    UtlInt* piMediaInterface = new UtlInt(pMediaInterface);
    mInterfaceList.insert(piMediaInterface);

    pMediaInterface->getVoiceEnginePtr()->GIPSVE_SetSoundDevices(mCurrentWaveInDevice, mCurrentWaveOutDevice);
    return pMediaInterface;
}

void VoiceEngineFactoryImpl::removeInterface(VoiceEngineMediaInterface* pMediaInterface)
{
    mInterfaceList.destroy(& UtlInt(pMediaInterface));
}


OsStatus VoiceEngineFactoryImpl::setSpeakerVolume(int iVolume)
{
    OsStatus rc = OS_FAILED ;
    int gipsVolume = (int) (((float)iVolume / 100.0) * 255.0 );

    if (iVolume < 0 || iVolume > 100)
    {
        rc = OS_FAILED;
    }
    else if (0 == mMediaLib.GIPSVE_SetSpeakerVolume(gipsVolume))
    {
        rc = OS_SUCCESS;
    }
    return rc ;
}

OsStatus VoiceEngineFactoryImpl::setSpeakerDevice(const UtlString& device)
{
    OsStatus rc = OS_FAILED;

    UtlSListIterator iterator(mInterfaceList);
    VoiceEngineMediaInterface* pInterface = NULL;
    UtlInt* pInterfaceInt = NULL;

    if (outputDeviceStringToIndex(mCurrentWaveOutDevice, device) == OS_SUCCESS)
    {
        if (0 == mMediaLib.GIPSVE_SetSoundDevices(mCurrentWaveInDevice, mCurrentWaveOutDevice))
        {
            rc = OS_SUCCESS;
        }
    }
    while (pInterfaceInt = (UtlInt*)iterator())
    {
        pInterface = (VoiceEngineMediaInterface*)pInterfaceInt->getValue();
        GipsVoiceEngineLib* pGips = NULL;

        if (pInterface)
        {
            pGips = pInterface->getVoiceEnginePtr();
        }

        if (pGips && outputDeviceStringToIndex(mCurrentWaveOutDevice, device) == OS_SUCCESS)
        {
            if (0 == pGips->GIPSVE_SetSoundDevices(mCurrentWaveInDevice, mCurrentWaveOutDevice))
            {
                rc = OS_SUCCESS;
            }
        }
    }

    return rc ;
}


OsStatus VoiceEngineFactoryImpl::setMicrophoneGain(int iGain)
{
    OsStatus rc = OS_FAILED ;

    if (0 == iGain)
    {
        rc  = muteMicrophone(true);
    }
    else
    {

        int gipsGain = (int)((float)((float)iGain / 100.0) * 255.0);

        //check AGC status
        int x = mMediaLib.GIPSVE_GetAGCStatus();
        assert(0 == x);

        if (iGain < 0 || iGain > 100)
        {
            rc = OS_FAILED;
        }
        else if (0 == mMediaLib.GIPSVE_SetMicVolume(gipsGain))
        {
            miGain = gipsGain;
            rc = OS_SUCCESS;
        }
    }
    return rc ;
}

OsStatus VoiceEngineFactoryImpl::setMicrophoneDevice(const UtlString& device)
{
    OsStatus rc = OS_FAILED;
    UtlSListIterator iterator(mInterfaceList);
    VoiceEngineMediaInterface* pInterface = NULL;
    UtlInt* pInterfaceInt = NULL;

    assert(device.length() > 0);

    if (OS_SUCCESS == inputDeviceStringToIndex(mCurrentWaveInDevice, device)  &&
        0 == mMediaLib.GIPSVE_SetSoundDevices(mCurrentWaveInDevice, mCurrentWaveOutDevice) )
    {
        rc = OS_SUCCESS;
    }
    while (pInterfaceInt = (UtlInt*)iterator())
    {
        pInterface = (VoiceEngineMediaInterface*)pInterfaceInt->getValue();
        GipsVoiceEngineLib* pGips = NULL;

        if (pInterface)
        {
            pGips = pInterface->getVoiceEnginePtr();
        }

        if (pGips && outputDeviceStringToIndex(mCurrentWaveOutDevice, device) == OS_SUCCESS)
        {
            if (0 == pGips->GIPSVE_SetSoundDevices(mCurrentWaveInDevice, mCurrentWaveOutDevice))
            {
                rc = OS_SUCCESS;
            }
        }
    }

    return rc ;
}

OsStatus VoiceEngineFactoryImpl::muteMicrophone(UtlBoolean bMute)
{
    OsStatus rc = OS_SUCCESS;

    mbMute = (bMute==TRUE) ? true : false;

    UtlSListIterator iterator(mInterfaceList);
    UtlInt* iMediaInterface = NULL;
    while ((iMediaInterface = (UtlInt*)iterator()))
    {
        VoiceEngineMediaInterface* pInterface = (VoiceEngineMediaInterface*)iMediaInterface->getValue();
        rc = pInterface->muteMicrophone((bMute==TRUE) ? true : false);
        if (OS_FAILED == rc)
        {
            break;
        }
    }

/*
    if (!bMute)  // restore the Mic Gain if we are un-muting - do we need this?
    {
        mMediaLib.GIPSVE_SetMicVolume(miGain);
    }
*/

    return rc ;
}

OsStatus VoiceEngineFactoryImpl::enableAudioAEC(UtlBoolean bEnable)
{
    int irc;
    OsStatus rc = OS_FAILED;

    // iterate through all of the VoiceEngineMedia interface pointers,
    // and set AEC for all of them
    if (bEnable)
    {
        // Mode 1: echo cancellation enable, mode2: automatic
        if (0 == mMediaLib.GIPSVE_SetECStatus(1))
        {
            // Type 0: echo canvcellation, type 1: echo suppression
            irc = mMediaLib.GIPSVE_SetECType(0);
            assert(irc == 0);
            rc = OS_SUCCESS;
        }
    }
    else
    {
        if (0 == mMediaLib.GIPSVE_SetECStatus(0))
        {
            rc = OS_SUCCESS;
        }
    }

    // iterate through all of the VoiceEngineMedia interface pointers,
    // and set AEC for all of them
    if (OS_SUCCESS == rc)
    {
        UtlSListIterator iterator(mInterfaceList);
        VoiceEngineMediaInterface* pInterface = NULL;
        UtlInt* pInterfaceInt = NULL;

        while (pInterfaceInt = (UtlInt*)iterator())
        {
            pInterface = (VoiceEngineMediaInterface*)pInterfaceInt->getValue();
            GipsVoiceEngineLib* pGips = NULL;

            if (pInterface)
            {
                pGips = pInterface->getVoiceEnginePtr();
            }

            if (pGips)
            {
                if (bEnable)
                {
                    // Mode 1: echo cancellation enable, mode2: automatic
                    if (0 == pGips->GIPSVE_SetECStatus(1))
                    {
                        // Type 0: echo canvcellation, type 1: echo suppression
                        irc = pGips->GIPSVE_SetECType(0);
                        assert(irc == 0);
                        rc = OS_SUCCESS;
                    }
                }
                else
                {
                    if (0 == pGips->GIPSVE_SetECStatus(0))
                    {
                        rc = OS_SUCCESS;
                    }
                }
            } // end if pGips
        } // end while
    } // end if (OS_SUCCESS == rc)

    return rc;
}

OsStatus VoiceEngineFactoryImpl::enableOutOfBandDTMF(UtlBoolean bEnable)
{
    mbDTMFOutOfBand = bEnable;

    return OS_SUCCESS;
}

OsStatus VoiceEngineFactoryImpl::buildCodecFactory(SdpCodecFactory *pFactory,
                                                   const UtlString& sAudioPreferences,
                                                   const UtlString& sVideoPreferences,
                                                   int* iRejected)
{
    OsStatus rc = OS_FAILED;
    int iCodecs = 0;
    UtlString codec;
    UtlString codecList;
    int index;
    GIPSVE_CodecInst cInst;
#ifdef VIDEO
    GIPSVideo_CodecInst vInst;
#endif

    *iRejected = 0;
    if (pFactory)
    {
        pFactory->clearCodecs();

        // add preferred codecs first
        if (sAudioPreferences.length() > 0)
        {
            UtlString audioPreferences = sAudioPreferences;
            *iRejected = pFactory->buildSdpCodecFactory(audioPreferences);
            rc = OS_SUCCESS;
        }

        // If there were no preferred codecs specified build factory with all engine codecs
        if (sAudioPreferences.length() == 0 && (iCodecs=mMediaLib.GIPSVE_GetNofCodecs()) != 0)
        {
            for (index=0; index<iCodecs; index++)
            {
                if (mMediaLib.GIPSVE_GetCodec(index, &cInst) == 0)
                {
                    // Check if this already got added with the preferences
                    SdpCodec::SdpCodecTypes cType = pFactory->getCodecType(cInst.plname);

                    if (pFactory->getCodec(cType) == NULL)
                    {
                        codecList = codecList + " " + cInst.plname;
                    }
                }
                else
                {
                    assert(0);
                }
            }
            *iRejected += pFactory->buildSdpCodecFactory(codecList);
            rc = OS_SUCCESS;
        }
        else
        {
            if (iCodecs == 0)
            {
                OsSysLog::add(FAC_MP, PRI_DEBUG,
                              "buildCidecFactory: VoiceEngine has no codecs");
            }
        }
#ifdef VIDEO
        // For now include all video codecs
        rc = OS_FAILED;
        codecList = "";
        // add preferred codecs first
        if (sVideoPreferences.length() > 0)
        {
            UtlString videoPreferences = sVideoPreferences;
            *iRejected = pFactory->buildSdpCodecFactory(videoPreferences);
            rc = OS_SUCCESS;
        }
        else
        {
            if ((iCodecs=mVideoLib.GIPSVideo_GetNofCodecs()) != 0)
            {
                UtlString firstName;
                for (index=0; index<iCodecs; index++)
                {
                    if (mVideoLib.GIPSVideo_GetCodec(index, &vInst) == 0)
                    {
                        char *p = vInst.plname;
                        while (*p && *p != ' ') ++p;
                        *p = '\0';
                        codecList = codecList + " " + vInst.plname;
                        codecList = codecList + "-CIF";
                        codecList = codecList + " " + vInst.plname;
                        codecList = codecList + "-QCIF";
                        codecList = codecList + " " + vInst.plname;
                        codecList = codecList + "-SQCIF";
                    }
                    else
                    {
                        assert(0);
                    }
                }
                *iRejected += pFactory->buildSdpCodecFactory(codecList);
                rc = OS_SUCCESS;
            }
            else
            {
                if (iCodecs == 0)
                {
                    OsSysLog::add(FAC_MP, PRI_DEBUG,
                                "buildCidecFactory: VideoEngine has no codecs");
                }
            }
        }
#endif

    }

    return rc;
}


// Set the global video preview window
OsStatus VoiceEngineFactoryImpl::setVideoPreviewDisplay(void* pDisplay)
{
    if (mpPreviewWindowDisplay)
    {
        delete mpPreviewWindowDisplay;
        mpPreviewWindowDisplay = NULL;
    }

    if (pDisplay)
    {
        SIPXVE_VIDEO_DISPLAY* pVideoDisplay = (SIPXVE_VIDEO_DISPLAY*) pDisplay;
        mpPreviewWindowDisplay = new SIPXVE_VIDEO_DISPLAY(*pVideoDisplay) ;
    }

    return OS_SUCCESS ;
}

void* VoiceEngineFactoryImpl::getVideoPreviewDisplay()
{
    return mpPreviewWindowDisplay;
}

// Update the video preview window given the specified display context.
OsStatus VoiceEngineFactoryImpl::updateVideoPreviewWindow(void* displayContext)
{
    // VIDEO: TODO

    return OS_SUCCESS ;
}

OsStatus VoiceEngineFactoryImpl::setVideoQuality(int quality)
{
    mVideoQuality = quality;
    return OS_SUCCESS ;
}

OsStatus VoiceEngineFactoryImpl::startPlayFile(const char* szFile, bool bLoop)
{
    OsStatus rc = OS_FAILED ;
    int fileFormat ;
    int check ;
    FILE *fp;
    char cBuffer[5] ;

    if ((fp=fopen(szFile,"rb")) != NULL)
    {
        // Determine if file is PCM or WAV
        memset(cBuffer, 0, 5);
        fgets(cBuffer, 5, fp);
        if (strcmp(cBuffer, "RIFF") == 0)
        {
            fileFormat = FILE_FORMAT_WAV_FILE;
        }
        else
        {
            fileFormat = FILE_FORMAT_PCM_FILE;
        }
        fclose(fp);

        check = mMediaLib.GIPSVE_PlayPCM(mConnectionId, (char*) szFile, bLoop, fileFormat) ;
        assert(check == 0);
        check = mMediaLib.GIPSVE_GetLastError();
        //assert(check == 0);

        rc = OS_SUCCESS ;
    }

    return rc ;
}


OsStatus VoiceEngineFactoryImpl::stopPlayFile()
{
    if (mMediaLib.GIPSVE_IsPlayingFile(mConnectionId))
    {
        mMediaLib.GIPSVE_StopPlayingFile(mConnectionId) ;
    }
    return OS_SUCCESS ;
}


OsStatus VoiceEngineFactoryImpl::playTone(int toneId)
{
    int check = mMediaLib.GIPSVE_PlayDTMFTone(toneId);
    assert(check == 0) ;
    check = mMediaLib.GIPSVE_GetLastError();
    //assert(check == 0);

    return OS_SUCCESS ;
}

OsStatus VoiceEngineFactoryImpl::setVideoParameters(int bitRate, int frameRate)
{
    mVideoBitRate = bitRate;
    mVideoFrameRate = frameRate;
    return OS_SUCCESS;
}

/* ============================ ACCESSORS ================================= */

OsStatus VoiceEngineFactoryImpl::getSpeakerVolume(int& iVolume) const
{
    OsStatus rc = OS_SUCCESS ;

    int gipsVolume = 0;
    gipsVolume = mMediaLib.GIPSVE_GetSpeakerVolume();

    iVolume = (int) ((double) ((((double)gipsVolume ) / 255.0) * 100.0) + 0.5) ;

    return rc ;
}

OsStatus VoiceEngineFactoryImpl::getSpeakerDevice(UtlString& device) const
{
    OsStatus rc = OS_SUCCESS ;

    rc = outputDeviceIndexToString(device, mCurrentWaveOutDevice);

    return rc ;
}


OsStatus VoiceEngineFactoryImpl::getMicrophoneGain(int& iGain) const
{
    OsStatus rc = OS_SUCCESS ;
    double gipsGain = (double) mMediaLib.GIPSVE_GetMicVolume();

    iGain = (int) ((double)((double)((gipsGain) / 255.0) * 100.0) + 0.5);

    return rc ;
}


OsStatus VoiceEngineFactoryImpl::getMicrophoneDevice(UtlString& device) const
{
    OsStatus rc = OS_SUCCESS ;

    rc = this->inputDeviceIndexToString(device, this->mCurrentWaveInDevice);
    return rc ;
}


OsStatus VoiceEngineFactoryImpl::getCodecNameByType(SdpCodec::SdpCodecTypes codecType, UtlString& codecName) const
{
    OsStatus rc = OS_FAILED;

    if (VoiceEngineMediaInterface::getCodecNameByType(codecType, codecName))
    {
        rc = OS_SUCCESS;
    }

    return rc;
}


GipsVoiceEngineLib* VoiceEngineFactoryImpl::getVoiceEnginePointer() const
{
    return &mMediaLib;
}

#ifdef VIDEO
GipsVideoEngineWindows* VoiceEngineFactoryImpl::getVideoEnginePointer() const
{
    return &mVideoLib;
}
#endif /* VIDEO */

OsStatus VoiceEngineFactoryImpl::getVideoQuality(int& quality) const
{
    quality = mVideoQuality;

    return OS_SUCCESS;
}

OsStatus VoiceEngineFactoryImpl::getVideoBitRate(int& bitRate) const
{
    bitRate = mVideoBitRate;

    return OS_SUCCESS;
}

OsStatus VoiceEngineFactoryImpl::getVideoFrameRate(int& frameRate) const
{
    frameRate = mVideoFrameRate;

    return OS_SUCCESS;
}

/* ============================ INQUIRY =================================== */

OsStatus VoiceEngineFactoryImpl::isAudioAECEnabled(UtlBoolean& bEnabled) const
{
    OsStatus rc = OS_SUCCESS;

    bEnabled = FALSE;

    switch (mMediaLib.GIPSVE_GetECStatus())
    {
    case 0:
        break;
    case 1:
        bEnabled = TRUE;
        break;
    default:
        rc = OS_FAILED;
        break;
    }

    return rc;
}

OsStatus VoiceEngineFactoryImpl::isOutOfBandDTMFEnabled(UtlBoolean& bEnabled) const
{
    bEnabled = mbDTMFOutOfBand;

    return OS_SUCCESS;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

OsStatus VoiceEngineFactoryImpl::outputDeviceStringToIndex(int& deviceIndex, const UtlString& device) const
{
    OsStatus rc = OS_FAILED;
#ifdef _WIN32
    WAVEOUTCAPS outcaps ;
    int numDevices ;
    int i ;

    numDevices = waveOutGetNumDevs();
    for (i=0; i<numDevices && i<MAX_AUDIO_DEVICES; i++)
    {
        waveOutGetDevCaps(i, &outcaps, sizeof(WAVEOUTCAPS)) ;
        if (device.length() == 0 || device == UtlString(outcaps.szPname))
        {
            deviceIndex = i;
            rc = OS_SUCCESS;
            break;
        }
    }
#endif
    return rc;
}

OsStatus VoiceEngineFactoryImpl::outputDeviceIndexToString(UtlString& device, const int deviceIndex) const
{
    OsStatus rc = OS_FAILED;
#ifdef _WIN32
    WAVEOUTCAPS outcaps ;
    int numDevices ;

    numDevices = waveOutGetNumDevs();
    if (deviceIndex < numDevices)
    {
        waveOutGetDevCaps(deviceIndex, &outcaps, sizeof(WAVEOUTCAPS)) ;
        device = outcaps.szPname;
        rc = OS_SUCCESS;
    }
#endif
    return rc;
}

OsStatus VoiceEngineFactoryImpl::inputDeviceStringToIndex(int& deviceIndex, const UtlString& device) const
{
    OsStatus rc = OS_FAILED;
#ifdef _WIN32
    WAVEINCAPS incaps ;
    int numDevices ;
    int i ;

    numDevices = waveInGetNumDevs();
    for (i=0; i<numDevices && i<MAX_AUDIO_DEVICES; i++)
    {
        waveInGetDevCaps(i, &incaps, sizeof(WAVEINCAPS)) ;
        if (device == UtlString(incaps.szPname))
        {
            deviceIndex = i;
            rc = OS_SUCCESS;
        }
    }
#endif
    return rc;
}

OsStatus VoiceEngineFactoryImpl::inputDeviceIndexToString(UtlString& device, const int deviceIndex) const
{
    OsStatus rc = OS_FAILED;
#ifdef _WIN32
    WAVEINCAPS incaps ;
    int numDevices ;

    numDevices = waveInGetNumDevs();
    if (deviceIndex < numDevices)
    {
        waveInGetDevCaps(deviceIndex, &incaps, sizeof(WAVEINCAPS)) ;
        device = incaps.szPname;
        rc = OS_SUCCESS;
    }
#endif
    return rc;
}

const bool VoiceEngineFactoryImpl::isMuted() const
{
    return mbMute;
}
