//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <os/OsMsg.h>
#include <os/OsEventMsg.h>
#include <utl/UtlHashBagIterator.h>
#include <net/SipSubscribeServer.h>
#include <net/SipUserAgent.h>
#include <net/SipPublishContentMgr.h>
#include <net/SipSubscriptionMgr.h>
#include <net/SipSubscribeServerEventHandler.h>
#include <net/HttpBody.h>
#include <net/SipMessage.h>
#include <net/SipDialogMgr.h>


// Private class to contain event type and event specific utilities
class SubscribeServerEventData : public UtlString
{
public:
    SubscribeServerEventData();

    virtual ~SubscribeServerEventData();

    // Parent UtlString contains the eventType
    SipSubscribeServerEventHandler* mpEventSpecificHandler;
    // Values from the enableEventType call.
    SipUserAgent* mpEventSpecificUserAgent;
    SipPublishContentMgr* mpEventSpecificContentMgr;
    SipSubscriptionMgr* mpEventSpecificSubscriptionMgr;
    SipContentVersionCallback mpEventSpecificContentVersionCallback;
    UtlBoolean mEventSpecificFullState;

    //! Dump the object's internal state.
    void dumpState();

private:
    //! DISALLOWED accidental copying
    SubscribeServerEventData(const SubscribeServerEventData& rSubscribeServerEventData);
    SubscribeServerEventData& operator=(const SubscribeServerEventData& rhs);
};
SubscribeServerEventData::SubscribeServerEventData() :
   mpEventSpecificHandler(NULL),
   mpEventSpecificUserAgent(NULL),
   mpEventSpecificContentMgr(NULL),
   mpEventSpecificSubscriptionMgr(NULL),
   mpEventSpecificContentVersionCallback(NULL),
   mEventSpecificFullState(TRUE)
{
}

SubscribeServerEventData::~SubscribeServerEventData()
{
}

// Dump the object's internal state.
void SubscribeServerEventData::dumpState()
{
   // indented 4

   OsSysLog::add(FAC_RLS, PRI_INFO,
                 "\t    SubscribeServerEventData %p UtlString = '%s', mpEventSpecificHandler = %p, mpEventSpecificUserAgent = %p, mpEventSpecificContentMgr = %p, mpEventSpecificSubscriptionMgr = %p",
                 this, data(), mpEventSpecificHandler, mpEventSpecificUserAgent,
                 mpEventSpecificContentMgr, mpEventSpecificSubscriptionMgr);
}


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

SipSubscribeServer* SipSubscribeServer::buildBasicServer(SipUserAgent& userAgent,
                                                         const char* eventType)
{
    // Create a default publisher container
    SipPublishContentMgr* publishContent = new SipPublishContentMgr();

    // Create a default event handler
    SipSubscribeServerEventHandler* eventHandler = new SipSubscribeServerEventHandler();

    // Create a default subscription mgr
    SipSubscriptionMgr* subscriptionMgr = new SipSubscriptionMgr();

    SipSubscribeServer* newServer = new SipSubscribeServer(userAgent,
                                                           *publishContent,
                                                           *subscriptionMgr,
                                                           *eventHandler);

    if (eventType && *eventType)
    {
        // Enable the server to accept the given SIP event package
        newServer->enableEventType(eventType,
                                   &userAgent,
                                   publishContent,
                                   eventHandler,
                                   subscriptionMgr,
                                   SipSubscribeServer::standardVersionCallback
           );
    }

    return newServer;
}

// Constructor
SipSubscribeServer::SipSubscribeServer(SipUserAgent& defaultUserAgent,
                                       SipPublishContentMgr& defaultContentMgr,
                                       SipSubscriptionMgr& defaultSubscriptionMgr,
                                       SipSubscribeServerEventHandler& defaultEventHandler)
    : OsServerTask("SipSubscribeServer-%d")
    , mSubscribeServerMutex(OsMutex::Q_FIFO)
{
    mpDefaultUserAgent = &defaultUserAgent;
    mpDefaultContentMgr = &defaultContentMgr;
    mpDefaultSubscriptionMgr = &defaultSubscriptionMgr;
    mpDefaultEventHandler = &defaultEventHandler;
}


// Destructor
SipSubscribeServer::~SipSubscribeServer()
{
   /*
    * Don't delete  mpDefaultContentMgr, mpDefaultSubscriptionMgr, or mpDefaultEventHandler
    *   they are owned by whoever constructed this server.
    */

   // Iterate through and delete all the event data
   mEventDefinitions.destroyAll();
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
SipSubscribeServer&
SipSubscribeServer::operator=(const SipSubscribeServer& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

void SipSubscribeServer::contentChangeCallback(void* applicationData,
                                               const char* resourceId,
                                               const char* eventTypeKey,
                                               const char* eventType)
{
    SipSubscribeServer* subServer =
       (SipSubscribeServer*) applicationData;
    subServer->notifySubscribers(resourceId,
                                 eventTypeKey,
                                 eventType);
}

// Send a NOTIFY to all subscribers to the given resourceId and eventTypeKey.
UtlBoolean SipSubscribeServer::notifySubscribers(const char* resourceId,
                                                 const char* eventTypeKey,
                                                 const char* eventType)
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipSubscribeServer::notifySubscribers resourceId '%s', eventTypeKey '%s', eventType '%s'",
                 resourceId, eventTypeKey, eventType);
    UtlBoolean notifiedSubscribers = FALSE;
    UtlString eventName(eventType ? eventType : "");

    lockForRead();
    SubscribeServerEventData* eventData =
       dynamic_cast <SubscribeServerEventData*>
           (mEventDefinitions.find(&eventName));

    // Get the event-specific info to find subscriptions interested in
    // this content.
    if (eventData)
    {
        int numSubscriptions = 0;
        SipMessage** notifyArray = NULL;
        UtlString** acceptHeaderValuesArray = NULL;

        // Construct a NOTIFY (without body) for each subscription.
        eventData->mpEventSpecificSubscriptionMgr->
           createNotifiesDialogInfo(resourceId,
                                    eventTypeKey,
                                    numSubscriptions,
                                    acceptHeaderValuesArray,
                                    notifyArray);

        OsSysLog::add(FAC_SIP, PRI_DEBUG,
             "SipSubscribeServer::notifySubscribers numSubscriptions for %s = %d",
              resourceId, numSubscriptions);

        // Set up and send a NOTIFY for each subscription interested in
        // this resourceId and eventTypeKey.
        for (int notifyIndex = 0;
             notifyArray != NULL &&
                notifyIndex < numSubscriptions &&
                notifyArray[notifyIndex] != NULL;
             notifyIndex++)
        {
            SipMessage* notify = notifyArray[notifyIndex];

            // Check to see if the NOTIFY headers could be set.
            // (The subscription might have been destroyed between when
            // it was decided to respond to it, and when the dialog information
            // was retrieved.)
            UtlString callId;
            notify->getCallIdField(&callId);
            if (!callId.isNull())
            {
               // Fill in the NOTIFY request body/content
               eventData->mpEventSpecificHandler->
                  getNotifyContent(resourceId,
                                   eventTypeKey,
                                   eventType,
                                   *(eventData->mpEventSpecificContentMgr),
                                   *(acceptHeaderValuesArray[notifyIndex]),
                                   *notify,
                                   eventData->mEventSpecificFullState);

               // Call the application callback to edit the NOTIFY
               // content if that is required for this event type.
               // Sets 'version' and 'eventTypeKey'.
               int version;
               UtlString savedEventTypeKey;
               eventData->mpEventSpecificSubscriptionMgr->
                  updateNotifyVersion(eventData->mpEventSpecificContentVersionCallback,
                                      *notify,
                                      version,
                                      savedEventTypeKey);

               // Update the saved record of the NOTIFY CSeq and the
               // XML version number for the specified eventTypeKey,
               // as needed by the subscription manager.
               // In practice, this is only used by SipPersistentSubscriptionMgr
               // to write the NOTIFY Cseq and XML version into the IMDB.
               eventData->mpEventSpecificSubscriptionMgr->
                  updateVersion(*notify, version, savedEventTypeKey);

               // Set up the NOTIFY To/From headers.
               setContact(notify);

               // Send the NOTIFY request
               eventData->mpEventSpecificUserAgent->send(*notify);
            }
        }

        // Free the NOTIFY requests and accept header field values.
        eventData->mpEventSpecificSubscriptionMgr->
           freeNotifies(numSubscriptions,
                        acceptHeaderValuesArray,
                        notifyArray);
    }
    // event type not enabled
    else
    {
        OsSysLog::add(FAC_SIP, PRI_ERR,
            "SipSubscribeServer::notifySubscribers event type: %s not enabled",
            eventName.data());
    }

    unlockForRead();

    return (notifiedSubscribers);
}

UtlBoolean SipSubscribeServer::enableEventType(const char* eventTypeToken,
                                               SipUserAgent* userAgent,
                                               SipPublishContentMgr* contentMgr,
                                               SipSubscribeServerEventHandler* eventHandler,
                                               SipSubscriptionMgr* subscriptionMgr,
                                               SipContentVersionCallback contentVersionCallback,
                                               UtlBoolean onlyFullState)
{
    UtlBoolean addedEvent = FALSE;
    UtlString eventName(eventTypeToken ? eventTypeToken : "");
    lockForWrite();
    // Only add the event support if it does not already exist;
    SubscribeServerEventData* eventData =
       dynamic_cast <SubscribeServerEventData*> (mEventDefinitions.find(&eventName));
    if (!eventData)
    {
        addedEvent = TRUE;
        eventData = new SubscribeServerEventData();
        static_cast <UtlString&> (*eventData) = eventName;
        eventData->mpEventSpecificUserAgent =
           userAgent ? userAgent : mpDefaultUserAgent;
        eventData->mpEventSpecificContentMgr =
           contentMgr ? contentMgr : mpDefaultContentMgr;
        eventData->mpEventSpecificHandler =
           eventHandler ? eventHandler : mpDefaultEventHandler;
        eventData->mpEventSpecificSubscriptionMgr =
           subscriptionMgr ? subscriptionMgr : mpDefaultSubscriptionMgr;
        eventData->mpEventSpecificContentVersionCallback = contentVersionCallback;
        eventData->mEventSpecificFullState = onlyFullState;
        mEventDefinitions.insert(eventData);

        // Register an interest in SUBSCRIBE requests and NOTIFY responses
        // for this event type
        eventData->mpEventSpecificUserAgent->addMessageObserver(*(getMessageQueue()),
                                                                SIP_SUBSCRIBE_METHOD,
                                                                TRUE, // requests
                                                                FALSE, // not reponses
                                                                TRUE, // incoming
                                                                FALSE, // no outgoing
                                                                eventName,
                                                                NULL,
                                                                NULL);
        eventData->mpEventSpecificUserAgent->addMessageObserver(*(getMessageQueue()),
                                                                SIP_NOTIFY_METHOD,
                                                                FALSE, // no requests
                                                                TRUE, // reponses
                                                                TRUE, // incoming
                                                                FALSE, // no outgoing
                                                                eventName,
                                                                NULL,
                                                                NULL);

        // Register the callback for changes that occur in the
        // publish content manager.
        eventData->mpEventSpecificContentMgr->
           setContentChangeObserver(eventName,
                                    this,
                                    contentChangeCallback);
    }

    unlockForWrite();

    return(addedEvent);
}

UtlBoolean SipSubscribeServer::disableEventType(const char* eventTypeToken,
                                                SipUserAgent*& userAgent,
                                                SipPublishContentMgr*& contentMgr,
                                                SipSubscribeServerEventHandler*& eventHandler,
                                                SipSubscriptionMgr*& subscriptionMgr)
{
    UtlBoolean removedEvent = FALSE;
    UtlString eventName(eventTypeToken ? eventTypeToken : "");
    lockForWrite();
    // Only remove the event support if it already exists;
    SubscribeServerEventData* eventData =
       dynamic_cast <SubscribeServerEventData*> (mEventDefinitions.remove(&eventName));
    if (eventData)
    {
        removedEvent = TRUE;
        userAgent = eventData->mpEventSpecificUserAgent == mpDefaultUserAgent ?
                        NULL : eventData->mpEventSpecificUserAgent;
        contentMgr = eventData->mpEventSpecificContentMgr == mpDefaultContentMgr ?
                        NULL : eventData->mpEventSpecificContentMgr;
        eventHandler = eventData->mpEventSpecificHandler == mpDefaultEventHandler ?
                        NULL : eventData->mpEventSpecificHandler;
        subscriptionMgr = eventData->mpEventSpecificSubscriptionMgr == mpDefaultSubscriptionMgr ?
                        NULL : eventData->mpEventSpecificSubscriptionMgr;

        // Unregister interest in SUBSCRIBE requests and NOTIFY
        // responses for this event type
        eventData->mpEventSpecificUserAgent->removeMessageObserver(*(getMessageQueue()));


        delete eventData;
        eventData = NULL;
    }
    else
    {
        userAgent = NULL;
        contentMgr = NULL;
        eventHandler = NULL;
        subscriptionMgr = NULL;
    }

    unlockForWrite();

    return(removedEvent);
}

UtlBoolean SipSubscribeServer::handleMessage(OsMsg &eventMessage)
{
    int msgType = eventMessage.getMsgType();
    int msgSubType = eventMessage.getMsgSubType();

    // Timer fired
    if (msgType == OsMsg::OS_EVENT &&
        msgSubType == OsEventMsg::NOTIFY)
    {
        OsTimer* timer = 0;
        UtlString* subscribeDialogHandle = NULL;
        intptr_t timerIntptr;
        void* subscribeDialogHandleVoid;

        ((OsEventMsg&)eventMessage).getUserData(subscribeDialogHandleVoid);
        ((OsEventMsg&)eventMessage).getEventData(timerIntptr);
	subscribeDialogHandle = (UtlString*)subscribeDialogHandleVoid;
        timer = (OsTimer*)timerIntptr;

        if (subscribeDialogHandle)
        {
            // Check if the subscription really expired and send
            // the final NOTIFY if it did.
            handleExpiration(subscribeDialogHandle, timer);

            // Delete the handle;
            delete subscribeDialogHandle;

            // do not delete the timer.
            // handlExpiration deals with that and may reuse the timer
        }
    }

    // SIP message
    else if (msgType == OsMsg::PHONE_APP &&
             msgSubType == SipMessage::NET_SIP_MESSAGE)
    {
        const SipMessage* sipMessage = ((SipMessageEvent&)eventMessage).getMessage();

        UtlString method;
        if (sipMessage)
        {
            sipMessage->getCSeqField(NULL, &method);
        }

        // SUBSCRIBE requests
        if (sipMessage &&
            !sipMessage->isResponse() &&
            method.compareTo(SIP_SUBSCRIBE_METHOD) == 0)
        {
            handleSubscribe(*sipMessage);
        }

        // NOTIFY responses
        else if (sipMessage &&
                 sipMessage->isResponse() &&
                 method.compareTo(SIP_NOTIFY_METHOD) == 0)
        {
            handleNotifyResponse(*sipMessage);
        }
    }

    return(TRUE);
}

void SipSubscribeServer::setContact(SipMessage* message)
{
    // Pull out the From userId and make sure that is used
    // in the contact -- otherwise, re-subscribes may request
    // a different resource (e.g. ~~park~id instead of the
    // the park orbit).
    UtlString fromUserId ;
    Url from ;
    if (message->isResponse())
    {
        message->getToUrl(from) ;
    }
    else
    {
        message->getFromUrl(from) ;
    }

    from.getUserId(fromUserId) ;
    if (fromUserId.length())
    {
        UtlString defaultContact;
        mpDefaultUserAgent->getContactUri(&defaultContact);
        Url defaultContactUrl(defaultContact) ;
        defaultContactUrl.setUserId(fromUserId);
        message->setContactField(defaultContactUrl.toString()) ;
    }
}


/* ============================ ACCESSORS ================================= */

SipSubscribeServerEventHandler*
SipSubscribeServer::getEventHandler(const UtlString& eventType)
{
    SipSubscribeServerEventHandler* eventHandler = NULL;
    lockForRead();
    SubscribeServerEventData* eventData =
       dynamic_cast <SubscribeServerEventData*> (mEventDefinitions.find(&eventType));
    if (eventData)
    {
        eventHandler = eventData->mpEventSpecificHandler;
    }

    else
    {
        eventHandler = mpDefaultEventHandler;
    }
    unlockForRead();

    return(eventHandler);
}

SipPublishContentMgr*
SipSubscribeServer::getPublishMgr(const UtlString& eventType)
{
    SipPublishContentMgr* contentMgr = NULL;
    lockForRead();
    SubscribeServerEventData* eventData =
       dynamic_cast <SubscribeServerEventData*> (mEventDefinitions.find(&eventType));
    if (eventData)
    {
        contentMgr = eventData->mpEventSpecificContentMgr;
    }

    else
    {
        contentMgr = mpDefaultContentMgr;
    }
    unlockForRead();

    return(contentMgr);
}

SipSubscriptionMgr* SipSubscribeServer::getSubscriptionMgr(const UtlString& eventType)
{
    SipSubscriptionMgr* subscribeMgr = NULL;
    lockForRead();
    SubscribeServerEventData* eventData =
       dynamic_cast <SubscribeServerEventData*> (mEventDefinitions.find(&eventType));
    if (eventData)
    {
        subscribeMgr = eventData->mpEventSpecificSubscriptionMgr;
    }

    else
    {
        subscribeMgr = mpDefaultSubscriptionMgr;
    }
    unlockForRead();

    return(subscribeMgr);
}

/* ============================ INQUIRY =================================== */

// Dump the object's internal state.
void SipSubscribeServer::dumpState()
{
   lockForRead();

   // indented 2

   OsSysLog::add(FAC_RLS, PRI_INFO,
                 "\t  SipSubscribeServer %p",
                 this);

   mpDefaultContentMgr->dumpState();
   mpDefaultSubscriptionMgr->dumpState();
   // mpDefaultEventHandler has no internal state.
   UtlHashBagIterator itor(mEventDefinitions);
   SubscribeServerEventData* ed;
   while ((ed = dynamic_cast <SubscribeServerEventData*> (itor())))
   {
      ed->dumpState();
   }

   unlockForRead();
}

UtlBoolean SipSubscribeServer::isEventTypeEnabled(const UtlString& eventType)
{
    lockForRead();
    // Only add the event support if it does not already exist;
    SubscribeServerEventData* eventData =
       dynamic_cast <SubscribeServerEventData*> (mEventDefinitions.find(&eventType));
    unlockForRead();

    return(eventData != NULL);
}

// Static callback routine used to find and replace variable string values in
// subscription content.
// For example the NOTIFY message in the SIP stack contains "&version;" rather
// than an actual version number [like "22"].
// The content provided by publish() provides the "context
// independent" part of the content, and the SIP stack keeps knowledge
// of the version number sequence for each subscription.  This
// callback combines these sources of information.
UtlBoolean SipSubscribeServer::standardVersionCallback(SipMessage& notifyRequest,
                                                       int version)
{
   // Search and replace the version number in the Notify.
   UtlBoolean result = FALSE;

   if (notifyRequest.getBody() != NULL)
   {
      UtlString msgBytes;
      ssize_t msgLength;
      // Extract the NOTIFY body as a UtlString.
      notifyRequest.getBody()->getBytes(&msgBytes, &msgLength);
      const char* contentType = notifyRequest.getBody()->getContentType();

      // Look for the placeholder for the version number, "&version;".
      ssize_t msgIndex = msgBytes.index(VERSION_PLACEHOLDER);
      if (msgIndex != UTL_NOT_FOUND)
      {
         char buffer[20];
         sprintf(buffer, "%d", version);
         msgBytes.replace(msgIndex, sizeof (VERSION_PLACEHOLDER) - 1, buffer);

         HttpBody* tempBody =
            new HttpBody(msgBytes.data(), msgBytes.length(), contentType);

         // Write the new message contents (this deletes the old body)
         notifyRequest.setBody(tempBody);
         result = TRUE;
      }
   }

   return result;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

UtlBoolean SipSubscribeServer::handleSubscribe(const SipMessage& subscribeRequest)
{
    UtlBoolean handledSubscribe = FALSE;
    UtlString eventName;
    subscribeRequest.getEventField(&eventName, NULL);

    // Not modifying the SubscribeServerEventData, just reading it
    lockForRead();

    // Get the event specific handler and information
    SubscribeServerEventData* eventPackageInfo =
       dynamic_cast <SubscribeServerEventData*> (mEventDefinitions.find(&eventName));

    // We handle this event type
    if (eventPackageInfo)
    {
        handledSubscribe = TRUE;
        SipSubscribeServerEventHandler* handler =
            eventPackageInfo->mpEventSpecificHandler;

        SipMessage subscribeResponse;

        // Check if authenticated (or if it needs to be authenticated)
        if (handler->isAuthenticated(subscribeRequest,
                                     subscribeResponse))
        {
            // Check if authorized (or if authorization is required)
            if (handler->isAuthorized(subscribeRequest,
                                     subscribeResponse))
            {
                // The subscription is allowed, so update the
                // subscription state.  Set the To field tag if
                // this request initiated the dialog
                UtlString subscribeDialogHandle;
                UtlBoolean isNewDialog;
                UtlBoolean isExpiredSubscription;
                UtlString resourceId, eventTypeKey, eventType;
                eventPackageInfo->mpEventSpecificSubscriptionMgr->
                   updateDialogInfo(
                      subscribeRequest,
                      resourceId,
                      eventTypeKey,
                      subscribeDialogHandle,
                      isNewDialog,
                      isExpiredSubscription,
                      subscribeResponse,
                      // The event-specific handler provides a getKeys method
                      // which is used to determine the resource ID
                      // and event type if this is a new subscription.
                      *handler);

                 // Send the response ASAP to minimize resending.
                 setContact(&subscribeResponse) ;
                 eventPackageInfo->mpEventSpecificUserAgent->send(subscribeResponse);

                 // Build a NOTIFY
                 SipMessage notifyRequest;

                 // Set the dialog information
                 // Note that the dialog can have ended by now, because of
                 // a race condition with the processing of dialog-ending
                 // messages from the outside world.
                 if (eventPackageInfo->mpEventSpecificSubscriptionMgr->
                     getNotifyDialogInfo(subscribeDialogHandle,
                                         notifyRequest))
                 {
                    // The NOTIFY headers were set because we still
                    // have record of the dialog.

                    // Set the NOTIFY content
                    UtlString acceptHeaderValue;
                    subscribeRequest.getAcceptField(acceptHeaderValue);
                    // Note that since this NOTIFY is due to a SUBSCRIBE,
                    // it should contain 'full' content.  Hence,
                    // the fullState parameter of getNotifyContent is TRUE,
                    // and is not taken from
                    // eventPackageInfo->mEventSpecificFullState.
                    if (handler->getNotifyContent(resourceId,
                                                  eventTypeKey,
                                                  eventType,
                                                  *(eventPackageInfo->mpEventSpecificContentMgr),
                                                  acceptHeaderValue,
                                                  notifyRequest,
                                                  TRUE))
                    {
                       // Update the NOTIFY content if required for this event type.
                       // Sets 'version' and 'eventTypeKey'.
                       int version;
                       eventPackageInfo->mpEventSpecificSubscriptionMgr->
                          updateNotifyVersion(eventPackageInfo->mpEventSpecificContentVersionCallback,
                                              notifyRequest,
                                              version,
                                              eventTypeKey);

                       // Update the saved record of the NOTIFY CSeq and the
                       // XML version number for the specified eventTypeKey,
                       // as needed by the subscription manager.
                       // In practice, this is only used by SipPersistentSubscriptionMgr
                       // to write the NOTIFY Cseq and XML version into the IMDB.
                       eventPackageInfo->mpEventSpecificSubscriptionMgr->
                          updateVersion(notifyRequest, version, eventTypeKey);

                       setContact(&notifyRequest);
                    }
                    else
                    {
                       setContact(&notifyRequest);
                    }

                    // Send the notify request
                    eventPackageInfo->mpEventSpecificUserAgent->send(notifyRequest);

                    if (OsSysLog::willLog(FAC_SIP, PRI_INFO))
                    {
                       UtlString requestContact;
                       subscribeRequest.getContactField(0, requestContact);
                       OsSysLog::add(FAC_SIP, PRI_INFO,
                           "SipSubscribeServer::handleSubscribe: %s has setup subscription to %s, eventTypeKey %s",
                           requestContact.data(), resourceId.data(), eventTypeKey.data());
                    }
                 }
            }
            // Not authorized
            else
            {
                // Send the response
                setContact(&subscribeResponse);
                eventPackageInfo->mpEventSpecificUserAgent->send(subscribeResponse);
            }
        }

        // Not authenticated
        else
        {
            // Send the response
            setContact(&subscribeResponse);
            eventPackageInfo->mpEventSpecificUserAgent->send(subscribeResponse);
        }
    }

    // We should not have received SUBSCRIBE requests for this event type
    // This event type has not been enabled in this SubscribeServer
    else
    {
        OsSysLog::add(FAC_SIP, PRI_ERR,
            "SipSubscribeServer::handleSubscribe event type: %s not enabled",
            eventName.data());

        SipMessage eventTypeNotHandled;
        eventTypeNotHandled.setResponseData(&subscribeRequest,
            SIP_BAD_EVENT_CODE, SIP_BAD_EVENT_TEXT);

        setContact(&eventTypeNotHandled);
        mpDefaultUserAgent->send(eventTypeNotHandled);
    }
    unlockForRead();

    return(handledSubscribe);
}

UtlBoolean SipSubscribeServer::handleNotifyResponse(const SipMessage& notifyResponse)
{
    UtlBoolean handledNotifyResponse = FALSE;
    int responseCode = notifyResponse.getResponseStatusCode();

    // If it is a non-Timeout error response and has no Retry-After header,
    // terminate the subscription.
    if (responseCode >= SIP_3XX_CLASS_CODE)
    {
       UtlString dialogHandle;
       notifyResponse.getDialogHandleReverse(dialogHandle);

       if (   SIP_REQUEST_TIMEOUT_CODE == responseCode
           || SIP_LOOP_DETECTED_CODE == responseCode)
       {
          // Log the response.
          OsSysLog::add(FAC_SIP, PRI_WARNING,
                        "SipSubscribeServer::handleNotifyResponse "
                        "Not terminating subscription due to %d response. Handle: %s",
                        responseCode, dialogHandle.data());

          // RFC 3265 section 3.2.2 says that when a "NOTIFY request
          // fails ... due to a timeout condition ... the notifier
          // SHOULD remove the subscription."  However, we feel this
          // is a problem with the spec.  Endpoints that become
          // temporarily unavailable and do not respond to the NOTIFY
          // (or do not respond in time) should not cause the
          // subscription to be silently terminated.  Our deviation
          // from the spec will cause subscriptions to now-defunct
          // subscribers to continue, but only until the subscription
          // expires.  This is far preferable to the alternative.

          // Similarly, RFC 3265 prescribes that when the notifier
          // receives a 482 response, it should remove the
          // subscription.  But that seems to be counterproductive in
          // practice, as a 482 response shows that the subscriber did
          // receive the NOTIFY.  Unfortunately, under congested
          // circumstances, a notifier can receive a 482 response
          // rather than the more informative 2xx response.
       }
       else if (notifyResponse.getHeaderValue(0, SIP_RETRY_AFTER_FIELD) == NULL)
       {
          OsSysLog::add(FAC_SIP, PRI_WARNING,
                        "SipSubscribeServer::handleNotifyResponse "
                        "Terminating subscription due to %d response. Handle: %s",
                        responseCode, dialogHandle.data());
          // Not modifying the SubscribeServerEventData, just reading it
          lockForRead();

          // Get the event specific handler and information
          SubscribeServerEventData* eventPackageInfo = NULL;
          UtlHashBagIterator iterator(mEventDefinitions);

          while ((eventPackageInfo =
                  dynamic_cast <SubscribeServerEventData*> (iterator())))
          {
             // End this subscription because we got an error response from
             // the NOTIFY request.
             // Returns TRUE if the SipSubscriptionMgr has this dialog.
             handledNotifyResponse =
                eventPackageInfo->
                mpEventSpecificSubscriptionMgr->
                endSubscription(dialogHandle);
             if (handledNotifyResponse)
             {
                break;
             }
          }

          unlockForRead();

          if (!handledNotifyResponse)
          {
             // Should not happen, first of all we should never get a
             // response which does not correspond to a request sent from
             // the SipUserAgent.  Secondly, we should not get a response to
             // and event type that we do not support
             OsSysLog::add(FAC_SIP, PRI_ERR,
                "SipSubscribeServer::handleNotifyResponse NOTIFY response with no dialog. Handle: %s",
                dialogHandle.data());
          }
       }
    }

    return TRUE;
}

UtlBoolean SipSubscribeServer::handleExpiration(UtlString* subscribeDialogHandle,
                                                OsTimer* timer)
{
    // TODO: Currently timers are not set for the subscription
    // expiration time.  It is not clear this is really a useful
    // thing to do other than the fact that RFC 3265 says you
    // should send a final NOTIFY indicating that the subscription
    // expired.  I cannot come up with a use case where it is
    // needed that the subscribe client gets a final NOTIFY.
    // The client should already know when the expiration is
    // going to occur and that it has not reSUBSCRIBEd.
    OsSysLog::add(FAC_SIP, PRI_ERR,
                  "SipSubscribeServer::handleExpiration not implemented");
    return(FALSE);
}


void SipSubscribeServer::lockForRead()
{
    mSubscribeServerMutex.acquireRead();
}

void SipSubscribeServer::unlockForRead()
{
    mSubscribeServerMutex.releaseRead();
}

void SipSubscribeServer::lockForWrite()
{
    mSubscribeServerMutex.acquireWrite();
}

void SipSubscribeServer::unlockForWrite()
{
    mSubscribeServerMutex.releaseWrite();
}

/* ============================ FUNCTIONS ================================= */
