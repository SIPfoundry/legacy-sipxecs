//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _CpMediaInterfaceFactoryImpl_h_
#define _CpMediaInterfaceFactoryImpl_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "utl/UtlDefs.h"
#include "utl/UtlString.h"
#include "net/SdpCodecFactory.h"
#include "utl/UtlSList.h"
#include "os/OsMutex.h"


// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class CpMediaInterface ;
class SdpCodec ;

/**
 *
 */
class CpMediaInterfaceFactoryImpl
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   /**
    * Default constructor
    */
   CpMediaInterfaceFactoryImpl();

/* =========================== DESTRUCTORS ================================ */

protected:
   /**
    * Destructor
    */
   virtual ~CpMediaInterfaceFactoryImpl();
public:

   /**
    * public interface for destroying this media interface
    */
   virtual void release();

/* ============================ MANIPULATORS ============================== */

    /**
     * Create a media interface given the designated parameters.
     */
    virtual CpMediaInterface* createMediaInterface( const char* publicAddress,
                                                    const char* localAddress,
                                                    int numCodecs,
                                                    SdpCodec* sdpCodecArray[],
                                                    const char* locale,
                                                    int expeditedIpTos,
                                                    const char* szStunServer,
                                                    int stunOptions,
                                                    int iStunKeepAliveSecs
                                                  ) = 0 ;


    /**
     * Set the speaker volume.  Valid range includes 0 to 50.
     */
    virtual OsStatus setSpeakerVolume(int iVolume) = 0 ;

    /**
     * Set the speaker device.
     */
    virtual OsStatus setSpeakerDevice(const UtlString& device) = 0 ;

    /**
     * Set the microphone gain.  Valid range includes 0 to 10.
     */
    virtual OsStatus setMicrophoneGain(int iGain) = 0 ;

    /**
     * Set the Microphone device
     */
    virtual OsStatus setMicrophoneDevice(const UtlString& device) = 0 ;


    /**
     * Mute the microphone
     */
    virtual OsStatus muteMicrophone(UtlBoolean bMute) = 0 ;

    /**
     * Enable/Disable echo cancellation
     */
    virtual OsStatus enableAudioAEC(UtlBoolean enable) = 0;

    /**
     * Enable/Disable sending DTMF tones inband
     */
    virtual OsStatus enableOutOfBandDTMF(UtlBoolean enable) = 0;

    /**
     * Populate the codec factory with the codecs provided by this
     * media subsystem., Return number of rejected codecs.
     */
    virtual OsStatus buildCodecFactory(SdpCodecFactory *pFactory,
                                       const UtlString& sAudioPreferences,
                                       const UtlString& sVideoPreferences,
                                       int* iRejected) = 0;

    /**
     * Set the global video preview window
     */
    virtual OsStatus setVideoPreviewDisplay(void* pDisplay) = 0 ;

    /**
     * Set the global video quality
     */
    virtual OsStatus setVideoQuality(int quality) = 0 ;

    /**
     * Set the global video parameters
     */
    virtual OsStatus setVideoParameters(int bitRate, int frameRate) = 0 ;

    /**
     * Update the video preview window given the specified display context.
     */
    virtual OsStatus updateVideoPreviewWindow(void* displayContext) = 0 ;

    /**
     * Sets the RTP port range for this factory
     */
    virtual void setRtpPortRange(int startRtpPort, int lastRtpPort) ;

    /**
     * Gets the next available rtp port
     */
    virtual OsStatus getNextRtpPort(int &rtpPort) ;

    /**
     * Release the rtp port back to the pool of available RTP ports
     */
    virtual OsStatus releaseRtpPort(const int rtpPort) ;

/* ============================ ACCESSORS ================================= */

    /**
     * Get the speaker volume
     */
    virtual OsStatus getSpeakerVolume(int& iVolume) const = 0 ;

    /**
     * Get the speaker device
     */
    virtual OsStatus getSpeakerDevice(UtlString& device) const = 0 ;

    /**
     * Get the microphone gain
     */
    virtual OsStatus getMicrophoneGain(int& iVolume) const = 0 ;

    /**
     * Get the microphone device
     */
    virtual OsStatus getMicrophoneDevice(UtlString& device) const = 0 ;

    /*
     * Get specific codec identified by iCodec
     */
    virtual OsStatus getCodecNameByType(SdpCodec::SdpCodecTypes codecType, UtlString& codecName) const = 0;

    /*
     * Get video quality
     */
    virtual OsStatus getVideoQuality(int& quality) const = 0;

    /*
     * Get video bit rate
     */
    virtual OsStatus getVideoBitRate(int& bitRate) const = 0;\

    /*
     * Get video frame rate
     */
    virtual OsStatus getVideoFrameRate(int& frameRate) const = 0;


/* ============================ INQUIRY =================================== */

    /**
     * Return status of echo cancellation
     */
    virtual OsStatus isAudioAECEnabled(UtlBoolean& enabled) const = 0;

    /**
     * Return status of inband DTMF
     */
    virtual OsStatus isOutOfBandDTMFEnabled(UtlBoolean& enabled) const = 0;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:
    int      miGain ;          /**< Gain value stored for unmuting */
    int      miStartRtpPort ;  /**< Requested starting rtp port */
    int      miLastRtpPort ;   /**< Requested ending rtp port */
    int      miNextRtpPort ;   /**< Next available rtp port */
    UtlSList mlistFreePorts ;  /**< List of recently freed ports */
    OsMutex  mlockList ;       /**< Lock for port allocation */



/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

    /**
     * Disabled copy constructor
     */
    CpMediaInterfaceFactoryImpl(const CpMediaInterfaceFactoryImpl&
            rCpMediaInterfaceFactoryImpl);

   /**
    * Disabled equals operator
    */
   CpMediaInterfaceFactoryImpl& operator=(
            const CpMediaInterfaceFactoryImpl& rhs);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _CpMediaInterfaceFactoryImpl_h_
