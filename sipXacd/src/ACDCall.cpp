//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsSysLog.h>
#include <os/OsTimer.h>
#include <os/OsEventMsg.h>
#include <os/OsQueuedEvent.h>
#include <utl/UtlSListIterator.h>
#include <tapi/sipXtapi.h>
#include <tapi/sipXtapiEvents.h>
#include <tapi/sipXtapiInternal.h>
#include "ACDCall.h"
#include "ACDLine.h"
#include "ACDAgent.h"
#include "ACDCallMsg.h"
#include "ACDCallRouteState.h"
#include "ACDAudioManager.h"
#include "ACDServer.h"
#include "ACDAgentManager.h"
#include "ACDCallManager.h"
#include "ACDRtRecord.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
extern OsSysLogPriority gACD_DEBUG;

// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
#include "ConfirmationTone.h"
#include "RingbackTone.h"
#include "BusyTone.h"
#include "ConfirmShortTone.h"

const UtlContainableType ACDCall::TYPE = "ACDCall";

const char* ACDCall::eCallTimers2str[7] = {
   "NO_AUDIO_PLAYING",
   "RING_TIMEOUT_TIMER",
   "WELCOME_AUDIO_TIMER",
   "QUEUE_AUDIO_TIMER",
   "QUEUE_DELAY_TIMER",
   "TERMINATION_AUDIO_TIMER",
   "QUEUE_MAX_WAIT_TIMER"
};



/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCall::ACDCall
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ACDCall::ACDCall(ACDCallManager* pAcdCallManager, ACDLine* pLineRef, SIPX_CALL hCallHandle)
: OsServerTask("ACDCall-%d"), mLock(OsMutex::Q_FIFO)
{
   mpAcdCallManager       = pAcdCallManager;
   mpAcdLineReference     = pLineRef;
   mhCallHandle           = hCallHandle;
   mhAcdCallManagerHandle = mpAcdCallManager->getAcdCallManagerHandle();

   mpAcdAudioManager      = mpAcdCallManager->getAcdAudioManager();

   // Initially no ACDQueue is controlling this call
   mpManagingQueue     = NULL;

   // Initially no active ACDAgent
   mpActiveAgent        = NULL;
   // Use the inbound line to make the calls on
   mhAcdAgentLineHandle = pLineRef->getLineHandle();

   // Initially no Transfer Agent
   mpTransferAgent = NULL;

   // Initially no associated call
   mhAssociatedCallHandle = SIPX_CALL_NULL;

   // Initially Transfer flag is false
   mFlagTransfer = FALSE;
   // Initially Transfer Connect flag is false
   // This flag is for preventing any execution of
   // the code if the CONNECT for transfer agent is called
   // more than once during transfer
   mFlagTransferConnect = FALSE;
   // This flag to make a real time event record
   // when a transfer call is terminated
   mFlagTransferRt = FALSE;
   // This flag is for blind transfer
   mFlagTransferBlind = FALSE;
   // Initialize max num transfer to 0
   mNumTransfer = 0;

#ifdef CML
   mbIsBeingPickedUp = false;
   mpACDAgentPickedUpBy = NULL;
#endif

   mhConferenceHandle = SIPX_CONF_NULL;

   // Get the internal call ID for this call
   sipxCallGetCommonData(mhCallHandle, NULL, &mCallId, NULL, NULL);

#ifdef CML
   // create the call identity for this call
   setCallIdentity();
#else
   // Get the call identity for this call
   setCallId(mhCallHandle);
#endif

   mCallState = ALERTING;

   mPlayingAudio        = NO_AUDIO_PLAYING;
   mPlayingRingback     = false;
   mWelcomeAudioPlayed  = false;
   mConnectionScheme    = 0;
   mRingNoAnswerTime    = 0;
   mWelcomeAudioPlaying = false;
   mRnaState = FALSE;
   mXferPendingAnswer   = false ;

   // Transfer flag related to transfer mode
   mFlagCTransfer = FALSE;

   // Start the Route FSM
   resetRouteState() ;

   mpLastAgent = NULL;

   // preCreate all the timers we will need

   mpRingTimeoutTimer =
      new OsTimer(getMessageQueue(), (void*)RING_TIMEOUT_TIMER);
   mpWelcomeAudioPlayTimer =
      new OsTimer(getMessageQueue(), (void*)WELCOME_AUDIO_TIMER);
   mpQueueAudioPlayTimer =
      new OsTimer(getMessageQueue(), (void*)QUEUE_AUDIO_TIMER);
   mpQueueAudioDelayTimer =
      new OsTimer(getMessageQueue(), (void*)QUEUE_DELAY_TIMER);
   mpTerminationAudioPlayTimer =
      new OsTimer(getMessageQueue(), (void*)TERMINATION_AUDIO_TIMER);
   mpQueueMaxWaitTimer =
      new OsTimer(getMessageQueue(), (void*)QUEUE_MAX_WAIT_TIMER);

   OsSysLog::add(FAC_ACD, gACD_DEBUG,
                 "ACDCall::ACDCall - "
                 "New Call(%d) [%s] is created.",
                 mhCallHandle, mpCallIdentity);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCall::destroyACDCall
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void
ACDCall::destroyACDCall()
{
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCall::destroyACDCall - Call(%d) [%s] is being destroyed",
                 mhCallHandle, mpCallIdentity);
   if (mpCallIdentity == NULL) {
      return ;
   }

   // Abort any routing that may still be going on
   routeRequestAbortEvent() ;

   // For safety, we will try to delete all the timers associated with the call
   // before destructing itself
   stopAudioMessage();

   // Delete all timers (which stops them, too)
   delete mpQueueMaxWaitTimer;
   delete mpTerminationAudioPlayTimer;
   delete mpQueueAudioDelayTimer;
   delete mpQueueAudioPlayTimer;
   delete mpWelcomeAudioPlayTimer;
   delete mpRingTimeoutTimer;

   // Remove the call from the associated line
   mpAcdLineReference->deleteCall(this);

   // Destroy any leftover conference
   if (mhConferenceHandle != 0)
   {
      sipxConferenceDestroy(mhConferenceHandle);
      mhConferenceHandle = 0 ;
   }

   // Destroy the call!
   if (mhCallHandle != SIPX_CALL_NULL)
   {
      sipxCallDestroy(mhCallHandle) ; // sets mhCallHandle to 0
   }

   free(mpCallIdentity);
   mpCallIdentity = NULL ;

   // Clear the Agent Candidate List
   UtlSListIterator agentListIterator(mAgentCandidateList);
   ACDAgent *pAgent;
   while ((pAgent = dynamic_cast<ACDAgent*>(agentListIterator())) != NULL) {
      mAgentCandidateList.remove(pAgent);
   }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCall::~ACDCall
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ACDCall::~ACDCall()
{
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCall::~ACDCall - ACDCall(%p)",
                 this);
}

/* ============================ MANIPULATORS ============================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCall::setManagingQueue
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCall::setManagingQueue(ACDQueue* pManagingQueue, int waitTime)
{
   ACDCallMsg setManagingQueueMsg(ACDCallMsg::SET_QUEUE, pManagingQueue, waitTime);
   postMessage(setManagingQueueMsg);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCall::routeRequest
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCall::routeRequest(ACDAgent* pTargetAgent, int connectionScheme, int timeout)
{
   UtlSList targetAgentList;

   // Add the single ACDAgent to the targetAgentList
   targetAgentList.append(pTargetAgent);

   // Create the route request message and send
   ACDCallMsg routeRequestMsg(ACDCallMsg::ROUTE_CALL, &targetAgentList, connectionScheme, timeout);
   postMessage(routeRequestMsg);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCall::routeRequest
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCall::routeRequest(UtlSList& rTargetAgentList, int connectionScheme, int timeout)
{
   ACDCallMsg routeRequestMsg(ACDCallMsg::ROUTE_CALL, &rTargetAgentList, connectionScheme, timeout);
   postMessage(routeRequestMsg);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCall::routeRequestAddAgent
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCall::routeRequestAddAgent(ACDAgent* pTargetAgent)
{
   // Create the route add message and send
   ACDCallMsg routeAddMsg(ACDCallMsg::ROUTE_ADD, pTargetAgent);
   postMessage(routeAddMsg);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCall::answerCall
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCall::answerCall(UtlString& rWelcomeAudio, bool bargeIn)
{
   ACDCallMsg answerCallMsg(ACDCallMsg::ANSWER_CALL, &rWelcomeAudio, bargeIn);
   postMessage(answerCallMsg);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCall::abortRouteRequest
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCall::abortRouteRequest(void)
{
   ACDCallMsg abortRouteMsg(ACDCallMsg::ABORT_ROUTE);
   postMessage(abortRouteMsg);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCall::dropCall
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCall::dropCall(int terminationToneDuration, UtlString& rTerminationAudio)
{
   ACDCallMsg dropCallMsg(ACDCallMsg::DROP_CALL, terminationToneDuration, &rTerminationAudio);
   postMessage(dropCallMsg);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCall::playAudio
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCall::playAudio(UtlString& rQueueAudio, int queueAudioInterval, UtlString& rBackgroundAudio)
{
   ACDCallMsg playAudioMsg(ACDCallMsg::PLAY_AUDIO, &rQueueAudio, queueAudioInterval, &rBackgroundAudio);
   postMessage(playAudioMsg);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCall::stopAudio
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCall::stopAudio(void)
{
   ACDCallMsg stopAudioMsg(ACDCallMsg::STOP_AUDIO);
   postMessage(stopAudioMsg);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCall::updateState
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCall::updateState(SIPX_CALL callHandle, int event, int cause)
{
   ACDCallMsg updateStateMsg(ACDCallMsg::UPDATE_STATE, callHandle, event, cause);
   postMessage(updateStateMsg);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCall::resetQueueMaxWaitTimer
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCall::resetQueueMaxWaitTimer(void)
{
   mpQueueMaxWaitTimer->stop(true);
}

/* ============================ ACCESSORS ================================= */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCall::getCallHandle
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

SIPX_CALL ACDCall::getCallHandle(void)
{
   return mhCallHandle;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCall::getCallIdentity
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

const char* ACDCall::getCallIdentity(void)
{
   return mpCallIdentity;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCall::hash
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned ACDCall::hash() const
{
   return mhCallHandle;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCall::getContainableType
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

UtlContainableType ACDCall::getContainableType() const
{
   return ACDCall::TYPE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCall::getAcdAgent
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ACDAgent*  ACDCall::getTransferAcdAgent()
{

   return mpTransferAgent;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCall::getAcdAgent
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ACDAgent*  ACDCall::getAcdAgent()
{

   return mpActiveAgent;
}
/* ============================ INQUIRY =================================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCall::compareTo
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

int ACDCall::compareTo(UtlContainable const* pInVal) const
{
   int result ;

   if (pInVal->isInstanceOf(ACDCall::TYPE)) {
      result = mhCallHandle - (((ACDCall*)pInVal)->getCallHandle());
   }
   else {
      result = -1;
   }

   return result;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCall::handleMessage
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

UtlBoolean ACDCall::handleMessage(OsMsg& rMessage)
{
   ACDCallMsg*    pMessage;
   SIPX_CALL      hCallHandle;
   int            event;
   int            cause;
   int            timerSource;
   void*          timerSourceVoid;
   SIPX_CALL      handleCopy;
   ACDQueue*      pManagingQueue;
   UtlString*     pWelcomeAudio;
   UtlString*     pQueueAudio;
   UtlString*     pBackgroundAudio;
   UtlString*     pTerminationAudio;
   UtlSList*      pTargetAgentList;
   int            connectionScheme;
   int            timeout;
   bool           bargeIn;
   ACDAgent*      pTargetAgent;
   char*          audioBuffer;
   unsigned long  audioLength;


//   osPrintf("ACDCall::handleMessage - MsgType: %d, MsgSubType: %d\n", rMessage.getMsgType(), rMessage.getMsgSubType());
   OsSysLog::add(FAC_ACD, gACD_DEBUG,
      "ACDCall::handleMessage - Call(%d) MsgType: %d, MsgSubType: %d",
      mhCallHandle, rMessage.getMsgType(), rMessage.getMsgSubType());


   if (rMessage.getMsgType() == OsMsg::OS_EVENT)
   {
      // Timer Event, determine which timer fired
      ((OsEventMsg&)rMessage).getUserData(timerSourceVoid);
      timerSource = (int)(intptr_t)timerSourceVoid;
      OsSysLog::add(FAC_ACD, gACD_DEBUG,
         "ACDCall::handleMessage - Call(%d) Timer %s expired",
         mhCallHandle, eCallTimers2str[timerSource]);
      switch (timerSource)
      {
         case RING_TIMEOUT_TIMER:
            routeRequestTimeoutEvent();
            return true;
            break;

         case WELCOME_AUDIO_TIMER:
            // The timer firing is only valid if the Welcome audio is still
            // playing and we haven't stopped it
            if (mPlayingAudio == WELCOME_AUDIO_TIMER)
            {
               // Stop the Welcome Audio
               sipxCallPlayBufferStop(mhCallHandle);

               // Indicate that the Welcome Audio is no longer playing
               mWelcomeAudioPlaying = false;
               mPlayingAudio = NO_AUDIO_PLAYING;

               // Only play the Welcome Audio once until this call is moved to another queue
               mWelcomeAudioPlayed = true;

               // Send the CALL_CONNECTED message to the managing queue
               mpManagingQueue->callConnected(this);
            }
            break;

         case QUEUE_MAX_WAIT_TIMER:
            // The Queue Max Wait Timer expired
            // Send the MAX_WAIT_TIME message to the managing queue
            mpManagingQueue->queueMaxWaitTime(this);
            break;

         case QUEUE_AUDIO_TIMER:
            // The Queue Audio timer expired

            // Only start up again if the audio being played was the queue
            // background audio.  Else a race condition of the timer firing,
            // and enqueing this message, but before processing it the audio
            // is stopped may occur, causing the audio to start up again
            // when it shouldn't!
            if (mPlayingAudio == QUEUE_AUDIO_TIMER)
            {
               // Stop the queue background audio
               sipxCallPlayBufferStop(mhCallHandle);
               // Start playing the audio
               // Load and play the Queue Background Audio
               if (mpAcdAudioManager->getAudio(mQueueAudio, audioBuffer, audioLength)) {
                  sipxCallPlayBufferStart(mhCallHandle, audioBuffer, audioLength, RAW_PCM_16, false, false, true);

                  // Start the Queue Audio Delay Timer
                  mpQueueAudioDelayTimer->stop(true);
                  mpQueueAudioDelayTimer->oneshotAfter(
                     OsTime(mQueueAudioInterval, 0));
               }
               else {
                  // Couldn't load, play the default confirmation tone
                  sipxCallPlayBufferStart(mhCallHandle, ConfirmationTone, ConfirmationToneLength, RAW_PCM_16, true, false, true);
               }
               mPlayingAudio = QUEUE_DELAY_TIMER ;
            }

            break;

         case QUEUE_DELAY_TIMER:
            // The Queue Audio Delay timer expired

            // Only start up again if the audio being played was the queue
            // background audio.  Else a race condition of the timer firing,
            // and enqueing this message, but before processing it the audio
            // is stopped may occur, causing the audio to start up again
            // when it shouldn't!
            if (mPlayingAudio == QUEUE_DELAY_TIMER)
            {
               OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCall::handleMessage we are going to play the audio %s again",
                                                mQueueAudio.data());

               // Stop the queue background audio
               sipxCallPlayBufferStop(mhCallHandle);
               // Load and play the Queue Background Audio
               if (mpAcdAudioManager->getAudio(mQueueAudio, audioBuffer, audioLength)) {
                  sipxCallPlayBufferStart(mhCallHandle, audioBuffer, audioLength, RAW_PCM_16, false, false, true);

                  // reStart the Queue Audio Delay Timer
                  mpQueueAudioDelayTimer->oneshotAfter(
                     OsTime(mQueueAudioInterval, 0));
               }
               else {
                  // Couldn't load, play the default confirmation tone
                  sipxCallPlayBufferStart(mhCallHandle, ConfirmationTone, ConfirmationToneLength, RAW_PCM_16, true, false, true);
               }
            }

            break;

         case TERMINATION_AUDIO_TIMER:
            // The Termination Audio Play Timer expired
            if (mPlayingAudio == TERMINATION_AUDIO_TIMER)
            {
               // Stop the audio and drop the call
               sipxCallPlayBufferStop(mhCallHandle);
               mPlayingAudio = NO_AUDIO_PLAYING ;
               handleCopy = mhCallHandle;
               if (handleCopy != SIPX_CALL_NULL)
               {
                  // sipxCalLDestroy clears it's arg.
                  sipxCallDestroy(handleCopy);
               }
            }
            break;

         default:
            // Bad message
            OsSysLog::add(FAC_ACD, PRI_ERR, "ACDCall::handleMessage - Received bad timer");
            break;
      }

      return true;
   }

   else if (rMessage.getMsgType() == OsMsg::USER_START)
   {
      switch (rMessage.getMsgSubType())
      {
         case ACDCallMsg::UPDATE_STATE:
            pMessage = (ACDCallMsg*)&rMessage;
            hCallHandle = pMessage->getCallHandle();
            event       = pMessage->getCallEvent();
            cause       = pMessage->getCallCause();
            updateStateMessage(hCallHandle, event, cause);
            OsSysLog::add(FAC_ACD, gACD_DEBUG,
                          "ACDCall::handleMessage - "
                          "ACDCallMsg::UPDATE_STATE(%d)(%d)",
                          hCallHandle, event);
            break;

         case ACDCallMsg::SET_QUEUE:
            pMessage = (ACDCallMsg*)&rMessage;
            pManagingQueue = pMessage->getRequestingQueue();
            timeout        = pMessage->getTimeout();
            setManagingQueueMessage(pManagingQueue, timeout);
            break;

         case ACDCallMsg::ANSWER_CALL:
            pMessage = (ACDCallMsg*)&rMessage;
            pWelcomeAudio = pMessage->getWelcomeAudio();
            bargeIn       = pMessage->getBargeIn();
            answerCallMessage(pWelcomeAudio, bargeIn);
            break;

         case ACDCallMsg::ABORT_ROUTE:
            pMessage = (ACDCallMsg*)&rMessage;
            abortRouteRequestMessage();
            break;

         case ACDCallMsg::DROP_CALL:
            pMessage = (ACDCallMsg*)&rMessage;
            timeout           = pMessage->getTerminationToneDuration();
            pTerminationAudio = pMessage->getTerminationAudio();
            dropCallMessage(timeout, pTerminationAudio);
            break;

         case ACDCallMsg::PLAY_AUDIO:
            pMessage = (ACDCallMsg*)&rMessage;
            pQueueAudio      = pMessage->getQueueAudio();
            timeout          = pMessage->getQueueAudioInterval();
            pBackgroundAudio = pMessage->getBackgroundAudio();
            playAudioMessage(pQueueAudio, timeout, pBackgroundAudio);
            break;

         case ACDCallMsg::STOP_AUDIO:
            pMessage = (ACDCallMsg*)&rMessage;
            stopAudioMessage();
            break;

         case ACDCallMsg::ROUTE_CALL:
            pMessage = (ACDCallMsg*)&rMessage;
            pTargetAgentList = pMessage->getTargetAgentList();
            connectionScheme = pMessage->getConnectionScheme();
            timeout          = pMessage->getTimeout();
            routeRequestMessage(pTargetAgentList, connectionScheme, timeout);
            break;

         case ACDCallMsg::ROUTE_ADD:
            pMessage = (ACDCallMsg*)&rMessage;
            pTargetAgent= pMessage->getTargetAgent();
            routeRequestAddAgentMessage(pTargetAgent);
            break;

#ifdef CML
         case ACDCallMsg::CALL_PICKUP:
            pMessage = (ACDCallMsg*)&rMessage;
            pTargetAgent= pMessage->getTargetAgent();
            doCallPickUpMessage(pTargetAgent);
            break;
#endif

         default:
            // Bad message
            OsSysLog::add(FAC_ACD, PRI_ERR, "ACDCall::handleMessage - Received bad message");
            break;
      }

      return true;
   }
   else {
      // Otherwise, pass the message to the base for processing.
      return false;
   }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCall::setManagingQueueMessage
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCall::setManagingQueueMessage(ACDQueue* pManagingQueue, int waitTime)
{
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCall::setManagingQueueMessage - Call(%s) now on ACDQueue: %s\n",
            mpCallIdentity, pManagingQueue->getUriString()->data());

   // Reset a previous max-wait-time timer
   resetQueueMaxWaitTimer();

   // Associate the ACDCall with a managing ACDQueue
   mpManagingQueue = pManagingQueue;

   if (waitTime != 0) {
      // Set the queue max-wait-time timer
      mpQueueMaxWaitTimer->stop(true);
      mpQueueMaxWaitTimer->oneshotAfter(OsTime(waitTime, 0));

      OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCall::setManagingQueueMessage - max wait timer started for Call(%s) in ACDQueue(%s) = %d",
               mpCallIdentity, pManagingQueue->getUriString()->data(), waitTime);
   }
   else {
      OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCall::setManagingQueueMessage - max wait timer is not enabled for Call(%s) in ACDQueue(%s)",
               mpCallIdentity, pManagingQueue->getUriString()->data());
   }

   // Allow the Welcome Audio to be played once
   mWelcomeAudioPlayed = false;

   // Set the connection scheme
   mConnectionScheme = mpManagingQueue->getConnectionScheme();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCall::answerCallMessage
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCall::answerCallMessage(UtlString* pWelcomeAudio, bool bargeIn)
{
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCall::answerCallMessage - Call(%d) [%s] is being answered.",
                 mhCallHandle, mpCallIdentity);

   // Remember the Welcome Audio and barge-in flag
   mWelcomeAudio = pWelcomeAudio->data();
   mBargeIn = bargeIn;

   // If the call is not yet connected, answer first before
   // playing the welcome audio.  Once the connection is
   // confirmed, the call route FSM will play the welcome audio.
   if (mCallState != CONNECTED) {
      // Answer the call
      sipxCallAnswer(mhCallHandle);
   }
   else {
      // See if the optional Welcome Audio has been specified
      // and not already played once for a given queue
      if ((mWelcomeAudio != NULL) && (mWelcomeAudioPlayed == false)) {
         // If there is audio already being played, stop it first
         if (mPlayingAudio != NO_AUDIO_PLAYING) {
            sipxCallPlayBufferStop(mhCallHandle);
         }

         // Retrieve the audio from the ACDAudioManager
         char* audioBuffer;
         unsigned long audioLength;
         if (mpAcdAudioManager->getAudio(mWelcomeAudio, audioBuffer, audioLength)) {
            // Start playing the Welcome Audio
            sipxCallPlayBufferStart(mhCallHandle,
                                    audioBuffer,
                                    audioLength,
                                    RAW_PCM_16, false, false, true);
            mPlayingAudio = WELCOME_AUDIO_TIMER;
            mWelcomeAudioPlaying = true;
            OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCall::answerCallMessage - start to play audio %s for Call(%d) %s",
                          mWelcomeAudio.data(), mhCallHandle, mpCallIdentity);

            // Set the WelcomeAudioPlay timer to fire after the Welcome Audio has completed
            // This will then send the CALL_CONNECTED message to the managing queue
            // Calculate the time required for the audio to play
            mpWelcomeAudioPlayTimer->stop(true);
            mpWelcomeAudioPlayTimer->oneshotAfter(OsTime(audioLength / 16));
         }
         else {
            // Failed to load the Welcome Audio, so send
            // the CALL_CONNECTED message to the managing queue
            mpManagingQueue->callConnected(this);
         }
      }
      else {
         // No need to wait for Welcome Audio to finish playing,
         // so send the CALL_CONNECTED message to the managing queue
         mpManagingQueue->callConnected(this);
      }
   }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCall::abortRouteRequestMessage
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCall::abortRouteRequestMessage(void)
{
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCall::abortRouteRequestMessage - Call(%d) [%s] is being aborted.",
                 mhCallHandle, mpCallIdentity);

   // Abort the Route FSM
   mpRouteStateMachine->routeRequestAbortEvent(this);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCall::dropCallMessage
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCall::dropCallMessage(int terminationToneDuration, UtlString* pTerminationAudio)
{
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCall::dropCallMessage - Call(%d) [%s] is being dropped.",
                 mhCallHandle, mpCallIdentity);

   char* audioBuffer;
   unsigned long audioLength;

   // If the call is not yet connected, ignore the request
   if (mCallState != CONNECTED) {
      // Send out the CANCEL
      SIPX_CALL handleCopy = mhCallHandle;
      if (handleCopy != SIPX_CALL_NULL)
      {
         // sipxCalLDestroy clears it's arg.
         sipxCallDestroy(handleCopy);
      }
      return;
   }

   if (terminationToneDuration == 0) {
      // No termination tone or audio, drop call immediately
      SIPX_CALL handleCopy = mhCallHandle;
      if (handleCopy != SIPX_CALL_NULL)
      {
         // sipxCalLDestroy clears it's arg.
         sipxCallDestroy(handleCopy);
      }
   }
   else {
      // Play termination tone or specified audio before dropping call

      // If there is audio already being played, stop it first
      stopAudioMessage() ;

      // See if a specific audio object has been specified
      if (pTerminationAudio != NULL) {
         // Retrieve the audio from the ACDAudioManager
         if (mpAcdAudioManager->getAudio(*pTerminationAudio, audioBuffer, audioLength)) {
            // Start playing the audio
            sipxCallPlayBufferStart(mhCallHandle, audioBuffer, audioLength, RAW_PCM_16, false, false, true);
         }
         else {
            // Couldn't load, play the default termination tone
            sipxCallPlayBufferStart(mhCallHandle, NA_BusyTone, NA_BusyToneLength, RAW_PCM_16, true, false, true);
         }
      }
      else {
         // Play the default termination tone
         sipxCallPlayBufferStart(mhCallHandle, NA_BusyTone, NA_BusyToneLength, RAW_PCM_16, true, false, true);
      }

      mPlayingAudio = TERMINATION_AUDIO_TIMER;
      // Set the TerminationAudioPlay timer to fire after the Termination Audio has completed
      // Set the timer (converting duration from seconds to milliseconds) and start it
      mpTerminationAudioPlayTimer->oneshotAfter(
         OsTime(terminationToneDuration, 0));
   }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCall::playAudioMessage
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCall::playAudioMessage(UtlString* pQueueAudio, int queueAudioInterval, UtlString* pBackgroundAudio)
{
   char* audioBuffer;
   unsigned long audioLength;

   // If the call is not yet connected, ignore the request
   if (mCallState != CONNECTED) {
      // Error
   }
   else {
      mQueueAudio         = pQueueAudio->data();
      mQueueAudioInterval = queueAudioInterval;
      mBackgroundAudio    = pBackgroundAudio->data();

      // If there is audio already being played, stop it first
      stopAudioMessage();

      // See if a specific queue audio object has been specified
      if (pQueueAudio != NULL) {
         // Retrieve the audio from the ACDAudioManager
         if (mpAcdAudioManager->getAudio(*pQueueAudio, audioBuffer, audioLength)) {
            // Set the Queue Audio Play Timer
            mpQueueAudioPlayTimer->oneshotAfter(OsTime(mQueueAudioInterval, 0));

            // Start playing the audio
            sipxCallPlayBufferStart(mhCallHandle, audioBuffer, audioLength, RAW_PCM_16, false, false, true);
         }
         else {
            // Could not load the Queue Audio, play just the Background Audio
            if (mpAcdAudioManager->getAudio(*pBackgroundAudio, audioBuffer, audioLength)) {
               sipxCallPlayBufferStart(mhCallHandle, audioBuffer, audioLength, RAW_PCM_16, true, false, true);
            }
            else {
               // Couldn't load, play the default confirmation tone
               sipxCallPlayBufferStart(mhCallHandle, ConfirmationTone, ConfirmationToneLength, RAW_PCM_16, true, false, true);
            }
         }
      }
      else {
         // Play just the Background Audio
         if (mpAcdAudioManager->getAudio(*pBackgroundAudio, audioBuffer, audioLength)) {
            sipxCallPlayBufferStart(mhCallHandle, audioBuffer, audioLength, RAW_PCM_16, true, false, true);
         }
         else {
            // Couldn't load, play the default confirmation tone
            sipxCallPlayBufferStart(mhCallHandle, ConfirmationTone, ConfirmationToneLength, RAW_PCM_16, true, false, true);
         }
      }

      mPlayingAudio = QUEUE_AUDIO_TIMER;
   }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCall::stopAudioMessage
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCall::stopAudioMessage(void)
{
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCall::stopAudioMessage - stop all the audio timers for Call(%d)",
                 mhCallHandle);

   sipxCallPlayBufferStop(mhCallHandle);
   mPlayingAudio = NO_AUDIO_PLAYING;
   mWelcomeAudioPlaying = false ;
   mPlayingRingback = false;

   mpWelcomeAudioPlayTimer->stop(true);
   mpQueueAudioPlayTimer->stop(true);
   mpQueueAudioDelayTimer->stop(true);
   mpTerminationAudioPlayTimer->stop(true);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCall::routeRequestMessage
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCall::routeRequestMessage(UtlSList* pTargetAgentList, int connectionScheme, int timeout)
{
   OsSysLog::add(FAC_ACD, gACD_DEBUG,
                 "ACDCall::routeRequestMessage - "
                 "Call(%d) [%s] is being routed.",
                 mhCallHandle, mpCallIdentity);

   // Check for Welcome Audio and barge-in status
   if (mWelcomeAudioPlaying && (mBargeIn == true))
   {
      stopAudioMessage();
   }

   // First clear the existing list
   UtlSListIterator agentListIterator(mAgentCandidateList);
   UtlContainable* pEntry;
   while ((pEntry = agentListIterator()) != NULL)
   {
      mAgentCandidateList.remove(pEntry);
   }

   /**
    * Iterate through the list of target agents, making a local copy
    */
   UtlSListIterator listIterator(*pTargetAgentList);
   ACDAgent* pAgent;
   while ((pAgent = dynamic_cast<ACDAgent*>(listIterator())) != NULL)
   {
      if (mAgentCandidateList.find(pAgent) == NULL)
      {
         mAgentCandidateList.append(pAgent);
         OsSysLog::add(FAC_ACD, gACD_DEBUG,
                       "ACDCall::routeRequestMessage -"
                       "agent(%s) is being added to handle call(%s)",
                       pAgent->getUriString()->data(), mpCallIdentity);
      }
   }

   mConnectionScheme = connectionScheme;
   mRingNoAnswerTime = timeout;

   if (mRouteState != ACDCallRouteState::IDLE)
   {
      if ((mRouteState == ACDCallRouteState::FAILED) ||
         (mRouteState == ACDCallRouteState::TERMINATED) ||
         (mRouteState == ACDCallRouteState::ABORTED)) {
         // reStart the Route FSM from a terminal condition
         resetRouteState() ;
      }
      else
      {
         OsSysLog::add(FAC_ACD, PRI_CRIT,
                       "ACDCall::routeRequestMessage -"
                       "INVALID route state %s",
                       mpRouteStateMachine->getStateString()) ;
         abort() ;
      }
   }

   mpRouteStateMachine->routeRequestEvent(this);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCall::routeRequestAddAgentMessage
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCall::routeRequestAddAgentMessage(ACDAgent* pTargetAgent)
{
   // Make sure that the call still requires routing and that
   // the agent is still available before proceeding
   if (mRouteState != ACDCallRouteState::TRYING) {
      OsSysLog::add(FAC_ACD, gACD_DEBUG,
                    "ACDCall::routeRequestAddAgentMessage -"
                    "call(%s) is in invalid state (%d) for routing",
                    mpCallIdentity, mRouteState);
      return;
   }

   if (pTargetAgent->isAvailable(true) == false) {
      OsSysLog::add(FAC_ACD, gACD_DEBUG,
                    "ACDCall::routeRequestAddAgentMessage -"
                    "agent(%s) is not available any more for handling call(%s)",
                    pTargetAgent->getUriString()->data(), mpCallIdentity);
      return;
   }

   if (mAgentCandidateList.find(pTargetAgent) == NULL) {
      // Add the agent to the candidate list
      mAgentCandidateList.append(pTargetAgent);
      // and initiate a connect request to it.
      SIPX_CALL hCall = pTargetAgent->connect(this);
      if (hCall != SIPX_CALL_NULL)
      {
         mpAcdCallManager->addMapAgentCallHandleToCall(hCall, this);
         OsSysLog::add(FAC_ACD, gACD_DEBUG,
                    "ACDCall::routeRequestAddAgentMessage -"
                    "agent(%s) is being added to handle call(%s)",
                    pTargetAgent->getUriString()->data(), mpCallIdentity);
      }
   }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCall::updateStateMessage
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCall::updateStateMessage(SIPX_CALL callHandle, int event, int cause)
{
   OsSysLog::add(FAC_ACD, gACD_DEBUG,
                 "ACDCall::updateStateMessage - "
                 "Call(%d) incoming hCall=%d [%s] state is being published. "
                 "event %d cause %d TransferFlag %d, TransferConnect %d numTransfer %d",
                 mhCallHandle, callHandle, mpCallIdentity,
                 event, cause, mFlagTransfer, mFlagTransferConnect, mNumTransfer);

   if (mhCallHandle == SIPX_CALL_NULL)
   {
      OsSysLog::add(FAC_ACD, PRI_CRIT,
                    "ACDCall::updateStateMessage - "
                    "mhCallHandle == SIPX_CALL_NULL");
      return ;
   }

   // Process the new state
   switch (event)
   {
      case CALLSTATE_ALERTING:
         if (callHandle == mhCallHandle)
         {
            // Give the state to the associated line to publish
            mpAcdLineReference->publishCallState(this, ALERTING);
         }
         break;

      case CALLSTATE_CONNECTED:
         if (callHandle != mhCallHandle)
         {
     	    // get the Agent call handle
	        if (   (mFlagTransfer == TRUE)
                && (mFlagTransferConnect == FALSE))
            {
                // This is connect for the transfer agent
                acdTransferAgentConnectedEvent(callHandle);
            }
            // This is non transfer case
            else
            { // This is non transfer case
               // This must be an agent answering
               if (cause == CALLSTATE_CONNECTED_ACTIVE)
               {
                  acdAgentConnectedActiveEvent(callHandle);

                  if(mpActiveAgent)
                  {
                     mpActiveAgent->setCallEstablished(true);
                  }
               }
               else if (cause == CALLSTATE_CONNECTED_INACTIVE)
               {
                  acdAgentConnectedInactiveEvent(callHandle);
               }
            }
         }
         else
         {
            // The caller has answered
            // But should not connect if the cause is ACTIVE_HELD case befo
            if (cause != CALLSTATE_CONNECTED_ACTIVE_HELD)
            {
                acdCallConnectedEvent(cause);
            }
         }
         break;

      case CALLSTATE_DISCONNECTED:
         if (mRouteState == ACDCallRouteState::ROUTED)
         {
            // Give the new state to the associated line to publish
            mpAcdLineReference->publishCallState(this, DISCONNECTED);
         }

         if (callHandle != mhCallHandle)
         {
            if (mFlagTransfer == TRUE)
            {
                //if ((mpTransferAgent)&&(mpActiveAgent->getCallHandle() == callHandle))
                // Transfer case - the previous agent is hanging up
                acdTransferAgentDisconnectedEvent(callHandle);
            }
            else
            {
                // This must be regular case active agent hanging up
                acdAgentDisconnectedEvent(callHandle);
            }
         }
         else
         {
            // The caller has hung up
            acdCallDisconnectedEvent();
         }
         break;

      case CALLSTATE_NEWCALL:
         if (cause == CALLSTATE_NEW_CALL_TRANSFER)
         {
            // Play the ringback tone
            sipxCallPlayBufferStart(mhCallHandle,
                              NA_RingbackTone,
                              NA_RingbackToneLength,
                              RAW_PCM_16, true, false, true);
            mPlayingRingback = true;
         }
	 break;

      case CALLSTATE_TRANSFER:
         if (CALLSTATE_TRANSFER_FAILURE == cause)
         {
            OsSysLog::add(FAC_ACD, gACD_DEBUG,
                          "ACDCall::updateStateMessage - "
                          "Call(%d) [%s] CALLSTATE_TRANSFER failed",
                          mhCallHandle, mpCallIdentity);
            acdCallTransferModeFailure();
         }
         break;
   }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCall::acdCallConnectedEvent
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCall::acdCallConnectedEvent(int cause)
{
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCall::acdCallConnectedEvent - Call(%d) [%s] state %d cause %d is being connected.",
                 mhCallHandle, mpCallIdentity, getCurrentCallState(), cause);

   if ((true == getXferPendingAnswer()) && (cause == CALLSTATE_CONNECTED_INACTIVE))
   {
      // Ignore this.  It happens during the bind xfer and we don't want
      // to act on it.
   }
   else if ((TRUE == mFlagCTransfer) && (cause == CALLSTATE_CONNECTED_INACTIVE))
   {
      mCallState = CONNECTED;
      mpRouteStateMachine->acdCTransferConnectedEvent(this);
   }
   else
   {

      // Update the call state
      mCallState = CONNECTED;

      mpRouteStateMachine->acdCallConnectedEvent(this);
   }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCall::acdCallDisconnectedEvent
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCall::acdCallDisconnectedEvent(void)
{
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCall::acdCallDisconnectedEvent - Call(%d) [%s] is being disconected.",
                 mhCallHandle, mpCallIdentity);

   // Update the call state
   mCallState = DISCONNECTED;

   mpRouteStateMachine->acdCallDisconnectedEvent(this);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCall::acdAgentConnectedActiveEvent
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCall::acdAgentConnectedActiveEvent(SIPX_CALL callHandle)
{
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCall::acdAgentConnectedActiveEvent - Call(%d) [%s] is being connected to AgentCall %d",
                 mhCallHandle, mpCallIdentity, callHandle);

   mpRouteStateMachine->acdAgentConnectedActiveEvent(this, callHandle);

   if (mRouteState == ACDCallRouteState::ROUTED) {
      // The call has been successfully routed to the ACDAgent
      // Give the new state to the associated line to publish
      mpAcdLineReference->publishCallState(this, CONNECTED);
   }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCall::acdTransferAgentConnectedEvent
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCall::acdTransferAgentConnectedEvent(SIPX_CALL agentCallHandle)
{
   OsSysLog::add(FAC_ACD, gACD_DEBUG,
                 "ACDCall::acdTransferAgentConnectedEvent - "
                 "Call(%d) [%s] is being connected to AgentCall %d via Conference Join, mFlagTransfer %d mFlagTransferConnect %d",
                 mhCallHandle, mpCallIdentity, agentCallHandle, mFlagTransfer, mFlagTransferConnect);

   SIPX_RESULT rc;
   // Add this call to the conference
   sipxCallHold(agentCallHandle);
   delay(2000);
   rc = sipxConferenceJoin(mhConferenceHandle, agentCallHandle, TRUE);
   if (rc != SIPX_RESULT_SUCCESS)
   {
       OsSysLog::add(FAC_ACD, PRI_ERR,
                     "ACDCall::acdTransferAgentConnectedEvent"
                     "Error(%d) on sipxConferenceJoin for Call(%u), AgentCall(%u), Conference(%u)",
                     rc, mhCallHandle, agentCallHandle, mhConferenceHandle);

       // This is a fatal condition.  Destroy the conference, update state and return
       sipxConferenceDestroy(mhConferenceHandle);
       transitionRouteState(ACDCallRouteState::FAILED);

       // Notify the managing ACDQueue, if defined, of the new state
       if (mpManagingQueue != NULL)
       {
          mpManagingQueue->updateRouteState(this, ACDCallRouteState::FAILED);
       }

       return;
   }
   sipxCallUnhold(agentCallHandle);
   sipxConferenceUnhold(mhConferenceHandle);
   sipxCallPlayBufferStart(agentCallHandle,
                           ConfirmationShortTone,
                           ConfirmationShortToneLength,
                           RAW_PCM_16, false, false, true);
   // mark mFlagTransferConnect TRUE so that if
   // a connect callback is repeated we do not execute
   // this function again
   mFlagTransferConnect = TRUE ;
   // Increment the number of successful transfers
   // This should be incremented right here because
   // resources are going to be allocated right now
   mNumTransfer++;

   if (TRUE == mFlagTransferBlind)
   {
       // Because the transfer agent is going to become the regular agent -
       // take out the mapping between agent call handle and the caller call handle
       mpAcdCallManager->removeMapTransferAgentCallHandleToCall(mpTransferAgent->getCallHandle());
       // It is better to remove this guy from the call target list right now
       UtlSListIterator agentListIterator(mAgentCandidateList);
       UtlContainable* pEntry;
       ACDAgent* pAgent;
       while ((pEntry = agentListIterator()) != NULL)
       {
          pAgent = dynamic_cast<ACDAgent*>(pEntry);
          if ((pAgent == mpActiveAgent) )
          {
             mAgentCandidateList.remove(pEntry);
             break;
          }
       }
       // assign the transfer agent to be the active agent
       mpActiveAgent = mpTransferAgent;
       // Reset the transfer agent
       mpTransferAgent = NULL;
       // create a mapping between the regular agent (which was transfer agent) and
       // the call object
       mpAcdCallManager->addMapAgentCallHandleToCall(mpActiveAgent->getCallHandle(), this);
       mhAssociatedCallHandle = SIPX_CALL_NULL;
       mFlagTransfer = FALSE;
       mFlagTransferConnect = FALSE;
       // At this point transfer has happened so mark
       // mFlagTransferRt TRUE so that Rt for terminate
       // can be recorded when it happens.
       mFlagTransferRt = TRUE;
       ACDRtRecord* pACDRtRec;
       if (NULL != (pACDRtRec = mpAcdCallManager->getAcdServer()->getAcdRtRecord()))
       {
           pACDRtRec->appendTransferCallEvent(ACDRtRecord::TRANSFER, this);
       }
       // And mark the blindTransfer to false !
       mFlagTransferBlind = FALSE;
       // At this point check the number of transfers that have been done
       // If it has hit the max possible limit then disconnect the agent
       if (mNumTransfer >= MAX_NUM_TRANSFER)
       {
           acdAgentDisconnectedEvent(agentCallHandle);
       }

       // Also stop playing the ringback tone to the caller
       if (mPlayingRingback)
       {
          sipxCallPlayBufferStop(mhCallHandle);
          mPlayingRingback = false;
       }
    }

    OsSysLog::add(FAC_ACD, gACD_DEBUG,
                  "ACDCall::acdTransferAgentConnectedEvent"
                  "Completed sipxConferenceJoin for Call(%d), AgentCall(%d), Conference(%d)",
                  mhCallHandle, agentCallHandle, mhConferenceHandle);

}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCall::acdAgentConnectedInactiveEvent
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCall::acdAgentConnectedInactiveEvent(SIPX_CALL callHandle)
{
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCall::acdAgentConnectedInactiveEvent - Call(%d) [%s] is being connected to AgentCall %d",
                 mhCallHandle, mpCallIdentity, callHandle);

   mpRouteStateMachine->acdAgentConnectedInactiveEvent(this, callHandle);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCall::acdAgentDisconnectedEvent
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCall::acdAgentDisconnectedEvent(SIPX_CALL callHandle)
{
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCall::acdAgentDisconnectedEvent - Call(%d) [%s] is being disconnected from AgentCall %d",
                 mhCallHandle, mpCallIdentity, callHandle);

    mpRouteStateMachine->acdAgentDisconnectedEvent(this, callHandle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCall::acdCallTransferModeFailure
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCall::acdCallTransferModeFailure()
{
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCall::acdCallTransferModeFailure - Call(%d) [%s] has failed to transfer.",
                 mhCallHandle, mpCallIdentity);

   // Update the call state
   mpRouteStateMachine->acdCallTransferModeFailure(this);

}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCall::acdTransferAgentDisconnectedEvent
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCall::acdTransferAgentDisconnectedEvent(SIPX_CALL agentCallHandle)
{
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCall::acdTransferAgentDisconnectedEvent - Call(%d) [%s] is being disconnected from AgentCall %d",
                 mhCallHandle, mpCallIdentity, agentCallHandle);
   // If the active ACDAgent is the one that disconnected..., otherwise ignore
   if (mpActiveAgent != NULL &&
       mpActiveAgent->getCallHandle() == agentCallHandle) {
      // Drop the ACDAgent
      mpActiveAgent->drop();

      if (mConnectionScheme == ACDQueue::CONFERENCE) {
         // Remove this leg from the conference
         sipxConferenceRemove(mhConferenceHandle, agentCallHandle);
      }

      // Is it a blind transfer ?
      if (FALSE == mFlagTransferConnect) {
         // This means disconnect of the previous agent
         // has happened before the connect of the transfer agent
         // That indicates it is a blind transfer
         mFlagTransferBlind = TRUE;
      }
      // Now reset some vars related with the call

      // If the current active agent (which is about to be overwritten) is a pseudo agent
      // then delete it
      if(TRUE == mpActiveAgent->isPseudoAgent()) {
         mpAcdCallManager->getAcdServer()->getAcdAgentManager()->deleteACDAgent(mpActiveAgent->getUriString()->data());
      }
      if (FALSE == mFlagTransferBlind) {
         // Because the transfer agent is going to become the regular agent -
         // take out the mapping between agent call handle and the caller call handle
         mpAcdCallManager->removeMapTransferAgentCallHandleToCall(mpTransferAgent->getCallHandle());
         // It is better to remove this guy from the call target list right now
         UtlSListIterator agentListIterator(mAgentCandidateList);
         UtlContainable* pEntry;
         ACDAgent* pAgent;
         while ((pEntry = agentListIterator()) != NULL) {
            pAgent = dynamic_cast<ACDAgent*>(pEntry);
            if ((pAgent == mpActiveAgent) ) {
               mAgentCandidateList.remove(pEntry);
               break;
            }
         }
         // assign the transfer agent to be the active agent
         mpActiveAgent = mpTransferAgent;
         // Reset the transfer agent
         mpTransferAgent = NULL;
         // create a mapping between the regular agent (which was transfer agent) and
         // the call object
         mpAcdCallManager->addMapAgentCallHandleToCall(mpActiveAgent->getCallHandle(), this);
         mhAssociatedCallHandle = SIPX_CALL_NULL;
         mFlagTransfer = FALSE;
         mFlagTransferConnect = FALSE;
         // At this point transfer has happened so mark
         // mFlagTransferRt TRUE so that Rt for terminate
         // can be recorded when it happens.
         mFlagTransferRt = TRUE;
         ACDRtRecord* pACDRtRec;
         if (NULL != (pACDRtRec = mpAcdCallManager->getAcdServer()->getAcdRtRecord()))
         {
            pACDRtRec->appendTransferCallEvent(ACDRtRecord::TRANSFER, this);
         }

         // At this point check the number of transfers that have been done
         // If it has hit the max possible limit then disconnect the agent
         if (mNumTransfer >= MAX_NUM_TRANSFER) {
            // disconnect the latest active agent !
            acdAgentDisconnectedEvent(mpActiveAgent->getCallHandle());
         }

         // Also stop playing the ringback tone to the caller
         if (mPlayingRingback) {
            sipxCallPlayBufferStop(mhCallHandle);
            mPlayingRingback = false;
         }
      }

      OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCall::acdTransferAgentDisconnectedEvent"
                    "Agent(%d) dropped call(%d)", agentCallHandle, mhCallHandle);

   }


}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCall::routeRequestTimeoutEvent
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCall::routeRequestTimeoutEvent(void)
{
   mpRouteStateMachine->routeRequestTimeoutEvent(this);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCall::routeRequestAbortEvent
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCall::routeRequestAbortEvent(void)
{
   mpRouteStateMachine->routeRequestAbortEvent(this);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCall::resetRouteState
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCall::resetRouteState(void)
{
   // Set the FSM to the initial state of IDLE
   mpRouteStateMachine = ACDCallRouteState_IDLE::Instance();
   mRouteState = ACDCallRouteState::IDLE;

   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCall::resetRouteState -"
                 "Call(%s) Route FSM being reset to: IDLE", mpCallIdentity);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCall::transitionRouteState
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCall::transitionRouteState(ACDCallRouteState::eRouteState newState)
{
   ACDCallRouteState::eRouteState currentState;
   UtlString currentStateString;
   UtlString newStateString;

   currentState = mpRouteStateMachine->getState();
   currentStateString = mpRouteStateMachine->getStateString();

   // Now update the FSM to the new state
   mpRouteStateMachine = ACDCallRouteState::Instance(newState);

   mRouteState = newState;
   newStateString = mpRouteStateMachine->getStateString();

   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCall::transitionRouteState - Call(%s) transitioning from: %s to: %s",
                 mpCallIdentity, currentStateString.data(), newStateString.data());

   osPrintf("ACDCall::transitionRouteState - Call(%s) transitioning from: %s to: %s\n",
            mpCallIdentity, currentStateString.data(), newStateString.data());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCall::setCallId
//
//  SYNOPSIS:
//
//  DESCRIPTION: Use the remote call id to set the call id for this call
//               so that outbound calls seem to come "from" the inbound caller.
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////
void ACDCall::setCallId(int callHandle)
{
   char remoteID[1024] ;

   // Get remote from address
   sipxCallGetRemoteID(mhCallHandle, remoteID, sizeof(remoteID));

   // save it where it belongs.
   mpCallIdentity = strdup(remoteID) ;

    OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCall::setCallId callId '%s' callHandle %d", mpCallIdentity, callHandle);
}

#ifdef CML
////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCall::setCallIdentity
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCall::setCallIdentity(void)
{
   UtlString log;
   char      callIdentity[512];
   UtlString aniDigits;

   char callLocalId[256];
   sipxCallGetLocalID(mhCallHandle, callLocalId, sizeof(callLocalId));
   Url localUrl(callLocalId);
   UtlString localUserId;
   localUrl.getUserId(localUserId);
   log += "\n  localUserId = " + localUserId;

   char fromUri[256];
   sipxCallGetFromUri(mhCallHandle, fromUri, sizeof(fromUri));
   Url fromUrl(fromUri);
   UtlString fromUserId;
   fromUrl.getUserId(fromUserId);
   log += "\n  fromUserId = " + fromUserId;

   char toUri[256];
   sipxCallGetToUri(mhCallHandle, toUri, sizeof(toUri));
   Url toUrl(toUri);
   UtlString toUserId;
   toUrl.getUserId(toUserId);
   log += "\n  toUserId = " + toUserId;

   UtlString toDisplayName;
   toUrl.getDisplayName(toDisplayName);
   toDisplayName.strip(UtlString::both, '"');
   log += "\n  toDisplayName = " + toDisplayName;

   UtlString toHostAddress;
   toUrl.getHostAddress(toHostAddress);
   log += "\n  toHostAddress = " + toHostAddress;

   // Get the ANI field
   if ( toUserId.index("CAMA911-") == 0 )
   {
      // Dont need the CAMA911 tag anymore.
      toUserId.remove(0, 8);
      if ( fromUserId.index("CAMA911-") == 0 )
         fromUserId.remove(0, 8);

      aniDigits = fromUserId;
      log += "\n  aniDigits(CAMA911) = " + aniDigits;

      int iPos = toUserId.index('-');
      if ( iPos != -1 )
      {
         UtlString moreDigits = toUserId(iPos+1, UtlString::UTLSTRING_TO_END);

         // T1 with FGD ??
         if ( aniDigits != moreDigits )
         {
            log += "\n  moreDigits = " + moreDigits;

            // If we dont have the * and # then add them.
            if ( ! aniDigits.isNull() && aniDigits.index('*') != 0 )
            {
               aniDigits.insert(0, '*');
               aniDigits.append('#');
            }
            if ( ! moreDigits.isNull() && moreDigits.index('*') != 0 )
            {
               moreDigits.insert(0, '*');
               moreDigits.append('#');
            }

            aniDigits += moreDigits;
         }
      }
   }
   else
   {
      aniDigits = toUserId;
      log += "\n  aniDigits(ANALOG) = " + aniDigits;
   }

   // Already got a displayName ??
   if ( toDisplayName.isNull() )
   {
      // Replace the * and # with spaces
      // and trim the result
      UtlString digits = aniDigits;
      digits.replace('*', ' ');
      digits.replace('#', ' ');
      digits.strip(UtlString::both);

      // Remove consecutive spaces
      int iPos;
      while ( (iPos = digits.index("  ")) != -1 )
         digits.remove(iPos, 1);

      // Shrink the ANI digits by removing the info digits
      UtlString digits1 = digits;
      UtlString digits2;

      // Do we have 2 spills, Analog or T1 FGD ??
      iPos = digits.index(' ');
      if ( iPos != -1 )
      {
         digits1 = digits(0,iPos);
         digits2 = digits(iPos+1, UtlString::UTLSTRING_TO_END);
      }

      // Shrink both spills to a max of 10 digits.
      if ( digits1.length() > 10 )
         digits1.remove(0, digits1.length()-10);
      if ( digits2.length() > 10 )
         digits2.remove(0, digits2.length()-10);

      // Concatenate if 2 spills
      digits = digits1;
      if ( ! digits2.isNull() )
         digits += "-" + digits2;

      // No displayName,
      // we will show the ANI digits instead.
      toDisplayName = digits;
   }
   else
   {
      // Remove leading zeros from the Cisco GTW
      while ( toDisplayName.index('0') == 0 )
         toDisplayName.remove(0,1);
   }

   // Format the new From field
   sprintf(callIdentity, "\"%s\" <sip:%s-%s@%s>",
         toDisplayName.data(),
         localUserId.data(),
         aniDigits.data(),
         toHostAddress.data());

   // Got to know what happened...
   log += "\n  callIdentity = ";
   log += callIdentity;
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCall::setCallIdentity %s", log.data());

   // save it where it belongs.
   mpCallIdentity = new char[strlen(callIdentity)+1];
   strcpy(mpCallIdentity, callIdentity);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  CML
//
//  NAME:        ACDCall::doCallPickUp
//
//  SYNOPSIS:
//
//  DESCRIPTION: Posts the message for call pickup.
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCall::doCallPickUp(ACDAgent* const pAgent) {

   ACDCallMsg callPickUpMessage(ACDCallMsg::CALL_PICKUP,pAgent);
   postMessage(callPickUpMessage);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  CML
//
//  NAME:        ACDCall::doCallPickUp
//
//  SYNOPSIS:
//
//  DESCRIPTION: Initiates the simulated RNA event that will be propagated to the ACDQueue where the
//               actual pickup will be done.
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCall::doCallPickUpMessage(ACDAgent* const pAgent) {
   mpACDAgentPickedUpBy = pAgent;
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCall::doCallPickUpMessage - mpACDAgentPickedUpBy=%d",mpACDAgentPickedUpBy);

   mbIsBeingPickedUp = true;

   // Generate the simulated RNA FAILED. This will get things started into the managing ACDQueue.
   if(mpRouteStateMachine && mpRouteStateMachine->getState() == ACDCallRouteState::TRYING)
   {
      ((ACDCallRouteState_TRYING*)mpRouteStateMachine)->disconnectCallPickUp(this);
   }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  CML
//
//  NAME:        ACDCall::clearBeingPickedUp
//
//  SYNOPSIS:
//
//  DESCRIPTION: Clears the pickup flag for that ACDCall.
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCall::clearBeingPickedUp() {
   mbIsBeingPickedUp = false;
   mpACDAgentPickedUpBy = NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  CML
//
//  NAME:        ACDCall::isBeingPickedUp
//
//  SYNOPSIS:
//
//  DESCRIPTION: Returns true if the current ACDCall is being picked up and also returns the
//               ACDAgent that is picking up the call through pAgent.
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ACDAgent* ACDCall::isBeingPickedUp(bool& bRet) {
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCall::isBeingPickedUp - mpACDAgentPickedUpBy=%d",mpACDAgentPickedUpBy);
   bRet=mbIsBeingPickedUp;
   return mpACDAgentPickedUpBy;
}
#endif

/* ============================ FUNCTIONS ================================= */
