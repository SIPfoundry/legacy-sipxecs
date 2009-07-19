//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _ACDQueue_h_
#define _ACDQueue_h_

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
#include "ACDCall.h"
#include "ACDCallRouteState.h"

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


#define HUNT_GROUP_TAG                      "HuntGroup"
#define QUEUE_TAG                           "Queue"

class ACDQueue : public UtlContainable, public OsServerTask {

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   enum eAcdScheme {
      RING_ALL     = 1,
      CIRCULAR     = 2,
      LINEAR       = 3,
      LONGEST_IDLE = 4
   };

   enum eCallConnectScheme {
      BRIDGE       = 1,
      TRANSFER     = 2,
      CONFERENCE   = 3,
      CUT_THROUGH  = 4
   };

   enum eAnswerMode {
      IMMEDIATE    = 1,
      DEFERRED     = 2,
      NEVER        = 3
   };

/* ============================ CREATORS ================================== */

   // Default constructor
   ACDQueue(ACDQueueManager* pAcdQueueManager,
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
            const char*      pAcdLineList);

   // Destructor
   ~ACDQueue();

/* ============================ MANIPULATORS ============================== */

   virtual void setAttributes(ProvisioningAttrList& rRequestAttributes);

   // Add an ACDCall object to this queue for routing
   virtual void addCall(ACDCall* pCallRef);

   // Remove an ACDCall object from this queue
   virtual void removeCall(ACDCall* pCallRef);

   // Signal that the call associated with the ACDCall object has connected
   virtual void callConnected(ACDCall* pCallRef);

   // Indicate to the queue that an ACDAgent is available to receive calls
   virtual void agentAvailable(ACDAgent* pAgentRef);

   // Called by the corresponding ACDCall object to update the queue
   // of the results of a previous call route request made by this queue.
   virtual void updateRouteState(ACDCall* pCallRef, ACDCallRouteState::eRouteState state);

   // Signal that the call associated with the ACDCall object has exceeded
   // the max-wait-time for this queue
   virtual void queueMaxWaitTime(ACDCall* pCallRef);

   virtual void setRingNoAnswerState(ACDCall* pCallRef, ACDAgent* pAgentRef, bool state);

   virtual bool checkAgentAvailable();

   virtual bool checkOverflowEntryAvailable();

   virtual bool isOverflowEntryAvailable() { return mOverflowEntry!=NULL; };

/* ============================ ACCESSORS ================================= */

   // Return the AOR for this queue.
   UtlString* getUriString(void);

   void getAttributes(ProvisioningAttrList& rRequestAttributes, ProvisioningAttrList*& prResponse);

   // Calculate a unique hash code for this object.
   virtual unsigned hash() const;

   // Get the ContainableType for a UtlContainable derived class.
   virtual UtlContainableType getContainableType() const;

   // Return the connection scheme used by this queue
   virtual int getConnectionScheme() {return mCallConnectScheme;};

   // Return the name of this queue
   virtual UtlString* getQueueName() {return &mName;};

   virtual int getAgentsWrapupTime() {return mAgentsWrapupTime;};
   virtual int getAgentsNonResponsiveTime() {return mAgentsNonResponsiveTime;};
   virtual int getMaxBounceCount() {return mMaxBounceCount;};

/* ============================ INQUIRY =================================== */

   // Compare the this object to another like-objects.
   virtual int compareTo(UtlContainable const *) const;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   static const UtlContainableType TYPE;           // Class type used for runtime checking

   // Convert the comma delimited list of ACDAgent URI's to an SList of ACDAgent pointers
   virtual void buildACDAgentList(void);

   // Convert the comma delimited list of ACDLine URI's to an SList of ACDLine pointers
   virtual void buildACDLineList(void);

   // Adjust various timer values according to the queue setting
   virtual void adjustTimers(void);

   ACDQueueManager* mpAcdQueueManager;       // Reference to the parent QueueManager object
   Url              mUri;                    // The AOR for this Queue
   UtlString        mUriString;              // The AOR for this Queue
   UtlString        mName;                   // The descriptive name for this Queue
   int              mAcdScheme;
   const char*      mAcdSchemeString;
   int              mMaxRingDelay;
   int              mMaxQueueDepth;
   int              mMaxWaitTime;
   bool             mFifoOverflow;
   UtlString        mOverflowQueue;
   UtlString        mOverflowDestination;
   UtlString        mOverflowEntry;
   UtlString        mOverflowType;
   int              mAnswerMode;
   int              mCallConnectScheme;
   UtlString        mWelcomeAudio;
   bool             mBargeIn;
   UtlString        mQueueAudio;
   UtlString        mBackgroundAudio;
   int              mQueueAudioInterval;
   UtlString        mCallTerminationAudio;
   int              mTerminationToneDuration;
   UtlString        mAcdAgentListString;
   UtlSList         mAcdAgentList;
   UtlString        mAcdLineListString;
   UtlSList         mAcdLineList;

   SIPX_INST        mhAcdCallManagerHandle;   // The sipXtapi handle for the UA
   ACDCallManager*  mpAcdCallManager;         // Reference to the parent CallManager object
   ACDAgentManager* mpAcdAgentManager;        // Reference to the peer AgentManager object
   ACDLineManager*  mpAcdLineManager;         // Reference to the peer LineManager object

   UtlSList         mUnroutedCallList;        // List of unrouted calls this queue is handling
   UtlSList         mRoutingCallList;         // List of calls this queue is currently trying to route - USE appendRoutingCall()/removeRoutingCall() methods.
   UtlHashMap       mCallToRnaListMap;        // Maps a call to a RNA(RingNoAnswer) list. The RNA list parallels the mAcdAgentList to specify whether or not a RingNoAnswer occurred on a particular Agent

   int              mUnroutedCallCount;       // The number of unrouted calls this queue is handling
   ACDCall*         mpRoutePendingAnswer;     // Pointer to ACDCall that should be routed once answered

   int              mAgentsWrapupTime;        // Time to allow an agent to idle before giving him another call after he answered one.

   int              mAgentsNonResponsiveTime; // Time to allow an agent to idle before giving him another call after he RNA/Rejected one

   int              mMaxBounceCount;          // Maximum number of times an agent can RNA/Reject a call before being "bounced" (automatically signed out)

   // Handle incoming IPC messages
   virtual UtlBoolean handleMessage(OsMsg& rMessage);

   // Move a call to the unrouted list
   virtual void unroute(ACDCall* pCallRef, bool priority);

   // Deal with an unrouteable call
   virtual void unableToRoute(ACDCall* pCallRef, bool priority);

   // Route a call
   virtual void routeCall(ACDCall *pCallRef);

   // Handler for ACDQueueMsg::ADD_CALL message
   virtual void addCallMessage(ACDCall* pCallRef);

   // Handle a queue overflow condition
   virtual void overflowCall(ACDCall* pCallRef);

   // To transfer call to other destination other than overflow queue upon queue overflow condition
   virtual void transferOverflowCall(ACDCall* pCallRef);

   // Handler for ACDQueueMsg::REMOVE_CALL message
   virtual void removeCallMessage(ACDCall* pCallRef);

   // Handler for ACDQueueMsg::CALL_CONNECTED message
   virtual void callConnectedMessage(ACDCall* pCallRef);

   // virtual Handler for ACDQueueMsg::AGENT_AVAILABLE message
   virtual void agentAvailableMessage(ACDAgent* pAgentRef);

   // Handler for ACDQueueMsg::UPDATE_ROUTE_STATE::FAILED message
   virtual void updateRouteStateFailed(ACDCall* pCallRef);

   // Handler for ACDQueueMsg::UPDATE_ROUTE_STATE::ABORTED message
   virtual void updateRouteStateAborted(ACDCall* pCallRef);

   // Handler for ACDQueueMsg::UPDATE_ROUTE_STATE message
   virtual void updateRouteStateMessage(ACDCall* pCallRef, ACDCallRouteState::eRouteState state);

   // Handler for ACDQueueMsg::MAX_WAIT_TIME message
   virtual void queueMaxWaitTimeMessage(ACDCall* pCallRef);

   // Based upon the queue strategy and agent availability, this
   // method will build a list of ACDAgent object candidates
   // pure virtual...no default implemtation.
   virtual bool buildTargetAgentList(UtlSList& rTargetAgentList, ACDCall* pCallRef) = 0;

   // This method releases all the agents in the target list
   virtual void releaseTargetAgentList(UtlSList& rTargetAgentList);

   // This method checks whether any agent has signed in
   virtual bool anyAgentSignedIn(bool checkAgentAlwaysAvailable = false);

   // This method "resets" (sets to false) the RingNoAnswer states for all of the Agents for a particular call.
   virtual void resetRingNoAnswerStates(ACDCall* pCallRef);

   // This method gets the RingNoAnswer state for a particular Agent.
   virtual bool getRingNoAnswerState(ACDCall* pCallRef, ACDAgent* pAgentRef);

   virtual void appendRoutingCall(ACDCall* pCallRef);
   virtual void removeRoutingCall(ACDCall* pCallRef);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
};

#endif  // _ACDQueue_h_
