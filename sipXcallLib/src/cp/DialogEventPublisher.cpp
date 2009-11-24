//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <cp/DialogEventPublisher.h>
#include <tao/TaoMessage.h>
#include <tao/TaoString.h>
#include <cp/CallManager.h>
#include <net/SipDialog.h>
#include <net/SipDialogEvent.h>
#include <net/SipPublishContentMgr.h>
#include <os/OsFS.h>
#include <os/OsDateTime.h>
#include <utl/XmlContent.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define TAO_OFFER_PARAM_CALLID             0
#define TAO_OFFER_PARAM_ADDRESS            2
#define TAO_OFFER_PARAM_LOCAL_CONNECTION   6
#define STATE "full"

//#define DEBUGGING 1

// STATIC VARIABLE INITIALIZATIONS

// Objects to construct default content for dialog events.

class DialogDefaultConstructor : public SipPublishContentMgrDefaultConstructor
{
  public:

   /** Generate the content for a resource and event.
    */
   virtual void generateDefaultContent(SipPublishContentMgr* contentMgr,
                                       const char* resourceId,
                                       const char* eventTypeKey,
                                       const char* eventType);

   /// Make a copy of this object according to its real type.
   virtual SipPublishContentMgrDefaultConstructor* copy();

   // Service routine for UtlContainable.
   virtual const char* const getContainableType() const;

protected:
   static UtlContainableType TYPE;    /** < Class type used for runtime checking */
};

// Static identifier for the type.
const UtlContainableType DialogDefaultConstructor::TYPE = "DialogDefaultConstructor";

// Generate the default content for dialog status.
void DialogDefaultConstructor::generateDefaultContent(SipPublishContentMgr* contentMgr,
                                                      const char* resourceId,
                                                      const char* eventTypeKey,
                                                      const char* eventType)
{
   // Construct the body, an empty notice for the user.
   UtlString content;
   content.append("<?xml version=\"1.0\"?>\r\n"
                  "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" "
                  "version=\"&version;\" state=\"full\" entity=\"");
   XmlEscape(content, resourceId);
   content.append("\">\r\n"
                  "</dialog-info>\r\n");

   // Build an HttpBody.
   HttpBody* body = new HttpBody(content, strlen(content),
                                 DIALOG_EVENT_CONTENT_TYPE);

   // Install it for the resource, but do not publish it, because our
   // caller will publish it.
   contentMgr->publish(resourceId, eventTypeKey, eventType, 1, &body, TRUE);
}

// Make a copy of this object according to its real type.
SipPublishContentMgrDefaultConstructor* DialogDefaultConstructor::copy()
{
   // Copying these objects is easy, since they have no member variables, etc.
   return new DialogDefaultConstructor;
}

// Get the ContainableType for a UtlContainable derived class.
UtlContainableType DialogDefaultConstructor::getContainableType() const
{
    return DialogDefaultConstructor::TYPE;
}


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
DialogEventPublisher::DialogEventPublisher(CallManager* callManager,
                                           SipPublishContentMgr* contentMgr,
                                           const UtlString& entityHost,
                                           int entityPort,
                                           bool maskEstablished) :
   mpCallManager(callManager),
   mpSipPublishContentMgr(contentMgr),
   mDialogId(0),
   mEntityHost(entityHost),
   mEntityPort(entityPort),
   mMaskEstablishedasEarly(maskEstablished)
{
   // Arrange to generate default content for dialog events.
   mpSipPublishContentMgr->publishDefault(DIALOG_EVENT_TYPE, DIALOG_EVENT_TYPE,
                                          new DialogDefaultConstructor);
   // Determine the host/port parts if none are given.
   if (mEntityHost.isNull())
   {
      mpCallManager->getUserAgent()->getLocalAddress(&mEntityHost, &mEntityPort);
   }
}

//Destructor
DialogEventPublisher::~DialogEventPublisher()
{
   mProvisionalResponses.destroyAll();
}


/* ============================ MANIPULATORS ============================== */

UtlBoolean DialogEventPublisher::handleMessage(OsMsg& rMsg)
{
   SipDialog sipDialog;
   UtlString sipDialogContent;
   Url requestUrl;
   UtlString entity;
   UtlString* pEntity;
   char dialogId[10];
   SipDialogEvent* pThisCall;
   Dialog* pDialog;
   UtlString localTag, remoteTag;
   Url localIdentity, remoteIdentity;
   Url localTarget, remoteTarget;
   UtlString identity, displayName;
   UtlString failCallId;
   OsTime receivedTime;
   UtlString remoteRequestUri;
   UtlString temp;

   UtlString dialogEvent;
   Url tmpLocalContact;

   // React to telephony events
   if(rMsg.getMsgSubType()== TaoMessage::EVENT)
   {
      TaoMessage* taoMessage = (TaoMessage*)&rMsg;

      TaoEventId taoEventId = taoMessage->getTaoObjHandle();
      UtlString argList(taoMessage->getArgList());
      TaoString arg(argList, TAOMESSAGE_DELIMITER);

      UtlBoolean localConnection = atoi(arg[TAO_OFFER_PARAM_LOCAL_CONNECTION]);
      UtlString  callId = arg[TAO_OFFER_PARAM_CALLID] ;
      UtlString  address = arg[TAO_OFFER_PARAM_ADDRESS] ;
      OsSysLog::add(FAC_PARK, PRI_DEBUG, "DialogEventPublisher::handleMessage TaoMessage type %d, subtype %d, Tao event %d, args %d, localConnection %d, callId '%s', address '%s'",
                    rMsg.getMsgType(), rMsg.getMsgSubType(),
                    (int)taoEventId, arg.getCnt(),
                    localConnection, callId.data(), address.data());
#ifdef DEBUGGING
      dumpTaoMessageArgs(taoEventId, arg) ;
#endif

      switch (taoEventId)
      {
         case PtEvent::CONNECTION_INITIATED:
         {
            if (!localConnection)
            {
               OsSysLog::add(FAC_SIP, PRI_DEBUG,
                     "DialogEventPublisher::handleMessage CONNECTION_INITIATED");

               if (mpCallManager->getSipDialog(callId, address, sipDialog)
                     != OS_SUCCESS)
               {
                  OsSysLog::add(
                        FAC_ACD,
                        PRI_ERR,
                        "DialogEventPublisher::handleMessage - CONNECTION_INITIATED - Failed call to getSipDialog(%s, %s)",
                              callId.data(), address.data());
                  // Give up, since we can't get any information about this call.
                  break;
               }
#ifdef DEBUGGING
               sipDialog.toString(sipDialogContent);
               OsSysLog::add(
                     FAC_SIP,
                     PRI_DEBUG,
                     "DialogEventPublisher::handleMessage sipDialog = '%s'",
                     sipDialogContent.data());
#endif

               // This is an outgoing Call - get the entity identity from local contact

               sipDialog.getLocalContact(tmpLocalContact);
               getEntity(tmpLocalContact.toString(), entity);

               if (entity.isNull())
               {
                  OsSysLog::add(
                        FAC_SIP,
                        PRI_WARNING,
                        "DialogEventPublisher::handleMessage Call arrived: callId '%s' address '%s' without localContact",
                               callId.data(), address.data());
                  break;
               }
               else
               {
                  requestUrl = Url(entity);
                  requestUrl.getIdentity(entity);
                  // Make entity into a URI by prepending "sip:".
                  entity.prepend("sip:");
               }

               // Create a dialog event if has not been created yet
               pThisCall = dynamic_cast<SipDialogEvent *> (mCalls.findValue(&entity));
               if (pThisCall == NULL)
               {
                  pEntity = new UtlString(entity);
                  pThisCall = new SipDialogEvent(STATE, entity);
                  mCalls.insertKeyAndValue(pEntity, pThisCall);
                  OsSysLog::add(
                      FAC_SIP,
                      PRI_DEBUG,
                      "DialogEventPublisher::handleMessage insert DialogEvent object %p to the list",
                           pThisCall);
               }

               // Create the dialog element
               sipDialog.getLocalField(localIdentity);
               localIdentity.getFieldParameter("tag", localTag);

               sipDialog.getRemoteField(remoteIdentity);
               remoteIdentity.getFieldParameter("tag", remoteTag);

               sprintf(dialogId, "%ld", mDialogId);
               mDialogId++;

               pDialog = new Dialog(dialogId, callId, localTag, remoteTag, "initiator");

               pDialog->setState(STATE_TRYING, NULL, NULL);

               localIdentity.getIdentity(identity);
               localIdentity.getDisplayName(displayName);
               pDialog->setLocalIdentity(identity, displayName);

               remoteIdentity.getIdentity(identity);
               remoteIdentity.getDisplayName(displayName);
               pDialog->setRemoteIdentity(identity, displayName);

               sipDialog.getLocalContact(localTarget);
               localTarget.getUri(temp);
               pDialog->setLocalTarget(temp);

               sipDialog.getRemoteContact(remoteTarget);
               remoteTarget.getUri(temp);
               pDialog->setRemoteTarget(temp);

               pDialog->setDuration(OsDateTime::getSecsSinceEpoch());

               pThisCall->insertDialog(pDialog);

               // Insert it into the active call list
               pThisCall->buildBody();

               // Send the content to the subscribe server.
               {
                  // Make a copy, because mpSipPublishContentMgr will own it.
                  HttpBody* pHttpBody =
                          new HttpBody(*(HttpBody*) pThisCall);
                  mpSipPublishContentMgr->publish(entity.data(),
                          DIALOG_EVENT_TYPE, DIALOG_EVENT_TYPE, 1,
                          &pHttpBody);

               }
            }
            break;
         }

         case PtEvent::CONNECTION_ALERTING:
         {
            if(!localConnection)
            {
               OsSysLog::add(FAC_SIP, PRI_DEBUG,
                     "DialogEventPublisher::handleMessage CONNECTION_ALERTING");

               if (mpCallManager->getSipDialog(callId, address, sipDialog)
                     != OS_SUCCESS)
               {
                  OsSysLog::add(
                        FAC_ACD,
                        PRI_ERR,
                        "DialogEventPublisher::handleMessage - CONNECTION_ALERTING - Failed call to getSipDialog(%s, %s)",
                              callId.data(), address.data());
                  // Give up, since we can't get any information about this call.
                   break;
                }

#ifdef DEBUGGING
                sipDialog.toString(sipDialogContent);
                OsSysLog::add(FAC_SIP, PRI_DEBUG,
                      "DialogEventPublisher::handleMessage sipDialog = '%s'",
                            sipDialogContent.data());
#endif

                // This is an outgoing Call - get the entity identity from local contact

                sipDialog.getLocalContact(tmpLocalContact);
                getEntity(tmpLocalContact.toString(), entity);

                OsSysLog::add(
                      FAC_SIP,
                      PRI_DEBUG,
                      "DialogEventPublisher::handleMessage Call arrived: callId '%s' address '%s' localContact '%s'",
                            callId.data(), address.data(), entity.data());


                if (entity.isNull())
                {
                   OsSysLog::add(
                         FAC_SIP,
                         PRI_WARNING,
                         "DialogEventPublisher::handleMessage Call arrived: callId '%s' address '%s' without localContact",
                               callId.data(), address.data());
                   break;
                }
                else
                {
                   requestUrl = Url(entity);
                   requestUrl.getIdentity(entity);
                   // Make entity into a URI by prepending "sip:".
                   entity.prepend("sip:");
                }

                // Create a dialog event if has not been created yet
                pThisCall = dynamic_cast<SipDialogEvent *> (mCalls.findValue(&entity));
                if (pThisCall == NULL)
                {
                   pEntity = new UtlString(entity);
                   pThisCall = new SipDialogEvent(STATE, entity);
                   mCalls.insertKeyAndValue(pEntity, pThisCall);
                   OsSysLog::add(
                         FAC_SIP,
                         PRI_DEBUG,
                         "DialogEventPublisher::handleMessage insert DialogEvent object %p to the list",
                               pThisCall);
                }

                // Create the dialog element
                sipDialog.getLocalField(localIdentity);
                localIdentity.getFieldParameter("tag", localTag);

                sipDialog.getRemoteField(remoteIdentity);
                remoteIdentity.getFieldParameter("tag", remoteTag);

                /*
                 * Remote Tags are not usually stored with the remote field as
                 * the call may have been forked resulting in multiple
                 * 180 responses. Hence look up the tag from the provisionalToTag
                 * hash map in SipSession.
                 */

                UtlString provisionalRemoteContact("");

                if(remoteTag.isNull())
                {

                   SipSession sipSession;
                   mpCallManager->getSession(callId, address, sipSession);

                   UtlHashMap provisionalToTags;
                   sipSession.getProvisionalToTags(provisionalToTags);

                   UtlHashMapIterator toTagMapIterator(provisionalToTags);
                   toTagMapIterator.reset();

                   while(NULL != toTagMapIterator())
                   {
                      UtlString key (*(dynamic_cast<UtlString *> (toTagMapIterator.key())));
                      UtlString value (*(dynamic_cast<UtlString *> (toTagMapIterator.value())));

                      if(!mProvisionalResponses.contains(&key))
                      {
                          // This is a new TO TAG for a response we have not sent NOTIFY for

                         UtlString * tmpKey = new UtlString(key);
                         UtlString * tmpValue = new UtlString(callId);

                         mProvisionalResponses.insertKeyAndValue(tmpKey, tmpValue);
                         remoteTag = key;

                         Url temp(value);
                         temp.getUri(provisionalRemoteContact);

                         break;
                      }

                   }

                   /*
                    * SipSession::getProvisionalToTags() passes everything by
                    * copy, so explicitly delete the hash map here
                    */
                   provisionalToTags.destroyAll();
                }

                pDialog = pThisCall->getDialog(callId, localTag, remoteTag);
                // Update the dialog content if it exists.
                if (pDialog)
                {
                   pDialog->setTags(localTag, remoteTag);

                   sipDialog.getLocalContact(localTarget);
                   localTarget.getUri(temp);
                   pDialog->setLocalTarget(temp);

                   if(provisionalRemoteContact.isNull())
                   {
                      sipDialog.getRemoteContact(remoteTarget);
                      remoteTarget.getUri(temp);
                      pDialog->setRemoteTarget(temp);
                   }
                   else
                   {
                       /*
                        * Get the Remote Target URI from the value we found from the
                        * provisionalToTags hash map. This would guarantee that we
                        * are matching the To Tag with the correct Remote Contact
                        */

                       pDialog->setRemoteTarget(provisionalRemoteContact);
                   }

                   // 1XX Response without a To Tag is considered a "proceeding" state
                   if(remoteTag.isNull())
                   {
                       pDialog->setState(STATE_PROCEEDING, NULL, NULL);
                   }
                   else
                   {
                       pDialog->setState(STATE_EARLY, NULL, NULL);
                   }
               }
               else
               {
                   sprintf(dialogId, "%ld", mDialogId);
                   mDialogId++;

                   pDialog = new Dialog(dialogId, callId, localTag, remoteTag, "initiator");

                   // 1XX Response without a To Tag is considered a "proceeding" state
                   if(remoteTag.isNull())
                   {
                       pDialog->setState(STATE_PROCEEDING, NULL, NULL);
                   }
                   else
                   {
                       pDialog->setState(STATE_EARLY, NULL, NULL);
                   }

                   localIdentity.getIdentity(identity);
                   localIdentity.getDisplayName(displayName);
                   pDialog->setLocalIdentity(identity, displayName);

                   remoteIdentity.getIdentity(identity);
                   remoteIdentity.getDisplayName(displayName);
                   pDialog->setRemoteIdentity(identity, displayName);

                   sipDialog.getLocalContact(localTarget);
                   localTarget.getUri(temp);
                   pDialog->setLocalTarget(temp);

                   if(provisionalRemoteContact.isNull())
                   {
                      sipDialog.getRemoteContact(remoteTarget);
                      remoteTarget.getUri(temp);
                      pDialog->setRemoteTarget(temp);
                   }
                   else
                   {
                      /*
                       * Get the Remote Target URI from the value we found from the
                       * provisionalToTags hash map. This would guarantee that we
                       * are matching the To Tag with the correct Remote Contact
                       */

                      pDialog->setRemoteTarget(provisionalRemoteContact);
                   }

                   pDialog->setDuration(OsDateTime::getSecsSinceEpoch());

                   pThisCall->insertDialog(pDialog);
               }

               // Insert it into the active call list
               pThisCall->buildBody();

               // Send the content to the subscribe server.
               {
                  // Make a copy, because mpSipPublishContentMgr will own it.
                  HttpBody* pHttpBody = new HttpBody(*(HttpBody*) pThisCall);
                  mpSipPublishContentMgr->publish(entity.data(),
                                                  DIALOG_EVENT_TYPE, DIALOG_EVENT_TYPE, 1,
                                                  &pHttpBody);
               }
            }
            break;
         }
         case PtEvent::CONNECTION_OFFERED:
            if(!localConnection)
            {
               OsSysLog::add(FAC_SIP, PRI_DEBUG, "DialogEventPublisher::handleMessage CONNECTION_OFFERED");
               if (mpCallManager->getSipDialog(callId, address, sipDialog) !=
                   OS_SUCCESS)
               {
                  OsSysLog::add(FAC_ACD, PRI_ERR,
                                "DialogEventPublisher::handleMessage - CONNECTION_OFFERED - Failed call to getSipDialog(%s, %s)",
                                callId.data(), address.data());
                  // Give up, since we can't get any information about this call.
                  break;
               }
#ifdef DEBUGGING
               sipDialog.toString(sipDialogContent);
               OsSysLog::add(FAC_SIP, PRI_DEBUG, "DialogEventPublisher::handleMessage sipDialog = '%s'",
                             sipDialogContent.data());
#endif

               sipDialog.getRemoteRequestUri(remoteRequestUri);
               getEntity(remoteRequestUri, entity);

               OsSysLog::add(FAC_SIP, PRI_DEBUG, "DialogEventPublisher::handleMessage Call arrived: callId '%s' address '%s' requestUri '%s'",
                             callId.data(), address.data(), entity.data());

               if (entity.isNull())
               {
                  OsSysLog::add(FAC_SIP, PRI_WARNING, "DialogEventPublisher::handleMessage Call arrived: callId '%s' address '%s' without requestUrl",
                                callId.data(), address.data());
                  break;
               }
               else
               {
                  requestUrl = Url(entity);
                  requestUrl.getIdentity(entity);
                  // Make entity into a URI by prepending "sip:".
                  entity.prepend("sip:");
               }

               // Create a dialog event if has not been created yet
               pThisCall = dynamic_cast<SipDialogEvent *> (mCalls.findValue(&entity));
               if (pThisCall == NULL)
               {
                  pEntity = new UtlString(entity);
                  pThisCall = new SipDialogEvent(STATE, entity);
                  mCalls.insertKeyAndValue(pEntity, pThisCall);
                  OsSysLog::add(FAC_SIP, PRI_DEBUG, "DialogEventPublisher::handleMessage insert DialogEvent object %p to the list",
                                pThisCall);
               }

               // Create the dialog element
               sipDialog.getLocalField(localIdentity);
               localIdentity.getFieldParameter("tag", localTag);

               sipDialog.getRemoteField(remoteIdentity);
               remoteIdentity.getFieldParameter("tag", remoteTag);

               sprintf(dialogId, "%ld", mDialogId);
               mDialogId++;

               pDialog = new Dialog(dialogId, callId, localTag, remoteTag, "recipient");
               pDialog->setState(STATE_EARLY, NULL, NULL);

               localIdentity.getIdentity(identity);
               localIdentity.getDisplayName(displayName);
               pDialog->setLocalIdentity(identity, displayName);

               remoteIdentity.getIdentity(identity);
               remoteIdentity.getDisplayName(displayName);
               pDialog->setRemoteIdentity(identity, displayName);

               sipDialog.getLocalContact(localTarget);
               localTarget.getUri(temp);
               pDialog->setLocalTarget(temp);

               sipDialog.getRemoteContact(remoteTarget);
               remoteTarget.getUri(temp);
               pDialog->setRemoteTarget(temp);

               pDialog->setDuration(OsDateTime::getSecsSinceEpoch());

               pThisCall->insertDialog(pDialog);

               // Insert it into the active call list
               pThisCall->buildBody();

               // Send the content to the subscribe server.
               {
                  // Make a copy, because mpSipPublishContentMgr will own it.
                  HttpBody* pHttpBody = new HttpBody(*(HttpBody*)pThisCall);
                  mpSipPublishContentMgr->publish(entity.data(),
                                                  DIALOG_EVENT_TYPE,
                                                  DIALOG_EVENT_TYPE,
                                                  1, &pHttpBody);
               }
            }
            break;

         case PtEvent::CONNECTION_ESTABLISHED:
         {
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "DialogEventPublisher::handleMessage "
                          "CONNECTION_ESTABLISHED");

            // Set default as an incoming call
            bool incomingCall = TRUE;

            if (mpCallManager->getSipDialog(callId, address, sipDialog) !=
             OS_SUCCESS)
            {
               OsSysLog::add(FAC_ACD, PRI_ERR,
                             "DialogEventPublisher::handleMessage "
                             "- CONNECTION_ESTABLISHED - Failed call to getSipDialog(%s, %s)",
                             callId.data(), address.data());
               // Give up, since we can't get any information about this call.
               break;
            }
#ifdef DEBUGGING
            sipDialog.toString(sipDialogContent);
            OsSysLog::add(FAC_SIP, PRI_DEBUG, "DialogEventPublisher::handleMessage sipDialog = %s",
                          sipDialogContent.data());
#endif
            sipDialog.getRemoteRequestUri(remoteRequestUri);
            getEntity(remoteRequestUri, entity);

            OsSysLog::add(FAC_SIP, PRI_DEBUG, "DialogEventPublisher::handleMessage Call connected: callId '%s' address '%s' with requestUrl '%s'",
                          callId.data(), address.data(), entity.data());

            if (entity.isNull())
            {
               if (localConnection)
               {
                  OsSysLog::add(FAC_SIP, PRI_WARNING,
                                "DialogEventPublisher::handleMessage Call arrived: callId '%s' address '%s' without requestUrl",
                                callId.data(), address.data());
                  break;
               }
               else
               {
                  // This is an outgoing call - get the entity identity from local contact

                  incomingCall = FALSE;
                  Url tmpUrlForRequestUri;
                  sipDialog.getLocalContact(tmpUrlForRequestUri);
                  remoteRequestUri = tmpUrlForRequestUri.toString();
                  getEntity(remoteRequestUri, entity);

               }
            }
            else
            {
               if (!localConnection)
               {
                  OsSysLog::add(FAC_SIP, PRI_DEBUG,
                                "DialogEventPublisher::handleMessage No local connection");
                  break;
               }
            }

            if (entity.isNull())
            {
               OsSysLog::add(FAC_SIP, PRI_WARNING,
                             "DialogEventPublisher::handleMessage Call connected: callId '%s' address '%s' without requestUrl",
                             callId.data(), address.data());
               break;
            }
            else
            {
               requestUrl = Url(entity);
               requestUrl.getIdentity(entity);
               // Make entity into a URI by prepending "sip:".
               entity.prepend("sip:");
            }
            pThisCall = dynamic_cast<SipDialogEvent *> (mCalls.findValue(&entity));
            if (pThisCall == NULL)
            {
               pEntity = new UtlString(entity);
               pThisCall = new SipDialogEvent(STATE, entity);

               // Insert it into the active call list
               mCalls.insertKeyAndValue(pEntity, pThisCall);
               OsSysLog::add(FAC_SIP, PRI_DEBUG,
                             "DialogEventPublisher::handleMessage inserting entity '%s'",
                             entity.data());
            }

            // Get the new callId because it might have changed
            sipDialog.getCallId(callId);
            sipDialog.getLocalField(localIdentity);
            localIdentity.getFieldParameter("tag", localTag);
            sipDialog.getRemoteField(remoteIdentity);
            remoteIdentity.getFieldParameter("tag", remoteTag);

            pDialog = pThisCall->getDialog(callId, localTag, remoteTag);

            if (!incomingCall)
            {
                terminateProvisionalResponses(callId, localTag, pThisCall);
            }

            // Update the dialog content if it exists.
            if (pDialog)
            {
               // This may be the establishment of a dialog for an
               // INVITE we sent, so the remote tag may only be getting
               // set now.

               pDialog->setTags(localTag, remoteTag);

               sipDialog.getLocalContact(localTarget);
               localTarget.getUri(temp);
               pDialog->setLocalTarget(temp);

               sipDialog.getRemoteContact(remoteTarget);
               remoteTarget.getUri(temp);
               pDialog->setRemoteTarget(temp);

               if (!mMaskEstablishedasEarly)
               {
                  pDialog->setState(STATE_CONFIRMED, NULL, NULL);
               }
               else
               {
                  // special case for park orbits so that call in orbit is always flashing.  Used for one button pickup.
                  OsSysLog::add(FAC_SIP, PRI_DEBUG,
                             "DialogEventPublisher::handleMessage setting state to early when connected");
                  pDialog->setState(STATE_EARLY, NULL, NULL);
               }
            }
            else
            {
               // Create a new dialog element
               sprintf(dialogId, "%ld", mDialogId);
               mDialogId++;

               if (incomingCall)
               {
                  pDialog = new Dialog(dialogId, callId, localTag, remoteTag, "recipient");
               }
               else
               {
                  pDialog = new Dialog(dialogId, callId, localTag, remoteTag, "initiator");
               }

               if (!mMaskEstablishedasEarly)
               {
                  pDialog->setState(STATE_CONFIRMED, NULL, NULL);
               }
               else
               {
                  // special case for park orbits so that call in orbit is always flashing.  Used for one button pickup.
                  OsSysLog::add(FAC_SIP, PRI_DEBUG,
                             "DialogEventPublisher::handleMessage setting state to early when connected");
                  pDialog->setState(STATE_EARLY, NULL, NULL);
               }

               localIdentity.getIdentity(identity);
               localIdentity.getDisplayName(displayName);
               pDialog->setLocalIdentity(identity, displayName);

               remoteIdentity.getIdentity(identity);
               remoteIdentity.getDisplayName(displayName);
               pDialog->setRemoteIdentity(identity, displayName);

               sipDialog.getLocalContact(localTarget);
               localTarget.getUri(temp);
               pDialog->setLocalTarget(temp);

               sipDialog.getRemoteContact(remoteTarget);
               remoteTarget.getUri(temp);
               pDialog->setRemoteTarget(temp);

               pDialog->setDuration(OsDateTime::getSecsSinceEpoch());

               pThisCall->insertDialog(pDialog);
            }

            pThisCall->buildBody();

            // Publish the content to the subscribe server.
            // Make a copy, because mpSipPublishContentMgr will own it.
            HttpBody* pHttpBody = new HttpBody(*(HttpBody*)pThisCall);
            mpSipPublishContentMgr->publish(entity.data(),
                                            DIALOG_EVENT_TYPE, DIALOG_EVENT_TYPE,
                                            1, &pHttpBody);

            if (!incomingCall)
            {
                deleteProvisionalResponses(callId, localTag, pThisCall);
            }

            break;
         }

         case PtEvent::CONNECTION_DISCONNECTED:
         case PtEvent::CONNECTION_FAILED:
         {
            OsSysLog::add(FAC_SIP, PRI_DEBUG, "DialogEventPublisher::handleMessage CONNECTION_DISCONNECTED/CONNECTION_FAILED");
            if (!localConnection)
            {
               if (mpCallManager->getSipDialog(callId, address, sipDialog) !=
                   OS_SUCCESS)
               {
                  OsSysLog::add(FAC_ACD, PRI_ERR,
                                "DialogEventPublisher::handleMessage - CONNECTION_DISCONNECTED - Failed call to getSipDialog(%s, %s)",
                                callId.data(), address.data());
                  // Fill sipDialog with empty information as if we had gotten an empty SipDialog - then go on
                  // and try to remove the event by just knowing the callId that we got passed in.
                  UtlString emptyUri("");
                  sipDialog.setRemoteRequestUri(emptyUri);
               }
#ifdef DEBUGGING
               sipDialog.toString(sipDialogContent);
               OsSysLog::add(FAC_SIP, PRI_DEBUG, "DialogEventPublisher::handleMessage sipDialog = %s",
                             sipDialogContent.data());
#endif
               sipDialog.getRemoteRequestUri(remoteRequestUri);
               getEntity(remoteRequestUri, entity);

               OsSysLog::add(FAC_SIP, PRI_DEBUG, "DialogEventPublisher::handleMessage Call dropped: '%s' address '%s' with entity '%s'",
                             callId.data(), address.data(), entity.data());

               if (entity.isNull())
               {
                  // Under some circumstances (example failed transfer) the getSipDialog call above will return an empty SipDialog
                  // with an empty requestUrl and callId. We need to remember th callId that was passed in as a Tao message parameter
                  // and try to associate the callId with an entity.
                  OsSysLog::add(FAC_SIP, PRI_WARNING, "DialogEventPublisher::handleMessage Call dropped: callId '%s' address '%s' without requestUrl",
                                callId.data(), address.data());
                  // We have no request Url - try to get entity from callId
                  failCallId = callId;
                  if (!findEntryByCallId(callId, entity))
                  {
                     break;
                  }
               }

               requestUrl = Url(entity);
               requestUrl.getIdentity(entity);
               // Make identity into a URI by prepending "sip:".
               entity.prepend("sip:");

               // Get the new callId because it might have changed
               sipDialog.getCallId(callId);

               sipDialog.getLocalField(localIdentity);
               localIdentity.getFieldParameter("tag", localTag);
               sipDialog.getRemoteField(remoteIdentity);
               remoteIdentity.getFieldParameter("tag", remoteTag);

               // If the SipDialog is empty we can't use the empty callId. Reassign the remembered callId.
               if (callId.isNull() && !failCallId.isNull())
               {
                  OsSysLog::add(FAC_SIP, PRI_WARNING, "DialogEventPublisher::handleMessage callId is empty, using fail callId '%s'",
                                failCallId.data());
                  callId = failCallId;
               }

               sipDialog.getLocalField(localIdentity);
               localIdentity.getFieldParameter("tag", localTag);
               sipDialog.getRemoteField(remoteIdentity);
               remoteIdentity.getFieldParameter("tag", remoteTag);

               // If the SipDialog is empty we can't use the empty callId. Reassign the remembered callId.
               if (callId.isNull() && !failCallId.isNull())
               {
                  OsSysLog::add(FAC_SIP, PRI_WARNING, "DialogEventPublisher::handleMessage callId is empty, using fail callId '%s'",
                                failCallId.data());
                  callId = failCallId;
               }

               // Remove the call from the pool and clean up the call
               OsSysLog::add(FAC_SIP, PRI_DEBUG, "Trying to find entity '%s'", entity.data());
               pThisCall = dynamic_cast<SipDialogEvent *> (mCalls.findValue(&entity));
               if (pThisCall)
               {
                  terminateProvisionalResponses(callId, localTag, pThisCall);
                  // Use the work-around for XCL-98 in case that we can't get tags from
                  // the SipDialog object
                  if (localTag.isNull())
                  {
                     pDialog = pThisCall->getDialogByCallId(callId);
                  }
                  else
                  {
                     pDialog = pThisCall->getDialog(callId, localTag, remoteTag);
                  }
                  if (pDialog)
                  {
                     pDialog->setState(STATE_TERMINATED, NULL, NULL);

                     pThisCall->buildBody();

                     // Publish the content to the subscribe server.
                     // Make a copy, because mpSipPublishContentMgr will own it.
                     HttpBody* pHttpBody = new HttpBody(*(HttpBody*)pThisCall);
                     mpSipPublishContentMgr->publish(entity.data(),
                                                     DIALOG_EVENT_TYPE, DIALOG_EVENT_TYPE,
                                                     1, &pHttpBody);

                     // Remove the dialog from the dialog event package
                     pDialog = pThisCall->removeDialog(pDialog);
                     delete pDialog;
                  }
               }
               else
               {
                  OsSysLog::add(FAC_SIP, PRI_ERR, "DialogEventPublisher::handleMessage Call dropped - no entity %s founded in the active call list",
                                entity.data());
               }

               // Delete All the Provisional Responses associated with this call
               deleteProvisionalResponses(callId, localTag, pThisCall);
            }

            break;
         }
      }
   }
   return(TRUE);
}


/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */


void DialogEventPublisher::dumpTaoMessageArgs(unsigned char eventId, TaoString& args)
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "===>Message type: %d args:\n", eventId) ;

   int argc = args.getCnt();
   for(int argIndex = 0; argIndex < argc; argIndex++)
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "\targ[%d]=\"%s\"", argIndex, args[argIndex]);
   }
}

void DialogEventPublisher::getEntity(const UtlString& requestUri,
                                     UtlString& entity)
{
   entity = "";

   UtlString userId;
   Url tempRequestUri(requestUri);
   tempRequestUri.getUserId(userId);

   if (!userId.isNull())
   {
      Url entityUrl(mEntityHost);
      entityUrl.setHostPort(mEntityPort);
      entityUrl.setUserId(userId.data());

      entityUrl.toString(entity);
   }
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "DialogEventPublisher::getEntity requestUri = '%s', userId = '%s', entity '%s'",
                 requestUri.data(), userId.data(), entity.data());
}

bool DialogEventPublisher::findEntryByCallId(UtlString& callId, UtlString& entity)
{
   bool bRet = false;
   UtlHashMapIterator iterator(mCalls);
   UtlString* pKey = NULL;

   while ((pKey = dynamic_cast<UtlString *>(iterator())))
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "DialogEventPublisher::findEntryByCallId callId '%s', key '%s'", callId.data(), pKey->data());

      SipDialogEvent* pEvent;
      pEvent = dynamic_cast<SipDialogEvent *>(iterator.value());

      Dialog* pDialog;

      if (pEvent)
      {
         pDialog = pEvent->getDialogByCallId(callId);
         OsSysLog::add(FAC_SIP, PRI_DEBUG, "DialogEventPublisher::findEntryByCallId After getDialog, result %p", pDialog);
         if (pDialog)
         {
            bRet = true;
            entity = pKey->data();
            break;
         }
      }
      else
      {
         OsSysLog::add(FAC_SIP, PRI_WARNING, "DialogEventPublisher::findEntryByCallId pEvent == NULL");
      }
   }
   return bRet;
}

void DialogEventPublisher::deleteProvisionalResponses(UtlString callId, UtlString localTag, SipDialogEvent* pThisCall)
{
    UtlHashMapIterator pResponsesIterator(mProvisionalResponses);
    pResponsesIterator.reset();
    Dialog * pDialog;

    while(NULL != pResponsesIterator())
    {
        if(callId == *( dynamic_cast<UtlString *> (pResponsesIterator.value())))
        {
            // Use the work-around for XCL-98 in case that we can't get tags from
            // the SipDialog object
            if (localTag.isNull())
            {
                pDialog = pThisCall->getDialogByCallId(callId);
            }
            else
            {
                UtlString * remoteTag = dynamic_cast<UtlString *> (pResponsesIterator.key());
                pDialog = pThisCall->getDialog(callId, localTag, *remoteTag);
            }

            // Remove the dialog from the dialog event package if the dialog has been
            // terminated

            if (pDialog)
            {
                UtlString tmpState;
                UtlString tmpEvent;
                UtlString tmpCode;

                pDialog->getState(tmpState, tmpEvent, tmpCode);

                if(tmpState.compareTo(STATE_TERMINATED) == 0)
                {
                    pDialog = pThisCall->removeDialog(pDialog);
                    delete pDialog;
                }
            }

            mProvisionalResponses.destroy(pResponsesIterator.key());
        }
    }
}

void DialogEventPublisher::terminateProvisionalResponses(UtlString callId, UtlString localTag, SipDialogEvent* pThisCall)
{
    UtlHashMapIterator pResponsesIterator(mProvisionalResponses);
    pResponsesIterator.reset();
    Dialog * pDialog;

    while(NULL != pResponsesIterator())
    {
        if(callId == *( dynamic_cast<UtlString *> (pResponsesIterator.value())))
        {
            // Use the work-around for XCL-98 in case that we can't get tags from
            // the SipDialog object
            if (localTag.isNull())
            {
                pDialog = pThisCall->getDialogByCallId(callId);
            }
            else
            {
                UtlString * remoteTag = dynamic_cast<UtlString *> (pResponsesIterator.key());
                pDialog = pThisCall->getDialog(callId, localTag, *remoteTag);
            }

            if (pDialog)
            {
                pDialog->setState(STATE_TERMINATED, NULL, NULL);
            }
        }
    }
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */
