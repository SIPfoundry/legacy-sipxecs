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
#include <utl/UtlTokenizer.h>
#include <utl/UtlSListIterator.h>
#include <utl/UtlHashMapIterator.h>
#include <utl/UtlBool.h>
#include <os/OsTimer.h>
#include <os/OsEventMsg.h>
#include <os/OsQueuedEvent.h>
#include "ACDQueueManager.h"
#include "ACDQueueMsg.h"
#include "ACDAgentManager.h"
#include "ACDAgent.h"
#include "ACDLineManager.h"
#include "ACDLine.h"
#include "ACDCallManager.h"
#include "ACDRtRecord.h"
#include "ACDQueue.h"
#include "ACDServer.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
extern OsSysLogPriority gACD_DEBUG;

// CONSTANTS
const int MIN_TIMEOUT_VALUE = 5;

// STATIC VARIABLE INITIALIZATIONS
const UtlContainableType ACDQueue::TYPE = "ACDQueue";

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueue::ACDQueue
//
//  SYNOPSIS:
//
//  DESCRIPTION: Default constructor
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ACDQueue::ACDQueue(ACDQueueManager* pAcdQueueManager,
                   const char*      pQueueUriString,
                   const char*      pName,
                   int              acdScheme,
                   int              maxRingDelay,
                   int              maxQueueDepth,
                   int              maxWaitTime,
                   bool             fifoOverflow,
                   const char*      pOverflowDestination,
                   const char*      pOverflowEntry,
                   const char*      pOverflowType,
                   int              answerMode,
                   int              callConnectScheme,
                   const char*      pWelcomeAudio,
                   bool             bargeIn,
                   const char*      pQueueAudio,
                   const char*      pBackgroundAudio,
                   int              queueAudioInterval,
                   const char*      pCallTerminationAudio,
                   int              terminationToneDuration,
                   int              agentsWrapupTime,
                   int              agentsNonResponsiveTime,
                   int              maxBounceCount,
                   const char*      pAcdAgentList,
                   const char*      pAcdLineList)
: OsServerTask("ACDQueue-%d")
{
   mpAcdQueueManager        = pAcdQueueManager;
   mUri                     = pQueueUriString;
   mUriString               = pQueueUriString;
   mName                    = pName;
   mAcdScheme               = acdScheme;
   mAcdSchemeString         = "ACDQueue" ;
   mMaxRingDelay            = maxRingDelay;
   mMaxQueueDepth           = maxQueueDepth;
   mMaxWaitTime             = maxWaitTime;
   mFifoOverflow            = fifoOverflow;
   mOverflowDestination     = pOverflowDestination;
   mOverflowEntry           = pOverflowEntry;
   mOverflowType            = pOverflowType;
   mAnswerMode              = answerMode;
   mCallConnectScheme       = callConnectScheme;
   mWelcomeAudio            = pWelcomeAudio;
   // BargeIn - not effective any more
   mBargeIn                 = FALSE;
   mQueueAudio              = pQueueAudio;
   mBackgroundAudio         = pBackgroundAudio;
   mQueueAudioInterval      = queueAudioInterval;
   mCallTerminationAudio    = pCallTerminationAudio;
   mTerminationToneDuration = terminationToneDuration;
   mAgentsWrapupTime        = agentsWrapupTime;
   mAgentsNonResponsiveTime = agentsNonResponsiveTime;
   mMaxBounceCount          = maxBounceCount;
   mAcdAgentListString      = pAcdAgentList;
   mAcdLineListString       = pAcdLineList;

   mpAcdAgentManager        = mpAcdQueueManager->getAcdAgentManagerReference();
   mpAcdCallManager         = mpAcdQueueManager->getAcdCallManagerReference();
   mhAcdCallManagerHandle   = mpAcdQueueManager->getAcdCallManagerHandle();

   mpAcdLineManager         = NULL;
   mUnroutedCallCount       = 0;
   mpRoutePendingAnswer     = NULL;

   UtlString domainName;
   domainName =  (mpAcdQueueManager->getAcdServer())->getDomain();

   if (mOverflowType == HUNT_GROUP_TAG && mOverflowDestination != NULL) {
      mOverflowEntry = mOverflowDestination;
      OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDQueue::ACDQueue[%s] - Overflow to huntgroup. Set mOverflowEntry  = %s",mUriString.data(),mOverflowEntry.data());
   } else if (mOverflowType == QUEUE_TAG && mOverflowDestination != NULL) {
      mOverflowQueue = mOverflowDestination;
      OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDQueue::ACDQueue[%s] - set mOverflowQueue  = %s",mUriString.data(),mOverflowQueue.data());
   } else if (mOverflowEntry!= NULL && !mOverflowEntry.contains("@")) {
      mOverflowEntry.append("@");
      mOverflowEntry.append(domainName);
      OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDQueue::ACDQueue[%s] - set mOverflowEntry  = %s",mUriString.data(),mOverflowEntry.data());
   }

   // Convert the comma delimited list of ACDAgent URI's to an SList of ACDAgent pointers
   buildACDAgentList();

   // Convert the comma delimited list of ACDLine URI's to an SList of ACDLine pointers
   buildACDLineList();

   // Adjust various timers
   adjustTimers();

   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDQueue::ACDQueue[%s] - MaxRingDelay = %d, MaxWaitTime = %d, Overflow Queue = %s",
                 mUriString.data(), mMaxRingDelay, mMaxWaitTime, mOverflowQueue.data());
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueue::~ACDQueue
//
//  SYNOPSIS:
//
//  DESCRIPTION: Destructor
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ACDQueue::~ACDQueue()
{
   // Clear the Call-RNAList map
   UtlHashMapIterator iter(mCallToRnaListMap);
   UtlContainable* key = NULL;
   UtlSList* value = NULL ;
   while ( (key = iter())!=NULL )
   {
       value = dynamic_cast<UtlSList*>(iter.value());
       mCallToRnaListMap.removeReference( key );
       value->destroyAll();
       delete value;
       iter.reset();
   }
}

/* ============================ MANIPULATORS ============================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueue::addCall
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

void ACDQueue::addCall(ACDCall* pCallRef)
{
   ACDQueueMsg addCallMsg(ACDQueueMsg::ADD_CALL, pCallRef);
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDQueue::addCall - postMessage MsgType: %d, MsgSubType: %d", addCallMsg.getMsgType(), addCallMsg.getMsgSubType());
   postMessage(addCallMsg);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueue::removeCall
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

void ACDQueue::removeCall(ACDCall* pCallRef)
{
   ACDQueueMsg removeCallMsg(ACDQueueMsg::REMOVE_CALL, pCallRef);
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDQueue::removeCall - post MsgType: %d, MsgSubType: %d", removeCallMsg.getMsgType(), removeCallMsg.getMsgSubType());
   postMessage(removeCallMsg);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueue::callConnected
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

void ACDQueue::callConnected(ACDCall* pCallRef)
{
   ACDQueueMsg callConnectedMsg(ACDQueueMsg::CALL_CONNECTED, pCallRef);
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDQueue::callConnected - postMessage MsgType: %d, MsgSubType: %d", callConnectedMsg.getMsgType(), callConnectedMsg.getMsgSubType());
   postMessage(callConnectedMsg);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueue::updateRouteState
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

void ACDQueue::updateRouteState(ACDCall* pCallRef, ACDCallRouteState::eRouteState state)
{
   ACDQueueMsg updateRouteStateMsg(ACDQueueMsg::UPDATE_ROUTE_STATE, pCallRef, state);
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDQueue::updateRouteState - postMessage MsgType: %d, MsgSubType: %d", updateRouteStateMsg.getMsgType(), updateRouteStateMsg.getMsgSubType());
   postMessage(updateRouteStateMsg);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueue::queueMaxWaitTime
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

void ACDQueue::queueMaxWaitTime(ACDCall* pCallRef)
{
   ACDQueueMsg queueMaxWaitTimeMsg(ACDQueueMsg::MAX_WAIT_TIME, pCallRef);
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDQueue::queueMaxWaitTime - postMessage MsgType: %d, MsgSubType: %d", queueMaxWaitTimeMsg.getMsgType(), queueMaxWaitTimeMsg.getMsgSubType());
   postMessage(queueMaxWaitTimeMsg);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueue::agentAvailable
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

void ACDQueue::agentAvailable(ACDAgent* pAgentRef)
{
   ACDQueueMsg agentAvailableMsg(ACDQueueMsg::AGENT_AVAILABLE, pAgentRef);
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDQueue::agentAvailable - postMessage MsgType: %d, MsgSubType: %d", agentAvailableMsg.getMsgType(), agentAvailableMsg.getMsgSubType());
   postMessage(agentAvailableMsg);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueue::setAttributes
//
//  SYNOPSIS:    void setAttributes(
//                 ProvisioningAttrList& rRequestAttributes) ProvisioningAgent attribute list
//                                                           containing one or more ACDQueue
//                                                           attributes to be updated.
//
//  DESCRIPTION: Used by the ACDQueueManager::set() function to update on or more attributes
//               in response to recieving a provisioning set request.
//
//  RETURNS:     None.
//
//  ERRORS:      If an error is encountered, an exception will be thrown, with the description
//               of the error being specified in "UtlString error".
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////


void ACDQueue::setAttributes(ProvisioningAttrList& rRequestAttributes)
{
   // Set the individual attributes
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDQueue::setAttributes(%s)",
                 mUriString.data());

   // take action only if adminstrative state is STANDBY for some attributes
   if (mpAcdQueueManager->getAcdServer()->getAdministrativeState() == ACDServer::STANDBY){
      // max-queue-depth
      rRequestAttributes.getAttribute(QUEUE_MAX_QUEUE_DEPTH_TAG, mMaxQueueDepth);
      // acd-scheme
      rRequestAttributes.getAttribute(QUEUE_ACD_SCHEME_TAG, mAcdScheme);

      // fifo-overflow
      rRequestAttributes.getAttribute(QUEUE_FIFO_OVERFLOW_TAG, mFifoOverflow);
   }

   // name
   rRequestAttributes.getAttribute(QUEUE_NAME_TAG, mName);

   // max-ring-delay
   rRequestAttributes.getAttribute(QUEUE_MAX_RING_DELAY_TAG, mMaxRingDelay);


   // max-wait-time
   rRequestAttributes.getAttribute(QUEUE_MAX_WAIT_TIME_TAG, mMaxWaitTime);

   // overflow-destination
   rRequestAttributes.getAttribute(QUEUE_OVERFLOW_DESTINATION_TAG, mOverflowDestination);

   // overflow-entry
   rRequestAttributes.getAttribute(QUEUE_OVERFLOW_ENTRY_TAG, mOverflowEntry);

   // overflow-type
   rRequestAttributes.getAttribute(QUEUE_OVERFLOW_TYPE_TAG, mOverflowType);

   // answer-mode
   rRequestAttributes.getAttribute(QUEUE_ANSWER_MODE_TAG, mAnswerMode);

   // call-connect-scheme
   rRequestAttributes.getAttribute(QUEUE_CALL_CONNECT_SCHEME_TAG, mCallConnectScheme);

   // welcome-audio
   rRequestAttributes.getAttribute(QUEUE_WELCOME_AUDIO_TAG, mWelcomeAudio);

   // barge-in
   //rRequestAttributes.getAttribute(QUEUE_BARGE_IN_TAG, mBargeIn);

   // queue-audio
   rRequestAttributes.getAttribute(QUEUE_QUEUE_AUDIO_TAG, mQueueAudio);

   // background-audio
   rRequestAttributes.getAttribute(QUEUE_BACKGROUND_AUDIO_TAG, mBackgroundAudio);

   // queue-audio-interval
   rRequestAttributes.getAttribute(QUEUE_QUEUE_AUDIO_INTERVAL_TAG, mQueueAudioInterval);

   // call-terimination-audio
   rRequestAttributes.getAttribute(QUEUE_CALL_TERMINATION_AUDIO_TAG, mCallTerminationAudio);

   // termination-tone-duration
   rRequestAttributes.getAttribute(QUEUE_TERMINATION_TONE_DURATION_TAG, mTerminationToneDuration);

   // agents-wrap-up-time
   rRequestAttributes.getAttribute(QUEUE_AGENTS_WRAP_UP_TIME_TAG, mAgentsWrapupTime);

   // agents-non-responsive-time
   rRequestAttributes.getAttribute(QUEUE_AGENTS_NON_RESPONSIVE_TIME_TAG, mAgentsNonResponsiveTime);

   // max-bounce-count
   rRequestAttributes.getAttribute(QUEUE_MAX_BOUNCE_COUNT_TAG, mMaxBounceCount);

   // acd-agent-list
   if (rRequestAttributes.getAttribute(QUEUE_ACD_AGENT_LIST_TAG, mAcdAgentListString)) {
      buildACDAgentList();
   }

   // acd-line-list
   if (rRequestAttributes.getAttribute(QUEUE_ACD_LINE_LIST_TAG, mAcdLineListString)) {
      buildACDLineList();
   }

   // Adjust various timers
   adjustTimers();

   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDQueue::setAttributes ACDQueue[%s] has been updated - MaxRingDelay = %d, MaxWaitTime = %d, Overflow Queue = %s",
                 mUriString.data(), mMaxRingDelay, mMaxWaitTime, mOverflowQueue.data());
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueue::buildACDAgentList
//
//  SYNOPSIS:
//
//  DESCRIPTION: Convert the comma delimited list of ACDAgent URI's to an SList of ACDAgent pointers
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDQueue::buildACDAgentList(void)
{
   UtlSList         newAgentList;
   ACDAgent *pAgent ;

   OsSysLog::add(FAC_ACD, gACD_DEBUG,
      "ACDQueue::buildACDAgentList[%s] mAcdAgentListString='%s'",
         mUriString.data(),
         mAcdAgentListString ? mAcdAgentListString.data() : "(empty)") ;

   // First add the agents in the string to the mAcdAgentList

   if (mAcdAgentListString != NULL) {
      // Walk thru the string, looking for comma delemited URIs
      UtlTokenizer tokenList(mAcdAgentListString);
      UtlString entry;
      while (tokenList.next(entry, ",")) {
         entry.strip(UtlString::both);
         pAgent = mpAcdAgentManager->getAcdAgentReference(entry);
         if (pAgent != NULL) {
            // Add valid entries to the new Agent list
            newAgentList.append(pAgent);
            if (mAcdAgentList.find(pAgent) == NULL) {
               // And to the real list, if it isn't already there
               mAcdAgentList.append(pAgent);
               // Log the fact he is signed in/out so the stats are correct
               pAgent->logSignIn(this, pAgent->isAvailable(false));
               OsSysLog::add(FAC_ACD, gACD_DEBUG,
                  "ACDQueue::buildACDAgentList[%s] Agent='%s' added",
                     mUriString.data(), entry.data()) ;
            }
         }
         else
         {
            OsSysLog::add(FAC_ACD, gACD_DEBUG,
               "ACDQueue::buildACDAgentList[%s] Agent='%s' not found",
                     mUriString.data(), entry.data()) ;
         }
      }
   }

   UtlSListIterator listIterator(mAcdAgentList);

   // Then remove the agents in the mAcdAgentList that are not in the string

   // Iterate through the ACDAgent list remove ones not in the new list
   while ((pAgent = dynamic_cast<ACDAgent*>(listIterator())) != NULL) {
      if (newAgentList.find(pAgent) == NULL) {
         OsSysLog::add(FAC_ACD, gACD_DEBUG,
            "ACDQueue::buildACDAgentList[%s] Agent='%s' removed",
                  mUriString.data(), pAgent->getUriString()->data()) ;
         mAcdAgentList.remove(pAgent);
         // Sign out agent so it shows up in the logs for stats
         pAgent->logSignIn(this, false);
      }
   }

   newAgentList.removeAll() ;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueue::buildACDLineList
//
//  SYNOPSIS:
//
//  DESCRIPTION: Convert the comma delimited list of ACDLine URI's to an SList of ACDLine pointers
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDQueue::buildACDLineList(void)
{
   // First clear the existing list
   UtlSListIterator listIterator(mAcdLineList);
   UtlContainable* pEntry;
   while ((pEntry = listIterator()) != NULL) {
      mAcdLineList.remove(pEntry);
   }

   // Check to see if there is anything in the mAcdLineListString
   if (mAcdLineListString == NULL) {
      // It is empty, just return
      return;
   }

   // Add the new entries to the list
   UtlTokenizer tokenList(mAcdLineListString);
   UtlString entry;
   while (tokenList.next(entry, ",")) {
      entry.strip(UtlString::both);
      ACDLine* pLine = mpAcdLineManager->getAcdLineReference(entry);
      if (pLine != NULL) {
         if (mAcdLineList.find(pLine) == NULL) {
            mAcdLineList.append(pLine);
         }
      }
   }
}

/* ============================ ACCESSORS ================================= */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueue::getUriString
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

UtlString* ACDQueue::getUriString(void)
{
   return &mUriString;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueue::hash
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

unsigned ACDQueue::hash() const
{
   return mUriString.hash();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueue::getContainableType
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

UtlContainableType ACDQueue::getContainableType() const
{
   return ACDQueue::TYPE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueue::getAttributes
//
//  SYNOPSIS:    bool getAttributes(
//                 ProvisioningAttrList&  rRequestAttributes, ProvisioningAgent attribute list
//                                                            containing one or more ACDQueue
//                                                            attributes to be retrieved.
//                 ProvisioningAttrList*& prResponse)         ProvisioningAgent attribute list
//                                                            containing the requested attributes
//                                                            and their corresponding values.
//
//  DESCRIPTION: Used by the ACDQueueManager::get() function to request the value of one or more
//               attributes in response to recieving a provisioning get request.
//
//  ERRORS:      If an error is encountered, an exception will be thrown, with the description
//               of the error being specified in "UtlString error".
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDQueue::getAttributes(ProvisioningAttrList& rRequestAttributes, ProvisioningAttrList*& prResponse)
{
   // See if there are any specific attributes listed in the request
   if (rRequestAttributes.attributePresent(QUEUE_NAME_TAG) ||
       rRequestAttributes.attributePresent(QUEUE_ACD_SCHEME_TAG) ||
       rRequestAttributes.attributePresent(QUEUE_MAX_RING_DELAY_TAG) ||
       rRequestAttributes.attributePresent(QUEUE_MAX_QUEUE_DEPTH_TAG) ||
       rRequestAttributes.attributePresent(QUEUE_MAX_WAIT_TIME_TAG) ||
       rRequestAttributes.attributePresent(QUEUE_FIFO_OVERFLOW_TAG) ||
       rRequestAttributes.attributePresent(QUEUE_OVERFLOW_DESTINATION_TAG) ||
       rRequestAttributes.attributePresent(QUEUE_ANSWER_MODE_TAG) ||
       rRequestAttributes.attributePresent(QUEUE_CALL_CONNECT_SCHEME_TAG) ||
       rRequestAttributes.attributePresent(QUEUE_WELCOME_AUDIO_TAG) ||
       rRequestAttributes.attributePresent(QUEUE_BARGE_IN_TAG) ||
       rRequestAttributes.attributePresent(QUEUE_QUEUE_AUDIO_TAG) ||
       rRequestAttributes.attributePresent(QUEUE_BACKGROUND_AUDIO_TAG) ||
       rRequestAttributes.attributePresent(QUEUE_QUEUE_AUDIO_INTERVAL_TAG) ||
       rRequestAttributes.attributePresent(QUEUE_CALL_TERMINATION_AUDIO_TAG) ||
       rRequestAttributes.attributePresent(QUEUE_TERMINATION_TONE_DURATION_TAG) ||
       rRequestAttributes.attributePresent(QUEUE_AGENTS_WRAP_UP_TIME_TAG) ||
       rRequestAttributes.attributePresent(QUEUE_MAX_BOUNCE_COUNT_TAG) ||
       rRequestAttributes.attributePresent(QUEUE_ACD_AGENT_LIST_TAG) ||
       rRequestAttributes.attributePresent(QUEUE_ACD_LINE_LIST_TAG) ||
       rRequestAttributes.attributePresent(QUEUE_QUEUE_DEPTH_TAG)) {
      // At least one attribute has been requested, go through list and retrieve
      // name
      if (rRequestAttributes.attributePresent(QUEUE_NAME_TAG)) {
         prResponse->setAttribute(QUEUE_NAME_TAG, mName);
      }

      // acd-scheme
      if (rRequestAttributes.attributePresent(QUEUE_ACD_SCHEME_TAG)) {
         prResponse->setAttribute(QUEUE_ACD_SCHEME_TAG, mAcdScheme);
      }

      // max-ring-delay
      if (rRequestAttributes.attributePresent(QUEUE_MAX_RING_DELAY_TAG)) {
         prResponse->setAttribute(QUEUE_MAX_RING_DELAY_TAG, mMaxRingDelay);
      }

      // max-queue-depth
      if (rRequestAttributes.attributePresent(QUEUE_MAX_QUEUE_DEPTH_TAG)) {
         prResponse->setAttribute(QUEUE_MAX_QUEUE_DEPTH_TAG, mMaxQueueDepth);
      }

      // max-wait-time
      if (rRequestAttributes.attributePresent(QUEUE_MAX_WAIT_TIME_TAG)) {
         prResponse->setAttribute(QUEUE_MAX_WAIT_TIME_TAG, mMaxWaitTime);
      }

      // fifo-overflow
      if (rRequestAttributes.attributePresent(QUEUE_FIFO_OVERFLOW_TAG)) {
         prResponse->setAttribute(QUEUE_FIFO_OVERFLOW_TAG, mFifoOverflow);
      }

      // overflow-destination
      if (rRequestAttributes.attributePresent(QUEUE_OVERFLOW_DESTINATION_TAG)) {
         prResponse->setAttribute(QUEUE_OVERFLOW_DESTINATION_TAG, mOverflowDestination);
      }

      // overflow-entry
      if (rRequestAttributes.attributePresent(QUEUE_OVERFLOW_ENTRY_TAG)) {
         prResponse->setAttribute(QUEUE_OVERFLOW_ENTRY_TAG, mOverflowEntry);
      }

      // overflow-type
      if (rRequestAttributes.attributePresent(QUEUE_OVERFLOW_TYPE_TAG)) {
         prResponse->setAttribute(QUEUE_OVERFLOW_TYPE_TAG, mOverflowType);
      }

      // answer-mode
      if (rRequestAttributes.attributePresent(QUEUE_ANSWER_MODE_TAG)) {
         prResponse->setAttribute(QUEUE_ANSWER_MODE_TAG, mAnswerMode);
      }

      // call-connect-scheme
      if (rRequestAttributes.attributePresent(QUEUE_CALL_CONNECT_SCHEME_TAG)) {
         prResponse->setAttribute(QUEUE_CALL_CONNECT_SCHEME_TAG, mCallConnectScheme);
      }

      // welcome-audio
      if (rRequestAttributes.attributePresent(QUEUE_WELCOME_AUDIO_TAG)) {
         prResponse->setAttribute(QUEUE_WELCOME_AUDIO_TAG, mWelcomeAudio);
      }

#if 0
      // barge-in
      if (rRequestAttributes.attributePresent(QUEUE_BARGE_IN_TAG)) {
         prResponse->setAttribute(QUEUE_BARGE_IN_TAG, mBargeIn);
      }
#endif

      // queue-audio
      if (rRequestAttributes.attributePresent(QUEUE_QUEUE_AUDIO_TAG)) {
         prResponse->setAttribute(QUEUE_QUEUE_AUDIO_TAG, mQueueAudio);
      }

      // background-audio
      if (rRequestAttributes.attributePresent(QUEUE_BACKGROUND_AUDIO_TAG)) {
         prResponse->setAttribute(QUEUE_BACKGROUND_AUDIO_TAG, mBackgroundAudio);
      }

      // queue-audio-interval
      if (rRequestAttributes.attributePresent(QUEUE_QUEUE_AUDIO_INTERVAL_TAG)) {
         prResponse->setAttribute(QUEUE_QUEUE_AUDIO_INTERVAL_TAG, mQueueAudioInterval);
      }

      // call-termination-audio
      if (rRequestAttributes.attributePresent(QUEUE_CALL_TERMINATION_AUDIO_TAG)) {
         prResponse->setAttribute(QUEUE_CALL_TERMINATION_AUDIO_TAG, mCallTerminationAudio);
      }

      // termination-tone-duration
      if (rRequestAttributes.attributePresent(QUEUE_TERMINATION_TONE_DURATION_TAG)) {
         prResponse->setAttribute(QUEUE_TERMINATION_TONE_DURATION_TAG, mTerminationToneDuration);
      }

      // agents-wrap-up-time
      if (rRequestAttributes.attributePresent(QUEUE_AGENTS_WRAP_UP_TIME_TAG)) {
         prResponse->setAttribute(QUEUE_AGENTS_WRAP_UP_TIME_TAG, mAgentsWrapupTime);
      }

      // agents-non-responsive-time
      if (rRequestAttributes.attributePresent(QUEUE_AGENTS_NON_RESPONSIVE_TIME_TAG)) {
         prResponse->setAttribute(QUEUE_AGENTS_NON_RESPONSIVE_TIME_TAG, mAgentsNonResponsiveTime);
      }

      // max-bounce-count
      if (rRequestAttributes.attributePresent(QUEUE_MAX_BOUNCE_COUNT_TAG)) {
         prResponse->setAttribute(QUEUE_MAX_BOUNCE_COUNT_TAG, mMaxBounceCount);
      }

      // acd-agent-list
      if (rRequestAttributes.attributePresent(QUEUE_ACD_AGENT_LIST_TAG)) {
         prResponse->setAttribute(QUEUE_ACD_AGENT_LIST_TAG, mAcdAgentListString);
      }

      // acd-line-list
      if (rRequestAttributes.attributePresent(QUEUE_ACD_LINE_LIST_TAG)) {
         prResponse->setAttribute(QUEUE_ACD_LINE_LIST_TAG, mAcdLineListString);
      }

      // queue-depth
      if (rRequestAttributes.attributePresent(QUEUE_QUEUE_DEPTH_TAG)) {
         prResponse->setAttribute(QUEUE_QUEUE_DEPTH_TAG, static_cast<int>(mUnroutedCallList.entries()));
      }
   }
   else {
      // No specific attributes were requested, send them all back
      // name
      prResponse->setAttribute(QUEUE_NAME_TAG, mName);

      // acd-scheme
      prResponse->setAttribute(QUEUE_ACD_SCHEME_TAG, mAcdScheme);

      // max-ring-delay
      prResponse->setAttribute(QUEUE_MAX_RING_DELAY_TAG, mMaxRingDelay);

      // max-queue-depth
      prResponse->setAttribute(QUEUE_MAX_QUEUE_DEPTH_TAG, mMaxQueueDepth);

      // max-wait-time
      prResponse->setAttribute(QUEUE_MAX_WAIT_TIME_TAG, mMaxWaitTime);

      // fifo-overflow
      prResponse->setAttribute(QUEUE_FIFO_OVERFLOW_TAG, mFifoOverflow);

      // overflow-destination
      prResponse->setAttribute(QUEUE_OVERFLOW_DESTINATION_TAG, mOverflowDestination);

      // overflow-entry
      prResponse->setAttribute(QUEUE_OVERFLOW_ENTRY_TAG, mOverflowEntry);

      // overflow-type
      prResponse->setAttribute(QUEUE_OVERFLOW_TYPE_TAG, mOverflowType);

      // answer-mode
      prResponse->setAttribute(QUEUE_ANSWER_MODE_TAG, mAnswerMode);

      // call-connect-scheme
      prResponse->setAttribute(QUEUE_CALL_CONNECT_SCHEME_TAG, mCallConnectScheme);

      // welcome-audio
      prResponse->setAttribute(QUEUE_WELCOME_AUDIO_TAG, mWelcomeAudio);

#if 0
      // barge-in
      prResponse->setAttribute(QUEUE_BARGE_IN_TAG, mBargeIn);
#endif

      // queue-audio
      prResponse->setAttribute(QUEUE_QUEUE_AUDIO_TAG, mQueueAudio);

      // background-audio
      prResponse->setAttribute(QUEUE_BACKGROUND_AUDIO_TAG, mBackgroundAudio);

      // queue-audio-interval
      prResponse->setAttribute(QUEUE_QUEUE_AUDIO_INTERVAL_TAG, mQueueAudioInterval);

      // call-termination-audio
      prResponse->setAttribute(QUEUE_CALL_TERMINATION_AUDIO_TAG, mCallTerminationAudio);

      // termination-tone-duration
      prResponse->setAttribute(QUEUE_TERMINATION_TONE_DURATION_TAG, mTerminationToneDuration);

      // agents-wrap-up-time
      prResponse->setAttribute(QUEUE_AGENTS_WRAP_UP_TIME_TAG, mAgentsWrapupTime);

      // agents-non-responsive-time
      prResponse->setAttribute(QUEUE_AGENTS_NON_RESPONSIVE_TIME_TAG, mAgentsNonResponsiveTime);

      // max-bounce-count
      prResponse->setAttribute(QUEUE_MAX_BOUNCE_COUNT_TAG, mMaxBounceCount);

      // acd-agent-list
      prResponse->setAttribute(QUEUE_ACD_AGENT_LIST_TAG, mAcdAgentListString);

      // acd-line-list
      prResponse->setAttribute(QUEUE_ACD_LINE_LIST_TAG, mAcdLineListString);

      // queue-depth
      prResponse->setAttribute(QUEUE_QUEUE_DEPTH_TAG, static_cast<int>(mUnroutedCallList.entries()));
   }
}

/* ============================ INQUIRY =================================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueue::compareTo
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

int ACDQueue::compareTo(UtlContainable const * pInVal) const
{
   int result ;

   if (pInVal->isInstanceOf(ACDQueue::TYPE)) {
      result = mUriString.compareTo(((ACDQueue*)pInVal)->getUriString());
   }
   else {
      result = -1;
   }

   return result;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueue::handleMessage
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

UtlBoolean ACDQueue::handleMessage(OsMsg& rMessage)
{
   ACDQueueMsg*     pMessage;
   ACDCall*         pCallRef;
   ACDCallRouteState::eRouteState state;
   ACDAgent*        pAgentRef;

//   osPrintf("ACDQueue::handleMessage - MsgType: %d, MsgSubType: %d\n", rMessage.getMsgType(), rMessage.getMsgSubType());
     OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDQueue::handleMessage MsgType: %d, MsgSubType: %d", rMessage.getMsgType(), rMessage.getMsgSubType());
   if (rMessage.getMsgType() == OsMsg::USER_START) {
      switch (rMessage.getMsgSubType()) {
         case ACDQueueMsg::ADD_CALL:
            pMessage = (ACDQueueMsg*)&rMessage;
            pCallRef = pMessage->getCallReference();
            if (mpAcdCallManager->verifyACDCallExists(pCallRef))
            {
               // Add the call to this Queue
               addCallMessage(pCallRef);
            }
            break;

         case ACDQueueMsg::REMOVE_CALL:
            pMessage = (ACDQueueMsg*)&rMessage;
            pCallRef = pMessage->getCallReference();
            if (mpAcdCallManager->verifyACDCallExists(pCallRef))
            {
               // Remove the call from this Queue
               removeCallMessage(pCallRef);
            }
            break;

         case ACDQueueMsg::CALL_CONNECTED:
            pMessage = (ACDQueueMsg*)&rMessage;
            pCallRef = pMessage->getCallReference();
            if (mpAcdCallManager->verifyACDCallExists(pCallRef))
            {
               // Notify the queue that the ACDCall is now connected
               callConnectedMessage(pCallRef);
            }
            break;

         case ACDQueueMsg::UPDATE_ROUTE_STATE:
            pMessage = (ACDQueueMsg*)&rMessage;
            pCallRef = pMessage->getCallReference();
            state    = pMessage->getCallRouteState();
            if (mpAcdCallManager->verifyACDCallExists(pCallRef))
            {
               // Update the call staus for this call
               updateRouteStateMessage(pCallRef, state);
            }
            break;

         case ACDQueueMsg::MAX_WAIT_TIME:
            pMessage = (ACDQueueMsg*)&rMessage;
            pCallRef = pMessage->getCallReference();
            if (mpAcdCallManager->verifyACDCallExists(pCallRef))
            {
               // Notify the queue that the ACDCall has waited long enouth
               queueMaxWaitTimeMessage(pCallRef);
            }
            break;

         case ACDQueueMsg::AGENT_AVAILABLE:
            pMessage = (ACDQueueMsg*)&rMessage;
            pAgentRef = pMessage->getAgentReference();
            // Delete this agent if it is marked
            if (TRUE == pAgentRef->getDelete()) {
               mpAcdAgentManager->deleteACDAgent(pAgentRef->getUriString()->data());
            }
            else {
            // Notify the queue that an ACDAgent has become available
               agentAvailableMessage(pAgentRef);
            }
            break;

         default:
            // Bad message
            OsSysLog::add(FAC_ACD, PRI_ERR, "ACDQueue::handleMessage - Received bad message");
            break;
      }

      return true;
   }
   else {
      // Otherwise, pass the message to the base for processing.
      OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDQueue::handleMessage MsgType - not USER_START");
      return false;
   }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueue::unroute
//
//  SYNOPSIS:    enqueue the call to the unrouted call list
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

void ACDQueue::unroute(ACDCall* pCallRef, bool priority)
{
   if (mUnroutedCallList.find(pCallRef) != NULL) {
      OsSysLog::add(FAC_ACD, PRI_ERR, "ACDQueue::unroute - ACDCall(%d) is already in unrouted call list",
                    pCallRef->getCallHandle());
      return ;
   }

   // Add the call to the unrouted call list
   if (priority) {
      // Priority calls go to the head of the queue
      OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDQueue::unroute - ACDCall(%d) is added to the beginning of the queue (priority)",
                    pCallRef->getCallHandle());
      mUnroutedCallList.insertAt(0, pCallRef);
   }
   else {
      // Others  go to the tail
      OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDQueue::unroute - ACDCall(%d) is added to the queue",
                    pCallRef->getCallHandle());
      mUnroutedCallList.append(pCallRef);
   }

   // Answer it, if need be
   if (mAnswerMode == NEVER) {
      return ;
   }

   if (pCallRef->getCurrentCallState() != ACDCall::CONNECTED) {
      pCallRef->answerCall(mWelcomeAudio, mBargeIn);
      OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDQueue::unroute - ACDCall(%d) is being answered",
                 pCallRef->getCallHandle());
      return ;
   }

   // Start the background audio playing
   pCallRef->playAudio(mQueueAudio, mQueueAudioInterval, mBackgroundAudio);
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDQueue::unroute - ACDCall(%d) is already being answered and added to the queue",
              pCallRef->getCallHandle());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueue::unableToRoute
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

void ACDQueue::unableToRoute(ACDCall* pCallRef, bool priority)
{
   // Clear old RNA history.  It starts clean again
   resetRingNoAnswerStates(pCallRef);

   // Before decide whether the call should be overflowed or not, first check
   // whether there is an overflow queue/entry being defined
   if (mOverflowQueue == NULL &&  mOverflowEntry == NULL) {
      if (anyAgentSignedIn()) {
         // Add the call to the unrouted call list
         unroute(pCallRef, priority) ;
      }
      else
      {
         OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDQueue::unableToRoute - no agents have signed in, drop ACDCall(%d)",
                       pCallRef->getCallHandle());
         pCallRef->dropCall(mTerminationToneDuration, mCallTerminationAudio);
      }
      return ;
   }

   // First check to see if adding this call will trigger an overflow condition
   // If max-queue-depth is set to -1, do not test
   if ((mMaxQueueDepth != -1) && (mUnroutedCallList.entries() >= static_cast<unsigned>(mMaxQueueDepth))) {
      // The queue is full, overflow the call
      OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDQueue::unableToRoute - no agent is available and the queue is full, overflow ACDCall(%d)",
                    pCallRef->getCallHandle());
      overflowCall(pCallRef);
   }
   else {
      // Check whether there are any agents signed in
      if (anyAgentSignedIn()) {
         // There is room for this call, add it to the unrouted call list
         unroute(pCallRef, priority);
      }
      else {
         OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDQueue::unableToRoute - no agents have signed in, overflow ACDCall(%d)",
                       pCallRef->getCallHandle());
         overflowCall(pCallRef);
      }
   }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueue::routeCall
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

void ACDQueue::routeCall(ACDCall* pCallRef)
{
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "%s::routeCall - ACDCall(%d)",
      mAcdSchemeString, pCallRef->getCallHandle());

   // Build up a list of available agents and test
   UtlSList targetAgentList;
   if (buildTargetAgentList(targetAgentList, pCallRef)) {
      // Agent(s) available, route
      if (mRoutingCallList.find(pCallRef) == NULL) {
         appendRoutingCall(pCallRef);
         pCallRef->routeRequest(targetAgentList, mCallConnectScheme, mMaxRingDelay);
      }
      else {
            OsSysLog::add(FAC_ACD, PRI_ERR, "%s::routeCall - ACDCall(%d) is already in routing call list",
                          mAcdSchemeString, pCallRef->getCallHandle());
      }
   }
   else {
      // Remove the call out of the routing list if exist
      removeRoutingCall(pCallRef);

      // enqueue it
      unableToRoute(pCallRef, false);
   }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueue::addCallMessage
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

void ACDQueue::addCallMessage(ACDCall* pCallRef)
{
   bool agentsSignedIn = anyAgentSignedIn();

   // Hangup on the call if there is no way to handle it.
   if (mOverflowQueue == NULL &&  mOverflowEntry == NULL) {
      // Check whether there is any agents signed in
      if (agentsSignedIn != true) {
         OsSysLog::add(FAC_ACD, gACD_DEBUG, "%s::addCallMessage - no agents have signed in and no overflow so drop ACDCall(%d), state %d",
                       mAcdSchemeString, pCallRef->getCallHandle(), pCallRef->getCurrentCallState());
         // Just reject the call at this stage
         //sipxCallReject(pCallRef->getCallHandle());
         pCallRef->dropCall(mTerminationToneDuration, mCallTerminationAudio);
         return ;
      }
   }

   // Inform the ACDCall that this queue is in control of it
   pCallRef->setManagingQueue(this, mMaxWaitTime);
   pCallRef->clearRoutePendingAnswer();


   // Clear old RNA history.  It starts clean again
   resetRingNoAnswerStates(pCallRef);

   // Clear route state back to IDLE
   pCallRef->resetRouteState();

   // See if the call needs to be answered first before attempting to route
   if (mAnswerMode == IMMEDIATE && pCallRef->getCurrentCallState() != ACDCall::CONNECTED) {
      // Mark this call to be routed once answered
      pCallRef->setRoutePendingAnswer();
      pCallRef->answerCall(mWelcomeAudio, mBargeIn);

      OsSysLog::add(FAC_ACD, gACD_DEBUG, "%s::addCallMessage - ACDCall(%d) is being answered",
                    mAcdSchemeString, pCallRef->getCallHandle());

      return ;
   }
   else {
      OsSysLog::add(FAC_ACD, gACD_DEBUG, "%s::addCallMessage - ACDCall(%d) is being routed without being answered",
                    mAcdSchemeString, pCallRef->getCallHandle());
   }
   routeCall(pCallRef);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueue::overflowCall
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

void ACDQueue::overflowCall(ACDCall* pCallRef)
{
   ACDCall* pOverflowCall = dynamic_cast<ACDCall*>(mUnroutedCallList.first());
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDQueue::overflowCall - ACDCall(%d) entering overflowCall routine.",
                       pCallRef->getCallHandle());

   // Determine how to handle the overflow condition
   if ( (mOverflowQueue!= NULL || mOverflowEntry != NULL) && (mMaxQueueDepth != 0) && mFifoOverflow && (pOverflowCall != NULL)) {

      OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDQueue::overflowCall - ACDCall(%d) entering overflowCall routine in the overflow queue %s",pCallRef->getCallHandle(), mOverflowQueue.data() ? mOverflowQueue.data() : NULL);

      // The queue is configured as a FIFO, so overflow the oldest call and add this one
      // Send the call to the configured overflow queue or overflow entry

      // Remove the oldest call from the list for overflow
      pOverflowCall = dynamic_cast<ACDCall*>(mUnroutedCallList.get());

      ACDQueue* pOverflowQueue;
      pOverflowQueue = mpAcdQueueManager->getAcdQueueReference(mOverflowQueue);
      if (pOverflowQueue != NULL) {

         pOverflowQueue->addCall(pOverflowCall);

         // Now send the call to the ACDQueue assigned to this line.
         ACDRtRecord* pACDRtRec;
         if (NULL != (pACDRtRec = mpAcdQueueManager->getAcdServer()->getAcdRtRecord())) {
            pACDRtRec->appendCallEvent(ACDRtRecord::ENTER_QUEUE, mOverflowQueue.data(), pOverflowCall);
         }

         OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDQueue::overflowCall - ACDCall(%d) is being added to the overflow queue %s",
                       pOverflowCall->getCallHandle(), mOverflowQueue.data());
      }
      else if (mOverflowEntry != NULL) {
         // Could not find the overflow queue, but there is overflowEntry defined, so transfer the call
         transferOverflowCall(pOverflowCall);
      }
      else {
         // Could not find the overflow queue, drop the call
         pCallRef->dropCall(mTerminationToneDuration, mCallTerminationAudio);
         OsSysLog::add(FAC_ACD, PRI_ERR, "ACDQueue::overflowCall - could not find the overflow queue so drop ACDCall(%d)", pCallRef->getCallHandle());
      }

      // Now we add this call to the unrouted list
      unroute(pCallRef, false);
   }
   else {
      // The max-queue-depth == 0 or queue is configured as a LIFO, so this call gets overflowed
      if (mOverflowQueue == NULL) {
         if (mOverflowEntry!= NULL) {
            transferOverflowCall(pCallRef);
         } else {
            // We should never come to here, but just in case
            // No overflow queue configured, drop the call
            pCallRef->dropCall(mTerminationToneDuration, mCallTerminationAudio);
            OsSysLog::add(FAC_ACD, PRI_ERR, "ACDQueue::overflowCall - no overflow is set up so drop ACDCall(%d)",
                       pCallRef->getCallHandle());
         }
      }
      else {
         // Send the call to the configured overflow queue
         ACDQueue* pOverflowQueue;
         pOverflowQueue = mpAcdQueueManager->getAcdQueueReference(mOverflowQueue);
         OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDQueue::overflowCall - sending ACDCall(%d) to overflow queue",
                       pCallRef->getCallHandle());
         if (pOverflowQueue != NULL) {
            pOverflowQueue->addCall(pCallRef);

            // Now send the call to the ACDQueue assigned to this line.
            ACDRtRecord* pACDRtRec;
            if (NULL != (pACDRtRec = mpAcdQueueManager->getAcdServer()->getAcdRtRecord())) {
               pACDRtRec->appendCallEvent(ACDRtRecord::ENTER_QUEUE, mOverflowQueue.data(), pCallRef);
            }
         }
         else if (mOverflowEntry != NULL) {
            transferOverflowCall(pCallRef);
         }
         else {
            // Could not find the overflow queue, drop the call
            pCallRef->dropCall(mTerminationToneDuration, mCallTerminationAudio);
            OsSysLog::add(FAC_ACD, PRI_ERR, "ACDQueue::overflowCall - ACDCall(%d) has to be dropped due to bad overflow queue",
                          pCallRef->getCallHandle());
         }
      }
   }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueue::transferOverflowCall
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

void ACDQueue::transferOverflowCall(ACDCall* pCallRef)
{
   SIPX_RESULT rc ;

   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDQueue::transferOverflowCall - ACDCall(%d) to %s",
                 pCallRef->getCallHandle(),mOverflowEntry.data());

   if (pCallRef->getCurrentCallState() != ACDCall::CONNECTED) {
      OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDQueue::transferOverflowCall - ACDCall(%d) needs to be answered first",
                 pCallRef->getCallHandle());
      // Clear route state back to IDLE
      pCallRef->resetRouteState();

      // Mark this call to be xfered once answered
      pCallRef->setXferPendingAnswer();

      // Answer the call.
      pCallRef->answerCall(mWelcomeAudio, mBargeIn);

      // Will get back to here once call is connected due to XferPendingAnswer
   } else {
      // now transfer the call
      rc = sipxCallBlindTransfer(pCallRef->getCallHandle(),mOverflowEntry.data());
      mUnroutedCallList.remove(pCallRef);
      removeRoutingCall(pCallRef);

      if ( rc!= SIPX_RESULT_SUCCESS) {
         // failed to transfer the call to overflow entry, drop the call
         pCallRef->dropCall(mTerminationToneDuration, mCallTerminationAudio);
         OsSysLog::add(FAC_ACD, PRI_ERR, "ACDQueue::transferOverflowCall - could not transfer the call to desinated address so drop ACDCall(%d)", pCallRef->getCallHandle());

      }
   }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueue::removeCallMessage
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

void ACDQueue::removeCallMessage(ACDCall* pCallRef)
{
   // FUTURE
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDQueue::removeCallMessage - ACDCall(%d)",
                 pCallRef->getCallHandle());
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueue::callConnectedMessage
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

void ACDQueue::callConnectedMessage(ACDCall* pCallRef)
{
   // See if this call is pending a transfer
   if (pCallRef->getXferPendingAnswer() == TRUE) {
      OsSysLog::add(FAC_ACD, gACD_DEBUG,
         "%s::callConnectedMessage - ACDCall(%d) transfer pending",
             mAcdSchemeString, pCallRef->getCallHandle());
      transferOverflowCall(pCallRef) ;
      return ;
   }

   // See if this call is pending a route
   if (pCallRef->routePendingAnswer() == FALSE) {
      // Now that the call has connected, give the caller something to listen to
      pCallRef->playAudio(mQueueAudio, mQueueAudioInterval, mBackgroundAudio);
      OsSysLog::add(FAC_ACD, gACD_DEBUG, "%s::callConnectedMessage - ACDCall(%d) has already connected and playing queue audio ...",
         mAcdSchemeString, pCallRef->getCallHandle());
      return ;
   }

   pCallRef->clearRoutePendingAnswer();

   routeCall(pCallRef);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueue::agentAvailableMessage
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

void ACDQueue::agentAvailableMessage(ACDAgent* pAgentRef)
{
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "%s::agentAvailableMessage - ACDAgent(%s) is available to take a call",
                 mAcdSchemeString, pAgentRef->getUriString()->data());

   // Pop the oldest call (if any) off the UnroutedCallList
   ACDCall* pCallRef = dynamic_cast<ACDCall*>(mUnroutedCallList.get());
   if (pCallRef == NULL) {
      OsSysLog::add(FAC_ACD, gACD_DEBUG, "%s::agentAvailableMessage - no call is in the queue at this moment for ACDAgent(%s).",
                    mAcdSchemeString, pAgentRef->getUriString()->data());
      return ;
   }

   // Check that the call isn't already in the RoutingCallList
   if (mRoutingCallList.find(pCallRef) != NULL ) {
      OsSysLog::add(FAC_ACD, PRI_ERR, "%s::agentAvailableMessage - The call %s is already in the routing list",
                    mAcdSchemeString, pCallRef->getCallIdentity());
      return ;
   }

   // Try to get the agent if he is available
   if (pAgentRef->isAvailable(true)) {
      // Append the call to the end of the RoutingCallList
      appendRoutingCall(pCallRef);
      // Send it on it's way
      OsSysLog::add(FAC_ACD, gACD_DEBUG, "%s::agentAvailableMessage - The call %s is being routed to ACDAgent(%s)",
                 mAcdSchemeString, pCallRef->getCallIdentity(), pAgentRef->getUriString()->data());

      pCallRef->routeRequest(pAgentRef, mCallConnectScheme, mMaxRingDelay);
      return ;
   }

   OsSysLog::add(FAC_ACD, gACD_DEBUG, "%s::agentAvailableMessage - ACDAgent(%s) is no longer available to take the call",
     mAcdSchemeString, pAgentRef->getUriString()->data());
   // Put the call back on the end of the queue so we don't forget it
   mUnroutedCallList.insertAt(0, pCallRef);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueue::updateRouteStateFailed
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

void ACDQueue::updateRouteStateFailed(ACDCall* pCallRef)
{
   // The previous route attempt failed.  See if there are more agent candidates available
   pCallRef->clearRoutePendingAnswer();

#ifdef CML
   // CML:
   // If this FAILED event was generated because of a call pickup request...
   pACDAgent = pCallRef->isBeingPickedUp(bTemp);
   if (bTemp) {
      if(pACDAgent) {
         // We need to reinit a routing request that only contains the agent that requested the call pickup
         UtlSList targetAgentList;
         targetAgentList.append(pACDAgent);

         // Reset the call pickup flag so that normal processing is resumed
         pCallRef->clearBeingPickedUp();

         // Route the call to the agent that picked it up
         pCallRef->routeRequest(targetAgentList,mCallConnectScheme,mMaxRingDelay);

         OsSysLog::add(FAC_ACD, gACD_DEBUG, "%s::updateRouteStateFailed - ACDCall(%d) is being picked up by %s",
             mAcdSchemeString, pCallRef->getCallHandle(),pACDAgent->getUriString()->data());
      }
      else {
         OsSysLog::add(FAC_ACD, gACD_DEBUG, "%s::updateRouteStateFailed - ACDCall(%d) could not be picked up because reference to ACDAgent is NULL.",
            mAcdSchemeString, pCallRef->getCallHandle());
      }
      return ;
   }
#endif

   UtlSList targetAgentList;
   if (buildTargetAgentList(targetAgentList, pCallRef)) {
      OsSysLog::add(FAC_ACD, gACD_DEBUG, "%s::updateRouteStateFailed - ACDCall(%d) is being routed again",
                    mAcdSchemeString, pCallRef->getCallHandle());
      pCallRef->routeRequest(targetAgentList, mCallConnectScheme, mMaxRingDelay);
      return ;
   }

   OsSysLog::add(FAC_ACD, gACD_DEBUG,
            "%s::updateRouteStateFailed - ACDCall(%d) no agents available on reroute",
            mAcdSchemeString, pCallRef->getCallHandle());

   // There are no available agents to handle this call
   removeRoutingCall(pCallRef);
   // requeue it, with priority
   unableToRoute(pCallRef, true) ;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueue::updateRouteStateAborted
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

void ACDQueue::updateRouteStateAborted(ACDCall* pCallRef)
{
   mUnroutedCallList.remove(pCallRef);
   removeRoutingCall(pCallRef);

   // See if an overflow queue has been configured
   if (mOverflowQueue == NULL) {

      // See if an overflow queue/entry has been configured
      if(mOverflowEntry!= NULL) {
         transferOverflowCall(pCallRef);
         return;
      }
      // No overflow queue configured, drop the call
      OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDQueue::updateRouteStateAborted - Dropping call %s",
                    pCallRef->getCallIdentity());

      pCallRef->dropCall(mTerminationToneDuration, mCallTerminationAudio);
   }
   else {
      OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDQueue::updateRouteStateAborted - Sending call %s to: %s",
                    pCallRef->getCallIdentity(), mOverflowQueue.data());

      // Send the call to the configured overflow queue
      ACDQueue* pOverflowQueue;
      pOverflowQueue = mpAcdQueueManager->getAcdQueueReference(mOverflowQueue);
      if (pOverflowQueue != NULL) {
         ACDRtRecord* pACDRtRec;
         if (NULL != (pACDRtRec = mpAcdQueueManager->getAcdServer()->getAcdRtRecord()))
         {
            pACDRtRec->appendCallEvent(ACDRtRecord::ENTER_QUEUE, mOverflowQueue.data(), pCallRef);
         }
         pOverflowQueue->addCall(pCallRef);
      }
      else {
         // Could not find the overflow queue, drop the call
         pCallRef->dropCall(mTerminationToneDuration, mCallTerminationAudio);
      }
   }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueue::updateRouteStateMessage
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

void ACDQueue::updateRouteStateMessage(ACDCall* pCallRef, ACDCallRouteState::eRouteState state)
{
   switch (state) {
      case ACDCallRouteState::ROUTED:
         OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDQueue::updateRouteStateMessage - Call(%s) new state: ROUTED\n", pCallRef->getCallIdentity());
         osPrintf("ACDQueue::updateRouteStateMessage - Call(%s) new state: ROUTED\n", pCallRef->getCallIdentity());

         // Record the real-time event here
         ACDRtRecord* pACDRtRec;
         if (NULL != (pACDRtRec = mpAcdQueueManager->getAcdServer()->getAcdRtRecord())) {
            pACDRtRec->appendCallEvent(ACDRtRecord::PICK_UP, *(getUriString()), pCallRef, TRUE);
         }

         // Reset the calls QueueMaxWaitTimer
         pCallRef->resetQueueMaxWaitTimer();

         // The call has been routed, remove it from this queue
         removeRoutingCall(pCallRef);

         break;

      case ACDCallRouteState::FAILED:
         OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDQueue::updateRouteStateMessage - Call(%s) new state: FAILED\n", pCallRef->getCallIdentity());
         osPrintf("ACDQueue::updateRouteStateMessage - Call(%s) new state: FAILED\n", pCallRef->getCallIdentity());
         updateRouteStateFailed(pCallRef);
         break;

      case ACDCallRouteState::ABORTED:
         OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDQueue::updateRouteStateMessage - Call(%s) new state: ABORTED\n", pCallRef->getCallIdentity());
         updateRouteStateAborted(pCallRef);
         break;

      case ACDCallRouteState::TERMINATED:
         OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDQueue::updateRouteStateMessage - Call(%s) new state: TERMINATED\n", pCallRef->getCallIdentity());

         // Record the real-time event here
         ACDRtRecord* pACDRtRecord;
         if (NULL !=(pACDRtRecord = mpAcdQueueManager->getAcdServer()->getAcdRtRecord())) {
            pACDRtRecord->appendCallEvent(ACDRtRecord::TERMINATE, *(getUriString()), pCallRef);
         }

         // Reset the calls QueueMaxWaitTimer
         pCallRef->resetQueueMaxWaitTimer();

         // The call has been terminated, remove it from this queue
         mUnroutedCallList.remove(pCallRef);
         removeRoutingCall(pCallRef);

         // Tell the Call Manager to destroy the call
         mpAcdCallManager->destroyACDCall(pCallRef);
         break;

      case ACDCallRouteState::DISCOVERED:
         OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDQueue::updateRouteStateMessage - Call(%s) new state: DISCOVERED\n", pCallRef->getCallIdentity());

         if (mAcdScheme == RING_ALL) {
            // In a RING_ALL scheme, we need to remove the call from the routing list as soon as possible
            // so that the next call in the unrouted queue can be routed to all the agents immediately
            removeRoutingCall(pCallRef);
         }

         break;

      case ACDCallRouteState::IDLE:
      case ACDCallRouteState::TRYING:
      case ACDCallRouteState::CONNECTING:
      case ACDCallRouteState::ON_HOLD:
      case ACDCallRouteState::ROUTE_STATE_UNDEFINED:
         break;
   }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueue::queueMaxWaitTimeMessage
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

void ACDQueue::queueMaxWaitTimeMessage(ACDCall* pCallRef)
{
   // The max-wait-time has been reached, remove it from this queue

   // Tell the ACDCall Route FSM to ABORT
   pCallRef->abortRouteRequest();

   // The call will then send an ABORTED message when it is done.
   // Then ACDQueue::updateRouteStateAborted(pCallRef) will be called
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueue::buildTargetAgentList
//
//  SYNOPSIS:
//
//  DESCRIPTION: Based upon the queue strategy and agent availability, this method will build a
//               list of ACDAgent object candidates
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#if 0
bool ACDQueue::buildTargetAgentList(UtlSList& rTargetAgentList, ACDCall* pCallRef)
{
  This is pure virtual.  No default implementation.
}
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueue::releaseTargetAgentList
//
//  SYNOPSIS:
//
//  DESCRIPTION: This method releases all the agents built up in target agent list
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////
void ACDQueue::releaseTargetAgentList(UtlSList& rTargetAgentList)
{
   ACDAgent* pAgent;

   UtlSListIterator listIterator(rTargetAgentList);

   // Iterate through the ACDAgent list and mark them available
   while ((pAgent = dynamic_cast<ACDAgent*>(listIterator())) != NULL) {
      // Mark the agent as being free
      pAgent->setFree();
   }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueue::anyAgentSignedIn
//
//  SYNOPSIS:
//
//  DESCRIPTION: This method checks anybody has signed in the queue or not
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////
bool ACDQueue::anyAgentSignedIn(bool checkAlwaysAvailable)
{
   bool signedIn = false;

   ACDAgent* pAgent;

   pAgent = dynamic_cast<ACDAgent*>(mAcdAgentList.at(0));
   if (pAgent == NULL) {
      // The ACDAgent list is empty, try rebuilding
      buildACDAgentList();
   }

   UtlSListIterator listIterator(mAcdAgentList);

   // Iterate through the ACDAgent list and mark them available
   while ((pAgent = dynamic_cast<ACDAgent*>(listIterator())) != NULL) {
      if (checkAlwaysAvailable && pAgent->alwaysAvailable()) {
         signedIn = true;
         break;
      }

      if (pAgent->getState(LinePresenceBase::SIGNED_IN)) {
         signedIn = true;
         break;
      }
   }

   return signedIn;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueue::adjustTimers
//
//  SYNOPSIS:
//
//  DESCRIPTION: This method adjust various timer values according to the queue setting
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////
void ACDQueue::adjustTimers()
{
   // Disable the overflow queue if max wait time is 0
   if (mMaxWaitTime == 0) {
      mOverflowQueue = NULL;
   }
   // Disable the max wait time if there is no overflow queue/entry is defined
   if (mOverflowQueue == NULL &&  mOverflowEntry == NULL ) {
      mMaxWaitTime = 0;

      if (mAcdScheme == RING_ALL) {
         mMaxRingDelay = 0;
      }
      else {
         // Set the minimum to MIN_TIMEOUT_VALUE
         if (mMaxRingDelay >= 0 && mMaxRingDelay < MIN_TIMEOUT_VALUE) {
            mMaxRingDelay = MIN_TIMEOUT_VALUE;
         }
      }
   }
   else {
      // mMaxRingDelay cannot be longer than mMaxWaitTime. So set mMaxRingDelay
      // to be zero, i.e., disable mpRingTimeoutTimer in ACDCall.
      if (mMaxRingDelay >= mMaxWaitTime) {
         mMaxRingDelay = 0;
      }
      else {
         // Set the minimum to MIN_TIMEOUT_VALUE
         if (mMaxRingDelay >= 0 && mMaxRingDelay < MIN_TIMEOUT_VALUE) {
            mMaxRingDelay = MIN_TIMEOUT_VALUE;
         }

         // Set the minimum to MIN_TIMEOUT_VALUE
         if (mMaxWaitTime > 0 && mMaxWaitTime < MIN_TIMEOUT_VALUE) {
            mMaxWaitTime = MIN_TIMEOUT_VALUE;
         }
      }
   }
}


/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueue::resetRingNoAnswerStates
//
//  SYNOPSIS:
//
//  DESCRIPTION: This method "resets" the RingNoAnswerState for all of the Agents in the list
//               "Reset" here means setting the value to false.
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////
void ACDQueue::resetRingNoAnswerStates(ACDCall* pCallRef)
{
   // Find the appropriate RnaList from the map table.
   UtlSList* rnaList = dynamic_cast<UtlSList*>(mCallToRnaListMap.findValue( pCallRef ));

   OsSysLog::add(FAC_ACD, PRI_DEBUG, "ACDQueue::resetRingNoAnswerStates - resetting RNA for ACDCall %p : RnaList %p", pCallRef, rnaList );
   if ( rnaList )
   {
       UtlSListIterator listIterator(*rnaList);
       UtlBool* rnaState;

       rnaState = dynamic_cast<UtlBool*>(rnaList->at(0));

       while ((rnaState = dynamic_cast<UtlBool*>(listIterator())) != NULL)
       {
           rnaState->setValue( false );
       }
   }
   else
   {
       // Error/Assert/Warn
   }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueue::setRingNoAnswerState
//
//  SYNOPSIS:
//
//  DESCRIPTION: This method sets the boolean RingNoAnswer state of a particular Agent.
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////
void ACDQueue::setRingNoAnswerState(ACDCall* pCallRef, ACDAgent* pAgentRef, bool state)
{
   // First find the position of the Agent within the mAcdAgentList
   ssize_t agentPos;
   agentPos = mAcdAgentList.index( pAgentRef );
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDQueue::setRingNoAnswerState - agent %s pos %zd state %d", pAgentRef->getUriString()->data(), agentPos, state );

   if ( agentPos!=UTL_NOT_FOUND )
   {
      // Find the appropriate RnaList from the map table.
      UtlSList* rnaList = dynamic_cast<UtlSList*>(mCallToRnaListMap.findValue( pCallRef ));

      if ( rnaList )
      {
         UtlBool* rnaState = dynamic_cast<UtlBool*>(rnaList->at( agentPos ));
         if ( rnaState )
         {
            rnaState->setValue( state );
         }
      }
      else
      {
         // Probably used a call to mRoutingCallList.append() rather than ACDQeueue::appendRoutingCall() !!!
         OsSysLog::add(FAC_ACD, PRI_ERR, "ACDQueue::setRingNoAnswerState - CallRef %p not found", pCallRef);
      }
   }
   else
   {
      // Error/Assert/Warn
      OsSysLog::add(FAC_ACD, PRI_ERR, "ACDQueue::setRingNoAnswerState - Invalid Agent Reference %p", pAgentRef);
   }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueue::getRingNoAnswerState
//
//  SYNOPSIS:
//
//  DESCRIPTION: This method gets the boolean RingNoAnswer state of a particular Agent.
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////
bool ACDQueue::getRingNoAnswerState(ACDCall* pCallRef, ACDAgent* pAgentRef)
{
   // First find the position of the Agent within the mAcdAgentList
   ssize_t agentPos;
   agentPos = mAcdAgentList.index( pAgentRef );
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDQueue::getRingNoAnswerState - agent %s pos %zd", pAgentRef->getUriString()->data(), agentPos);

   if ( agentPos!=UTL_NOT_FOUND )
   {
      // Find the appropriate RnaList from the map table.
      UtlSList* rnaList = dynamic_cast<UtlSList*>(mCallToRnaListMap.findValue( pCallRef ));
      if (rnaList)
      {
         UtlBool* rnaState = dynamic_cast<UtlBool*>(rnaList->at( agentPos ));
         if ( rnaState )
         {
            return rnaState->getValue();
         }
      }
      else
      {
         // Error/Assert/Warn
         // Probably used a call to mRoutingCallList.append() rather than ACDQueue::appendRoutingCall() !!!
         // or call to buildTargetAgentList() was called before it had a chance to call appendRoutingCall()
         // In this case, we want to return "false" state in order to enable this Agent regardless.
         return false;
      }
   }

   OsSysLog::add(FAC_ACD, PRI_ERR, "ACDQueue::getRingNoAnswerState - Invalid Agent Reference %p", pAgentRef);
   return true; // We want to return a "true" state in the default case in order to "disable" this bad Agent reference.
}

void ACDQueue::appendRoutingCall(ACDCall* pCallRef)
{
   // Sanity Check. Make sure we don't already have this pCallRef entry in the Mapping
   UtlSList* rnaList = dynamic_cast<UtlSList*>(mCallToRnaListMap.findValue( pCallRef ));
   if ( rnaList )
   {
      OsSysLog::add(FAC_ACD, PRI_ERR, "ACDQueue::appendRoutingCall - ACDCall %p already exists in mCallToRnaListMap", pCallRef);
      return;
   }

   // Add it to the Routing call list.
   mRoutingCallList.append(pCallRef);

   // Create a RNA list and a Call-RNAList map entry
   rnaList = new UtlSList();

   mCallToRnaListMap.insertKeyAndValue( pCallRef, rnaList );

   //Determine how many Agents we have and create an equal number of UtlBool variables in the RingNoAnswer list.
   size_t numOfAgents = mAcdAgentList.entries();
   for ( size_t loop=0; loop<numOfAgents; loop++ )
   {
      UtlBool* rnaState = new UtlBool(false);
      rnaList->append( rnaState );
   }
   OsSysLog::add(FAC_ACD, PRI_DEBUG, "ACDQueue::appendRoutingCall - Added entry into mCallToRnaListMap for ACDCall %p : RnaList %p : size %zu", pCallRef, rnaList, numOfAgents);
}

void ACDQueue::removeRoutingCall(ACDCall* pCallRef)
{
   // Remove it from the Routing call list.
   mRoutingCallList.remove(pCallRef);

   // Find the Call-RNAList map entry and remove it as well.
   UtlSList* rnaList = dynamic_cast<UtlSList*>(mCallToRnaListMap.findValue( pCallRef ));
   if ( rnaList )
   {
      OsSysLog::add(FAC_ACD, PRI_DEBUG, "ACDQueue::removeRoutingCall - Removed entry into mCallToRnaListMap for ACDCall %p : RnaList %p", pCallRef, rnaList);
      mCallToRnaListMap.removeReference( pCallRef );
      rnaList->destroyAll();
      delete rnaList;
   }
   else
   {
      // Probably used a call to mRoutingCallList.append() rather than ACDQueue::appendRoutingCall() !!!
      OsSysLog::add(FAC_ACD, PRI_DEBUG, "ACDQueue::removeRoutingCall - No entry in mCallToRnaListMap for ACDCall %p", pCallRef);
   }

}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueue::checkAgentAvailable
//
//  SYNOPSIS:
//
//  DESCRIPTION: This method returns if there is any agent available in this and the
//               overflow queues.
//
//  RETURNS:     bool.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

bool ACDQueue::checkAgentAvailable()
{
   ACDQueue* pDestinationQueue = this;

   while (pDestinationQueue) {
      if (pDestinationQueue->anyAgentSignedIn(true)){
        return true;
      }
      if (pDestinationQueue->mOverflowQueue){
         pDestinationQueue = mpAcdQueueManager->getAcdQueueReference(pDestinationQueue->mOverflowQueue);
      }
      else {
         pDestinationQueue = NULL;
      }
   }

   return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueue::checkOverflowEntryAvailable
//
//  SYNOPSIS:
//
//  DESCRIPTION: This method returns true if overflowEntry or overflow to huntGroup is configured in
//               this and the overflow queues.
//
//  RETURNS:     bool.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

bool ACDQueue::checkOverflowEntryAvailable()
{
   ACDQueue* pDestinationQueue = this;

   while (pDestinationQueue) {
      if (pDestinationQueue->isOverflowEntryAvailable()){
        return true;
      }
      if (pDestinationQueue->mOverflowQueue){
         pDestinationQueue = mpAcdQueueManager->getAcdQueueReference(pDestinationQueue->mOverflowQueue);
      }
      else {
         pDestinationQueue = NULL;
      }
   }

   return false;
}
