//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////


#ifndef _ConferenceEngineFactoryImpl_h_
#define _ConferenceEngineFactoryImpl_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "ConferenceEngine.h"
#include <utl/UtlSList.h>
#include "mi/CpMediaInterfaceFactoryImpl.h"
#include "include/ConferenceEngineMediaInterface.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MpMediaTask ;
class OsConfigDb ;
class ConferenceEngineMediaInterface;

/**
 * The ConferenceEngineFactoryImpl is an object derived from CpMediaInterfaceFactoryImpl
 * class. It implements all the interafces defined in CpMediaInterfaceFactoryImpl.
 * However, since ConferenceEngine is only used for the server applications such as
 * conference bridge, there is no concept of setting the microphone gains, setting
 * the speaker devices, etc. Consequently these functions are not supported.
 *
 * Here are the functions being supported in ConferenceEngineFactoryImpl:
 *
 * enableAudioAEC() - enable the automatic gain control (AGC) for the overall system.
 * isAudioAECEnabled() - get the status of AGC setting.
 * setSpeakerVolume() - set a default value for all the conference participants.
 * getSpeakerVolume() - get the current value set for the overall system.
 * enableOutOfBandDTMF() - set the transport of DTMF as out of band mode.
 * isOutOfBandDTMFEnabled() - get the status of DTMF setting.
 * buildCodecFactory() - build a codec factory based on the supported ones.
 * getCodecNameByType() - return the name of codec.
 *
 */
class ConferenceEngineFactoryImpl : public CpMediaInterfaceFactoryImpl
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */


    /// Default constructor
    ConferenceEngineFactoryImpl(OsConfigDb* pConfigDb);


    /// Destructor
    virtual ~ConferenceEngineFactoryImpl();

/* ============================ MANIPULATORS ============================== */

    /// Create a ConferenceEngine based media interface
    virtual CpMediaInterface* createMediaInterface( const char* publicAddress,
                                                    const char* localAddress,
                                                    int numCodecs,
                                                    SdpCodec* sdpCodecArray[],
                                                    const char* locale,
                                                    int expeditedIpTos,
                                                    const char* szStunServer,
                                                    int stunOptions,
                                                    int iStunKeepAliveSecs
                                                  ) ;

    /// Removes a media interface from a list of media interfaces
    void removeMediaInterface(CpMediaInterface* pMediaInterface);

    /// Set a volume for every channel in the system
    virtual OsStatus setSpeakerVolume(int iVolume);

    /// Unsupported
    virtual OsStatus setSpeakerDevice(const UtlString& device);

    /// Unsupported
    virtual OsStatus setMicrophoneGain(int iGain);

    /// Unsupported
    virtual OsStatus setMicrophoneDevice(const UtlString& device);

    /// Unsupported
    virtual OsStatus muteMicrophone(UtlBoolean bMute);

    /// Enable automatic gain control
    virtual OsStatus enableAudioAEC(UtlBoolean bEnable);

    /// Enable out-of-band DTMF
    virtual OsStatus enableOutOfBandDTMF(UtlBoolean bEnable);

    /// Populate the codec factory
    virtual OsStatus buildCodecFactory(SdpCodecFactory *pFactory,
                                       const UtlString& sPreferences,
                                       const UtlString& sVideoPreferences,
                                       int* iRejected);

    /// Set the global video preview window
    virtual OsStatus setVideoPreviewDisplay(void* pDisplay);

    /// Set the global video quality
    virtual OsStatus setVideoQuality(int quality);

    /// Set the global video parameters
    virtual OsStatus setVideoParameters(int bitRate, int frameRate);

    /// Update the video preview window given the specified display context.
    virtual OsStatus updateVideoPreviewWindow(void* displayContext);

/* ============================ ACCESSORS ================================= */

    /// Get the current volume setting
    virtual OsStatus getSpeakerVolume(int& iVolume) const;

    /// Unsupported
    virtual OsStatus getSpeakerDevice(UtlString& device) const;

    /// Unsupported
    virtual OsStatus getMicrophoneGain(int& iVolume) const;

    /// Unsupported
    virtual OsStatus getMicrophoneDevice(UtlString& device) const;

    /// Get the name of the codec
    virtual OsStatus getCodecNameByType(SdpCodec::SdpCodecTypes codecType, UtlString& codecName) const;

    /// Return the pointer of ConferenceEngine
    ConferenceEngine* getConferenceEnginePointer() const;

    /// Get the current system volume setting
    OsStatus getSystemVolume(float& volume) const;

    /// Get video quality
    virtual OsStatus getVideoQuality(int& quality) const;

    /// Get video bit rate
    virtual OsStatus getVideoBitRate(int& bitRate) const;

    /// Get video frame rate
    virtual OsStatus getVideoFrameRate(int& frameRate) const;

/* ============================ INQUIRY =================================== */

    /// Get the status of automatic gain control setting
    virtual OsStatus isAudioAECEnabled(UtlBoolean& bEnabled) const;

    /// Get the status of DTMF setting
    virtual OsStatus isOutOfBandDTMFEnabled(UtlBoolean& bEnabled) const;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    static int mInstanceCount;

    ConferenceEngine& mConferenceEngine;

    float mGipsVolume;
    UtlBoolean mDTMFOutOfBand;
    UtlBoolean mAGC;

    UtlSList mInterfaceList;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _ConferenceEngineFactoryImpl_h_
