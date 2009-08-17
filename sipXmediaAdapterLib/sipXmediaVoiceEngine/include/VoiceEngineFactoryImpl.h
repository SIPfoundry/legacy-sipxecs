//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////


#ifndef _VoiceEngineFactoryImpl_h_
#define _VoiceEngineFactoryImpl_h_

// SYSTEM INCLUDES
#ifdef _WIN32
#include "GipsVoiceEngineLib.h"
#else
#include "GipsVoiceEngineLibLinux.h"
#endif
#ifdef VIDEO
#ifdef _WIN32
    #include <windows.h>
    #include "GipsVideoEngineWindows.h"
#endif
#endif

// APPLICATION INCLUDES
#include "mi/CpMediaInterfaceFactoryImpl.h"
#include "include/VoiceEngineMediaInterface.h"

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
class VoiceEngineMediaInterface;
#ifdef INCLUDE_RTCP /* [ */
struct IRTCPControl ;
#endif /* INCLUDE_RTCP ] */


/**
 *
 */
class VoiceEngineFactoryImpl : public CpMediaInterfaceFactoryImpl
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

/* ============================ CREATORS ================================== */

   /**
    * Default constructor
    */
   VoiceEngineFactoryImpl(OsConfigDb* pConfigDb);


   /**
    * Destructor
    */
   virtual ~VoiceEngineFactoryImpl();

/* ============================ MANIPULATORS ============================== */

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

    virtual OsStatus setSpeakerVolume(int iVolume) ;
    virtual OsStatus setSpeakerDevice(const UtlString& device) ;

    virtual OsStatus setMicrophoneGain(int iGain) ;
    virtual OsStatus setMicrophoneDevice(const UtlString& device) ;
    virtual OsStatus muteMicrophone(UtlBoolean bMute) ;

    virtual OsStatus enableAudioAEC(UtlBoolean bEnable);
    virtual OsStatus enableOutOfBandDTMF(UtlBoolean bEnable);

    virtual OsStatus buildCodecFactory(SdpCodecFactory *pFactory,
                                       const UtlString& sAudioPreferences,
                                       const UtlString& sVideoPreferences,
                                       int* iRejected);

    virtual OsStatus setVideoPreviewDisplay(void* pDisplay) ;
    virtual void* getVideoPreviewDisplay() ;
    virtual OsStatus updateVideoPreviewWindow(void* displayContext) ;
    virtual OsStatus setVideoQuality(int quality);
    virtual OsStatus setVideoParameters(int bitRate, int frameRate);

    virtual OsStatus startPlayFile(const char* szFile, bool bLoop) ;
    virtual OsStatus stopPlayFile() ;
    virtual OsStatus playTone(int toneId) ;

/* ============================ ACCESSORS ================================= */

    virtual OsStatus getSpeakerVolume(int& iVolume) const  ;
    virtual OsStatus getSpeakerDevice(UtlString& device) const ;
    virtual OsStatus getMicrophoneGain(int& iVolume) const ;
    virtual OsStatus getMicrophoneDevice(UtlString& device) const ;

    virtual OsStatus getVideoQuality(int& quality) const;
    virtual OsStatus getVideoBitRate(int& bitRate) const;
    virtual OsStatus getVideoFrameRate(int& frameRate) const;

    virtual OsStatus getCodecNameByType(SdpCodec::SdpCodecTypes codecType, UtlString& codecName) const;

    virtual GipsVoiceEngineLib* getVoiceEnginePointer() const;

#ifdef VIDEO
    virtual GipsVideoEngineWindows* getVideoEnginePointer() const ;
#endif

/* ============================ INQUIRY =================================== */

    virtual OsStatus isAudioAECEnabled(UtlBoolean& bEnabled) const;
    virtual OsStatus isOutOfBandDTMFEnabled(UtlBoolean& bEnabled) const;

    /**
     * Removes a media interface from our stored list of media interfaces
     */
    void removeInterface(VoiceEngineMediaInterface* pMediaInterface);

    const bool isMuted() const;



/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:
    mutable OsMutex mRtpPortLock;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:
    /**
     * Converts a system device string to a device id.
     *
     * @param deviceIndex Integer reference - to be filled in with the system
     *                    device id.
     * @param device The name of the system audio device.
     */
    OsStatus outputDeviceStringToIndex(int& deviceIndex, const UtlString& device) const;

    /**
     * Converts a system device id to a device string.
     *
     * @param device UtlString reference - to be filled in with the system device string.
     * @param deviceIndex The index of the system audio device.
     */
    OsStatus outputDeviceIndexToString(UtlString& device, const int deviceIndex) const;

    /**
     * Converts a system device string to a device id.
     *
     * @param deviceIndex Integer reference - to be filled in with the system
     *                    device id.
     * @param device The name of the system audio device.
     */
    OsStatus inputDeviceStringToIndex(int& deviceIndex, const UtlString& device) const;

    /**
     * Converts a system device id to a device string.
     *
     * @param device UtlString reference - to be filled in with the system device string.
     * @param deviceIndex The index of the system audio device.
     */
    OsStatus inputDeviceIndexToString(UtlString& device, const int deviceIndex) const;

    GipsVoiceEngineLib& mMediaLib;
#ifdef VIDEO
    GipsVideoEngineWindows& mVideoLib;
#endif
    int mConnectionId ;
    int mCurrentWaveInDevice;
    int mCurrentWaveOutDevice;
    int mVideoQuality;
    int mVideoBitRate;
    int mVideoFrameRate;
    UtlBoolean mbDTMFOutOfBand;
    UtlSList mInterfaceList;
    bool mbMute;
    void* mpPreviewWindowDisplay;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _VoiceEngineFactoryImpl_h_
