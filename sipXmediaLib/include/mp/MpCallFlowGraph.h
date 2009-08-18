//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _MpCallFlowGraph_h_
#define _MpCallFlowGraph_h_

#include "rtcp/RtcpConfig.h"

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "mp/MpMisc.h"
#include "mp/MpFlowGraphBase.h"
#include "mp/MpConnection.h"
#include "mp/StreamDefs.h"
#include "mp/MpStreamMsg.h"
#include "os/OsProtectEvent.h"
#include "mp/MprRecorder.h"
#include "net/Url.h"
#ifdef INCLUDE_RTCP /* [ */
#include "rtcp/RTCManager.h"
#endif /* INCLUDE_RTCP ] */

// DEFINES
#define DEBUG_POSTPONE
#undef DEBUG_POSTPONE

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
class MprBridge;
class MprFromStream;
class MprFromFile;
class MprFromMic;
class MprEchoSuppress;
class MprMixer;
class MprSplitter;
class MprToSpkr;
class MprToneGen;

//:Flow graph used to handle a basic call
#ifdef INCLUDE_RTCP /* [ */
class MpCallFlowGraph : public MpFlowGraphBase,
                        public CBaseClass,
                        public IRTCPNotify
#else /* INCLUDE_RTCP ] [ */
class MpCallFlowGraph : public MpFlowGraphBase
#endif /* INCLUDE_RTCP ] */
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum ToneOptions
   {
      TONE_TO_SPKR = 0x1,   // Play locally
      TONE_TO_NET  = 0x2    // mix the tone to play out the network connection
   };

   enum RecorderChoice {
      RECORDER_MIC = 0,
      RECORDER_MIC32K,
      RECORDER_ECHO_OUT,
      RECORDER_SPKR,
      RECORDER_SPKR32K,
      RECORDER_ECHO_IN8,
      RECORDER_ECHO_IN32,
      MAX_RECORDERS = 10
   };

/* ============================ CREATORS ================================== */

   MpCallFlowGraph(const char* pLocale = "",
                   int samplesPerFrame=DEF_SAMPLES_PER_FRAME,
                   int samplesPerSec=DEF_SAMPLES_PER_SEC);
     //:Default constructor

   virtual
   ~MpCallFlowGraph();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

void startTone(int toneId, int toneOptions);
  //:Starts playing the indicated tone.

void stopTone(void);
  //:Stops playing the tone (applies to all tone destinations).

int closeRecorders(void);

OsStatus Record(int ms,
      const char* playFilename, //if NULL, defaults to previous string
      const char* baseName,     //if NULL, defaults to previous string
      const char* endName,      //if NULL, defaults to previous string
      int recorderMask);

OsStatus ezRecord(int ms,
                   int silenceLength,
                   const char* fileName,
                   double& duration,
                   int& dtmfTerm,
                   MprRecorder::RecordFileFormat format = MprRecorder::RAW_PCM_16);


OsStatus mediaRecord(int ms,
                   int silenceLength,
                   const char* fileName,
                   double& duration,
                   int& dtmfTerm,
                   MprRecorder::RecordFileFormat format = MprRecorder::RAW_PCM_16,
                   OsProtectedEvent* recordEvent = NULL);


OsStatus record(int timeMS, int silenceLength, const char* micName = NULL,
   const char* echoOutName = NULL, const char* spkrName = NULL,
   const char* mic32Name = NULL, const char* spkr32Name = NULL,
   const char* echoIn8Name = NULL, const char* echoIn32Name = NULL,
   const char* playName = NULL,
   int toneOptions = 0,
   int repeat = 0, OsNotification* completion = NULL,
   MprRecorder::RecordFileFormat format = MprRecorder::RAW_PCM_16);


OsStatus startRecording(const char* audioFileName, UtlBoolean repeat,
                    int toneOptions, OsNotification* completion = NULL);

UtlBoolean setupRecorder(RecorderChoice which, const char* audioFileName,
   int timeMS, int silenceLength, OsNotification* event = NULL,
   MprRecorder::RecordFileFormat format = MprRecorder::RAW_PCM_16);

OsStatus playBuffer(char* audioBuf,
                   unsigned long bufSize,
                   int type,
                   UtlBoolean repeat,
                   int toneOptions,
                   OsProtectedEvent* event = NULL);
OsStatus playFile(const char* audioFileName, UtlBoolean repeat,
                    int toneOptions, OsNotification* completion = NULL);
  //: Start playing audio from a file
  //! param: audioFileName - name of the audio file
  //! param: repeat - TRUE/FALSE continue playing audio from the beginning
  //!        after the end of file is reached.
  //! param: toneOptions - TONE_TO_SPKR/TONE_TO_NET file audio played locally
  //!        or both locally and remotely.
  //! retcode: OS_INVALID_ARGUMENT - if open on the given file name failed.

void stopFile(UtlBoolean closeFile);
  //: Stop playing audio from a file
  //! param: closeFile - TRUE/FALSE whether to close the audio file.

void startSendRtp(OsSocket& rRtpSocket, OsSocket& rRtcpSocket,
          MpConnectionID connID=1, SdpCodec* pPrimaryCodec = NULL,
          SdpCodec* pDtmfCodec = NULL, SdpCodec* pSecondaryCodec = NULL);
  //:Starts sending RTP and RTCP packets.

void startSendRtp(SdpCodec& rPrimaryCodec,
                  OsSocket& rRtpSocket, OsSocket& rRtcpSocket,
                  MpConnectionID connID=1);
  //:Starts sending RTP and RTCP packets.

void stopSendRtp(MpConnectionID connID=1);
  //:Stops sending RTP and RTCP packets.

// void startReceiveRtp(SdpCodec& rCodec,
//                   OsSocket& rRtpSocket, OsSocket& rRtcpSocket,
//                   MpConnectionID connID=1);
void startReceiveRtp(SdpCodec* pCodecs[], int numCodecs,
                     OsSocket& rRtpSocket, OsSocket& rRtcpSocket,
                     MpConnectionID connID=1);
  //:Starts receiving RTP and RTCP packets.

void stopReceiveRtp(MpConnectionID connID=1);
  //:Stops receiving RTP and RTCP packets.

virtual OsStatus gainFocus(void);
  //:Informs the flow graph that it now has the MpMediaTask focus.
  // Only the flow graph that has the focus is permitted to access
  // the audio hardware.  This may only be called if this flow graph
  // is managed and started!
  //!retcode: OS_SUCCESS, always

virtual OsStatus loseFocus(void);
  //:Informs the flow graph that it has lost the MpMediaTask focus.
  // Only the flow graph that has the focus is permitted to access
  // the audio hardware.  This should only be called if this flow graph
  // is managed and started!
  //!retcode: OS_SUCCESS, always

MpConnectionID createConnection(void);
  //:Creates a new MpConnection; returns -1 if failure.

UtlBoolean unmuteInput(MpConnectionID connID);
  //:enables hearing audio data from a source
UtlBoolean unmuteOutput(MpConnectionID connID);
  //:enables sending audio data to a remote party
UtlBoolean muteInput(MpConnectionID connID);
  //:disables hearing audio data from a source
UtlBoolean muteOutput(MpConnectionID connID);
  //:disables sending audio data to a remote party

OsStatus deleteConnection(MpConnectionID connID);
  //:Removes an MpConnection and deletes it and all its resources.

void setPremiumSound(MpConnection::PremiumSoundOptions op);
  //:Disables or enables the GIPS premium sound.

void disablePremiumSound(void);
  //:Disables the GIPS premium sound.

void enablePremiumSound(void);
  //:Enables the GIPS premium sound.

static UtlBoolean setInbandDTMF(UtlBoolean bEnable);
  //:Enables/Disable the transmission of inband DTMF audio

static UtlBoolean setEnableAEC(UtlBoolean bEnable);
  //:Enables/Disables Acoustic Echo cancellation

OsStatus addToneListener(OsNotification* pNotify, MpConnectionID connectionId);
  //:Adds tone listener to receive the dtmf key events.

OsStatus removeToneListener(MpConnectionID connectionId);
  //:Adds tone listener to receive the dtmf key events.

/* ============================ ACCESSORS ================================= */

MpConnection* getConnectionPtr(MpConnectionID id);
#ifdef INCLUDE_RTCP /* [ */
IRTCPSession* getRTCPSessionPtr(void);
#endif /* INCLUDE_RTCP ] */

void synchronize(const char* tag=NULL, int val=0);
  //:sends a message to self, and waits for reply.

/* ============================ INQUIRY =================================== */

UtlBoolean isCodecSupported(SdpCodec& rCodec);
  //:Returns TRUE if the indicated codec is supported.

UtlBoolean isPremiumSoundEnabled(void);
  //:Returns TRUE if GIPS premium sound is currently enabled.

/* ============================ CALLBACKS ================================= */
#ifdef INCLUDE_RTCP /* [ */

/**
 *
 * Method Name:  GetEventInterest()
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     unsigned long - Mask of Event Interests
 *
 * Description: The GetEventInterest() event method shall allow the dispatcher
 *              of notifications to access the event interests of a subscriber
 *              and use these wishes to dispatch RTCP event notifications
 *
 * Usage Notes:
 *
 */
    unsigned long GetEventInterest(void);

/**
 *
 * Method Name:  LocalSSRCCollision()
 *
 *
 * Inputs:      IRTCPConnection *piRTCPConnection - Interface to
 *                                                   associated RTCP Connection
 *              IRTCPSession    *piRTCPSession    - Interface to associated
 *                                                   RTCP Session
 *
 * Outputs:     None
 *
 * Returns:     None
 *
 * Description: The LocalSSRCCollision() event method shall inform the
 *              recipient of a collision between the local SSRC and one
 *              used by one of the remote participants.
 *
 * Usage Notes:
 *
 */
    void LocalSSRCCollision(IRTCPConnection    *piRTCPConnection,
                            IRTCPSession       *piRTCPSession);


/**
 *
 * Method Name:  RemoteSSRCCollision()
 *
 *
 * Inputs:      IRTCPConnection *piRTCPConnection - Interface to associated
 *                                                    RTCP Connection
 *              IRTCPSession    *piRTCPSession    - Interface to associated
 *                                                    RTCP Session
 *
 * Outputs:     None
 *
 * Returns:     None
 *
 * Description: The RemoteSSRCCollision() event method shall inform the
 *              recipient of a collision between two remote participants.
 *              .
 *
 * Usage Notes:
 *
 */
    void RemoteSSRCCollision(IRTCPConnection    *piRTCPConnection,
                             IRTCPSession       *piRTCPSession);


/**
 *
 * Macro Name:  DECLARE_IBASE_M
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     None
 *
 * Description: This implements the IBaseClass functions used and exposed by
 *              derived classes.
 *
 * Usage Notes:
 *
 *
 */
DECLARE_IBASE_M

#endif /* INCLUDE_RTCP ] */
/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   enum PlayStart {
      START_PLAY_NONE = 0,
      START_PLAY_FILE,
      START_PLAY_SPKR
   };

   static const int DEF_SAMPLES_PER_FRAME;
   static const int DEF_SAMPLES_PER_SEC;

   static UtlBoolean sbSendInBandDTMF ;
   static UtlBoolean sbEnableAEC ;

   enum { MAX_CONNECTIONS = 10 };

   MprBridge*    mpBridge;
   MprFromFile*  mpFromFile;
   MprFromStream*  mpFromStream;
   MprFromMic*   mpFromMic;
   MprEchoSuppress*   mpEchoSuppress;
   MprMixer*     mpTFsMicMixer;
   MprMixer*     mpTFsBridgeMixer;
   MprSplitter*  mpToneFileSplitter;
   MprToSpkr*    mpToSpkr;
   MprToneGen*   mpToneGen;
   OsBSem        mConnTableLock;
   UtlBoolean     mToneIsGlobal;
   MpConnection* mpConnections[MAX_CONNECTIONS];
   UtlBoolean     mToneGenDefocused; // disabled during defocused state flag
#ifdef INCLUDE_RTCP /* [ */
   IRTCPSession* mpiRTCPSession;
   // Event Interest Attribute for RTCP Notifications
   unsigned long mulEventInterest;
#endif /* INCLUDE_RTCP ] */

   //these arrays below should really be made into a structure
   //but for now we'll just use em this way
   // D.W.
   MprRecorder* mpRecorders[MAX_RECORDERS];

   UtlBoolean mPremiumSoundEnabled;

   UtlBoolean writeWAVHeader(int handle);
   //: Write out standard 16bit 8k sampled WAV Header

   virtual UtlBoolean handleMessage(OsMsg& rMsg);
     //:Handles an incoming message for the flow graph.
     // Returns TRUE if the message was handled, otherwise FALSE.

   UtlBoolean handleRemoveConnection(MpFlowGraphMsg& rMsg);
     //:Handle the FLOWGRAPH_REMOVE_CONNECTION message.
     // Returns TRUE if the message was handled, otherwise FALSE.

   UtlBoolean handleStartPlay(MpFlowGraphMsg& rMsg);
     //:Handle the FLOWGRAPH_START_PLAY message for MprFromFile.
     // Returns TRUE if the message was handled, otherwise FALSE.

   UtlBoolean handleStartRecord(MpFlowGraphMsg& rMsg);
     //:Handle the FLOWGRAPH_START_RECORD message.
     // Returns TRUE if the message was handled, otherwise FALSE.

   UtlBoolean handleStopRecord(MpFlowGraphMsg& rMsg);
     //:Handle the FLOWGRAPH_STOP_RECORD message.
     // Returns TRUE if the message was handled, otherwise FALSE.

   UtlBoolean handleStartTone(MpFlowGraphMsg& rMsg);
     //:Handle the FLOWGRAPH_START_TONE message.
     // Returns TRUE if the message was handled, otherwise FALSE.

   UtlBoolean handleStopToneOrPlay(void);
     //:Handle the FLOWGRAPH_STOP_TONE message.
     //:Handle the FLOWGRAPH_STOP_PLAY message.
     // Returns TRUE if the message was handled, otherwise FALSE.

#ifdef DEBUG_POSTPONE /* [ */
   void postPone(int ms);
     //:sends a message requesting a delay for race condition detection...
#endif /* DEBUG_POSTPONE ] */

   UtlBoolean handleSynchronize(MpFlowGraphMsg& rMsg);
     //:Handle the FLOWGRAPH_SYNCHRONIZE message.
     // Returns TRUE if the message was handled, otherwise FALSE.

   UtlBoolean handleSetPremiumSound(MpFlowGraphMsg& rMsg);
     //:Handle the FLOWGRAPH_SET_PREMIUM_SOUND message.
     // Returns TRUE

   UtlBoolean handleSetDtmfNotify(MpFlowGraphMsg& rMsg);
     //:Handle the FLOWGRAPH_SET_DTMF_NOTIFY message.
     // Returns TRUE

   UtlBoolean handleStreamRealizeUrl(MpStreamMsg& rMsg);
     //:Handle the FLOWGRAPH_STREAM_REALIZE_URL message.
     // Returns TRUE if the message was handled, otherwise FALSE.

   UtlBoolean handleStreamRealizeBuffer(MpStreamMsg& rMsg);
     //:Handle the FLOWGRAPH_STREAM_REALIZE_BUFFER message.
     // Returns TRUE if the message was handled, otherwise FALSE.

   UtlBoolean handleStreamPrefetch(MpStreamMsg& rMsg);
     //:Handle the FLOWGRAPH_STREAM_PREFETCH message.
     // Returns TRUE if the message was handled, otherwise FALSE.

   UtlBoolean handleStreamPlay(MpStreamMsg& rMsg);
     //:Handle the FLOWGRAPH_STREAM_PLAY message.
     // Returns TRUE if the message was handled, otherwise FALSE.

   UtlBoolean handleStreamRewind(MpStreamMsg& rMsg);
     //:Handle the FLOWGRAPH_STREAM_REWIND message.
     // Returns TRUE if the message was handled, otherwise FALSE.

   UtlBoolean handleStreamPause(MpStreamMsg& rMsg);
     //:Handle the FLOWGRAPH_STREAM_PAUSE message.
     // Returns TRUE if the message was handled, otherwise FALSE.

   UtlBoolean handleStreamStop(MpStreamMsg& rMsg);
     //:Handle the FLOWGRAPH_STREAM_STOP message.
     // Returns TRUE if the message was handled, otherwise FALSE.

   UtlBoolean handleStreamDestroy(MpStreamMsg& rMsg);
     //:Handle the FLOWGRAPH_STREAM_DESTROY message.
     // Returns TRUE if the message was handled, otherwise FALSE.

   MpCallFlowGraph(const MpCallFlowGraph& rMpCallFlowGraph);
     //:Copy constructor (not implemented for this class)

   MpCallFlowGraph& operator=(const MpCallFlowGraph& rhs);
     //:Assignment operator (not implemented for this class)

};

/* ============================ INLINE METHODS ============================ */
#ifdef INCLUDE_RTCP /* [ */
/**
 *
 * Method Name: getRTCPSessionPtr
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     IRTCPSession *   - RTCP Session interface pointer associated
 *                               with this flow graph and call.
 *
 * Description: Returns the RTCP Session interface pointer associated with
 *              this call's flow graph.
 *
 *
 * Usage Notes:
 *
 *
 */
inline IRTCPSession *MpCallFlowGraph::getRTCPSessionPtr(void)
{

    return(mpiRTCPSession);

}



/**
 *
 * Method Name:  GetEventInterest()
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     unsigned long - Mask of Event Interests
 *
 * Description: The GetEventInterest() event method shall allow the dispatcher
 *              of notifications to access the event interests of a subscriber
 *              and use these wishes to dispatch RTCP event notifications
 *
 * Usage Notes:
 *
 */
inline unsigned long MpCallFlowGraph::GetEventInterest(void)
{

    return(mulEventInterest);
}
#endif /* INCLUDE_RTCP ] */

#endif  // _MpCallFlowGraph_h_
