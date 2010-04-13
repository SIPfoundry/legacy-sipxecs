// $Id$
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _sipXmediaFactoryImpl_h_
#define _sipXmediaFactoryImpl_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mi/CpMediaInterfaceFactoryImpl.h"
#include <rtcp/RtcpConfig.h>

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
class MpMediaTask ;
class OsConfigDb ;
#ifdef INCLUDE_RTCP /* [ */
struct IRTCPControl ;
#endif /* INCLUDE_RTCP ] */


/**
 *
 */
class sipXmediaFactoryImpl : public CpMediaInterfaceFactoryImpl
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

/* ============================ CREATORS ================================== */

   /**
    * Default constructor
    */
   sipXmediaFactoryImpl(OsConfigDb* pConfigDb);


   /**
    * Destructor
    */
   virtual ~sipXmediaFactoryImpl();

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
                                       const UtlString& sPreferences,
                                       const UtlString& sVideoPreferences,
                                       int* iRejected);

    virtual OsStatus updateVideoPreviewWindow(void* displayContext) ;

    /**
     * Set the global video preview window
     */
    virtual OsStatus setVideoPreviewDisplay(void* pDisplay);

    virtual OsStatus setVideoQuality(int quality);
    virtual OsStatus setVideoParameters(int bitRate, int frameRate);


/* ============================ ACCESSORS ================================= */

    virtual OsStatus getSpeakerVolume(int& iVolume) const  ;
    virtual OsStatus getSpeakerDevice(UtlString& device) const ;
    virtual OsStatus getMicrophoneGain(int& iVolume) const ;
    virtual OsStatus getMicrophoneDevice(UtlString& device) const ;

    virtual OsStatus getNumOfCodecs(int& iCodecs) const;
    virtual OsStatus getCodec(int iCodec, UtlString& codec, int& bandWidth) const;

    virtual OsStatus getCodecNameByType(SdpCodec::SdpCodecTypes codecType, UtlString& codecName) const;

    virtual OsStatus getVideoQuality(int& quality) const;
    virtual OsStatus getVideoBitRate(int& bitRate) const;
    virtual OsStatus getVideoFrameRate(int& frameRate) const;

/* ============================ INQUIRY =================================== */

    virtual OsStatus isAudioAECEnabled(UtlBoolean& bEnabled) const;
    virtual OsStatus isOutOfBandDTMFEnabled(UtlBoolean& bEnabled) const;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:
    MpMediaTask*    mpMediaTask ;     /**< Media task instance */
#ifdef INCLUDE_RTCP /* [ */
    IRTCPControl*   mpiRTCPControl;   /**< Realtime Control Interface */
#endif /* INCLUDE_RTCP ] */


/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:
    static int miInstanceCount;

};

/* ============================ INLINE METHODS ============================ */

#endif  // _sipXmediaFactoryImpl_h_
