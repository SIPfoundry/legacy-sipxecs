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
#include <os/OsFS.h>
#include <os/OsSysLog.h>
#include <utl/UtlSList.h>
#include <utl/UtlSListIterator.h>
#include <utl/UtlHashMap.h>
#include <utl/UtlHashMapIterator.h>
#include <utl/UtlHashBag.h>
#include <utl/UtlHashBagIterator.h>
#include <net/SipResourceList.h>
#include <net/NetMd5Codec.h>
#include <net/SipMessage.h>
#include <net/SipDialogMonitor.h>


// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
// STATIC VARIABLE INITIALIZATIONS

// Constructor
SipDialogMonitor::SipDialogMonitor(SipUserAgent* userAgent,
                                   UtlString& domainName,
                                   int hostPort,
                                   int refreshTimeout,
                                   bool toBePublished)
   : mLock(OsBSem::Q_PRIORITY, OsBSem::FULL)
{
   mpUserAgent = userAgent;
   mDomainName = domainName;

   UtlString localAddress;
   OsSocket::getHostIp(&localAddress);

   Url url(localAddress);
   url.setHostPort(hostPort);
   url.includeAngleBrackets();
   mContact = url.toString();
   mRefreshTimeout = refreshTimeout;
   mToBePublished = toBePublished;

   // Create the SIP Subscribe Client
   mpRefreshMgr = new SipRefreshManager(*mpUserAgent, mDialogManager); // Component for refreshing the subscription
   mpRefreshMgr->start();

   mpSipSubscribeClient = new SipSubscribeClient(*mpUserAgent, mDialogManager, *mpRefreshMgr);
   mpSipSubscribeClient->start();

   if (mToBePublished)
   {
      // Create the SIP Subscribe Server
      mpSubscriptionMgr = new SipSubscriptionMgr(); // Component for holding the subscription data

      mpSubscribeServer = new SipSubscribeServer(*mpUserAgent, mSipPublishContentMgr,
                                              *mpSubscriptionMgr, mPolicyHolder);
      mpSubscribeServer->enableEventType(DIALOG_EVENT_TYPE,
                                         NULL,
                                         NULL,
                                         NULL,
                                         NULL,
                                         SipSubscribeServer::standardVersionCallback);
      mpSubscribeServer->start();
   }
   else
   {
      mpSubscriptionMgr = NULL;
      mpSubscribeServer = NULL;
   }
}

// Destructor
SipDialogMonitor::~SipDialogMonitor()
{
   if (mpSipSubscribeClient)
   {
      mpSipSubscribeClient->endAllSubscriptions();
      delete mpSipSubscribeClient;
   }

   if (mpRefreshMgr)
   {
      delete mpRefreshMgr;
   }

   if (mpSubscriptionMgr)
   {
      delete mpSubscriptionMgr;
   }

   if (mpSubscribeServer)
   {
      delete mpSubscribeServer;
   }

   mMonitoredLists.destroyAll();

   mDialogEventList.destroyAll();

   mStateChangeNotifiers.destroyAll();
}


bool SipDialogMonitor::addExtension(UtlString& groupName, Url& contactUrl)
{
   bool result = false;
   mLock.acquire();

   // Check whether the group already exists. If not, create one.
   SipResourceList* list =
      dynamic_cast <SipResourceList *> (mMonitoredLists.findValue(&groupName));
   if (list == NULL)
   {
      UtlString* listName = new UtlString(groupName);
      list = new SipResourceList((UtlBoolean)TRUE, listName->data(),
                                 DIALOG_EVENT_TYPE);

      mMonitoredLists.insertKeyAndValue(listName, list);
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipDialogMonitor::addExtension insert listName %s and object %p to the resource list",
                    groupName.data(), list);
   }

   // Check whether the contact has already been added to the group.
   UtlString resourceId;
   contactUrl.getIdentity(resourceId);
   Resource* resource = list->getResource(resourceId);
   if (resource == NULL)
   {
      // If not, add it.
      resource = new Resource(resourceId);

      UtlString userName;
      contactUrl.getDisplayName(userName);
      resource->setName(userName);

      UtlString id;
      NetMd5Codec::encode(resourceId, id);
      resource->setInstance(id, STATE_PENDING);
      list->insertResource(resource);

      // Set up the subscription to the URI.
      OsSysLog::add(FAC_LOG, PRI_DEBUG,
                    "SipDialogMonitor::addExtension Sending out the SUBSCRIBE to contact %s",
                    resourceId.data());

      UtlString toUrl;
      contactUrl.toString(toUrl);

      UtlString fromUri = "dialogMonitor@" + mDomainName;
      UtlString earlyDialogHandle;

      UtlBoolean status = mpSipSubscribeClient->addSubscription(resourceId.data(),
                                                                DIALOG_EVENT_TYPE,
                                                                DIALOG_EVENT_CONTENT_TYPE,
                                                                fromUri.data(),
                                                                toUrl.data(),
                                                                mContact.data(),
                                                                mRefreshTimeout,
                                                                (void *) this,
                                                                SipDialogMonitor::subscriptionStateCallback,
                                                                SipDialogMonitor::notifyEventCallback,
                                                                earlyDialogHandle);

      if (!status)
      {
         result = false;
         OsSysLog::add(FAC_LOG, PRI_ERR,
                       "SipDialogMonitor::addExtension Subscription failed to contact %s.",
                       resourceId.data());
      }
      else
      {
         mDialogHandleList.insertKeyAndValue(new UtlString(resourceId),
                                             new UtlString(earlyDialogHandle));
         createDialogState(&earlyDialogHandle);
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "SipDialogMonitor::addExtension Added earlyDialogHandle: %s",
                       earlyDialogHandle.data());
         result = true;
      }
   }
   else
   {
      OsSysLog::add(FAC_LOG, PRI_WARNING,
                    "SipDialogMonitor::addExtension contact %s already exists.",
                    resourceId.data());
   }

   list->buildBody();

   mLock.release();
   return result;
}

bool SipDialogMonitor::removeExtension(UtlString& groupName, Url& contactUrl)
{
   bool result = false;
   mLock.acquire();
   // Check whether the group exists or not. If not, return false.
   SipResourceList* list =
      dynamic_cast <SipResourceList *> (mMonitoredLists.findValue(&groupName));
   if (list == NULL)
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipDialogMonitor::removeExtension group %s does not exist",
                    groupName.data());
   }
   else
   {
      // Check whether the contact exists in the group or not.
      UtlString resourceId;
      contactUrl.getIdentity(resourceId);
      Resource* resource = list->getResource(resourceId);
      if (resource)
      {
         // If it exists, get the early dialog handle for the SUBSCRIBE,
         // which specifies all of its subscriptions.
         UtlString* earlyDialogHandle =
            dynamic_cast <UtlString *> (mDialogHandleList.findValue(&resourceId));
         if (earlyDialogHandle)
         {
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "SipDialogMonitor::removeExtension Calling endSubscription(%s)",
                          earlyDialogHandle->data());
            // Terminate the subscription.
            UtlBoolean status = mpSipSubscribeClient->endSubscriptionGroup(earlyDialogHandle->data());

            if (!status)
            {
               OsSysLog::add(FAC_SIP, PRI_ERR,
                             "SipDialogMonitor::removeExtension Unsubscription failed for %s.",
                             resourceId.data());
            }

            // Remove the remembered state for dialog event notices.
            destroyDialogState(earlyDialogHandle);
         }
         else
         {
            OsSysLog::add(FAC_SIP, PRI_ERR,
                          "SipDialogMonitor::removeExtension no dialogHandle for %s.",
                          resourceId.data());
         }

         // Now delete all the references to this URI.
         mDialogHandleList.destroy(&resourceId);
         resource = list->removeResource(resource);
         delete resource;

         result = true;
      }
      else
      {
         OsSysLog::add(FAC_LOG, PRI_WARNING,
                       "SipDialogMonitor::removeExtension subscription for contact %s does not exists.",
                       resourceId.data());
      }
   }

   mLock.release();
   return result;
}

// Add 'dialogEvent' to mDialogEventList as the last dialog event for
// AOR 'contact'.
// Call notifyStateChange(), and if mToBePUblished, publishContent()
// to report this information.
void SipDialogMonitor::addDialogEvent(UtlString& contact,
                                      SipDialogEvent* dialogEvent,
                                      const char* earlyDialogHandle,
                                      const char* dialogHandle)
{
   if (mDialogEventList.find(&contact) == NULL)
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipDialogMonitor::addDialogEvent adding dialogEvent %p for contact '%s'",
                    dialogEvent, contact.data());
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipDialogMonitor::addDialogEvent dialogEvent %p for contact '%s' already exists, updating the content.",
                    dialogEvent, contact.data());

      // Get the object from the dialog event list
      UtlContainable* oldKey;
      UtlContainable* foundValue;
      oldKey = mDialogEventList.removeKeyAndValue(&contact, foundValue);
      delete oldKey;
      SipDialogEvent* oldDialogEvent = dynamic_cast <SipDialogEvent *> (foundValue);

      OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipDialogMonitor::addDialogEvent removing the dialogEvent %p for contact '%s'",
                    oldDialogEvent, contact.data());

      if (oldDialogEvent)
      {
         delete oldDialogEvent;
      }
   }

   // Insert the AOR that we subscribed to into the DialogEvent
   // object, to overwrite the entity URI provided in the body of the
   // dialog event.
   dialogEvent->setEntity(contact.data());
   // Rebuild the body.
   dialogEvent->buildBody();

   // Insert it into the dialog event list
   // :TODO: This does not merge partial dialogs with the previous state.
   mDialogEventList.insertKeyAndValue(new UtlString(contact), dialogEvent);

   if (mToBePublished)
   {
      // Publish the content to the resource list.
      publishContent(contact, dialogEvent);
   }

   // Merge the information in the current NOTIFY with the recorded state.
   // Return the on/off hook status.
   StateChangeNotifier::Status status = mergeEventInformation(dialogEvent,
                                                              earlyDialogHandle,
                                                              dialogHandle);

   // Notify listeners of the state change.
   notifyStateChange(contact, status);
}


void SipDialogMonitor::publishContent(UtlString& contact,
                                      SipDialogEvent* dialogEvent)
{
   // Loop through all the resource lists
   UtlHashMapIterator iterator(mMonitoredLists);
   UtlString* listUri;
   SipResourceList* list;
   Resource* resource;
   UtlString id, state;
   while ((listUri = dynamic_cast <UtlString *> (iterator())))
   {
      bool contentChanged = false;

      list = dynamic_cast <SipResourceList *> (mMonitoredLists.findValue(listUri));
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipDialogMonitor::publishContent listUri %s list %p",
                    listUri->data(), list);

      // Search for the contact in this list
      resource = list->getResource(contact);
      if (resource)
      {
         resource->getInstance(id, state);

         // :TODO:
         // The following code is incorrect.  The "state" of a resource in a
         // resource list reports on the status of the subscription
         // from the resource list server to the resource's event
         // publisher (draft-ietf-simple-event-list-07, top of p. 10),
         // not the on-hook/off-hook status of a SIP agent for an AOR.

         if (dialogEvent->isEmpty())
         {
            resource->setInstance(id, STATE_TERMINATED);
         }
         else
         {
            resource->setInstance(id, STATE_ACTIVE);
         }

         list->buildBody();
         contentChanged = true;
      }

      if (contentChanged)
      {
         // Publish the content to the subscribe server
         // Make a copy, because mpSipPublishContentMgr will own it.
         HttpBody* pHttpBody = new HttpBody(*(HttpBody*)list);
	 mSipPublishContentMgr.publish(listUri->data(), DIALOG_EVENT_TYPE,
                                       DIALOG_EVENT_TYPE, 1,
                                       &pHttpBody);
      }
   }
}

void SipDialogMonitor::subscriptionStateCallback(SipSubscribeClient::SubscriptionState newState,
                                                 const char* earlyDialogHandle,
                                                 const char* dialogHandle,
                                                 void* applicationData,
                                                 int responseCode,
                                                 const char* responseText,
                                                 long expiration,
                                                 const SipMessage* subscribeResponse)
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipDialogMonitor::subscriptionStateCallback is called with responseCode = %d (%s)",
                 responseCode, responseText);
}


// Callback to handle incoming NOTIFYs.
bool SipDialogMonitor::notifyEventCallback(const char* earlyDialogHandle,
                                           const char* dialogHandle,
                                           void* applicationData,
                                           const SipMessage* notifyRequest)
{
   // Receive the notification and process the message
   // Our SipdialogMonitor is pointed to by the applicationData.
   SipDialogMonitor* pThis = (SipDialogMonitor *) applicationData;

   pThis->handleNotifyMessage(notifyRequest, earlyDialogHandle, dialogHandle);

   return true;
}


/// Non-static callback to handle incoming NOTIFYs.
void SipDialogMonitor::handleNotifyMessage(const SipMessage* notifyMessage,
                                           const char* earlyDialogHandle,
                                           const char* dialogHandle)
{
   Url fromUrl;
   notifyMessage->getFromUrl(fromUrl);
   UtlString contact;
   fromUrl.getIdentity(contact);

   OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipDialogMonitor::handleNotifyMessage receiving a notify message from %s",
                 contact.data());

   const HttpBody* notifyBody = notifyMessage->getBody();

   if (notifyBody)
   {
      UtlString messageContent;
      ssize_t bodyLength;

      notifyBody->getBytes(&messageContent, &bodyLength);

      // Parse the content and store it in a SipDialogEvent object
      SipDialogEvent* sipDialogEvent = new SipDialogEvent(messageContent);

      // Add the SipDialogEvent object to the hash table
      addDialogEvent(contact, sipDialogEvent, earlyDialogHandle, dialogHandle);
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_WARNING,
                    "SipDialogMonitor::handleNotifyMessage receiving an empty notify body from %s",
                    contact.data());
   }
}

void SipDialogMonitor::addStateChangeNotifier(const char* listUri, StateChangeNotifier* notifier)
{
   mLock.acquire();
   UtlString* name = new UtlString(listUri);
   UtlVoidPtr* value = new UtlVoidPtr(notifier);
   mStateChangeNotifiers.insertKeyAndValue(name, value);
   mLock.release();
}

void SipDialogMonitor::removeStateChangeNotifier(const char* listUri)
{
   mLock.acquire();
   UtlString name(listUri);
   mStateChangeNotifiers.destroy(&name);
   mLock.release();
}

// Report to all the notifiers in mStateChangeNotifiers a new event
// 'dialogEvent' for AOR 'contact'.
void SipDialogMonitor::notifyStateChange(UtlString& contact,
                                         StateChangeNotifier::Status status)
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipDialogMonitor::notifyStateChange "
                 "AOR = '%s', status = %s",
                 contact.data(),
                 (status == StateChangeNotifier::ON_HOOK ? "ON_HOOK" :
                  status == StateChangeNotifier::OFF_HOOK ? "OFF_HOOK" :
                  "UNKNOWN"));
   Url contactUrl(contact);

   // Loop through the notifier list, reporting the status to the notifiers.
   UtlHashMapIterator iterator(mStateChangeNotifiers);
   UtlString* listUri;
   UtlVoidPtr* container;
   StateChangeNotifier* notifier;
   while ((listUri = dynamic_cast <UtlString *> (iterator())))
   {
      container = dynamic_cast <UtlVoidPtr *> (mStateChangeNotifiers.findValue(listUri));
      notifier = (StateChangeNotifier *) container->getValue();
      // Report the status to the notifier.
      notifier->setStatus(contactUrl, status);
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipDialogMonitor::notifyStateChange setting state to %d",
                    status);
   }
}

StateChangeNotifier::Status
SipDialogMonitor::mergeEventInformation(SipDialogEvent* dialogEvent,
                                        const char* earlyDialogHandle,
                                        const char* dialogHandle)
{
   // Status to be returned to caller.
   StateChangeNotifier::Status rc;

   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipDialogMonitor::mergeEventInformation "
                 "earlyDialogHandle = '%s', dialogHandle = '%s'",
                 earlyDialogHandle, dialogHandle);

   // Get the list of active dialogs for the group of subscriptions generated
   // by this SUBSCRIBE.
   UtlString earlyDialogHandleString(earlyDialogHandle);
   UtlHashBag* active_dialog_list =
      dynamic_cast <UtlHashBag*> (mDialogState.findValue(&earlyDialogHandleString));
   // Ignore the event if there is no entry in mDialogState -- this is a
   // NOTIFY that arrived after and un-SUBSCRIBE terminated its subscription.
   if (active_dialog_list)
   {
      // If this is a full update, remove from the active list any dialog
      // for this dialog handle.
      UtlString notify_state;
      dialogEvent->getState(notify_state);
      if (notify_state.compareTo("full") == 0)
      {
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "SipDialogMonitor::mergeEventInformation "
                       "active_dialog_list elements for handle '%s'",
                       dialogHandle);
         // Iterate through the active dialog list, removing elements
         // for this dialog handle.
         UtlHashBagIterator dialog_id_itor(*active_dialog_list);
         UtlString* dialog_id;
         while ((dialog_id = dynamic_cast <UtlString*> (dialog_id_itor())))
         {
            // Extract the dialog handle part of the dialog identifier string
            // and compare it to the dialog handle of this event notice.
            if (strcmp(dialogHandle,
                       dialog_id->data() + dialog_id->index("\001") + 1) == 0)
            {
               // This is a dialog for this subscription, so remove it.
               active_dialog_list->remove(dialog_id);
            }
         }
      }

      // Iterate through the dialog event, updating active_dialog_list for
      // each dialog mentioned.
      UtlSListIterator* dialog_itor = dialogEvent->getDialogIterator();
      Dialog* dialog;
      UtlString state, event, code, dialogId;
      while ((dialog = dynamic_cast <Dialog*> ((*dialog_itor)())))
      {
         // Construct the dialog identifier string,
         // <dialog id><ctrl-A><dialog handle>
         dialog->getDialogId(dialogId);
         dialogId.append("\001");
         dialogId.append(dialogHandle);

         dialog->getState(state, event, code);
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "SipDialogMonitor::mergeEventInformation "
                       "dialogId '%s' state '%s' event '%s' code '%s'",
                       dialogId.data(), state.data(), event.data(), code.data());
         if (state.compareTo("terminated") != 0)
         {
            // Active dialog.
            // If it is not in active_dialog_list, add it.
            if (!active_dialog_list->contains(&dialogId))
            {
               OsSysLog::add(FAC_SIP, PRI_DEBUG,
                             "SipDialogMonitor::mergeEventInformation "
                             "adding dialog '%s'",
                             dialogId.data());
               active_dialog_list->insert(new UtlString(dialogId));
            }
         }
         else
         {
            // Terminated dialog
            // If it is in active_dialog_list, remove it.
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "SipDialogMonitor::mergeEventInformation "
                          "removing dialog '%s'",
                          dialogId.data());
            active_dialog_list->destroy(&dialogId);
         }
      }
      delete dialog_itor;

      // If debugging, list the active dialog list.
      if (OsSysLog::willLog(FAC_SIP, PRI_DEBUG))
      {
         UtlHashBagIterator dialog_list_itor(*active_dialog_list);
         UtlString* dialog;
         while ((dialog = dynamic_cast <UtlString*> (dialog_list_itor())))
         {
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "SipDialogMonitor::mergeEventInformation "
                          "active dialog '%s'",
                          dialog->data());
         }
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "SipDialogMonitor::mergeEventInformation "
                       "End of list");
      }

      // Set the return code based on whether there are any active dialogs.
      rc = active_dialog_list->isEmpty() ?
         StateChangeNotifier::ON_HOOK :
         StateChangeNotifier::OFF_HOOK;
   }
   else
   {
      // This is a late NOTIFY and there are (should be) no notifiers for it,
      // so the return code is arbitrary.
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipDialogMonitor::mergeEventInformation "
                    "No active dialog list found");
      rc = StateChangeNotifier::ON_HOOK;
   }
   return rc;
}

void SipDialogMonitor::createDialogState(UtlString* earlyDialogHandle)
{
   mDialogState.insertKeyAndValue(new UtlString(*earlyDialogHandle),
                                  new UtlHashBag);
}

void SipDialogMonitor::destroyDialogState(UtlString* earlyDialogHandle)
{
   // Remove the remembered state for the subscriptions with this
   // early dialog handle.
   UtlHashBag* active_dialog_list =
      dynamic_cast <UtlHashBag*> (mDialogState.findValue(earlyDialogHandle));
   // Remove the contents of the UtlHashBag which is the value.
   active_dialog_list->destroyAll();
   // Now remove the entry in mDialogState and the UtlHashBag itself.
   mDialogState.destroy(earlyDialogHandle);
}
