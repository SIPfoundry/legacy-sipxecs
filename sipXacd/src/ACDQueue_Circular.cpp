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
#include "ACDQueue_Circular.h"
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
//  NAME:        ACDQueue_Circular::ACDQueue_Circular
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

ACDQueue_Circular::ACDQueue_Circular(ACDQueueManager* pAcdQueueManager,
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
   mAcdSchemeString = "ACDQueue_Circular" ;
   mpLastAttemptedAgent     = NULL;

   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDQueue_Circular::ACDQueue_Circular[%s] - MaxRingDelay = %d, MaxWaitTime = %d, Overflow Queue = %s",
                 mUriString.data(), mMaxRingDelay, mMaxWaitTime, mOverflowQueue.data());
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueue_Circular::~ACDQueue_Circular
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

ACDQueue_Circular::~ACDQueue_Circular()
{
}

/* ============================ MANIPULATORS ============================== */
/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueue_Circular::buildTargetAgentList
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

bool ACDQueue_Circular::buildTargetAgentList(UtlSList& rTargetAgentList, ACDCall* pCallRef)
{
   ACDAgent* pAgent;

   // Always rebuild the list to get the agents added on the fly
   buildACDAgentList();

   UtlSListIterator listIterator(mAcdAgentList);
   int numAgents = mAcdAgentList.entries();

   // Iterate through the ACDAgent list, starting after the agent that
   // mpLastAttemptedAgent is pointing to.  If the end of the list is
   // reached, wrap around to the beginning.  If mpLastAttemptedAgent
   // is NULL, start from the head of the list
   if (mpLastAttemptedAgent != NULL) {
      // walk the iterator to point after mpLastAttemptedAgent
      if (listIterator.findNext(mpLastAttemptedAgent) == NULL) {
         // Didn't find mpLastAttemptedAgent, start again from the top.
         listIterator.reset();
         mpLastAttemptedAgent = NULL;
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
         mpLastAttemptedAgent  = pAgent;
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
