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
#include <os/OsTimer.h>
#include <os/OsEventMsg.h>
#include <os/OsQueuedEvent.h>
#include <utl/UtlSListIterator.h>
#include "ACDCall.h"
#include "ACDServer.h"
#include "ACDCallManager.h"
#include "ACDAudioManager.h"
#include "ACDAgent.h"
#include "ACDQueue.h"
#include "ACDCallRouteState.h"
#include "ACDRtRecord.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
extern OsSysLogPriority gACD_DEBUG;

// CONSTANTS


////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ACDCallRouteState_IDLE State Implementation
//
////////////////////////////////////////////////////////////////////////////////////////////////////

// STATIC VARIABLE INITIALIZATIONS
ACDCallRouteState* ACDCallRouteState_IDLE::mInstance = NULL;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState_IDLE::Instance
//
//  SYNOPSIS:
//
//  DESCRIPTION: Singleton Constructor
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ACDCallRouteState* ACDCallRouteState_IDLE::Instance(void)
{
   // If the singleton instance does not yet exist, create it
   if (mInstance == NULL) {
      mInstance = new ACDCallRouteState_IDLE;
   }

   return mInstance;
}

/* ============================ MANIPULATORS ============================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState_IDLE::routeRequestEvent
//
//  SYNOPSIS:
//
//  DESCRIPTION: Handle the routeRequest Event
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCallRouteState_IDLE::routeRequestEvent(ACDCall* pAcdCallInstance)
{
   // Transition to the TRYING state
   pAcdCallInstance->transitionRouteState(TRYING);

   /**
    * Iterate through the list of target agents,
    * and initiating connect requests to them.
    */
   UtlSListIterator listIterator(pAcdCallInstance->mAgentCandidateList);
   ACDAgent* pAgent;
   while ((pAgent = dynamic_cast<ACDAgent*>(listIterator())) != NULL) {
      SIPX_CALL hCall = pAgent->connect(pAcdCallInstance);
      if (hCall != SIPX_CALL_NULL)
      {
         pAcdCallInstance->getAcdCallManager()->addMapAgentCallHandleToCall(hCall, pAcdCallInstance);
      }
   }

   // If the caller is connected, play ringback tone
   if (pAcdCallInstance->mCallState == ACDCall::CONNECTED)
   {
      // Stop the current audio, if being played
      if (pAcdCallInstance->mPlayingAudio != ACDCall::NO_AUDIO_PLAYING)
      {
         OsSysLog::add(FAC_ACD, gACD_DEBUG,
                       "ACDCallRouteState_IDLE::routeRequestEvent - "
                       "Stopping the current audio for call %d", pAcdCallInstance->mhCallHandle);
         // Use ACDCall::stopAudioMessage() since we are in the same thread context.
         pAcdCallInstance->stopAudioMessage();
      }

      // Play the ringback tone
      sipxCallPlayBufferStart(pAcdCallInstance->mhCallHandle,
                              pAcdCallInstance->NA_RingbackTone,
                              pAcdCallInstance->NA_RingbackToneLength,
                              RAW_PCM_16, true, false, true);
      pAcdCallInstance->mPlayingRingback = true;
   }

   // If max-ring-delay is non-zero, start the ring-no-answer timer
   pAcdCallInstance->mpRingTimeoutTimer->stop(true);
   if (pAcdCallInstance->mRingNoAnswerTime > 0)
   {
      OsTime timeoutTime(pAcdCallInstance->mRingNoAnswerTime, 0);
      pAcdCallInstance->mpRingTimeoutTimer->oneshotAfter(timeoutTime);

      OsSysLog::add(FAC_ACD, gACD_DEBUG,
                    "ACDCallRouteState_IDLE::routeRequestEvent - "
                    "RingNoAnswerTimer is started for Call(%d) at %d seconds",
                    pAcdCallInstance->mhCallHandle, pAcdCallInstance->mRingNoAnswerTime);
   }

   OsSysLog::add(FAC_ACD, gACD_DEBUG,
                 "ACDCallRouteState_IDLE::routeRequestEvent - "
                 "Attempting to route Call(%d)",
                 pAcdCallInstance->mhCallHandle);

}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState_IDLE::acdCallConnectedEvent
//
//  SYNOPSIS:
//
//  DESCRIPTION: Handle the acdCallConnected Event
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCallRouteState_IDLE::acdCallConnectedEvent(ACDCall* pAcdCallInstance)
{
   if (pAcdCallInstance->getXferPendingAnswer() == TRUE)
   {
      OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCallRouteState_IDLE::acdCallConnectedEvent - "
                    "Call(%d) is answered pending transfer",
                    pAcdCallInstance->mhCallHandle);
      // so send the CALL_CONNECTED message to the managing queue
      pAcdCallInstance->mpManagingQueue->callConnected(pAcdCallInstance);
      return ;
   }

   if (pAcdCallInstance->mConnectionScheme == ACDQueue::CONFERENCE) {
      if (join(pAcdCallInstance, pAcdCallInstance->mhCallHandle) != OS_SUCCESS)
      {
         dropAgents(pAcdCallInstance) ;
         terminate(pAcdCallInstance) ;
         return ;
      }
   }
   else {
      OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCallRouteState_IDLE::acdCallConnectedEvent - "
                    "Call(%d) is answered in %s mode",
                    pAcdCallInstance->mhCallHandle,
                    (pAcdCallInstance->mConnectionScheme == ACDQueue::TRANSFER)? "TRANSFER" : "Unknown");
   }

   // See if the optional Welcome Audio has been specified
   if (pAcdCallInstance->mWelcomeAudio != NULL) {
      // Retrieve the audio from the ACDAudioManager
      char* audioBuffer;
      unsigned long audioLength;
      if (pAcdCallInstance->mpAcdAudioManager->getAudio(pAcdCallInstance->mWelcomeAudio,
                                                        audioBuffer, audioLength)) {
         // Wait a tad for the RTP to flow...
         OsTask::delay(500);

         // Start playing the Welcome Audio
         sipxCallPlayBufferStart(pAcdCallInstance->mhCallHandle,
                                 audioBuffer,
                                 audioLength,
                                 RAW_PCM_16, false, false, true);

         pAcdCallInstance->mPlayingAudio = ACDCall::WELCOME_AUDIO_TIMER;
         pAcdCallInstance->mWelcomeAudioPlaying = true;

         // Set the WelcomeAudioPlay timer to fire after the Welcome Audio has completed
         // This will then send the CALL_CONNECTED message to the managing queue
         // Calculate the time required for the audio to play
         OsTime timeoutTime(audioLength / 16);
         pAcdCallInstance->mpWelcomeAudioPlayTimer->stop(true);
         pAcdCallInstance->mpWelcomeAudioPlayTimer->oneshotAfter(timeoutTime);
      }
      else {
         // Failed to load the Welcome Audio, so send
         // the CALL_CONNECTED message to the managing queue
         pAcdCallInstance->mpManagingQueue->callConnected(pAcdCallInstance);
      }
   }
   else {
      // No need to wait for Welcome Audio to finish playing,
      // so send the CALL_CONNECTED message to the managing queue
      pAcdCallInstance->mpManagingQueue->callConnected(pAcdCallInstance);
   }

   // Remain in the IDLE state
   pAcdCallInstance->transitionRouteState(IDLE);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState_IDLE::acdCallDisconnectedEvent
//
//  SYNOPSIS:
//
//  DESCRIPTION: Handle the acdCallDisconnected Event
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCallRouteState_IDLE::acdCallDisconnectedEvent(ACDCall* pAcdCallInstance)
{
   // Stop the audio
   // Use ACDCall::stopAudioMessage() since we are in the same thread context.
   pAcdCallInstance->stopAudioMessage();

   dropAgents(pAcdCallInstance) ;
   terminate(pAcdCallInstance) ;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState_IDLE::acdAgentDisconnectedEvent
//
//  SYNOPSIS:
//
//  DESCRIPTION: handle for the acdAgentDisconnected Event.
//               How could we ever get this event while IDLE????
//               Well, it happens after the previous attempt was
//               ABORTED.  This call is moved to an overflow queue, and
//               the state is reset to IDLE.  But the agent that was being
//               called previously hasn't reported DISCONNECT yet.  So it
//               shows up here.  It is harmless to ignore it.
//
//  RETURNS:     None.
//
//  ERRORS:
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCallRouteState_IDLE::acdAgentDisconnectedEvent(ACDCall* pAcdCallInstance, SIPX_CALL callHandle)
{
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCallRouteState_IDLE::acdAgentDisconnectedEvent -  Call(%d)",
              pAcdCallInstance->mhCallHandle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState_IDLE::acdCallTransferModeFailure
//
//  SYNOPSIS:
//
//  DESCRIPTION: Handle the acdCallTransferModeFailure Event
//    This can happen on overflow transfer's that aren't answered, or
//    aren't authorized, or otherwise fail.
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCallRouteState_IDLE::acdCallTransferModeFailure(ACDCall* pAcdCallInstance)
{
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCallRouteState_IDLE::acdCallTransferModeFailure, Call(%d)",
      pAcdCallInstance->mhCallHandle);

   // Drop the call...nothing further we can do
   UtlString noAudio ;
   pAcdCallInstance->dropCall(0, noAudio) ;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState_IDLE::routeRequestAbortEvent
//
//  SYNOPSIS:
//
//  DESCRIPTION: Handle the routeRequestAbort Event
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCallRouteState_IDLE::routeRequestAbortEvent(ACDCall* pAcdCallInstance)
{
   // If the caller is connected, stop the audio
   if (pAcdCallInstance->mCallState == ACDCall::CONNECTED) {
      pAcdCallInstance->stopAudioMessage();
   }

   OsSysLog::add(FAC_ACD, gACD_DEBUG,
      "ACDCallRouteState_IDLE::routeRequestAbortEvent - "
      "Route request for Call(%d) was ABORTED", pAcdCallInstance->mhCallHandle);

   abort(pAcdCallInstance) ;

}



////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ACDCallRouteState_TRYING State Implementation
//
////////////////////////////////////////////////////////////////////////////////////////////////////

// STATIC VARIABLE INITIALIZATIONS
ACDCallRouteState* ACDCallRouteState_TRYING::mInstance = NULL;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState_TRYING::Instance
//
//  SYNOPSIS:    Singleton Constructor
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

ACDCallRouteState* ACDCallRouteState_TRYING::Instance(void)
{
   // If the singleton instance does not yet exist, create it
   if (mInstance == NULL) {
      mInstance = new ACDCallRouteState_TRYING;
   }

   return mInstance;
}

/* ============================ MANIPULATORS ============================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState_TRYING::acdCallConnectedEvent
//
//  SYNOPSIS:
//
//  DESCRIPTION: Handle the acdCallConnected Event
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCallRouteState_TRYING::acdCallConnectedEvent(ACDCall* pAcdCallInstance)
{
   if (pAcdCallInstance->mConnectionScheme == ACDQueue::CONFERENCE)
   {
      if (join(pAcdCallInstance, pAcdCallInstance->mhCallHandle) != OS_SUCCESS)
      {
         dropAgents(pAcdCallInstance) ;
         terminate(pAcdCallInstance) ;
         return ;
      }
   }

   // Play the ringback tone
   sipxCallPlayBufferStart(pAcdCallInstance->mhCallHandle,
                           pAcdCallInstance->NA_RingbackTone,
                           pAcdCallInstance->NA_RingbackToneLength,
                           RAW_PCM_16, true, false, true);
   pAcdCallInstance->mPlayingRingback = true;

   // Remain in the TRYING state
   pAcdCallInstance->transitionRouteState(TRYING);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState_TRYING::acdCallDisconnecedEvent
//
//  SYNOPSIS:
//
//  DESCRIPTION: Handle the acdCallDisconnected Event
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCallRouteState_TRYING::acdCallDisconnectedEvent(ACDCall* pAcdCallInstance)
{
   // Stop the audio
   // Use ACDCall::stopAudioMessage() since we are in the same thread context.
   pAcdCallInstance->stopAudioMessage();

   dropAgents(pAcdCallInstance) ;

   // Set the terminal route state
   terminate(pAcdCallInstance) ;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState_TRYING::acdAgentConnectedActiveEvent
//
//  SYNOPSIS:
//
//  DESCRIPTION: Handle the acdAgentConnectedActive Event
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCallRouteState_TRYING::acdAgentConnectedActiveEvent(ACDCall* pAcdCallInstance, SIPX_CALL callHandle)
{
   SIPX_RESULT rc;

   // Stop the ring back
   if (pAcdCallInstance->mPlayingRingback) {
      sipxCallPlayBufferStop(pAcdCallInstance->mhCallHandle);
      pAcdCallInstance->mPlayingRingback = false;
   }

   // Stop the ring timeout timer
   pAcdCallInstance->mpRingTimeoutTimer->stop(true);

   /**
    * Iterate through the list of target agents and direct
    * all but the one that answered to drop their calls
    */
   UtlSListIterator listIterator(pAcdCallInstance->mAgentCandidateList);
   ACDAgent* pAgent;
   while ((pAgent = dynamic_cast<ACDAgent*>(listIterator())) != NULL) {
      // Remember the agent that is handling this call
      if (pAgent->getCallHandle() == callHandle) {
         pAcdCallInstance->mpActiveAgent = pAgent;

         // Notify the managing ACDQueue, if defined, of the new state
         if (pAcdCallInstance->mpManagingQueue != NULL) {
            pAcdCallInstance->mpManagingQueue->updateRouteState(pAcdCallInstance, DISCOVERED);
         }
      }
      else {
         // Drop the other agents
         pAgent->drop();
      }
   }

   // Now answer the caller if not already answered
   if (pAcdCallInstance->mCallState != ACDCall::CONNECTED) {
      rc = sipxCallAnswer(pAcdCallInstance->mhCallHandle);
      if (rc != SIPX_RESULT_SUCCESS) {
         OsSysLog::add(FAC_ACD, PRI_ERR, "ACDCallRouteState_TRYING::acdAgentConnectedActive - "
                       "Error(%d) on sipxCallAnswer for Call(%d)", rc, pAcdCallInstance->mhCallHandle);

         // This is a fatal condition.  Drop the agent, update state and return
         if (pAcdCallInstance->mpActiveAgent) {
            pAcdCallInstance->mpActiveAgent->drop();
         }
         else {
            OsSysLog::add(FAC_ACD, PRI_ERR, "ACDCallRouteState_TRYING::acdAgentConnectedActive - "
                          "Failed to drop the active agent for Call(%d) Object(%p)",
                          pAcdCallInstance->mhCallHandle, pAcdCallInstance);
         }
         terminate(pAcdCallInstance) ;

         return;
      }
   }

   if (pAcdCallInstance->mConnectionScheme == ACDQueue::CONFERENCE) {
      // Finally put the agent on-hold
      if (pAcdCallInstance->mpActiveAgent != NULL) {
         pAcdCallInstance->mpActiveAgent->hold();

         OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCallRouteState_TRYING::acdAgentConnectedActive - "
                       "ACDAgent(%s) discovered for Call(%d)",
                       pAcdCallInstance->mpActiveAgent->getUriString()->data(), pAcdCallInstance->mhCallHandle);
      }
      else {
         OsSysLog::add(FAC_ACD, PRI_ERR, "ACDCallRouteState_TRYING::acdAgentConnectedActive - "
                       "Could not find the active agent for Call(%d)",
                       pAcdCallInstance->mhCallHandle);
      }

      // Transition to the DISCOVERED state
      pAcdCallInstance->transitionRouteState(DISCOVERED);
   }
   else {
      if (pAcdCallInstance->mConnectionScheme == ACDQueue::TRANSFER) {
         pAcdCallInstance->mFlagCTransfer = TRUE;
         rc = sipxCallHold(pAcdCallInstance->mhCallHandle);
         if (rc != SIPX_RESULT_SUCCESS) {
            OsSysLog::add(FAC_ACD, PRI_ERR, "ACDCallRouteState_TRYING::acdAgentConnectedActive - "
                          "Error(%d) on sipxCallHold for Call(%d)",
                          rc, pAcdCallInstance->mhCallHandle);
         }
         else {
            OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCallRouteState_TRYING::acdAgentConnectedActive - "
                          "SUCCESS(%d) on sipxCallHold for Call(%d) to Agent(%d)",
                          rc, pAcdCallInstance->mhCallHandle, callHandle);
         }

         rc = sipxCallHold(callHandle);
         if (rc != SIPX_RESULT_SUCCESS) {
            OsSysLog::add(FAC_ACD, PRI_ERR, "ACDCallRouteState_TRYING::acdAgentConnectedActive - "
                          "Error(%d) on sipxCallHold for agent(%d)",
                          rc, callHandle);
         }
         else {
            OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCallRouteState_TRYING::acdAgentConnectedActive - "
                          "SUCCESS(%d) on sipxCallHold for agent(%d)",
                          rc, callHandle);
         }
         // Transition to the ROUTED state
         pAcdCallInstance->transitionRouteState(ROUTED);

#if 0
         rc = sipxCallTransfer(pAcdCallInstance->mhCallHandle, callHandle);

         if (rc != SIPX_RESULT_SUCCESS) {
            OsSysLog::add(FAC_ACD, PRI_ERR, "ACDCallRouteState_TRYING::acdAgentConnectedActive - "
                          "Error(%d) on sipxCallTransfer for Call(%d) to Agent(%d)",
                          rc, pAcdCallInstance->mhCallHandle, callHandle);
         }

         if (pAcdCallInstance->mpActiveAgent != NULL) {
            OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCallRouteState_TRYING::acdAgentConnectedActive - "
                          "Call (%d) is being transferred to ACDAgent(%s)",
                          pAcdCallInstance->mhCallHandle, pAcdCallInstance->mpActiveAgent->getUriString()->data());
         }
         else {
            OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCallRouteState_TRYING::acdAgentConnectedActive - "
                          "Call (%d) is being transferred to Call (%d)",
                          pAcdCallInstance->mhCallHandle, callHandle);
         }

         // Transition to the ROUTED state
         pAcdCallInstance->transitionRouteState(ROUTED);
#endif

      }
   }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState_TRYING::acdAgentDisconnectedEvent
//
//  SYNOPSIS:
//
//  DESCRIPTION: Handle the acdAgentDisconnected Event
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCallRouteState_TRYING::acdAgentDisconnectedEvent(ACDCall* pAcdCallInstance, SIPX_CALL callHandle)
{
   /**
    * Iterate through the list of target agents
    * and find the agent that triggered this event.
    */
   UtlSListIterator listIterator(pAcdCallInstance->mAgentCandidateList);
   ACDAgent* pAgent;
   while ((pAgent = dynamic_cast<ACDAgent*>(listIterator())) != NULL) {
      if (pAgent->getCallHandle() == callHandle) {
         OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCallRouteState_TRYING::acdAgentDisconnectedEvent - "
                       "ACDAgent(%s) has rejected/dropped connection", pAgent->getUriString()->data());
         // Now that the responsible agent has been found, instruct it to drop the call
         pAgent->drop(true);
         // And then remove it from the candidate list
         pAcdCallInstance->mAgentCandidateList.remove(pAgent);
         // And then remove it from being used again by pretending it RNAd.
         pAcdCallInstance->mpManagingQueue->setRingNoAnswerState(pAcdCallInstance, pAgent, true);
         pAcdCallInstance->setRnaState(TRUE);
      }
   }

   // If there are no more agents in the candidate list, clean up and fail the route request
   if (pAcdCallInstance->mAgentCandidateList.isEmpty()) {
      // Stop the ring back
      if (pAcdCallInstance->mPlayingRingback) {
         sipxCallPlayBufferStop(pAcdCallInstance->mhCallHandle);
         pAcdCallInstance->mPlayingRingback = false;
      }

      OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCallRouteState_TRYING::acdAgentDisconnectedEvent - "
                    "All ACDAgents have rejected/dropped connection, FAILING");
      failed(pAcdCallInstance) ;
      return ;
   }
   return ;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState_TRYING::disconnectCallPickUp
//
//  SYNOPSIS:
//
//  DESCRIPTION: Disconnect the call for call pickup. This is similar to a simulated RNA.
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCallRouteState_TRYING::disconnectCallPickUp(ACDCall* pAcdCallInstance)
{
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCallRouteState_TRYING::disconnectCallPickUp - "
                 "Call was picked up");

   failedToRouteCall(pAcdCallInstance,false);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState_TRYING::routeRequestTimeoutEvent
//
//  SYNOPSIS:
//
//  DESCRIPTION: Handle the routeRequestTimeout Event
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCallRouteState_TRYING::routeRequestTimeoutEvent(ACDCall* pAcdCallInstance)
{
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCallRouteState_TRYING::routeRequestTimeoutEvent - "
                 "Ring-no-answer timeout");

   failedToRouteCall(pAcdCallInstance,true);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState_TRYING::failedToRouteCall
//
//  SYNOPSIS:
//
//  DESCRIPTION: Clears up things after a call routing failure while trying.
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCallRouteState_TRYING::failedToRouteCall(ACDCall* pAcdCallInstance, bool bRingNoAnswer) {
   /**
    * Iterate through the list of target agents
    * and direct them to drop their calls
    */
   UtlSListIterator listIterator(pAcdCallInstance->mAgentCandidateList);
   ACDAgent* pAgent;
   while ((pAgent = dynamic_cast<ACDAgent*>(listIterator())) != NULL) {
      pAgent->drop(bRingNoAnswer);

      // Notify the managing ACDQueue, if defined, that a RingNoAnswer occurred on this Agent
      if (pAcdCallInstance->mpManagingQueue != NULL && bRingNoAnswer) {
         pAcdCallInstance->mpManagingQueue->setRingNoAnswerState(pAcdCallInstance, pAgent, true);
         pAcdCallInstance->setRnaState(TRUE);

      }
   }

   failed(pAcdCallInstance) ;
   return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState_TRYING::routeRequestAbortEvent
//
//  SYNOPSIS:
//
//  DESCRIPTION: Handle the routeRequestAbort Event
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCallRouteState_TRYING::routeRequestAbortEvent(ACDCall* pAcdCallInstance)
{
   // Stop the audio (including any ring back)
   // Use ACDCall::stopAudioMessage() since we are in the same thread context.
   pAcdCallInstance->stopAudioMessage();

   dropAgents(pAcdCallInstance) ;

   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCallRouteState_TRYING::routeRequestAbortEvent - "
                 "Route request for Call(%d) was ABORTED", pAcdCallInstance->mhCallHandle);

   abort(pAcdCallInstance) ;

}



////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ACDCallRouteState_DISCOVERED State Implementation
//
////////////////////////////////////////////////////////////////////////////////////////////////////

// STATIC VARIABLE INITIALIZATIONS
ACDCallRouteState* ACDCallRouteState_DISCOVERED::mInstance = NULL;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState_DISCOVERED::Instance
//
//  SYNOPSIS:
//
//  DESCRIPTION: Singleton Constructor
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ACDCallRouteState* ACDCallRouteState_DISCOVERED::Instance(void)
{
   // If the singleton instance does not yet exist, create it
   if (mInstance == NULL) {
      mInstance = new ACDCallRouteState_DISCOVERED;
   }

   return mInstance;
}

/* ============================ MANIPULATORS ============================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState_DISCOVERED::acdCallConnectedEvent
//
//  SYNOPSIS:
//
//  DESCRIPTION: Handle the acdCallConnected Event
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCallRouteState_DISCOVERED::acdCallConnectedEvent(ACDCall* pAcdCallInstance)
{
   if (pAcdCallInstance->mConnectionScheme == ACDQueue::CONFERENCE) {
      if (join(pAcdCallInstance, pAcdCallInstance->mhCallHandle) == OS_FAILED) {
         if (pAcdCallInstance->mpActiveAgent != NULL) {
            // Drop the ACDAgent;
            pAcdCallInstance->mpActiveAgent->drop();
         }
         terminate(pAcdCallInstance) ;
         return;
      }
   }

   // Play the ringback tone
   sipxCallPlayBufferStart(pAcdCallInstance->mhCallHandle,
                           pAcdCallInstance->NA_RingbackTone,
                           pAcdCallInstance->NA_RingbackToneLength,
                           RAW_PCM_16, true, false, true);
   pAcdCallInstance->mPlayingRingback = true;

   // Remain in the DISCOVERED state
   pAcdCallInstance->transitionRouteState(DISCOVERED);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState_DISCOVERED::acdCallDisconnectedEvent
//
//  SYNOPSIS:
//
//  DESCRIPTION: Handle the acdCallDisconnected Event
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCallRouteState_DISCOVERED::acdCallDisconnectedEvent(ACDCall* pAcdCallInstance)
{
   // Stop the audio
   // Use ACDCall::stopAudioMessage() since we are in the same thread context.
   pAcdCallInstance->stopAudioMessage();

   if (pAcdCallInstance->mpActiveAgent != NULL) {
      // Drop the ACDAgent;
      pAcdCallInstance->mpActiveAgent->drop();
   }
   else {
      OsSysLog::add(FAC_ACD, PRI_ERR, "ACDCallRouteState_DISCOVERED::acdCallDisconnectedEvent - "
                    "No active agent to drop Call(%d)", pAcdCallInstance->mhCallHandle);
   }

   terminate(pAcdCallInstance) ;
   return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState_DISCOVERED::acdAgentConnectedInactiveEvent
//
//  SYNOPSIS:
//
//  DESCRIPTION: Handle the acdAgentConnectedInactive Event
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCallRouteState_DISCOVERED::acdAgentConnectedInactiveEvent(ACDCall* pAcdCallInstance, SIPX_CALL callHandle)
{
   SIPX_RESULT rc;

   if (pAcdCallInstance->mCallState == ACDCall::CONNECTED) {
      if (pAcdCallInstance->mConnectionScheme == ACDQueue::CONFERENCE) {
         // Short delay before joining the agent
         OsTask::delay(500);  // Hack to work around conference issues that
                              // seem to occur when RTP isn't yet halted
                              // Better would be to wait for AUDIO_STOP...

         // Add the agent to the conference
         rc = sipxConferenceJoin(pAcdCallInstance->mhConferenceHandle, callHandle);
         if (rc != SIPX_RESULT_SUCCESS) {
            OsSysLog::add(FAC_ACD, PRI_ERR, "ACDCallRouteState_DISCOVERED::acdAgentConnectedInactiveEvent - "
                          "Error(%d) on sipxConferenceJoin for Agent(%d)", rc, callHandle);

            // This is a fatal condition.  Drop the agent
            pAcdCallInstance->mpActiveAgent->drop();

            terminate(pAcdCallInstance) ;
            return;
         }

         osPrintf("ACDCallRouteState_DISCOVERED::acdAgentConnectedInactiveEvent - delaying before taking agent off hold\n");
//         OsTask::delay(500);
         // Unhold the agent
         pAcdCallInstance->mpActiveAgent->unhold();

         OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCallRouteState_DISCOVERED::acdAgentConnectedInactiveEvent - "
                       "Completed sipxConferenceJoin for Agent(%d)", callHandle);

         // Transition to the ON_HOLD state
         pAcdCallInstance->transitionRouteState(ON_HOLD);
      }
   }
   else {
      OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCallRouteState_DISCOVERED::acdAgentConnectedInactiveEvent - "
                    "Caller not yet connected, transitioning to CONNECTING state");

      // The caller has not yet been connected, just transition to the CONNECTING state
      pAcdCallInstance->transitionRouteState(CONNECTING);
   }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState_DISCOVERED::acdAgentDisconnectedEvent
//
//  SYNOPSIS:
//
//  DESCRIPTION: Handle the acdAgentDisconnected Event
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCallRouteState_DISCOVERED::acdAgentDisconnectedEvent(ACDCall* pAcdCallInstance, SIPX_CALL callHandle)
{
   // If the active ACDAgent is the one that disconnected..., otherwise ignore
   if (pAcdCallInstance->mpActiveAgent != NULL &&
       pAcdCallInstance->mpActiveAgent->getCallHandle() == callHandle) {
      // Drop the ACDAgent
      pAcdCallInstance->mpActiveAgent->drop();

      OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCallRouteState_DISCOVERED::acdAgentDisconnectedEvent - "
                    "Agent(%d) dropped call", pAcdCallInstance->mhCallHandle);

      terminate(pAcdCallInstance) ;
   }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState_DISCOVERED::routeRequestAbortEvent
//
//  SYNOPSIS:
//
//  DESCRIPTION: Handle the routeRequestAbort Event
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCallRouteState_DISCOVERED::routeRequestAbortEvent(ACDCall* pAcdCallInstance)
{
   // Stop the audio
   // Use ACDCall::stopAudioMessage() since we are in the same thread context.
   pAcdCallInstance->stopAudioMessage();

   if (pAcdCallInstance->mpActiveAgent != NULL) {
      // Drop the ACDAgent
      pAcdCallInstance->mpActiveAgent->drop();
   }
   else {
      OsSysLog::add(FAC_ACD, PRI_ERR, "ACDCallRouteState_DISCOVERED::routeRequestAbortEvent - "
                    "no active agent to drop Call(%d)", pAcdCallInstance->mhCallHandle);
   }

   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCallRouteState_DISCOVERED::routeRequestAbortEvent - "
                 "Route request for Call(%d) was ABORTED", pAcdCallInstance->mhCallHandle);
   abort(pAcdCallInstance) ;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ACDCallRouteState_CONNECTING State Implementation
//
////////////////////////////////////////////////////////////////////////////////////////////////////

// STATIC VARIABLE INITIALIZATIONS
ACDCallRouteState* ACDCallRouteState_CONNECTING::mInstance = NULL;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState_CONNECTING::Instance
//
//  SYNOPSIS:
//
//  DESCRIPTION: Singleton Constructor
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ACDCallRouteState* ACDCallRouteState_CONNECTING::Instance(void)
{
   // If the singleton instance does not yet exist, create it
   if (mInstance == NULL) {
      mInstance = new ACDCallRouteState_CONNECTING;
   }

   return mInstance;
}

/* ============================ MANIPULATORS ============================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState_CONNECTING::acdCallConnectedEvent
//
//  SYNOPSIS:
//
//  DESCRIPTION: Handle the acdCallConnected Event
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCallRouteState_CONNECTING::acdCallConnectedEvent(ACDCall* pAcdCallInstance)
{
   SIPX_RESULT rc;

   if (pAcdCallInstance->mConnectionScheme == ACDQueue::CONFERENCE) {
      if (join(pAcdCallInstance, pAcdCallInstance->mhCallHandle) == OS_FAILED) {
         if (pAcdCallInstance->mpActiveAgent != NULL) {
            // Drop the ACDAgent
            pAcdCallInstance->mpActiveAgent->drop();
         }
         terminate(pAcdCallInstance) ;
         return;
      }

      // Add the agent to the conference
      SIPX_CALL agentHandle = pAcdCallInstance->mpActiveAgent->getCallHandle();

      // Short delay before joining the agent
//      OsTask::delay(2000);

      rc = sipxConferenceJoin(pAcdCallInstance->mhConferenceHandle, agentHandle);
      if (rc != SIPX_RESULT_SUCCESS) {
         OsSysLog::add(FAC_ACD, PRI_ERR, "ACDCallRouteState_CONNECTING::acdCallConnectedEvent - "
                       "Error(%d) on sipxConferenceJoin for Agent(%d)", rc, agentHandle);

         // This is a fatal condition.
         if (pAcdCallInstance->mpActiveAgent != NULL) {
            // Drop the ACDAgent
            pAcdCallInstance->mpActiveAgent->drop();
         }
         terminate(pAcdCallInstance) ;
         return;
      }

      // Unhold the agent
//      OsTask::delay(500);
      pAcdCallInstance->mpActiveAgent->unhold();

      OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCallRouteState_CONNECTING::acdCallConnectedEvent - "
                    "Completed sipxConferenceJoin for Agent(%d)", agentHandle);

      // Transition to ON_HOLD state
      pAcdCallInstance->transitionRouteState(ON_HOLD);
   }
   else {
      SIPX_CALL agentHandle = pAcdCallInstance->mpActiveAgent->getCallHandle();

      if (pAcdCallInstance->mConnectionScheme == ACDQueue::TRANSFER) {
         pAcdCallInstance->mFlagCTransfer = TRUE;
         rc = sipxCallHold(pAcdCallInstance->mhCallHandle);
         if (rc != SIPX_RESULT_SUCCESS) {
            OsSysLog::add(FAC_ACD, PRI_ERR, "ACDCallRouteState_CONNECTING::acdCallConnectedEvent - "
                          "Error(%d) on sipxCallHold for Call(%d)",
                          rc, pAcdCallInstance->mhCallHandle);
         }
         rc = sipxCallHold(agentHandle);
         if (rc != SIPX_RESULT_SUCCESS) {
            OsSysLog::add(FAC_ACD, PRI_ERR, "ACDCallRouteState_CONNECTING::acdAgentConnectedActive - "
                          "Error(%d) on sipxCallHold for agent(%d)",
                          rc, agentHandle);
         }
         else {
            OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCallRouteState_CONNECTING::acdAgentConnectedActive - "
                          "SUCCESS(%d) on sipxCallHold for agent(%d)",
                          rc, agentHandle);
         }
         // Transition to the ROUTED state
         pAcdCallInstance->transitionRouteState(ROUTED);

#if 0
         rc = sipxCallTransfer(pAcdCallInstance->mhCallHandle, agentHandle);

         if (rc != SIPX_RESULT_SUCCESS) {
            OsSysLog::add(FAC_ACD, PRI_ERR, "ACDCallRouteState_CONNECTING::acdCallConnectedEvent - "
                          "Error(%d) on sipxCallTransfer for Call(%d) to Agent(%d)",
                          rc, pAcdCallInstance->mhCallHandle, agentHandle);
         }

         OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCallRouteState_CONNECTING::acdCallConnectedEvent - "
                       "Call (%d) is being transferred to ACDAgent(%s)",
                       pAcdCallInstance->mhCallHandle, pAcdCallInstance->mpActiveAgent->getUriString()->data());

         // Transition to the ROUTED state
         pAcdCallInstance->transitionRouteState(ROUTED);
#endif
      }
   }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState_CONNECTING::acdCallDisconnectedEvent
//
//  SYNOPSIS:
//
//  DESCRIPTION: Handle the acdCallDisconnected Event
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCallRouteState_CONNECTING::acdCallDisconnectedEvent(ACDCall* pAcdCallInstance)
{
   // Stop the audio
   // Use ACDCall::stopAudioMessage() since we are in the same thread context.
   pAcdCallInstance->stopAudioMessage();

   if (pAcdCallInstance->mpActiveAgent != NULL) {
      // Drop the ACDAgent;
      pAcdCallInstance->mpActiveAgent->drop();
   }
   else {
      OsSysLog::add(FAC_ACD, PRI_ERR, "ACDCallRouteState_CONNECTING::acdCallDisconnectedEvent - "
                    "no active agent to drop Call(%d)", pAcdCallInstance->mhCallHandle);
   }

   terminate(pAcdCallInstance) ;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState_CONNECTING::acdAgentDisconnectedEvent
//
//  SYNOPSIS:
//
//  DESCRIPTION: Handle the acdAgentDisconnected Event
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCallRouteState_CONNECTING::acdAgentDisconnectedEvent(ACDCall* pAcdCallInstance, SIPX_CALL callHandle)
{
   // If the active ACDAgent is the one that disconnected..., otherwise ignore
   if (pAcdCallInstance->mpActiveAgent != NULL &&
       pAcdCallInstance->mpActiveAgent->getCallHandle() == callHandle) {
      // Drop the ACDAgent
      pAcdCallInstance->mpActiveAgent->drop();

      OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCallRouteState_CONNECTING::acdAgentDisconnectedEvent - "
                    "Agent(%d) dropped call", pAcdCallInstance->mhCallHandle);

      terminate(pAcdCallInstance) ;
   }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState_CONNECTING::routeRequestAbortEvent
//
//  SYNOPSIS:
//
//  DESCRIPTION: Handle the routeRequestAbort Event
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCallRouteState_CONNECTING::routeRequestAbortEvent(ACDCall* pAcdCallInstance)
{
   // Stop the audio
   // Use ACDCall::stopAudioMessage() since we are in the same thread context.
   pAcdCallInstance->stopAudioMessage();

   if (pAcdCallInstance->mpActiveAgent != NULL) {
      // Drop the ACDAgent
      pAcdCallInstance->mpActiveAgent->drop();
   }
   else {
      OsSysLog::add(FAC_ACD, PRI_ERR, "ACDCallRouteState_CONNECTING::routeRequestAbortEvent - "
                    "no active agent to drop Call(%d)", pAcdCallInstance->mhCallHandle);
   }

   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCallRouteState_CONNECTING::routeRequestAbortEvent - "
                 "Route request for Call(%d) was ABORTED", pAcdCallInstance->mhCallHandle);

   abort(pAcdCallInstance) ;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ACDCallRouteState_ON_HOLD State Implementation
//
////////////////////////////////////////////////////////////////////////////////////////////////////

// STATIC VARIABLE INITIALIZATIONS
ACDCallRouteState* ACDCallRouteState_ON_HOLD::mInstance = NULL;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState_ON_HOLD::Instance
//
//  SYNOPSIS:
//
//  DESCRIPTION: Singleton Constructor
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ACDCallRouteState* ACDCallRouteState_ON_HOLD::Instance(void)
{
   // If the singleton instance does not yet exist, create it
   if (mInstance == NULL) {
      mInstance = new ACDCallRouteState_ON_HOLD;
   }

   return mInstance;
}

/* ============================ MANIPULATORS ============================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState_ON_HOLD::acdCallDisconnectedEvent
//
//  SYNOPSIS:
//
//  DESCRIPTION: Handle the acdCallDisconnected Event
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCallRouteState_ON_HOLD::acdCallDisconnectedEvent(ACDCall* pAcdCallInstance)
{
   // Stop the audio
   // Use ACDCall::stopAudioMessage() since we are in the same thread context.
   pAcdCallInstance->stopAudioMessage();

   if (pAcdCallInstance->mpActiveAgent != NULL) {
      // Drop the ACDAgent;
      pAcdCallInstance->mpActiveAgent->drop();
   }
   else {
      OsSysLog::add(FAC_ACD, PRI_ERR, "ACDCallRouteState_ON_HOLD::acdCallDisconnectedEvent - "
                    "no active agent to drop Call(%d)", pAcdCallInstance->mhCallHandle);
   }

   // Remove any association between transfer agent call handle and call object
   if (   (pAcdCallInstance->mFlagTransfer == TRUE)
       && (pAcdCallInstance->mpTransferAgent))
   {
      pAcdCallInstance->getAcdCallManager()->removeMapTransferAgentCallHandleToCall(pAcdCallInstance->mpTransferAgent->getCallHandle());
   }

   terminate(pAcdCallInstance) ;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState_ON_HOLD::acdAgentConnectedActiveEvent
//
//  SYNOPSIS:
//
//  DESCRIPTION: Handle the acdAgentConnectedActive Event
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCallRouteState_ON_HOLD::acdAgentConnectedActiveEvent(ACDCall* pAcdCallInstance, SIPX_CALL callHandle)
{
   // If Ringback or Queueu Audio is being played, stop it
   if (pAcdCallInstance->mPlayingRingback || pAcdCallInstance->mPlayingAudio != ACDCall::NO_AUDIO_PLAYING) {
      // Use ACDCall::stopAudioMessage() since we are in the same thread context.
      pAcdCallInstance->stopAudioMessage();
   }

   OsSysLog::add(FAC_ACD, PRI_INFO, "ACDCallRouteState_ON_HOLD::acdAgentConnectedActiveEvent - "
                 "Call(%s) successfully routed to ACDAgent(%s)",
                 pAcdCallInstance->mpCallIdentity, pAcdCallInstance->mpActiveAgent->getUriString()->data());

   // Notify the managing ACDQueue, if defined, of the new state
   if (pAcdCallInstance->mpManagingQueue != NULL) {
      pAcdCallInstance->mpManagingQueue->updateRouteState(pAcdCallInstance, ROUTED);
   }

   if (pAcdCallInstance->mConnectionScheme == ACDQueue::CONFERENCE) {
      sipxConferenceUnhold(pAcdCallInstance->mhConferenceHandle);
      sipxCallPlayBufferStart(callHandle, pAcdCallInstance->ConfirmationShortTone,
                             pAcdCallInstance->ConfirmationShortToneLength,
                             RAW_PCM_16, false, false, true);
   }

   // Transition to the ROUTED state
   pAcdCallInstance->transitionRouteState(ROUTED);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState_ON_HOLD::acdAgentDisconnectedEvent
//
//  SYNOPSIS:
//
//  DESCRIPTION: Handle the acdAgentDisconnected Event
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCallRouteState_ON_HOLD::acdAgentDisconnectedEvent(ACDCall* pAcdCallInstance, SIPX_CALL callHandle)
{
   // The agent hung up before going off-hold.   While it would be nice
   // to reRoute this call, all the timers have been stopped and the
   // conference set up.  The easyist thing to do is terminate the call.

   // It is no different then if the agent hangs up 1 mS after going off
   // hold, when it would be in the ROUTED state.

   // If the active ACDAgent is the one that disconnected..., otherwise ignore
   if (pAcdCallInstance->mpActiveAgent != NULL &&
       pAcdCallInstance->mpActiveAgent->getCallHandle() == callHandle) {
      // Drop the ACDAgent
      pAcdCallInstance->mpActiveAgent->drop();


      OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCallRouteState_ON_HOLD::acdAgentDisconnectedEvent - "
                    "Agent(%d) dropped call", pAcdCallInstance->mhCallHandle);

      // remove any association between transfer agent call handle and call object
      if (   (pAcdCallInstance->mFlagTransfer == TRUE)
          && (pAcdCallInstance->mpTransferAgent))
      {
         pAcdCallInstance->getAcdCallManager()->removeMapTransferAgentCallHandleToCall(pAcdCallInstance->mpTransferAgent->getCallHandle());
      }

      terminate(pAcdCallInstance) ;
   }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState_ON_HOLD::routeRequestAbortEvent
//
//  SYNOPSIS:
//
//  DESCRIPTION: Handle the routeRequestAbort Event
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCallRouteState_ON_HOLD::routeRequestAbortEvent(ACDCall* pAcdCallInstance)
{
   // Stop the audio
   // Use ACDCall::stopAudioMessage() since we are in the same thread context.
   pAcdCallInstance->stopAudioMessage();

   if (pAcdCallInstance->mpActiveAgent != NULL) {
      // Drop the ACDAgent
      pAcdCallInstance->mpActiveAgent->drop();
   }
   else {
      OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCallRouteState_ON_HOLD::routeRequestAbortEvent - "
                    "no active agent to drop Call(%d)", pAcdCallInstance->mhCallHandle);
   }

   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCallRouteState_ON_HOLD::routeRequestAbortEvent - "
                 "Route request for Call(%d) was ABORTED", pAcdCallInstance->mhCallHandle);

   abort(pAcdCallInstance) ;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ACDCallRouteState_ROUTED State Implementation
//
////////////////////////////////////////////////////////////////////////////////////////////////////

// STATIC VARIABLE INITIALIZATIONS
ACDCallRouteState* ACDCallRouteState_ROUTED::mInstance = NULL;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState_ROUTED::Instance
//
//  SYNOPSIS:
//
//  DESCRIPTION: Singleton Constructor
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ACDCallRouteState* ACDCallRouteState_ROUTED::Instance(void)
{
   // If the singleton instance does not yet exist, create it
   if (mInstance == NULL) {
      mInstance = new ACDCallRouteState_ROUTED;
   }

   return mInstance;
}

/* ============================ MANIPULATORS ============================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState_ROUTED::acdCallConnectedEvent
//
//  SYNOPSIS:
//
//  DESCRIPTION: Handle the acdCallConnected Event (do nothing!)
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCallRouteState_ROUTED::acdCallConnectedEvent(ACDCall* pAcdCallInstance)
{
   // This appears to happen after placing both sides on hold for the transfer.
   // Since there is nothing to do, just note it and move on.
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCallRouteState_ROUTED::acdCallConnectedEvent - "
                 "Call(%d)", pAcdCallInstance->mhCallHandle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState_ROUTED::acdCallDisconnectedEvent
//
//  SYNOPSIS:
//
//  DESCRIPTION: Handle the acdCallDisconnected Event
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCallRouteState_ROUTED::acdCallDisconnectedEvent(ACDCall* pAcdCallInstance)
{
   // Stop the audio
   // Use ACDCall::stopAudioMessage() since we are in the same thread context.
   pAcdCallInstance->stopAudioMessage();

   if (pAcdCallInstance->mpActiveAgent != NULL) {
      // Drop the ACDAgent;
      OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCallRouteState_ROUTED::acdCallDisconnectedEvent - "
                    "AgentCall(%d) dropped call",
                    pAcdCallInstance->mpActiveAgent->getCallHandle()) ;
      pAcdCallInstance->mpActiveAgent->drop();
   }
   else {
      OsSysLog::add(FAC_ACD, PRI_ERR, "ACDCallRouteState_ROUTED::acdCallDisconnectedEvent - "
                    "no active agent to drop Call(%d)", pAcdCallInstance->mhCallHandle);
   }

   // Remove any association between transfer agent call handle and call object
   if (   (pAcdCallInstance->mFlagTransfer == TRUE)
       && (pAcdCallInstance->mpTransferAgent))
   {
      pAcdCallInstance->getAcdCallManager()->removeMapTransferAgentCallHandleToCall(pAcdCallInstance->mpTransferAgent->getCallHandle());
   }

   terminate(pAcdCallInstance) ;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState_ROUTED::acdAgentConnectedActiveEvent
//
//  SYNOPSIS:
//
//  DESCRIPTION: Handle the acdAgentConnectedActive Event
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCallRouteState_ROUTED::acdAgentConnectedActiveEvent(ACDCall* pAcdCallInstance, SIPX_CALL callHandle)
{
   // This happens if the Agent releases the routed call from hold.
   // The on hold event CONNECTED::CONNECTED_ACTIVE_HELD is ignored.
   // Then the offhold event CONNECTED::CONNECTED_ACTIVE triggers this.
   // Since there is nothing to do, just note it and move on.
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCallRouteState_ROUTED::acdAgentConnectedEvent - "
                 "Call(%d)", pAcdCallInstance->mhCallHandle);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState_ROUTED::acdAgentDisconnectedEvent
//
//  SYNOPSIS:
//
//  DESCRIPTION: Handle the acdAgentDisconnected Event
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCallRouteState_ROUTED::acdAgentDisconnectedEvent(ACDCall* pAcdCallInstance, SIPX_CALL callHandle)
{
   // If the active ACDAgent is the one that disconnected..., otherwise ignore
   if (pAcdCallInstance->mpActiveAgent != NULL &&
       pAcdCallInstance->mpActiveAgent->getCallHandle() == callHandle) {
      // Drop the ACDAgent
      pAcdCallInstance->mpActiveAgent->drop();

      OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCallRouteState_ROUTED::acdAgentDisconnectedEvent - "
                    "Agent(%d) dropped call", pAcdCallInstance->mhCallHandle);

      // remove any association between transfer agent call handle and call object
      if (   (pAcdCallInstance->mFlagTransfer == TRUE)
          && (pAcdCallInstance->mpTransferAgent))
      {
         pAcdCallInstance->getAcdCallManager()->removeMapTransferAgentCallHandleToCall(pAcdCallInstance->mpTransferAgent->getCallHandle());
      }

      terminate(pAcdCallInstance) ;
   }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState_ROUTED::acdCallTransferModeFailure
//
//  SYNOPSIS:
//
//  DESCRIPTION: Handle the acdCallTransferModeFailure Event
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCallRouteState_ROUTED::acdCallTransferModeFailure(ACDCall* pAcdCallInstance)
{
   // Stop the audio
   // Use ACDCall::stopAudioMessage() since we are in the same thread context.
   pAcdCallInstance->stopAudioMessage();

   if (pAcdCallInstance->mpActiveAgent != NULL) {
      // Drop the ACDAgent;
      pAcdCallInstance->mpActiveAgent->drop();
   }
   else {
      OsSysLog::add(FAC_ACD, PRI_ERR, "ACDCallRouteState_ROUTED::acdCallTransferModeFailure - "
                    "no active agent to drop Call(%d)", pAcdCallInstance->mhCallHandle);
   }

   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCallRouteState_ROUTED::acdCallTransferModeFailure - "
                 "Call(%d) dropped call", pAcdCallInstance->mhCallHandle);

   terminate(pAcdCallInstance) ;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState_ROUTED::routeRequestAbortEvent
//
//  SYNOPSIS:
//
//  DESCRIPTION: Handle the routeRequestAbort Event
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCallRouteState_ROUTED::routeRequestAbortEvent(ACDCall* pAcdCallInstance)
{
   // Stop the audio
   // Use ACDCall::stopAudioMessage() since we are in the same thread context.
   pAcdCallInstance->stopAudioMessage();

   if (pAcdCallInstance->mpActiveAgent != NULL) {
      // Drop the ACDAgent
      pAcdCallInstance->mpActiveAgent->drop();
   }
   else {
      OsSysLog::add(FAC_ACD, PRI_ERR, "ACDCallRouteState_ROUTED::routeRequestAbortEvent - "
                    "no active agent to drop Call(%s)", pAcdCallInstance->mpCallIdentity);
   }

   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCallRouteState_ROUTED::routeRequestAbortEvent - "
                 "Routed connection for Call(%s) was ABORTED", pAcdCallInstance->mpCallIdentity);

   // Remove any association between transfer agent call handle and call object
   if (   (pAcdCallInstance->mFlagTransfer == TRUE)
       && (pAcdCallInstance->mpTransferAgent))
   {
      pAcdCallInstance->getAcdCallManager()->removeMapTransferAgentCallHandleToCall(pAcdCallInstance->mpTransferAgent->getCallHandle());
   }

   terminate(pAcdCallInstance) ;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState_ROUTED::acdCTransferConnectedEvent
//
//  SYNOPSIS:
//
//  DESCRIPTION: Handle the response to the sipxCallHold from the caller(only Transfer mode)
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCallRouteState_ROUTED::acdCTransferConnectedEvent(ACDCall* pAcdCallInstance)
{
   int rc;
   if (pAcdCallInstance->mConnectionScheme == ACDQueue::TRANSFER) {
      if (pAcdCallInstance->mpActiveAgent == NULL) {
         OsSysLog::add(FAC_ACD, PRI_ERR, "ACDCallRouteState_ROUTED::acdCTransferConnectedActive - Null active agent !"
                       "Call (%d) Object(%p)",
                       pAcdCallInstance->mhCallHandle, pAcdCallInstance);
      // Imp - The assert below is temporary - remove it after the testing !
         assert(0);
         return;
      }
      SIPX_CALL agentHandle = pAcdCallInstance->mpActiveAgent->getCallHandle();
      OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCallRouteState_ROUTED::acdCTransferConnectedActive - call sipxCallTransfer"
                       "Call(%d) ACDAgent(%s)",
                       pAcdCallInstance->mhCallHandle, pAcdCallInstance->mpActiveAgent->getUriString()->data());
      rc = sipxCallTransfer(pAcdCallInstance->mhCallHandle, agentHandle);

      if (rc != SIPX_RESULT_SUCCESS) {
         OsSysLog::add(FAC_ACD, PRI_ERR, "ACDCallRouteState_ROUTED::acdCTransferConnectedActive - "
                       "Error(%d) on sipxCallTransfer for Call(%d) to Agent(%d)",
                       rc, pAcdCallInstance->mhCallHandle, agentHandle);
      }
      else {

         OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCallRouteState_ROUTED::acdCTransferConnectedActive - "
                       "Call (%d) is being transferred to ACDAgent(%s)",
                       pAcdCallInstance->mhCallHandle, pAcdCallInstance->mpActiveAgent->getUriString()->data());
      }

      pAcdCallInstance->mFlagCTransfer = FALSE;

   }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ACDCallRouteState_FAILED State Implementation
//
////////////////////////////////////////////////////////////////////////////////////////////////////

// STATIC VARIABLE INITIALIZATIONS
ACDCallRouteState* ACDCallRouteState_FAILED::mInstance = NULL;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState_FAILED::Instance
//
//  SYNOPSIS:
//
//  DESCRIPTION: Singleton Constructor
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ACDCallRouteState* ACDCallRouteState_FAILED::Instance(void)
{
   // If the singleton instance does not yet exist, create it
   if (mInstance == NULL) {
      mInstance = new ACDCallRouteState_FAILED;
   }

   return mInstance;
}

/* ============================ MANIPULATORS ============================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState_FAILED::acdCallDisconnectedEvent
//
//  SYNOPSIS:
//
//  DESCRIPTION: Handle the acdCallDisconnected Event
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCallRouteState_FAILED::acdCallDisconnectedEvent(ACDCall* pAcdCallInstance)
{
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCallRouteState_FAILED::acdCallDisconnectedEvent - "
              "Call(%d)", pAcdCallInstance->mhCallHandle);
   terminate(pAcdCallInstance) ;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ACDCallRouteState_ABORTED State Implementation
//
////////////////////////////////////////////////////////////////////////////////////////////////////

// STATIC VARIABLE INITIALIZATIONS
ACDCallRouteState* ACDCallRouteState_ABORTED::mInstance = NULL;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState_ABORTED::Instance
//
//  SYNOPSIS:
//
//  DESCRIPTION: Singleton Constructor
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ACDCallRouteState* ACDCallRouteState_ABORTED::Instance(void)
{
   // If the singleton instance does not yet exist, create it
   if (mInstance == NULL) {
      mInstance = new ACDCallRouteState_ABORTED;
   }

   return mInstance;
}

/* ============================ MANIPULATORS ============================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState_ABORTED::acdAgentDisconnectedEvent
//
//  SYNOPSIS:
//
//  DESCRIPTION: handle for the acdAgentDisconnected Event.  If the route
//               is aborted, it may hangup on agents.  They report disconnect
//               and we happily do nothing.
//
//  RETURNS:     None.
//
//  ERRORS:
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCallRouteState_ABORTED::acdAgentDisconnectedEvent(ACDCall* pAcdCallInstance, SIPX_CALL callHandle)
{
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCallRouteState_ABORTED::acdAgentDisconnectedEvent -  Call(%d)",
              pAcdCallInstance->mhCallHandle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState_ABORTED::acdCallDisconnectedEvent
//
//  SYNOPSIS:
//
//  DESCRIPTION: handle for the acdCallDisconnected Event.  If the route
//               is aborted, it may hangup on the call.  It reports disconnect
//               and we terminate to cleanup
//
//  RETURNS:     None.
//
//  ERRORS:
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCallRouteState_ABORTED::acdCallDisconnectedEvent(ACDCall* pAcdCallInstance)
{
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCallRouteState_ABORTED::acdCallDisconnectedEvent - "
              "Call(%d)", pAcdCallInstance->mhCallHandle);
   terminate(pAcdCallInstance) ;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ACDCallRouteState_TERMINATED State Implementation
//
////////////////////////////////////////////////////////////////////////////////////////////////////

// STATIC VARIABLE INITIALIZATIONS
ACDCallRouteState* ACDCallRouteState_TERMINATED::mInstance = NULL;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState_TERMINATED::Instance
//
//  SYNOPSIS:
//
//  DESCRIPTION: Singleton Constructor
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ACDCallRouteState* ACDCallRouteState_TERMINATED::Instance(void)
{
   // If the singleton instance does not yet exist, create it
   if (mInstance == NULL) {
      mInstance = new ACDCallRouteState_TERMINATED;
   }

   return mInstance;
}

/* ============================ MANIPULATORS ============================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState_TERMINATED::acdCallDisconnectedEvent
//
//  SYNOPSIS:
//
//  DESCRIPTION: Do nothing, we have hung up on the caller.  The disconnected
//               event just lets us know the hangup happend.
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCallRouteState_TERMINATED::acdCallDisconnectedEvent(ACDCall* pAcdCallInstance)
{
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCallRouteState_TERMINATED::acdCallDisconnectedEvent - "
              "Call(%d)", pAcdCallInstance->mhCallHandle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState_TERMINATED::acdAgentDisconnectedEvent
//
//  SYNOPSIS:
//
//  DESCRIPTION: Do nothing, we have hung up on the agent(s).  The disconnected
//               event just lets us know the hangup happend.
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////
void ACDCallRouteState_TERMINATED::acdAgentDisconnectedEvent(ACDCall* pAcdCallInstance, SIPX_CALL callHandle)
{
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCallRouteState_ROUTED::acdAgentDisconnectedEvent - "
              "Agent(%d)", callHandle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ACDCallRouteState Base Implementation
//
////////////////////////////////////////////////////////////////////////////////////////////////////

// Factory Method
ACDCallRouteState* ACDCallRouteState::Instance(eRouteState state)
{
   switch (state)
   {
      case IDLE: return ACDCallRouteState_IDLE::Instance() ;
      case TRYING: return ACDCallRouteState_TRYING::Instance() ;
      case DISCOVERED: return ACDCallRouteState_DISCOVERED::Instance() ;
      case CONNECTING: return ACDCallRouteState_CONNECTING::Instance() ;
      case ON_HOLD: return ACDCallRouteState_ON_HOLD::Instance() ;
      case ROUTED: return ACDCallRouteState_ROUTED::Instance() ;
      case FAILED: return ACDCallRouteState_FAILED::Instance() ;
      case ABORTED: return ACDCallRouteState_ABORTED::Instance() ;
      case TERMINATED: return ACDCallRouteState_TERMINATED::Instance() ;
      default:
         return NULL ;
   }
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* ============================ MANIPULATORS ============================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState::routeRequestEvent
//
//  SYNOPSIS:
//
//  DESCRIPTION: Default handle for the routeRequest Event
//
//  RETURNS:     None.
//
//  ERRORS:      In theory, if a state transition lands here, it should be considered an error.
//               There is really nothing that can be done other than logging the event.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCallRouteState::routeRequestEvent(ACDCall* pAcdCallInstance)
{
   // Stop the audio
   // Use ACDCall::stopAudioMessage() since we are in the same thread context.
   pAcdCallInstance->stopAudioMessage();

   OsSysLog::add(FAC_ACD, PRI_ERR,
                 "ACDCallRouteState::routeRequestEvent - "
                 "Invalid call route state %s Call(%d)",
                 getStateString(), pAcdCallInstance->mhCallHandle);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState::acdCallConnectedEvent
//
//  SYNOPSIS:
//
//  DESCRIPTION: Default handle for the acdCallConnected Event
//
//  RETURNS:     None.
//
//  ERRORS:      In theory, if a state transition lands here, it should be considered an error.
//               There is really nothing that can be done other than logging the event.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCallRouteState::acdCallConnectedEvent(ACDCall* pAcdCallInstance)
{
   OsSysLog::add(FAC_ACD, PRI_ERR, "ACDCallRouteState::acdCallConnectedEvent - "
                 "Invalid call route state %s Call(%d)",
                 getStateString(), pAcdCallInstance->mhCallHandle);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState::acdCallDisconnectedEvent
//
//  SYNOPSIS:
//
//  DESCRIPTION: Default handle for the acdCallDisconnected Event
//
//  RETURNS:     None.
//
//  ERRORS:      In theory, if a state transition lands here, it should be considered an error.
//               There is really nothing that can be done other than logging the event.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCallRouteState::acdCallDisconnectedEvent(ACDCall* pAcdCallInstance)
{
   // Stop the audio
   // Use ACDCall::stopAudioMessage() since we are in the same thread context.
   pAcdCallInstance->stopAudioMessage();
   OsSysLog::add(FAC_ACD, PRI_ERR, "ACDCallRouteState::acdCallDisconnectedEvent - "
              "Invalid call route state %s Call(%d)",
              getStateString(), pAcdCallInstance->mhCallHandle);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState::acdAgentConnectedActiveEvent
//
//  SYNOPSIS:
//
//  DESCRIPTION: Default handle for the acdAgentConnectedActive Event
//
//  RETURNS:     None.
//
//  ERRORS:      In theory, if a state transition lands here, it should be considered an error.
//               There is really nothing that can be done other than logging the event.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCallRouteState::acdAgentConnectedActiveEvent(ACDCall* pAcdCallInstance, SIPX_CALL callHandle)
{
   OsSysLog::add(FAC_ACD, PRI_ERR, "ACDCallRouteState::acdAgentConnectedActiveEvent - "
              "Invalid call route state %s Call(%d) Agent(%d)",
              getStateString(), pAcdCallInstance->mhCallHandle, callHandle);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState::acdAgentConnectedInactiveEvent
//
//  SYNOPSIS:
//
//  DESCRIPTION: Default handle for the acdAgentConnectedInactive Event
//
//  RETURNS:     None.
//
//  ERRORS:      In theory, if a state transition lands here, it should be considered an error.
//               There is really nothing that can be done other than logging the event.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCallRouteState::acdAgentConnectedInactiveEvent(ACDCall* pAcdCallInstance, SIPX_CALL callHandle)
{
   OsSysLog::add(FAC_ACD, PRI_DEBUG, "ACDCallRouteState::acdAgentConnectedInactiveEvent - "
              "Invalid call route state %s Call(%d) Agent(%d)",
              getStateString(), pAcdCallInstance->mhCallHandle, callHandle);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState::acdAgentDisconnectedEvent
//
//  SYNOPSIS:
//
//  DESCRIPTION: Default handle for the acdAgentDisconnected Event
//
//  RETURNS:     None.
//
//  ERRORS:      In theory, if a state transition lands here, it should be considered an error.
//               There is really nothing that can be done other than logging the event.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCallRouteState::acdAgentDisconnectedEvent(ACDCall* pAcdCallInstance, SIPX_CALL callHandle)
{
   OsSysLog::add(FAC_ACD, PRI_ERR, "ACDCallRouteState::acdAgentDisconnectedEvent - "
              "Invalid call route state %s Call(%d)",
              getStateString(), pAcdCallInstance->mhCallHandle);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState::acdCallTransferModeFailure
//
//  SYNOPSIS:
//
//  DESCRIPTION: Default handle for the CallTransferFailure Event
//
//  RETURNS:     None.
//
//  ERRORS:      In theory, if a state transition lands here, it should be considered an error.
//               There is really nothing that can be done other than logging the event.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////
void ACDCallRouteState::acdCallTransferModeFailure(ACDCall* pAcdCallInstance)
{
   OsSysLog::add(FAC_ACD, PRI_ERR,
      "ACDCallRouteState::acdCallTransferModeFailure - "
              "Invalid call route state %s Call(%d)",
              getStateString(), pAcdCallInstance->mhCallHandle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState::routeRequestTimeoutEvent
//
//  SYNOPSIS:
//
//  DESCRIPTION: Default handle for the routeRequestTimeout Event
//
//  RETURNS:     None.
//
//  ERRORS:      In theory, if a state transition lands here, it should be considered an error.
//               There is really nothing that can be done other than logging the event.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCallRouteState::routeRequestTimeoutEvent(ACDCall* pAcdCallInstance)
{
   OsSysLog::add(FAC_ACD, PRI_ERR, "ACDCallRouteState::routeRequestTimeoutEvent - "
              "Invalid call route state %s Call(%d)",
              getStateString(), pAcdCallInstance->mhCallHandle);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState::routeRequestAbortEvent
//
//  SYNOPSIS:
//
//  DESCRIPTION: Default handle for the routeRequestAbort Event
//
//  RETURNS:     None.
//
//  ERRORS:      In theory, if a state transition lands here, it should be considered an error.
//               There is really nothing that can be done other than logging the event.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCallRouteState::routeRequestAbortEvent(ACDCall* pAcdCallInstance)
{
   // If the caller is connected, stop the audio
   if (pAcdCallInstance->mCallState == ACDCall::CONNECTED) {
      // Stop the current audio, if being played
      if (pAcdCallInstance->mPlayingAudio != ACDCall::NO_AUDIO_PLAYING) {
         OsSysLog::add(FAC_ACD, PRI_DEBUG, "ACDCallRouteState::routeRequestEvent - "
                       "Stopping the current audio for call %d", pAcdCallInstance->mhCallHandle);
         // Use ACDCall::stopAudioMessage() since we are in the same thread context.
         pAcdCallInstance->stopAudioMessage();
      }
   }

   abort(pAcdCallInstance) ;
}

void ACDCallRouteState::acdCTransferConnectedEvent(ACDCall* pAcdCallInstance)
{
   OsSysLog::add(FAC_ACD, PRI_ERR, "ACDCallRouteState::acdCTransferConnectedEvent - "
              "Invalid call route state %s Call(%d)",
              getStateString(), pAcdCallInstance->mhCallHandle);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState::join
//
//  SYNOPSIS:
//
//  DESCRIPTION: Create the conference (if needed) and join the call into it
//
//  RETURNS:     true it worked, false, it didn't and bad things are afoot.
//
//  ERRORS:      If it fails. transit to FAILED state
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////
OsStatus ACDCallRouteState::join(ACDCall* pAcdCallInstance, SIPX_CALL callHandle)
{
   SIPX_RESULT rc;

   // Only create a conference if not exist for this call
   if (pAcdCallInstance->mhConferenceHandle == 0) {
      // Create a conference instance for this call
      rc = sipxConferenceCreate(pAcdCallInstance->mhAcdCallManagerHandle, &pAcdCallInstance->mhConferenceHandle);
      if (rc != SIPX_RESULT_SUCCESS) {
         OsSysLog::add(FAC_ACD, PRI_ERR, "ACDCallRouteState::join - "
            "Error(%d) on sipxConferenceCreate for Call(%d)", rc, callHandle);

            return OS_FAILED;
         }
      }

      // Add this call to the conference
      rc = sipxConferenceJoin(pAcdCallInstance->mhConferenceHandle, callHandle);
      if (rc != SIPX_RESULT_SUCCESS) {
         OsSysLog::add(FAC_ACD, PRI_ERR, "ACDCallRouteState::join - "
            "Error(%d) on sipxConferenceJoin for Call(%d)", rc, callHandle);
         return OS_FAILED;
      }

      OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCallRouteState::join - "
         "Completed sipxConferenceCreate/join for Call(%d)", callHandle);

      return OS_SUCCESS ;
}

void ACDCallRouteState::dropAgents(ACDCall *pAcdCallInstance)
{
    /**
    * Iterate through the list of target agents
    * and direct them to drop their calls (even if they haven't called
    * anyone yet, but they are marked as busy and we need to clear that)
    */
    UtlSListIterator listIterator(pAcdCallInstance->mAgentCandidateList);
   ACDAgent* pAgent;
   while ((pAgent = dynamic_cast<ACDAgent*>(listIterator())) != NULL) {
      pAgent->drop();
   }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState::cleanup
//
//  SYNOPSIS:
//
//  DESCRIPTION: Cleanup everything about the call
//
//  RETURNS:  void
//
//  ERRORS:
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCallRouteState::cleanup(ACDCall *pAcdCallInstance)
{
   // Stop the ring timeout timer
   pAcdCallInstance->mpRingTimeoutTimer->stop(true);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState::terminate
//
//  SYNOPSIS:
//
//  DESCRIPTION: Cleanup and terminate the call, set state to TERMINATED
//               so the queue knows to cleanup the call.
//
//  RETURNS:  void
//
//  ERRORS:
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////
void ACDCallRouteState::terminate(ACDCall *pAcdCallInstance)
{
   cleanup(pAcdCallInstance) ;

   // Set the terminal route state
   pAcdCallInstance->transitionRouteState(TERMINATED);

   // Notify the managing ACDQueue, if defined, of the new state
   if (pAcdCallInstance->mpManagingQueue != NULL) {
      pAcdCallInstance->mpManagingQueue->updateRouteState(pAcdCallInstance, TERMINATED);
   }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState::failed
//
//  SYNOPSIS:
//
//  DESCRIPTION: Cleanup and fail the call, set state to FAILED
//               so the queue can find another agent to handle the call.
//
//  RETURNS:  void
//
//  ERRORS:
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////
void ACDCallRouteState::failed(ACDCall *pAcdCallInstance)
{
   cleanup(pAcdCallInstance) ;

   // Set the terminal route state
   pAcdCallInstance->transitionRouteState(FAILED);

   // Notify the managing ACDQueue, if defined, of the new state
   if (pAcdCallInstance->mpManagingQueue != NULL) {
      pAcdCallInstance->mpManagingQueue->updateRouteState(pAcdCallInstance, FAILED);
   }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallRouteState::abort
//
//  SYNOPSIS:
//
//  DESCRIPTION: Cleanup and abort the call, set state to ABORTED
//
//  RETURNS:  void
//
//  ERRORS:
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////
void ACDCallRouteState::abort(ACDCall *pAcdCallInstance)
{
   cleanup(pAcdCallInstance) ;

   // Set the terminal route state
   pAcdCallInstance->transitionRouteState(ABORTED);

   // Notify the managing ACDQueue, if defined, of the new state
   if (pAcdCallInstance->mpManagingQueue != NULL) {
      pAcdCallInstance->mpManagingQueue->updateRouteState(pAcdCallInstance, ABORTED);
   }
}
