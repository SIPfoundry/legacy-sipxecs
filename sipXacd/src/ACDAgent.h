//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _ACDAgent_h_
#define _ACDAgent_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <tapi/sipXtapi.h>
#include <os/OsMsg.h>
#include <os/OsTime.h>
#include <os/OsServerTask.h>
#include <net/Url.h>
#include <net/ProvisioningAttrList.h>
#include <utl/UtlString.h>
#include <utl/UtlSList.h>
#include <utl/UtlContainable.h>
#include <cp/LinePresenceBase.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class OsTimer;
class UtlSList;
class ACDCallManager;
class ACDAgentManager;
class ACDQueue;
class ACDCall;

class ACDAgent : public UtlContainable, public LinePresenceBase, public OsServerTask {
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum eAgentTimers {
      WRAP_UP_TIMER           = 0
   };

/* ============================ CREATORS ================================== */

   // Default constructor
   ACDAgent(ACDAgentManager* pAcdAgentManager,
            const char*     pAgentUriString,
            const char*     pName,
            const char*     pExtension,
            int             agentId,
            bool            monitorPresence,
            bool            alwaysAvailable,
            const char*     pAcdQueueList,
            bool     pseudoAgent = FALSE);

   // Destructor
   ~ACDAgent();

/* ============================ MANIPULATORS ============================== */

   // Update the presence / line state for this line.
   void updateState(ePresenceStateType type, bool state);

   // Set the requested ACDAgent attributes.
   void setAttributes(ProvisioningAttrList& rRequestAttributes);

   // Convert the comma seperated list of ACDQueues to an SList of ACDQueue pointers.
   void buildACDQueueList(void);

   // Initiate an outbound call to this agent using ACDCall information
   SIPX_CALL connect(ACDCall* pCall);

   // Drop the ACDAgent connection
   void drop(bool rna=false);

   // Place the ACDAgent connection on-hold
   void hold(void);

   // Take the ACDAgent connection off-hold
   void unhold(void);

   // Set the agent Busy (not free)
   void setBusy(void) ;

   // Set the agent free.
   void setFree(void) ;

   void setCallHandle (SIPX_CALL hCallHandle) { mhCallHandle = hCallHandle;}

   bool isPseudoAgent(void){ return mIsPseudo;};

   void setIsPseudoAgent(bool pseudo) {mIsPseudo = pseudo;};

   void setDelete(bool flagDelete) {mFlagDelete = flagDelete;};

   // Log the fact the agent signed in/out
   void logSignIn(ACDQueue *pQueue, bool agentSignIn);

/* ============================ ACCESSORS ================================= */

   // Return the AOR for this line.
   Url* getUri(void);

   // Return the AOR for this line.
   UtlString* getUriString(void);

   // Return the presence / line state information for this line.
   bool getState(ePresenceStateType type);

   // Retrieve the requested attribute values.
   void getAttributes(ProvisioningAttrList& rRequestAttributes, ProvisioningAttrList*& prResponse);

   // Return the sipXtapi call handle associated with this agent
   SIPX_CALL getCallHandle(void);

   // Return the time, in seconds, that this ACDAgent has been idle.
   unsigned long getIdleTime(void);

   // Calculate a unique hash code for this object.
   virtual unsigned hash() const;

   // Get the ContainableType for a UtlContainable derived class.
   virtual UtlContainableType getContainableType() const;

   // Get the Queue List String
   UtlString getAcdQueueListString();

/* ============================ INQUIRY =================================== */

   // Return the value of the always-available attribute
   bool alwaysAvailable(void);

   // Test to see if the ACDAgent is available to service calls
   // If so, and markBusy is true, then set it busy atomically
   bool isAvailable(bool markBusy);

   bool isFree(void) { return mFree;};

   // Return true if the ACDAgent connection is on-hold
   bool isOnHold(void);

   bool getDelete(void) { return mFlagDelete;};

   // Compare the this object to another like-object.
   virtual int compareTo(UtlContainable const *) const;

   // Get a link to its ACDCallManager. Required by call pickup.
   ACDCallManager* getAcdCallManager(void) { return mpAcdCallManager; }

   void setCallEstablished(bool established) { mCallEstablished = established; }

   void signOut();
/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   static const UtlContainableType TYPE;           // Class type used for runtime checking

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   OsMutex          mLock;                   // Lock used for atomic access
   Url              mUri;                    // The AOR for this Agent
   UtlString        mUriString;              // The AOR for this Agent
   UtlString        mName;                   // The descriptive name for this Agent
   UtlString        mAcdQueueListString;     // The ACDQueues associated with this Agent
   UtlSList         mAcdQueueList;           // The ACDQueues associated with this Agent
   UtlString        mExtension;              // The extension associated with this Agent
   int              mAgentId;                // The internal numeric ID for this Agent
   bool             mMonitorPresence;        // Subscripbe to the Presence Server
   bool             mAlwaysAvailable;        // Assume that this Agent is always available
   ACDAgentManager* mpAcdAgentManager;       // Reference to the parent AgentManager object
   ACDCallManager*  mpAcdCallManager;        // Reference to the parent CallManager object
   SIPX_INST        mhAcdCallManagerHandle;  // The sipXtapi handle for the UA
   SIPX_CALL        mhCallHandle;            // The sipXtapi call handle for the outbound call to this agent
   bool             mOnHold;                 // Flag to indicate if this agent is on-hold
   unsigned int     mAgentLineState;         // The line attributes for this ACDAgent object
   bool             mAvailable;              // Flag indicating if this ACDAgent is available to take calls
   bool             mFree;                   // Flag indicating if this ACDAgent is inuse
   OsTime           mIdleTimeStart;          // The time that the agent last went idle
   int              mAgentTicker;
   bool             mIsPseudo;
   bool             mFlagDelete;
   int              mWrapupTime;
   int              mNonResponsiveTime;
   OsTimer*         mpWrapupTimer;
   bool             mCallEstablished;        // Flag indicating if the ACDAgent has established call
   int              mMaxBounceCount;         // Maximum number of unanswered calls before the agent is "bounced"
   int              mBounceCount;            // Current count of unanswered calls

   // Notify the queues of this agent's availability
   void notifyAvailability();

   // Keep mAvailable flag in line with current state
   void setAvailable();

   // Set the agent on/off hook
   void setOnHook(bool onHook) ;

   // Set the agent sign in/out
   void setSignIn(bool agentSignIn);

   // Handle incomming IPC messages
   UtlBoolean handleMessage(OsMsg& rMessage);

   // Update the presence / line state for this line.
   void updateStateMessage(ePresenceStateType type, bool state, bool recordIdle = false);

   // Drop the call to the agent
   void dropCallMessage(bool rna) ;

   void handleWrapupTimeout();
};

#endif  // _ACDAgent_h_
