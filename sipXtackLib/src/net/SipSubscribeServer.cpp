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
    SubscribeServerEventData(const UtlString& eventName);

    virtual ~SubscribeServerEventData();

    // Parent UtlString contains the eventType
    SipSubscribeServerEventHandler* mpEventSpecificHandler;
    // Values from the enableEventType call.
    SipUserAgent* mpEventSpecificUserAgent;
    SipPublishContentMgr* mpEventSpecificContentMgr;
    SipContentVersionCallback mpEventSpecificContentVersionCallback;
    UtlBoolean mEventSpecificFullState;

    //! Dump the object's internal state.
    void dumpState();

private:
    //! DISALLOWED accidental copying
    SubscribeServerEventData(const SubscribeServerEventData& rSubscribeServerEventData);
    SubscribeServerEventData& operator=(const SubscribeServerEventData& rhs);
};
SubscribeServerEventData::SubscribeServerEventData(const UtlString& eventName) :
   UtlString(eventName),
   mpEventSpecificHandler(NULL),
   mpEventSpecificUserAgent(NULL),
   mpEventSpecificContentMgr(NULL),
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
                 "\t    SubscribeServerEventData %p UtlString = '%s', mpEventSpecificHandler = %p, mpEventSpecificUserAgent = %p, mpEventSpecificContentMgr = %p",
                 this, data(), mpEventSpecificHandler, mpEventSpecificUserAgent,
                 mpEventSpecificContentMgr);
}


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

//! Reason values for termination of subscriptions.
const char SipSubscribeServer::terminationReasonNone[] = "";
const char SipSubscribeServer::terminationReasonSilent[] = "[SILENT]";
const char SipSubscribeServer::terminationReasonDeactivated[] = "deactivated";
const char SipSubscribeServer::terminationReasonProbation[] = "probation";
const char SipSubscribeServer::terminationReasonRejected[] = "rejected";
const char SipSubscribeServer::terminationReasonTimeout[] = "timeout";
const char SipSubscribeServer::terminationReasonGiveup[] = "giveup";
const char SipSubscribeServer::terminationReasonNoresource[] = "noresource";

// Milliseconds of delay to allow when sending out bulk NOTIFYs.
const int SipSubscribeServer::sNotifyDelay = 100;


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

SipSubscribeServer* SipSubscribeServer::buildBasicServer(SipUserAgent& userAgent,
                                                         const char* eventType)
{
    // Create a default publisher container
    SipPublishContentMgr* publishContent = new SipPublishContentMgr();

    // Create a default event handler
    SipSubscribeServerEventHandler* eventHandler = new SipSubscribeServerEventHandler();

    // Create a subscription mgr.
    SipSubscriptionMgr* subscriptionMgr = new SipSubscriptionMgr();

    SipSubscribeServer* newServer =
       new SipSubscribeServer(SipSubscribeServer::terminationReasonNone,
                              userAgent,
                              *publishContent,
                              *subscriptionMgr,
                              *eventHandler);

    if (eventType && *eventType)
    {
        // Enable the server to accept the given SIP event package
        newServer->enableEventType(eventType, NULL, NULL, NULL,
                                   SipSubscribeServer::standardVersionCallback
           );
    }

    return newServer;
}

/// Terminate all subscriptions and accept no new ones.
void SipSubscribeServer::shutdown(const char* reason)
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipSubscribeServer::shutdown reason = '%s'",
                 reason ? reason : "[null]");

   // If reason is NULL, use the default reason established by the constructor.
   if (!reason)
   {
      reason = mDefaultTermination;
   }

   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipSubscribeServer::shutdown reason after defaulting = '%s'",
                 reason ? reason : "[null]");                 

   lockForRead();

   // Select the subscriptionChange value to describe what we're doing
   // with the subscriptions.
   enum SipSubscriptionMgr::subscriptionChange change;

   // Select the correct format for generating the Subscription-State value.
   UtlString* formatp = NULL; // We may need a temp UtlString.
   const char* format;

   if (reason == NULL)
   {
      format = "active;expires=%ld";
      change = SipSubscriptionMgr::subscriptionContinues;
   }
   else if (strcmp(reason, terminationReasonSilent) == 0)
   {
      // Do not admit that the subscription is ending.
      format = "active;expires=%ld";
      change = SipSubscriptionMgr::subscriptionTerminatedSilently;
   }
   else if (strcmp(reason, terminationReasonNone) == 0)
   {
      format = "terminated";
      change = SipSubscriptionMgr::subscriptionTerminated;
   }
   else
   {
      // Allocate a UtlString and assemble the Subscription-State format in it.
      formatp = new UtlString();
      formatp->append("terminated;reason=");
      formatp->append(reason);
      format = formatp->data();
      change = SipSubscriptionMgr::subscriptionTerminated;
   }

   // For each event type that is registered, delete all the subscriptions.

   UtlHashBagIterator event_itor(mEventDefinitions);
   SubscribeServerEventData* eventData;
   while ((eventData =
              dynamic_cast <SubscribeServerEventData*> (event_itor())))
   {
      // Unregister interest in SUBSCRIBE requests and NOTIFY
      // responses for this event type, so we do not service new subscriptions.

      eventData->mpEventSpecificUserAgent->removeMessageObserver(*(getMessageQueue()));
      
      int numSubscriptions = 0;
      SipMessage* notifyArray = NULL;
      UtlString* acceptHeaderValuesArray = NULL;
      UtlString* resourceIdArray = NULL;
      UtlString* eventTypeKeyArray = NULL;
      bool* fullContentArray = NULL;

      // :TODO: The four situations where NOTIFYs are generated should
      // be factored into a series of methods in
      // SipSubscriptionMgr that generate NOTIFYs
      // sequentially, and for each NOTIFY, call a common service
      // method that does the remaining operations and sends the
      // NOTIFY.

      // Construct a NOTIFY (without body) for every subscription, containing
      // the dialog-related information about each subscription.
      mpSubscriptionMgr->
         createNotifiesDialogInfoEvent(static_cast <const UtlString&> (*eventData),
                                       format,
                                       numSubscriptions,
                                       acceptHeaderValuesArray,
                                       notifyArray,
                                       resourceIdArray,
                                       eventTypeKeyArray,
                                       fullContentArray);

      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipSubscribeServer::shutdown eventType = '%s', numSubscriptions = %d",
                    eventData->data(), numSubscriptions);

      // For each NOTIFY, add the subscription-related information and then
      // send it.
      for (int notifyIndex = 0;
           notifyIndex < numSubscriptions;
           notifyIndex++)
      {
         SipMessage* notify = &notifyArray[notifyIndex];

         // Check to see if the dialog information could be added.
         // (The subscription might have been destroyed between when
         // it was decided to respond to it, and when the dialog information
         // was retrieved.)
         UtlString callId;
         notify->getCallIdField(&callId);
         if (!callId.isNull())
         {
            if (change != SipSubscriptionMgr::subscriptionTerminatedSilently)
            {
               // Fill in the NOTIFY request body/content
               eventData->mpEventSpecificHandler->
                  getNotifyContent(resourceIdArray[notifyIndex],
                                   eventTypeKeyArray[notifyIndex],
                                   *eventData,
                                   *(eventData->mpEventSpecificContentMgr),
                                   acceptHeaderValuesArray[notifyIndex],
                                   *notify,
                                   eventData->mEventSpecificFullState ||
                                   fullContentArray[notifyIndex],
                                   NULL);

               // Call the application callback to edit the NOTIFY
               // content if that is required for this event type.
               // Also gets 'version' (if relevant) and 'savedEventTypeKey'.
               int version;
               UtlString savedEventTypeKey;
               mpSubscriptionMgr->
                  updateNotifyVersion(eventData->mpEventSpecificContentVersionCallback,
                                      *notify,
                                      version,
                                      savedEventTypeKey);

               // Set the Contact header.
               setContact(notify);

               // Send the NOTIFY request.
               eventData->mpEventSpecificUserAgent->send(*notify);

               // Reset the resend timer to the minimum, so if this NOTIFY fails,
               // we will resend it quickly.
               UtlString dialogHandle;
               notify->getDialogHandleReverse(dialogHandle);
               mpSubscriptionMgr->newNotify(dialogHandle);

               // Pace sending the NOTIFYs, of which there may be many.
               OsTask::delay(sNotifyDelay);
            }

            // Remove the record of the subscription.
            UtlString dialogHandle;
            notify->getDialogHandleReverse(dialogHandle);
            mpSubscriptionMgr->endSubscription(dialogHandle, change);
         }
      }

      // Free the allocated arrays.
      delete[] acceptHeaderValuesArray;
      delete[] notifyArray;
      delete[] resourceIdArray;
      delete[] eventTypeKeyArray;
      delete[] fullContentArray;

      // Remove eventData from mEventDefinitions.
      mEventDefinitions.removeReference(eventData);
      delete eventData;
   }

   unlockForRead();

   // Free the temporary UtlString, if necessary.
   if (formatp)
   {
      delete formatp;
   }

   lockForWrite();
   mEventDefinitions.destroyAll();
   unlockForWrite();
}

// Constructor
SipSubscribeServer::SipSubscribeServer(const char* defaultTermination,
                                       SipUserAgent& defaultUserAgent,
                                       SipPublishContentMgr& defaultContentMgr,
                                       SipSubscriptionMgr& subscriptionMgr,
                                       SipSubscribeServerEventHandler& defaultEventHandler)
    : OsServerTask("SipSubscribeServer-%d")
    , mpDefaultUserAgent(&defaultUserAgent)
    , mpDefaultContentMgr(&defaultContentMgr)
    , mpSubscriptionMgr(&subscriptionMgr)
    , mpDefaultEventHandler(&defaultEventHandler)
    , mDefaultTermination(defaultTermination)
    , mSubscribeServerMutex(OsMutex::Q_FIFO)
{
   // Set our queue to be the queue for resend event messages in
   // *mpSubscriptionMgr.
   mpSubscriptionMgr->initialize(getMessageQueue());
}


// Destructor
SipSubscribeServer::~SipSubscribeServer()
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipSubscribeServer::~");

   // Execute the default ::shutdown().
   shutdown(mDefaultTermination);

   /*
    * Don't delete  mpDefaultContentMgr, mpDefaultSubscriptionMgr, or
    * mpDefaultEventHandler -- they are owned by whoever constructed
    * this SipSubscribeServer.
    */

   // Delete all the event data
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
                                               const char* eventType,
                                               const char* reason)
{
    SipSubscribeServer* subServer =
       (SipSubscribeServer*) applicationData;
    subServer->notifySubscribers(resourceId,
                                 eventTypeKey,
                                 eventType,
                                 reason);
}

// Send a NOTIFY to all subscribers to the given resourceId and eventTypeKey.
UtlBoolean SipSubscribeServer::notifySubscribers(const char* resourceId,
                                                 const char* eventTypeKey,
                                                 const char* eventType,
                                                 const char* reason)
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipSubscribeServer::notifySubscribers resourceId '%s', eventTypeKey '%s', eventType '%s', reason '%s'",
                 resourceId, eventTypeKey, eventType,
                 reason ? reason : "[null]");
    UtlBoolean notifiedSubscribers = FALSE;
    UtlString eventName(eventType);

    lockForRead();
    SubscribeServerEventData* eventData =
       dynamic_cast <SubscribeServerEventData*>
           (mEventDefinitions.find(&eventName));

    // Get the event-specific info to find subscriptions interested in
    // this content.
    if (eventData)
    {
        int numSubscriptions = 0;
        SipMessage* notifyArray = NULL;
        UtlString* acceptHeaderValuesArray = NULL;
        bool* fullContentArray = NULL;

        // Select the subscriptionChange value to describe what we're doing
        // with the subscriptions.
        enum SipSubscriptionMgr::subscriptionChange change;

        {
           // Select the correct format for generating the Subscription-State value.
           UtlString* formatp = NULL; // We may need a temp UtlString.
           const char* format;

           if (reason == NULL)
           {
              format = "active;expires=%ld";
              change = SipSubscriptionMgr::subscriptionContinues;
           }
           else if (strcmp(reason, terminationReasonSilent) == 0)
           {
              // Do not admit that the subscription is ending.
              format = "active;expires=%ld";
              change = SipSubscriptionMgr::subscriptionTerminatedSilently;
           }
           else if (strcmp(reason, terminationReasonNone) == 0)
           {
              format = "terminated";
              change = SipSubscriptionMgr::subscriptionTerminated;
           }
           else
           {
              // Allocate a UtlString and assemble the Subscription-State format in it.
              formatp = new UtlString();
              formatp->append("terminated;reason=");
              formatp->append(reason);
              format = formatp->data();
              change = SipSubscriptionMgr::subscriptionTerminated;
           }

           // Construct a NOTIFY (without body) for each subscription, containing
           // the dialog-related information about each subscription.
           mpSubscriptionMgr->
              createNotifiesDialogInfo(resourceId,
                                       eventTypeKey,
                                       format,
                                       numSubscriptions,
                                       acceptHeaderValuesArray,
                                       fullContentArray,
                                       notifyArray);

           OsSysLog::add(FAC_SIP, PRI_DEBUG,
                         "SipSubscribeServer::notifySubscribers numSubscriptions for '%s' = %d",
                         resourceId, numSubscriptions);

           // Free the temporary UtlString, if necessary.
           if (formatp)
           {
              delete formatp;
           }
        }

        // For each NOTIFY, add the subscription-related information and then
        // send it.
        for (int notifyIndex = 0;
             notifyIndex < numSubscriptions;
             notifyIndex++)
        {
            SipMessage* notify = &notifyArray[notifyIndex];

            // Check to see if the dialog information could be added.
            // (The subscription might have been destroyed between when
            // it was decided to respond to it, and when the dialog information
            // was retrieved.)
            UtlString callId;
            notify->getCallIdField(&callId);
            if (!callId.isNull())
            {
               if (change != SipSubscriptionMgr::subscriptionTerminatedSilently)
               {
                  // Fill in the NOTIFY request body/content
                  eventData->mpEventSpecificHandler->
                     getNotifyContent(resourceId,
                                      eventTypeKey,
                                      eventType,
                                      *(eventData->mpEventSpecificContentMgr),
                                      acceptHeaderValuesArray[notifyIndex],
                                      *notify,
                                      eventData->mEventSpecificFullState ||
                                      fullContentArray[notifyIndex],
                                      NULL);

                  // Call the application callback to edit the NOTIFY
                  // content if that is required for this event type.
                  // Also gets 'version' (if relevant) and 'savedEventTypeKey'.
                  int version;
                  UtlString savedEventTypeKey;
                  mpSubscriptionMgr->
                     updateNotifyVersion(eventData->mpEventSpecificContentVersionCallback,
                                         *notify,
                                         version,
                                         savedEventTypeKey);

                  // Update the saved record of the NOTIFY CSeq and the
                  // XML version number for the specified savedEventTypeKey,
                  // as needed by the subscription manager.
                  // In practice, this is only used by SipPersistentSubscriptionMgr
                  // to write the NOTIFY Cseq and XML version into the IMDB.
                  mpSubscriptionMgr->
                     updateVersion(*notify, version, savedEventTypeKey);

                  // Set the Contact header.
                  setContact(notify);

                  // Send the NOTIFY request.
                  eventData->mpEventSpecificUserAgent->send(*notify);

                  // Reset the resend timer to the minimum, so if this NOTIFY fails,
                  // we will resend it quickly.
                  UtlString dialogHandle;
                  notify->getDialogHandleReverse(dialogHandle);
                  mpSubscriptionMgr->newNotify(dialogHandle);
               }

               if (change != SipSubscriptionMgr::subscriptionContinues)
               {
                  // Remove the record of the subscription.
                  UtlString dialogHandle;
                  notify->getDialogHandle(dialogHandle);
                  mpSubscriptionMgr->endSubscription(dialogHandle, change);
               }
            }
        }

        // Free the allocated arrays.
        delete[] acceptHeaderValuesArray;
        delete[] notifyArray;
        delete[] fullContentArray;
    }
    // event type not enabled
    else
    {
        OsSysLog::add(FAC_SIP, PRI_ERR,
                      "SipSubscribeServer::notifySubscribers "
                      "event type: %s not enabled - "
                      "Why are we seeing a callback for this?",
            eventName.data());
    }

    unlockForRead();

    return notifiedSubscribers;
}

UtlBoolean SipSubscribeServer::enableEventType(const char* eventTypeToken,
                                               SipUserAgent* userAgent,
                                               SipPublishContentMgr* contentMgr,
                                               SipSubscribeServerEventHandler* eventHandler,
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
        eventData = new SubscribeServerEventData(eventName);
        eventData->mpEventSpecificUserAgent =
           userAgent ? userAgent : mpDefaultUserAgent;
        eventData->mpEventSpecificContentMgr =
           contentMgr ? contentMgr : mpDefaultContentMgr;
        eventData->mpEventSpecificHandler =
           eventHandler ? eventHandler : mpDefaultEventHandler;
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
                                    contentChangeCallback,
                                    this);
    }

    unlockForWrite();

    return(addedEvent);
}

UtlBoolean SipSubscribeServer::disableEventType(const char* eventType)
{
   // :TODO: We should terminate all subscriptions for this event type.

   UtlString eventName(eventType);
   lockForWrite();

   SubscribeServerEventData* eventData =
      dynamic_cast <SubscribeServerEventData*> (mEventDefinitions.remove(&eventName));
   if (eventData)
   {
      // Unregister interest in SUBSCRIBE requests and NOTIFY
      // responses for this event type
      eventData->mpEventSpecificUserAgent->removeMessageObserver(*(getMessageQueue()));

      delete eventData;
   }

   unlockForWrite();

   return eventData != NULL;
}

UtlBoolean SipSubscribeServer::handleMessage(OsMsg &message)
{
    int msgType = message.getMsgType();
    int msgSubType = message.getMsgSubType();

    // Timer fired
    if (msgType == OsMsg::OS_EVENT &&
        msgSubType == OsEventMsg::NOTIFY)
    {
        OsTimer* timer = 0;
        UtlString* subscribeDialogHandle = NULL;
        intptr_t timerIntptr;
        void* subscribeDialogHandleVoid;

        ((OsEventMsg&)message).getUserData(subscribeDialogHandleVoid);
        ((OsEventMsg&)message).getEventData(timerIntptr);
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
            // handleExpiration deals with that and may reuse the timer
        }
    }

    // Resend timer fired
    else if (msgType == OsMsg::OS_EVENT &&
             msgSubType == SipSubscriptionMgr::sEventResend)
    {
       handleResendEvent(
          *SipSubscriptionMgr::getHandleOfResendEventMsg(message));
    }

    // SIP message
    else if (msgType == OsMsg::PHONE_APP &&
             msgSubType == SipMessage::NET_SIP_MESSAGE)
    {
        const SipMessage* sipMessage = ((SipMessageEvent&)message).getMessage();

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
   // The SipSubscribeServer does not depend on eventType.
   return mpSubscriptionMgr;
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
   mpSubscriptionMgr->dumpState();
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
    subscribeRequest.getEventFieldParts(&eventName);

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
                mpSubscriptionMgr->
                   updateDialogInfo(
                      subscribeRequest,
                      resourceId,
                      eventTypeKey,
                      eventType,
                      subscribeDialogHandle,
                      isNewDialog,
                      isExpiredSubscription,
                      subscribeResponse,
                      // The event-specific handler provides a getKeys method
                      // which is used to determine the resource ID
                      // and event type if this is a new subscription.
                      *handler);

                 // Build a NOTIFY
                 SipMessage notifyRequest;

                 // Set the dialog information into the NOTIFY.
                 // Note that the dialog can have ended by now, because of
                 // a race condition with the processing of dialog-ending
                 // messages from the outside world.
                 if (mpSubscriptionMgr->
                     getNotifyDialogInfo(subscribeDialogHandle,
                                         notifyRequest,
                                         "active;expires=%ld"))
                 {
                    // We still have record of the dialog, so the
                    // NOTIFY headers were set.

                    // Set the NOTIFY content
                    UtlString acceptHeaderValue;
                    if (!subscribeRequest.getAcceptField(acceptHeaderValue))
                    {
                       // No Accept header seen, set special value allowing any
                       // content type.
                       acceptHeaderValue = SipPublishContentMgr::acceptAllTypes;
                    }
                    // Note that since this NOTIFY is due to a SUBSCRIBE,
                    // it should contain 'full' content.  Hence,
                    // the fullState parameter of getNotifyContent is TRUE,
                    // and is not taken from
                    // eventPackageInfo->mEventSpecificFullState.
                    UtlString availableMediaTypes;
                    if (handler->getNotifyContent(resourceId,
                                                  eventTypeKey,
                                                  eventType,
                                                  *(eventPackageInfo->mpEventSpecificContentMgr),
                                                  acceptHeaderValue,
                                                  notifyRequest,
                                                  TRUE,
                                                  &availableMediaTypes))
                    {
                       // Update the NOTIFY content if required for this event type.
                       // Sets 'version' and 'eventTypeKey'.
                       int version;
                       mpSubscriptionMgr->
                          updateNotifyVersion(eventPackageInfo->mpEventSpecificContentVersionCallback,
                                              notifyRequest,
                                              version,
                                              eventTypeKey);

                       // Update the saved record of the NOTIFY CSeq and the
                       // XML version number for the specified eventTypeKey,
                       // as needed by the subscription manager.
                       // In practice, this is only used by SipPersistentSubscriptionMgr
                       // to write the NOTIFY Cseq and XML version into the IMDB.
                       mpSubscriptionMgr->
                          updateVersion(notifyRequest, version, eventTypeKey);

                       // Send the NOTIFY request
                       setContact(&notifyRequest);
                       eventPackageInfo->mpEventSpecificUserAgent->send(notifyRequest);

                       // Reset the resend timer to the minimum, so if this NOTIFY fails,
                       // we will resend it quickly.
                       UtlString dialogHandle;
                       notifyRequest.getDialogHandleReverse(dialogHandle);
                       mpSubscriptionMgr->newNotify(dialogHandle);

                       if (OsSysLog::willLog(FAC_SIP, PRI_INFO))
                       {
                          UtlString requestContact;
                          UtlString callId;
                          int expires;
                          subscribeRequest.getContactField(0, requestContact);
                          subscribeRequest.getCallIdField(&callId);
                          subscribeResponse.getExpiresField(&expires);
                          if (expires > 0)
                          {
                             OsSysLog::add(FAC_SIP, PRI_INFO,
                                 "SipSubscribeServer::handleSubscribe: %s has set up subscription to %s, eventTypeKey %s, callId %s, expires %d",
                                 requestContact.data(), resourceId.data(), eventTypeKey.data(), callId.data(), expires);
                          }
                          else
                          {
                             OsSysLog::add(FAC_SIP, PRI_INFO,
                                 "SipSubscribeServer::handleSubscribe: %s has terminated subscription to %s, eventTypeKey %s, callId %s",
                                 requestContact.data(), resourceId.data(), eventTypeKey.data(), callId.data());
                          }
                       }
                    }
                    else
                    {
                       // No content was available, so the subscription fails.

                       // Determine the reason and set the response code.
                       if (availableMediaTypes.isNull())
                       {
                          // No MIME types are available, so the resource does not exist.
                          subscribeResponse.setResponseFirstHeaderLine(SIP_PROTOCOL_VERSION,
                                                                       SIP_NOT_FOUND_CODE,
                                                                       SIP_NOT_FOUND_TEXT);
                       }
                       else
                       {
                          // MIME types are available, so the resource exists.
                          subscribeResponse.setResponseFirstHeaderLine(SIP_PROTOCOL_VERSION,
                                                                       SIP_BAD_MEDIA_CODE,
                                                                       SIP_BAD_MEDIA_TEXT);
                          subscribeResponse.setAcceptField(availableMediaTypes);
                       }

                       // Destroy the subscription.
                       mpSubscriptionMgr->
                          endSubscription(subscribeDialogHandle,
                                          SipSubscriptionMgr::subscriptionTerminated);

                       // Do not send the NOTIFY.
                    }
                 }
                 else
                 {
                    // Oops, the subscription was destroyed while we looked.
                    OsSysLog::add(FAC_SIP, PRI_WARNING,
                                  "SipSubscribeServer::handleSubscribe "
                                  "subscription '%s' vanished while being processed",
                                  subscribeDialogHandle.data());
                 }

                 // Send the response ASAP to minimize resending.
                 setContact(&subscribeResponse);
                 eventPackageInfo->mpEventSpecificUserAgent->send(subscribeResponse);
            }
            // Not authorized
            else
            {
                // Send the response that was prepared by ::isAuthorized().
                setContact(&subscribeResponse);
                eventPackageInfo->mpEventSpecificUserAgent->send(subscribeResponse);
            }
        }

        // Not authenticated
        else
        {
            // Send the response that was prepared by ::isAuthenticated().
            setContact(&subscribeResponse);
            eventPackageInfo->mpEventSpecificUserAgent->send(subscribeResponse);
        }
    }

    // We should not have received SUBSCRIBE requests for this event type.
    // This event type has not been enabled in this SubscribeServer.
    else
    {
        OsSysLog::add(FAC_SIP, PRI_ERR,
            "SipSubscribeServer::handleSubscribe event type: %s not enabled",
            eventName.data());

        SipMessage eventTypeNotHandled;
        eventTypeNotHandled.setResponseData(&subscribeRequest,
                                            SIP_BAD_EVENT_CODE,
                                            SIP_BAD_EVENT_TEXT);

        setContact(&eventTypeNotHandled);
        mpDefaultUserAgent->send(eventTypeNotHandled);
    }
    unlockForRead();

    return(handledSubscribe);
}

void SipSubscribeServer::handleNotifyResponse(const SipMessage& notifyResponse)
{
    int responseCode = notifyResponse.getResponseStatusCode();
    UtlString dialogHandle;
    notifyResponse.getDialogHandleReverse(dialogHandle);

    // If it is a non-Timeout error response and has no Retry-After header,
    // terminate the subscription.
    if (responseCode >= SIP_3XX_CLASS_CODE)
    {
       if (   SIP_REQUEST_TIMEOUT_CODE == responseCode
           || SIP_LOOP_DETECTED_CODE == responseCode
           || SIP_SERVER_INTERNAL_ERROR_CODE == responseCode)
       {
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

          // Similar problems arise with the 500 response, which usually
          // indicates that a request was retried with a second transport.
          // But note that SipTransaction translates 503 into 500,
          // so 503 is also exempt.

          // Log the response.
          OsSysLog::add(FAC_SIP, PRI_DEBUG,
                        "SipSubscribeServer::handleNotifyResponse "
                        "Not terminating subscription due to %d response. Handle: %s",
                        responseCode, dialogHandle.data());

          // This is considered a transient error condition, so schedule a
          // resend of the NOTIFY.
          mpSubscriptionMgr->startResendTimer(dialogHandle);
       }
       else if (notifyResponse.getHeaderValue(0, SIP_RETRY_AFTER_FIELD) == NULL)
       {
          OsSysLog::add(FAC_SIP, PRI_WARNING,
                        "SipSubscribeServer::handleNotifyResponse "
                        "Terminating subscription due to %d response. Handle: %s",
                        responseCode, dialogHandle.data());

          // End this subscription because we got an error response from
          // the NOTIFY request.
          generateTerminatingNotify(dialogHandle);
       }
       else
       {
          // This was an error response, but because there was a Retry-After
          // header, we should not terminate the subscription.
          OsSysLog::add(FAC_SIP, PRI_DEBUG,
                        "SipSubscribeServer::handleNotifyResponse "
                        "Not terminating subscription due to Retry-After header. Handle: %s",
                        dialogHandle.data());
       }
    }
    else
    {
       // This was a success response, so clear the flag to send full content
       // in the next NOTIFY.
       mpSubscriptionMgr->successResponse(dialogHandle);
    }
}

// Handle a resend event message.
void SipSubscribeServer::handleResendEvent(const UtlString& dialogHandle)
{
   // Not modifying the SubscribeServerEventData, just reading it.
   lockForRead();

   // The NOTIFY request that we will send.
   SipMessage notifyRequest;

   // Look up the information about the subscription.
   UtlString resourceId;
   UtlString eventTypeKey;
   UtlString eventType;
   UtlString acceptHeaderValue;
   bool fullContent;

   // Returns TRUE if this subscription is recorded in mpSubscriptionMgr.
   if (mpSubscriptionMgr->
       getNotifyDialogInfo(dialogHandle,
                           notifyRequest,
                           "active;expires=%ld",
                           &resourceId,
                           &eventTypeKey,
                           &eventType,
                           &acceptHeaderValue,
                           &fullContent))
   {
      // Get the event-specific handler and information.
      SubscribeServerEventData* eventPackageInfo =
         dynamic_cast <SubscribeServerEventData*>
         (mEventDefinitions.find(&eventType));
      if (eventPackageInfo)
      {
         // There is still record of the subscription.

         // The NOTIFY headers were set by getNotifyDialogInfo.

         // Get the body for the NOTIFY.
         if (eventPackageInfo->mpEventSpecificHandler->
             getNotifyContent(resourceId,
                              eventTypeKey,
                              eventType,
                              *(eventPackageInfo->mpEventSpecificContentMgr),
                              acceptHeaderValue,
                              notifyRequest,
                              eventPackageInfo->mEventSpecificFullState ||
                              fullContent,
                              NULL))
         {
            // Update the NOTIFY content if required for this event type.
            // Sets 'version' and 'eventTypeKey'.
            int version;
            mpSubscriptionMgr->
               updateNotifyVersion(eventPackageInfo->mpEventSpecificContentVersionCallback,
                                   notifyRequest,
                                   version,
                                   eventTypeKey);

            // Update the saved record of the NOTIFY CSeq and the
            // XML version number for the specified eventTypeKey,
            // as needed by the subscription manager.
            // In practice, this is only used by SipPersistentSubscriptionMgr
            // to write the NOTIFY Cseq and XML version into the IMDB.
            mpSubscriptionMgr->
               updateVersion(notifyRequest, version, eventTypeKey);

            // Send the NOTIFY request
            setContact(&notifyRequest);
            eventPackageInfo->mpEventSpecificUserAgent->send(notifyRequest);
         }
      }
   }

   unlockForRead();
}

// End a subscription because we got an error response from a NOTIFY request.
// This requires sending a terminating NOTIFY.
void SipSubscribeServer::generateTerminatingNotify(const UtlString& dialogHandle)
{
   // Sending a terminating NOTIFY presents the risk of an infinite
   // loop, as the terminating NOTIFY may get an error response.  We
   // prevent this loop by removing record of the subscription as soon
   // as we send the terminating NOTIFY, and only initiating sending a
   // terminating NOTIFY if record of the subscription exists.

   // Not modifying the SubscribeServerEventData, just reading it.
   lockForRead();

   // The terminating NOTIFY request that we will send.
   SipMessage notifyRequest;

   // Look up the information about the subscription.
   UtlString resourceId;
   UtlString eventTypeKey;
   UtlString eventType;
   UtlString acceptHeaderValue;
   bool fullContent;

   // Returns TRUE if this subscription is recorded in mpSubscriptionMgr.
   if (mpSubscriptionMgr->
       getNotifyDialogInfo(dialogHandle,
                           notifyRequest,
                           "terminated;reason=deactivated",
                           &resourceId,
                           &eventTypeKey,
                           &eventType,
                           &acceptHeaderValue,
                           &fullContent))
   {
      // Get the event-specific handler and information.
      SubscribeServerEventData* eventPackageInfo =
         dynamic_cast <SubscribeServerEventData*>
         (mEventDefinitions.find(&eventType));
      if (eventPackageInfo)
      {
         // There is still record of the subscription (hence a
         // terminating NOTIFY has not yet been sent).

         // The NOTIFY headers were set by getNotifyDialogInfo.

         // Get the body for the NOTIFY.
         if (eventPackageInfo->mpEventSpecificHandler->
             getNotifyContent(resourceId,
                              eventTypeKey,
                              eventType,
                              *(eventPackageInfo->mpEventSpecificContentMgr),
                              acceptHeaderValue,
                              notifyRequest,
                              eventPackageInfo->mEventSpecificFullState ||
                              fullContent,
                              NULL))
         {
            // Update the NOTIFY content if required for this event type.
            // Sets 'version' and 'eventTypeKey'.
            int version;
            mpSubscriptionMgr->
               updateNotifyVersion(eventPackageInfo->mpEventSpecificContentVersionCallback,
                                   notifyRequest,
                                   version,
                                   eventTypeKey);

            // Update the saved record of the NOTIFY CSeq and the
            // XML version number for the specified eventTypeKey,
            // as needed by the subscription manager.
            // In practice, this is only used by SipPersistentSubscriptionMgr
            // to write the NOTIFY Cseq and XML version into the IMDB.
            mpSubscriptionMgr->
               updateVersion(notifyRequest, version, eventTypeKey);

            // Send the NOTIFY request
            setContact(&notifyRequest);
            eventPackageInfo->mpEventSpecificUserAgent->send(notifyRequest);
         }

         // Destroy the subscription.
         // This prevents this method from sending a second terminating
         // NOTIFY for this subscription, even if it is called again.
         mpSubscriptionMgr->
            endSubscription(dialogHandle,
                            SipSubscriptionMgr::subscriptionTerminated);
      }
   }

   unlockForRead();
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
