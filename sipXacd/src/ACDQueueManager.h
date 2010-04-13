//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _ACDQueueManager_h_
#define _ACDQueueManager_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsMutex.h>
#include <utl/UtlHashMap.h>
#include <net/ProvisioningClass.h>
#include <net/ProvisioningAttrList.h>
#include "ACDQueue.h"
#include "ACDQueue_Circular.h"

// DEFINES
// Provisioning Tag Definitions
#define ACD_QUEUE_TAG                       "acd-queue"
#define QUEUE_URI_TAG                       "uri"
#define QUEUE_NAME_TAG                      "name"
#define QUEUE_ACD_SCHEME_TAG                "acd-scheme"
#define QUEUE_MAX_RING_DELAY_TAG            "max-ring-delay"
#define QUEUE_MAX_QUEUE_DEPTH_TAG           "max-queue-depth"
#define QUEUE_MAX_WAIT_TIME_TAG             "max-wait-time"
#define QUEUE_FIFO_OVERFLOW_TAG             "fifo-overflow"
#define QUEUE_OVERFLOW_DESTINATION_TAG      "overflow-destination"
#define QUEUE_OVERFLOW_ENTRY_TAG            "overflow-entry"
#define QUEUE_OVERFLOW_TYPE_TAG             "overflow-type"
#define QUEUE_ANSWER_MODE_TAG               "answer-mode"
#define QUEUE_CALL_CONNECT_SCHEME_TAG       "call-connect-scheme"
#define QUEUE_WELCOME_AUDIO_TAG             "welcome-audio"
#define QUEUE_BARGE_IN_TAG                  "barge-in"
#define QUEUE_QUEUE_AUDIO_TAG               "queue-audio"
#define QUEUE_BACKGROUND_AUDIO_TAG          "background-audio"
#define QUEUE_QUEUE_AUDIO_INTERVAL_TAG      "queue-audio-interval"
#define QUEUE_CALL_TERMINATION_AUDIO_TAG    "call-termination-audio"
#define QUEUE_TERMINATION_TONE_DURATION_TAG "termination-tone-duration"
#define QUEUE_ACD_AGENT_LIST_TAG            "acd-agent-list"
#define QUEUE_ACD_LINE_LIST_TAG             "acd-line-list"
#define QUEUE_QUEUE_DEPTH_TAG               "queue-depth"
#define QUEUE_AGENTS_WRAP_UP_TIME_TAG       "agents-wrap-up-time"
#define QUEUE_AGENTS_NON_RESPONSIVE_TIME_TAG "agents-non-responsive-time"
#define QUEUE_MAX_BOUNCE_COUNT_TAG          "max-bounce-count"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class ACDServer;
class ACDCallManager;
class ACDLineManager;
class ACDAgentManager;
class ACDQueue_Circular;


// ACDQueueManager object is a centralized manager of ACDQueue objects.
// ACDQueue objects register and deregister with the QueueManager
// in response to construction and destruction.

class ACDQueueManager : public ProvisioningClass {
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   // Default constructor
   ACDQueueManager(ACDServer* pAcdServer);

   // Destructor
   ~ACDQueueManager();

/* ============================ MANIPULATORS ============================== */

   OsStatus initialize(void);

   OsStatus start(void);

   bool loadConfiguration(void);

   ACDQueue* createACDQueue(const char* pQueueUriString,
                            const char* pName,
                            int         acdScheme,
                            int         maxRingDelay,
                            int         maxQueueDepth,
                            int         maxWaitTime,
                            bool        fifoOverflow,
                            const char* pOverflowQueue,
                            const char* pOverflowEntry,
                            const char* pOverflowType,
                            int         answerMode,
                            int         callConnectScheme,
                            const char* pWelcomeAudio,
                            bool        bargeIn,
                            const char* pQueueAudio,
                            const char* pBackgroundAudio,
                            int         queueAudioInterval,
                            const char* pCallTerminationAudio,
                            int         terminationToneDuration,
                            int         agentsWrapupTime,
                            int         agentsNonResponsiveTime,
                            int         maxBounceCount,
                            const char* pAcdAgentList,
                            const char* pExternalLineList);

   void deleteACDQueue(const char* pQueueUriString);

   ProvisioningAttrList* Create(ProvisioningAttrList& rRequestAttributes);
   ProvisioningAttrList* Delete(ProvisioningAttrList& rRequestAttributes);
   ProvisioningAttrList* Set(ProvisioningAttrList& rRequestAttributes);
   ProvisioningAttrList* Get(ProvisioningAttrList& rRequestAttributes);
//   ProvisioningAttrList* Action(ProvisioningAttrList& rRequestAttributes);

/* ============================ ACCESSORS ================================= */

   ACDQueue*        getAcdQueueReference(UtlString& rQueueUriString);

   SIPX_INST        getAcdCallManagerHandle(void);

   ACDCallManager*  getAcdCallManagerReference(void);

   ACDAgentManager* getAcdAgentManagerReference(void);

   ACDLineManager*  getAcdLineManagerReference(void);

   ACDServer*  getAcdServer(void);

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
   private:
   OsMutex          mLock;                     // Lock used for atomic access
   ACDServer*       mpAcdServer;               // Reference to the parent ACDServer
   ACDCallManager*  mpAcdCallManager;          // Reference to the associated UA
   SIPX_INST        mhAcdCallManagerHandle;    // The sipXtapi handle for the UA
   ACDAgentManager* mpAcdAgentManager;         // Reference to the peer AgentManager
   ACDLineManager*  mpAcdLineManager;          // Reference to the peer LineManager
   UtlHashMap       mAcdQueueList;             // List of ACDQueue objects
};

#endif  // _ACDQueueManager_h_
