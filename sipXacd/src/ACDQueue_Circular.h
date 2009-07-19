//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _ACDQueue_Circular_h_
#define _ACDQueue_Circular_h_

// SYSTEM INCLUDES
#include <tapi/sipXtapi.h>
#include <net/Url.h>
#include <net/ProvisioningAttrList.h>
#include <utl/UtlString.h>
#include <utl/UtlSList.h>
#include <utl/UtlContainable.h>
#include <utl/UtlHashMap.h>
#include <os/OsMsg.h>
#include <os/OsServerTask.h>
//#include "ACDCall.h"
//#include "ACDCallRouteState.h"
//#include "ACDQueue.h"

// APPLICATION INCLUDES
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class ACDCallManager;
class ACDQueueManager;
class ACDAgentManager;
class ACDLineManager;
class ACDLine;
class ACDAgent;
class UtlSList;
class OsTimer;


class ACDQueue_Circular : public ACDQueue {

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   // Default constructor
   ACDQueue_Circular(ACDQueueManager* pAcdQueueManager,
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
            const char*      pAcdLineList);

   // Destructor
   ~ACDQueue_Circular();

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   // Based upon the queue strategy and agent availability, this
   // method will build a list of ACDAgent object candidates
   virtual bool buildTargetAgentList(UtlSList& rTargetAgentList, ACDCall* pCallRef);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   ACDAgent*        mpLastAttemptedAgent;     // Pointer to last ACDAgent that a call route was attempted

};

#endif  // _ACDQueue_Circular_h_
