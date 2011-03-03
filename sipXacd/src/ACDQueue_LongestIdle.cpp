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
#include "ACDQueue_LongestIdle.h"
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
//  NAME:        ACDQueue_LongestIdle::ACDQueue_LongestIdle
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

ACDQueue_LongestIdle::ACDQueue_LongestIdle(ACDQueueManager* pAcdQueueManager,
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
   mAcdSchemeString = "ACDQueue_LongestIdle";
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDQueue_LongestIdle::ACDQueue_LongestIdle[%s] - MaxRingDelay = %d, MaxWaitTime = %d, Overflow Queue = %s",
                 mUriString.data(), mMaxRingDelay, mMaxWaitTime, mOverflowQueue.data());
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueue_LongestIdle::~ACDQueue_LongestIdle
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

ACDQueue_LongestIdle::~ACDQueue_LongestIdle()
{
}

/* ============================ MANIPULATORS ============================== */
/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueue_LongestIdle::buildTargetAgentList
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

bool ACDQueue_LongestIdle::buildTargetAgentList(UtlSList& rTargetAgentList, ACDCall* pCallRef)
{
   // Always rebuild the list to get the agents added on the fly
   buildACDAgentList();

   // Iterate through the ACDAgent list, starting at the head of the list
   // and find the agent that has been idle the longest.
   for(;;) {
      ACDAgent* pAgent;
      ACDAgent* pLongestIdleAgent = NULL;
      unsigned long maxIdleTime = 0;
      UtlSListIterator listIterator(mAcdAgentList);

      // Find the available agent with the longest idle time
      while ((pAgent = dynamic_cast<ACDAgent*>(listIterator())) != NULL) {
         if (pAgent->isAvailable(false)) {
            if (pAgent->getIdleTime() >= maxIdleTime) {
               maxIdleTime = pAgent->getIdleTime();
               pLongestIdleAgent = pAgent;
            }
         }
      }

      if (pLongestIdleAgent == NULL) {
         // None available
         break ;
      }

      // If he is still available, use him.  Otherwise start again.
      if (pLongestIdleAgent->isAvailable(true)) {
         rTargetAgentList.append(pLongestIdleAgent);
         OsSysLog::add(FAC_ACD, gACD_DEBUG, "%s::buildTargetAgentList - agent(%s) is added to the target list.  Idle time was %lu seconds",
                 mAcdSchemeString, pLongestIdleAgent->getUriString()->data(),
                 maxIdleTime);
         return true ;
      }
   }

   // The whole list was tried, yet no appropriate agents were found.
   return false;
}



/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
