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
#include <cp/LinePresenceMonitor.h>
#include <net/ProvisioningAgent.h>
#include <utl/UtlHashMapIterator.h>
#include "ACDServer.h"
#include "ACDAgent.h"
#include "ACDCallManager.h"
#include "ACDAgentManager.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
extern OsSysLogPriority gACD_DEBUG;

// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgentManager::ACDAgentManager
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

ACDAgentManager::ACDAgentManager(ACDServer* pAcdServer, int presenceMonitorPort, const char* pPresenceServerUriString, const char* pPresenceServiceUriString)
: ProvisioningClass(ACD_AGENT_TAG), mLock(OsMutex::Q_FIFO)
{
   mpAcdServer = pAcdServer;
   mpAcdCallManager = NULL;
   mhAcdCallManagerHandle = NULL;
   mpAcdQueueManager = NULL;
   mPresenceServiceUrl = pPresenceServiceUriString;

   int bindPort = presenceMonitorPort;
   UtlString domainName;
   Url remoteServerUri;
   Url presenceServerUri = pPresenceServerUriString;
   UtlString groupName("ACD");
   mAgentIndex = 0;

   domainName = mpAcdServer->getDomain();

   // Create the Sip Dialog Monitor
   mpLinePresenceMonitor = new LinePresenceMonitor(bindPort,
                                                   domainName,
                                                   groupName,
                                                   true,
                                                   remoteServerUri,
                                                   presenceServerUri);

   // Start it up
   mpLinePresenceMonitor->start();

   // Test to see if the optional Presence Server has been provisioned and record.
   if (presenceServerUri.toString() == NULL ||
       presenceServerUri.toString().compareTo("sip:") == 0) {
      mPresenceServerEnabled = false;
   }
   else {
      mPresenceServerEnabled = true;
      OsSysLog::add(FAC_ACD, PRI_INFO, "ACDAgentManager::ACDAgentManager - Presence Server (%s) is enabled",
                    presenceServerUri.toString().data());
   }
}

/**
 * Destructor
 */
////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgentManager::~ACDAgentManager
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

ACDAgentManager::~ACDAgentManager()
{
   // Clean all the agents in the list
   mLock.acquire();

   mAcdAgentList.destroyAll();
   delete mpLinePresenceMonitor;

   mLock.release();
}


/* ============================ MANIPULATORS ============================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgentManager::initialize
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

OsStatus ACDAgentManager::initialize(void)
{
   mpAcdCallManager       = mpAcdServer->getAcdCallManager();
   mhAcdCallManagerHandle = mpAcdCallManager->getAcdCallManagerHandle();
   mpAcdQueueManager      = mpAcdServer->getAcdQueueManager();

   // Register this with the Provisioning Agent
   mpAcdServer->getProvisioningAgent()->registerClass(this);

   return OS_SUCCESS;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgentManager::start
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

OsStatus ACDAgentManager::start(void)
{
   // Load the configuration
   loadConfiguration();

   return OS_SUCCESS;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgentManager::createACDAgent
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

ACDAgent* ACDAgentManager::createACDAgent(const char* pAgentUriString,
                                          const char* pName,
                                          const char* pExtension,
                                          bool        monitorPresence,
                                          bool        alwaysAvailable,
                                          const char* pAcdQueueList,
                                          bool        pseudoAgent)
{
   ACDAgent*  pAgentRef;

   mLock.acquire();

   // Create an ACDAgent object.
   ++mAgentIndex;
   pAgentRef = new ACDAgent(this, pAgentUriString, pName, pExtension, mAgentIndex,
                            monitorPresence, alwaysAvailable, pAcdQueueList, pseudoAgent);

   // Start it up
   pAgentRef->start();

   // Create a mapping between the ACDAgent URI and the ACDAgent instance.
   mAcdAgentList.insertKeyAndValue(new UtlString(pAgentUriString), pAgentRef);

   OsSysLog::add(FAC_ACD, PRI_INFO, "ACDAgentManager::createACDAgent - Agent added: %s",
                 pAgentUriString);

   mLock.release();

   return pAgentRef;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgentManager::deleteACDAgent
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

void ACDAgentManager::deleteACDAgent(const char* pAgentUriString)
{
   UtlContainable* pAgentRef;
   UtlContainable* pKey;

   mLock.acquire();

   // Remove the mapping between the ACDAgent URI and the ACDAgent instance.
   const UtlString searchUriKey(pAgentUriString);
   pKey = mAcdAgentList.removeKeyAndValue(&searchUriKey, pAgentRef);
   if (pKey == NULL) {
      // Error. Did not find a matching ACDAgent object.
      OsSysLog::add(FAC_ACD, PRI_ERR, "ACDAgentManager::deleteACDAgent - Failed to find reference to Agent: %s",
                    pAgentUriString);
      mLock.release();
      return;
   }
   delete pKey;

   // Signal the ACDAgent's associated task to shutdown
   dynamic_cast<ACDAgent*>(pAgentRef)->requestShutdown();
   while (!dynamic_cast<ACDAgent*>(pAgentRef)->isShutDown()) {
      // Wait for it to complete
   }

   // Finally delete the ACDAgent object.
   delete pAgentRef;

   OsSysLog::add(FAC_ACD, PRI_INFO, "ACDAgentManager::deleteACDAgent - Agent: %s deleted",
                 pAgentUriString);

   mLock.release();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgentManager::linePresenceSubscribe
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

void ACDAgentManager::linePresenceSubscribe(ACDAgent* pAgent)
{
   mpLinePresenceMonitor->subscribeDialog(pAgent);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgentManager::linePresenceUnsubscribe
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

void ACDAgentManager::linePresenceUnsubscribe(ACDAgent* pAgent, OsEvent *e)
{
   mpLinePresenceMonitor->unsubscribeDialog(pAgent, e);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgentManager::presenceServerSubscribe
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

void ACDAgentManager::presenceServerSubscribe(ACDAgent* pAgent)
{
   if (mPresenceServerEnabled) {
      mpLinePresenceMonitor->subscribePresence(pAgent);
   }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgentManager::presenceServerUnsubscribe
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

void ACDAgentManager::presenceServerUnsubscribe(ACDAgent* pAgent, OsEvent *e)
{
   if (mPresenceServerEnabled) {
      mpLinePresenceMonitor->unsubscribePresence(pAgent, e);
   }
   else {
      if (e) {
         e->signal(0) ;  // signal event so no one waits forever
      }
   }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgentManager::Create
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

ProvisioningAttrList* ACDAgentManager::Create(ProvisioningAttrList& rRequestAttributes)
{
   ProvisioningAttrList* pResponse;

   UtlString             agentUriString;
   UtlString             name;
   UtlString             acdQueueList;
   UtlString             extension;
   bool                  monitorPresence;
   bool                  alwaysAvailable;

   osPrintf("{method} = create\n{object-class} = acd-agent\n");
   rRequestAttributes.dumpAttributes();
   osPrintf("\n");

   // Verify that the required set of attributes are there.
   try {
      rRequestAttributes.validateAttribute(AGENT_URI_TAG,              ProvisioningAttrList::STRING);
      rRequestAttributes.validateAttribute(AGENT_MONITOR_PRESENCE_TAG, ProvisioningAttrList::BOOL);
      rRequestAttributes.validateAttribute(AGENT_ALWAYS_AVAILABLE_TAG, ProvisioningAttrList::BOOL);
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
   rRequestAttributes.getAttribute(AGENT_URI_TAG, agentUriString);

   mLock.acquire();

   // Verify that an instance of this object has not already been created.
   TiXmlNode* pInstanceNode = findPSInstance(ACD_AGENT_TAG, AGENT_URI_TAG, agentUriString);
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
      rRequestAttributes.validateAttributeType(AGENT_NAME_TAG,           ProvisioningAttrList::STRING);
      rRequestAttributes.validateAttributeType(AGENT_EXTENSION_TAG,      ProvisioningAttrList::STRING);
      rRequestAttributes.validateAttributeType(AGENT_ACD_QUEUE_LIST_TAG, ProvisioningAttrList::STRING);
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
   pInstanceNode = createPSInstance(ACD_AGENT_TAG, AGENT_URI_TAG, agentUriString);
   if (pInstanceNode == NULL) {
      mLock.release();
      // Instance creation failed.
      pResponse = new ProvisioningAttrList;
      pResponse->setAttribute("method-name", "create");
      pResponse->setAttribute("result-code", ProvisioningAgent::UNDEFINED);
      pResponse->setAttribute("result-text", "Managed Object Instance creation failed");
      return pResponse;
   }

   // Now save the individual attributes

   // name (optional)
   if (rRequestAttributes.getAttribute(AGENT_NAME_TAG, name)) {
      setPSAttribute(pInstanceNode, AGENT_NAME_TAG, name);
   }

   // extension (optional)
   if (rRequestAttributes.getAttribute(AGENT_EXTENSION_TAG, extension)) {
      setPSAttribute(pInstanceNode, AGENT_EXTENSION_TAG, extension);
   }

   // monitor-presence
   rRequestAttributes.getAttribute(AGENT_MONITOR_PRESENCE_TAG, monitorPresence);
   setPSAttribute(pInstanceNode, AGENT_MONITOR_PRESENCE_TAG, monitorPresence);

   // always-available
   rRequestAttributes.getAttribute(AGENT_ALWAYS_AVAILABLE_TAG, alwaysAvailable);
   setPSAttribute(pInstanceNode, AGENT_ALWAYS_AVAILABLE_TAG, alwaysAvailable);

   // acd-queue-list (optional)
   if (rRequestAttributes.getAttribute(AGENT_ACD_QUEUE_LIST_TAG, acdQueueList)) {
      setPSAttribute(pInstanceNode, AGENT_ACD_QUEUE_LIST_TAG, acdQueueList);
   }

   // If Administrative State == ACTIVE, create the ACDAgent
   if (mpAcdServer->getAdministrativeState() == ACDServer::ACTIVE) {
      createACDAgent(agentUriString, name, extension, monitorPresence, alwaysAvailable, acdQueueList);
   }

   // Update the configuration file
   OsSysLog::add(LOG_FACILITY, PRI_INFO, "ACDAgentManager::Create - Updating the config file");
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
//  NAME:        ACDAgentManager::Delete
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

ProvisioningAttrList* ACDAgentManager::Delete(ProvisioningAttrList& rRequestAttributes)
{
   ProvisioningAttrList* pResponse;
   UtlString             agentUriString;

   osPrintf("{method} = delete\n{object-class} = acd-agent\n");
   rRequestAttributes.dumpAttributes();
   osPrintf("\n");

   // Extract the instance index from the request attributes.
   rRequestAttributes.getAttribute(AGENT_URI_TAG, agentUriString);

   mLock.acquire();

   // Verify that the instance has already been created.
   TiXmlNode* pInstanceNode = findPSInstance(ACD_AGENT_TAG, AGENT_URI_TAG, agentUriString);
   if (pInstanceNode == NULL) {
      // There is no instance.
      mLock.release();
      pResponse = new ProvisioningAttrList;
      pResponse->setAttribute("method-name", "delete");
      pResponse->setAttribute("result-code", ProvisioningAgent::UNKNOWN_OBJECT);
      pResponse->setAttribute("result-text", "Unknown instance");
      return pResponse;
   }

   // If Administrative State == ACTIVE, delete the ACDAgent
   if (mpAcdServer->getAdministrativeState() == ACDServer::ACTIVE) {
      // Delete the agent only if he is not handling any call
      ACDAgent* pAgentRef = getAcdAgentReference(agentUriString);
      if ((pAgentRef) && (false == pAgentRef->isFree())) {
         pAgentRef->setDelete(TRUE);
      }
      else {
         deleteACDAgent(agentUriString);
      }
   }

   // Remove the instance from the configuration file
   deletePSInstance(ACD_AGENT_TAG, AGENT_URI_TAG, agentUriString);

   // Update the configuration file
   OsSysLog::add(LOG_FACILITY, PRI_INFO, "ACDAgentManager::Delete - Updating the config file");
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
//  NAME:        ACDAgentManager::Set
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

ProvisioningAttrList* ACDAgentManager::Set(ProvisioningAttrList& rRequestAttributes)
{
   ProvisioningAttrList* pResponse;

   UtlString             agentUriString;
   UtlString             name;
   UtlString             acdQueueList;
   UtlString             extension;
   bool                  monitorPresence;
   bool                  alwaysAvailable;

   osPrintf("{method} = set\n{object-class} = acd-agent\n");
   rRequestAttributes.dumpAttributes();
   osPrintf("\n");

   // Extract the instance index from the request attributes.
   rRequestAttributes.getAttribute(AGENT_URI_TAG, agentUriString);

   mLock.acquire();

   // Verify that the instance has already been created.
   TiXmlNode* pInstanceNode = findPSInstance(ACD_AGENT_TAG, AGENT_URI_TAG, agentUriString);
   if (pInstanceNode == NULL) {
      // There is no instance.
      mLock.release();
      pResponse = new ProvisioningAttrList;
      pResponse->setAttribute("method-name", "set");
      pResponse->setAttribute("result-code", ProvisioningAgent::UNKNOWN_OBJECT);
      pResponse->setAttribute("result-text", "Unknown instance");
      return pResponse;
   }

   // Validate that the attribute types are correct, ignoring any that are missing
   try {
      rRequestAttributes.validateAttributeType(AGENT_NAME_TAG,             ProvisioningAttrList::STRING);
      rRequestAttributes.validateAttributeType(AGENT_EXTENSION_TAG,        ProvisioningAttrList::STRING);
      rRequestAttributes.validateAttributeType(AGENT_MONITOR_PRESENCE_TAG, ProvisioningAttrList::BOOL);
      rRequestAttributes.validateAttributeType(AGENT_ALWAYS_AVAILABLE_TAG, ProvisioningAttrList::BOOL);
      rRequestAttributes.validateAttributeType(AGENT_ACD_QUEUE_LIST_TAG,   ProvisioningAttrList::STRING);
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

   // If Administrative State == ACTIVE, update the ACDAgent
   if (mpAcdServer->getAdministrativeState() == ACDServer::ACTIVE) {
      // Lookup the instance
      ACDAgent* pAgentRef = getAcdAgentReference(agentUriString);
      if (pAgentRef == NULL) {
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
         pAgentRef->setAttributes(rRequestAttributes);
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

   // Now save the individual attributes
   // name
   if (rRequestAttributes.getAttribute(AGENT_NAME_TAG, name)) {
      setPSAttribute(pInstanceNode, AGENT_NAME_TAG, name);
   }

   // extension
   if (rRequestAttributes.getAttribute(AGENT_EXTENSION_TAG, extension)) {
      setPSAttribute(pInstanceNode, AGENT_EXTENSION_TAG, extension);
   }

   // monitor-presence
   if (rRequestAttributes.getAttribute(AGENT_MONITOR_PRESENCE_TAG, monitorPresence)) {
      setPSAttribute(pInstanceNode, AGENT_MONITOR_PRESENCE_TAG, monitorPresence);
   }

   // always-available
   if (rRequestAttributes.getAttribute(AGENT_ALWAYS_AVAILABLE_TAG, alwaysAvailable)) {
      setPSAttribute(pInstanceNode, AGENT_ALWAYS_AVAILABLE_TAG, alwaysAvailable);
   }

   // acd-queue-list
   if (rRequestAttributes.getAttribute(AGENT_ACD_QUEUE_LIST_TAG, acdQueueList)) {
      setPSAttribute(pInstanceNode, AGENT_ACD_QUEUE_LIST_TAG, acdQueueList);
   }

   // Update the configuration file
   OsSysLog::add(LOG_FACILITY, PRI_INFO, "ACDAgentManager::Set - Updating the config file");
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
//  NAME:        ACDAgentManager::Get
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

ProvisioningAttrList* ACDAgentManager::Get(ProvisioningAttrList& rRequestAttributes)
{
   ProvisioningAttrList* pResponse;
   ProvisioningAttrList* pAgentInstance;
   UtlString             agentUriString;
   UtlSList*             pAgentList;


   osPrintf("{method} = get\n{object-class} = acd-agent\n");
   rRequestAttributes.dumpAttributes();
   osPrintf("\n");

   mLock.acquire();

   // Extract the instance index from the request attributes.
   if (rRequestAttributes.getAttribute(AGENT_URI_TAG, agentUriString)) {
      // A specific instance has been specified, verify that it exists
      ACDAgent* pAgentRef = getAcdAgentReference(agentUriString);
      if (pAgentRef == NULL) {
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
         pAgentRef->getAttributes(rRequestAttributes, pResponse);
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
      pResponse->setAttribute("object-class", ACD_AGENT_TAG);
      pResponse->setAttribute(AGENT_URI_TAG, agentUriString);
      return pResponse;
   }
   else {
      // No specific instance was requested, send back a list of the available instances
      // Find the first instance.
      TiXmlNode* pInstanceNode = findPSInstance(ACD_AGENT_TAG);
      pAgentList = new UtlSList;

      // Build up a list of instances
      while (pInstanceNode != NULL) {
         // Read the index parameter
         getPSAttribute(pInstanceNode, AGENT_URI_TAG, agentUriString);

         // Create the list entry
         pAgentInstance = new ProvisioningAttrList;
         pAgentInstance->setAttribute("object-class", ACD_AGENT_TAG);
         pAgentInstance->setAttribute(AGENT_URI_TAG, agentUriString);
         pAgentList->append(pAgentInstance->getData());

         // Get the next instance.
         pInstanceNode = pInstanceNode->NextSibling();
      }

      // Send back the response
      mLock.release();
      pResponse = new ProvisioningAttrList;
      pResponse->setAttribute("method-name", "get");
      pResponse->setAttribute("result-code", ProvisioningAgent::SUCCESS);
      pResponse->setAttribute("result-text", "SUCCESS");
      pResponse->setAttribute("object-class-list", pAgentList);
      return pResponse;
   }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgentManager::loadConfiguration
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

bool ACDAgentManager::loadConfiguration(void)
{
   UtlString             agentUriString;
   UtlString             name;
   UtlString             acdQueueList;
   UtlString             extension;
   bool                  monitorPresence;
   bool                  alwaysAvailable;

   mLock.acquire();

   // Find the first instance.
   TiXmlNode* pInstanceNode = findPSInstance(ACD_AGENT_TAG);
   if (pInstanceNode == NULL) {
      // There is no instances.
      mLock.release();
      return false;
   }

   while (pInstanceNode != NULL) {
      // Set default values
      name         = "";
      extension    = "";
      acdQueueList = "";

      // Read in the parameters
      getPSAttribute(pInstanceNode, AGENT_URI_TAG,              agentUriString);
      getPSAttribute(pInstanceNode, AGENT_NAME_TAG,             name);
      getPSAttribute(pInstanceNode, AGENT_EXTENSION_TAG,        extension);
      getPSAttribute(pInstanceNode, AGENT_MONITOR_PRESENCE_TAG, monitorPresence);
      getPSAttribute(pInstanceNode, AGENT_ALWAYS_AVAILABLE_TAG, alwaysAvailable);
      getPSAttribute(pInstanceNode, AGENT_ACD_QUEUE_LIST_TAG,   acdQueueList);

      // Create the ACDAgent
      createACDAgent(agentUriString, name, extension, monitorPresence, alwaysAvailable, acdQueueList);
#if 0
      // remove this after test
      UtlString fakeAgent = "sip:206@cdhcp178.pingtel.com" ;
      createACDAgent(fakeAgent, name, extension, monitorPresence, alwaysAvailable, acdQueueList);
#endif

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
//  NAME:        ACDAgentManager::getAcdAgentReference
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

ACDAgent* ACDAgentManager::getAcdAgentReference(UtlString& rAgentUriString)
{
   ACDAgent*  pAgentRef;

   mLock.acquire();

   pAgentRef = dynamic_cast<ACDAgent*>(mAcdAgentList.findValue(&rAgentUriString));

   mLock.release();

   return pAgentRef;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgentManager::getAcdCallManagerHandle
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

SIPX_INST ACDAgentManager::getAcdCallManagerHandle(void)
{
   return mhAcdCallManagerHandle;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgentManager::getAcdCallManager
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

ACDCallManager* ACDAgentManager::getAcdCallManager(void)
{
   return mpAcdCallManager;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgentManager::getAcdQueueManager
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

ACDQueueManager* ACDAgentManager::getAcdQueueManager(void)
{
   return mpAcdQueueManager;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgentManager::getAcdServer
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

ACDServer* ACDAgentManager::getAcdServer(void)
{
   return mpAcdServer;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAgentManager::getLinePresenceMonitor
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

LinePresenceMonitor* ACDAgentManager::getLinePresenceMonitor(void)
{
   return  mpLinePresenceMonitor;
}
/* ============================ INQUIRY =================================== */

UtlBoolean ACDAgentManager::isOk(void) const
{
   UtlBoolean bOk = false;

   if (mpLinePresenceMonitor)
   {
     bOk = mpLinePresenceMonitor->isOk();
   }

   return bOk;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
