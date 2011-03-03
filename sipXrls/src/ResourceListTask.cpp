//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////////

// Define VALGRIND_LEAK_CHECK_ON_DEBUG to cause ::debugDumpState() to call
// VALGRIND_DO_LEAK_CHECK in order to report memory leaks.
//#define VALGRIND_LEAK_CHECK_ON_DEBUG

// SYSTEM INCLUDES

#ifdef VALGRIND_LEAK_CHECK_ON_DEBUG
#include <valgrind/valgrind.h>
#include <valgrind/memcheck.h>
#endif

// APPLICATION INCLUDES

#include "ResourceListServer.h"
#include "ResourceListTask.h"
#include "ResourceListSet.h"
#include "ResourceListMsg.h"
#include <os/OsSysLog.h>
#include <os/OsEventMsg.h>
#include <os/OsMsg.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// See sipXregistry/doc/service-tokens.txt for assignment of this URI user-part.
static const char dumpStateUri[] = "~~rl~D~dumpstate";

// STATIC VARIABLE INITIALIZATIONS


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
ResourceListTask::ResourceListTask(ResourceListServer* parent) :
   OsServerTask("ResourceListTask-%d"),
   mResourceListServer(parent)
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceListTask:: this = %p, mResourceListServer = %p",
                 this, mResourceListServer);
}

// Destructor
ResourceListTask::~ResourceListTask()
{
}

/* ============================ MANIPULATORS ============================== */

UtlBoolean ResourceListTask::handleMessage(OsMsg& rMsg)
{
   UtlBoolean handled = FALSE;

   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceListTask::handleMessage message type %d subtype %d",
                 rMsg.getMsgType(), rMsg.getMsgSubType());

   if (rMsg.getMsgType() == OsMsg::OS_EVENT &&
       rMsg.getMsgSubType() == OsEventMsg::NOTIFY)
   {
      // An event notification.

      // The userdata of the original OsQueuedEvent was copied by
      // OsQueuedEvent::signal() into the userdata of this OsEventMsg.
      // The userdata is an "enum notifyCodes" value indicating what sort of
      // operation is to be done, plus a sequence number indicating (via
      // ResourceListSet) the object to be operated upon.
      intptr_t userData;
      OsEventMsg* pEventMsg = dynamic_cast <OsEventMsg*> (&rMsg);
      pEventMsg->getUserData((void*&)userData);
      int seqNo;
      enum ResourceListSet::notifyCodes type;
      ResourceListSet::splitUserData(userData, seqNo, type);

      switch (type)
      {
      case ResourceListSet::PUBLISH_TIMEOUT:
      {
         // This is a request to publish lists that have changed.
         OsSysLog::add(FAC_RLS, PRI_DEBUG,
                       "ResourceListTask::handleMessage PUBLISH_TIMEOUT");

         // Don't publish if this is a gap timeout with no new RLMI content.
         if (getResourceListServer()->getResourceListSet().publishOnTimeout())
         {
            getResourceListServer()->getResourceListSet().publish();

            // Allow a longer gap timer before the next timer we publish.
            getResourceListServer()->getResourceListSet().startGapTimeout();
         }

         handled = TRUE;
         break;
      }

      default:
         OsSysLog::add(FAC_RLS, PRI_ERR,
                       "ResourceListTask::handleMessage unknown event type %d, userData = %" PRIdPTR,
                       type, userData);
         break;
      }
   }
   else if (rMsg.getMsgType() == RLS_SUBSCRIPTION_MSG)
   {
      // This is a subscription event.
      OsSysLog::add(FAC_RLS, PRI_DEBUG,
                    "ResourceListTask::handleMessage RLS_SUBSCRIPTION_MSG");
      SubscriptionCallbackMsg* pSubscriptionMsg =
         dynamic_cast <SubscriptionCallbackMsg*> (&rMsg);
      getResourceListServer()->getResourceListSet().
         subscriptionEventCallbackSync(pSubscriptionMsg->getEarlyDialogHandle(),
                                       pSubscriptionMsg->getDialogHandle(),
                                       pSubscriptionMsg->getNewState(),
                                       pSubscriptionMsg->getSubscriptionState());
      handled = TRUE;
   }
   else if (rMsg.getMsgType() == RLS_NOTIFY_MSG)
   {
      // This is a NOTIFY.
      OsSysLog::add(FAC_RLS, PRI_DEBUG,
                    "ResourceListTask::handleMessage RLS_NOTIFY_MSG");
      NotifyCallbackMsg* pNotifyMsg =
         dynamic_cast <NotifyCallbackMsg*> (&rMsg);
      getResourceListServer()->getResourceListSet().
         notifyEventCallbackSync(pNotifyMsg->getDialogHandle(),
                                 pNotifyMsg->getContent());
      handled = TRUE;
   }
   else if (rMsg.getMsgType() == OsMsg::PHONE_APP &&
            rMsg.getMsgSubType() == SipMessage::NET_SIP_MESSAGE)
   {
      // An incoming SIP message.
      const SipMessage* sipMessage = ((SipMessageEvent&) rMsg).getMessage();

      // If this is a MESSAGE request
      if (sipMessage)
      {
         UtlString method;
         sipMessage->getRequestMethod(&method);
         if (method.compareTo(SIP_MESSAGE_METHOD) == 0 &&
             !sipMessage->isResponse())
         {
            // Process the request and send a response.
            handleMessageRequest(*sipMessage);
            handled = TRUE;
         }
         else
         {
            OsSysLog::add(FAC_SIP, PRI_ERR,
                          "ResourceListTask::handleMessage unexpected %s %s",
                          method.data(),
                          sipMessage->isResponse() ? "response" : "request");
         }
      }
      else
      {
         OsSysLog::add(FAC_SIP, PRI_ERR,
                       "SipSubscribeClient::handleMessage  SipMessageEvent with NULL SipMessage");
      }
   }
   else if (rMsg.getMsgType() == OsMsg::OS_SHUTDOWN)
   {
      // Leave 'handled' false and pass on to OsServerTask::handleMessage.
   }
   else
   {
      OsSysLog::add(FAC_RLS, PRI_ERR,
                    "ResourceListTask::handleMessage unknown msg type %d",
                    rMsg.getMsgType());
   }

   return handled;
}

// Process a MESSAGE request, which is used to trigger debugging actions.
void ResourceListTask::handleMessageRequest(const SipMessage& msg)
{
   // Extract the user-part of the request-URI, which should tell us what
   // to do.
   UtlString user;
   msg.getUri(NULL, NULL, NULL, &user);

   // Construct the response.
   SipMessage response;

   if (user.compareTo(dumpStateUri) == 0)
   {
      // dumpStateUri is used to request to dump the RLS state into the log.
      debugDumpState(msg);
      response.setOkResponseData(&msg, NULL);
   }
   else
   {
      response.setInterfaceIpPort(msg.getInterfaceIp(), msg.getInterfacePort());
      response.setResponseData(&msg, SIP_NOT_FOUND_CODE, SIP_NOT_FOUND_TEXT);
   }

   // Send the response.
   getResourceListServer()->getServerUserAgent().send(response);
}

// Dump the state of the RLS into the log.
void ResourceListTask::debugDumpState(const SipMessage& msg)
{
   #ifdef VALGRIND_LEAK_CHECK_ON_DEBUG
   VALGRIND_DO_LEAK_CHECK;
   #endif

   // Get the 'id' URI parameter off the request-URI.
   UtlString request_string;
   msg.getRequestUri(&request_string);
   Url request_uri(request_string, TRUE);
   UtlString id;
   request_uri.getUrlParameter("id", id);
   // 'id' is empty string if no 'id' URI parameter.

   OsSysLog::add(FAC_RLS, PRI_INFO,
                 "ResourceListTask::debugDumpState called, id = '%s':",
                 id.data());
   getResourceListServer()->dumpState();
   OsSysLog::add(FAC_RLS, PRI_INFO,
                 "ResourceListTask::debugDumpState finished");
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */
