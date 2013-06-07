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

#include "net/SipMessage.h"
#include "os/OsLogger.h"
#include "AppAgentTask.h"
#include "AppearanceAgent.h"
#include "ResourceListMsg.h"
#include "AppearanceGroup.h"

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
   Os::Logger::instance().log(FAC_SAA, PRI_DEBUG,
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
  std::string errorString;

  try
  {
   UtlBoolean handled = FALSE;

   if (rMsg.getMsgType() == SAA_SUBSCRIPTION_MSG)
   {
      // This is a request to refresh a Resource's subscription state.
      Os::Logger::instance().log(FAC_SAA, PRI_DEBUG,
                    "AppearanceAgentTask::handleMessage SAA_SUBSCRIPTION_MSG");
      SubscriptionCallbackMsg* pSubscriptionMsg =
         dynamic_cast <SubscriptionCallbackMsg*> (&rMsg);
         getAppearanceAgent()->getAppearanceGroupSet().
         subscriptionEventCallbackSync(pSubscriptionMsg->getEarlyDialogHandle(),
                                       pSubscriptionMsg->getDialogHandle(),
                                       pSubscriptionMsg->getNewState(),
                                       pSubscriptionMsg->getSubscriptionState());
      handled = TRUE;
   }
   else if (rMsg.getMsgType() == SAA_NOTIFY_MSG)
   {
      // This is a NOTIFY.
      Os::Logger::instance().log(FAC_SAA, PRI_DEBUG,
                    "AppearanceAgentTask::handleMessage SAA_NOTIFY_MSG");
      NotifyCallbackMsg* pNotifyMsg =
         dynamic_cast <NotifyCallbackMsg*> (&rMsg);
         getAppearanceAgent()->getAppearanceGroupSet().
         notifyEventCallbackSync(pNotifyMsg->getDialogHandle(),
                                 pNotifyMsg->getMsg());
      handled = TRUE;
   }
   else if (SAA_APPEARANCE_MSG == rMsg.getMsgType())
   {
       // This is a create new subscription request
       Os::Logger::instance().log(FAC_SAA, PRI_DEBUG,
                     "AppearanceAgentTask::handleMessage SAA_APPEARANCE_MSG");

       AppearanceMsg* pCSMsg = dynamic_cast <AppearanceMsg*> (&rMsg);
       AppearanceGroup *appearanceGroup = static_cast<AppearanceGroup*> (pCSMsg->getHandler());

       appearanceGroup->addAppearance(pCSMsg->getCallidContact());
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
         if (method.compareTo(SIP_MESSAGE_METHOD) == 0 &&
               !sipMessage->isResponse())
         {
            // Process the request and send a response.
            handleMessageRequest(*sipMessage);
            handled = TRUE;
         }
         else
         {
            Os::Logger::instance().log(FAC_SIP, PRI_ERR,
                          "AppearanceAgentTask::handleMessage unexpected %s %s",
                          method.data(),
                          sipMessage->isResponse() ? "response" : "request");
         }
      }
      else
      {
         Os::Logger::instance().log(FAC_SIP, PRI_ERR,
                       "AppearanceAgentTask::handleMessage  SipMessageEvent with NULL SipMessage");
      }
   }
   else if (rMsg.getMsgType() == OsMsg::OS_SHUTDOWN)
   {
      // Leave 'handled' false and pass on to OsServerTask::handleMessage.
   }
   else
   {
      Os::Logger::instance().log(FAC_SAA, PRI_ERR,
                    "AppearanceAgentTask::handleMessage unknown msg type %d subtype %d",
                    rMsg.getMsgType(), rMsg.getMsgSubType());
   }

   return handled;
  }

#ifdef MONGO_assert
  catch (mongo::DBException& e)
  {
    errorString = "RLS - Mongo DB Exception";
    OS_LOG_ERROR( FAC_SIP, "ResourceListTask::handleMessage() Exception: "
             << e.what() );
  }
#endif
  catch (boost::exception& e)
  {
    errorString = "RLS - Boost Library Exception";
    OS_LOG_ERROR( FAC_SIP, "ResourceListTask::handleMessage() Exception: "
             << boost::diagnostic_information(e));
  }
  catch (std::exception& e)
  {
    errorString = "RLS - Standard Library Exception";
    OS_LOG_ERROR( FAC_SIP, "ResourceListTask::handleMessage() Exception: "
             << e.what() );
  }
  catch (...)
  {
    errorString = "RLS - Unknown Exception";
    OS_LOG_ERROR( FAC_SIP, "ResourceListTask::handleMessage() Exception: Unknown Exception");
  }

  //
  // If it ever get here, that means we caught an exception
  //
  if (rMsg.getMsgType()  == OsMsg::PHONE_APP)
  {
    const SipMessage& message = *((SipMessageEvent&)rMsg).getMessage();
    if (!message.isResponse())
    {
      SipMessage finalResponse;
      finalResponse.setResponseData(&message, SIP_5XX_CLASS_CODE, errorString.c_str());
      getAppearanceAgent()->getServerUserAgent().send(finalResponse);
    }
  }

  return(TRUE);
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

   Os::Logger::instance().log(FAC_SAA, PRI_INFO,
                 "AppearanceAgentTask::debugDumpState called, id = '%s':",
                 id.data());
   getAppearanceAgent()->dumpState();
   Os::Logger::instance().log(FAC_SAA, PRI_INFO,
                 "AppearanceAgentTask::debugDumpState finished");
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */
