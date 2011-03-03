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
#include <xmlparser/tinyxml.h>
#include <net/ProvisioningAgent.h>
#include "ACDServer.h"
#include "ACDCallManager.h"
#include "ACDLineManager.h"
#include "ACDQueue.h"
#include "ACDQueue_Circular.h"
#include "ACDQueue_Linear.h"
#include "ACDQueue_LongestIdle.h"
#include "ACDQueue_RingAll.h"
#include "ACDQueueManager.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
extern OsSysLogPriority gACD_DEBUG;

// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueueManager::ACDQueueManager
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

ACDQueueManager::ACDQueueManager(ACDServer* pAcdServer)
: ProvisioningClass(ACD_QUEUE_TAG), mLock(OsMutex::Q_FIFO)
{
   mpAcdServer = pAcdServer;
   mpAcdCallManager = NULL;
   mhAcdCallManagerHandle = NULL;
   mpAcdAgentManager = NULL;
   mpAcdLineManager = NULL;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueueManager::~ACDQueueManager
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

ACDQueueManager::~ACDQueueManager()
{
}

/* ============================ MANIPULATORS ============================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueueManager::initialize
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

OsStatus ACDQueueManager::initialize(void)
{
   mpAcdCallManager       = mpAcdServer->getAcdCallManager();
   mhAcdCallManagerHandle = mpAcdCallManager->getAcdCallManagerHandle();
   mpAcdAgentManager      = mpAcdServer->getAcdAgentManager();
   mpAcdLineManager       = mpAcdServer->getAcdLineManager();

   // Register this with the Provisioning Agent
   mpAcdServer->getProvisioningAgent()->registerClass(this);

   return OS_SUCCESS;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueueManager::start
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

OsStatus ACDQueueManager::start(void)
{
   // Load the configuration
   loadConfiguration();

   return OS_SUCCESS;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueueManager::createACDQueue
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

ACDQueue* ACDQueueManager::createACDQueue(const char* pQueueUriString,
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
                                          const char* pExternalLineList)
{
   ACDQueue*  pQueueRef;

   mLock.acquire();

   // Create the ACDQueue instance.
   switch(acdScheme)
   {
      case ACDQueue::CIRCULAR:
         pQueueRef = new ACDQueue_Circular(this, pQueueUriString, pName,
            acdScheme, maxRingDelay, maxQueueDepth, maxWaitTime, fifoOverflow,
            pOverflowQueue, pOverflowEntry, pOverflowType, answerMode, callConnectScheme,
            pWelcomeAudio, bargeIn, pQueueAudio, pBackgroundAudio,
            queueAudioInterval, pCallTerminationAudio, terminationToneDuration,
            agentsWrapupTime, agentsNonResponsiveTime, maxBounceCount,
            pAcdAgentList, pExternalLineList);

         break ;

      case ACDQueue::LINEAR:
         pQueueRef = new ACDQueue_Linear(this, pQueueUriString, pName,
            acdScheme, maxRingDelay, maxQueueDepth, maxWaitTime, fifoOverflow,
            pOverflowQueue, pOverflowEntry,  pOverflowType, answerMode, callConnectScheme,
            pWelcomeAudio, bargeIn, pQueueAudio, pBackgroundAudio,
            queueAudioInterval, pCallTerminationAudio, terminationToneDuration,
            agentsWrapupTime, agentsNonResponsiveTime, maxBounceCount,
            pAcdAgentList, pExternalLineList);

         break ;

      case ACDQueue::LONGEST_IDLE:
         pQueueRef = new ACDQueue_LongestIdle(this, pQueueUriString, pName,
            acdScheme, maxRingDelay, maxQueueDepth, maxWaitTime, fifoOverflow,
            pOverflowQueue, pOverflowEntry, pOverflowType,  answerMode, callConnectScheme,
            pWelcomeAudio, bargeIn, pQueueAudio, pBackgroundAudio,
            queueAudioInterval, pCallTerminationAudio, terminationToneDuration,
            agentsWrapupTime, agentsNonResponsiveTime, maxBounceCount,
            pAcdAgentList, pExternalLineList);

         break ;

      case ACDQueue::RING_ALL:
         pQueueRef = new ACDQueue_RingAll(this, pQueueUriString, pName,
            acdScheme, maxRingDelay, maxQueueDepth, maxWaitTime, fifoOverflow,
            pOverflowQueue, pOverflowEntry, pOverflowType, answerMode, callConnectScheme,
            pWelcomeAudio, bargeIn, pQueueAudio, pBackgroundAudio,
            queueAudioInterval, pCallTerminationAudio, terminationToneDuration,
            agentsWrapupTime, agentsNonResponsiveTime, maxBounceCount,
            pAcdAgentList, pExternalLineList);
         break ;

      default:
         OsSysLog::add(FAC_ACD, PRI_CRIT, "ACDQueueManager::createACDAQueue- Queue scheme unknown for %s!",
            pQueueUriString);
         abort();
   }

   // Start it up
   pQueueRef->start();

   // Create a mapping between the ACDQueue URI and the ACDQueue instance.
   mAcdQueueList.insertKeyAndValue(new UtlString(pQueueUriString), pQueueRef);

   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDQueueManager::createACDAQueue- Queue added: %s",
                 pQueueUriString);

   mLock.release();

   return pQueueRef;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueueManager::deleteACDQueue
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

void ACDQueueManager::deleteACDQueue(const char* pQueueUriString)
{
   UtlContainable* pQueueRef;
   UtlContainable* pKey;

   mLock.acquire();

   // Remove the mapping between the ACDQueue URI and the ACDQueue instance.
   const UtlString searchUriKey(pQueueUriString);
   pKey = mAcdQueueList.removeKeyAndValue(&searchUriKey, pQueueRef);
   if (pKey == NULL) {
      // Error. Did not find a matching ACDAgent object.
      OsSysLog::add(FAC_ACD, PRI_ERR, "ACDQueueManager::deleteACDQueue - Failed to find reference to Queue: %s",
                    pQueueUriString);
      mLock.release();
      return;
   }
   delete pKey;

   // Signal the ACDQueue's associated task to shutdown
   dynamic_cast<ACDQueue*>(pQueueRef)->requestShutdown();
   while (!dynamic_cast<ACDQueue*>(pQueueRef)->isShutDown()) {
      // Wait for it to complete
   }

   // Finally delete the ACDQueue object.
   delete pQueueRef;

   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDQueueManager::deleteACDQueue - Queue: %s deleted",
                 pQueueUriString);

   mLock.release();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueueManager::Create
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

ProvisioningAttrList* ACDQueueManager::Create(ProvisioningAttrList& rRequestAttributes)
{
   ProvisioningAttrList* pResponse;

   UtlString             queueUriString;
   UtlString             name;
   UtlString             overflowDestination;
   UtlString             overflowEntry;
   UtlString             overflowType;
   UtlString             welcomeAudio;
   UtlString             queueAudio;
   UtlString             backgroundAudio;
   UtlString             callTerminationAudio;
   UtlString             acdAgentList;
   UtlString             acdLineList;
   int                   acdScheme;
   int                   maxRingDelay;
   int                   maxQueueDepth;
   int                   maxWaitTime;
   int                   answerMode;
   int                   callConnectScheme;
   int                   queueAudioInterval;
   int                   terminationToneDuration;
   int                   agentsWrapupTime;
   int                   agentsNonResponsiveTime;
   int                   maxBounceCount;
   bool                  fifoOverflow;
   bool                  bargeIn;

   osPrintf("{method} = create\n{object-class} = acd-queue\n");
   rRequestAttributes.dumpAttributes();
   osPrintf("\n");

   // Verify that the required set of attributes are there.
   try {
      rRequestAttributes.validateAttribute(QUEUE_URI_TAG,                 ProvisioningAttrList::STRING);
      rRequestAttributes.validateAttribute(QUEUE_ACD_SCHEME_TAG,          ProvisioningAttrList::INT);
      rRequestAttributes.validateAttribute(QUEUE_MAX_RING_DELAY_TAG,      ProvisioningAttrList::INT);
      rRequestAttributes.validateAttribute(QUEUE_MAX_QUEUE_DEPTH_TAG,     ProvisioningAttrList::INT);
      rRequestAttributes.validateAttribute(QUEUE_MAX_WAIT_TIME_TAG,       ProvisioningAttrList::INT);
      rRequestAttributes.validateAttribute(QUEUE_FIFO_OVERFLOW_TAG,       ProvisioningAttrList::BOOL);
      rRequestAttributes.validateAttribute(QUEUE_ANSWER_MODE_TAG,         ProvisioningAttrList::INT);
      rRequestAttributes.validateAttribute(QUEUE_CALL_CONNECT_SCHEME_TAG, ProvisioningAttrList::INT);
   }
   catch (UtlString error) {
      // We're missing at least one mandatory attribute.
      pResponse = new ProvisioningAttrList;
      pResponse->setAttribute("method-name", "create");
      pResponse->setAttribute("result-code", ProvisioningAgent::MISSING_ATTR);
      pResponse->setAttribute("result-text", error);
      return pResponse;
   }

   // Extract the instance index from the request attributes.
   rRequestAttributes.getAttribute(QUEUE_URI_TAG, queueUriString);

   mLock.acquire();

   // Verify that an instance of this object has not already been created.
   TiXmlNode* pInstanceNode = findPSInstance(ACD_QUEUE_TAG, QUEUE_URI_TAG, queueUriString);
   if (pInstanceNode != NULL) {
      //The instance has already been created, send back the response.
      mLock.release();
      pResponse = new ProvisioningAttrList;
      pResponse->setAttribute("method-name", "create");
#ifdef CML
      pResponse->setAttribute("result-code", ProvisioningAgent::ALREADY_EXISTS);
#else
      pResponse->setAttribute("result-code", ProvisioningAgent::DUPLICATE);
#endif
      pResponse->setAttribute("result-text", "Managed Object Instance already exists");
      return pResponse;
   }

   // Validate that the optional attribute types are correct, ignoring any that are missing
   try {
      rRequestAttributes.validateAttributeType(QUEUE_NAME_TAG,                      ProvisioningAttrList::STRING);
      rRequestAttributes.validateAttributeType(QUEUE_OVERFLOW_DESTINATION_TAG,      ProvisioningAttrList::STRING);
      rRequestAttributes.validateAttributeType(QUEUE_WELCOME_AUDIO_TAG,             ProvisioningAttrList::STRING);
      rRequestAttributes.validateAttributeType(QUEUE_BARGE_IN_TAG,                  ProvisioningAttrList::BOOL);
      rRequestAttributes.validateAttributeType(QUEUE_QUEUE_AUDIO_TAG,               ProvisioningAttrList::STRING);
      rRequestAttributes.validateAttributeType(QUEUE_BACKGROUND_AUDIO_TAG,          ProvisioningAttrList::STRING);
      rRequestAttributes.validateAttributeType(QUEUE_QUEUE_AUDIO_INTERVAL_TAG,      ProvisioningAttrList::INT);
      rRequestAttributes.validateAttributeType(QUEUE_CALL_TERMINATION_AUDIO_TAG,    ProvisioningAttrList::STRING);
      rRequestAttributes.validateAttributeType(QUEUE_TERMINATION_TONE_DURATION_TAG, ProvisioningAttrList::INT);
      rRequestAttributes.validateAttributeType(QUEUE_AGENTS_WRAP_UP_TIME_TAG,       ProvisioningAttrList::INT);
      rRequestAttributes.validateAttributeType(QUEUE_AGENTS_NON_RESPONSIVE_TIME_TAG,ProvisioningAttrList::INT);
      rRequestAttributes.validateAttributeType(QUEUE_MAX_BOUNCE_COUNT_TAG,          ProvisioningAttrList::INT);
      rRequestAttributes.validateAttributeType(QUEUE_ACD_AGENT_LIST_TAG,            ProvisioningAttrList::STRING);
      rRequestAttributes.validateAttributeType(QUEUE_ACD_LINE_LIST_TAG,             ProvisioningAttrList::STRING);
   }
   catch (UtlString error) {
      // One of the optional attributes is not set to the correct type
      mLock.release();
      pResponse = new ProvisioningAttrList;
      pResponse->setAttribute("method-name", "create");
      pResponse->setAttribute("result-code", ProvisioningAgent::INVALID_ATTR);
      pResponse->setAttribute("result-text", error);
      return pResponse;
   }

   // Create the instance in the configuration file
   pInstanceNode = createPSInstance(ACD_QUEUE_TAG, QUEUE_URI_TAG, queueUriString);
   if (pInstanceNode == NULL) {
      // Instance cacquirereation failed.
      mLock.release();
      pResponse = new ProvisioningAttrList;
      pResponse->setAttribute("method-name", "create");
      pResponse->setAttribute("result-code", ProvisioningAgent::UNDEFINED);
      pResponse->setAttribute("result-text", "Managed Object Instance creation failed");
      return pResponse;
   }

   // Now save the individual attributes
   // name (optional)
   if (rRequestAttributes.getAttribute(QUEUE_NAME_TAG, name)) {
      setPSAttribute(pInstanceNode, QUEUE_NAME_TAG, name);
   }

   // acd-scheme
   rRequestAttributes.getAttribute(QUEUE_ACD_SCHEME_TAG, acdScheme);
   setPSAttribute(pInstanceNode, QUEUE_ACD_SCHEME_TAG, acdScheme);

   // max-ring-delay
   rRequestAttributes.getAttribute(QUEUE_MAX_RING_DELAY_TAG, maxRingDelay);
   setPSAttribute(pInstanceNode, QUEUE_MAX_RING_DELAY_TAG, maxRingDelay);

   // max-queue-depth
   rRequestAttributes.getAttribute(QUEUE_MAX_QUEUE_DEPTH_TAG, maxQueueDepth);
   setPSAttribute(pInstanceNode, QUEUE_MAX_QUEUE_DEPTH_TAG, maxQueueDepth);

   // max-wait-time
   rRequestAttributes.getAttribute(QUEUE_MAX_WAIT_TIME_TAG, maxWaitTime);
   setPSAttribute(pInstanceNode, QUEUE_MAX_WAIT_TIME_TAG, maxWaitTime);

   // fifo-overflow
   rRequestAttributes.getAttribute(QUEUE_FIFO_OVERFLOW_TAG, fifoOverflow);
   setPSAttribute(pInstanceNode, QUEUE_FIFO_OVERFLOW_TAG, fifoOverflow);

   // overflow-destination (optional)
   if (rRequestAttributes.getAttribute(QUEUE_OVERFLOW_DESTINATION_TAG, overflowDestination)) {
      setPSAttribute(pInstanceNode, QUEUE_OVERFLOW_DESTINATION_TAG, overflowDestination);
   }

   // overflow-entry (optional)
   if (rRequestAttributes.getAttribute(QUEUE_OVERFLOW_ENTRY_TAG, overflowEntry)) {
      setPSAttribute(pInstanceNode, QUEUE_OVERFLOW_ENTRY_TAG, overflowEntry);
   }

   // overflow-type (optional)
   if (rRequestAttributes.getAttribute(QUEUE_OVERFLOW_TYPE_TAG, overflowType)) {
      setPSAttribute(pInstanceNode, QUEUE_OVERFLOW_TYPE_TAG, overflowType);
   }

   // answer-mode
   rRequestAttributes.getAttribute(QUEUE_ANSWER_MODE_TAG, answerMode);
   setPSAttribute(pInstanceNode, QUEUE_ANSWER_MODE_TAG, answerMode);

   // call-connect-scheme
   rRequestAttributes.getAttribute(QUEUE_CALL_CONNECT_SCHEME_TAG, callConnectScheme);
   setPSAttribute(pInstanceNode, QUEUE_CALL_CONNECT_SCHEME_TAG, callConnectScheme);

   // welcome-audio (optional)
   if (rRequestAttributes.getAttribute(QUEUE_WELCOME_AUDIO_TAG, welcomeAudio)) {
      setPSAttribute(pInstanceNode, QUEUE_WELCOME_AUDIO_TAG, welcomeAudio);
   }

   // barge-in (optional)
   if (rRequestAttributes.getAttribute(QUEUE_BARGE_IN_TAG, bargeIn)) {
      setPSAttribute(pInstanceNode, QUEUE_BARGE_IN_TAG, bargeIn);
   }

   // queue-audio (optional)
   if (rRequestAttributes.getAttribute(QUEUE_QUEUE_AUDIO_TAG, queueAudio)) {
      setPSAttribute(pInstanceNode, QUEUE_QUEUE_AUDIO_TAG, queueAudio);
   }

   // background-audio (optional)
   if (rRequestAttributes.getAttribute(QUEUE_BACKGROUND_AUDIO_TAG, backgroundAudio)) {
      setPSAttribute(pInstanceNode, QUEUE_BACKGROUND_AUDIO_TAG, backgroundAudio);
   }

   // queue-audio-interval (optional)
   if (rRequestAttributes.getAttribute(QUEUE_QUEUE_AUDIO_INTERVAL_TAG, queueAudioInterval)) {
      setPSAttribute(pInstanceNode, QUEUE_QUEUE_AUDIO_INTERVAL_TAG, queueAudioInterval);
   }

   // call-termination-audio (optional)
   if (rRequestAttributes.getAttribute(QUEUE_CALL_TERMINATION_AUDIO_TAG, callTerminationAudio)) {
      setPSAttribute(pInstanceNode, QUEUE_CALL_TERMINATION_AUDIO_TAG, callTerminationAudio);
   }

   // termination-tone-duration (optional)
   if (rRequestAttributes.getAttribute(QUEUE_TERMINATION_TONE_DURATION_TAG, terminationToneDuration)) {
      setPSAttribute(pInstanceNode, QUEUE_TERMINATION_TONE_DURATION_TAG, terminationToneDuration);
   }

   // agents-wrap-up-time (optional)
   if (rRequestAttributes.getAttribute(QUEUE_AGENTS_WRAP_UP_TIME_TAG, agentsWrapupTime)) {
      setPSAttribute(pInstanceNode, QUEUE_AGENTS_WRAP_UP_TIME_TAG, agentsWrapupTime);
   }

   // agents-non-responsive-time (optional)
   if (rRequestAttributes.getAttribute(QUEUE_AGENTS_NON_RESPONSIVE_TIME_TAG, agentsNonResponsiveTime)) {
      setPSAttribute(pInstanceNode, QUEUE_AGENTS_NON_RESPONSIVE_TIME_TAG, agentsNonResponsiveTime);
   }

   // max-bounces (optional)
   if (rRequestAttributes.getAttribute(QUEUE_MAX_BOUNCE_COUNT_TAG, maxBounceCount)) {
      setPSAttribute(pInstanceNode, QUEUE_MAX_BOUNCE_COUNT_TAG, maxBounceCount);
   }

   // acd-agent-list (optional)
   if (rRequestAttributes.getAttribute(QUEUE_ACD_AGENT_LIST_TAG, acdAgentList)) {
      setPSAttribute(pInstanceNode, QUEUE_ACD_AGENT_LIST_TAG, acdAgentList);
   }

   // external-line-list (optional)
   if (rRequestAttributes.getAttribute(QUEUE_ACD_LINE_LIST_TAG, acdLineList)) {
      setPSAttribute(pInstanceNode, QUEUE_ACD_LINE_LIST_TAG, acdLineList);
   }

   // If Administrative State == ACTIVE, create the ACDQueue
   if (mpAcdServer->getAdministrativeState() == ACDServer::ACTIVE) {
      createACDQueue(queueUriString, name, acdScheme,
                     maxRingDelay, maxQueueDepth, maxWaitTime, fifoOverflow,
                     overflowDestination,overflowEntry, overflowType, answerMode, callConnectScheme,
                     welcomeAudio, bargeIn, queueAudio,
                     backgroundAudio, queueAudioInterval,
                     callTerminationAudio, terminationToneDuration,
                     agentsWrapupTime, agentsNonResponsiveTime, maxBounceCount,
                     acdAgentList, acdLineList);
   }

   // Update the configuration file
   OsSysLog::add(LOG_FACILITY, PRI_INFO, "ACDQueueManager::Create - Updating the config file");
   mpXmlConfigDoc->SaveFile();

   mLock.release();

   // Build up the response.
   pResponse = new ProvisioningAttrList;
   pResponse->setAttribute("method-name", "create");
   pResponse->setAttribute("result-code", ProvisioningAgent::SUCCESS);
   pResponse->setAttribute("result-text", "SUCCESS");
   return pResponse;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueueManager::Delete
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

ProvisioningAttrList* ACDQueueManager::Delete(ProvisioningAttrList& rRequestAttributes)
{
   ProvisioningAttrList* pResponse;
   UtlString             queueUriString;

   osPrintf("{method} = delete\n{object-class} = acd-queue\n");
   rRequestAttributes.dumpAttributes();
   osPrintf("\n");

   // Extract the instance index from the request attributes.
   rRequestAttributes.getAttribute(QUEUE_URI_TAG, queueUriString);

   mLock.acquire();

   // Verify that the instance has already been created.
   TiXmlNode* pInstanceNode = findPSInstance(ACD_QUEUE_TAG, QUEUE_URI_TAG, queueUriString);
   if (pInstanceNode == NULL) {
      // There is no instance.
      mLock.release();
      pResponse = new ProvisioningAttrList;
      pResponse->setAttribute("method-name", "delete");
      pResponse->setAttribute("result-code", ProvisioningAgent::UNKNOWN_OBJECT);
      pResponse->setAttribute("result-text", "Unknown 'address' instance");
      return pResponse;
   }

   // If Administrative State == ACTIVE, delete the ACDQueue
   // For now do not delete the queue on fly
   #if 0
   if (mpAcdServer->getAdministrativeState() == ACDServer::ACTIVE) {
      deleteACDQueue(queueUriString);
   }
   #endif

   // Remove the instance from the configuration file
   deletePSInstance(ACD_QUEUE_TAG, QUEUE_URI_TAG, queueUriString);

   // Update the configuration file
   OsSysLog::add(LOG_FACILITY, PRI_INFO, "ACDQueueManager::Delete - Updating the config file");
   mpXmlConfigDoc->SaveFile();

   mLock.release();

   // Build up the response.
   pResponse = new ProvisioningAttrList;
   pResponse->setAttribute("method-name", "delete");
   pResponse->setAttribute("result-code", ProvisioningAgent::SUCCESS);
   pResponse->setAttribute("result-text", "SUCCESS");
   return pResponse;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueueManager::Set
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

ProvisioningAttrList* ACDQueueManager::Set(ProvisioningAttrList& rRequestAttributes)
{
   ProvisioningAttrList* pResponse;

   UtlString             queueUriString;
   UtlString             name;
   UtlString             overflowDestination;
   UtlString             overflowEntry;
   UtlString             overflowType;
   UtlString             welcomeAudio;
   UtlString             queueAudio;
   UtlString             backgroundAudio;
   UtlString             callTerminationAudio;
   UtlString             acdAgentList;
   UtlString             acdLineList;
   int                   acdScheme;
   int                   maxRingDelay;
   int                   maxQueueDepth;
   int                   maxWaitTime;
   int                   answerMode;
   int                   callConnectScheme;
   int                   queueAudioInterval;
   int                   terminationToneDuration;
   int                   agentsWrapupTime;
   int                   agentsNonResponsiveTime;
   int                   maxBounceCount;
   bool                  fifoOverflow;
   bool                  bargeIn;

   osPrintf("{method} = set\n{object-class} = acd-queue\n");
   rRequestAttributes.dumpAttributes();
   osPrintf("\n");

   // Extract the instance index from the request attributes.
   rRequestAttributes.getAttribute(QUEUE_URI_TAG, queueUriString);

   mLock.acquire();

   // Verify that the instance has already been created.
   TiXmlNode* pInstanceNode = findPSInstance(ACD_QUEUE_TAG, QUEUE_URI_TAG, queueUriString);
   if (pInstanceNode == NULL) {
      // There is no instance.
      mLock.release();
      pResponse = new ProvisioningAttrList;
      pResponse->setAttribute("method-name", "set");
      pResponse->setAttribute("result-code", ProvisioningAgent::UNKNOWN_OBJECT);
      pResponse->setAttribute("result-text", "Unknown 'address' instance");
      return pResponse;
   }

   // Validate that the attribute types are correct, ignoring any that are missing
   try {
      rRequestAttributes.validateAttributeType(QUEUE_NAME_TAG,                      ProvisioningAttrList::STRING);
      rRequestAttributes.validateAttributeType(QUEUE_ACD_SCHEME_TAG,                ProvisioningAttrList::INT);
      rRequestAttributes.validateAttributeType(QUEUE_MAX_RING_DELAY_TAG,            ProvisioningAttrList::INT);
      rRequestAttributes.validateAttributeType(QUEUE_MAX_QUEUE_DEPTH_TAG,           ProvisioningAttrList::INT);
      rRequestAttributes.validateAttributeType(QUEUE_MAX_WAIT_TIME_TAG,             ProvisioningAttrList::INT);
      rRequestAttributes.validateAttributeType(QUEUE_FIFO_OVERFLOW_TAG,             ProvisioningAttrList::BOOL);
      rRequestAttributes.validateAttributeType(QUEUE_OVERFLOW_DESTINATION_TAG,      ProvisioningAttrList::STRING);
      rRequestAttributes.validateAttributeType(QUEUE_OVERFLOW_ENTRY_TAG,            ProvisioningAttrList::STRING);
      rRequestAttributes.validateAttributeType(QUEUE_OVERFLOW_TYPE_TAG,            ProvisioningAttrList::STRING);
      rRequestAttributes.validateAttributeType(QUEUE_ANSWER_MODE_TAG,               ProvisioningAttrList::INT);
      rRequestAttributes.validateAttributeType(QUEUE_CALL_CONNECT_SCHEME_TAG,       ProvisioningAttrList::INT);
      rRequestAttributes.validateAttributeType(QUEUE_WELCOME_AUDIO_TAG,             ProvisioningAttrList::STRING);
      rRequestAttributes.validateAttributeType(QUEUE_BARGE_IN_TAG,                  ProvisioningAttrList::BOOL);
      rRequestAttributes.validateAttributeType(QUEUE_QUEUE_AUDIO_TAG,               ProvisioningAttrList::STRING);
      rRequestAttributes.validateAttributeType(QUEUE_BACKGROUND_AUDIO_TAG,          ProvisioningAttrList::STRING);
      rRequestAttributes.validateAttributeType(QUEUE_QUEUE_AUDIO_INTERVAL_TAG,      ProvisioningAttrList::INT);
      rRequestAttributes.validateAttributeType(QUEUE_CALL_TERMINATION_AUDIO_TAG,    ProvisioningAttrList::STRING);
      rRequestAttributes.validateAttributeType(QUEUE_TERMINATION_TONE_DURATION_TAG, ProvisioningAttrList::INT);
      rRequestAttributes.validateAttributeType(QUEUE_AGENTS_WRAP_UP_TIME_TAG,       ProvisioningAttrList::INT);
      rRequestAttributes.validateAttributeType(QUEUE_AGENTS_NON_RESPONSIVE_TIME_TAG,ProvisioningAttrList::INT);
      rRequestAttributes.validateAttributeType(QUEUE_MAX_BOUNCE_COUNT_TAG,          ProvisioningAttrList::INT);
      rRequestAttributes.validateAttributeType(QUEUE_ACD_AGENT_LIST_TAG,            ProvisioningAttrList::STRING);
      rRequestAttributes.validateAttributeType(QUEUE_ACD_LINE_LIST_TAG,             ProvisioningAttrList::STRING);
   }
   catch (UtlString error) {
      // One of the attributes is not set to the correct type
      mLock.release();
      pResponse = new ProvisioningAttrList;
      pResponse->setAttribute("method-name", "set");
      pResponse->setAttribute("result-code", ProvisioningAgent::INVALID_ATTR);
      pResponse->setAttribute("result-text", error);
      return pResponse;
   }

   // If Administrative State == ACTIVE, update the ACDQueue
   if (mpAcdServer->getAdministrativeState() == ACDServer::ACTIVE) {
      // Lookup the instance
      ACDQueue* pQueueRef = getAcdQueueReference(queueUriString);
      if (pQueueRef == NULL) {
         // There is no instance.
         mLock.release();
         pResponse = new ProvisioningAttrList;
         pResponse->setAttribute("method-name", "set");
         pResponse->setAttribute("result-code", ProvisioningAgent::UNKNOWN_OBJECT);
         pResponse->setAttribute("result-text", "Unknown instance");
         return pResponse;
      }

      // Give the list of changed attributes to the object
      try {
         pQueueRef->setAttributes(rRequestAttributes);
      }
      catch (UtlString error) {
         // The object rejected the request, return error.
         mLock.release();
         pResponse = new ProvisioningAttrList;
         pResponse->setAttribute("method-name", "set");
         pResponse->setAttribute("result-code", ProvisioningAgent::INVALID_ATTR);
         pResponse->setAttribute("result-text", error);
         return pResponse;
      }
   }

   // Now save the individual attribute changes

   // name (optional)
   if (rRequestAttributes.getAttribute(QUEUE_NAME_TAG, name)) {
      setPSAttribute(pInstanceNode, QUEUE_NAME_TAG, name);
   }
   else {
       deletePSAttribute(pInstanceNode, QUEUE_NAME_TAG);
   }

   // acd-scheme (required)
   if (rRequestAttributes.getAttribute(QUEUE_ACD_SCHEME_TAG, acdScheme)) {
      setPSAttribute(pInstanceNode, QUEUE_ACD_SCHEME_TAG, acdScheme);
   }

   // max-ring-delay (required)
   if (rRequestAttributes.getAttribute(QUEUE_MAX_RING_DELAY_TAG, maxRingDelay)) {
      setPSAttribute(pInstanceNode, QUEUE_MAX_RING_DELAY_TAG, maxRingDelay);
   }

   // max-queue-depth (required)
   if (rRequestAttributes.getAttribute(QUEUE_MAX_QUEUE_DEPTH_TAG, maxQueueDepth)) {
      setPSAttribute(pInstanceNode, QUEUE_MAX_QUEUE_DEPTH_TAG, maxQueueDepth);
   }

   // max-wait-time (required)
   if (rRequestAttributes.getAttribute(QUEUE_MAX_WAIT_TIME_TAG, maxWaitTime)) {
      setPSAttribute(pInstanceNode, QUEUE_MAX_WAIT_TIME_TAG, maxWaitTime);
   }

   // fifo-overflow (required)
   if (rRequestAttributes.getAttribute(QUEUE_FIFO_OVERFLOW_TAG, fifoOverflow)) {
      setPSAttribute(pInstanceNode, QUEUE_FIFO_OVERFLOW_TAG, fifoOverflow);
   }

   // overflow-destination (optional)
   if (rRequestAttributes.getAttribute(QUEUE_OVERFLOW_DESTINATION_TAG, overflowDestination)) {
      setPSAttribute(pInstanceNode, QUEUE_OVERFLOW_DESTINATION_TAG, overflowDestination);
   }
   else {
       deletePSAttribute(pInstanceNode, QUEUE_OVERFLOW_DESTINATION_TAG);
   }

   // overflow-entry (optional)
   if (rRequestAttributes.getAttribute(QUEUE_OVERFLOW_ENTRY_TAG, overflowEntry)) {
      setPSAttribute(pInstanceNode, QUEUE_OVERFLOW_ENTRY_TAG, overflowEntry);
   }
   else {
       deletePSAttribute(pInstanceNode, QUEUE_OVERFLOW_ENTRY_TAG);
   }

   // overflow-type (optional)
   if (rRequestAttributes.getAttribute(QUEUE_OVERFLOW_TYPE_TAG, overflowType)) {
      setPSAttribute(pInstanceNode, QUEUE_OVERFLOW_TYPE_TAG, overflowType);
   }
   else {
       deletePSAttribute(pInstanceNode, QUEUE_OVERFLOW_TYPE_TAG);
   }

   // answer-mode (required)
   if (rRequestAttributes.getAttribute(QUEUE_ANSWER_MODE_TAG, answerMode)) {
      setPSAttribute(pInstanceNode, QUEUE_ANSWER_MODE_TAG, answerMode);
   }

   // call-connect-scheme (required)
   if (rRequestAttributes.getAttribute(QUEUE_CALL_CONNECT_SCHEME_TAG, callConnectScheme)) {
      setPSAttribute(pInstanceNode, QUEUE_CALL_CONNECT_SCHEME_TAG, callConnectScheme);
   }

   // welcome-audio (optional)
   if (rRequestAttributes.getAttribute(QUEUE_WELCOME_AUDIO_TAG, welcomeAudio)) {
      setPSAttribute(pInstanceNode, QUEUE_WELCOME_AUDIO_TAG, welcomeAudio);
   }
   else {
       deletePSAttribute(pInstanceNode, QUEUE_WELCOME_AUDIO_TAG);
   }

   // barge-in (optional)
   if (rRequestAttributes.getAttribute(QUEUE_BARGE_IN_TAG, bargeIn)) {
      setPSAttribute(pInstanceNode, QUEUE_BARGE_IN_TAG, bargeIn);
   }
   else {
       deletePSAttribute(pInstanceNode, QUEUE_BARGE_IN_TAG);
   }

   // queue-audio (optional)
   if (rRequestAttributes.getAttribute(QUEUE_QUEUE_AUDIO_TAG, queueAudio)) {
      setPSAttribute(pInstanceNode, QUEUE_QUEUE_AUDIO_TAG, queueAudio);
   }
   else {
       deletePSAttribute(pInstanceNode, QUEUE_QUEUE_AUDIO_TAG);
   }

   // background-audio (optional)
   if (rRequestAttributes.getAttribute(QUEUE_BACKGROUND_AUDIO_TAG, backgroundAudio)) {
      setPSAttribute(pInstanceNode, QUEUE_BACKGROUND_AUDIO_TAG, backgroundAudio);
   }
   else {
       deletePSAttribute(pInstanceNode, QUEUE_BACKGROUND_AUDIO_TAG);
   }

   // queue-audio-interval (optional)
   if (rRequestAttributes.getAttribute(QUEUE_QUEUE_AUDIO_INTERVAL_TAG, queueAudioInterval)) {
      setPSAttribute(pInstanceNode, QUEUE_QUEUE_AUDIO_INTERVAL_TAG, queueAudioInterval);
   }
   else {
       deletePSAttribute(pInstanceNode, QUEUE_QUEUE_AUDIO_INTERVAL_TAG);
   }

   // call-termination-audio (optional)
   if (rRequestAttributes.getAttribute(QUEUE_CALL_TERMINATION_AUDIO_TAG, callTerminationAudio)) {
      setPSAttribute(pInstanceNode, QUEUE_CALL_TERMINATION_AUDIO_TAG, callTerminationAudio);
   }
   else {
       deletePSAttribute(pInstanceNode, QUEUE_CALL_TERMINATION_AUDIO_TAG);
   }

   // termination-tone-duration (optional)
   if (rRequestAttributes.getAttribute(QUEUE_TERMINATION_TONE_DURATION_TAG, terminationToneDuration)) {
      setPSAttribute(pInstanceNode, QUEUE_TERMINATION_TONE_DURATION_TAG, terminationToneDuration);
   }
   else {
       deletePSAttribute(pInstanceNode, QUEUE_TERMINATION_TONE_DURATION_TAG);
   }

   // agents-wrap-up-time (optional)
   if (rRequestAttributes.getAttribute(QUEUE_AGENTS_WRAP_UP_TIME_TAG, agentsWrapupTime)) {
      setPSAttribute(pInstanceNode, QUEUE_AGENTS_WRAP_UP_TIME_TAG, agentsWrapupTime);
   }
   else {
       deletePSAttribute(pInstanceNode, QUEUE_AGENTS_WRAP_UP_TIME_TAG);
   }

   // agents-non-responsive-time (optional)
   if (rRequestAttributes.getAttribute(QUEUE_AGENTS_NON_RESPONSIVE_TIME_TAG, agentsNonResponsiveTime)) {
      setPSAttribute(pInstanceNode, QUEUE_AGENTS_NON_RESPONSIVE_TIME_TAG, agentsNonResponsiveTime);
   }
   else {
       deletePSAttribute(pInstanceNode, QUEUE_AGENTS_NON_RESPONSIVE_TIME_TAG);
   }

   // max-bounces (optional)
   if (rRequestAttributes.getAttribute(QUEUE_MAX_BOUNCE_COUNT_TAG, maxBounceCount)) {
      setPSAttribute(pInstanceNode, QUEUE_MAX_BOUNCE_COUNT_TAG, maxBounceCount);
   }
   else {
       deletePSAttribute(pInstanceNode, QUEUE_MAX_BOUNCE_COUNT_TAG);
   }

   // acd-agent-list (optional)
   if (rRequestAttributes.getAttribute(QUEUE_ACD_AGENT_LIST_TAG, acdAgentList)) {
      setPSAttribute(pInstanceNode, QUEUE_ACD_AGENT_LIST_TAG, acdAgentList);
   }
   else {
       deletePSAttribute(pInstanceNode, QUEUE_ACD_AGENT_LIST_TAG);
   }

   // external-line-list (optional)
   if (rRequestAttributes.getAttribute(QUEUE_ACD_LINE_LIST_TAG, acdLineList)) {
      setPSAttribute(pInstanceNode, QUEUE_ACD_LINE_LIST_TAG, acdLineList);
   }
   else {
       deletePSAttribute(pInstanceNode, QUEUE_ACD_LINE_LIST_TAG);
   }

   // Update the configuration file
   OsSysLog::add(LOG_FACILITY, PRI_INFO, "ACDQueueManager::Set - Updating the config file");
   mpXmlConfigDoc->SaveFile();

   mLock.release();

   // Build up the response.
   pResponse = new ProvisioningAttrList;
   pResponse->setAttribute("method-name", "set");
   pResponse->setAttribute("result-code", ProvisioningAgent::SUCCESS);
   pResponse->setAttribute("result-text", "SUCCESS");
   return pResponse;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueueManager::Get
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

ProvisioningAttrList* ACDQueueManager::Get(ProvisioningAttrList& rRequestAttributes)
{
   ProvisioningAttrList* pResponse;
   ProvisioningAttrList* pQueueInstance;
   UtlString             queueUriString;
   UtlSList*             pQueueList;


   osPrintf("{method} = get\n{object-class} = acd-queue\n");
   rRequestAttributes.dumpAttributes();
   osPrintf("\n");

   mLock.acquire();

   // Extract the instance index from the request attributes.
   if (rRequestAttributes.getAttribute(QUEUE_URI_TAG, queueUriString)) {
      // A specific instance has been specified, verify that it exists
      ACDQueue* pQueueRef = getAcdQueueReference(queueUriString);
      if (pQueueRef == NULL) {
         // There is no instance.
         mLock.release();
         pResponse = new ProvisioningAttrList;
         pResponse->setAttribute("method-name", "get");
         pResponse->setAttribute("result-code", ProvisioningAgent::UNKNOWN_OBJECT);
         pResponse->setAttribute("result-text", "Unknown instance");
         return pResponse;
      }

      // Call the instance to get the requested attributes
      pResponse = new ProvisioningAttrList;
      try {
         pQueueRef->getAttributes(rRequestAttributes, pResponse);
      }
      catch (UtlString error) {
         // The request for attributes failed, send back the response
         mLock.release();
         pResponse->setAttribute("method-name", "get");
         pResponse->setAttribute("result-code", ProvisioningAgent::FAILURE);
         pResponse->setAttribute("result-text", error);
         return pResponse;
      }

      // Send back the response
      mLock.release();
      pResponse->setAttribute("method-name", "get");
      pResponse->setAttribute("result-code", ProvisioningAgent::SUCCESS);
      pResponse->setAttribute("result-text", "SUCCESS");
      pResponse->setAttribute("object-class", ACD_QUEUE_TAG);
      pResponse->setAttribute(QUEUE_URI_TAG, queueUriString);
      return pResponse;
   }
   else {
      // No specific instance was requested, send back a list of the available instances
      // Find the first instance.
      TiXmlNode* pInstanceNode = findPSInstance(ACD_QUEUE_TAG);
      pQueueList = new UtlSList;

      // Build up a list of instances
      while (pInstanceNode != NULL) {
         // Read the index parameter
         getPSAttribute(pInstanceNode, QUEUE_URI_TAG, queueUriString);

         // Create the list entry
         pQueueInstance = new ProvisioningAttrList;
         pQueueInstance->setAttribute("object-class", ACD_QUEUE_TAG);
         pQueueInstance->setAttribute(QUEUE_URI_TAG, queueUriString);
         pQueueList->append(pQueueInstance->getData());

         // Get the next instance.
         pInstanceNode = pInstanceNode->NextSibling();
      }

      // Send back the response
      mLock.release();
      pResponse = new ProvisioningAttrList;
      pResponse->setAttribute("method-name", "get");
      pResponse->setAttribute("result-code", ProvisioningAgent::SUCCESS);
      pResponse->setAttribute("result-text", "SUCCESS");
      pResponse->setAttribute("object-class-list", pQueueList);
      return pResponse;
   }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueueManager::loadConfiguration
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

bool ACDQueueManager::loadConfiguration(void)
{
   UtlString             queueUriString;
   UtlString             name;
   UtlString             overflowDestination;
   UtlString             overflowEntry;
   UtlString             overflowType;
   UtlString             welcomeAudio;
   UtlString             queueAudio;
   UtlString             backgroundAudio;
   UtlString             callTerminationAudio;
   UtlString             acdAgentList;
   UtlString             acdLineList;
   int                   acdScheme;
   int                   maxRingDelay;
   int                   maxQueueDepth;
   int                   maxWaitTime;
   int                   answerMode;
   int                   callConnectScheme;
   int                   queueAudioInterval;
   int                   terminationToneDuration;
   int                   agentsWrapupTime;
   int                   agentsNonResponsiveTime;
   int                   maxBounceCount;
   bool                  fifoOverflow;
   bool                  bargeIn;

   mLock.acquire();

   // Find the first instance.
   TiXmlNode* pInstanceNode = findPSInstance(ACD_QUEUE_TAG);
   if (pInstanceNode == NULL) {
      // There are no instances.
      mLock.release();
      return false;
   }

   while (pInstanceNode != NULL) {
      // Set default values
      name                    = "";
      overflowDestination     = "";
      overflowEntry           = "";
      overflowType            = "";
      welcomeAudio            = "";
      bargeIn                 = FALSE;
      queueAudio              = "";
      backgroundAudio         = "";
      queueAudioInterval      = 0;
      callTerminationAudio    = "";
      terminationToneDuration = 0;
      acdAgentList            = "";
      acdLineList             = "";

      // Read in the parameters
      getPSAttribute(pInstanceNode, QUEUE_URI_TAG,                       queueUriString);
      getPSAttribute(pInstanceNode, QUEUE_NAME_TAG,                      name);
      getPSAttribute(pInstanceNode, QUEUE_ACD_SCHEME_TAG,                acdScheme);
      getPSAttribute(pInstanceNode, QUEUE_MAX_RING_DELAY_TAG,            maxRingDelay);
      getPSAttribute(pInstanceNode, QUEUE_MAX_QUEUE_DEPTH_TAG,           maxQueueDepth);
      getPSAttribute(pInstanceNode, QUEUE_MAX_WAIT_TIME_TAG,             maxWaitTime);
      getPSAttribute(pInstanceNode, QUEUE_FIFO_OVERFLOW_TAG,             fifoOverflow);
      getPSAttribute(pInstanceNode, QUEUE_OVERFLOW_DESTINATION_TAG,      overflowDestination);
      getPSAttribute(pInstanceNode, QUEUE_OVERFLOW_ENTRY_TAG,            overflowEntry);
      getPSAttribute(pInstanceNode, QUEUE_OVERFLOW_TYPE_TAG,             overflowType);
      getPSAttribute(pInstanceNode, QUEUE_ANSWER_MODE_TAG,               answerMode);
      getPSAttribute(pInstanceNode, QUEUE_CALL_CONNECT_SCHEME_TAG,       callConnectScheme);
      getPSAttribute(pInstanceNode, QUEUE_WELCOME_AUDIO_TAG,             welcomeAudio);
      getPSAttribute(pInstanceNode, QUEUE_BARGE_IN_TAG,                  bargeIn);
      getPSAttribute(pInstanceNode, QUEUE_QUEUE_AUDIO_TAG,               queueAudio);
      getPSAttribute(pInstanceNode, QUEUE_BACKGROUND_AUDIO_TAG,          backgroundAudio);
      getPSAttribute(pInstanceNode, QUEUE_QUEUE_AUDIO_INTERVAL_TAG,      queueAudioInterval);
      getPSAttribute(pInstanceNode, QUEUE_CALL_TERMINATION_AUDIO_TAG,    callTerminationAudio);
      getPSAttribute(pInstanceNode, QUEUE_TERMINATION_TONE_DURATION_TAG, terminationToneDuration);
      getPSAttribute(pInstanceNode, QUEUE_AGENTS_WRAP_UP_TIME_TAG,       agentsWrapupTime);
      getPSAttribute(pInstanceNode, QUEUE_AGENTS_NON_RESPONSIVE_TIME_TAG,agentsNonResponsiveTime);
      getPSAttribute(pInstanceNode, QUEUE_MAX_BOUNCE_COUNT_TAG          ,maxBounceCount);
      getPSAttribute(pInstanceNode, QUEUE_ACD_AGENT_LIST_TAG,            acdAgentList);
      getPSAttribute(pInstanceNode, QUEUE_ACD_LINE_LIST_TAG,             acdLineList);

      // Create the ACDQueue
      createACDQueue(queueUriString, name, acdScheme,
                     maxRingDelay, maxQueueDepth, maxWaitTime, fifoOverflow,
                     overflowDestination, overflowEntry, overflowType, answerMode, callConnectScheme,
                     welcomeAudio, bargeIn, queueAudio,
                     backgroundAudio, queueAudioInterval,
                     callTerminationAudio, terminationToneDuration,
                     agentsWrapupTime, agentsNonResponsiveTime,
                     maxBounceCount, acdAgentList, acdLineList);

      // Get the next instance.
      pInstanceNode = pInstanceNode->NextSibling();
   }

   // Indicate that the configuration has been succesfully loaded.
   mConfigurationLoaded = true;

   mLock.release();

   return true;
}

/* ============================ ACCESSORS ================================= */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueueManager::getAcdQueueReference
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

ACDQueue* ACDQueueManager::getAcdQueueReference(UtlString& rQueueUriString)
{
   ACDQueue*  pQueueRef;

   mLock.acquire();

   pQueueRef = dynamic_cast<ACDQueue*>(mAcdQueueList.findValue(&rQueueUriString));

   mLock.release();

   return pQueueRef;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueueManager::getAcdCallManagerReference
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

ACDCallManager* ACDQueueManager::getAcdCallManagerReference(void)
{
   return mpAcdCallManager;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueueManager::getAcdCallManagerHandle
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

SIPX_INST ACDQueueManager::getAcdCallManagerHandle(void)
{
   return mhAcdCallManagerHandle;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueueManager::getAcdAgentManagerReference
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

ACDAgentManager* ACDQueueManager::getAcdAgentManagerReference(void)
{
   return mpAcdAgentManager;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueueManager::getAcdLineManagerReference
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

ACDLineManager* ACDQueueManager::getAcdLineManagerReference(void)
{
   return mpAcdLineManager;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDQueueManager::getAcdServer
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

ACDServer* ACDQueueManager::getAcdServer(void)
{
	return mpAcdServer;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
