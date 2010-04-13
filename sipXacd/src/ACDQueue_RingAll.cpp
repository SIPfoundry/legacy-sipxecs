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
#include "ACDQueue_RingAll.h"
#include "ACDServer.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
extern OsSysLogPriority gACD_DEBUG;

// CONSTANTS
const int MIN_TIMEOUT_VALUE = 5;


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueue_RingAll::ACDQueue_RingAll
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

ACDQueue_RingAll::ACDQueue_RingAll(ACDQueueManager* pAcdQueueManager,
                   const char*      pQueueUriString,
                   const char*      pName,
                   int              acdScheme,
                   int              maxRingDelay,
                   int              maxQueueDepth,
                   int              maxWaitTime,
                   bool             fifoOverflow,
                   const char*      pOverflowQueue,
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
: ACDQueue(pAcdQueueManager,
                   pQueueUriString,
                   pName,
                   acdScheme,
                   maxRingDelay,
                   maxQueueDepth,
                   maxWaitTime,
                   fifoOverflow,
                   pOverflowQueue,
                   pOverflowEntry,
                   pOverflowType,
                   answerMode,
                   callConnectScheme,
                   pWelcomeAudio,
                   bargeIn,
                   pQueueAudio,
                   pBackgroundAudio,
                   queueAudioInterval,
                   pCallTerminationAudio,
                   terminationToneDuration,
                   agentsWrapupTime,
                   agentsNonResponsiveTime,
                   maxBounceCount,
                   pAcdAgentList,
                   pAcdLineList)
{
   mAcdSchemeString = "ACDQueue_RingAll" ;

   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDQueue_RingAll::ACDQueue_RingAll[%s] - MaxRingDelay = %d, MaxWaitTime = %d, Overflow Queue = %s",
                 mUriString.data(), mMaxRingDelay, mMaxWaitTime, mOverflowQueue.data());
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueue_RingAll::~ACDQueue_RingAll
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

ACDQueue_RingAll::~ACDQueue_RingAll()
{
}

/* ============================ MANIPULATORS ============================== */
/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueue_RingAll::routeCall
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

void ACDQueue_RingAll::routeCall(ACDCall* pCallRef)
{
   // There is only 1 call allowed in the active routing list.
   // If a second is being added, just queue/overflow it.
   if (mRoutingCallList.isEmpty()) {
      ACDQueue::routeCall(pCallRef) ;
   }
   else {
      OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDQueue_RingAll::addCallMessage - There is already a call in active routing list for RING_ALL so ACDCall(%d) is being queued",
                          pCallRef->getCallHandle());
      unableToRoute(pCallRef, false);
   }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueue_RingAll::agentAvailableMessage
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

void ACDQueue_RingAll::agentAvailableMessage(ACDAgent* pAgentRef)
{
   // An ACDAgent has indicated that it is available to take calls
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "%s::agentAvailableMessage - ACDAgent(%s) is available to take the call",
                 mAcdSchemeString, pAgentRef->getUriString()->data());

   if (mRoutingCallList.isEmpty() == TRUE) {
      ACDCall* pCallRef = dynamic_cast<ACDCall*>(mUnroutedCallList.get());

      if (pCallRef != NULL) {
         // Check if he is still available, and grab him if so.
         if (pAgentRef->isAvailable(true) == false) {
            OsSysLog::add(FAC_ACD, gACD_DEBUG, "%s::agentAvailableMessage - ACDAgent(%s) is not really  available to take the call",
                       mAcdSchemeString, pAgentRef->getUriString()->data());
            return ;
         }
         // Append to the end of the RoutingCallList
         appendRoutingCall(pCallRef);
         // Send it on it's way
         OsSysLog::add(FAC_ACD, gACD_DEBUG, "%s::agentAvailableMessage - The call %s is being routed to ACDAgent(%s) in RING_ALL",
                 mAcdSchemeString, pCallRef->getCallIdentity(), pAgentRef->getUriString()->data());

         pCallRef->routeRequest(pAgentRef, mCallConnectScheme, mMaxRingDelay);
         return ;
      }
      OsSysLog::add(FAC_ACD, gACD_DEBUG, "%s::agentAvailableMessage - no call is in the queue at this moment for ACDAgent(%s) in RING_ALL.",
                    mAcdSchemeString, pAgentRef->getUriString()->data());
   }
   else
   {
      ACDCall* pCallRef = dynamic_cast<ACDCall*>(mRoutingCallList.first());

      // Signal the call to add this agent to it's route attempt
      if (pCallRef != NULL) {
          // But not if he already RNA'd/reject this call!
          if (getRingNoAnswerState(pCallRef, pAgentRef) == true) {
             OsSysLog::add(FAC_ACD, gACD_DEBUG,
                "%s::agentAvailableMessage -"
                "agent(%s) has already RNA/rejected call(%s)",
                mAcdSchemeString,
                pAgentRef->getUriString()->data(), pCallRef->getCallIdentity());
             return;
          }

         // No need to check if he is still available, routeRequestAddAgent
         // will do that when the time comes.  If we grab him here,
         // it will fail there!
         OsSysLog::add(FAC_ACD, gACD_DEBUG, "%s::agentAvailableMessage - The call %s is also being routed to ACDAgent(%s) in RING_ALL",
                       mAcdSchemeString, pCallRef->getCallIdentity(), pAgentRef->getUriString()->data());
         pCallRef->routeRequestAddAgent(pAgentRef);
         return ;
      }
      OsSysLog::add(FAC_ACD, gACD_DEBUG, "%s::agentAvailableMessage - no call is in the routing list at this moment for ACDAgent(%s).",
                    mAcdSchemeString, pAgentRef->getUriString()->data());
   }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueue_RingAll::buildTargetAgentList
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

bool ACDQueue_RingAll::buildTargetAgentList(UtlSList& rTargetAgentList, ACDCall* pCallRef)
{
   ACDAgent* pAgent;
   bool foundAgent = false;

   // Always rebuild the list to get the agents added on the fly
   buildACDAgentList();

   UtlSListIterator listIterator(mAcdAgentList);

   // Iterate through the ACDAgent list, and build a list of all
   // available agents
   while((pAgent = dynamic_cast<ACDAgent*>(listIterator())) != NULL) {
      if (pAgent->isAvailable(true)) {
         foundAgent = true ;
         rTargetAgentList.append(pAgent);
         OsSysLog::add(FAC_ACD, gACD_DEBUG, "%s::buildTargetAgentList - agent(%s) is added to the target list",
                       mAcdSchemeString, pAgent->getUriString()->data());
      }
   }

   return foundAgent;
}



/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
