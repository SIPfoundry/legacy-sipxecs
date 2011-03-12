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
#include <os/OsSysLog.h>
#include <tapi/sipXtapi.h>
#include <tapi/sipXtapiEvents.h>
#include <tapi/sipXtapiInternal.h>
#include <utl/UtlInt.h>
#include <utl/UtlHashMap.h>
#include <utl/UtlHashMapIterator.h>
#include <cp/CallManager.h>
#include "ACDServer.h"
#include "ACDCall.h"
#include "ACDLine.h"
#include "ACDLineManager.h"
#include "ACDCallManager.h"
#include "ACDAgentManager.h"
#include "ACDQueueManager.h"
#include "ACDQueue.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
extern OsSysLogPriority gACD_DEBUG;

// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallManager::ACDCallManager
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

ACDCallManager::ACDCallManager(ACDServer* pAcdServer, const int udpPort,
                               const int tcpPort, const int tlsPort,
                               const int rtpPortStart, const int maxConnections,
                               const char* pIdentity, const char* pBindToAddr,
                               bool useSequentialPorts)
: mLock(OsMutex::Q_FIFO)
{
   SIPX_RESULT rc;

   mAcdCallManagerHandle = SIPX_INST_NULL;
   mpAcdLineManager = NULL;
   mpAcdServer = pAcdServer;
   mSipPort = udpPort;
   mTotalCalls = 0;

   mpAcdAudioManager = mpAcdServer->getAcdAudioManager();

   // For each connection, we increase 10 times to handle the situation of RNA
   // for each incoming call.
   rc = sipxInitialize(&mAcdCallManagerHandle, udpPort, tcpPort, tlsPort, rtpPortStart,
                       maxConnections*10, pIdentity, pBindToAddr, useSequentialPorts);

   // sipXtapi failed to initialize, log the error
   if (rc != SIPX_RESULT_SUCCESS) {
      OsSysLog::add(FAC_ACD, PRI_CRIT, "ACDCallManager::ACDCallManager"
                    " sipxInitialize failed with error code: %d", rc);
      osPrintf("ACDCallManager::ACDCallManager - sipxInitialize failed with error code: %d\n", rc);
   }
   else
   {
      UtlString agentName;
      agentName.append("sipXacd (");
      agentName.append(PACKAGE_VERSION);
      agentName.append(")");

      rc = sipxConfigSetUserAgentName(mAcdCallManagerHandle,
                                      agentName.data(), true /* include platform id */);
   }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallManager::~ACDCallManager
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

ACDCallManager::~ACDCallManager()
{
   sipxEventListenerRemove(mAcdCallManagerHandle, ACDCallManager_EventCallback, static_cast<void*>(this));
   sipxUnInitialize(mAcdCallManagerHandle);
}

/* ============================ MANIPULATORS ============================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallManager::initialize
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

OsStatus ACDCallManager::initialize(void)
{
   mpAcdLineManager  = mpAcdServer->getAcdLineManager();
   mpAcdAudioManager = mpAcdServer->getAcdAudioManager();

   return OS_SUCCESS;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallManager::start
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

OsStatus ACDCallManager::start(void)
{
   sipxEventListenerAdd(mAcdCallManagerHandle, ACDCallManager_EventCallback, static_cast<void*>(this));

   return OS_SUCCESS;
}


/* ============================ ACCESSORS ================================= */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallManager::getAcdCallManagerHandle
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

SIPX_INST ACDCallManager::getAcdCallManagerHandle(void)
{
   return mAcdCallManagerHandle;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallManager::getAcdAudioManager
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

ACDAudioManager* ACDCallManager::getAcdAudioManager(void)
{
   return mpAcdAudioManager;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallManager::getSipPort
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

int ACDCallManager::getSipPort(void)
{
   return mSipPort;
}

/* ============================ INQUIRY =================================== */
////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallManager::verifyACDCallExists
//
//  SYNOPSIS:    check if the ACDCall reference is valid
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
bool ACDCallManager::verifyACDCallExists(ACDCall *pCallRef)
{
   return verifyCallInMap(pCallRef, &mACDCallExistsMap, "mACDCallExistsMap") ;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallManager::eventCallback
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

bool ACDCallManager::eventCallback(SIPX_EVENT_CATEGORY category, void *pInfo)
{
   ACDLine* pLineRef;

   if (pInfo == NULL)
      return false;

   mLock.acquire();

   SIPX_CALLSTATE_INFO* pCallInfo = static_cast<SIPX_CALLSTATE_INFO*>(pInfo);
   char sEvent[128] ;
   sipxEventToString(category, pInfo, sEvent, sizeof(sEvent)) ;


   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCallManager::eventCallback - SIPXTAPI event hCall=%d %s", pCallInfo->hCall, sEvent) ;

   if (category == EVENT_CATEGORY_CALLSTATE) {
      switch (pCallInfo->event) {
         case CALLSTATE_OFFERING:
            osPrintf("OFFERING(%d) - Line: %d, Call: %d\n", pCallInfo->cause, pCallInfo->hLine, pCallInfo->hCall);
            // An incoming call is being offered. First validate
            // that it is not a call that has been transferred to the ACD line
            if (TRUE == validateTransferToLine(pCallInfo)) {
               OsSysLog::add(FAC_ACD, PRI_WARNING, "ACDCallManager::eventCallback - CALLSTATE_OFFERING::%d to the ACD Line REJECTED",
                             pCallInfo->cause);
               sipxCallReject(pCallInfo->hCall, SIP_BAD_REQUEST_CODE,
                              "ACD Transfer Offer Rejected");
            }

            else {
            // Validate the line that it is arriving on.
            pLineRef = mpAcdLineManager->getAcdLineReference(pCallInfo->hLine);
            if (pLineRef == NULL) {
               // Call arrived on an invalid line. Reject it!
               sipxCallReject(pCallInfo->hCall, SIP_NOT_FOUND_CODE, "Invalid ACD Line Offered");
               osPrintf("OFFERING REJECTED - Line: %d, Call: %d\n", pCallInfo->hLine, pCallInfo->hCall);
               OsSysLog::add(FAC_ACD, PRI_WARNING, "ACDCallManager::eventCallback"
                             " - CALLSTATE_OFFERING::%d on LINE:%d INVALID LINE REJECTED",
                             pCallInfo->cause, pCallInfo->hLine);
            }
            else {
               // The line is valid.  Now see if the line will accept the call.
               if (pLineRef->isLineBusy()) {
                  // The line is busy.  Reject it!
                  sipxCallReject(pCallInfo->hCall, SIP_BUSY_CODE, "ACD Line Busy");
                  osPrintf("OFFERING REJECTED - Line: %d, Call: %d\n", pCallInfo->hLine, pCallInfo->hCall);
                  OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCallManager::eventCallback - CALLSTATE_OFFERING::%d on LINE:%d BUSY",
                                pCallInfo->cause, pCallInfo->hLine);
               }
               else {
                  // Before we accept the call, check whether we exceed the max number of calls allowed
                  if (mCallHandleMap.entries() < mpAcdServer->getMaxCallAllowed()) {
                     // Also check if there is any agent signed in !
                     UtlString mQueueUriString = pLineRef->getAcdQueue();
                     ACDQueue* pDestinationQueue = pLineRef->getAcdLineManager()->getAcdQueueManager()->getAcdQueueReference(mQueueUriString);
                     if ((pDestinationQueue != NULL) && ( pDestinationQueue->checkAgentAvailable() || pDestinationQueue->checkOverflowEntryAvailable())) {
                     // Accept the call!
                     sipxCallAccept(pCallInfo->hCall);
                     osPrintf("OFFERING ACCEPTED - Line: %d, Call: %d\n", pCallInfo->hLine, pCallInfo->hCall);
                     OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCallManager::eventCallback - CALLSTATE_OFFERING::%d on LINE:%d ACCEPTED",
                                   pCallInfo->cause, pCallInfo->hLine);
                     }
                     else {
                        // No signed in agent reject the call
                        sipxCallReject(pCallInfo->hCall, SIP_TEMPORARILY_UNAVAILABLE_CODE,
                                       "ACD No Agent");
                        OsSysLog::add(FAC_ACD, PRI_WARNING, "ACDCallManager::eventCallback - CALLSTATE_OFFERING::reject the call %d due to no signed in agent",
                                   pCallInfo->hCall);
                     }
                  }
                  else {
                     // We exceed the max allowed, reject the call
                     sipxCallReject(pCallInfo->hCall, SIP_TEMPORARILY_UNAVAILABLE_CODE,
                                    "ACD Maximum Calls Exceeded");
                     OsSysLog::add(FAC_ACD, PRI_WARNING, "ACDCallManager::eventCallback - CALLSTATE_OFFERING::reject the call %d due to exceed the max calls allowed (%zu)",
                                   pCallInfo->hCall, mCallHandleMap.entries());
                  }
               }
            }
            }
            break;

         case CALLSTATE_ALERTING:
            osPrintf("ALERTING(%d) - Line: %d, Call: %d\n", pCallInfo->cause, pCallInfo->hLine, pCallInfo->hCall);
            // Validate the incoming line
            pLineRef = mpAcdLineManager->getAcdLineReference(pCallInfo->hLine);
            if (pLineRef == NULL) {
               // Call arrived on an invalid line. Reject it!
               sipxCallReject(pCallInfo->hCall, SIP_NOT_FOUND_CODE, "Invalid ACD Line Alerting");
               OsSysLog::add(FAC_ACD, PRI_WARNING, "ACDCallManager::eventCallback - CALLSTATE_ALERTING::%d on LINE:%d INVALID LINE",
                             pCallInfo->cause, pCallInfo->hLine);
            }
            else {
               // Now create an ACDCall object to handle it.
               if (createACDCall(pLineRef, pCallInfo->hCall) != OS_SUCCESS) {
                  // The line rejected the call, drop it!
                  sipxCallReject(pCallInfo->hCall, SIP_BAD_REQUEST_CODE, "ACD Line Reject");
                  osPrintf("ALERTING REJECTED - Line: %d, Call: %d\n", pCallInfo->hLine, pCallInfo->hCall);
               }
               else {
                  // Success
                  updateCallState(pCallInfo);
               }
            }
            break;

         case CALLSTATE_CONNECTED:
            osPrintf("CONNECTED(%d) - Line: %d, Call: %d\n", pCallInfo->cause, pCallInfo->hLine, pCallInfo->hCall);
            updateCallState(pCallInfo);
            break;

         case CALLSTATE_DISCONNECTED:
            osPrintf("DISCONNECTED(%d) - Line: %d, Call: %d\n", pCallInfo->cause, pCallInfo->hLine, pCallInfo->hCall);
            if (updateCallState(pCallInfo) == OS_FAILED)
            {
               // If no one handled it, make sure to destroy the call
               // otherwise there is a handle leak.
               SIPX_CALL tmpCall = pCallInfo->hCall ;
               sipxCallDestroy(tmpCall) ;
            }
            break;

         case CALLSTATE_DESTROYED:
            osPrintf("DESTROYED(%d) - Line: %d, Call: %d\n", pCallInfo->cause, pCallInfo->hLine, pCallInfo->hCall);
            updateCallState(pCallInfo);
            break;

         case CALLSTATE_NEWCALL:
            OsSysLog::add(FAC_ACD, gACD_DEBUG,
                          "ACDCallManager::eventCallback - "
                          "CALLSTATE_NEWCALL::cause %d, hCallHandle %d hAssociateCallHandle %d remAddr %s",
                          pCallInfo->cause, pCallInfo->hCall, pCallInfo->hAssociatedCall, pCallInfo->remoteAddress);
            if (pCallInfo->cause == CALLSTATE_NEW_CALL_TRANSFER)
               updateTransferCallState(pCallInfo);
            break;
         case CALLSTATE_REMOTE_OFFERING:
            OsSysLog::add(FAC_ACD, gACD_DEBUG,
                          "ACDCallManager::eventCallback - "
                          "CALLSTATE_REMOTE_OFFERING::cause %d, hCallHandle %d hAssociateCallHandle %d remAddr %s",
                          pCallInfo->cause, pCallInfo->hCall, pCallInfo->hAssociatedCall, pCallInfo->remoteAddress);
            if (pCallInfo->cause == CALLSTATE_REMOTE_OFFERING_NORMAL)
               updateTransferCallState(pCallInfo);
            break ;

         case CALLSTATE_TRANSFER:
            updateCallState(pCallInfo);
            break;

         case CALLSTATE_DIALTONE:
         case CALLSTATE_REMOTE_ALERTING:
         case CALLSTATE_AUDIO_EVENT:
         case CALLSTATE_SECURITY_EVENT:
         case CALLSTATE_UNKNOWN:
            break;
      }
   }
   mLock.release();
   return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallManager::createACDCall
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

OsStatus ACDCallManager::createACDCall(ACDLine* pLineRef, SIPX_CALL hCallHandle)
{
   ACDCall* pCallRef;

   mLock.acquire();

   // Create a matching ACDCall object.
   pCallRef = new ACDCall(this, pLineRef, hCallHandle);
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCallManager::createACDCall - Call(%d), Object(%p) added",
                 hCallHandle, pCallRef);

   // Note the new ACDCalls existance
   mACDCallExistsMap.insertKeyAndValue(new UtlInt(hCallHandle), pCallRef);

   // Create a mapping between the sipXtapi call handle and the ACDCall
   mCallHandleMap.insertKeyAndValue(new UtlInt(hCallHandle), pCallRef);

   // Attempt to add the call to the associated line
   if (pLineRef->addCall(pCallRef)) {
      // Start the associated thread
      pCallRef->start();

      mTotalCalls++;

      OsSysLog::add(FAC_ACD, PRI_INFO, "ACDCallManager::createACDCall - Total number of calls received since the server started = %d",
                    mTotalCalls);

      mLock.release();

      return OS_SUCCESS;
   }
   else {
      // The line rejected the call
      destroyACDCall(pCallRef) ;

      OsSysLog::add(FAC_ACD, PRI_ERR, "ACDCallManager::createACDCall - Call(%d), Object(%p) rejected",
                    hCallHandle, pCallRef);

      mLock.release();

      return OS_FAILED;
   }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallManager::updateCallState
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

OsStatus ACDCallManager::updateCallState(SIPX_CALLSTATE_INFO* pCallInfo)
{
   SIPX_CALL hCallHandle;
   SIPX_LINE hLineHandle;
   int       callEvent;
   int       callCause;
   OsStatus status = OS_FAILED ;

   mLock.acquire();

   // Extract the call handle and state info
   hCallHandle = pCallInfo->hCall;
   hLineHandle = pCallInfo->hLine;
   callEvent   = pCallInfo->event;
   callCause   = pCallInfo->cause;

   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCallManager::updateCallState hCall=%d  hLine=%d callEvent %d callCause %d associated callHandle %d remoteAddr %s",
                    hCallHandle, hLineHandle, callEvent, callCause, pCallInfo->hAssociatedCall, pCallInfo->remoteAddress);

   // Search for the call handle in the various maps

   // Inbound calls are in the mCallHandleMap
   char *which = "mCallHandleMap" ;
   UtlHashMap *map = &mCallHandleMap ;
   UtlInt callKey(hCallHandle);
   UtlContainable *pValue = NULL ;
   pValue = mCallHandleMap.findValue(&callKey) ;

   if (pValue == NULL) {
      // Calls to agents are in the mAgentCallHandleMap
      which = "mAgentCallHandleMap" ;
      map = &mAgentCallHandleMap ;
      pValue = mAgentCallHandleMap.findValue(&callKey) ;
   }

   if (pValue == NULL) {
      // In transfer mode, sometimes calls end up in the mTransferCallHandleMap
      which = "mTransferCallHandleMap" ;
      map = &mTransferCallHandleMap ;
      pValue = mTransferCallHandleMap.findValue(&callKey) ;
   }

   if (pValue == NULL) {
      which = "mDeadCallHandleMap" ;
      map = &mDeadCallHandleMap ;
      pValue = mDeadCallHandleMap.findValue(&callKey) ;
   }

   if (pValue != NULL)
   {
      if (callEvent == CALLSTATE_DESTROYED) {
         // If event is destroyed, then that is the last we will see of this
         // handle.  Remove it from whatever map we found it in.
         OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCallManager::updateCallState - "
            "removing hCall=%d from %s", hCallHandle, which) ;
         map->removeReference(&callKey);
      }
      else
      {
         if (map != &mDeadCallHandleMap) {
            // Pass event along to the ACDCall we found
            ACDCall*  pCallRef = dynamic_cast<ACDCall*>(pValue);
            OsSysLog::add(FAC_ACD, gACD_DEBUG,
               "ACDCallManager::updateCallState - "
               "Found ACDCall Call(%d) hCall=%d Object(%p) using %s",
                  pCallRef->mhCallHandle, hCallHandle,
                  pCallRef, which) ;
            pCallRef->updateState(hCallHandle, callEvent, callCause);
         } else {
            OsSysLog::add(FAC_ACD, gACD_DEBUG,
              "ACDCallManager::updateCallState - ignoring dead hCall=%d events",
                 hCallHandle);
            // Kill the darn thing, just to make sure it goes away
            sipxCallDestroy(hCallHandle) ;
         }
      }
      status = OS_SUCCESS ;
   } else {
      OsSysLog::add(FAC_ACD, gACD_DEBUG,
         "ACDCallManager::updateCallState - did not find hCall=%d hLine=%d",
            hCallHandle, hLineHandle);
   }

   mLock.release();
   return status;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallManager::validateTransferToLine
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     bool.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

bool ACDCallManager::validateTransferToLine(SIPX_CALLSTATE_INFO* pCallInfo)
{
   SIPX_CALL hAssociatedCallHandle;
   int       callEvent;
   int       callCause;
   const char*     remUri;
   char      userUri[512];

   mLock.acquire();

   // Extract the call handle and state info
   hAssociatedCallHandle = pCallInfo->hAssociatedCall;
   callEvent   = pCallInfo->event;
   callCause   = pCallInfo->cause;
   remUri      = pCallInfo->remoteAddress;

   UtlString userId, hostAddress;
   Url remoteUrl(remUri);
   remoteUrl.getUserId(userId);
   remoteUrl.getHostAddress(hostAddress);

   if (remUri) {
      // Now find out if there is an agent for this remUri
      sprintf(userUri,"sip:%s@%s",userId.data(),hostAddress.data());
      UtlString agentUri(userUri);
      ACDAgent* pAgentRef =
         mpAcdServer->getAcdAgentManager()->getAcdAgentReference(agentUri);
      if ((pAgentRef) && (FALSE == pAgentRef->isFree())) {
         mLock.release();
         return TRUE;
      }
   }

   mLock.release();
   return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallManager::updateTransferCallState
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

void ACDCallManager::updateTransferCallState(SIPX_CALLSTATE_INFO* pCallInfo)
{
   SIPX_CALL hCallHandle = SIPX_CALL_NULL;
   SIPX_CALL hAssociatedCallHandle;
   int       callEvent;
   int       callCause;
   const char*     remUri;
   char      userUri[512];
   ACDCall*  pCallRef;

   mLock.acquire();


   // Extract the call handle and state info
   hAssociatedCallHandle = pCallInfo->hAssociatedCall;
   callEvent   = pCallInfo->event;
   callCause   = pCallInfo->cause;
   remUri      = pCallInfo->remoteAddress;

   /**
    * For the NEWCALL_TRANSFER event - find the ACDCall object instance
    * on the basis of the Associated call handle (from the older leg).
    */
   if (pCallInfo->cause == CALLSTATE_NEW_CALL_TRANSFER)
   {
      if (TRUE == validateTransferToLine(pCallInfo))
      {
         // Don't allow agents to transfer calls INTO the acd.  It screws
         // things up.  The correct behavior would be to move the call
         // the agent is currently handling into a new queue, but due to
         // the inability to remove calls from a conference, this just doesn't
         // work.  Hangup on the transfer attempt.  Ths should leave
         // caller and agent connected.
         OsSysLog::add(FAC_ACD, PRI_WARNING,
                       "ACDCallManager::updateTransferCallState - "
                       "CALLSTATE_OFFERING::%d to the ACD Line REJECTED",
                       pCallInfo->cause);
         sipxCallReject(pCallInfo->hCall, SIP_BAD_REQUEST_CODE, "Agent Transfer Loop Rejected");
         return ;
      }

      // not an agent transferring into the acd
      hCallHandle = hAssociatedCallHandle;
      UtlInt callKey(hCallHandle);
      pCallRef = dynamic_cast<ACDCall*>(mAgentCallHandleMap.findValue(&callKey));
   }
   else     // not new call transfer
   {
      UtlInt searchKey(pCallInfo->hCall);
      pCallRef = dynamic_cast<ACDCall*>(mTransferCallHandleMap.findValue(&searchKey));
   }

   if (pCallRef != NULL)
   {
      if (callCause == CALLSTATE_NEW_CALL_TRANSFER)
      {
         addMapTransferAgentCallHandleToCall(pCallInfo->hCall, pCallRef);
         pCallRef->mFlagTransfer = TRUE;    // only set TRUE here.
      }

      if (   (callCause == CALLSTATE_REMOTE_OFFERING_NORMAL)
          && (pCallRef->mFlagTransfer == TRUE))
      {
         UtlString userId, hostAddress;
         Url remoteUrl(remUri);
         remoteUrl.getUserId(userId);
         remoteUrl.getHostAddress(hostAddress);

         if (remUri)
         {
            // Now find the agent for this remUri
            sprintf(userUri,"sip:%s@%s",userId.data(),hostAddress.data());
            UtlString agentUri(userUri);
            ACDAgent* pAgentRef =
                     mpAcdServer->getAcdAgentManager()->getAcdAgentReference(agentUri);
            if (!pAgentRef)
            {
               OsSysLog::add(FAC_ACD, gACD_DEBUG,
                             "ACDCallManager::updateTransferCallState - "
                             "Failed to find Agent. This is probably an agent that is not signed in: "
                             "call(%d), TransferAgentCall(%d), agentUri(%s)",
                             pCallRef->getCallHandle(), pCallInfo->hCall, agentUri.data());
               // A non registered agent is not yet supported - so do not try !
               pAgentRef = mpAcdServer->getAcdAgentManager()->createACDAgent(userUri,
                                                                             "dummy", "",
                                                                             FALSE, FALSE, NULL, TRUE);

               if (!pAgentRef)
               {
                   assert(0);
               }
            }

            //set the mhCallHandle of the Agent object
            pAgentRef->setCallHandle(pCallInfo->hCall);
            // set the transfer agent object in the call object
            pCallRef->mpTransferAgent = pAgentRef;
            OsSysLog::add(FAC_ACD, gACD_DEBUG,
                          "ACDCallManager::updateTransferCallState - "
                          "success in finding Agent: call(%d), TransferAgentCall(%d) AgentUri(%s)",
                          pCallRef->getCallHandle(), pCallInfo->hCall, pAgentRef->getUriString()->data());
         }
      }

      if (   (callCause == CALLSTATE_REMOTE_OFFERING_NORMAL)
          && (pCallRef->mFlagTransfer == FALSE))
      {
           ; // do nothing
      }
      else
      {
           pCallRef->updateState(hCallHandle, callEvent, callCause);
      }
   }

   mLock.release();
   return;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallManager::removeCallFromMap
//
//  SYNOPSIS:    delete the ACDCall reference from the given HashMap
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
void ACDCallManager::removeCallFromMap(ACDCall *pCallRef,
   UtlHashMap *pMap, char *mapName)
{
   mLock.acquire();

   UtlHashMapIterator mapIterator(*pMap);
   ACDCall* pACDCall = NULL;
   UtlInt* pTemp = NULL;
   UtlContainable* pKey;

   while ((pTemp = (UtlInt*)mapIterator()) != NULL) {
      pACDCall = (ACDCall*)pMap->findValue(pTemp);
      if(pACDCall == pCallRef ) {
         OsSysLog::add(FAC_ACD, gACD_DEBUG,
            "ACDCallManager::removeCallFromMap(%s) - removed ACDCall(%p) matching hCall=%" PRIdPTR,
               mapName, pACDCall, pTemp->getValue());
         pKey = pMap->removeReference(pTemp);
         if (pKey != NULL) {
            if (pMap != &mDeadCallHandleMap) {
               // Add it to the "dead" list so we know to ignore messages
               // from it Yet can tell it isn't "unknown".
               addDeadCallToMap(pTemp->getValue(), pCallRef) ;
            }
            delete pKey ;
         }
      }
   }
   mLock.release();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallManager::verifyCallInMap
//
//  SYNOPSIS:    check if the ACDCall reference is in the given HashMap
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
bool ACDCallManager::verifyCallInMap(ACDCall *pCallRef,
   UtlHashMap *pMap, char *mapName)
{
   mLock.acquire();

   UtlHashMapIterator mapIterator(*pMap);
   ACDCall* pACDCall = NULL;
   UtlInt* pTemp = NULL;
   bool found = false ;

   while ((pTemp = (UtlInt*)mapIterator()) != NULL) {
      pACDCall = (ACDCall*)pMap->findValue(pTemp);
      if(pACDCall == pCallRef ) {
         found = true ;
         break ;
         }
      }

   if (found) {
      OsSysLog::add(FAC_ACD, gACD_DEBUG,
         "ACDCallManager::verifyCallInMap(%s) - found ACDCall(%p)",
            mapName, pCallRef);
   }
   else {
      OsSysLog::add(FAC_ACD, gACD_DEBUG,
         "ACDCallManager::verifyCallInMap(%s) - did not find ACDCall(%p)",
            mapName, pCallRef);
   }
   mLock.release();

   return found ;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallManager::removeCallFromMaps
//
//  SYNOPSIS:    delete the ACDCall reference from all HashMaps
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
void ACDCallManager::removeCallFromMaps(ACDCall *pCallRef)
{
   mLock.acquire();

   // The ACD call no longer exists
   removeCallFromMap(pCallRef, &mACDCallExistsMap, "mACDCallExistsMap") ;

   // Remove the mapping between all sipXtapi call handles that use
   // the ACDCall instance.
   removeCallFromMap(pCallRef, &mCallHandleMap, "mCallHandleMap") ;
   removeCallFromMap(pCallRef, &mAgentCallHandleMap, "mAgentCallHandleMap") ;
   removeCallFromMap(pCallRef, &mTransferCallHandleMap, "mTransferCallHandleMap") ;

   // But not the mDeadCallHandleMap, keep that
//   removeCallFromMap(pCallRef, &mDeadCallHandleMap, "mDeadCallHandleMap") ;
   mLock.release();

}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallManager::destroyACDCall
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

void ACDCallManager::destroyACDCall(ACDCall* pCallRef)
{
//   SIPX_CALL hCallHandle = pCallRef->getCallHandle();

   // Ignore events to this call handle from now on.
   // Moves still active handles into the "Dead" map
   // so it won't cast to ACDCALL, as this pointer is invalid from this point
   // Also ACDQueue will ignore (old) messages with this call ref
   removeCallFromMaps(pCallRef) ;

   // Signal the ACDCall's associated task to shutdown,
   // and wait for it do go away.
   pCallRef->waitUntilShutDown(20000);


   // ACDCall's queue is stopped, no one else should be
   // using it.  Blow it all away
   pCallRef->destroyACDCall() ;

   // Now delete it
   delete pCallRef ;


   return;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallManager::addDeadCallToMap(void)
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
void ACDCallManager::addDeadCallToMap(SIPX_CALL hCall, UtlContainable* pCall)
{
   if (!pCall)
     return;

   mLock.acquire();

   UtlInt callKey(hCall);
   UtlContainable* pOld = mDeadCallHandleMap.findValue(&callKey) ;
   if (!pOld) {
      mDeadCallHandleMap.insertKeyAndValue(new UtlInt(hCall), pCall);
      OsSysLog::add(FAC_ACD, PRI_INFO,
         "ACDCallManager::addDeadCallToMap added hCall=%d ACDCall(%p)",
            hCall, pCall);
   }

   mLock.release();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallManager::addMapAgentCallHandleToCall(void)
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
void ACDCallManager::addMapAgentCallHandleToCall(SIPX_CALL hCall, ACDCall* pCallRef)
{
   if (!pCallRef)
     return;
   mLock.acquire();

   UtlInt callKey(hCall);
   ACDCall* pCall = dynamic_cast<ACDCall*>(mAgentCallHandleMap.findValue(&callKey));
   if (!pCall) {
      mAgentCallHandleMap.insertKeyAndValue(new UtlInt(hCall), pCallRef);
      OsSysLog::add(FAC_ACD, PRI_INFO,
         "ACDCallManager::addMapAgent added hCall=%d ACDCall(%p)", hCall, pCallRef);
   }
   else {
      OsSysLog::add(FAC_ACD, PRI_ERR,
         "ACDCallManager::addMapAgent failed because hCall=%d already there.",
            hCall);
   }

   mLock.release();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallManager::removeMapAgentCallHandleToCall(void)
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
//
////////////////////////////////////////////////////////////////////////////////////////////////////
void ACDCallManager::removeMapAgentCallHandleToCall(SIPX_CALL hCall)
{
   UtlContainable* pKey;
   UtlContainable* pValue;
   // Remove the mapping between the sipXtapi call handle and the ACDCall instance.

   if (hCall == SIPX_CALL_NULL)
      return ;

   mLock.acquire();
   const UtlInt searchKey(hCall);
   pKey = mAgentCallHandleMap.removeKeyAndValue(&searchKey, pValue);
   if (pKey != NULL) {
      OsSysLog::add(FAC_ACD, gACD_DEBUG,
         "ACDCallManager::removeMapAgent - removed hCall=%d ACDCall(%p)",
            hCall, pValue);
      delete pKey ;

      // Add it to the "dead" list so we know to ignore messages from it,
      // Yet can tell it isn't "unknown".
      addDeadCallToMap(hCall, pValue) ;
   }

   mLock.release();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallManager::addMapTransferAgentCallHandleToCall(SIPX_CALL hCall, ACDCall* pCallRef)
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

void ACDCallManager::addMapTransferAgentCallHandleToCall(SIPX_CALL hCall, ACDCall* pCallRef)
{
   if (!pCallRef)
     return;
   mLock.acquire();

   UtlInt callKey(hCall);
   ACDCall* pCall = dynamic_cast<ACDCall*>(mTransferCallHandleMap.findValue(&callKey));
   if (!pCall)
      mTransferCallHandleMap.insertKeyAndValue(new UtlInt(hCall), pCallRef);

   mLock.release();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:     ACDCallManager::removeMapTransferAgentCallHandleToCall(SIPX_CALL hCall)
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
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDCallManager::removeMapTransferAgentCallHandleToCall(SIPX_CALL hCall)
{
   UtlContainable* pKey;
   // Remove the mapping between the sipXtapi call handle and the ACDCall instance.
   mLock.acquire();
   const UtlInt searchKey(hCall);
   pKey = mTransferCallHandleMap.removeReference(&searchKey);
   if (pKey == NULL) {
   // Error. Did not find a matching associated call handle.
      OsSysLog::add(FAC_ACD, PRI_ERR, "ACDCallManager::removeMap - Failed to find associated call handle: %d",
                 hCall);
      delete pKey ;
   }
   mLock.release();
}

#ifdef CML
////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  CML
//
//  NAME:        ACDCallManager::getAcdCallByCallId
//
//  SYNOPSIS:
//
//  DESCRIPTION: Returns an ACDCall pointer that matches a call ID. Used for call pickup.
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ACDCall* ACDCallManager::getAcdCallByCallId(UtlString callId) {
   UtlHashMapIterator mapIterator(mCallHandleMap);
   ACDCall* pACDCall = NULL;
   UtlInt* pTemp = NULL;

   while ((pTemp = (UtlInt*)mapIterator()) != NULL) {
      OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCallManager::getAcdCallByCallId - looking at call handle=%d",pTemp->getValue());
      pACDCall = (ACDCall*)mCallHandleMap.findValue(pTemp);

      if(pACDCall && pACDCall->getCallId() == callId ) {
         OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCallManager::getAcdCallByCallId - found ACDCall matching callid=%s",callId.data());
         return pACDCall;
      }
   }

   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDCallManager::getAcdCallByCallId - could not find ACDCall matching callid=%s",callId.data());
   return NULL;
}
#endif

/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallManager::isThereActiveCalls
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

bool ACDCallManager::isThereActiveCalls()
{
   return !mCallHandleMap.isEmpty();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDCallManager_EventCallback
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

bool ACDCallManager_EventCallback(SIPX_EVENT_CATEGORY category,
                                  void* pInfo,
                                  void* pUserData) {
   return static_cast<ACDCallManager*>(pUserData)->eventCallback(category, pInfo);
}
