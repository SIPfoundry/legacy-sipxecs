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
#include <tapi/sipXtapiEvents.h>
#include <tapi/sipXtapiInternal.h>
#include <os/OsSysLog.h>
#include <os/OsDateTime.h>
#include <net/SipUserAgent.h>
#include <net/SipDialogEvent.h>
#include <sipdb/CredentialDB.h>
#include <cp/CallManager.h>
#include "ACDCallManager.h"
#include "ACDLineManager.h"
#include "ACDQueueManager.h"
#include "ACDCall.h"
#include "ACDLine.h"
#include "ACDQueue.h"
#include "ACDServer.h"
#include "ACDRtRecord.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
extern OsSysLogPriority gACD_DEBUG;
extern char realfile[64];

// CONSTANTS
#define STATE "full"

// STATIC VARIABLE INITIALIZATIONS
const UtlContainableType ACDLine::TYPE = "ACDLine";


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDLine::ACDLine
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

ACDLine::ACDLine(ACDLineManager* pAcdLineManager,
                 SIPX_LINE       lineHandle,
                 const char*     pLineUriString,
                 const char*     pName,
                 const char*     pExtension,
                 bool            trunkMode,
                 bool            publishLinePresence,
                 const char*     pAcdQueue)
: mLock(OsMutex::Q_FIFO)
{
   mpAcdLineManager     = pAcdLineManager;
   mhLineHandle         = lineHandle;
   mUriString           = pLineUriString;
   mUri                 = pLineUriString;
   mName                = pName;
   mExtension           = pExtension;
   mTrunkMode           = trunkMode;
   mPublishLinePresence = publishLinePresence;
   mAcdQueue            = pAcdQueue;
   mDialogId            = 1;
   mhPublisherHandle    = 0;
   mDialogPDULength     = 0;

   OsSysLog::add(FAC_ACD, PRI_DEBUG,
                 "ACDLine::ACDLine mUriString= '%s'",
                 mUriString.data()
                 );

   mpAcdCallManager = mpAcdLineManager->getAcdCallManager();
   mhAcdCallManagerHandle = mpAcdLineManager->getAcdCallManagerHandle();

   UtlString lineIdentity;
   mUri.getIdentity(lineIdentity);

   // Create the dialog publisher
   if (mPublishLinePresence) {
      mpDialogEventPackage = new SipDialogEvent(STATE, lineIdentity);

      mpDialogEventPackage->buildBodyGetBytes(&mDialogPDU, &mDialogPDULength);

      sipxPublisherCreate(mhAcdCallManagerHandle,
                          &mhPublisherHandle,
                          lineIdentity,
                          DIALOG_EVENT_TYPE,
                          DIALOG_EVENT_CONTENT_TYPE,
                          mDialogPDU.data(),
                          mDialogPDULength);
   }

   CredentialDB* credentialDb = mpAcdLineManager->getCredentialDb();
   if (credentialDb)
   {
      UtlString user;
      UtlString ha1_authenticator;
      UtlString authtype;
      UtlString realm(mpAcdLineManager->getAcdServer()->getRealm());

      if (credentialDb->getCredential(mUri, realm, user, ha1_authenticator, authtype))
      {
         if (SIPX_RESULT_SUCCESS
             == sipxLineAddDigestCredential(lineHandle, user, ha1_authenticator, realm))
         {
            OsSysLog::add(FAC_ACD, PRI_DEBUG,
                          "ACDLine::ACDLine added credentials for "
                          "identity '%s': user '%s'",
                          lineIdentity.data(), user.data()
                          );
         }
         else
         {
            OsSysLog::add(FAC_ACD, PRI_ERR,
                          "ACDLine::ACDLine setting credentials failed:"
                          " any blind transfer may not work."
                          );
         }
      }
      else
      {
         // could not get credentials for the line identity - try for a default ACD identity
         OsSysLog::add(FAC_ACD, PRI_WARNING, "ACDLine::ACDLine"
                       " no line-specific credentials found for '%s' in realm '%s'",
                       lineIdentity.data(), realm.data()
                       );

         Url defaultAcdIdentity;
         mpAcdLineManager->getAcdServer()->getDefaultIdentity(defaultAcdIdentity);

         if (credentialDb->getCredential(defaultAcdIdentity, realm,
                                         user, ha1_authenticator, authtype))
         {
            if (SIPX_RESULT_SUCCESS
                == sipxLineAddDigestCredential(lineHandle, user, ha1_authenticator, realm))
            {
               OsSysLog::add(FAC_ACD, PRI_DEBUG,
                             "ACDLine::ACDLine added default (%s) credentials for "
                             "identity '%s': user '%s'",
                             defaultAcdIdentity.toString().data(),
                             lineIdentity.data(), user.data()
                             );
            }
            else
            {
               OsSysLog::add(FAC_ACD, PRI_ERR,
                             "ACDLine::ACDLine setting default credentials failed:"
                             " any blind transfer may not work."
                             );
            }
         }
         else
         {
            OsSysLog::add(FAC_ACD, PRI_ERR,
                          "ACDLine::ACDLine no default (%s) credentials found in realm '%s'"
                          "; transfer functions will not work",
                          defaultAcdIdentity.toString().data(), realm.data()
                          );
         }
      }
   }
   else
   {
      OsSysLog::add(FAC_ACD, PRI_ERR,
                    "ACDLine::ACDLine failed to open credentials database"
                    "; transfer functions will not work");
   }

   mLineBusy = false;
   mDialogId = 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDLine::~ACDLine
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

ACDLine::~ACDLine()
{
}

/* ============================ MANIPULATORS ============================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDLine::addCall
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

bool ACDLine::addCall(ACDCall* pCallRef)
{
   // Verify that there is an ACDQueue configured for this line
   if (mAcdQueue == NULL) {
      // No ACDQueue configured, reject the request
      OsSysLog::add(FAC_ACD, PRI_ERR, "ACDLine::addCall - Call(%d) rejected due to no ACDQueue configured",
                    pCallRef->getCallHandle());
      return false;
   }


   // Call the ACDQueueManager to get the ACDQueue reference
   ACDQueue* pDestinationQueue = mpAcdLineManager->getAcdQueueManager()->getAcdQueueReference(mAcdQueue);
   if (pDestinationQueue == NULL) {
      // The ACDQueueManager doesn't know about this Queue.  Must be a bad configuration, reject!
      OsSysLog::add(FAC_ACD, PRI_ERR, "ACDLine::addCall - Call(%d) rejected due to no ACDQueue configured",
                    pCallRef->getCallHandle());
      return false;
   }

   // If this is not a trunk line, update the line busy flag
   if (!mTrunkMode) {
      mLineBusy = true;
   }

   // Now send the call to the ACDQueue assigned to this line.
   ACDRtRecord* pACDRtRec;
   if (NULL != (pACDRtRec = mpAcdLineManager->getAcdServer()->getAcdRtRecord())) {
      pACDRtRec->appendCallEvent(ACDRtRecord::ENTER_QUEUE, *(pDestinationQueue->getUriString()), pCallRef);
   }

   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDLine::addCall - Call(%d) sent to ACDQueue: %s",
                 pCallRef->getCallHandle(), pDestinationQueue->getUriString()->data());

   pDestinationQueue->addCall(pCallRef);

   return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDLine::deleteCall
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

void ACDLine::deleteCall(ACDCall* pCallRef)
{
   // If this is not a trunk line, update the line busy flag
   if (!mTrunkMode) {
      mLineBusy = false;
   }
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDLine::deleteCall - Call(%d)",
                 pCallRef->getCallHandle());
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDLine::publishCallState
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

OsStatus ACDLine::publishCallState(ACDCall* pCallRef, ACDCall::eCallState state)
{
   SipDialog sipDialog;
   char dialogId[10];
   Dialog* pDialog;
   UtlString callId, remoteAddress;
   UtlString localTag, remoteTag;
   UtlString localIdentity, localDisplayName;
   UtlString remoteIdentity, remoteDisplayName;
   Url localUrl, remoteUrl;
   Url localTarget, remoteTarget;


   // See if dialog event publishing is enabled for this line
   if (!mPublishLinePresence) {
      // Not enabled, return
      return OS_SUCCESS;
   }

   SIPX_INSTANCE_DATA* pInst;
   sipxCallGetCommonData(pCallRef->getCallHandle(), &pInst, &callId, &remoteAddress, NULL);
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDLine::publishCallState - Publishing Call state for: hCall = %d, callId = %s, remoteAddress = %s, state = %s",
                 pCallRef->getCallHandle(), callId.data(), remoteAddress.data(),
                 (state == ACDCall::ALERTING ? "ALERTING" :
                  state == ACDCall::CONNECTED ? "CONNECTED" :
                  state == ACDCall::DISCONNECTED ? "DISCONNECTED" :
                  "???"));

   switch (state) {
      case ACDCall::ALERTING:
         // Create the dialog element
         if (pInst->pCallManager->getSipDialog(callId, remoteAddress, sipDialog) != OS_SUCCESS) {
            OsSysLog::add(FAC_ACD, PRI_ERR, "ACDLine::publishCallState - Failed call to getSipDialog(%s, %s)\n",
                          callId.data(), remoteAddress.data());
            return OS_FAILED;
         }

         // Extract the local and remote identities and tags
         sipDialog.getLocalField(localUrl);
         localUrl.getFieldParameter("tag", localTag);

         sipDialog.getRemoteField(remoteUrl);
         remoteUrl.getFieldParameter("tag", remoteTag);

         localUrl.getIdentity(localIdentity);
         localUrl.getDisplayName(localDisplayName);

         remoteUrl.getIdentity(remoteIdentity);
         remoteUrl.getDisplayName(remoteDisplayName);

         sipDialog.getLocalContact(localTarget);
         sipDialog.getRemoteContact(remoteTarget);

         sprintf(dialogId, "%ld", mDialogId);

         mLock.acquire();
         mDialogId++;
         mLock.release();

         pDialog = new Dialog(dialogId, callId, localTag, remoteTag, "recipient");

         pDialog->setState(STATE_EARLY, NULL, NULL);
         pDialog->setLocalIdentity(localIdentity, localDisplayName);
         pDialog->setRemoteIdentity(remoteIdentity, remoteDisplayName);
         pDialog->setLocalTarget(localTarget.toString());
         pDialog->setRemoteTarget(remoteTarget.toString());
         pDialog->setDuration(OsDateTime::getSecsSinceEpoch());

         // Insert it into the active call list
         mpDialogEventPackage->insertDialog(pDialog);

         // Send the content to the subscribe server
         mLock.acquire();
         mpDialogEventPackage->buildBodyGetBytes(&mDialogPDU, &mDialogPDULength);
         sipxPublisherUpdate(mhPublisherHandle,
                             DIALOG_EVENT_CONTENT_TYPE,
                             mDialogPDU.data(),
                             mDialogPDULength);

         // Remember the call ID
         mCalls.insertKeyAndValue(new UtlInt(pCallRef->getCallHandle()), new UtlString(callId));
         mLock.release();
         break;

      case ACDCall::CONNECTED:
         if (pInst->pCallManager->getSipDialog(callId, remoteAddress, sipDialog) != OS_SUCCESS) {
            OsSysLog::add(FAC_ACD, PRI_ERR, "ACDLine::publishCallState - Failed call to getSipDialog(%s, %s)\n",
                          callId.data(), remoteAddress.data());
            return OS_FAILED;
         }

         // Extract the local and remote identities and tags
         sipDialog.getLocalField(localUrl);
         localUrl.getFieldParameter("tag", localTag);

         sipDialog.getRemoteField(remoteUrl);
         remoteUrl.getFieldParameter("tag", remoteTag);

         localUrl.getIdentity(localIdentity);
         localUrl.getDisplayName(localDisplayName);

         remoteUrl.getIdentity(remoteIdentity);
         remoteUrl.getDisplayName(remoteDisplayName);

         sipDialog.getLocalContact(localTarget);
         sipDialog.getRemoteContact(remoteTarget);

         pDialog = mpDialogEventPackage->getDialog(callId, localTag, remoteTag);
         // Update the dialog content if it exists
         if (pDialog != NULL) {
            pDialog->setTags(localTag, remoteTag);
            pDialog->setLocalIdentity(localIdentity, localDisplayName);
            pDialog->setRemoteIdentity(remoteIdentity, remoteDisplayName);
            pDialog->setLocalTarget(localTarget.toString());
            pDialog->setRemoteTarget(remoteTarget.toString());
            pDialog->setState(STATE_CONFIRMED, NULL, NULL);
         }
         else {
            // Create a new dialog element
            sprintf(dialogId, "%ld", mDialogId);
            mLock.acquire();
            mDialogId++;

            // Remember the call ID
            mCalls.insertKeyAndValue(new UtlInt(pCallRef->getCallHandle()), new UtlString(callId));
            mLock.release();

            pDialog = new Dialog(dialogId, callId, localTag, remoteTag, "recipient");

            pDialog->setState(STATE_CONFIRMED, NULL, NULL);
            pDialog->setLocalIdentity(localIdentity, localDisplayName);
            pDialog->setRemoteIdentity(remoteIdentity, remoteDisplayName);
            pDialog->setLocalTarget(localTarget.toString());
            pDialog->setRemoteTarget(remoteTarget.toString());
            pDialog->setDuration(OsDateTime::getSecsSinceEpoch());
            mpDialogEventPackage->insertDialog(pDialog);
         }

         // Send the content to the subscribe server
         mLock.acquire();
         mpDialogEventPackage->buildBodyGetBytes(&mDialogPDU, &mDialogPDULength);
         sipxPublisherUpdate(mhPublisherHandle,
                             DIALOG_EVENT_CONTENT_TYPE,
                             mDialogPDU.data(),
                             mDialogPDULength);
         mLock.release();
         break;

      case ACDCall::DISCONNECTED:
         {
            if (pInst->pCallManager->getSipDialog(callId, remoteAddress, sipDialog) != OS_SUCCESS) {
               OsSysLog::add(FAC_ACD, PRI_ERR, "ACDLine::publishCallState - Failed call to getSipDialog(%s, %s)\n",
                             callId.data(), remoteAddress.data());
               return OS_FAILED;
            }

            // Extract the local and remote tags
            sipDialog.getLocalField(localUrl);
            localUrl.getFieldParameter("tag", localTag);

            sipDialog.getRemoteField(remoteUrl);
            remoteUrl.getFieldParameter("tag", remoteTag);

            // Remove the call from the pool and clean up the call
            UtlInt thisCall = UtlInt(pCallRef->getCallHandle());

            pDialog = mpDialogEventPackage->getDialog(callId, localTag, remoteTag);
            if (pDialog != NULL) {
               pDialog->setState(STATE_TERMINATED, NULL, NULL);

               mLock.acquire();
               // Publish the content to the subscribe server
               mpDialogEventPackage->buildBodyGetBytes(&mDialogPDU, &mDialogPDULength);
               sipxPublisherUpdate(mhPublisherHandle,
                                   DIALOG_EVENT_CONTENT_TYPE,
                                   mDialogPDU.data(),
                                   mDialogPDULength);
               mLock.release();

               // Remove the dialog from the dialog event package
               pDialog = mpDialogEventPackage->removeDialog(pDialog);
               delete pDialog;

               mCalls.destroy(&thisCall);
            }
         }
         break;

      case ACDCall::OFFERING:
      case ACDCall::DESTROYED:
      case ACDCall::CALL_STATE_UNDEFINED:
         break;
   }

   return OS_SUCCESS;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDLine::setAttributes
//
//  SYNOPSIS:    void setAttributes(
//                 ProvisioningAttrList& rRequestAttributes) ProvisioningAgent attribute list
//                                                           containing one or more ACDLine
//                                                           attributes to be updated.
//
//  DESCRIPTION: Used by the ACDLineManager::set() function to update on or more attributes
//               in response to recieving a provisioning set request.
//
//  RETURNS:     None.
//
//  ERRORS:      If an error is encountered, an exception will be thrown, with the description
//               of the error being specified in "UtlString error".
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDLine::setAttributes(ProvisioningAttrList& rRequestAttributes)
{
   // Set the individual attributes
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDLine::setAttributes(%s)",
                 mUriString.data());

   // name
  	rRequestAttributes.getAttribute(LINE_NAME_TAG, mName);

   // extension
  	rRequestAttributes.getAttribute(LINE_EXTENSION_TAG, mExtension);

   // trunk-mode
  	rRequestAttributes.getAttribute(LINE_TRUNK_MODE_TAG, mTrunkMode);

   // publish-line-presence
  	rRequestAttributes.getAttribute(LINE_PUBLISH_PRESENCE_TAG, mPublishLinePresence);

   // acd-queue
  	rRequestAttributes.getAttribute(LINE_ACD_QUEUE_TAG, mAcdQueue);
}

/* ============================ ACCESSORS ================================= */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDLine::getLineHandle
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

SIPX_LINE ACDLine::getLineHandle(void)
{
   return mhLineHandle;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDLine::getUriString
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

UtlString* ACDLine::getUriString(void)
{
   return &mUriString;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDLine::hash
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

unsigned ACDLine::hash() const
{
   return mUriString.hash();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDLine::getContainableType
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

UtlContainableType ACDLine::getContainableType() const
{
   return ACDLine::TYPE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDLine::getAttributes
//
//  SYNOPSIS:    bool getAttributes(
//                 ProvisioningAttrList&  rRequestAttributes, ProvisioningAgent attribute list
//                                                            containing one or more ACDLine
//                                                            attributes to be retrieved.
//                 ProvisioningAttrList*& prResponse)         ProvisioningAgent attribute list
//                                                            containing the requested attributes
//                                                            and their corresponding values.
//
//  DESCRIPTION: Used by the ACDLineManager::get() function to request the value of one or more
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

void ACDLine::getAttributes(ProvisioningAttrList& rRequestAttributes, ProvisioningAttrList*& prResponse)
{
   // See if there are any specific attributes listed in the request
   if (rRequestAttributes.attributePresent(LINE_NAME_TAG) ||
       rRequestAttributes.attributePresent(LINE_EXTENSION_TAG) ||
       rRequestAttributes.attributePresent(LINE_TRUNK_MODE_TAG) ||
       rRequestAttributes.attributePresent(LINE_PUBLISH_PRESENCE_TAG) ||
       rRequestAttributes.attributePresent(LINE_ACD_QUEUE_TAG)) {
      // At least one attribute has been requested, go through list and retrieve
      // name
      if (rRequestAttributes.attributePresent(LINE_NAME_TAG)) {
         prResponse->setAttribute(LINE_NAME_TAG, mName);
      }

      // extension
      if (rRequestAttributes.attributePresent(LINE_EXTENSION_TAG)) {
         prResponse->setAttribute(LINE_EXTENSION_TAG, mExtension);
      }

      // trunk-mode
      if (rRequestAttributes.attributePresent(LINE_TRUNK_MODE_TAG)) {
         prResponse->setAttribute(LINE_TRUNK_MODE_TAG, mTrunkMode);
      }

      // publish-line-presence
      if (rRequestAttributes.attributePresent(LINE_PUBLISH_PRESENCE_TAG)) {
         prResponse->setAttribute(LINE_PUBLISH_PRESENCE_TAG, mPublishLinePresence);
      }

      // acd-queue
      if (rRequestAttributes.attributePresent(LINE_ACD_QUEUE_TAG)) {
         prResponse->setAttribute(LINE_ACD_QUEUE_TAG, mAcdQueue);
      }

   }
   else {
      // No specific attributes were requested, send them all back
      // name
      prResponse->setAttribute(LINE_NAME_TAG, mName);

      // extension
      prResponse->setAttribute(LINE_EXTENSION_TAG, mExtension);

      // trunk-mode
      prResponse->setAttribute(LINE_TRUNK_MODE_TAG, mTrunkMode);

      // publish-line-presence
      prResponse->setAttribute(LINE_PUBLISH_PRESENCE_TAG, mPublishLinePresence);

      // acd-queue
      prResponse->setAttribute(LINE_ACD_QUEUE_TAG, mAcdQueue);
   }
}

/* ============================ INQUIRY =================================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDLine::isLineBusy
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

bool ACDLine::isLineBusy(void)
{
   return mLineBusy;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDLine::compareTo
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

int ACDLine::compareTo(UtlContainable const * pInVal) const
{
   int result ;

   if (pInVal->isInstanceOf(ACDLine::TYPE)) {
      result = mUriString.compareTo(((ACDLine*)pInVal)->getUriString());
   }
   else {
      result = -1;
   }

   return result;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
