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
#include <tapi/sipXtapi.h>
#include <utl/UtlInt.h>
#include <os/OsSysLog.h>
#include <utl/UtlHashMapIterator.h>
#include <xmlparser/tinyxml.h>
#include <net/Url.h>
#include <net/ProvisioningAgent.h>
#include <sipdb/CredentialDB.h>

#include "ACDServer.h"
#include "ACDLine.h"
#include "ACDCallManager.h"
#include "ACDQueueMsg.h"
#include "ACDLineManager.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
extern OsSysLogPriority gACD_DEBUG;

// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDLineManager::ACDLineManager
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

ACDLineManager::ACDLineManager(ACDServer* pAcdServer)
: ProvisioningClass(ACD_LINE_TAG), mLock(OsMutex::Q_FIFO)
{
   mpAcdServer = pAcdServer;
   mpAcdCallManager = NULL;
   mhAcdCallManagerHandle = NULL;
   mpAcdQueueManager = NULL;
   mCredentialDb = NULL;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDLineManager::~ACDLineManager
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

ACDLineManager::~ACDLineManager()
{
   UtlHashMapIterator iterator(mLineHandleMap);
   UtlInt* line;
   while ((line = dynamic_cast <UtlInt *> (iterator())) != NULL)
   {
      SIPX_LINE lineHandle = (SIPX_LINE) line->getValue();
      sipxLineRemove(lineHandle);

      OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDLineManager::~ACDLineManager - Line(%d): removed",
                    lineHandle);
   }

   mLock.acquire();
   if (mCredentialDb)
   {
      mCredentialDb->releaseInstance();
      mCredentialDb = NULL;
   }
   mLock.release();
}

/* ============================ MANIPULATORS ============================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDLineManager::initialize
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

OsStatus ACDLineManager::initialize(void)
{
   mpAcdCallManager = mpAcdServer->getAcdCallManager();
   mhAcdCallManagerHandle = mpAcdCallManager->getAcdCallManagerHandle();
   mpAcdQueueManager = mpAcdServer->getAcdQueueManager();

   // Register this with the Provisioning Agent
   mpAcdServer->getProvisioningAgent()->registerClass(this);

   return OS_SUCCESS;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDLineManager::start
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

OsStatus ACDLineManager::start(void)
{
   // Load the configuration
   loadConfiguration();

   return OS_SUCCESS;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDLineManager::createACDLine
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

ACDLine* ACDLineManager::createACDLine(const char* pLineUriString,
                                       const char* pName,
                                       const char* pExtension,
                                       bool        trunkMode,
                                       bool        publishLinePresence,
                                       const char* pAcdQueue)

{
   ACDLine*  pLineRef = NULL;
   SIPX_LINE lineHandle;

   OsSysLog::add(FAC_ACD, PRI_DEBUG,
                 "ACDLineManager::createACDLine"
                 "LineUriString '%s' Name '%s' Extension '%s' %s mode %s Queue '%s'",
                 pLineUriString, pName, pExtension,
                 trunkMode ? "trunk" : "line",
                 publishLinePresence ? "published" : "unpublished",
                 pAcdQueue);

   mLock.acquire();

   /*
    * In earlier versions, the line was defined to be queue@host, and
    * a sipXtapi alias for each line was defined as extension@domain.
    * This caused problems with authentication challenges because
    * the real line identities didn't match the header values.
    *
    * We now set the lines to be extension@domain, and don't create any
    * alias.
    */
   Url lineUri(pLineUriString, Url::AddrSpec, NULL);
   if (pExtension)
   {
      lineUri.setUserId(pExtension);
   }
   lineUri.setHostAddress(mpAcdServer->getDomain());
   UtlString lineUriString;
   lineUri.toString(lineUriString);
/*
   pName is actually the Description field on the WebUI, which is free form
   and not checked to be a valid display name and setDisplayName will abort
   if given a bad name.

   As the outbound calls set the name to be that of the inbound caller,
   I don't see any reason to set this.  --Woof!

   if (pName)
   {
      lineUri.setDisplayName(pName);
   }
*/

   // Call sipXtapi to add the line presence.
   if (sipxLineAdd(mhAcdCallManagerHandle, lineUriString, &lineHandle) == SIPX_RESULT_SUCCESS)
   {
      // If successful, create a matching ACDLine object.
      pLineRef = new ACDLine(this, lineHandle, lineUriString,
                             pName, pExtension, trunkMode, publishLinePresence, pAcdQueue);

      // Create a mapping between the sipXtapi line handle and the ACDLine instance.
      mLineHandleMap.insertKeyAndValue(new UtlInt(lineHandle), pLineRef);

      // Create a mapping between the ACDLine URI and the ACDLine instance.
      mAcdLineList.insertKeyAndValue(new UtlString(lineUriString), pLineRef);

      // Create a mapping between the ACDLine Name and the ACDLine instance.
      if ( pName && *pName )
      {
         mAcdLineNameList.insertKeyAndValue(new UtlString(pName), pLineRef);
      }

      OsSysLog::add(FAC_ACD, gACD_DEBUG,
                    "ACDLineManager::createACDLine"
                    " Line(%d) '%s' added",
                    lineHandle, lineUriString.data());

      /*
       * Construct a sipXtapi alias for the contact URI
       */
      Url contactUrl(pLineUriString);
      contactUrl.setHostPort(mpAcdCallManager->getSipPort());

      UtlString contactUrlString;
      contactUrl.toString(contactUrlString);

      if (contactUrlString.compareTo(lineUriString) != 0)
      {
         // Tell sipXtapi this contact is this line
         sipxLineAddAlias(lineHandle, contactUrlString.data());

         // Create a mapping between the ACDLine Extension and the ACDLine instance.
         mAcdLineExtensionList.insertKeyAndValue(new UtlString(contactUrlString), pLineRef);

         OsSysLog::add(FAC_ACD, gACD_DEBUG,
                       "ACDLineManager::createACDLine"
                       " Line(%d) alias '%s' added",
                       lineHandle, contactUrlString.data());
      }

   }
   else
   {
      //Error
      OsSysLog::add(FAC_ACD, PRI_ERR, "ACDLineManager::createACDLine - Failed to create line '%s'",
                    lineUriString.data());
   }

   mLock.release();

   return pLineRef;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDLineManager::deleteACDLine
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

void ACDLineManager::deleteACDLine(const char* pLineUriString, const char* pName, const char* pExtension)
{
   UtlContainable* pLineRef;
   SIPX_LINE       lineHandle;
   UtlContainable* pKey;

   mLock.acquire();

   // Remove the mapping between the ACDLine Name and the ACDLine instance.
   if ( pName && *pName )
   {
      const UtlString searchNameKey(pName);
      mAcdLineNameList.removeKeyAndValue(&searchNameKey, pLineRef);
   }

   // Remove the mapping between the ACDLine Extension and the ACDLine instance.
   if ( pExtension && *pExtension )
   {
      const UtlString searchExtensionKey(pExtension);
      mAcdLineExtensionList.removeKeyAndValue(&searchExtensionKey, pLineRef);
   }

   // Remove the mapping between the ACDLine URI and the ACDLine instance.
   const UtlString searchUriKey(pLineUriString);
   pKey = mAcdLineList.removeKeyAndValue(&searchUriKey, pLineRef);
   if (pKey == NULL) {
      // Error. Did not find a matching ACDLine object.
      OsSysLog::add(FAC_ACD, PRI_ERR, "ACDLineManager::deleteACDLine - Failed to find reference to line: %s",
                    pLineUriString);
      mLock.release();
      return;
   }
   delete pKey;

   // Remove the mapping between the sipXtapi line handle and the ACDLine instance.
   lineHandle = dynamic_cast<ACDLine*>(pLineRef)->getLineHandle();
   UtlInt searchHandleKey(lineHandle);
   pKey = mLineHandleMap.remove(&searchHandleKey);
   delete pKey;

   // Call sipXtapi to remove the line presence.
   if (sipxLineRemove(lineHandle) != SIPX_RESULT_SUCCESS) {
      OsSysLog::add(FAC_ACD, PRI_ERR, "ACDLineManager::deleteACDLine - Failed to delete line: %s,",
                    pLineUriString);
   }

   // Finally delete the ACDLine object.
   delete pLineRef;

   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDLineManager::deleteACDLine - Line(%d): %s deleted",
                 lineHandle, pLineUriString);

   mLock.release();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDLineManager::Create
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

ProvisioningAttrList* ACDLineManager::Create(ProvisioningAttrList& rRequestAttributes)
{
   ProvisioningAttrList* pResponse;

   UtlString             lineUriString;
   UtlString             name;
   UtlString             acdQueue;
   UtlString             extension;
   bool                  trunkMode;
   bool                  publishLinePresence;

   osPrintf("{method} = create\n{object-class} = acd-line\n");
   rRequestAttributes.dumpAttributes();
   osPrintf("\n");

   // Verify that the required set of attributes are there.
   try {
      rRequestAttributes.validateAttribute(LINE_URI_TAG,              ProvisioningAttrList::STRING);
      rRequestAttributes.validateAttribute(LINE_TRUNK_MODE_TAG,       ProvisioningAttrList::BOOL);
      rRequestAttributes.validateAttribute(LINE_PUBLISH_PRESENCE_TAG, ProvisioningAttrList::BOOL);
      rRequestAttributes.validateAttribute(LINE_ACD_QUEUE_TAG,        ProvisioningAttrList::STRING);
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
   rRequestAttributes.getAttribute(LINE_URI_TAG, lineUriString);

   mLock.acquire();

   // Verify that an instance of this object has not already been created.
   TiXmlNode* pInstanceNode = findPSInstance(ACD_LINE_TAG, LINE_URI_TAG, lineUriString);
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
      rRequestAttributes.validateAttributeType(LINE_NAME_TAG,      ProvisioningAttrList::STRING);
      rRequestAttributes.validateAttributeType(LINE_EXTENSION_TAG, ProvisioningAttrList::STRING);
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
   pInstanceNode = createPSInstance(ACD_LINE_TAG, LINE_URI_TAG, lineUriString);
   if (pInstanceNode == NULL) {
      // Instance creation failed.
      mLock.release();
      pResponse = new ProvisioningAttrList;
      pResponse->setAttribute("method-name", "create");
      pResponse->setAttribute("result-code", ProvisioningAgent::CREATE_FAILURE);
      pResponse->setAttribute("result-text", "Managed Object Instance creation failed");
      return pResponse;
   }

   // Now save the individual attributes

   // name (optional)
   if (rRequestAttributes.getAttribute(LINE_NAME_TAG, name)) {
      setPSAttribute(pInstanceNode, LINE_NAME_TAG, name);
   }

   // extension (optional)
   if (rRequestAttributes.getAttribute(LINE_EXTENSION_TAG, extension)) {
      setPSAttribute(pInstanceNode, LINE_EXTENSION_TAG, extension);
   }

   // trunk-mode
   rRequestAttributes.getAttribute(LINE_TRUNK_MODE_TAG, trunkMode);
   setPSAttribute(pInstanceNode, LINE_TRUNK_MODE_TAG, trunkMode);

   // publish-line-presence
   rRequestAttributes.getAttribute(LINE_PUBLISH_PRESENCE_TAG, publishLinePresence);
   setPSAttribute(pInstanceNode, LINE_PUBLISH_PRESENCE_TAG, publishLinePresence);

   // acd-queue
   rRequestAttributes.getAttribute(LINE_ACD_QUEUE_TAG, acdQueue);
   setPSAttribute(pInstanceNode, LINE_ACD_QUEUE_TAG, acdQueue);

   // If Administrative State == ACTIVE, create the ACDLine
   if (mpAcdServer->getAdministrativeState() == ACDServer::ACTIVE) {
      createACDLine(lineUriString, name, extension, trunkMode, publishLinePresence, acdQueue);
   }

   // Update the configuration file
   OsSysLog::add(LOG_FACILITY, PRI_INFO, "ACDLineManager::Create - Updating the config file");
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
//  NAME:        ACDLineManager::Delete
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

ProvisioningAttrList* ACDLineManager::Delete(ProvisioningAttrList& rRequestAttributes)
{
   ProvisioningAttrList* pResponse;
   UtlString             lineUriString;
   UtlString             lineNameString;
   UtlString             lineExtensionString;

   osPrintf("{method} = delete\n{object-class} = acd-line\n");
   rRequestAttributes.dumpAttributes();
   osPrintf("\n");

   // Extract the instance index from the request attributes.
   rRequestAttributes.getAttribute(LINE_URI_TAG, lineUriString);
   rRequestAttributes.getAttribute(LINE_NAME_TAG, lineNameString);
   rRequestAttributes.getAttribute(LINE_EXTENSION_TAG, lineExtensionString);

   mLock.acquire();

   // Verify that the instance has already been created.
   TiXmlNode* pInstanceNode = findPSInstance(ACD_LINE_TAG, LINE_URI_TAG, lineUriString);
   if (pInstanceNode == NULL) {
      // There is no instance.
      mLock.release();
      pResponse = new ProvisioningAttrList;
      pResponse->setAttribute("method-name", "delete");
      pResponse->setAttribute("result-code", ProvisioningAgent::UNKNOWN_OBJECT);
      pResponse->setAttribute("result-text", "Unknown instance");
      return pResponse;
   }

   // If Administrative State == ACTIVE, delete the ACDLine
   // For now do not do anything
#if 0
   if (mpAcdServer->getAdministrativeState() == ACDServer::ACTIVE) {
      deleteACDLine(lineUriString, lineNameString, lineExtensionString);
   }
#endif

   // Remove the instance from the configuration file
   deletePSInstance(ACD_LINE_TAG, LINE_URI_TAG, lineUriString);

   // Update the configuration file
   OsSysLog::add(LOG_FACILITY, PRI_INFO, "ACDLineManager::Delete - Updating the config file");
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
//  NAME:        ACDLineManager::Set
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

ProvisioningAttrList* ACDLineManager::Set(ProvisioningAttrList& rRequestAttributes)
{
   ProvisioningAttrList* pResponse;

   UtlString             lineUriString;
   UtlString             name;
   UtlString             acdQueue;
   UtlString             extension;
   bool                  trunkMode;
   bool                  publishLinePresence;

   osPrintf("{method} = set\n{object-class} = acd-line\n");
   rRequestAttributes.dumpAttributes();
   osPrintf("\n");

   // Extract the instance index from the request attributes.
   rRequestAttributes.getAttribute(LINE_URI_TAG, lineUriString);

   mLock.acquire();

   // Verify that the instance has already been created.
   TiXmlNode* pInstanceNode = findPSInstance(ACD_LINE_TAG, LINE_URI_TAG, lineUriString);
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
      rRequestAttributes.validateAttributeType(LINE_NAME_TAG,             ProvisioningAttrList::STRING);
      rRequestAttributes.validateAttributeType(LINE_EXTENSION_TAG,        ProvisioningAttrList::STRING);
      rRequestAttributes.validateAttributeType(LINE_TRUNK_MODE_TAG,       ProvisioningAttrList::BOOL);
      rRequestAttributes.validateAttributeType(LINE_PUBLISH_PRESENCE_TAG, ProvisioningAttrList::BOOL);
      rRequestAttributes.validateAttributeType(LINE_ACD_QUEUE_TAG,        ProvisioningAttrList::STRING);
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

   // If Administrative State == ACTIVE, update the ACDLine
   if (mpAcdServer->getAdministrativeState() == ACDServer::ACTIVE) {
      // Lookup the instance
      ACDLine* pLineRef = getAcdLineReference(lineUriString);
      if (pLineRef == NULL) {
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
         pLineRef->setAttributes(rRequestAttributes);
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
   // name
   if (rRequestAttributes.getAttribute(LINE_NAME_TAG, name)) {
      setPSAttribute(pInstanceNode, LINE_NAME_TAG, name);
   }

   // extension
   if (rRequestAttributes.getAttribute(LINE_EXTENSION_TAG, extension)) {
      setPSAttribute(pInstanceNode, LINE_EXTENSION_TAG, extension);
   }

   // trunk-mode
   if (rRequestAttributes.getAttribute(LINE_TRUNK_MODE_TAG, trunkMode)) {
      setPSAttribute(pInstanceNode, LINE_TRUNK_MODE_TAG, trunkMode);
   }

   // publish-line-presence
   if (rRequestAttributes.getAttribute(LINE_PUBLISH_PRESENCE_TAG, publishLinePresence)) {
      setPSAttribute(pInstanceNode, LINE_PUBLISH_PRESENCE_TAG, publishLinePresence);
   }

   // acd-queue
   if (rRequestAttributes.getAttribute(LINE_ACD_QUEUE_TAG, acdQueue)) {
      setPSAttribute(pInstanceNode, LINE_ACD_QUEUE_TAG, acdQueue);
   }

   // Update the configuration file
   OsSysLog::add(LOG_FACILITY, PRI_INFO, "ACDLineManager::Set - Updating the config file");
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
//  NAME:        ACDLineManager::Get
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

ProvisioningAttrList* ACDLineManager::Get(ProvisioningAttrList& rRequestAttributes)
{
   ProvisioningAttrList* pResponse;
   ProvisioningAttrList* pLineInstance;
   UtlString             lineUriString;
   UtlString             lineNameString;
   UtlString             lineExtensionString;
   UtlSList*             pLineList;

   osPrintf("{method} = get\n{object-class} = acd-line\n");
   rRequestAttributes.dumpAttributes();
   osPrintf("\n");

   mLock.acquire();

   // Extract the instance index from the request attributes.
   if (rRequestAttributes.getAttribute(LINE_URI_TAG, lineUriString) ||
       rRequestAttributes.getAttribute(LINE_NAME_TAG, lineNameString) ||
       rRequestAttributes.getAttribute(LINE_EXTENSION_TAG, lineExtensionString) ) {
      // A specific instance has been specified, verify that it exists
      ACDLine* pLineRef = NULL;

      // A request for a URI mapping must have the name@server format
      if ( lineUriString.contains("@") ) {
         pLineRef = getAcdLineReference(lineUriString);
      }
      else {
         // A request for a NAME mapping
         if ( ! lineNameString.isNull() )
            pLineRef = getAcdLineReferenceByName(lineNameString);

         // A request for a EXTENSION mapping
         else if ( ! lineExtensionString.isNull() )
            pLineRef = getAcdLineReferenceByExtension(lineExtensionString);
      }

      if (pLineRef == NULL) {
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
         pLineRef->getAttributes(rRequestAttributes, pResponse);
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
      pResponse->setAttribute("object-class", ACD_LINE_TAG);
      pResponse->setAttribute(LINE_URI_TAG, lineUriString);
      return pResponse;
   }
   else {
      // No specific instance was requested, send back a list of the available instances
      // Find the first instance.
      TiXmlNode* pInstanceNode = findPSInstance(ACD_LINE_TAG);
      pLineList = new UtlSList;

      // Build up a list of instances
      while (pInstanceNode != NULL) {
         // Read the index parameter
         getPSAttribute(pInstanceNode, LINE_URI_TAG, lineUriString);

         // Create the list entry
         pLineInstance = new ProvisioningAttrList;
         pLineInstance->setAttribute("object-class", ACD_LINE_TAG);
         pLineInstance->setAttribute(LINE_URI_TAG, lineUriString);
         pLineList->append(pLineInstance->getData());

         // Get the next instance.
         pInstanceNode = pInstanceNode->NextSibling();
      }

      // Send back the response
      mLock.release();
      pResponse = new ProvisioningAttrList;
      pResponse->setAttribute("method-name", "get");
      pResponse->setAttribute("result-code", ProvisioningAgent::SUCCESS);
      pResponse->setAttribute("result-text", "SUCCESS");
      pResponse->setAttribute("object-class-list", pLineList);
      return pResponse;
   }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDLineManager::loadConfiguration
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

bool ACDLineManager::loadConfiguration(void)
{
   UtlString             lineUriString;
   UtlString             name;
   UtlString             acdQueue;
   UtlString             extension;
   bool                  trunkMode;
   bool                  publishLinePresence;

   mLock.acquire();

   // Find the first instance.
   TiXmlNode* pInstanceNode = findPSInstance(ACD_LINE_TAG);
   if (pInstanceNode == NULL) {
      // There is no instances.
      mLock.release();
      return false;
   }

   while (pInstanceNode != NULL) {
      // Set default values
      name      = "";
      extension = "";

      // Read in the parameters
      getPSAttribute(pInstanceNode, LINE_URI_TAG,              lineUriString);
      getPSAttribute(pInstanceNode, LINE_NAME_TAG,             name);
      getPSAttribute(pInstanceNode, LINE_EXTENSION_TAG,        extension);
      getPSAttribute(pInstanceNode, LINE_TRUNK_MODE_TAG,       trunkMode);
      getPSAttribute(pInstanceNode, LINE_PUBLISH_PRESENCE_TAG, publishLinePresence);
      getPSAttribute(pInstanceNode, LINE_ACD_QUEUE_TAG,        acdQueue);

      // Create the ACDLine
      createACDLine(lineUriString, name, extension, trunkMode, publishLinePresence, acdQueue);

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
//  NAME:        ACDLineManager::getAcdCallManagerHandle
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

SIPX_INST ACDLineManager::getAcdCallManagerHandle(void)
{
   return mhAcdCallManagerHandle;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDLineManager::getAcdCallManager
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

ACDCallManager* ACDLineManager::getAcdCallManager(void)
{
   return mpAcdCallManager;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDLineManager::getAcdQueueManager
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

ACDQueueManager* ACDLineManager::getAcdQueueManager(void)
{
   return mpAcdQueueManager;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDLineManager::getAcdLineReference
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

ACDLine* ACDLineManager::getAcdLineReference(SIPX_LINE hLineHandle)
{
   ACDLine*  pLineRef;
   UtlInt    key(hLineHandle);

   mLock.acquire();

   pLineRef = dynamic_cast<ACDLine*>(mLineHandleMap.findValue(&key));

   mLock.release();

   return pLineRef;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDLineManager::getAcdLineReference
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

ACDLine* ACDLineManager::getAcdLineReference(UtlString& rLineUriString)
{
   mLock.acquire();

   SIPX_LINE hLine = SIPX_LINE_NULL ;

   sipxLookupLine(mhAcdCallManagerHandle, rLineUriString.data(), hLine) ;

   mLock.release();

   return getAcdLineReference(hLine);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDLineManager::getAcdServer
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

ACDServer* ACDLineManager::getAcdServer(void)
{
   return mpAcdServer;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDLineManager::getAcdLineReferenceByName
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

ACDLine* ACDLineManager::getAcdLineReferenceByName(UtlString& rLineNameString)
{
   ACDLine*  pLineRef;

   mLock.acquire();

   pLineRef = dynamic_cast<ACDLine*>(mAcdLineNameList.findValue(&rLineNameString));

   mLock.release();

   return pLineRef;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDLineManager::getAcdLineReferenceByExtension
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

ACDLine* ACDLineManager::getAcdLineReferenceByExtension(UtlString& rLineExtensionString)
{
   ACDLine*  pLineRef;

   mLock.acquire();

   pLineRef = dynamic_cast<ACDLine*>(mAcdLineExtensionList.findValue(&rLineExtensionString));

   mLock.release();

   return pLineRef;
}

/// Get a handle to the database where we look up line credentials.
CredentialDB* ACDLineManager::getCredentialDb(void)
{
   mLock.acquire();
   if (!mCredentialDb)
   {
      mCredentialDb = CredentialDB::getInstance();
   }
   mLock.release();

   return mCredentialDb;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
