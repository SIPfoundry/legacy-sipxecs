//
//
// Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES

#include "AppearanceAgent.h"
#include "AppAgentTask.h"
#include "AppearanceGroupSet.h"
#include "AppearanceGroup.h"
#include "ResourceListMsg.h"
#include <os/OsSysLog.h>
#include <os/OsEventMsg.h>
#include <os/OsMsg.h>
#include <xmlparser/tinyxml.h>
#include <xmlparser/tinystr.h>
#include <xmlparser/ExtractContent.h>


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// See sipXregistry/doc/service-tokens.txt for assignment of this URI user-part.
static const char dumpStateUri[] = "~~sa~D~dumpstate";

// STATIC VARIABLE INITIALIZATIONS


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
AppearanceAgentTask::AppearanceAgentTask(AppearanceAgent* parent) :
   OsServerTask("AppearanceAgentTask-%d"),
   mAppearanceAgent(parent)
{
   OsSysLog::add(FAC_SAA, PRI_DEBUG,
                 "AppearanceAgentTask:: this = %p, mAppearanceAgent = %p",
                 this, mAppearanceAgent);
}

// Destructor
AppearanceAgentTask::~AppearanceAgentTask()
{
}

/* ============================ MANIPULATORS ============================== */

UtlBoolean AppearanceAgentTask::handleMessage(OsMsg& rMsg)
{
   UtlBoolean handled = FALSE;

   if (rMsg.getMsgType() == RLS_SUBSCRIPTION_MSG)
   {
      // This is a request to refresh a Resource's subscription state.
      OsSysLog::add(FAC_SAA, PRI_DEBUG,
                    "AppearanceAgentTask::handleMessage RLS_SUBSCRIPTION_MSG");
      SubscriptionCallbackMsg* pSubscriptionMsg =
         dynamic_cast <SubscriptionCallbackMsg*> (&rMsg);
         getAppearanceAgent()->getAppearanceGroupSet().
         subscriptionEventCallbackSync(pSubscriptionMsg->getEarlyDialogHandle(),
                                       pSubscriptionMsg->getDialogHandle(),
                                       pSubscriptionMsg->getNewState(),
                                       pSubscriptionMsg->getSubscriptionState());
      handled = TRUE;
   }
   else if (rMsg.getMsgType() == RLS_NOTIFY_MSG)
   {
      // This is a NOTIFY.
      OsSysLog::add(FAC_SAA, PRI_DEBUG,
                    "AppearanceAgentTask::handleMessage RLS_NOTIFY_MSG");
      NotifyCallbackMsg* pNotifyMsg =
         dynamic_cast <NotifyCallbackMsg*> (&rMsg);
         getAppearanceAgent()->getAppearanceGroupSet().
         notifyEventCallbackSync(pNotifyMsg->getDialogHandle(),
                                 pNotifyMsg->getContent());
      handled = TRUE;
   }
   else if (rMsg.getMsgType() == OsMsg::PHONE_APP &&
            rMsg.getMsgSubType() == SipMessage::NET_SIP_MESSAGE)
   {
      // An incoming SIP message.
      const SipMessage* sipMessage = ((SipMessageEvent&) rMsg).getMessage();

      // If this is a NOTIFY request
      if (sipMessage)
      {
         UtlString method;
         sipMessage->getRequestMethod(&method);
         if (method.compareTo(SIP_NOTIFY_METHOD) == 0 &&
             !sipMessage->isResponse())
         {
            // Process the request and send a response.
            handleNotifyRequest(*sipMessage);
            handled = TRUE;
         }
         else if (method.compareTo(SIP_MESSAGE_METHOD) == 0 &&
               !sipMessage->isResponse())
         {
            // Process the request and send a response.
            handleMessageRequest(*sipMessage);
            handled = TRUE;
         }
         else
         {
            OsSysLog::add(FAC_SIP, PRI_ERR,
                          "AppearanceAgentTask::handleMessage unexpected %s %s",
                          method.data(),
                          sipMessage->isResponse() ? "response" : "request");
         }
      }
      else
      {
         OsSysLog::add(FAC_SIP, PRI_ERR,
                       "AppearanceAgentTask::handleMessage  SipMessageEvent with NULL SipMessage");
      }
   }
   else if (rMsg.getMsgType() == OsMsg::OS_SHUTDOWN)
   {
      // Leave 'handled' false and pass on to OsServerTask::handleMessage.
   }
   else
   {
      OsSysLog::add(FAC_SAA, PRI_ERR,
                    "AppearanceAgentTask::handleMessage unknown msg type %d subtype %d",
                    rMsg.getMsgType(), rMsg.getMsgSubType());
   }

   return handled;
}


// Process a NOTIFY request
void AppearanceAgentTask::handleNotifyRequest(const SipMessage& msg)
{
   UtlString eventType;
   msg.getEventField(eventType);
   if (eventType == SLA_EVENT_TYPE)
   {
      UtlString sharedUri;
      msg.getToUri(&sharedUri);
      OsSysLog::add(FAC_SAA, PRI_DEBUG, "AppearanceAgentTask::handleNotifyRequest for '%s'", sharedUri.data());

      // send it to the AppearanceGroup matching
      AppearanceGroup* appearanceGroup =
         getAppearanceAgent()->getAppearanceGroupSet().findAppearanceGroup(sharedUri);
      if ( appearanceGroup )
      {
         appearanceGroup->handleNotifyRequest(msg);
      }
      else
      {
         // This happens on shutdown, since it is correct for the UA to send a NOTIFY
         // following the SUBSCRIBE with Expires: 0.  However, nothing is left to handle it.
         // Construct the response.
         SipMessage response;
         OsSysLog::add(FAC_SAA, PRI_ERR, "AppearanceAgentTask::handleNotifyRequest for unknown group '%s'", sharedUri.data());
         response.setInterfaceIpPort(msg.getInterfaceIp(), msg.getInterfacePort());
         response.setResponseData(&msg, SIP_NOT_FOUND_CODE, SIP_NOT_FOUND_TEXT);
         getAppearanceAgent()->getServerUserAgent().send(response);
      }
   }
}

// Process a MESSAGE request, which is used to trigger debugging actions.
void AppearanceAgentTask::handleMessageRequest(const SipMessage& msg)
{
   // Extract the user-part of the request-URI, which should tell us what
   // to do.
   UtlString user;
   msg.getUri(NULL, NULL, NULL, &user);

   // Construct the response.
   SipMessage response;

   if (user.compareTo(dumpStateUri) == 0)
   {
      // dumpStateUri is used to request to dump the Appearance Agent state into the log.
      debugDumpState(msg);
      response.setOkResponseData(&msg, NULL);
   }
   else
   {
      response.setInterfaceIpPort(msg.getInterfaceIp(), msg.getInterfacePort());
      response.setResponseData(&msg, SIP_NOT_FOUND_CODE, SIP_NOT_FOUND_TEXT);
   }

   // Send the response.
   getAppearanceAgent()->getServerUserAgent().send(response);
}


// Dump the state of the Appearance Agent into the log.
void AppearanceAgentTask::debugDumpState(const SipMessage& msg)
{
   // Get the 'id' URI parameter off the request-URI.
   UtlString request_string;
   msg.getRequestUri(&request_string);
   Url request_uri(request_string, TRUE);
   UtlString id;
   request_uri.getUrlParameter("id", id);
   // 'id' is empty string if no 'id' URI parameter.

   OsSysLog::add(FAC_SAA, PRI_INFO,
                 "AppearanceAgentTask::debugDumpState called, id = '%s':",
                 id.data());
   getAppearanceAgent()->dumpState();
   OsSysLog::add(FAC_SAA, PRI_INFO,
                 "AppearanceAgentTask::debugDumpState finished");
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */
