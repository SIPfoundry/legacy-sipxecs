//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _ACDCall_h_
#define _ACDCall_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsStatus.h>
#include <tapi/sipXtapi.h>
#include <utl/UtlString.h>
#include <utl/UtlContainable.h>
#include <utl/UtlHashMap.h>
#include <os/OsMsg.h>
#include <os/OsServerTask.h>
#include "ACDCallRouteState.h"
#include "ACDAgent.h"

// DEFINES
#define MAX_NUM_TRANSFER 6
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class OsTimer;
class ACDLine;
class ACDAgent;
class ACDQueue;
class ACDCallManager;
class ACDAudioManager;

// ACDCall object is an abstraction of a SIP UA call instances.  Through
// its public interface, it is possible to create, modify, monitor and
// destroy calls via the associated ACDCallManager UA.

class ACDCall : public UtlContainable, OsServerTask {
   friend class ACDCallManager;

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   enum eCallState {
      CALL_STATE_UNDEFINED,
      OFFERING,
      ALERTING,
      CONNECTED,
      DISCONNECTED,
      DESTROYED
   };

   enum eCallTimers {
      NO_AUDIO_PLAYING        = 0,
      RING_TIMEOUT_TIMER      = 1,
      WELCOME_AUDIO_TIMER     = 2,
      QUEUE_AUDIO_TIMER       = 3,
      QUEUE_DELAY_TIMER       = 4,
      TERMINATION_AUDIO_TIMER = 5,
      QUEUE_MAX_WAIT_TIMER    = 6
   };

   static const char* eCallTimers2str[];

   // Transfer flag below is related to transfer mode
   bool mFlagCTransfer;

   bool mFlagTransfer;
   bool mFlagTransferConnect;
   bool mFlagTransferRt;
   bool mFlagTransferBlind;
   int  mNumTransfer;

/* ============================ CREATORS ================================== */

   // Default constructor
   ACDCall(ACDCallManager* pAcdCallManager, ACDLine* pLineRef, SIPX_CALL hCallHandle);

   // Pre Destructor
   void destroyACDCall();

   // Destructor
   ~ACDCall();

/* ============================ MANIPULATORS ============================== */

   void setManagingQueue(ACDQueue* pManagingQueue, int waitTime);

   void routeRequest(ACDAgent* pTargetAgent, int connectionScheme, int timeout);

   void routeRequest(UtlSList& rTargetAgentList, int connectionScheme, int timeout);

   void routeRequestAddAgent(ACDAgent* pTargetAgent);

   void answerCall(UtlString& rWelcomeAudio, bool bargeIn);

   void dropCall(int terminationToneDuration, UtlString& rTerminationAudio);

   void playAudio(UtlString& rQueueAudio, int queueAudioInterval, UtlString& rBackgroundAudio);

   void stopAudio(void);

   void updateState(SIPX_CALL callHandle, int event, int cause);

   void abortRouteRequest(void);

   void resetQueueMaxWaitTimer(void);

   void setRoutePendingAnswer(void) { mRoutePendingAnswer = true; }

   void clearRoutePendingAnswer(void) { mRoutePendingAnswer = false; }

   void setXferPendingAnswer(void) { mXferPendingAnswer = true; }

   void clearXferPendingAnswer(void) { mXferPendingAnswer = false; }

   void setTransferAgent(ACDAgent* pAgentRef) { mpTransferAgent = pAgentRef; }

   void setCallId(int callHandle);

   void setRnaState(bool rnaState) {mRnaState = rnaState;};

#ifdef CML
   void setCallIdentity(void);

   // CML: methods used for call pickup

   void doCallPickUp(ACDAgent* const pAgent);

   void clearBeingPickedUp(void);

   ACDAgent* isBeingPickedUp(bool& bRet);


#endif

/* ============================ ACCESSORS ================================= */

   SIPX_CALL getCallHandle(void);

   const char* getCallIdentity(void);

   UtlString getCallId() { return mCallId; }

   virtual unsigned hash() const;

   virtual UtlContainableType getContainableType() const;

   ACDAgent*  getAcdAgent(void);

   ACDAgent*  getTransferAcdAgent(void);

   ACDCallRouteState::eRouteState getRouteState(void);

   ACDCallManager* getAcdCallManager() { return mpAcdCallManager; } // Reference to the parent CallManager object

   bool getRnaState() { return mRnaState;}

   void       setLastAgent(ACDAgent *pAgent) { mpLastAgent = pAgent; }
   ACDAgent*  getLastAgent() { return mpLastAgent; }

   void resetRouteState(void);

   ACDQueue* getMpManagingQueue() { return mpManagingQueue; };

   SIPX_LINE getAcdAgentLineHandle() { return mhAcdAgentLineHandle; };

   ACDLine*  getAcdAgentLineReference() { return mpAcdLineReference; };

/* ============================ INQUIRY =================================== */

   bool routePendingAnswer(void) { return mRoutePendingAnswer; }

   bool getXferPendingAnswer(void) { return mXferPendingAnswer; }

   eCallState getCurrentCallState(void) { return mCallState; }

   virtual int compareTo(UtlContainable const*) const;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   static const UtlContainableType TYPE;           // Class type used for runtime checking


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   // The ACDCallRouteState State Machine classes need to be friends
   friend class ACDCallRouteState;
   friend class ACDCallRouteState_IDLE;
   friend class ACDCallRouteState_TRYING;
   friend class ACDCallRouteState_DISCOVERED;
   friend class ACDCallRouteState_CONNECTING;
   friend class ACDCallRouteState_ON_HOLD;
   friend class ACDCallRouteState_ROUTED;
   friend class ACDCallRouteState_FAILED;
   friend class ACDCallRouteState_ABORTED;
   friend class ACDCallRouteState_TERMINATED;

#ifdef CML
   void doCallPickUpMessage(ACDAgent* const pAgent);
#endif

   // Handle incomming IPC messages
   UtlBoolean handleMessage(OsMsg& rMessage);

   void setManagingQueueMessage(ACDQueue* pManagingQueue, int waitTime);

   void routeRequestMessage(UtlSList* pTargetAgentList, int connectionScheme, int timeout);

   void routeRequestAddAgentMessage(ACDAgent* pTargetAgent);

   void answerCallMessage(UtlString* pWelcomeAudio, bool bargeIn);

   void dropCallMessage(int terminationToneDuration, UtlString* pTerminationAudio);

   void playAudioMessage(UtlString* pQueueAudio, int queueAudioInterval, UtlString* pBackgroundAudio);

   void stopAudioMessage(void);

   void updateStateMessage(SIPX_CALL callHandle, int event, int cause);

   void abortRouteRequestMessage(void);

   void acdCallConnectedEvent(int cause);

   void acdCallDisconnectedEvent(void);

   void acdAgentConnectedActiveEvent(SIPX_CALL callHandle);

   void acdTransferAgentConnectedEvent(SIPX_CALL callHandle);

   void acdAgentConnectedInactiveEvent(SIPX_CALL callHandle);

   void acdAgentDisconnectedEvent(SIPX_CALL callHandle);

   void acdTransferAgentDisconnectedEvent(SIPX_CALL callHandle);

   void acdCallTransferModeFailure(void);

   void routeRequestTimeoutEvent(void);

   void routeRequestAbortEvent(void);

   void transitionRouteState(ACDCallRouteState::eRouteState newState);

   OsMutex          mLock;                       // Lock used for atomic access
   SIPX_CALL        mhCallHandle;                // The sipXtapi handle for this call
   SIPX_CALL        mhAssociatedCallHandle;      // The sipXtapi handle for this call
   SIPX_INST        mhAcdCallManagerHandle;      // The sipXtapi handle for the UA
   ACDCallManager*  mpAcdCallManager;            // Reference to the parent CallManager object
   ACDAudioManager* mpAcdAudioManager;           // Reference to the peer AudioManager object
   ACDLine*         mpAcdLineReference;          // Reference to the ACDLine that this call resides on
   UtlString        mCallId;                     // The internal call ID of the associated call
   char*            mpCallIdentity;              // The identity of the associated call
   eCallState       mCallState;                  // The state of the associated call
   ACDCallRouteState::eRouteState mRouteState;   // The state of the call routing process
   ACDCallRouteState* mpRouteStateMachine;
   ACDQueue*        mpManagingQueue;             // The current ACDQueue that is managing this call
   OsTimer*         mpQueueMaxWaitTimer;         // The queue max-wait-time timer
   UtlSList         mAgentCandidateList;         // List of possible ACDAgents to route this call to
   ACDAgent*        mpActiveAgent;               // The ACDAgent that this call was routed to
   ACDAgent*        mpTransferAgent;             // The ACDAgent that this call is going to be transferred to
   SIPX_LINE        mhAcdAgentLineHandle;        // The sipXtapi line handle used for outbound call routing
   SIPX_CONF        mhConferenceHandle;          // The sipXtapi conference handle for this call
   int              mConnectionScheme;           // The call route connection scheme that should be employed
   int              mRingNoAnswerTime;           // The maximum amount of time to allow before giving up on an agent call
   OsTimer*         mpRingTimeoutTimer;          // The call ring timeout timer
   UtlString        mWelcomeAudio;               // The optional audio to play when first answering this call
   UtlString        mQueueAudio;                 // The optional audio to play repeatedly to the call
   UtlString        mBackgroundAudio;            // The optional audio to play repeatedly to the call
   OsTimer*         mpWelcomeAudioPlayTimer;     // The timer used to time the completion of the welcome audio
   OsTimer*         mpQueueAudioPlayTimer;       // The timer used to time the completion of the queue audio
   OsTimer*         mpQueueAudioDelayTimer;      // The timer used to time the delay between repeating queue audio
   OsTimer*         mpTerminationAudioPlayTimer; // The timer used to time the completion of the termination audio
   bool             mRoutePendingAnswer;         // Flag indicating that the call is pending route
   bool             mXferPendingAnswer;          // Flag indicating that the call is pending a transfer
   bool             mBargeIn;                    // Flag indicating that the welcome message can be barged-in
   enum eCallTimers mPlayingAudio;               // Flag indicating that audio is being played to caller (and which audio)
   bool             mPlayingRingback;            // Flag indicating that ringback tone is being played to caller
   bool             mWelcomeAudioPlayed;         // Flag indicating that the Welcome Audio has previously been played
   bool             mWelcomeAudioPlaying;        // Flag indicating that the Welcome Audio is currently being played
   UtlString*       mpQueueAudio;
   int              mQueueAudioInterval;
   UtlString*       mpBackgroundAudio;
   bool             mRnaState;

#ifdef CML
   bool             mbIsBeingPickedUp;           // CML: Flag indicating that a call pickup is in progress for this call
   ACDAgent*        mpACDAgentPickedUpBy;        // CML: Call is being picked up by this Agent
#endif

   static const char *  NA_BusyTone;             // Busy Tone audio data and length.  See: BusyTone.h
   static unsigned long NA_BusyToneLength;

   static const char *  NA_RingbackTone;         // Rinbback Tone audio data and length.  See: RingbackTone.h
   static unsigned long NA_RingbackToneLength;

   static const char *  ConfirmationTone;        // Confirmation Tone audio data and length.  See: ConfirmationTone.h
   static unsigned long ConfirmationToneLength;

   static const char *  ConfirmationShortTone;   // Confirmation Tone audio data and length.  See: ConfirmationShortTone.h
   static unsigned long ConfirmationShortToneLength;

   ACDAgent*  mpLastAgent;                       // Last agent to which this call was routed
};

#endif  // _ACDCall_h_
