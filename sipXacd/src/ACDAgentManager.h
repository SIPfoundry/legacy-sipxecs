//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _ACDAgentManager_h_
#define _ACDAgentManager_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <tapi/sipXtapi.h>
#include <os/OsMutex.h>
#include <os/OsEvent.h>
#include <utl/UtlHashMap.h>
#include <net/ProvisioningClass.h>
#include <net/ProvisioningAttrList.h>

// DEFINES
// Provisioning Tag Definitions
#define ACD_AGENT_TAG                 "acd-agent"
#define AGENT_URI_TAG                 "uri"
#define AGENT_NAME_TAG                "name"
#define AGENT_EXTENSION_TAG           "extension"
#define AGENT_MONITOR_PRESENCE_TAG    "monitor-presence"
#define AGENT_ALWAYS_AVAILABLE_TAG    "always-available"
#define AGENT_ACD_QUEUE_LIST_TAG      "acd-queue-list"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class ACDServer;
class ACDCallManager;
class ACDQueueManager;
class ACDAgent;
class LinePresenceMonitor;

/**
 * ACDAgentManager
 */
class ACDAgentManager : public ProvisioningClass {
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */

   // Default constructor
   ACDAgentManager(ACDServer* pAcdServer, int presenceMonitorPort, const char* pPresenceServerUriString, const char* pPresenceServiceUriString);

   // Destructor
   ~ACDAgentManager();

/* ============================ MANIPULATORS ============================== */

   OsStatus initialize(void);

   OsStatus start(void);

   bool loadConfiguration(void);

   ACDAgent* createACDAgent(const char* pAgentUriString,
                            const char* pName,
                            const char* pExtension,
                            bool        monitorPresence,
                            bool        alwaysAvailable,
                            const char* pAcdQueueList,
                            bool        pseudoAgent = FALSE);

   void deleteACDAgent(const char* pAgentUriString);

   void linePresenceSubscribe(ACDAgent* pAgent);

   void linePresenceUnsubscribe(ACDAgent* pAgent, OsEvent *e);

   void presenceServerSubscribe(ACDAgent* pAgent);

   void presenceServerUnsubscribe(ACDAgent* pAgent, OsEvent *e);

   ProvisioningAttrList* Create(ProvisioningAttrList& rRequestAttributes);
   ProvisioningAttrList* Delete(ProvisioningAttrList& rRequestAttributes);
   ProvisioningAttrList* Set(ProvisioningAttrList& rRequestAttributes);
   ProvisioningAttrList* Get(ProvisioningAttrList& rRequestAttributes);
//   ProvisioningAttrList* Action(ProvisioningAttrList& rRequestAttributes);


/* ============================ ACCESSORS ================================= */

   ACDAgent* getAcdAgentReference(UtlString& rAgentUriString);

   SIPX_INST getAcdCallManagerHandle(void);

   ACDCallManager* getAcdCallManager(void);

   ACDQueueManager* getAcdQueueManager(void);

   ACDServer* getAcdServer(void);

   LinePresenceMonitor* getLinePresenceMonitor(void);

   Url getPresenceServiceUrl() { return mPresenceServiceUrl; }
/* ============================ INQUIRY =================================== */

   /** Determine if the ACDAgentManager is health
     */
   UtlBoolean isOk() const;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
   private:
   OsMutex              mLock;                   // Lock used for atomic access
   ACDServer*           mpAcdServer;             // Reference to the parent ACDServer
   LinePresenceMonitor* mpLinePresenceMonitor;   // The SIP Dialog Monitor used to monitor on-hook/off-hook
   bool                 mPresenceServerEnabled;  // The URI for the optional Presence Server
   ACDCallManager*      mpAcdCallManager;        // Reference to the associated UA
   SIPX_INST            mhAcdCallManagerHandle;  // The sipXtapi handle for the UA
   ACDQueueManager*     mpAcdQueueManager;       // Reference to the associated UA
   UtlHashMap           mAcdAgentList;           // List of ACDAgent objects
   int                  mAgentIndex;             // Index used for supplying ACDAgent ID's
   Url                  mPresenceServiceUrl;     // The URI of presence server for XML-RPC request
};

#endif  // _ACDAgentManager_h_
