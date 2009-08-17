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
#include <utl/UtlVoidPtr.h>
#include <utl/UtlSListIterator.h>
#include <os/OsSysLog.h>
#include <os/OsConfigDb.h>
#include <net/SdpCodec.h>
#include <net/SdpCodecFactory.h>
#include "include/ConferenceEngineFactoryImpl.h"
#include "mi/CpMediaInterfaceFactoryFactory.h"


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
int ConferenceEngineFactoryImpl::mInstanceCount = 0;

CpMediaInterfaceFactory* pFactory = NULL;

extern "C" CpMediaInterfaceFactory* sipXmediaFactoryFactory(OsConfigDb* pConfigDb)
{
    OsSysLog::add(FAC_MP, PRI_DEBUG,
                  "sipXmediaFactoryFactory Loading the GIPS ConferenceEngine ...");
    if (pFactory == NULL)
    {
        pFactory = new CpMediaInterfaceFactory();
        pFactory->setFactoryImplementation(new ConferenceEngineFactoryImpl(pConfigDb));
    }

    return pFactory;
}
/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
ConferenceEngineFactoryImpl::ConferenceEngineFactoryImpl(OsConfigDb* pConfigDb) :
    mConferenceEngine(NewConfEngine())
{
    UtlString strOOBBandDTMF ;
    mDTMFOutOfBand = TRUE;

    if (pConfigDb)
    {
        pConfigDb->get("CONFIG_PHONESET_SEND_OOB_DTMF", strOOBBandDTMF) ;
        strOOBBandDTMF.toUpper() ;

        if (strOOBBandDTMF.compareTo("DISABLE") == 0)
        {
            mDTMFOutOfBand = FALSE;
        }
        else
        {
            mDTMFOutOfBand = TRUE;
        }
    }

    if (mInstanceCount == 0)
    {
        // start up the Conference Engine system
        int rc = mConferenceEngine.GIPSConf_Init();
        if (rc != 0)
        {
            OsSysLog::add(FAC_MP, PRI_ERR,
                          "ConferenceEngineFactoryImpl::ConferenceEngineFactoryImpl GIPS ConferenceEngine failed to initialize. error = %d.",
                          mConferenceEngine.GIPSConf_GetLastError());
        }

        assert(rc == 0);
        OsSysLog::add(FAC_MP, PRI_DEBUG,
                      "ConferenceEngineFactoryImpl::ConferenceEngineFactoryImpl GIPS ConferenceEngine is initialized.");
    }

    mInstanceCount++;
}


// Destructor
ConferenceEngineFactoryImpl::~ConferenceEngineFactoryImpl()
{
    int rc;

    mInstanceCount--;

    if (mInstanceCount == 0)
    {
        rc = mConferenceEngine.GIPSConf_Terminate();
        assert(rc == 0);

        if (mInterfaceList.isEmpty())
        {
            UtlSListIterator iterator(mInterfaceList);
            UtlVoidPtr* container;
            while (container = dynamic_cast <UtlVoidPtr *> (iterator()))
            {
                ConferenceEngineMediaInterface* interface = (ConferenceEngineMediaInterface *) container->getValue();
                mInterfaceList.destroy(container);
                delete interface;
            }
        }
    }
}

/* ============================ MANIPULATORS ============================== */

CpMediaInterface* ConferenceEngineFactoryImpl::createMediaInterface(const char* publicAddress,
                                                                    const char* localAddress,
                                                                    int numCodecs,
                                                                    SdpCodec* sdpCodecArray[],
                                                                    const char* locale,
                                                                    int expeditedIpTos,
                                                                    const char* szStunServer,
                                                                    int stunOptions,
                                                                    int iStunKeepAliveSecs)
{
    ConferenceEngineMediaInterface* pMediaInterface =
        new ConferenceEngineMediaInterface(this, publicAddress, localAddress,
                numCodecs, sdpCodecArray, locale, expeditedIpTos,
                szStunServer, stunOptions, iStunKeepAliveSecs, mDTMFOutOfBand) ;

    // store it in our internal list, as an int
    UtlVoidPtr* mediaInterface = new UtlVoidPtr(pMediaInterface);
    mInterfaceList.insert(mediaInterface);

    return pMediaInterface;
}

void ConferenceEngineFactoryImpl::removeMediaInterface(CpMediaInterface* pMediaInterface)
{
    // The media interface will not be deleted, it will be handled by
    // ConferenceEngineMediaInterface.
    UtlVoidPtr container(pMediaInterface);
    mInterfaceList.destroy(&container);
}


OsStatus ConferenceEngineFactoryImpl::setSpeakerVolume(int iVolume)
{
    OsStatus rc = OS_FAILED ;

    if (iVolume < 0 || iVolume > 100)
    {
        rc = OS_FAILED;
    }
    else
    {
        mGipsVolume = ( ((float) (iVolume - 50) / 50.0) * 20.0 );

        rc = OS_SUCCESS;
    }

    return rc ;
}

OsStatus ConferenceEngineFactoryImpl::setSpeakerDevice(const UtlString& device)
{
    OsStatus rc = OS_NOT_SUPPORTED;

    OsSysLog::add(FAC_MP, PRI_ERR,
                  "ConferenceEngineFactoryImpl::setSpeakerDevice is not supported.");

    return rc ;
}


OsStatus ConferenceEngineFactoryImpl::setMicrophoneGain(int iGain)
{
    OsStatus rc = OS_NOT_SUPPORTED;

    OsSysLog::add(FAC_MP, PRI_ERR,
                  "ConferenceEngineFactoryImpl::setMicrophoneGain is not supported.");

    return rc ;
}

OsStatus ConferenceEngineFactoryImpl::setMicrophoneDevice(const UtlString& device)
{
    OsStatus rc = OS_NOT_SUPPORTED;

    OsSysLog::add(FAC_MP, PRI_ERR,
                  "ConferenceEngineFactoryImpl::setMicrophoneDevice is not supported.");

    return rc ;
}

OsStatus ConferenceEngineFactoryImpl::muteMicrophone(UtlBoolean bMute)
{
    OsStatus rc = OS_NOT_SUPPORTED;

    OsSysLog::add(FAC_MP, PRI_ERR,
                  "ConferenceEngineFactoryImpl::muteMicrophone is not supported.");

    return rc ;
}

OsStatus ConferenceEngineFactoryImpl::enableAudioAEC(UtlBoolean bEnable)
{
    mAGC = bEnable;

    return OS_SUCCESS;
}

OsStatus ConferenceEngineFactoryImpl::enableOutOfBandDTMF(UtlBoolean bEnable)
{
    mDTMFOutOfBand = bEnable;

    return OS_SUCCESS;
}

OsStatus ConferenceEngineFactoryImpl::buildCodecFactory(SdpCodecFactory *pFactory,
                                                        const UtlString& sPreferences,
                                                        const UtlString& sVideoPreferences,
                                                        int* iRejected)
{
    OsStatus rc = OS_FAILED;

    int iCodecs = 0;
    UtlString codec;
    UtlString codecList;
    GIPS_CodecInst cInst;

    *iRejected = 0;
    if (pFactory)
    {
        pFactory->clearCodecs();

        // add preferred codecs first
        if (sPreferences.length() > 0)
        {
            UtlString references = sPreferences;
            *iRejected = pFactory->buildSdpCodecFactory(references);
            OsSysLog::add(FAC_MP, PRI_DEBUG,
                          "ConferenceEngineFactoryImpl::buildCodecFactory: sReferences = %s with NumReject %d",
                           references.data(), *iRejected);
            rc = OS_SUCCESS;
        }
        else
        {
            iCodecs = mConferenceEngine.GIPSConf_GetNofCodecs();
            OsSysLog::add(FAC_MP, PRI_DEBUG,
                          "ConferenceEngineFactoryImpl::buildCodecFactory: Number of codecs in ConferenceEngine = %d",
                          iCodecs);

            for (int index=0; index<iCodecs; index++)
            {
                if (mConferenceEngine.GIPSConf_GetCodec(index, &cInst) == 0)
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

            if (iCodecs > 0)
            {
                *iRejected += pFactory->buildSdpCodecFactory(codecList);
                OsSysLog::add(FAC_MP, PRI_DEBUG,
                              "ConferenceEngineFactoryImpl::buildCodecFactory: codecList = %s with NumReject %d",
                              codecList.data(), *iRejected);
                rc = OS_SUCCESS;
            }
            else
            {
                OsSysLog::add(FAC_MP, PRI_WARNING,
                              "ConferenceEngineFactoryImpl::buildCidecFactory: ConferenceEngine has no codecs");
                rc = OS_FAILED;
            }
        }
    }

    return rc;
}

OsStatus ConferenceEngineFactoryImpl::setVideoPreviewDisplay(void* pDisplay)
{
    OsStatus rc = OS_NOT_SUPPORTED;

    OsSysLog::add(FAC_MP, PRI_ERR,
                  "ConferenceEngineFactoryImpl::setVideoPreviewDisplay is not supported.");

    return rc ;
}

OsStatus ConferenceEngineFactoryImpl::updateVideoPreviewWindow(void* displayContext)
{
    OsStatus rc = OS_NOT_SUPPORTED;

    OsSysLog::add(FAC_MP, PRI_ERR,
                  "ConferenceEngineFactoryImpl::updateVideoPreviewWindow is not supported.");

    return rc ;
}

OsStatus ConferenceEngineFactoryImpl::setVideoQuality(int quality)
{
    OsStatus rc = OS_NOT_SUPPORTED;

    OsSysLog::add(FAC_MP, PRI_ERR,
                  "ConferenceEngineFactoryImpl::setVideoQuality is not supported.");

    return rc ;
}

OsStatus ConferenceEngineFactoryImpl::setVideoParameters(int bitRate, int frameRate)
{
    OsStatus rc = OS_NOT_SUPPORTED;

    OsSysLog::add(FAC_MP, PRI_ERR,
                  "ConferenceEngineFactoryImpl::setVideoParameters is not supported.");

    return rc ;
}


/* ============================ ACCESSORS ================================= */

OsStatus ConferenceEngineFactoryImpl::getSpeakerVolume(int& iVolume) const
{
    OsStatus rc = OS_SUCCESS ;

    iVolume = (int) ((((mGipsVolume ) / 20.0) * 50.0) + 50.0) ;

    return rc ;
}

OsStatus ConferenceEngineFactoryImpl::getSpeakerDevice(UtlString& device) const
{
    OsStatus rc = OS_NOT_SUPPORTED ;

    OsSysLog::add(FAC_MP, PRI_ERR,
                  "ConferenceEngineFactoryImpl::getSpeakerDevice is not supported.");

    return rc ;
}


OsStatus ConferenceEngineFactoryImpl::getMicrophoneGain(int& iGain) const
{
    OsStatus rc = OS_NOT_SUPPORTED ;

    OsSysLog::add(FAC_MP, PRI_ERR,
                  "ConferenceEngineFactoryImpl::getMicrophoneGain is not supported.");

    return rc ;
}


OsStatus ConferenceEngineFactoryImpl::getMicrophoneDevice(UtlString& device) const
{
    OsStatus rc = OS_NOT_SUPPORTED ;

    OsSysLog::add(FAC_MP, PRI_ERR,
                  "ConferenceEngineFactoryImpl::getMicrophoneDevice is not supported.");

    return rc ;
}


OsStatus ConferenceEngineFactoryImpl::getCodecNameByType(SdpCodec::SdpCodecTypes codecType, UtlString& codecName) const
{
    OsStatus rc = OS_FAILED;

    if (ConferenceEngineMediaInterface::getCodecNameByType(codecType, codecName))
    {
        rc = OS_SUCCESS;
    }

    return rc;
}


ConferenceEngine* ConferenceEngineFactoryImpl::getConferenceEnginePointer() const
{
    return &mConferenceEngine;
}


OsStatus ConferenceEngineFactoryImpl::getSystemVolume(float& volume) const
{
    volume = mGipsVolume;

    return OS_SUCCESS ;
}


OsStatus ConferenceEngineFactoryImpl::getVideoQuality(int& quality) const
{
    OsStatus rc = OS_NOT_SUPPORTED;

    OsSysLog::add(FAC_MP, PRI_ERR,
                  "ConferenceEngineFactoryImpl::getVideoQuality is not supported.");

    return rc ;
}


OsStatus ConferenceEngineFactoryImpl::getVideoBitRate(int& bitRate) const
{
    OsStatus rc = OS_NOT_SUPPORTED;

    OsSysLog::add(FAC_MP, PRI_ERR,
                  "ConferenceEngineFactoryImpl::ggetVideoBitRate is not supported.");

    return rc ;
}


OsStatus ConferenceEngineFactoryImpl::getVideoFrameRate(int& frameRate) const
{
    OsStatus rc = OS_NOT_SUPPORTED;

    OsSysLog::add(FAC_MP, PRI_ERR,
                  "ConferenceEngineFactoryImpl::getVideoFrameRate is not supported.");

    return rc ;
}

/* ============================ INQUIRY =================================== */

OsStatus ConferenceEngineFactoryImpl::isAudioAECEnabled(UtlBoolean& bEnabled) const
{
    bEnabled = mAGC;

    return OS_SUCCESS;
}

OsStatus ConferenceEngineFactoryImpl::isOutOfBandDTMFEnabled(UtlBoolean& bEnabled) const
{
    bEnabled = mDTMFOutOfBand;

    return OS_SUCCESS;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
