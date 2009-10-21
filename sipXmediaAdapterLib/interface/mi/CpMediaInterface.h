//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _CpMediaInterface_h_
#define _CpMediaInterface_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <os/OsStatus.h>
#include <os/OsDefs.h>
#include <os/OsProtectEvent.h>
#include <os/OsMsgQ.h>
#include <net/SipContactDb.h>
#include <net/SdpBody.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SdpCodec;
class SdpCodecFactory;
class MpStreamPlaylistPlayer;
class MpStreamPlayer;
class MpStreamQueuePlayer;
class CpMediaInterfaceFactoryImpl ;

/**
 * Abstract media control interface.
 *
 * The CpCallManager creates a CpMediaInterface for each
 * call created.  The media inteface is then used to control
 * and query the media sub-system used for that call.  As
 * connections are added to the call the media inteface is
 * used to add those connections to the media control system
 * such that all connections in that call are bridged together.
 * \par
 * This abstract class must be sub-classed and implemented to
 * replace the default media sub-system.
 */
class CpMediaInterface
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   /**
    * Default constructor
    */
   CpMediaInterface(CpMediaInterfaceFactoryImpl *pFactoryImpl);

/* =========================== DESTRUCTORS ================================ */

  protected:
   /**
    * Destructor - protected so that we can manage media interface pointers
    */
   virtual ~CpMediaInterface();
  public:

   /**
    * public interface for destroying this media interface
    */
   virtual void release() = 0;

/* ============================ MANIPULATORS ============================== */

   /**
    * Create a media connection in the media processing subsystem.  One
    * instance of the CpMediaInterface exists for each call, however, each
    * leg of the call requires an individual connection.
    *
    * @param connectionId A newly allocated connection id returned via this
    *        call.  The connection passed to many other media processing
    *        methods in this interface.
    * @param szLocalConnection Local address (interface) that should be used
    *        for this connection.
    * @param videoWindowHandle Video Window handle if using video.  Supply
    *        a window handle of NULL to disable video for this call/
    *        connection.
    */
   virtual OsStatus createConnection(int& connectionId,
                                     const char* szLocalAddress,
                                     void* videoWindowHandle
                                     ) = 0 ;

   /**
    * Get the port, address, and codec capabilities for the specified media
    * connection.  The CpMediaInterface implementation is responsible for
    * managing port allocations.
    *
    * @param connectionId Connection Id for the call leg obtained from
    *        createConnection
    * @param rtpHostAddress IP address or hostname that should be advertised
    *        in SDP data.
    * @param rtpPort RTP port number that should be advertised in SDP.
    * @param rtcpPort RTCP port number that should be advertised in SDP.
    * @param supportedCodecs List of supported codecs.
    */
   virtual OsStatus getCapabilities(int connectionId,
                                    UtlString& rtpHostAddress,
                                    int& rtpAudioPort,
                                    int& rtcpAudioPort,
                                    int& rtpVideoPort,
                                    int& rtcpVideoPort,
                                    SdpCodecFactory& supportedCodecs,
                                    SdpSrtpParameters& srtpParams) = 0;

    /** VIDEO: Include additional RTP ports */



   /**
    * Set the connection destination (target) for the designated media
    * connection.
    *
    * @param connectionId Connection Id for the call leg obtained from
    *        createConnection
    * @param rtpHostAddress IP (or host) address of remote party.
    * @param rtpPort RTP port of remote party
    * @param rtcpPort RTCP port of remote party
    */
   virtual OsStatus setConnectionDestination(int connectionId,
                                             const char* rtpHostAddress,
                                             int rtpAudioPort,
                                             int rtcpAudioPort,
                                             int rtpVideoPort,
                                             int rtcpVideoPort) = 0 ;

   /** VIDEO: include payload type */

   /**
    * Add an alternate connection destination for this media interface.
    * Alternerates are generally obtained from the SdpBody in the form
    * of candidate addresses.  When adding an alternate connection, the
    * implementation should use an ICE-like method to determine the
    * best destination address.
    *
    * @param connectionId Connection Id for the call leg obtained from
    *        createConnection
    * @param cPriority Relatively priority of the destination.  Higher
    *        numbers have greater priority (0..255)
    * @param rtpHostAddress alternate destination address
    * @param port Alternative destination port
    * @param bRtp Boolean that indicates whether this is for the rtp
    *        stream (true) or the rtcp stream (false).
    */
   virtual OsStatus addAlternateDestinations(int connectionId,
                                             unsigned char cPriority,
                                             const char* rtpHostAddress,
                                             int port,
                                             bool bRtp) = 0 ;

   /**
    * Start sending RTP using the specified codec list.  Generally, this
    * codec list is the intersection between both parties.
    *
    * @param connectionId Connection Id for the call leg obtained from
    *        createConnection
    * @param numCodec Number of codecs supplied in the sendCodec array
    * @param sendCodec Array of codecs ordered in sending preference.
    */
   virtual OsStatus startRtpSend(int connectionId,
                                 int numCodecs,
                                 SdpCodec* sendCodec[],
                                 SdpSrtpParameters& srtpParams) = 0 ;

   /**
    * Start receiving RTP using the specified codec list.  Generally, this
    * codec list is the intersection between both parties.  The media
    * processing subsystem should be prepared to receive any of the specified
    * payload type without additional signaling.  For example, it is perfectly
    * legal to switch between codecs on a whim if multiple codecs are agreed
    * upon.
    *
    * @param connectionId Connection Id for the call leg obtained from
    *        createConnection
    * @param numCodec Number of codecs supplied in the sendCodec array
    * @param sendCodec Array of receive codecs
    */
   virtual OsStatus startRtpReceive(int connectionId,
                                    int numCodecs,
                                    SdpCodec* sendCodec[],
                                    SdpSrtpParameters& srtpParams)  = 0 ;


   /**
    * Stop sending RTP (and RTCP) data for the specified connection
    *
    * @param connectionId Connection Id for the call leg obtained from
    *        createConnection
    */
   virtual OsStatus stopRtpSend(int connectionId) = 0 ;

   /**
    * Stop receiving RTP (and RTCP) data for the specified connection
    *
    * @param connectionId Connection Id for the call leg obtained from
    *        createConnection
    */
   virtual OsStatus stopRtpReceive(int connectionId) = 0 ;

   /**
    * Delete the specified connection and free up any resources associated
    * with that connection.
    *
    * @param connectionId Connection Id for the call leg obtained from
    *        createConnection
    */
   virtual OsStatus deleteConnection(int connectionId) = 0 ;

   /**
    * Start playing the specified tone for this call.  If the tone is a DTMF
    * tone and the remote flag is set, the interface should send out of
    * band DTMF using RFC 2833.  Inband audio should be sent to all
    * connections.  If a previous tone was playing, calling startTone should
    * automatically stop existing tone.
    *
    * @param toneId The designated tone to play (TODO: make enum)
    * @param local True indicates that sound should be played to the local
    *        speaker (assuming call is in focus).
    * @param remote True indicates that the sound should be played to all
    *        remote parties.
    */
   virtual OsStatus startTone(int toneId,
                              UtlBoolean local,
                              UtlBoolean remote) = 0 ;

   /**
    * Stop playing all tones.  Some tones/implementations may not support this.
    * For example, some DTMF playing implementations will only play DTMF for a
    * fixed interval.
    */
   virtual OsStatus stopTone() = 0 ;

   /**
    * Play the specified audio URL to the call.
    *
    * @param url Audio url to be played -- The sipX implementation is limited
    *        to file paths (not file Urls).
    * @param repeat If set, loop the audio file until stopAudio is called.
    * @param local True indicates that sound should be played to the local
    *        speaker (assuming call is in focus).
    * @param remote True indicates that the sound should be played to all
    *        remote parties.
    *
    */
   virtual OsStatus playAudio(const char* url,
                              UtlBoolean repeat,
                              UtlBoolean local,
                              UtlBoolean remote) = 0 ;


   /**
    * Play the specified audio buffer to the call.
    *
    * @TODO This method should also specify the audio format (e.g. samples/per
    *       second, etc.).
    */
   virtual OsStatus playBuffer(char* buf,
                               unsigned long bufSize,
                               int type,
                               UtlBoolean repeat,
                               UtlBoolean local,
                               UtlBoolean remote,
                               OsProtectedEvent* event = NULL) = 0;

   /**
    * Stop playing any URLs or buffers
    */
   virtual OsStatus stopAudio()  = 0 ;


   /**
    * Give the focus of the local audio device to the associated call
    * (for example, take this call off hold).
    */
   virtual OsStatus giveFocus() = 0 ;

   /**
    * Take this call out of focus for the local audio device
    *(for example, put this call on hold).
    */
   virtual OsStatus defocus() = 0 ;


   //! Create a simple player for this call to play a single stream
   /*! (see the CpCallManager createPlayer() method).
    */
   virtual OsStatus createPlayer(MpStreamPlayer** ppPlayer,
                                 const char* szStream,
                                 int flags,
                                 OsMsgQ *pMsgQ = NULL,
                                 const char* szTarget = NULL) = 0;

   //! Destroy a simple player in this call.
   virtual OsStatus destroyPlayer(MpStreamPlayer* pPlayer) = 0;

   //! Create a single-buffered play list player for this call
   /*! (see the CpCallManager createPlayer() method).
    */
   virtual OsStatus createPlaylistPlayer(MpStreamPlaylistPlayer** ppPlayer,
                                         OsMsgQ *pMsgQ = NULL,
                                         const char* szTarget = NULL) = 0;

   //! Destroy a single-buffered play list player in this call.
   virtual OsStatus destroyPlaylistPlayer(MpStreamPlaylistPlayer* pPlayer) = 0;

   //! Create a double-buffered list player for this call
   /*! (see the Call Manager's createPlayer() method).
    */
   virtual OsStatus createQueuePlayer(MpStreamQueuePlayer** ppPlayer,
                                      OsMsgQ *pMsgQ = NULL,
                                      const char* szTarget = NULL) = 0;

   //! Destroy a double-buffered list player in this call.
   virtual OsStatus destroyQueuePlayer(MpStreamQueuePlayer* pPlayer) = 0;

   //! Set the CPU resource limit for the media connections in this call.
   /*! This is used to limit the available codecs to only those within
    * the designated CPU cost limit.
    */
   virtual void setCodecCPULimit(int iLimit) = 0 ;

   //! Add a listener event to this call that will receive callback
   //! or queued event notifications upon receipt of DTMF tone events
   //! (RFC 2833).
   virtual void addToneListener(OsNotification *pListener, int connectionId) = 0;

   //! Remove the specified DTMF listener from this call.
   virtual void removeToneListener(int connectionId) = 0;

   //! Start recording audio for this call.
   virtual OsStatus ezRecord(int ms,
                             int silenceLength,
                             const char* fileName,
                             double& duration,
                             int& dtmfterm,
                             OsProtectedEvent* ev = NULL) = 0;

   //! Stop recording for this call.
   virtual OsStatus stopRecording() = 0;

   //! Set the preferred contact type for this media connection
   virtual void setContactType(int connectionId, ContactType eType) = 0 ;

/* ============================ ACCESSORS ================================= */

   //! For internal use only
   virtual void setPremiumSound(UtlBoolean enabled) = 0;

   //! Calculate the current cost for the current set of
   //! sending/receiving codecs.
   virtual int getCodecCPUCost() = 0 ;

   //! Calculate the worst case cost for the current set of
   //! sending/receiving codecs.
   virtual int getCodecCPULimit() = 0 ;

   //!Returns the flowgraph's message queue
   virtual OsMsgQ* getMsgQ() = 0 ;

   // Returns the primary codec for the connection
   virtual OsStatus getPrimaryCodec(int connectionId,
                                    UtlString& audioCodec,
                                    UtlString& videoCodec,
                                    int* audiopPayloadType,
                                    int* videoPayloadType) = 0;

   virtual OsStatus setVideoWindowDisplay(const void* hWnd) = 0;
   virtual const void* getVideoWindowDisplay() = 0;

/* ============================ INQUIRY =================================== */

   //! Query whether the specified media connection is enabled for
   //! sending RTP.
   virtual UtlBoolean isSendingRtpAudio(int connectionId) = 0 ;

   //! Query whether the specified media connection is enabled for
   //! sending RTP.
   virtual UtlBoolean isSendingRtpVideo(int connectionId) = 0 ;

   //! Query whether the specified media connection is enabled for
   //! sending RTP.
   virtual UtlBoolean isReceivingRtpAudio(int connectionId) = 0 ;

   //! Query whether the specified media connection is enabled for
   //! sending RTP.
   virtual UtlBoolean isReceivingRtpVideo(int connectionId) = 0 ;

   //! Query whether the specified media connection has a destination
   //! specified for sending RTP.
   virtual UtlBoolean isDestinationSet(int connectionId) = 0 ;

   //! Query whether a new party can be added to this media interfaces
   virtual UtlBoolean canAddParty() = 0 ;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    CpMediaInterfaceFactoryImpl *mpFactoryImpl ;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   //! Assignment operator disabled
   CpMediaInterface& operator=(const CpMediaInterface& rhs);

   //! Copy constructor disabled
   CpMediaInterface(const CpMediaInterface& rCpMediaInterface);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _CpMediaInterface_h_
