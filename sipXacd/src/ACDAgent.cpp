//
//
// Copyright (C) 2007, 2010 Avaya, Inc., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <cp/LinePresenceMonitor.h>
#include <net/StateChangeNotifier.h>
#include <net/XmlRpcRequest.h>
#include <net/Url.h>
#include <os/OsDateTime.h>
#include <os/OsEvent.h>
#include <os/OsTimer.h>
#include <os/OsEventMsg.h>
#include <utl/UtlTokenizer.h>
#include <utl/UtlSListIterator.h>
#include "ACDQueueManager.h"
#include "ACDQueue.h"
#include "ACDAgentManager.h"
#include "ACDAgentMsg.h"
#include "ACDAgent.h"
#include "ACDLine.h"
#include "ACDServer.h"
#include "ACDRtRecord.h"
#include "ACDCallManager.h"

//#define DUMP_URL 1
//#define TEST_PRINT 1

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
extern OsSysLogPriority gACD_DEBUG;

// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
const UtlContainableType ACDAgent::TYPE = "ACDAgent";


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgent::ACDAgent
//
//  SYNOPSIS:    ACDAgent(
//                 ACDAgentManager* pAcdAgentManager, Pointer to parent ACDAgentManager instance
//                 const char*      pAgentUriString,  The URI of this agent
//                 const char*      pName,            The descriptive name of this agent
//                 const char*      pExtension,       The PBX extension alias of this agent
//                 bool             monitorPresence,  Flag indicating if dialog state should be
//                                                    monitored
//                 bool             alwaysAvailable,  Flag indicating if this agent should always be
//                                                    considered signed-in
//                 const char*      pAcdQueueList)    List of ACDQueue URI's that this agent is
//                                                    assigned to
//
//  DESCRIPTION: The default constructor for the ACDAgent class.
//               The member variables are initialized. The ACDCallManager instance is retrieved
//               and stored.  The initial state is set to ON-HOOK.  If enabled, the ACDAgent
//               is subscribed to the LinePresenceMonitor.  The ACDQueue list is parsed and
//               converted to an SList and stored.
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ACDAgent::ACDAgent(ACDAgentManager* pAcdAgentManager,
                   const char*      pAgentUriString,
                   const char*      pName,
                   const char*      pExtension,
                   int              agentId,
                   bool             monitorPresence,
                   bool             alwaysAvailable,
                   const char*      pAcdQueueList,
                   bool             pseudoAgent)
: OsServerTask("ACDAgent-%d"), mLock(OsMutex::Q_FIFO)
{
   mpAcdAgentManager   = pAcdAgentManager;
   mUriString          = pAgentUriString;
   mUri                = pAgentUriString;
   mName               = pName;
   mExtension          = pExtension;
   mAgentId            = agentId;
   mMonitorPresence    = monitorPresence;
   mAlwaysAvailable    = alwaysAvailable;
   mAcdQueueListString = pAcdQueueList;
   mAgentLineState     = 0;
   mAvailable          = false;
   mFree               = true;
   mOnHold             = false;
   mhCallHandle        = SIPX_CALL_NULL;
   mFlagDelete         = false;
   mAgentTicker        = 0;
   mIsPseudo           = pseudoAgent;
   mWrapupTime         = 0;
   mNonResponsiveTime   = 0 ;
   mpAcdCallManager       = mpAcdAgentManager->getAcdCallManager();
   mhAcdCallManagerHandle = mpAcdAgentManager->getAcdCallManagerHandle();
   mBounceCount        = 0;

   mpWrapupTimer =
      new OsTimer(getMessageQueue(), (void*)WRAP_UP_TIMER);

   mCallEstablished = false;
   // Set the agents idle time to now
   OsDateTime nowTime;
   OsDateTime::getCurTime(nowTime);
   nowTime.cvtToTimeSinceEpoch(mIdleTimeStart);

   if (true == mIsPseudo) {
      return ;
   }

   // Start off assuming that Agent is ON-HOOK
   updateState(LinePresenceBase::ON_HOOK, true);

   // Check if the agent should subscribe with the Presence Server
   if (!mAlwaysAvailable) {
      mpAcdAgentManager->presenceServerSubscribe(this);
   }

   // Check if the agent should subscribe with the Line Presence Monitor
   if (mMonitorPresence && mAlwaysAvailable) {
      mpAcdAgentManager->linePresenceSubscribe(this);
   }

   // Convert the comma delimited list to an SList
   buildACDQueueList();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgent::~ACDAgent
//
//  SYNOPSIS:    None.
//
//  DESCRIPTION: The destructor for the ACDAgent class.
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ACDAgent::~ACDAgent()
{
   delete mpWrapupTimer;

   if (true == mIsPseudo) {
      return ;
   }

   // Check if the agent should unsubscribe from the Line Presence Monitor
   if (mMonitorPresence) {
      OsEvent e;
      mpAcdAgentManager->linePresenceUnsubscribe(this, &e);
      e.wait(); // Wait for Unsubscribe to finish before continuing
   }

   // Check if the agent should unsubscribe from the Presence Server
   if (!mAlwaysAvailable) {
      OsEvent e;
      mpAcdAgentManager->presenceServerUnsubscribe(this, &e);
      e.wait(); // Wait for Unsubscribe to finish before continuing
   }
}

/* ============================ MANIPULATORS ============================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgent::updateState
//
//  SYNOPSIS:    void updateState(
//                 ePresenceStateType type,  Enumeration indicating which Presence State attribute
//                                           is to be updated.  These enumerations are defined
//                                           in <net/LinePresenceBase.h>
//                 bool               state  What to set the specified Presence State attribute to.
//
//  DESCRIPTION: This is the subclass implementation of the virtual LinePresenceBase::updateState().
//               It is called by the LinePresenceMonitor, if subscribed, to update the ACDAgents
//               availability based upon SIP dialog and presence information.
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     The associated state information is protected by semaphore locks to ensure
//               thread safe access, both from this function and from the associated functions
//               getState() and isAvailable().
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDAgent::updateState(ePresenceStateType type, bool state)
{
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDAgent(%s)::updateState - type: %d, state: %d", mUriString.data(), type, state);
   osPrintf("ACDAgent(%s)::updateState - type: %d, state: %d\n", mUriString.data(), type, state);

   ACDAgentMsg updateStateMsg(ACDAgentMsg::UPDATE_STATE, type, state);
   postMessage(updateStateMsg);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgent::connect
//
//  SYNOPSIS:    SIPX_CALL connect(
//                 ACDCall *   pACDCall, incoming ACD call
//                                 )
//
//  DESCRIPTION: Create an outbound sipXtapi call to the ACDAgents configured URI.  It will
//               also update the line presence state to OFF-HOOK.
//
//  RETURNS:     On success, will return a sipXtapi call handle for the outbound call.
//
//  ERRORS:      On failure, will return zero.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

SIPX_CALL ACDAgent::connect(ACDCall* pACDCall)
{
   SIPX_LINE hLine = pACDCall->getAcdAgentLineHandle();
   const char *pCallId = pACDCall->getCallId() ; // The Call-Id of ACDCall
   ACDLine* acdLine = pACDCall->getAcdAgentLineReference();

   // Create the outbound call to the agent URI
   if (sipxCallCreate(mhAcdCallManagerHandle, hLine, &mhCallHandle) != SIPX_RESULT_SUCCESS) 
   {
      OsSysLog::add(FAC_ACD, PRI_ERR,
                    "ACDAgent::connect - "
                    "ACDAgent(%s) failed to create outbound call",
                    mUriString.data());
      mhCallHandle = SIPX_CALL_NULL ;
      return 0;
   }

   OsSysLog::add(FAC_ACD, gACD_DEBUG,
                 "ACDAgent::connect - "
                 "ACDAgent(%s) succeeded to create outbound call with call handle %d.  mBounceCount=%d",
                 mUriString.data(), mhCallHandle, mBounceCount);

   // Generate a unique call ID which is based upon the passed-in call ID and Agent ID
   // The SIPX_CURR_SERV_STATE env. variable is either set to "A", if the current server is the primary one
   // or "B", if it is the backup server.
   char pTempId[256];
   UtlString sCurrServState = getenv("SIPX_CURR_SERV_STATE");
   sprintf(pTempId, "%s-ACD%s%03d-%d", pCallId, sCurrServState.data(),mAgentId, mAgentTicker);
   mAgentTicker++;

   // Mark the agent as being off-hook
   setOnHook(false) ;

   // Create the string to be used as the FROM header
   // Note: FROM header is not used for authorization
   UtlString agentUrlString ;
   UtlString fromDisp, fromUser, displayUserAndQueue;

   const char *pFrom = pACDCall->getCallIdentity() ; // The "From" of incoming ACDCall
   const char *pAcdLineAor = acdLine->getUriString()->data(); // to be added to display field

   Url tempFromUrl(pFrom, FALSE);        // Url::NameAddr
   Url tempAcdUrl(pAcdLineAor, FALSE);   // Url::NameAddr

   // get the queue name where the call came from
   ACDQueue* acdQueue = pACDCall->getMpManagingQueue();
   if(acdQueue) 
   {
      UtlString *queueName = acdQueue->getQueueName();
      UtlString tempQueueName;
      if (queueName) 
      {
          // Add the queue name where the call came from as the text part 
          // of the SIP URL in To header.
          agentUrlString = "\"" + *queueName + "\" " ;

          // Queue name to use in from header display
          tempQueueName.append( queueName->data());
      }

      // Add queue name and line into from header display field
      tempFromUrl.getDisplayName(fromDisp);
      tempAcdUrl.getUserId(fromUser);

      if (fromDisp.isNull())
      {
          fromDisp.append("\"");
          //fromDisp.append("\".");
      }
      else 
      {
          fromDisp.strip(UtlString::trailing, '"');
      }
      fromDisp.append(" " + tempQueueName + "(" + fromUser + ")\"");

      tempFromUrl.setDisplayName(fromDisp.data());
      tempFromUrl.toString(displayUserAndQueue);

#ifdef TEST_PRINT
      OsSysLog::add(FAC_ACD, PRI_DEBUG, 
                    "ACDAgent::connect - "
                    "fromDisp '%s' fromUser '%s' "
                    "From '%s' pAcdLineAor '%s' qName '%s' "
                    "displayUserAndQueue '%s'",
                    fromDisp.data(), fromUser.data(),
                    pFrom, pAcdLineAor, queueName->data(),
                    displayUserAndQueue.data());
#endif
#ifdef DUMP_URL
      tempFromUrl.kedump();
      tempAcdUrl.kedump();
#endif

      // Update mWrapupTime from the queue
      mWrapupTime = acdQueue->getAgentsWrapupTime();
      // Update mNonResponsiveTime from the queue
      mNonResponsiveTime = acdQueue->getAgentsNonResponsiveTime();
      // Update mMaxBounceCount from the queue
      mMaxBounceCount = acdQueue->getMaxBounceCount();
      OsSysLog::add(FAC_ACD, gACD_DEBUG,
                    "ACDAgent::connect - ACDAgent(%s) "
                    "mWrapupTime=%d mNonResponsiveTime=%d mMaxBounceCount=%d",
                    mUriString.data(),mWrapupTime, mNonResponsiveTime, mMaxBounceCount);
   }

   // Add the sipx-noroute=Voicemail parameter to prevent the proxy from
   // routing this Agent call to voicemail if the agent doesn't answer, and
   // has voicemail enabled.

   // The angle brackets are required to make the parameter a URI parameter
   // and show up in the request URI not just the "To" field.
   agentUrlString += "<" + mUriString + ";sipx-noroute=VoiceMail>" ;

   // Fire off the call
   if (sipxCallConnect(mhCallHandle,
                       agentUrlString, 
                       0, NULL,
                       pTempId,
                       displayUserAndQueue.data(), 
                       TRUE) != SIPX_RESULT_SUCCESS)    //CpCallManager::AddPaiHdr
   {
      OsSysLog::add(FAC_ACD, PRI_ERR,
                    "ACDAgent::connect - "
                    "ACDAgent(%s) failed to initiate outbound call",
                    mUriString.data());
      return 0;
   }

   return mhCallHandle;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgent::drop
//
//  SYNOPSIS:    void drop(bool rna)
//
//  DESCRIPTION: send a DROP message to the queue, so dropCallMessage is called in the correct context.
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDAgent::drop(bool rna)
{
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDAgent::drop Agent(%s)",
                    getUriString()->data());

   // Remove the association 'tween Call and Agent right away, as
   // events on that agent should no longer be seen by the Call.
   // Otherwise race conditions can cause Agent events after this
   // point to be processed and mess up the call state.
   // For example, we are about to call sipxCallDestroy().  But say
   // the call was answered in the meantime and we get a CONNECT event?
   // We don't want the Call State machine to process that event, so by
   // removing the mapping right here, the Call will never see it.
   mpAcdCallManager->removeMapAgentCallHandleToCall(mhCallHandle);

   ACDAgentMsg dropMsg(ACDAgentMsg::DROP_CALL, rna);
   postMessage(dropMsg);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgent::hold
//
//  SYNOPSIS:    void hold(void)
//
//  DESCRIPTION: This will put the ACDAgent connection on hold.
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDAgent::hold(void)
{
   if (!mOnHold) {
      mOnHold = true;
      sipxCallHold(mhCallHandle);
   }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgent::unhold
//
//  SYNOPSIS:    void unhold(void)
//
//  DESCRIPTION: This will take a previously held ACDAgent connection off hold.
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDAgent::unhold(void)
{
   if (mOnHold) {
      mOnHold = false;
      sipxCallUnhold(mhCallHandle);
   }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgent::setBusy(void)
//
//  SYNOPSIS:    void setBusy(void)
//
//  DESCRIPTION: Mark the agent busy so it cannot be used by another.
//               Only when drop() is called will it be free again, or if
//               the agent is never used for a call, setFree().
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDAgent::setBusy(void)
{
    mFree = false ;
    setAvailable() ;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgent::setFree(void)
//
//  SYNOPSIS:    void setFree(void)
//
//  DESCRIPTION: Mark the agent free so it can now be used by another.
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDAgent::setFree(void)
{
    mFree = true ;
    mCallEstablished = false;
    setAvailable() ;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgent::setAttributes
//
//  SYNOPSIS:    void setAttributes(
//                 ProvisioningAttrList& rRequestAttributes) ProvisioningAgent attribute list
//                                                           containing one or more ACDAgent
//                                                           attributes to be updated.
//
//  DESCRIPTION: Used by the ACDAgentManager::set() function to update on or more attributes
//               in response to recieving a provisioning set request.  There are side effects
//               to setting monitor-presence and always-available if there is a change in value.
//
//  RETURNS:     None.
//
//  ERRORS:      If an error is encountered, an exception will be thrown, with the description
//               of the error being specified in "UtlString error".
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDAgent::setAttributes(ProvisioningAttrList& rRequestAttributes)
{
   mLock.acquire();

   // Set the individual attributes
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDAgent(%s)::setAttributes",
                 mUriString.data());

   // name
   rRequestAttributes.getAttribute(AGENT_NAME_TAG, mName);

   // extension
   rRequestAttributes.getAttribute(AGENT_EXTENSION_TAG, mExtension);

   // monitor-presence
   // take action only if adminstrative state is STANDBY
   if (((mpAcdAgentManager->getAcdServer())->getAdministrativeState()) == ACDServer::STANDBY) {
   	OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDAgent(%s)::setAttributes - monitor-presence",
                 mUriString.data());
   	rRequestAttributes.getAttribute(AGENT_MONITOR_PRESENCE_TAG, mMonitorPresence);
      	// FUTURE: Inteligently subscribe/unsubscribe to LinePresenceMonitor and PresenceServer
   }

   // always-available
   // take action only if adminstrative state is STANDBY
   if ((mpAcdAgentManager->getAcdServer())->getAdministrativeState() == ACDServer::STANDBY) {
   	OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDAgent(%s)::setAttributes - always-available",
                 mUriString.data());
      if (rRequestAttributes.getAttribute(AGENT_ALWAYS_AVAILABLE_TAG, mAlwaysAvailable)) {
      	 // FUTURE: Inteligently subscribe/unsubscribe to LinePresenceMonitor and PresenceServer

         // See if this will cause a transition from unavailable to available
         // if so, make it available and notify all who care
         setAvailable() ;
      }
   }

   // acd-queue-list
   if (rRequestAttributes.getAttribute(AGENT_ACD_QUEUE_LIST_TAG, mAcdQueueListString)) {
      // Convert the comma delimited list to an SList
      buildACDQueueList();
   }

   mLock.release();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgent::buildACDQueueList
//
//  SYNOPSIS:    void buildACDQueue(void)
//
//  DESCRIPTION: Converts the comma delimited list of ACDQueue URI's, maintained in
//               mAcdQueueListString, to an SList of ACDQueue pointers (mAcdQueueList).
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDAgent::buildACDQueueList(void)
{
   ACDQueueManager* pAcdQueueManager;

   pAcdQueueManager = mpAcdAgentManager->getAcdQueueManager();

   // First clear the existing list
   UtlSListIterator listIterator(mAcdQueueList);
   UtlContainable* pEntry;
   while ((pEntry = listIterator()) != NULL) {
      mAcdQueueList.remove(pEntry);
   }

   // Check to see if there is anything in the mAcdQueueListString
   if (mAcdQueueListString == NULL) {
      // It is empty, just return
      return;
   }
   // Add the new entries to the list
   UtlTokenizer tokenList(mAcdQueueListString);
   UtlString entry;
   while (tokenList.next(entry, ",")) {
      entry.strip(UtlString::both);
      ACDQueue* pQueue = pAcdQueueManager->getAcdQueueReference(entry);
      if (pQueue != NULL) {
         mAcdQueueList.append(pQueue);
      }
   }
}


/* ============================ ACCESSORS ================================= */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgent::getUri
//
//  SYNOPSIS:    Url* getUri(void)
//
//  DESCRIPTION: This is the subclass implementation of the virtual LinePresenceBase::getUri().
//               It returns a pointer to a Url object representing the URI which has been
//               configured as the contact / AOR for this ACDAgent
//
//  RETURNS:     Pointer to a Url object representing the configured URI of this ACDAgent
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

Url* ACDAgent::getUri(void)
{
   return &mUri;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgent::getUriString
//
//  SYNOPSIS:    UtlString* getUriString(void)
//
//  DESCRIPTION: Returns a pointer to a UtlString object representing the URI which has been
//               configured as the contact / AOR for this ACDAgent
//
//  RETURNS:     Pointer to a UtlString representing the configured URI of this ACDAgent
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

UtlString* ACDAgent::getUriString(void)
{
   return &mUriString;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgent::getState
//
//  SYNOPSIS:    bool getState(
//                 ePresenceStateType type)  Enumeration indicating which Presence State attribute
//                                           is to be retreived.  These enumerations are defined
//                                           in <net/LinePresenceBase.h>
//
//  DESCRIPTION: Returns the current state of the requested Presence State attribute.
//               If the ACDAgent is set to always-available, a request for the SIGNED_IN state
//               will always return TRUE, regardless of the actual attribute value.
//
//  RETURNS:     bool representing the requested Presence State attribute.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

bool ACDAgent::getState(ePresenceStateType type)
{
   bool state;

   mLock.acquire();

   // If checking the SIGNED_IN state and the the Agent is
   // flagged as always-available, return true
   if ((type == LinePresenceBase::SIGNED_IN) && mAlwaysAvailable) {
      state = true;
   }
   else
   {
      if ((mAgentLineState & static_cast<unsigned int>(type)) == static_cast<unsigned int>(type)) {
         state = true;
      }
      else {
         state = false;
      }
   }

   mLock.release();

   return state;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgent::getIdleTime
//
//  SYNOPSIS:    unsigned long getIdleTime(void)
//
//  DESCRIPTION: Returns the amount of time, in seconds, since the ACDAgent last serviced a call.
//
//  RETURNS:     unsigned long representing idle time in seconds.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned long ACDAgent::getIdleTime(void)
{
   unsigned long idleSeconds;
   OsTime     nowSeconds;
   OsDateTime nowTime;

   OsDateTime::getCurTime(nowTime);
   nowTime.cvtToTimeSinceEpoch(nowSeconds);

   idleSeconds = nowSeconds.seconds() - mIdleTimeStart.seconds();

   return idleSeconds;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgent::hash
//
//  SYNOPSIS:    unsigned hash(void) const
//
//  DESCRIPTION: Calculate a unique hash code for this object.  If the equals operator returns
//               true for another object, then both of those objects must return the same hashcode.
//               The URI String is used as the basis of the hash.
//
//  RETURNS:     unsigned int representing the calculated hash code
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned ACDAgent::hash(void) const
{
   return mUriString.hash();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgent::getContainableType
//
//  SYNOPSIS:    UtlContainableType getContainableType(void) const
//
//  DESCRIPTION: Get the ContainableType for a UtlContainable derived class.
//
//  RETURNS:     UtlContainableType identifying this container type.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

UtlContainableType ACDAgent::getContainableType(void) const
{
   return ACDAgent::TYPE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgent::getCallHandle
//
//  SYNOPSIS:    SIPX_CALL getCallHandle(void)
//
//  DESCRIPTION: Get the sipXtapi call handle associated with the outbound call made to the
//               ACDAgents configured URI.
//
//  RETURNS:     SIPX_CALL handle of the associated sipXtapi call instance.  If no active call,
//               zero will be returned.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

SIPX_CALL ACDAgent::getCallHandle(void)
{
   return mhCallHandle;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgent::getAttributes
//
//  SYNOPSIS:    bool getAttributes(
//                 ProvisioningAttrList&  rRequestAttributes, ProvisioningAgent attribute list
//                                                            containing one or more ACDAgent
//                                                            attributes to be retrieved.
//                 ProvisioningAttrList*& prResponse)         ProvisioningAgent attribute list
//                                                            containing the requested attributes
//                                                            and their corresponding values.
//
//  DESCRIPTION: Used by the ACDAgentManager::get() function to request the value of one or more
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

void ACDAgent::getAttributes(ProvisioningAttrList& rRequestAttributes, ProvisioningAttrList*& prResponse)
{
   // See if there are any specific attributes listed in the request
   if (rRequestAttributes.attributePresent(AGENT_NAME_TAG) ||
       rRequestAttributes.attributePresent(AGENT_EXTENSION_TAG) ||
       rRequestAttributes.attributePresent(AGENT_MONITOR_PRESENCE_TAG) ||
       rRequestAttributes.attributePresent(AGENT_ALWAYS_AVAILABLE_TAG) ||
       rRequestAttributes.attributePresent(AGENT_ACD_QUEUE_LIST_TAG)) {
      // At least one attribute has been requested, go through list and retrieve
      // name
      if (rRequestAttributes.attributePresent(AGENT_NAME_TAG)) {
         prResponse->setAttribute(AGENT_NAME_TAG, mName);
      }

      // extension
      if (rRequestAttributes.attributePresent(AGENT_EXTENSION_TAG)) {
         prResponse->setAttribute(AGENT_EXTENSION_TAG, mExtension);
      }

      // monitor-presence
      if (rRequestAttributes.attributePresent(AGENT_MONITOR_PRESENCE_TAG)) {
         prResponse->setAttribute(AGENT_MONITOR_PRESENCE_TAG, mMonitorPresence);
      }

      // always-available
      if (rRequestAttributes.attributePresent(AGENT_ALWAYS_AVAILABLE_TAG)) {
         prResponse->setAttribute(AGENT_ALWAYS_AVAILABLE_TAG, mAlwaysAvailable);
      }

      // acd-queue-list
      if (rRequestAttributes.attributePresent(AGENT_ACD_QUEUE_LIST_TAG)) {
         prResponse->setAttribute(AGENT_ACD_QUEUE_LIST_TAG, mAcdQueueListString);
      }

   }
   else {
      // No specific attributes were requested, send them all back
      // name
      prResponse->setAttribute(AGENT_NAME_TAG, mName);

      // extension
      prResponse->setAttribute(AGENT_EXTENSION_TAG, mExtension);

      // monitor-presence
      prResponse->setAttribute(AGENT_MONITOR_PRESENCE_TAG, mMonitorPresence);

      // always-available
      prResponse->setAttribute(AGENT_ALWAYS_AVAILABLE_TAG, mAlwaysAvailable);

      // acd-queue-list
      prResponse->setAttribute(AGENT_ACD_QUEUE_LIST_TAG, mAcdQueueListString);
   }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgent::getAcdQueueListString()
//
//
//  DESCRIPTION: Get the Utl List String of all the queues delimited by comma.
//
//  RETURNS:     UtlString
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

UtlString ACDAgent::getAcdQueueListString(void)
{
   return mAcdQueueListString;
}

/* ============================ INQUIRY =================================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgent::isAvailable
//
//  SYNOPSIS:    bool isAvailable(bool markBusy)
//
//  DESCRIPTION: Check to see if this ACDAgent is available to handle calls.
//               The general criteria for being able to handle calls is that the ACDAgent be:
//               Free, ON_HOOK, and SIGNED_IN.  The criteria of SIGNED_IN can
//               be removed based upon how the corresponding attribute
//               always-available is set.
//
//               If markBusy is true, and isAvailable is going to return true,
//               then atomically mark the agent busy.
//
//  RETURNS:     bool indicating availablility of the ACDAgent.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

bool ACDAgent::isAvailable(bool markBusy)
{
   bool available = false;
   const char *reason = "available" ;

   mLock.acquire();

   for(;;) // dummy loop for break ;
   {
      if (!mFree)
      {
         // If not free, then agent is not available
         reason = "not free";
         break;
      }

      if ((mAgentLineState & LinePresenceBase::ON_HOOK) == false)
      {
         reason = "OFF_HOOK";
         break;
      }

      if (mAlwaysAvailable)
      {
          available = true ;
          reason = "SIGNED_IN (always)";
          break ;
      }


      if (mAgentLineState & LinePresenceBase::SIGNED_IN)
      {
         available = true ;
         reason = "SIGNED_IN";
         break ;
      }
      else
      {
         reason = "SIGNED_OUT";
         break;
      }
      break ;
   }
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDAgent::isAvailable - ACDAgent(%s) is %s %s",
                 mUriString.data(), mAvailable == available ? "still":"now", reason);

   if (available && markBusy)
   {
      setBusy();
   }
   mLock.release();

   return available;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgent::alwaysAvailable
//
//  SYNOPSIS:    bool alwaysAvailable(void)
//
//  DESCRIPTION: Returns the value of the always-available attribute.
//
//  RETURNS:     bool indicating always-available attribute value.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

bool ACDAgent::alwaysAvailable(void)
{
   return mAlwaysAvailable;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgent::isOnHold
//
//  SYNOPSIS:    bool isOnHold(void)
//
//  DESCRIPTION: Return true if the associated outbound call is currently on-hold.
//
//  RETURNS:     bool indicating call-hold state.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

bool ACDAgent::isOnHold(void)
{
   return mOnHold;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgent::compareTo
//
//  SYNOPSIS:    int compareTo(
//                 UtlContainable const* pInVal) Pointer to a UtlContainable type to perform
//                                               the comparison against.
//
//  DESCRIPTION: Compare the this object to another UtlContainable derived object of the
//               same type.
//
//  RETURNS:     int value indicating result. zero if equal, non-zero if not.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

int ACDAgent::compareTo(UtlContainable const* pInVal) const
{
   int result ;

   if (pInVal->isInstanceOf(ACDAgent::TYPE)) {
      result = mUriString.compareTo(((ACDAgent*)pInVal)->getUriString());
   }
   else {
      result = -1;
   }

   return result;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */


/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgent::notifyAvailablility
//
//  SYNOPSIS:    void (void)
//
//  DESCRIPTION: send notification of availability to all the queues of which this agent is assigned.
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////
void ACDAgent::notifyAvailability()
{
   // Now notify all of the queues this agent is assigned to
   ACDQueue* pQueue;
   pQueue = dynamic_cast<ACDQueue*>(mAcdQueueList.at(0));
   if (pQueue == NULL)
   {
      // The ACDQueue list is empty, try rebuilding
      buildACDQueueList();
   }
   // Iterate through the list and send notification of availability
   UtlSListIterator listIterator(mAcdQueueList);
   while ((pQueue = dynamic_cast<ACDQueue*>(listIterator())) != NULL)
   {
      OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDAgent::notifyAvailability - informing the queue(%s) that agent(%s) is available",
         pQueue->getUriString()->data(),
         mUriString.data());
      pQueue->agentAvailable(this);
   }

}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgent::setAvailable
//
//  SYNOPSIS:    void setAvailable()
//
//  DESCRIPTION: Set the mAvailable flag based on isAvailable() and if it becomes available
//                 call notifyAvailability()
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDAgent::setAvailable()
{
   mLock.acquire();

   if (isAvailable(false))
   {
      // See if this will transition from unvailable to available
      if (mAvailable == false)
      {
         // This agent is now available
         mAvailable = true;
         OsSysLog::add(FAC_ACD, PRI_INFO, "ACDAgent::setAvailable - ACDAgent(%s) is now AVAILABLE",
                       mUriString.data());
         // Let others know the good news.
         notifyAvailability();
      }
   }
   else
   {
      // See if this will cause a transition from available to unavailable
      if (mAvailable == true)
      {
         // Mark this agent as unavailable for calls
         mAvailable = false;
         OsSysLog::add(FAC_ACD, PRI_INFO, "ACDAgent::setAvailable - ACDAgent(%s) is now UNAVAILABLE",
                       mUriString.data());
      }
   }

   mLock.release();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgent::setOnHook
//
//  SYNOPSIS:    void setOnHook(bool)
//
//  DESCRIPTION: Set the mAvailable & mAgentFree flags true or false.
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDAgent::setOnHook(bool onHook)
{
   mLock.acquire();
   if (onHook)
   {
      // Mark agent ON_HOOK
      mAgentLineState |= LinePresenceBase::ON_HOOK ;
   }
   else
   {
      // Mark agent OFF_HOOK
      mAgentLineState &= ~LinePresenceBase::ON_HOOK ;
   }
   setAvailable() ;
   mLock.release();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgent::logSignIn
//
//  SYNOPSIS:    void logSignIn(bool)
//
//  DESCRIPTION: Log the fact the agent signed in/out
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDAgent::logSignIn(ACDQueue *pQueue, bool agentSignIn)
{
   ACDRtRecord* pACDRtRec = mpAcdAgentManager->getAcdServer()->getAcdRtRecord();

   // Add Agent Event
   if (NULL != pACDRtRec) {
      int dir = agentSignIn ? ACDRtRecord::SIGNED_IN_AGENT :
                              ACDRtRecord::SIGNED_OUT_AGENT ;
      pACDRtRec->appendAgentEvent(dir, pQueue->getUriString(), this);
   }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgent::setSignIn
//
//  SYNOPSIS:    void setSignIn(bool)
//
//  DESCRIPTION: Set the mAvailable flag true or false.
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDAgent::setSignIn(bool agentSignIn)
{
   mLock.acquire();

   ACDQueue* pQueue;

   pQueue = dynamic_cast<ACDQueue*>(mAcdQueueList.at(0));
   if (pQueue == NULL) {
      // The ACDQueue list is empty, try rebuilding
      buildACDQueueList();
   }

   // Iterate through the list of queues this agent is on
   UtlSListIterator listIterator(mAcdQueueList);
   while ((pQueue = dynamic_cast<ACDQueue*>(listIterator())) != NULL) {
      // Note sign in/out in the log for stats
      logSignIn(pQueue, agentSignIn) ;
   }

   if (!mAlwaysAvailable)  // If always available, ignore this.
   {
      if (agentSignIn)
      {
         // Mark agent signed in.
         mAgentLineState |= LinePresenceBase::SIGNED_IN ;
         // reset mBounceCount
         mBounceCount = 0 ;
      }
      else
      {
         // Mark agent signed out.
         mAgentLineState &= ~LinePresenceBase::SIGNED_IN ;
      }
      setAvailable() ;
   }
   mLock.release();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgent::handleMessage
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

UtlBoolean ACDAgent::handleMessage(OsMsg& rMessage)
{
   ACDAgentMsg*     pMessage;
   LinePresenceBase::ePresenceStateType type;
   bool             state;

//   osPrintf("ACDAgent::handleMessage - MsgType: %d, MsgSubType: %d\n", rMessage.getMsgType(), rMessage.getMsgSubType());
   if (rMessage.getMsgType() == OsMsg::USER_START) {
      switch (rMessage.getMsgSubType()) {
         case ACDAgentMsg::UPDATE_STATE:
            pMessage = (ACDAgentMsg*)&rMessage;
            type     = pMessage->getPresenceStateType();
            state    = pMessage->getPresenceState();
            // Update the agent state
            updateStateMessage(type, state, true);
            break;

         case ACDAgentMsg::DROP_CALL:
            // Drop the call
            pMessage = (ACDAgentMsg*)&rMessage;
            dropCallMessage(pMessage->getRna());
            break ;

         default:
            // Bad message
            OsSysLog::add(FAC_ACD, PRI_ERR, "ACDAgent::handleMessage - Received bad message");
            break;
      }

      return true;
   }
   else if (rMessage.getMsgType() == OsMsg::OS_EVENT) {

      uintptr_t timerSource;
      // Timer Event, determine which timer fired
      ((OsEventMsg&)rMessage).getUserData((void*&)timerSource);
      switch (timerSource) {
         case WRAP_UP_TIMER:
            OsSysLog::add(FAC_ACD, gACD_DEBUG,
                 "ACDAgent::handleMessage - Agent(%s) WRAP_UP_TIMER expired", getUriString()->data());
            handleWrapupTimeout();
            break;
         default:
            // Bad message
            OsSysLog::add(FAC_ACD, PRI_ERR, "ACDAgent::handleMessage - Received bad message");
            break;
      }
      return true;
   }
   else {
      // Otherwise, pass the message to the base for processing.
      return false;
   }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgent::updateStateMessage
//
//  SYNOPSIS:    void updateStateMessage(
//                 ePresenceStateType type,  Enumeration indicating which Presence State attribute
//                                           is to be updated.  These enumerations are defined
//                                           in <sipXcallLib/include/net/LinePresenceBase.h>
//                 bool               state  What to set the specified Presence State attribute to.
//
//  DESCRIPTION: This is the subclass implementation of the virtual LinePresenceBase::updateState().
//               It is called by the LinePresenceMonitor, if subscribed, to update the ACDAgents
//               availability based upon SIP dialog and presence information.
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     The associated state information is protected by semaphore locks to ensure
//               thread safe access, both from this function and from the associated functions
//               getState() and isAvailable().
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDAgent::updateStateMessage(ePresenceStateType type, bool state, bool recordIdle)
{
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDAgent(%s)::updateStateMessage - type: %d, state: %d",
                 mUriString.data(), type, state);

   mLock.acquire();

   // Convert the type and state into the actual status
   // e.g. type==ON_HOOK state==false means OFF_HOOK
   // This breaks out the status cases cleaner then if/then/else logic
   enum {DUNNO, ON_HOOK, OFF_HOOK, SIGNED_IN, SIGNED_OUT} status = DUNNO;
   switch (type)
   {
      case LinePresenceBase::ON_HOOK:
         status = state ? ON_HOOK : OFF_HOOK;
         break;
      case LinePresenceBase::SIGNED_IN:
         status = state ? SIGNED_IN : SIGNED_OUT;
         break;
      default:
         status = DUNNO;
         break;
   }

   // Now we can switch based on status
   switch(status)
   {
       case ON_HOOK:
          // if the agent goes ON_HOOK in the transfer mode with mPresence
          // Reset the IDLE time of this agent
          if (mMonitorPresence && true == recordIdle)
          {
             OsDateTime nowTime;
             OsDateTime::getCurTime(nowTime);
             nowTime.cvtToTimeSinceEpoch(mIdleTimeStart);
             OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDAgent::updateStateMessage ON_HOOK Agent(%s), idleTime (%d)",
                           getUriString()->data(), mIdleTimeStart.seconds());
          }
          else
          {
             OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDAgent::updateStateMessage ON_HOOK Agent(%s)",
                           getUriString()->data());
          }
          setOnHook(true);
          break;

       case OFF_HOOK:
          OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDAgent::updateStateMessage OFF_HOOK Agent(%s)",
                        getUriString()->data());
          setOnHook(false);
          break;

       case SIGNED_IN:
          OsSysLog::add(FAC_ACD, PRI_INFO, "ACDAgent::updateStateMessage - ACDAgent(%s) SIGNED IN",
             mUriString.data());
          if (mMonitorPresence)
          {
             mpAcdAgentManager->linePresenceSubscribe(this);
          }
          setSignIn(true);
         break;

       case SIGNED_OUT:
          OsSysLog::add(FAC_ACD, PRI_INFO, "ACDAgent::updateStateMessage - ACDAgent(%s) SIGNED OUT",
                        mUriString.data());
          if (mMonitorPresence)
          {
             mpAcdAgentManager->linePresenceUnsubscribe(this, NULL);
          }
          setSignIn(false);
          break;
       default:
          break;
   }
   mLock.release();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgent::dropCallMessage
//
//  SYNOPSIS:    void dropCallMessage(bool rna)
//
//  DESCRIPTION: A previously created outbound call to the ACDAgent will be dropped.
//               The ACDAgent idle time will be updated and the line presence state will
//               be set to ON-HOOK.
//
//               If rna is true, this agent rejected or timed out answering
//               the call to him.
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////
void ACDAgent::dropCallMessage(bool rna)
{
   mLock.acquire();

   // No longer on hold
   mOnHold = false;

   // Record the time that the agent went idle
   OsDateTime nowTime;
   OsDateTime::getCurTime(nowTime);
   nowTime.cvtToTimeSinceEpoch(mIdleTimeStart);
   // Debug
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDAgent::dropCallMessage Agent(%s), idleTime (%d) rna=%s",
                    getUriString()->data(), mIdleTimeStart.seconds(),
                    rna?"true":"false");

   if (rna)
   {
      // update mBounceCount
      mBounceCount++;

      // If mMaxBounceCount is greater than 0 and the agent has not
      // answered a call in that many tries, then sign him out if possible.
      if (mMaxBounceCount != 0 && mBounceCount>= mMaxBounceCount)
      {
         OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDAgent::dropCallMessage ACDAgent(%s) is being signed Out. mBounceCount=%d",
            getUriString()->data(), mBounceCount);

         signOut();
      }
   }


   // Drop the call
   if (mhCallHandle != SIPX_CALL_NULL)
   {
      sipxCallDestroy(mhCallHandle);
      mhCallHandle = SIPX_CALL_NULL ;
   }

   // Do not reset the hook state upon call disconnection if monitoring presence. We must
   // rely on NOTIFY events to do that, else we will make this agent available before the
   // device is really ready.
   if(!mMonitorPresence)
   {
      // Mark the agent as being on-hook
      setOnHook(true) ;
   }

   int wrapUpDelay = 0 ;
   if (mCallEstablished)
   {
      // Delay mWrapupTime if the call was answered
      wrapUpDelay = mWrapupTime ;

      // reset mBounceCount
      mBounceCount = 0 ;
   }
   else
   {
      // Delay mNonResponsiveTime if the call was not answered
      wrapUpDelay = mNonResponsiveTime ;
   }

   if(wrapUpDelay != 0)
   {
      // Wait the for the Wrapup timer to fire before marking the agent free
      OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDAgent::dropCallMessage Agent(%s), wrapUpDelay=%d",
                       getUriString()->data(),wrapUpDelay);

      mpWrapupTimer->oneshotAfter(OsTime(wrapUpDelay,0));
   }
   else
   {
      // Mark the agent as free
      setFree() ;
   }

   mLock.release();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgent::handleWrapupTimeout
//
//  SYNOPSIS:    void handleWrapupTimeout(void)
//
//  DESCRIPTION: A wrap-up time defines a period of time that has to pass until the ACD
//               transferrs a new caller to an agent after a call finished. After agent
//               complete a call, a wrap-up timer will be fired and agent's state will
//               stay busy until the wrap-up timer timeout.
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDAgent::handleWrapupTimeout()
{
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDAgent::handleWrapupTimeout  Agent(%s), wrapupTime = %d ", getUriString()->data(), mWrapupTime);

   mLock.acquire();
   setFree();
   mLock.release();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgent::signOut
//
//  SYNOPSIS:    void signOut(void)
//
//  DESCRIPTION: Send a sign-out request through XMLRPC to ACD presence
//               server.
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDAgent::signOut()
{
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDAgent::signOut Agent(%s) is to be bounced out.",
      mUriString.data());

   if (mAlwaysAvailable) {
      OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDAgent::signOut but Agent(%s) is always available",
         mUriString.data());
      return ;
   }

   // Mark signed out locally (so it happens fast)
   setSignIn(false) ;

   // Build the XML_RPC request to tell the ACD presence server
   // to sign-out the agent.
   OsStatus result = OS_FAILED;
   UtlHashMap requstData;
   UtlString* objectKey = new UtlString("object-class");
   UtlString* objectValue = new UtlString("login");
   requstData.insertKeyAndValue(objectKey, objectValue);
   UtlString* signOutKey = new UtlString("sign-out");
   UtlString* signOutValue = new UtlString(mUriString);
   requstData.insertKeyAndValue(signOutKey, signOutValue);

   Url target = mpAcdAgentManager->getPresenceServiceUrl();
   UtlString url = target.toString();

   XmlRpcRequest request(target,"action");
   request.addParam(&requstData);

   XmlRpcResponse response;
   result = request.execute(response) ? OS_SUCCESS : OS_FAILED;
   if (result != OS_SUCCESS)
   {
      // Try again (could be a temp failure)
      result = request.execute(response) ? OS_SUCCESS : OS_FAILED;
   }

   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDAgent::signOut PresenceServiceUrl = %s  agent(%s) signOut result is %d ", url.data(), mUriString.data(),result);
}
