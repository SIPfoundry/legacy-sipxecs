//
// Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES

#include "os/OsLogger.h"
#include "net/SipMessage.h"
#include "net/HttpBody.h"
#include "net/SipDialogEvent.h"
#include "net/SipRegEvent.h"
#include "utl/UtlHashMapIterator.h"
#include "utl/UtlHashBagIterator.h"
#include "xmlparser/tinyxml.h"
#include "xmlparser/ExtractContent.h"
#include "AppearanceGroup.h"
#include "Appearance.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// After terminating subscriptions to a contact this is the minimum number of
// msecs to wait  before initiating a subscription that might reach the same UA.
// This is set to 50 ms and is similar with the one from RLS.
#define SUBSCRIPTION_WAIT_MSEC 50
// Increment in msecs of the wait in case subsequent subscriptions are initiated.
#define SUBSCRIPTION_WAIT_INCR_MSEC 2

// STATIC VARIABLE INITIALIZATIONS

const UtlContainableType AppearanceGroup::TYPE = "AppearanceGroup";


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
AppearanceGroup::AppearanceGroup(AppearanceGroupSet* appearanceGroupSet,
                       const char* user) :
//   mResource(resource),
   mAppearanceGroupSet(appearanceGroupSet),
   mSharedUser(user)
{
   Os::Logger::instance().log(FAC_SAA, PRI_DEBUG,
                 "AppearanceGroup:: this = %p, mSharedUser = '%s'",
                 this, mSharedUser.data());

   // Publish initial content (presumably empty)
   SipDialogEvent* lFullContent = new SipDialogEvent("full", mSharedUser.data());
   UtlHashMapIterator appitor(mAppearances);
   UtlString* handle;
   while ( (handle = dynamic_cast <UtlString*> (appitor())))
   {
      Appearance* inst = dynamic_cast <Appearance*> (appitor.value());
      inst->getDialogs(lFullContent);
   }
   lFullContent->buildBody();

  // Publish the content for this shared user to the Subscribe Server.
  // Make a copy, because mpSipPublishContentMgr will own it.
  HttpBody* pHttpBody = new HttpBody(*(HttpBody*)lFullContent);
  delete lFullContent;
  getAppearanceAgent()->getEventPublisher().publish(
        mSharedUser.data(),
        DIALOG_SLA_EVENT_TYPE, //eventTypeKey
        DIALOG_EVENT_TYPE,     //eventType
        1, &pHttpBody,
        TRUE, TRUE);

   startSubscription();
}

void AppearanceGroup::startSubscription()
{
   // Start the subscription for Reg events to this shared user.
   UtlBoolean ret;
   UtlString lUriNameAddr = "<" + mSharedUser + ">";
   ret = getAppearanceAgent()->getSubscribeClient().
      addSubscription(mSharedUser.data(),
                      REG_EVENT_TYPE,
                      REG_EVENT_CONTENT_TYPE,
                      getAppearanceAgent()->getServerFromURI(),
                      lUriNameAddr.data(),
                      getAppearanceAgent()->getServerContactURI(),
                      getAppearanceAgent()->getResubscribeInterval(),
                      mAppearanceGroupSet,
                      AppearanceGroupSet::subscriptionEventCallbackAsync,
                      AppearanceGroupSet::notifyEventCallbackAsync,
                      mSubscriptionEarlyDialogHandle);
   if (ret)
   {
      Os::Logger::instance().log(FAC_SAA, PRI_INFO,
                    "AppearanceGroup:: startSubscription succeeded mSharedUser = '%s', mSubscriptionEarlyDialogHandle = '%s'",
                    mSharedUser.data(),
                    mSubscriptionEarlyDialogHandle.data());
      // Add this AppearanceGroup to mSubscribeMap.
      getAppearanceAgent()->getAppearanceGroupSet().addSubscribeMapping(&mSubscriptionEarlyDialogHandle,
                                                this);
   }
   else
   {
      Os::Logger::instance().log(FAC_SAA, PRI_WARNING,
                    "AppearanceGroup:: startSubscription failed mSharedUser = '%s', mSubscriptionEarlyDialogHandle = '%s'",
                    mSharedUser.data(),
                    mSubscriptionEarlyDialogHandle.data());
   }
}

// Destructor
AppearanceGroup::~AppearanceGroup()
{
   Os::Logger::instance().log(FAC_SAA, PRI_DEBUG,
                 "AppearanceGroup::~ this = %p, mSharedUser = '%s'",
                 this, mSharedUser.data());
   // Delete this AppearanceGroup from mSubscribeMap (for the "reg" subscription).
   getAppearanceAgent()->getAppearanceGroupSet().deleteSubscribeMapping(&mSubscriptionEarlyDialogHandle);

   // End the "reg" subscription.
   UtlBoolean ret;
   ret = getAppearanceAgent()->getSubscribeClient().
      endSubscriptionGroup(mSubscriptionEarlyDialogHandle);
   Os::Logger::instance().log(FAC_SAA,
                 ret ? PRI_INFO : PRI_WARNING,
                 "AppearanceGroup::~ endSubscriptionGroup %s mSharedUser = '%s', mSubscriptionEarlyDialogHandle = '%s'",
                 ret ? "succeeded" : "failed",
                 mSharedUser.data(),
                 mSubscriptionEarlyDialogHandle.data());

   // Remove this AppearanceGroup from mNotifyMap for all subscriptions.
   {
      UtlHashMapIterator itor(mSubscriptions);
      UtlString* handle;
      while ((handle = dynamic_cast <UtlString*> (itor())))
      {
         getAppearanceAgent()->getAppearanceGroupSet().deleteNotifyMapping(handle);
      }
   }

   // Delete the contents of mSubscriptions.
   mSubscriptions.destroyAll();

   // Terminate all dialogs for this shared Appearance Group, then publish
   bool bContentChanged = false;
   SipDialogEvent* lPartialContent = new SipDialogEvent("partial", mSharedUser.data());
   {
      UtlHashMapIterator itor(mAppearances);
      UtlString* handle;
      while ( (handle = dynamic_cast <UtlString*> (itor())) )
      {
         Appearance* pApp = dynamic_cast <Appearance*> (itor.value());
         bContentChanged |= pApp->terminateDialogs(true); // terminate all dialogs
         pApp->getDialogs(lPartialContent);
      }
   }
   if (bContentChanged)
   {
      lPartialContent->buildBody();
      publish(true, true, lPartialContent);
   }
   delete lPartialContent;

   // Delete the subordinate Appearance's for the contacts.
   {
      // Have to use a loop to remove items individually, because
      // destroyAll() deadlocks with the access to mAppearances
      // during the the attempt to publish new status for the resource
      // as the Appearances are recursively deleted.
      int changeDelay = getAppearanceAgent()->getChangeDelay();
      UtlHashMapIterator itor(mAppearances);
      UtlContainable* k;
      while ((k = itor()))
      {
         UtlContainable* v = itor.value();
         mAppearances.removeReference(k);
         delete k;
         delete v;
         // Delay to allow the consequent processing to catch up.
         OsTask::delay(changeDelay);
      }
   }

   mTerminatedAppearances.destroyAll();
}

/* ============================ MANIPULATORS ============================== */

void AppearanceGroup::subscriptionEventCallback(
   const UtlString* earlyDialogHandle,
   const UtlString* dialogHandle,
   SipSubscribeClient::SubscriptionState newState,
   const UtlString* subscriptionState)
{
   Os::Logger::instance().log(FAC_SAA, PRI_DEBUG,
                 "AppearanceGroup::subscriptionEventCallback mUri = '%s', newState = %d, earlyDialogHandle = '%s', dialogHandle = '%s', subscriptionState = '%s'",
                 mSharedUser.data(),
                 newState, mSubscriptionEarlyDialogHandle.data(),
                 dialogHandle->data(), subscriptionState->data());

   switch (newState)
   {
   case SipSubscribeClient::SUBSCRIPTION_INITIATED:
      break;
   case SipSubscribeClient::SUBSCRIPTION_SETUP:
   {
      // Check that we don't have too many subscriptions.
      if (mSubscriptions.entries() <
          getAppearanceAgent()->getMaxRegSubscInGroup())
      {
         // Add this AppearanceGroup to mNotifyMap for the subscription.
         getAppearanceAgent()->getAppearanceGroupSet().addNotifyMapping(dialogHandle, this);
         // Remember it in mSubscriptions, if there isn't already an entry.
         if (!mSubscriptions.find(dialogHandle))
         {
            mSubscriptions.insertKeyAndValue(new UtlString(*dialogHandle), new UtlHashMap);
            Os::Logger::instance().log(FAC_SAA, PRI_INFO,
                          "AppearanceGroup::subscriptionEventCallback "
                          "subscription setup for AppearanceGroup = '%s', dialogHandle = '%s'",
                          mSharedUser.data(), dialogHandle->data());
         }
         else
         {
            Os::Logger::instance().log(FAC_SAA, PRI_DEBUG,
                          "AppearanceGroup::subscriptionEventCallback "
                          "mSubscriptions element already exists for this dialog handle mUri = '%s', dialogHandle = '%s'",
                          mSharedUser.data(),
                          dialogHandle->data());
         }
      }
      else
      {
         Os::Logger::instance().log(FAC_SAA, PRI_ERR,
                       "AppearanceGroup::subscriptionEventCallback "
                       "cannot add reg subscription with dialog handle '%s', already %zu in AppearanceGroup '%s'",
                       dialogHandle->data(), mSubscriptions.entries(),
                       mSharedUser.data());
      }
   }
   break;
   case SipSubscribeClient::SUBSCRIPTION_TERMINATED:
   {
      // Remove this dialogHandle from mNotifyMap for the subscription.
      // A new one will be added later if necessary.
      mAppearanceGroupSet->deleteNotifyMapping(dialogHandle);

      // Delete this subscription from mSubscriptions.
      mSubscriptions.destroy(dialogHandle);

      // Update the subscriptions.
      updateSubscriptions();
   }
   break;
   }
}

// Process a notify event callback.
// This involves parsing the content of the callback and revising our record
// of the state for that subscription.  Then, we must regenerate the list
// of contacts and update the set of subscriptions to match the current
// contacts.
// This method MUST generate a response to the event, since SAA tells
// the SubscribeClient not to.
void AppearanceGroup::notifyEventCallback(const UtlString* dialogHandle,
                                     const SipMessage* msg)
{
   // Get the NOTIFY content.
   const char* content;
   ssize_t l;
   const HttpBody* body = msg->getBody();
   if (body)
   {
      body->getBytes(&content, &l);
   }
   else
   {
      content = NULL;
      l = 0;
   }

   Os::Logger::instance().log(FAC_SAA, PRI_DEBUG,
                 "AppearanceGroup::notifyEventCallback mSharedUser = '%s', dialogHandle = '%s', content = '%s'",
                 mSharedUser.data(), dialogHandle->data(), content);

   // Acknowledge the NOTIFY.
   SipMessage response;
   response.setOkResponseData(msg, NULL);
   getAppearanceAgent()->getServerUserAgent().send(response);

   // Find the UtlHashMap for this subscription.
   UtlHashMap* state_from_this_subscr =
      dynamic_cast <UtlHashMap*> (mSubscriptions.findValue(dialogHandle));
   if (!state_from_this_subscr)
   {
      // No state for this dialogHandle, so we need to add one.
      Os::Logger::instance().log(FAC_SAA, PRI_INFO,
                    "AppearanceGroup::notifyEventCallback mSubscriptions element does not exist for this dialog handle mUri = '%s', dialogHandle = '%s'",
                    mSharedUser.data(),
                    dialogHandle->data());

      // Check that we don't have too many subscriptions.
      if (mSubscriptions.entries() <
          getAppearanceAgent()->getMaxRegSubscInGroup())
      {
         state_from_this_subscr = new UtlHashMap;
         mSubscriptions.insertKeyAndValue(new UtlString(*dialogHandle),
                                          state_from_this_subscr);
      }
      else
      {
         Os::Logger::instance().log(FAC_SAA, PRI_ERR,
                       "AppearanceGroup::notifyEventCallback cannot add reg subscription with dialog handle '%s', already %zu in AppearanceGroup '%s'",
                       dialogHandle->data(), mSubscriptions.entries(),
                       mSharedUser.data());
      }
   }

   // Perform the remainder of the processing if we obtained a hash map
   // from the above processing.
   // Parse the XML and update the contact status.
   if (state_from_this_subscr)
   {
      // Initialize Tiny XML document object.
      TiXmlDocument document;
      TiXmlNode* reginfo_node;
      if (
         // Load the XML into it.
         document.Parse(content) &&
         // Find the top element, which should be a <reginfo>.
         (reginfo_node = document.FirstChild("reginfo")) != NULL &&
         reginfo_node->Type() == TiXmlNode::ELEMENT)
      {
         // Check the state attribute.
         const char* p = reginfo_node->ToElement()->Attribute("state");
         if (p && strcmp(p, "full") == 0)
         {
            // If the state is "full", delete the current state.
            state_from_this_subscr->destroyAll();
            Os::Logger::instance().log(FAC_SAA, PRI_DEBUG,
                          "AppearanceGroup::notifyEventCallback clearing state");
         }

         // Find all the <registration> elements for this URI.
         for (TiXmlNode* registration_node = 0;
              (registration_node =
               reginfo_node->IterateChildren("registration",
                                             registration_node));
            )
         {
            // Do not test the aor attribute of <registration> elements
            // because the reg event server may be operating with the real
            // AOR for this URI, whereas we may have been told of an alias.

            // Find all the <contact> elements.
            for (TiXmlNode* contact_node = 0;
                 (contact_node =
                  registration_node->IterateChildren("contact",
                                                     contact_node));
               )
            {
               TiXmlElement* contact_element = contact_node->ToElement();

               // Get the state attribute.
               const char* state = contact_element->Attribute("state");
               // Get the id attribute
               const char* id = contact_element->Attribute("id");

               // Get the Contact URI for the phone. If a GRUU address is present we should
               // use that as the Contact URI. Otherwise, use the contact URI present in the
               // "uri" element, and append any path headers that are present to the ROUTE
               // header parameter in the URI. This will ensure proper routing in HA systems.
               // Please refer to XECS-1694 for more details.

               UtlString* uri_allocated = new UtlString;
               UtlBoolean check_uri = TRUE;
               TiXmlNode* pub_gruu_node = contact_element->FirstChild("gr:pub-gruu");
               if(pub_gruu_node)
               {
                  TiXmlElement* pub_gruu_element = pub_gruu_node->ToElement();
                  UtlString pub_gruu_uri (pub_gruu_element->Attribute("uri"));
                  if(!pub_gruu_uri.isNull())
                  {
                     // Check the URI Scheme. Only accept the GRUU address if it is of either
                     // a sip or sips scheme
                     Url tmp (pub_gruu_uri, TRUE );
                     Url::Scheme uriScheme = tmp.getScheme();
                     if(Url::SipUrlScheme == uriScheme ||
                        Url::SipsUrlScheme == uriScheme)
                     {
                        tmp.removeAngleBrackets();
                        tmp.getUri(*uri_allocated);
                        check_uri = FALSE;
                     }
                  }
               }

               // If we did not find a GRUU address, then use the address in the "uri" element as the
               // contact URI, and check for path headers.
               if(check_uri)
               {
                  TiXmlNode* u = contact_element->FirstChild("uri");
                  if (u)
                  {
                     textContentShallow(*uri_allocated, u);

                     // Iterate through all the path header elements. Path headers are stored in the
                     // "unknown-param" elements that have a "name" attribute value of "path".
                     for (TiXmlNode* unknown_param_node = 0;
                          (unknown_param_node = contact_node->IterateChildren("unknown-param",
                                                                               unknown_param_node));
                          )
                     {
                        TiXmlElement* unknown_param_element = unknown_param_node->ToElement();
                        UtlString path(unknown_param_element->Attribute("name"));
                        if(0 == path.compareTo("path"))
                        {
                           UtlString pathVector;
                           textContentShallow(pathVector, unknown_param_node);
                           if(!pathVector.isNull())
                           {
                              Url contact_uri(*uri_allocated, TRUE);

                              // there is already a Route header parameter in the contact; append it to the
                              // Route derived from the Path vector.
                              UtlString existingRouteValue;
                              if ( contact_uri.getHeaderParameter(SIP_ROUTE_FIELD, existingRouteValue))
                              {
                                 pathVector.append(SIP_MULTIFIELD_SEPARATOR);
                                 pathVector.append(existingRouteValue);
                              }
                              contact_uri.setHeaderParameter(SIP_ROUTE_FIELD, pathVector);
                              contact_uri.removeAngleBrackets();
                              contact_uri.getUri(*uri_allocated);
                           }
                        }
                     }
                  }
               }

               // Only process <contact> elements that have the needed values.
               if (state && state[0] != '\0' &&
                   id && id[0] != '\0' &&
                   !uri_allocated->isNull())
               {
                  UtlString* id_allocated = new UtlString(id);

                  if (strcmp(state, "active") == 0)
                  {
                     // Add the contact if it is not already present.
                     if (!state_from_this_subscr->find(id_allocated))
                     {
                        // Prepend the registration Call-Id and ';'.
                        uri_allocated->insert(0, ';');
                        const char* call_id = contact_element->Attribute("callid");
                        if (call_id)
                        {
                           uri_allocated->insert(0, call_id);
                        }

                        // Insert the registration record.
                        if (state_from_this_subscr->insertKeyAndValue(  id_allocated,
                                                                        uri_allocated))
                        {
                            Os::Logger::instance().log( FAC_SAA, PRI_DEBUG,
                                  "AppearanceGroup::notifyEventCallback adding id = '%s' Call-Id;URI = '%s'",
                                  id, uri_allocated->data());
                            id_allocated = NULL;
                            uri_allocated = NULL;
                        }
                        else
                        {
                            Os::Logger::instance().log(FAC_RLS, PRI_ERR,
                                          "AppearanceGroup::notifyEventCallback adding id = '%s' Call-Id;URI = '%s' failed",
                                          id_allocated->data(), uri_allocated->data());
                        }
                     }
                  }
                  else if (strcmp(state, "terminated") == 0)
                  {
                     // Delete it from the contact state.
                     state_from_this_subscr->destroy(id_allocated);
                     Os::Logger::instance().log(FAC_SAA, PRI_DEBUG,
                                   "AppearanceGroup::notifyEventCallback deleting id = '%s'",
                                   id);
                  }
                  // Free id_allocated, if it is not pointed to by a data
                  // structure, which is indicated by setting it to NULL.
                  if (id_allocated)
                  {
                     delete id_allocated;
                  }
               }
               else
               {
                  delete uri_allocated;
                  Os::Logger::instance().log(FAC_SAA, PRI_ERR,
                                "AppearanceGroup::notifyEventCallback <contact> element with id = '%s' is missing id, state, and/or URI",
                                id ? id : "(missing)");
               }
               // Free uri_allocated, if it is not pointed to by a data
               // structure, which is indicated by setting it to NULL.
               if (uri_allocated)
               {
                  delete uri_allocated;
               }
            }
         }
      }
      else
      {
         // Error parsing the contents.
         Os::Logger::instance().log(FAC_SAA, PRI_ERR,
                       "AppearanceGroup::notifyEventCallback malformed reg event content for mSharedUser = '%s'",
                       mSharedUser.data());
      }

      // Update the subscriptions we maintain to agree with the new state.
      updateSubscriptions();
   }
}


void AppearanceGroup::deleteTerminatedAppearances()
{
   Os::Logger::instance().log(FAC_SAA, PRI_DEBUG,
           "AppearanceGroup::deleteTerminatedAppearances");

   // Search for the resource instance in question.
   UtlSListIterator itor(mTerminatedAppearances);
   Appearance* inst = 0;

   int count = 0;
   while ((inst = dynamic_cast <Appearance*> (itor())))
   {
      if (true == inst->isTerminated())
      {
          mTerminatedAppearances.destroy(inst);
          count++;
      }
   }

   Os::Logger::instance().log(FAC_SAA, PRI_DEBUG, "AppearanceGroup::deleteTerminatedAppearances "
           "deleted %d terminated appearances", count);
}

void AppearanceGroup::publish(bool bSendFullContent, bool bSendPartialContent, SipDialogEvent* lContent)
{
   Os::Logger::instance().log(FAC_SAA, PRI_DEBUG,
         "AppearanceGroup::publish sending NOTIFY for: '%s'", mSharedUser.data());

   deleteTerminatedAppearances();

   if (bSendFullContent)
   {
      // Both the Full and the Partial dialog-infos are sent to the SIP Subscribe Server.
      // The Partial will then be sent out right away and the Full
      // will be stored in the Subscribe Server to be sent on any initial
      // SUBSCRIBEs and re-SUBSCRIBEs.
      // Note that the full-state publish() must be done before the partial-state
      // publish() to avoid race conditions with regard to starting a new
      // subscription.
      // The Full content is constructed from all dialogs being managed by all appearances.
      UtlHashMapIterator itor(mAppearances);
      SipDialogEvent* lFullContent = new SipDialogEvent("full", mSharedUser.data());
      UtlHashMapIterator appitor(mAppearances);
      UtlString* handle;
      while ( (handle = dynamic_cast <UtlString*> (appitor())))
      {
         Appearance* inst = dynamic_cast <Appearance*> (appitor.value());
         inst->getDialogs(lFullContent);
      }
      lFullContent->buildBody();

      // Publish the content to the subscribe server.
      // Make a copy, because SipPublishContentMgr will own it.
      HttpBody* pHttpBody = new HttpBody(*(HttpBody*)lFullContent);

      getAppearanceAgent()->getEventPublisher().publish(
            mSharedUser.data(),
            DIALOG_SLA_EVENT_TYPE, //eventTypeKey
            DIALOG_EVENT_TYPE,     //eventType
            1, &pHttpBody,
            TRUE, TRUE);
      delete lFullContent;
   }

   if ( bSendPartialContent )
   {
      // The Partial content is the incoming message, with unique dialog ids.
      lContent->setState(STATE_PARTIAL);
      lContent->buildBody();
      HttpBody* pPartialBody = new HttpBody(*(HttpBody*)lContent);
      getAppearanceAgent()->getEventPublisher().publish(
            mSharedUser.data(),
            DIALOG_SLA_EVENT_TYPE, //eventTypeKey
            DIALOG_EVENT_TYPE,     //eventType
            1, &pPartialBody,
            FALSE, FALSE);
   }

}

/// Process an incoming NOTIFY from an Appearance.  Always sends a response.
void AppearanceGroup::handleNotifyRequest(const UtlString* dialogHandle,
                                          const SipMessage* msg)
{
   SipMessage response;
   Appearance* pThisAppearance = findAppearance(*dialogHandle);
   if ( !pThisAppearance )
   {
      UtlString swappedDialogHandle;
      mAppearanceGroupSet->swapTags(*dialogHandle, swappedDialogHandle);
      pThisAppearance = findAppearance(swappedDialogHandle);
      if ( !pThisAppearance )
      {
         // should never happen, since the NOTIFY was sent straight to the Appearance
         Os::Logger::instance().log(FAC_SAA, PRI_WARNING,
               "AppearanceGroup::handleNotifyRequest: ignoring NOTIFY from unknown subscription, dialogHandle %s",
               dialogHandle->data());
         response.setInterfaceIpPort(msg->getInterfaceIp(), msg->getInterfacePort());
         response.setResponseData(msg, 481, "Subscription does not exist");
         getAppearanceAgent()->getServerUserAgent().send(response);
         return;
      }
   }
   UtlString contactUri = pThisAppearance->getUri()->data();

   // check that event type is supported
   UtlString eventType;
   msg->getEventField(eventType);
   if (eventType != SLA_EVENT_TYPE)
   {
      Os::Logger::instance().log(FAC_SAA, PRI_INFO,
                    "AppearanceGroup::handleNotifyRequest: ignoring NOTIFY(%s): not an SLA event", eventType.data());
      response.setOkResponseData(msg, NULL);
      getAppearanceAgent()->getServerUserAgent().send(response);
      return;
   }

   // Get the NOTIFY content.
   const char* content;
   ssize_t l;
   const HttpBody* body = msg->getBody();
   if (body)
   {
      body->getBytes(&content, &l);
   }
   else
   {
      Os::Logger::instance().log(FAC_SAA, PRI_WARNING,
            "AppearanceGroup::handleNotifyRequest: could not get NOTIFY content, dialogHandle %s",
            dialogHandle->data());
      response.setInterfaceIpPort(msg->getInterfaceIp(), msg->getInterfacePort());
      response.setResponseData(msg, 493, "Undecipherable");
      getAppearanceAgent()->getServerUserAgent().send(response);
      return;
   }
   SipDialogEvent* lContent = new SipDialogEvent(content);

   UtlString state;
   UtlString entity;
   lContent->getState(state);
   lContent->getEntity(entity);

   UtlString dialogState = STATE_TERMINATED;
   UtlString event;
   UtlString code;
   UtlString appearanceId;

   if (state == STATE_TERMINATED)
   {
      // probably not needed now that SipSubscribeClient checks for NOTIFY with terminated state
      // our subscription to this Appearance has terminated.
      Os::Logger::instance().log(FAC_SAA, PRI_DEBUG,
                    "AppearanceGroup::handleNotifyRequest: subscription to %s has been terminated",
                    contactUri.data());
      // terminate any non-held dialogs? or all dialogs?
   }

   Os::Logger::instance().log(FAC_SAA, PRI_DEBUG,
                 "AppearanceGroup::handleNotifyRequest: %s update from %s",
                 state.data(), contactUri.data());

   // Assign each dialog a globally-unique ID, if not done yet
   UtlSListIterator* itor = lContent->getDialogIterator();
   Dialog* pDialog;
   while ( (pDialog = dynamic_cast <Dialog*> ((*itor)())) )
   {
      UtlString dialogId;
      UtlString uniqueDialogId;
      UtlString rendering;
      pDialog->getDialogId(dialogId);
      pDialog->getLocalParameter("x-line-id", appearanceId);
      pDialog->getLocalParameter("+sip.rendering", rendering);
      pDialog->getState(dialogState, event, code);

      // Ignore calls with no appearanceId -  these are considered "exclusive".
      // (e.g. MoH calls are private to the set involved)
      // These dialogs are not forwarded on to other sets in either partial or full updates.
      if ( appearanceId == "" )
      {
         Os::Logger::instance().log(FAC_SAA, PRI_DEBUG, "AppearanceGroup::handleNotifyRequest skipping call with no appearance info");
         delete lContent->removeDialog(pDialog);
         continue;
      }

      if ( dialogId.contains("@@") )
      {
         // this is one of our identifiers already
         uniqueDialogId = dialogId;
      }
      else
      {
         // make a guaranteed unique dialog id, by
         // prepending to each id value the call-id of the subscription
         // from which the dialog event was obtained, with "@@" as a separator
         msg->getCallIdField(&uniqueDialogId);
         uniqueDialogId.append("@@");
         uniqueDialogId.append(dialogId);
         pDialog->setDialogId(uniqueDialogId);
      }
      Os::Logger::instance().log(FAC_SAA, PRI_INFO,
                    "AppearanceGroup::handleNotifyRequest: "
                    "%s update from %s: dialogId %s, x-line-id %s, state %s(%s)",
                    state.data(), contactUri.data(), uniqueDialogId.data(),
                    appearanceId.data(), dialogState.data(), rendering.data());
   }
   delete itor;

   // Check to see if this appearance (of the Appearance) is available
   bool okToProceed = true;
   if ( state == "partial" && dialogState == "trying" )
   {
      UtlHashMapIterator appitor(mAppearances);
      UtlString* handle;
      while ( okToProceed && (handle = dynamic_cast <UtlString*> (appitor())))
      {
         Appearance* inst = dynamic_cast <Appearance*> (appitor.value());
         okToProceed = !inst->appearanceIdIsSeized(appearanceId);
      }
   }

   bool bSendPartialContent = false;
   bool bSendFullContent = false;

   // Send the response.
   if (okToProceed)
   {
      response.setOkResponseData(msg, NULL);
   }
   else
   {
      Os::Logger::instance().log(FAC_SAA, PRI_DEBUG,
                    "AppearanceGroup::handleNotifyRequest '%s' appearanceId %s is busy",
                    entity.data(), appearanceId.data());
      response.setInterfaceIpPort(msg->getInterfaceIp(), msg->getInterfacePort());
      response.setResponseData(msg, 409, "Conflict");
   }
   getAppearanceAgent()->getServerUserAgent().send(response);

   if (okToProceed)
   {
      if (lContent)
      {
         // Send the content to the proper Appearance instance, so it can
         // save these dialogs, and set flag to indicate whether they should be sent in partial update
         bSendPartialContent = pThisAppearance->updateState(lContent, bSendFullContent);
      }
      // Publish full and partial updates to the SipPublishContentMgr
      publish(bSendFullContent, bSendPartialContent, lContent);
   }

   // Adjust the expiration of the subscription if the set has the line "seized"
   // and return it to the longer default if it does not.
   UtlHashMapIterator appitor(mAppearances);
   UtlString* handle;
   while ( (handle = dynamic_cast <UtlString*> (appitor())))
   {
      Appearance* inst = dynamic_cast <Appearance*> (appitor.value());

      // if set has line seized, use short subscription timer
      if ( inst->appearanceIsBusy() )
      {
         inst->setResubscribeInterval(true);
      }
      else
      {
         inst->setResubscribeInterval(false);
      }
   }

   if (lContent)
   {
      delete lContent;
   }
}


// Update the subscriptions we maintain to agree with the current contact state
void AppearanceGroup::updateSubscriptions()
{
   Os::Logger::instance().log(FAC_SAA, PRI_DEBUG,
                 "AppearanceGroup::updateSubscriptions mUri = '%s'",
                 mSharedUser.data());

   // First, scan mSubscriptions to construct a list of all the
   // call-id/contact combinations.
   // (If a phone reboots and starts registering with a different Call-Id,
   // the call-id/contact combination will be different even if the contact
   // URI is unchanged.  So the new registration will appear to be different
   // to this machinery, and it will establish a new Appearance to
   // the contact URI.  This compensates for the fact that the previous
   // Appearance to the contact URI appears to the Appearance Agent to be
   // working but the phone no longer knows of the subscription.  The
   // reg events will eventually terminate the old combination and we
   // will delete its Appearance.)
   UtlHashBag callid_contacts;

   UtlHashMapIterator subs_itor(mSubscriptions);
   while (subs_itor())
   {
      if (Os::Logger::instance().willLog(FAC_SAA, PRI_DEBUG))
      {
         Os::Logger::instance().log(FAC_SAA, PRI_DEBUG,
                       "AppearanceGroup::updateSubscriptions subscription '%s'",
                       (dynamic_cast <UtlString*> (subs_itor.key()))->data());
      }
      UtlHashMap* contact_state =
         dynamic_cast <UtlHashMap*> (subs_itor.value());
      Os::Logger::instance().log(FAC_SAA, PRI_DEBUG,
                    "AppearanceGroup::updateSubscriptions contact_state = %p",
                    contact_state);
      UtlHashMapIterator contact_itor(*contact_state);
      while (contact_itor())
      {
         UtlString* contact =
            dynamic_cast <UtlString*> (contact_itor.value());
         if (Os::Logger::instance().willLog(FAC_SAA, PRI_DEBUG))
         {
            Os::Logger::instance().log(FAC_SAA, PRI_DEBUG,
                          "AppearanceGroup::updateSubscriptions contact id '%s', Call-Id/URI '%s'",
                          (dynamic_cast <UtlString*> (contact_itor.key()))->data(),
                          contact->data());
         }
         // Check if the contact is already in callid_contacts.
         if (!callid_contacts.find(contact))
         {
            // If not, add it.
            UtlString* c = new UtlString(*contact);
            callid_contacts.insert(c);
            Os::Logger::instance().log(FAC_SAA, PRI_DEBUG,
                          "AppearanceGroup::updateSubscriptions contact '%s' added", c->data());
         }
      }
   }

   // Now that we have a clean list of callid_contacts, update
   // Appearances to match it.

   // If we both terminate subscriptions and create subscriptions,
   // wait a short while to allow the terminations to complete.  This
   // should not be necessary, but it makes life easier on Polycom
   // phones which (at this time) cannot support two subscriptions at
   // a time, and if the termination of the old subscription arrives
   // after the initiation of the new subscription, the new
   // subscription will be lost.
   // This variable tracks whether such a wait is needed before a
   // subscription is started.
   bool wait_after_subscription_ended = false;

   // Iterate through the list of Appearances and remove any that aren't
   // in callid_contacts.
   {
      UtlHashMapIterator itor(mAppearances);
      UtlString* ss;
      while ((ss = dynamic_cast <UtlString*> (itor())))
      {
         if (!callid_contacts.find(ss))
         {
            Os::Logger::instance().log(FAC_SAA, PRI_DEBUG,
                          "AppearanceGroup::updateSubscriptions terminating subscription for '%s' in mUri = '%s'",
                          ss->data(), mSharedUser.data());

            // Trigger termination of the appearance
            Appearance* inst = dynamic_cast <Appearance*> (itor.value());
            inst->terminate();

            // NOTE:The appearance needs some time to properly terminate the parent subscription
            // so it should not be destroyed here. Move the appearance to the list with terminated
            // appearances to be deleted later when it was completely terminated.
            mAppearances.removeReference(ss);
            mTerminatedAppearances.insert(inst);

            wait_after_subscription_ended = true;
         }
         else
         {
            Os::Logger::instance().log(FAC_SAA, PRI_DEBUG,
                          "AppearanceGroup::updateSubscriptions found subscription for '%s' in mUri = '%s'",
                          ss->data(), mSharedUser.data());
         }
      }
   }

    // Iterate through callid_contacts and add an Appearance for
    // any that aren't in mAppearances.
    // We don't limit the number of additions here, as the size of
    // callid_contacts is guarded by the tests in notifyEventCallback.
    {
        UtlHashBagIterator itor(callid_contacts);
        UtlString* callid_contact;
        long subscription_wait_msec = SUBSCRIPTION_WAIT_MSEC;
        int appearancesCount = 0;
        while ((callid_contact = dynamic_cast <UtlString*> (itor())))
        {
            // If we both terminate subscriptions and create subscriptions,
            // wait a short while to allow the terminations to complete.
            if (!mAppearances.find(callid_contact))
            {
            	if (wait_after_subscription_ended)
            	{
					Os::Logger::instance().log(FAC_SAA, PRI_DEBUG,
								"AppearanceGroup::updateSubscriptions waiting for %d msec",
								subscription_wait_msec);

					OsTime offset(subscription_wait_msec);
					bool ret = mAppearanceGroupSet->addAppearanceByTimer(*callid_contact, this, offset);
					if (ret)
					{
						appearancesCount++;
						subscription_wait_msec += SUBSCRIPTION_WAIT_INCR_MSEC;
					}
            	}
				else
				{
					appearancesCount++;
					addAppearance(callid_contact);
				}
            }
        }

        Os::Logger::instance().log(FAC_SAA, PRI_DEBUG,
                    "AppearanceGroup::updateSubscriptions added '%d' new appearances",
                    appearancesCount);

    }

   // Free callid_contacts.
   callid_contacts.destroyAll();
}


/* ============================ ACCESSORS ================================= */

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ INQUIRY =================================== */

Appearance* AppearanceGroup::findAppearance(UtlString app)
{
   Os::Logger::instance().log(FAC_SAA, PRI_DEBUG,
                 "AppearanceGroup::findAppearance mSharedUser = '%s', looking for '%s'",
                 mSharedUser.data(), app.data());
   Appearance *pApp = NULL;
   UtlHashMapIterator itor(mAppearances);
   UtlString* handle;
   while ( !pApp && (handle = dynamic_cast <UtlString*> (itor())))
   {
      Appearance* ss = dynamic_cast <Appearance*> (itor.value());
      if ( app == ss->getDialogHandle()->data() )
      {
         pApp = ss;
      }
   }
   if (pApp == NULL)
   {
      Os::Logger::instance().log(FAC_SAA, PRI_DEBUG,
                    "AppearanceGroup::findAppearance mSharedUser = '%s', looking for '%s': not found",
                    mSharedUser.data(), app.data());
   }
   return pApp;
}

// Dump the object's internal state.
void AppearanceGroup::dumpState()
{
   // indented 4
   Os::Logger::instance().log(FAC_SAA, PRI_INFO,
                 "\t    AppearanceGroup %p mSharedUser = '%s', mSubscriptionEarlyDialogHandle = '%s'",
                 this, mSharedUser.data(), mSubscriptionEarlyDialogHandle.data());

   UtlHashMapIterator itor3(mAppearances);
   UtlString* handle;
   while ((handle = dynamic_cast <UtlString*> (itor3())))
   {
      Appearance* ss = dynamic_cast <Appearance*> (itor3.value());
      ss->dumpState();
   }
}

/**
 * Get the ContainableType for a UtlContainable-derived class.
 */
UtlContainableType AppearanceGroup::getContainableType() const
{
   return AppearanceGroup::TYPE;
}

void AppearanceGroup::addAppearance(const UtlString* callidContact)
{
    Os::Logger::instance().log(FAC_SAA, PRI_DEBUG,
            "AppearanceGroup::addAppearance this = %p, mSharedUser = '%s' callid;contact = '%s'",
             this, mSharedUser.data(), callidContact->data());

     // Get the contact URI into a UtlString.
     UtlString uri(callidContact->data() +
                   callidContact->index(';') +
                   1);

	// Create the appearance
	Appearance* appearance = new Appearance(getAppearanceAgent(), this,  uri);

	// Add the appearance group to the set.
	mAppearances.insertKeyAndValue(new UtlString(*callidContact),
								   appearance
								   );

	Os::Logger::instance().log(FAC_SAA, PRI_DEBUG,
			"AppearanceGroup::addAppearance "
			"added Appearance for uri = '%s'",
			uri.data() );
}


/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */
