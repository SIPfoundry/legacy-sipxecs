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
#include "ACDQueue_Linear.h"
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
//  NAME:        ACDQueue_Linear::ACDQueue_Linear
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

ACDQueue_Linear::ACDQueue_Linear(ACDQueueManager* pAcdQueueManager,
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
   mAcdSchemeString = "ACDQueue_Linear";
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDQueue_Linear::ACDQueue_Linear[%s] - MaxRingDelay = %d, MaxWaitTime = %d, Overflow Queue = %s",
                 mUriString.data(), mMaxRingDelay, mMaxWaitTime, mOverflowQueue.data());
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueue_Linear::~ACDQueue_Linear
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

ACDQueue_Linear::~ACDQueue_Linear()
{
}

/* ============================ MANIPULATORS ============================== */
/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueue_Linear::addCallMessage
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

void ACDQueue_Linear::addCallMessage(ACDCall* pCallRef)
{
   pCallRef->setLastAgent(NULL);
   // from here on, ACDQueue is the same, so use it.
   ACDQueue::addCallMessage(pCallRef);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueue_Linear::buildTargetAgentList
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

bool ACDQueue_Linear::buildTargetAgentList(UtlSList& rTargetAgentList, ACDCall* pCallRef)
{
   ACDAgent* pAgent;
   ACDAgent* pLastAttemptedAgent;

   // Always rebuild the list to get the agents added on the fly
   buildACDAgentList();

   UtlSListIterator listIterator(mAcdAgentList);
   int numAgents = mAcdAgentList.entries();

   // Iterate through the ACDAgent list, starting after the agent that
   // The ACDCall was last using.  If the end of the list is
   // reached, wrap around to the beginning.  If this LastAttemptedAgent
   // is NULL, start from the head of the list
   pLastAttemptedAgent = pCallRef->getLastAgent();
   if (pLastAttemptedAgent != NULL) {
      // walk the iterator to point after pLastAttemptedAgent
      if (listIterator.findNext(pLastAttemptedAgent) == NULL) {
         // Didn't find pLastAttemptedAgent, start again from the top.
         listIterator.reset();
         pCallRef->setLastAgent(NULL);
      }
   }

   for(int i=0; i<numAgents; i++) {
      // Check the next agent
      pAgent = dynamic_cast<ACDAgent*>(listIterator());
      if (pAgent == NULL) {
         // We've hit the end of the list start back from the head of the list
         listIterator.reset();
         pAgent = dynamic_cast<ACDAgent*>(listIterator());
         if (pAgent == NULL) {
            // All out of agents to try
            return false;
         }
      }

      if (pAgent->isAvailable(true)) {
         rTargetAgentList.append(pAgent);
         // Remember the agent chosen
         pCallRef->setLastAgent(pAgent);
         OsSysLog::add(FAC_ACD, gACD_DEBUG, "%s::buildTargetAgentList - agent(%s) is added to the target list",
                       mAcdSchemeString, pAgent->getUriString()->data());
         return true;
      }
   }

   // The whole list was tried, yet no appropriate agents were found.
   return false;
}



/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
