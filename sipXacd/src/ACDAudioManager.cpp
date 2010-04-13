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
#include <utl/UtlSList.h>
#include <xmlparser/tinyxml.h>
#include <net/ProvisioningAgent.h>
#include <sipXecsService/SipXecsService.h>
#include "ACDServer.h"
#include "ACDAudio.h"
#include "ACDAudioManager.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
extern OsSysLogPriority gACD_DEBUG;

// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAudioManager::ACDAudioManager
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

ACDAudioManager::ACDAudioManager(ACDServer* pAcdServer)
: ProvisioningClass(ACD_AUDIO_TAG), mLock(OsMutex::Q_FIFO)
{
   mpAcdServer = pAcdServer;

   // Check for the existance of the ACD Audio Store directory
   UtlString acdAudioSubdir;
   acdAudioSubdir += OsPath::separator;
   acdAudioSubdir += SIPX_ACD_DIR;
   acdAudioSubdir += OsPath::separator;
   acdAudioSubdir += SIPX_ACD_AUDIO_DIR;

   mAudioStoreDirectory  = SipXecsService::Path(SipXecsService::LocalStateDirType,
                                                acdAudioSubdir.data());

   if (!OsFileSystem::exists(mAudioStoreDirectory)) {
      if (OsFileSystem::createDir(mAudioStoreDirectory,TRUE /* create Parent */) != OS_SUCCESS) {
         OsSysLog::add(FAC_ACD, PRI_ERR, "ACDAudioManager::ACDAudioManager - "
                       "failed to establish audio store directory: %s", mAudioStoreDirectory.data());
         mAudioStoreDirectory.remove(0);
      }
   }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAudioManager::~ACDAudioManager
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

ACDAudioManager::~ACDAudioManager()
{
}

/* ============================ MANIPULATORS ============================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAudioManager::initialize
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

OsStatus ACDAudioManager::initialize(void)
{
   // Register this with the Provisioning Agent
   mpAcdServer->getProvisioningAgent()->registerClass(this);

   return OS_SUCCESS;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAudioManager::start
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

OsStatus ACDAudioManager::start(void)
{
   // Load the configuration
   loadConfiguration();

   return OS_SUCCESS;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAudioManager::createACDAudio
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

ACDAudio* ACDAudioManager::createACDAudio(const char* pName,
                                          bool        localStore,
                                          const char* pAudioUriString,
                                          const char* pDescription)
{
   ACDAudio*  pAudioRef;

   mLock.acquire();

   // Create an ACDAudio object.
   pAudioRef = new ACDAudio(this, pName, localStore, pAudioUriString, pDescription);

   // Create a mapping between the ACDAudio name and the ACDAudio instance.
   mAcdAudioList.insertKeyAndValue(new UtlString(pName), pAudioRef);

   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDAudioManager::createACDAudio - Audio added: %s",
                 pName);

   mLock.release();

   return pAudioRef;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAudioManager::deleteACDAudio
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

void ACDAudioManager::deleteACDAudio(const char* pName)
{
   UtlContainable* pAudioRef;
   UtlContainable* pKey;

   mLock.acquire();

   // Remove the mapping between the ACDAudio name and the ACDAudio instance.
   const UtlString searchUriKey(pName);
   pKey = mAcdAudioList.removeKeyAndValue(&searchUriKey, pAudioRef);
   if (pKey == NULL) {
      // Error. Did not find a matching ACDAudio object.
      OsSysLog::add(FAC_ACD, PRI_ERR, "ACDAudioManager::deleteACDAudio - Failed to find reference to Audio: %s",
                    pName);
      mLock.release();
      return;
   }
   delete pKey;

   // Signal the ACDAudio's associated task to shutdown
   dynamic_cast<ACDAudio*>(pAudioRef)->requestShutdown();
   while (!dynamic_cast<ACDAudio*>(pAudioRef)->isShutDown()) {
      // Wait for it to complete
   }

   // Tell the ACDAudio object to clean up it's local store
   dynamic_cast<ACDAudio*>(pAudioRef)->deleteLocalStore();

   // Finally delete the ACDAudio object.
   delete pAudioRef;

   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDAudioManager::deleteACDAudio - Audio: %s deleted",
                 pName);

   mLock.release();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAudioManager::Create
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

ProvisioningAttrList* ACDAudioManager::Create(ProvisioningAttrList& rRequestAttributes)
{
   ProvisioningAttrList* pResponse;

   UtlString             name;
   UtlString             uriString;
   UtlString             description;
   bool                  localStore;

   osPrintf("{method} = create\n{object-class} = acd-audio\n");
   rRequestAttributes.dumpAttributes();
   osPrintf("\n");

   // Verify that the required set of attributes are there.
   try {
      rRequestAttributes.validateAttribute(AUDIO_NAME_TAG,        ProvisioningAttrList::STRING);
      rRequestAttributes.validateAttribute(AUDIO_LOCAL_STORE_TAG, ProvisioningAttrList::BOOL);
      rRequestAttributes.validateAttribute(AUDIO_URI_TAG,         ProvisioningAttrList::STRING);
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
   rRequestAttributes.getAttribute(AUDIO_NAME_TAG, name);

   mLock.acquire();

   // Verify that an instance of this object has not already been created.
   TiXmlNode* pInstanceNode = findPSInstance(ACD_AUDIO_TAG, AUDIO_NAME_TAG, name);
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
      rRequestAttributes.validateAttributeType(AUDIO_DESCRIPTION_TAG, ProvisioningAttrList::STRING);
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
   pInstanceNode = createPSInstance(ACD_AUDIO_TAG, AUDIO_NAME_TAG, name);
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

   // local-store
   if (rRequestAttributes.getAttribute(AUDIO_LOCAL_STORE_TAG, localStore)) {
      setPSAttribute(pInstanceNode, AUDIO_LOCAL_STORE_TAG, localStore);
   }

   // uri (optional)
   if (rRequestAttributes.getAttribute(AUDIO_URI_TAG, uriString)) {
      setPSAttribute(pInstanceNode, AUDIO_URI_TAG, uriString);
   }

   // description (optional)
   if (rRequestAttributes.getAttribute(AUDIO_DESCRIPTION_TAG, description)) {
      setPSAttribute(pInstanceNode, AUDIO_DESCRIPTION_TAG, description);
   }

   // If Administrative State == ACTIVE, create the ACDAgent
   if (mpAcdServer->getAdministrativeState() == ACDServer::ACTIVE) {
      createACDAudio(name, localStore, uriString, description);
   }

   // Update the configuration file
   OsSysLog::add(LOG_FACILITY, PRI_INFO, "ACDAudioManager::Create - Updating the config file");
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
//  NAME:        ACDAudioManager::Delete
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

ProvisioningAttrList* ACDAudioManager::Delete(ProvisioningAttrList& rRequestAttributes)
{
   ProvisioningAttrList* pResponse;
   UtlString             name;

   osPrintf("{method} = delete\n{object-class} = acd-audio\n");
   rRequestAttributes.dumpAttributes();
   osPrintf("\n");

   // Extract the instance index from the request attributes.
   rRequestAttributes.getAttribute(AUDIO_NAME_TAG, name);

   mLock.acquire();

   // Verify that the instance has already been created.
   TiXmlNode* pInstanceNode = findPSInstance(ACD_AUDIO_TAG, AUDIO_NAME_TAG, name);
   if (pInstanceNode == NULL) {
      // There is no instance.
      mLock.release();
      pResponse = new ProvisioningAttrList;
      pResponse->setAttribute("method-name", "delete");
      pResponse->setAttribute("result-code", ProvisioningAgent::UNKNOWN_OBJECT);
      pResponse->setAttribute("result-text", "Unknown instance");
      return pResponse;
   }

   // If Administrative State == ACTIVE, delete the ACDAudio object
   // For now - do not allow this to be deleted on fly - for protection
#if 0
   if (mpAcdServer->getAdministrativeState() == ACDServer::ACTIVE) {
      deleteACDAudio(name);
   }
#endif

   // Remove the instance from the configuration file
   deletePSInstance(ACD_AUDIO_TAG, AUDIO_NAME_TAG, name);

   // Update the configuration file
   OsSysLog::add(LOG_FACILITY, PRI_INFO, "ACDAudioManager::Delete - Updating the config file");
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
//  NAME:        ACDAudioManager::Set
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

ProvisioningAttrList* ACDAudioManager::Set(ProvisioningAttrList& rRequestAttributes)
{
   ProvisioningAttrList* pResponse;

   UtlString             name;
   UtlString             uriString;
   UtlString             description;
   bool                  localStore;

   osPrintf("{method} = set\n{object-class} = acd-audio\n");
   rRequestAttributes.dumpAttributes();
   osPrintf("\n");

   // Extract the instance index from the request attributes.
   rRequestAttributes.getAttribute(AUDIO_NAME_TAG, name);

   mLock.acquire();

   // Verify that the instance has already been created.
   TiXmlNode* pInstanceNode = findPSInstance(ACD_AUDIO_TAG, AUDIO_NAME_TAG, name);
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
      rRequestAttributes.validateAttributeType(AUDIO_URI_TAG,         ProvisioningAttrList::STRING);
      rRequestAttributes.validateAttributeType(AUDIO_DESCRIPTION_TAG, ProvisioningAttrList::STRING);
      rRequestAttributes.validateAttributeType(AUDIO_LOCAL_STORE_TAG, ProvisioningAttrList::BOOL);
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

   // If Administrative State == ACTIVE, update the ACDAudio object
   if (mpAcdServer->getAdministrativeState() == ACDServer::ACTIVE) {
      // Lookup the instance
      ACDAudio* pAudioRef = getAcdAudioReference(name);
      if (pAudioRef == NULL) {
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
         pAudioRef->setAttributes(rRequestAttributes);
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
   // local-store
   if (rRequestAttributes.getAttribute(AUDIO_LOCAL_STORE_TAG, localStore)) {
      setPSAttribute(pInstanceNode, AUDIO_LOCAL_STORE_TAG, localStore);
   }

   // uri
   if (rRequestAttributes.getAttribute(AUDIO_URI_TAG, uriString)) {
      setPSAttribute(pInstanceNode, AUDIO_URI_TAG, uriString);
   }

   // description
   if (rRequestAttributes.getAttribute(AUDIO_DESCRIPTION_TAG, description)) {
      setPSAttribute(pInstanceNode, AUDIO_DESCRIPTION_TAG, description);
   }

   // Update the configuration file
   OsSysLog::add(LOG_FACILITY, PRI_INFO, "ACDAudioManager::Set - Updating the config file");
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
//  NAME:        ACDAudioManager::Get
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

ProvisioningAttrList* ACDAudioManager::Get(ProvisioningAttrList& rRequestAttributes)
{
   ProvisioningAttrList* pResponse;
   ProvisioningAttrList* pAudioInstance;
   UtlString             name;
   UtlSList*             pAudioList;


   osPrintf("{method} = get\n{object-class} = acd-audio\n");
   rRequestAttributes.dumpAttributes();
   osPrintf("\n");

   mLock.acquire();

   // Extract the instance index from the request attributes.
   if (rRequestAttributes.getAttribute(AUDIO_NAME_TAG, name)) {
      // A specific instance has been specified, verify that it exists
      ACDAudio* pAudioRef = getAcdAudioReference(name);
      if (pAudioRef == NULL) {
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
         pAudioRef->getAttributes(rRequestAttributes, pResponse);
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
      pResponse->setAttribute("object-class", ACD_AUDIO_TAG);
      pResponse->setAttribute(AUDIO_NAME_TAG, name);
      return pResponse;
   }
   else {
      // No specific instance was requested, send back a list of the available instances
      // Find the first instance.
      TiXmlNode* pInstanceNode = findPSInstance(ACD_AUDIO_TAG);
      pAudioList = new UtlSList;

      // Build up a list of instances
      while (pInstanceNode != NULL) {
         // Read the index parameter
         getPSAttribute(pInstanceNode, AUDIO_NAME_TAG, name);

         // Create the list entry
         pAudioInstance = new ProvisioningAttrList;
         pAudioInstance->setAttribute("object-class", ACD_AUDIO_TAG);
         pAudioInstance->setAttribute(AUDIO_NAME_TAG, name);
         pAudioList->append(pAudioInstance->getData());

         // Get the next instance.
         pInstanceNode = pInstanceNode->NextSibling();
      }

      // Send back the response
      mLock.release();
      pResponse = new ProvisioningAttrList;
      pResponse->setAttribute("method-name", "get");
      pResponse->setAttribute("result-code", ProvisioningAgent::SUCCESS);
      pResponse->setAttribute("result-text", "SUCCESS");
      pResponse->setAttribute("object-class-list", pAudioList);
      return pResponse;
   }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAudioManager::loadConfiguration
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

bool ACDAudioManager::loadConfiguration(void)
{
   UtlString             name;
   UtlString             uriString;
   UtlString             description;
   bool                  localStore;

   mLock.acquire();

   // Find the first instance.
   TiXmlNode* pInstanceNode = findPSInstance(ACD_AUDIO_TAG);
   if (pInstanceNode == NULL) {
      // There is no instances.
      mLock.release();
      return false;
   }

   while (pInstanceNode != NULL) {
      // Set default values
      uriString   = "";
      description = "";

      // Read in the parameters
      getPSAttribute(pInstanceNode, AUDIO_NAME_TAG,        name);
      getPSAttribute(pInstanceNode, AUDIO_LOCAL_STORE_TAG, localStore);
      getPSAttribute(pInstanceNode, AUDIO_URI_TAG,         uriString);
      getPSAttribute(pInstanceNode, AUDIO_DESCRIPTION_TAG, description);

      // Create the ACDAudio object
      createACDAudio(name, localStore, uriString, description);

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
//  NAME:        ACDAudioManager::getAcdAudioReference
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

ACDAudio* ACDAudioManager::getAcdAudioReference(UtlString& rName)
{
   ACDAudio*  pAudioRef;

   mLock.acquire();

   pAudioRef = dynamic_cast<ACDAudio*>(mAcdAudioList.findValue(&rName));

   mLock.release();

   return pAudioRef;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAudioManager::getAudioStoreDirectory
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

OsPath& ACDAudioManager::getAudioStorePath(void)
{
   return mAudioStoreDirectory;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDAudioManager::getAudio
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

bool ACDAudioManager::getAudio(UtlString& rName, char*& prAudio, unsigned long& rLength)
{
   ACDAudio*  pAudioRef = getAcdAudioReference(rName);

   if (pAudioRef != NULL) {
      return pAudioRef->getAudio(prAudio, rLength);
   }
   else {
      return false;
   }
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
