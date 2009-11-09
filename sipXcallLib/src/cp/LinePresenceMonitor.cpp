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
#include <utl/UtlSListIterator.h>
#include <net/SipRefreshManager.h>
#include <net/SipSubscribeClient.h>
#include <net/XmlRpcRequest.h>
#include <net/SipPresenceEvent.h>
#include <net/NetMd5Codec.h>
#include <cp/LinePresenceMonitor.h>
#include <cp/LinePresenceMonitorMsg.h>
#include <os/OsLock.h>
#include <os/OsEvent.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define DEFAULT_REFRESH_INTERVAL      300

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
LinePresenceMonitor::LinePresenceMonitor(int userAgentPort,
                                         UtlString& domainName,
                                         UtlString& groupName,
                                         bool local,
                                         Url& remoteServerUrl,
                                         Url& presenceServerUrl)
   : OsServerTask("LinePresenceMonitor-%d"),
     mpDialogMonitor(NULL),
     mLock(OsBSem::Q_PRIORITY, OsBSem::FULL)
{
   // Create a SIP user agent using the specified port and start it up.
   mpUserAgent = new SipUserAgent(userAgentPort, userAgentPort);
   mpUserAgent->start();

   mGroupName = groupName;
   mLocal = local;
   mDomainName = domainName;

   if (mLocal)
   {
      // Create a local Sip Dialog Monitor
      mpDialogMonitor = new SipDialogMonitor(mpUserAgent,
                                             domainName,
                                             userAgentPort,
                                             DEFAULT_REFRESH_INTERVAL,
                                             false);

      // Add self to the dialog monitor for state change notifications.
      mpDialogMonitor->addStateChangeNotifier("Line_Presence_Monitor", this);

      // Verify that a presence server URI has been specified
      presenceServerUrl.getHostAddress(mPresenceServer);
      if (mPresenceServer != NULL)
      {
         presenceServerUrl.getIdentity(mPresenceServer);
      }
   }
   else
   {
      mRemoteServer = remoteServerUrl;
   }

   // Create the SIP Subscribe Client for subscribing to both dialog
   // events and presence events.
   mpRefreshMgr = new SipRefreshManager(*mpUserAgent, mDialogManager);
   mpRefreshMgr->start();

   mpSipSubscribeClient = new SipSubscribeClient(*mpUserAgent, mDialogManager,
                                                 *mpRefreshMgr);
   mpSipSubscribeClient->start();

   UtlString localAddress;
   OsSocket::getHostIp(&localAddress);

   Url url(localAddress);
   url.setHostPort(userAgentPort);
   url.includeAngleBrackets();
   mContact = url.toString();
}


// Destructor
LinePresenceMonitor::~LinePresenceMonitor()
{
   OsLock lock(mLock);

   // Delete the SipSubscribeClient.
   if (mpSipSubscribeClient)
   {
      mpSipSubscribeClient->endAllSubscriptions();
      delete mpSipSubscribeClient;
   }

   // Delete the SipRefreshManager.
   if (mpRefreshMgr)
   {
      delete mpRefreshMgr;
   }

   // Delete the SipDialogMonitor.
   if (mpDialogMonitor)
   {
      // Remove itself to the dialog monitor
      mpDialogMonitor->removeStateChangeNotifier("Line_Presence_Monitor");

      delete mpDialogMonitor;
   }

   // Free the subscription lists.
   mDialogSubscribeList.destroyAll();
   mPresenceSubscribeList.destroyAll();

   // Shut down the SipUserAgent
   mpUserAgent->shutdown(FALSE);

   while(!mpUserAgent->isShutdownDone())
   {
      /* null */ ;
   }

   // Delete the SipUserAgent.
   delete mpUserAgent;
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

bool LinePresenceMonitor::setStatus(const Url& aor,
                                    const StateChangeNotifier::Status value)
{
   // Copy the contact from the AOR into a string that setStatusMessage will delete.
   // We can only use userId to identify the line
   UtlString* s = new UtlString;
   aor.getUserId(*s);

   LinePresenceMonitorMsg setStatusMessage(s, value);
   postMessage(setStatusMessage);

   return true;
}

OsStatus LinePresenceMonitor::subscribeDialog(LinePresenceBase* line)
{
   OsStatus result = OS_FAILED;

   LinePresenceMonitorMsg subscribeDialogMsg(LinePresenceMonitorMsg::SUBSCRIBE_DIALOG, line);
   postMessage(subscribeDialogMsg);

   return result;
}

OsStatus LinePresenceMonitor::unsubscribeDialog(LinePresenceBase* line, OsEvent* e)
{
   OsStatus result = OS_FAILED;

   LinePresenceMonitorMsg unsubscribeDialogMsg(LinePresenceMonitorMsg::UNSUBSCRIBE_DIALOG,
                                               line, e);
   postMessage(unsubscribeDialogMsg);

   return result;
}

OsStatus LinePresenceMonitor::subscribeDialog(UtlSList& list)
{
   OsStatus result = OS_FAILED;

   UtlSListIterator iterator(list);
   LinePresenceBase* line;
   while ((line = dynamic_cast <LinePresenceBase *> (iterator())) != NULL)
   {
      subscribeDialog(line);
   }

   return result;
}

OsStatus LinePresenceMonitor::unsubscribeDialog(UtlSList& list, OsEvent* e)
{
   OsStatus result = OS_FAILED;

   UtlSListIterator iterator(list);
   LinePresenceBase* line;
   int n = list.entries() ;
   while ((line = dynamic_cast <LinePresenceBase *> (iterator())) != NULL)
   {
      unsubscribeDialog(line, (n-- == 1) ? e: NULL); // last message gets the event
   }

   return result;
}

OsStatus LinePresenceMonitor::subscribePresence(LinePresenceBase* line)
{
   OsStatus result = OS_FAILED;

   LinePresenceMonitorMsg subscribePresenceMsg(LinePresenceMonitorMsg::SUBSCRIBE_PRESENCE, line);
   postMessage(subscribePresenceMsg);

   return result;
}

OsStatus LinePresenceMonitor::unsubscribePresence(LinePresenceBase* line, OsEvent* e)
{
   OsStatus result = OS_FAILED;

   LinePresenceMonitorMsg unsubscribePresenceMsg(LinePresenceMonitorMsg::UNSUBSCRIBE_PRESENCE,
                                                 line, e);
   postMessage(unsubscribePresenceMsg);

   return result;
}

OsStatus LinePresenceMonitor::subscribePresence(UtlSList& list)
{
   OsStatus result = OS_FAILED;

   UtlSListIterator iterator(list);
   LinePresenceBase* line;
   while ((line = dynamic_cast <LinePresenceBase *> (iterator())) != NULL)
   {
      subscribePresence(line);
   }

   return result;
}

OsStatus LinePresenceMonitor::unsubscribePresence(UtlSList& list, OsEvent* e)
{
   OsStatus result = OS_FAILED;

   UtlSListIterator iterator(list);
   LinePresenceBase* line;
   int n = list.entries() ;
   while ((line = dynamic_cast <LinePresenceBase *> (iterator())) != NULL)
   {
      unsubscribePresence(line, (n-- == 1) ? e: NULL);   // last message gets the event
   }

   return result;
}


void LinePresenceMonitor::subscriptionStateCallback(SipSubscribeClient::SubscriptionState newState,
                                                    const char* earlyDialogHandle,
                                                    const char* dialogHandle,
                                                    void* applicationData,
                                                    int responseCode,
                                                    const char* responseText,
                                                    long expiration,
                                                    const SipMessage* subscribeResponse)
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "LinePresenceMonitor::subscriptionStateCallback is called with responseCode = %d (%s)",
                 responseCode, responseText);
}


bool LinePresenceMonitor::notifyEventCallback(const char* earlyDialogHandle,
                                              const char* dialogHandle,
                                              void* applicationData,
                                              const SipMessage* notifyRequest)
{
   // Receive the notification and process the message
   LinePresenceMonitor* pThis = (LinePresenceMonitor *) applicationData;

   pThis->handleNotifyMessage(notifyRequest);
   return true;
}


void LinePresenceMonitor::handleNotifyMessage(const SipMessage* notifyMessage)
{
   // This function does not hold mLock, as it does not manipulate the
   // instance variables itself.  All manipulation is done via setStatus.

   Url fromUrl;
   notifyMessage->getFromUrl(fromUrl);
   UtlString contact;

   fromUrl.getUserId(contact);

   contact += mPresenceServer;
   // Make the contact be a proper URI by prepending "sip:".
   // This matches what the presence server sends.
   contact.prepend("sip:");


   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "LinePresenceMonitor::handleNotifyMessage receiving a notify message from %s",
                 contact.data());

   const HttpBody* notifyBody = notifyMessage->getBody();

   if (notifyBody)
   {
      UtlString messageContent;
      ssize_t bodyLength;

      notifyBody->getBytes(&messageContent, &bodyLength);

      // Parse the content and store it in a SipPresenceEvent object
      SipPresenceEvent sipPresenceEvent(contact, messageContent);

      if (sipPresenceEvent.getTuples().entries() == 1)
      {
         Tuple* tuple = sipPresenceEvent.getTuple();
         UtlString status;
         tuple->getStatus(status);
         Url contactUrl(contact);

         // Call setStatus() to do the updating.
         setStatus(contactUrl,
                   status.compareTo(STATUS_CLOSED) == 0 ?
                   StateChangeNotifier::SIGN_OUT :
                   StateChangeNotifier::SIGN_IN);

         OsSysLog::add(FAC_SIP, PRI_WARNING,
                       "LinePresenceMonitor::handleNotifyMessage presence processed, contact '%s' set to '%s'",
                       contact.data(),
                       status.data());
      }
      else
      {
         OsSysLog::add(FAC_SIP, PRI_WARNING,
                       "LinePresenceMonitor::handleNotifyMessage presence event contains %d events: '%s'",
                       sipPresenceEvent.getTuples().entries(),
                       messageContent.data());
      }
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_WARNING,
                    "LinePresenceMonitor::handleNotifyMessage received an empty notify body from '%s'",
                    contact.data());
   }
}

UtlBoolean LinePresenceMonitor::isOk() const
{
   UtlBoolean bOk = false;

   if (mpUserAgent)
   {
         bOk = mpUserAgent->isOk();
   }

   return bOk ;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
UtlBoolean LinePresenceMonitor::handleMessage(OsMsg& rMessage)
{
   LinePresenceMonitorMsg*    pMessage;
   LinePresenceBase*          pLine;

   OsSysLog::add(FAC_SIP, PRI_DEBUG,
      "LinePresenceMonitor::handleMessage - MsgType: %d, MsgSubType: %d\n",
         rMessage.getMsgType(), rMessage.getMsgSubType());

   if (rMessage.getMsgType() == OsMsg::USER_START)
   {
      // Seize the lock to process this message.
      OsLock lock(mLock);

      switch (rMessage.getMsgSubType())
      {
         case LinePresenceMonitorMsg::SUBSCRIBE_DIALOG:
            pMessage = (LinePresenceMonitorMsg*)&rMessage;
            pLine     = pMessage->getLine();

            subscribeDialogMessage(pLine);
            break;

         case LinePresenceMonitorMsg::UNSUBSCRIBE_DIALOG:
         {
            pMessage = (LinePresenceMonitorMsg*)&rMessage;
            unsubscribeDialogMessage(pMessage->getLine()) ;
            OsEvent *event = pMessage->getEvent() ;
            if (event)
            {
               event->signal(0); // Signal the unsubscribe is complete.
            }
            break;
         }

         case LinePresenceMonitorMsg::SUBSCRIBE_PRESENCE:
            pMessage = (LinePresenceMonitorMsg*)&rMessage;
            pLine     = pMessage->getLine();

            subscribePresenceMessage(pLine);
            break;

         case LinePresenceMonitorMsg::UNSUBSCRIBE_PRESENCE:
         {
            pMessage = (LinePresenceMonitorMsg*)&rMessage;
            unsubscribePresenceMessage(pMessage->getLine()) ;
            OsEvent *event = pMessage->getEvent() ;
            if (event)
            {
               event->signal(0); // Signal the unsubscribe is complete.
            }
            break;
         }

         case LinePresenceMonitorMsg::SET_STATUS:
            pMessage = (LinePresenceMonitorMsg*)&rMessage;
            setStatusMessage(pMessage->getContact(),
                             pMessage->getStateChange());
            break;

         default:
            // Bad message
            OsSysLog::add(FAC_SIP, PRI_ERR, "LinePresenceMonitor::handleMessage - Received bad message");

            break;
      }

      return true;
   }
   else
   {
      // Otherwise, pass the message to the base for processing.
      return false;
   }
}

OsStatus LinePresenceMonitor::subscribeDialogMessage(LinePresenceBase* line)
{
   OsStatus result = OS_FAILED;

   Url* lineUrl = line->getUri();

   OsSysLog::add(FAC_SIP, PRI_DEBUG, "LinePresenceMonitor::subscribeDialogMessage subscribing dialog for line %s",
                 lineUrl->toString().data());

   if (mLocal)
   {
      result =
         mpDialogMonitor->addExtension(mGroupName, *lineUrl) ?
         OS_SUCCESS :
         OS_FAILED;
   }
   else
   {
      // Use XML-RPC to communicate with the sipX dialog monitor
      XmlRpcRequest request(mRemoteServer, "addExtension");

      request.addParam(&mGroupName);
      UtlString contact = lineUrl->toString();
      request.addParam(&contact);

      XmlRpcResponse response;
      result = request.execute(response) ? OS_SUCCESS : OS_FAILED;
   }

   // Insert the line to the Subscribe Map
   UtlString contactId;
   lineUrl->getUserId(contactId);
   mDialogSubscribeList.insertKeyAndValue(new UtlString(contactId),
                                          new UtlVoidPtr(line));

   return result;
}

OsStatus LinePresenceMonitor::unsubscribeDialogMessage(LinePresenceBase* line)
{
   OsStatus result = OS_FAILED;

   // Get the URI of the line into local memory.
   Url* p = line->getUri();
   if (p == NULL)
   {
      return OS_FAILED;
   }
   Url lineUrl(*p);

   // Remove it from the dialog subscription list.
   UtlString contact;
   lineUrl.getUserId(contact);
   mDialogSubscribeList.destroy(&contact);

   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "LinePresenceMonitor::unsubscribeDialogMessage unsubscribing dialog for line %s",
                 lineUrl.toString().data());

   if (mLocal)
   {
      result =
         mpDialogMonitor->removeExtension(mGroupName, lineUrl) ?
         OS_SUCCESS :
         OS_FAILED;
   }
   else
   {
      // Use XML-RPC to communicate with the sipX dialog monitor
      XmlRpcRequest request(mRemoteServer, "removeExtension");

      request.addParam(&mGroupName);
      UtlString contact = lineUrl.toString();
      request.addParam(&contact);

      XmlRpcResponse response;
      result = request.execute(response) ? OS_SUCCESS : OS_FAILED;
   }

   return result;
}

OsStatus LinePresenceMonitor::subscribePresenceMessage(LinePresenceBase* line)
{
   OsStatus result = OS_FAILED;

   Url* lineUrl = line->getUri();

   OsSysLog::add(FAC_SIP, PRI_DEBUG, "LinePresenceMonitor::subscribePresenceMessage subscribing presence for line %s",
                 lineUrl->toString().data());

   // Send out the SUBSCRIBE to the presence server
   UtlString contactId, resourceId;
   lineUrl->getUserId(contactId);
   if (!mPresenceServer.isNull())
   {
      resourceId = contactId + mPresenceServer;
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "LinePresenceMonitor::subscribePresenceMessage Sending out the SUBSCRIBE to contact %s",
                    resourceId.data());

      // Delay for 2 seconds because proxy might not be ready yet
      OsTask::delay(2000);

      UtlString toUrl;
      lineUrl->toString(toUrl);

      UtlString fromUri = "linePresenceMonitor@" + mDomainName;
      UtlString dialogHandle;

      UtlBoolean status = mpSipSubscribeClient->addSubscription(resourceId.data(),
                                                                PRESENCE_EVENT_TYPE,
                                                                PRESENCE_EVENT_CONTENT_TYPE,
                                                                fromUri.data(),
                                                                toUrl.data(),
                                                                mContact.data(),
                                                                DEFAULT_REFRESH_INTERVAL,
                                                                (void *) this,
                                                                LinePresenceMonitor::subscriptionStateCallback,
                                                                LinePresenceMonitor::notifyEventCallback,
                                                                dialogHandle);

      if (!status)
      {
         OsSysLog::add(FAC_SIP, PRI_ERR,
                       "LinePresenceMonitor::subscribePresenceMessage Subscription failed to contact %s.",
                       resourceId.data());
      }
      else
      {
         mDialogHandleList.insertKeyAndValue(new UtlString(contactId), new UtlString(dialogHandle));
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
            "LinePresenceMonitor::subscribePresenceMessage subscribed contact %s dialogHandle %s",
               contactId.data(), dialogHandle.data());

      }
   }

   // Insert the line to the Subscribe Map
   mPresenceSubscribeList.insertKeyAndValue(new UtlString(contactId),
                                            new UtlVoidPtr(line));

   return result;
}

OsStatus LinePresenceMonitor::unsubscribePresenceMessage(LinePresenceBase* line)
{
   OsStatus result = OS_FAILED;

   // Get the URI of the line into local memory.
   Url* p = line->getUri();
   if (p == NULL)
   {
      return OS_FAILED;
   }
   Url lineUrl(*p);

   // Get the contact string.
   UtlString contact;
   lineUrl.getUserId(contact);

   UtlString* dialogHandle = NULL;
   if (!mPresenceServer.isNull())
   {
      dialogHandle = dynamic_cast <UtlString *> (mDialogHandleList.findValue(&contact));
   }

   // Remove it from the presence subscription list.
   mPresenceSubscribeList.destroy(&contact);


   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "LinePresenceMonitor::unsubscribePresenceMessage unsubscribing presence for line %s",
                 lineUrl.toString().data());

   if (dialogHandle != NULL)
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
         "LinePresenceMonitor::unsubscribePresenceMessage unsubscribing contact %s dialogHandle %s",
            contact.data(), dialogHandle->data());

      UtlBoolean status =
         mpSipSubscribeClient->endSubscriptionGroup(dialogHandle->data());

      if (!status)
      {
         OsSysLog::add(FAC_SIP, PRI_ERR,
                       "LinePresenceMonitor::unsubscribePresenceMessage Unsubscription failed for %s.",
                       contact.data());
      }
      mDialogHandleList.destroy(&contact);
   }

   return result;
}

OsStatus LinePresenceMonitor::setStatusMessage(const UtlString* contact,
                                               const StateChangeNotifier::Status value)
{
   // Our caller has seized mLock.

   OsStatus result = OS_FAILED;

   OsSysLog::add(FAC_SIP, PRI_DEBUG, "LinePresenceMonitor::setStatusMessage set the value %d for %s",
                 value, contact->data());

   if (value == StateChangeNotifier::ON_HOOK ||
       value == StateChangeNotifier::OFF_HOOK ||
       value == StateChangeNotifier::RINGING)
   {
      // Set the dialog status
      UtlVoidPtr* container = dynamic_cast <UtlVoidPtr *> (mDialogSubscribeList.findValue(contact));
      if (container != NULL)
      {
         LinePresenceBase* line = (LinePresenceBase *) (container->getValue());
         // Set the state value in LinePresenceBase
         switch (value)
         {
         case StateChangeNotifier::ON_HOOK:
            if (!line->getState(LinePresenceBase::ON_HOOK))
            {
               line->updateState(LinePresenceBase::ON_HOOK, true);
               result = OS_SUCCESS;
            }

            break;

         case StateChangeNotifier::OFF_HOOK:
            if (line->getState(LinePresenceBase::ON_HOOK))
            {
               line->updateState(LinePresenceBase::ON_HOOK, false);
               result = OS_SUCCESS;
            }

            break;

         case StateChangeNotifier::RINGING:
            if (line->getState(LinePresenceBase::ON_HOOK))
            {
               line->updateState(LinePresenceBase::ON_HOOK, false);
               result = OS_SUCCESS;
            }

            break;

         // To prevent warnings.
         default:
            break;
         }
      }
   }

   if (value == StateChangeNotifier::SIGN_IN ||
       value == StateChangeNotifier::SIGN_OUT)
   {
      // Set the presence status
      UtlVoidPtr* container = dynamic_cast <UtlVoidPtr *> (mPresenceSubscribeList.findValue(contact));
      if (container != NULL)
      {
         LinePresenceBase* line = (LinePresenceBase *) (container->getValue());
         // Set the state value in LinePresenceBase
         switch (value)
         {
         case StateChangeNotifier::SIGN_IN:
            if (!line->getState(LinePresenceBase::SIGNED_IN))
            {
               line->updateState(LinePresenceBase::SIGNED_IN, true);
               result = OS_SUCCESS;
            }

            break;

         case StateChangeNotifier::SIGN_OUT:
            if (line->getState(LinePresenceBase::SIGNED_IN))
            {
               line->updateState(LinePresenceBase::SIGNED_IN, false);
               result = OS_SUCCESS;
            }

            break;

         // To prevent warnings.
         default:
            break;
         }
      }
   }

   // Free the string.
   delete contact;

   return result;
}

/* ============================ FUNCTIONS ================================= */
