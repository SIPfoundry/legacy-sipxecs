// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES

#include <os/OsDateTime.h>
#include <net/SipSubscribeClient.h>
#include <net/SipUserAgent.h>
#include <net/SipDialog.h>
#include <net/SipDialogMgr.h>
#include <net/NetMd5Codec.h>
#include <net/CallId.h>
#include <utl/UtlHashBagIterator.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

// Private class to contain subscription client states
class SubscribeClientState : public UtlString
{
public:
    // The parent UtlString contains the dialogHandle as a key
    // It is the early dialog handle until the dialog is established,
    // after which it is changed to the established dialog handle.

    SubscribeClientState();

    virtual ~SubscribeClientState();

    void toString(UtlString& dumpString);

    //! Copying operators.
    SubscribeClientState(const SubscribeClientState& rSubscribeClientState);
    SubscribeClientState& operator=(const SubscribeClientState& rhs);

    // UtlString::data contains the Dialog Handle.
    SipSubscribeClient::SubscriptionState mState;
    void* mpApplicationData;
    // Callback function for state changes, or NULL.
    SipSubscribeClient::SubscriptionStateCallback mpStateCallback;
    SipSubscribeClient::NotifyEventCallback mpNotifyCallback;

private:
};


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor for private class
SubscribeClientState::SubscribeClientState()
{
    mpApplicationData = NULL;
    mpStateCallback = NULL;
    mpNotifyCallback = NULL;
}

// Destructor for private class
SubscribeClientState::~SubscribeClientState()
{
}

// Debug dump of private class client state
void SubscribeClientState::toString(UtlString& dumpString)
{
    dumpString = "SubscribeClientState:\n";
    dumpString.append("\nmpData: ");
    dumpString.append(*this);
    dumpString.append("\nmState: ");
    UtlString subStateString;
    SipSubscribeClient::getSubscriptionStateEnumString(mState, subStateString);
    dumpString.append(subStateString);
    dumpString.append("\nmpApplicationData: ");
    char pointerString[20];
    sprintf(pointerString, "%p", mpApplicationData);
    dumpString.append(pointerString);
    dumpString.append("\nmpStateCallback: ");
    sprintf(pointerString, "%p", mpStateCallback);
    dumpString.append(pointerString);
    dumpString.append("\nmpNotifyCallback: ");
    sprintf(pointerString, "%p", mpNotifyCallback);
    dumpString.append(pointerString);
    dumpString.append('\n');
}

//! Copying operators.
SubscribeClientState::SubscribeClientState(const SubscribeClientState& rSubscribeClientState)
{
   static_cast <UtlString&> (*this) =
      static_cast <const UtlString&> (rSubscribeClientState);
   mState = rSubscribeClientState.mState;
   mpApplicationData = rSubscribeClientState.mpApplicationData;
   mpStateCallback = rSubscribeClientState.mpStateCallback;
   mpNotifyCallback = rSubscribeClientState.mpNotifyCallback;
}

SubscribeClientState& SubscribeClientState::operator=(const SubscribeClientState& rhs)
{
   static_cast <UtlString&> (*this) = static_cast <const UtlString&> (rhs);
   mState = rhs.mState;
   mpApplicationData = rhs.mpApplicationData;
   mpStateCallback = rhs.mpStateCallback;
   mpNotifyCallback = rhs.mpNotifyCallback;

   return *this;
}

// Constructor
SipSubscribeClient::SipSubscribeClient(SipUserAgent& userAgent, 
                                       SipDialogMgr& dialogMgr,
                                       SipRefreshManager& refreshMgr)
    : OsServerTask("SipSubscribeClient-%d")
    , mSubscribeClientMutex(OsMutex::Q_FIFO)
{
    mpUserAgent = &userAgent;
    mpDialogMgr = &dialogMgr;
    mpRefreshMgr = &refreshMgr;
}

// Copy constructor
SipSubscribeClient::SipSubscribeClient(const SipSubscribeClient& rSipSubscribeClient)
: mSubscribeClientMutex(OsMutex::Q_FIFO)
{
   assert(FALSE);
}

// Destructor
SipSubscribeClient::~SipSubscribeClient()
{
    // Do not delete mpUserAgent, mpDialogMgr or mpRefreshMgr.  They
    // may be used elsewhere and need to be deleted outside the
    // SipSubscribeClient.

    // Stop receiving NOTIFY requests
    mpUserAgent->removeMessageObserver(*(getMessageQueue()));

    // Wait until this OsServerTask has stopped or handleMethod
    // might access something we are about to delete here.
    waitUntilShutDown();

    // Delete the event type strings
    mEventTypes.destroyAll();

    // Unsubscribe to anything that is in the list
    endAllSubscriptions();
    // mSubscriptions should now be empty
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
SipSubscribeClient& 
SipSubscribeClient::operator=(const SipSubscribeClient& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   // unimplemented
   assert(FALSE);
   return *this;
}

UtlBoolean SipSubscribeClient::addSubscription(
   const char* resourceId,
   const char* eventHeaderValue,
   const char* acceptHeaderValue,
   const char* fromFieldValue,
   const char* toFieldValue,
   const char* contactFieldValue,
   int subscriptionPeriodSeconds,
   void* applicationData,
   const SubscriptionStateCallback subscriptionStateCallback,
   const NotifyEventCallback notifyEventsCallback,
   UtlString& earlyDialogHandle)
{
    UtlString callId;
    CallId::getNewCallId(callId);

    // Construct a SUBSCRIBE request
    SipMessage subscribeRequest;
    subscribeRequest.setSubscribeData(resourceId,
                                      fromFieldValue,
                                      toFieldValue,
                                      callId,
                                      1, // cseq
                                      eventHeaderValue,
                                      acceptHeaderValue,
                                      NULL, // Event header id parameter
                                      contactFieldValue,
                                      NULL, // initial request no routeField
                                      subscriptionPeriodSeconds);

    // Create a subscription (i.e. send the request and keep the
    // subscription refreshed).
    return (addSubscription(subscribeRequest, 
                            applicationData,
                            subscriptionStateCallback,
                            notifyEventsCallback,
                            earlyDialogHandle));
}

UtlBoolean SipSubscribeClient::addSubscription(
   SipMessage& subscriptionRequest,
   void* applicationData,
   const SubscriptionStateCallback subscriptionStateCallback,
   const NotifyEventCallback notifyEventsCallback,
   UtlString& earlyDialogHandle)
{
    // Set the from tag if it is not already set.
    Url fromUrl;
    subscriptionRequest.getFromUrl(fromUrl);
    UtlString fromTag;
    fromUrl.getFieldParameter("tag",fromTag);
    if (fromTag.isNull())
    {
        UtlString fromFieldValue;
        fromUrl.toString(fromFieldValue);
        CallId::getNewTag("", fromTag);
        fromUrl.setFieldParameter("tag", fromTag);
        fromUrl.toString(fromFieldValue);
        subscriptionRequest.setRawFromField(fromFieldValue);
    }

    // Get the event type and make sure we are registered to
    // receive NOTIFY requests for this event type.
    UtlString eventType;
    subscriptionRequest.getEventField(&eventType, NULL, NULL);
    // If this event type is not in the list, we need to register
    // to receive SUBSCRIBE responses for this event type
    lock();
    if (mEventTypes.find(&eventType) == NULL)
    {
        // receive NOTIFY requests for this event type
        mpUserAgent->addMessageObserver(*(getMessageQueue()), 
                                        SIP_NOTIFY_METHOD,
                                        TRUE, // yes requests
                                        FALSE, // no responses
                                        TRUE, // incoming,
                                        FALSE, // outgoing
                                        eventType);

        // Note: we do not register to receive SUBSCRIBE responses
        // as the refreshManager will do that and invoke the 
        // SubScribeClient's callback.

        // Add this event type to the list so we know we
        // have registered for the responses with the user agent
        mEventTypes.insert(new UtlString(eventType));
    }
    unlock();

    // Create a SubscribeState and set the members
    SubscribeClientState* clientState = new SubscribeClientState;
    subscriptionRequest.getDialogHandle(*clientState);
    clientState->mState = SUBSCRIPTION_INITIATED;
    clientState->mpApplicationData = applicationData;
    clientState->mpStateCallback = subscriptionStateCallback;
    clientState->mpNotifyCallback = notifyEventsCallback;
    earlyDialogHandle = *clientState;

    // Put the state in the list
    lock();
    addState(*clientState);
    unlock();

    // Give the request to the refresh manager to send the
    // subscribe and keep the subscription alive
    UtlBoolean initialSendOk = 
        mpRefreshMgr->initiateRefresh(subscriptionRequest,
                                      this,
                                      // refreshCallback receives 'this' as app. data
                                      SipSubscribeClient::refreshCallback,
                                      earlyDialogHandle);

    return (initialSendOk);
}

UtlBoolean SipSubscribeClient::endSubscription(const char* dialogHandle)
{
    UtlBoolean foundSubscription = FALSE;
    UtlString matchDialog(dialogHandle);
    lock();
    SubscribeClientState* clientState = removeState(matchDialog);
    unlock();

    if (clientState)
    {
        foundSubscription = TRUE;
        // If there is a state change of interest and
        // there is a callback function
        if (clientState->mState != SUBSCRIPTION_TERMINATED &&
            clientState->mpStateCallback)
        {
            UtlBoolean isEarlyDialog = mpDialogMgr->earlyDialogExists(matchDialog);

            // Indicate that the subscription was terminated
            (clientState->mpStateCallback)(SUBSCRIPTION_TERMINATED,
                                     isEarlyDialog ? dialogHandle : NULL,
                                     isEarlyDialog ? NULL : dialogHandle,
                                     clientState->mpApplicationData,
                                     -1, // no response code
                                     NULL, // no response text
                                     0, // expires now
                                     NULL); // no response
        }
        delete clientState;
    }

    // Did not find a dialog to match
    else
    {
        // dialogHandle may be a handle to an early dialog.
        // See if we can find the dialog matching an early dialog handle.
        // It is possible that there is more than one dialog that matches
        // this early dialog handle; if so, end all of them.
        UtlString earlyDialogHandle;
        while (mpDialogMgr->getEarlyDialogHandleFor(matchDialog, earlyDialogHandle))
        {
            lock();
            clientState = removeState(earlyDialogHandle);
            unlock();

            if (clientState)
            {
                foundSubscription = TRUE;
                // If there is a state change of interest and
                // there is a callback function
                if (clientState->mState != SUBSCRIPTION_TERMINATED &&
                    clientState->mpStateCallback)
                {
                    // Indicate that the subscription was terminated
                    (clientState->mpStateCallback)(SUBSCRIPTION_TERMINATED,
                                             earlyDialogHandle,
                                             dialogHandle,
                                             clientState->mpApplicationData,
                                             -1, // no response code
                                             NULL, // no response text
                                             0, // expires now
                                             NULL); // no response
                }
                delete clientState;
            }
        }
    }

    // Stop the refresh and unsubscribe
    UtlBoolean foundRefreshSubscription = mpRefreshMgr->stopRefresh(dialogHandle);

    return (foundSubscription || foundRefreshSubscription);
}

void SipSubscribeClient::endAllSubscriptions()
{
#ifdef TIME_LOG
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipSubscribeClient::endAllSubscriptions entered");
#endif

   // In order to avoid deadlocks, we first remove all entries from
   // mSubscriptions, and then process them.

   lock();
   SubscribeClientState** clientStateList =
      new SubscribeClientState*[2 * mSubscriptions.entries()];
   int subscriptions = 0;
   {
      UtlHashBagIterator iterator(mSubscriptions);
      SubscribeClientState* dialogKey;
      while ((dialogKey = dynamic_cast <SubscribeClientState*> (iterator())))
      {
         SubscribeClientState* clientState = removeState(*dialogKey);
         if (clientState)
         {
            clientStateList[subscriptions++] = clientState;
            clientStateList[subscriptions++] = dialogKey;
         }
      }
   }
   unlock();

   for (int i = 0; i < subscriptions; i += 2)
   {
      SubscribeClientState* clientState = clientStateList[i];
      SubscribeClientState* dialogKey = clientStateList[i + 1];

      lock();
          
      // If there is a state change of interest and
      // there is a callback function
      if (clientState->mState != SUBSCRIPTION_TERMINATED &&
          clientState->mpStateCallback)
      {
         UtlString earlyDialogHandle;
         mpDialogMgr->getEarlyDialogHandleFor(*dialogKey, earlyDialogHandle);

         // Indicate that the subscription was terminated
         (clientState->mpStateCallback)(
            SUBSCRIPTION_TERMINATED,
            clientState->mState == SUBSCRIPTION_INITIATED ? earlyDialogHandle.data() : NULL,
            clientState->mState == SUBSCRIPTION_SETUP ? dialogKey->data() : NULL,
            clientState->mpApplicationData,
            -1, // no response code
            NULL, // no response text
            0, // expires now
            NULL); // no response
      }

      UtlString dialogKeyAsString(*dialogKey);

      delete clientState;

      unlock();

      // Unsubscribe and stop refreshing the subscription.
      // Note: this next operation is performed outside of the 
      // critical section to avoid deadlocks.  See XECS-1988.
      mpRefreshMgr->stopRefresh(dialogKeyAsString.data());

      // Allow other threads waiting for the lock to run.
      OsTask::yield();
   }

   delete[] clientStateList;

#ifdef TIME_LOG
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipSubscribeClient::endAllSubscriptions exited %d entries",
                 subscriptions/2);
#endif
}

UtlBoolean SipSubscribeClient::handleMessage(OsMsg &eventMessage)
{
    int msgType = eventMessage.getMsgType();
    int msgSubType = eventMessage.getMsgSubType();

    // SIP message
    if (msgType == OsMsg::PHONE_APP &&
        msgSubType == SipMessage::NET_SIP_MESSAGE)
    {
        const SipMessage* sipMessage = ((SipMessageEvent&)eventMessage).getMessage();

        // If this is a NOTIFY request
        if (sipMessage)
        {
           UtlString method;
           sipMessage->getRequestMethod(&method);
           if (method.compareTo(SIP_NOTIFY_METHOD) == 0 &&
               !sipMessage->isResponse())
           {
              handleNotifyRequest(*sipMessage);
           }
           else
           {
              OsSysLog::add(FAC_SIP, PRI_ERR,
                            "SipSubscribeClient::handleMessage unexpected %s %s",
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

    return (TRUE);
}

/* ============================ ACCESSORS ================================= */

int SipSubscribeClient::countSubscriptions()
{
    int count = 0;
    lock();
    count = mSubscriptions.entries();
    unlock();
    return count;
}

int SipSubscribeClient::dumpStates(UtlString& dumpString)
{
    int count = 0;
    dumpString.remove(0);
    UtlString oneClientDump;
    SubscribeClientState* clientState = NULL;
    lock();
    UtlHashBagIterator iterator(mSubscriptions);
    while ((clientState = dynamic_cast <SubscribeClientState*> (iterator())))
    {
        clientState->toString(oneClientDump);
        dumpString.append(oneClientDump);

        count++;
    }
    unlock();

    return count;
}

void SipSubscribeClient::getSubscriptionStateEnumString(enum SubscriptionState stateValue, 
                                                        UtlString& stateString)
{
    switch(stateValue)
    {
    case SUBSCRIPTION_INITIATED: // Early dialog
        stateString = "SUBSCRIPTION_INITIATED";
        break;

    case SUBSCRIPTION_SETUP:     // Established dialog
        stateString = "SUBSCRIPTION_SETUP";
        break;

    case SUBSCRIPTION_TERMINATED: // Failed during setup or refresh, or ended by application.
        stateString = "SUBSCRIPTION_TERMINATED";
        break;

    default:
        {
            char stateNum[20];
            sprintf(stateNum, "%d", stateValue);
            stateString = "INVALID: ";
            stateString.append(stateNum);
        }
        break;
    }
}

/* ============================ INQUIRY =================================== */

// Dump the object's internal state.
void SipSubscribeClient::dumpState()
{
   lock();

   // indented 2

   OsSysLog::add(FAC_RLS, PRI_INFO,
                 "\t  SipSubscribeClient %p",
                 this);

   UtlString oneClientDump;
   SubscribeClientState* clientState;
   UtlHashBagIterator iterator(mSubscriptions);
   while ((clientState = dynamic_cast <SubscribeClientState*> (iterator())))
   {
      clientState->toString(oneClientDump);
      OsSysLog::add(FAC_RLS, PRI_INFO,
                    "\t    SubscribeClientState %p %s",
                    clientState, oneClientDump.data());

   }
   mpRefreshMgr->dumpState();

   unlock();
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

void SipSubscribeClient::refreshCallback(SipRefreshManager::RefreshRequestState newState,
                                         const char* earlyDialogHandle,
                                         const char* dialogHandle,
                                         void* subscribeClientPtr,
                                         int responseCode,
                                         const char* responseText,
                                         long expirationDate, // epoch seconds
                                         const SipMessage* subscribeResponse)
{
   ((SipSubscribeClient*) subscribeClientPtr)->
      refreshCallback(newState,
                      earlyDialogHandle,
                      dialogHandle,
                      responseCode,
                      responseText,
                      expirationDate,
                      subscribeResponse);
}

void SipSubscribeClient::refreshCallback(SipRefreshManager::RefreshRequestState newState,
                                         const char* earlyDialogHandle,
                                         const char* dialogHandle,
                                         int responseCode,
                                         const char* responseText,
                                         long expirationDate, // epoch seconds
                                         const SipMessage* subscribeResponse)
{
   long now = OsDateTime::getSecsSinceEpoch();
   switch(newState)
   {
      // Do nothing for early dialog for subscribes
   case SipRefreshManager::REFRESH_REQUEST_PENDING:
      break;

   case SipRefreshManager::REFRESH_REQUEST_SUCCEEDED:
   { // Variable scope
      // Either the subscription dialog went from early to established,
      // or a second dailog was created from the early dialog.  Hense
      // there may be more than one established dialog
      // Determine which case we have:
      //   1) transition of early dialog to established
      //   2) second or subsequent dialog established
      //   3) refresh failed, but subscription not expired yet

      lock();
      SubscribeClientState* clientState = NULL;

      // See if we can find any state for the early dialog
      if (earlyDialogHandle && *earlyDialogHandle)
      {
         UtlString dialogString(earlyDialogHandle);
         clientState = removeState(dialogString);

         // If we could not find the state for the early dialog
         // this must callback must be for a second dialog created from the
         // same early dialog (as evidenced by a second response to
         // the SUBSCRIBE).
         if (clientState == NULL)
         {
            OsSysLog::add(FAC_SIP, PRI_DEBUG, 
                          "SipSubscribeClient::refreshCallback second established dialog for early dialog: %s",
                          earlyDialogHandle);
            UtlString establishedDialogHandle;
            if (mpDialogMgr->getEstablishedDialogHandleFor(earlyDialogHandle,
                                                           establishedDialogHandle))
            {
               SubscribeClientState* establishedState = getState(establishedDialogHandle);
               if (establishedState)
               {
                  OsSysLog::add(FAC_SIP, PRI_DEBUG, 
                                "SipSubscribeClient::refreshCallback state duplicated from dialog: %s",
                                establishedDialogHandle.data());
                  clientState = new SubscribeClientState(*establishedState);
                  static_cast <UtlString&> (*clientState) = dialogHandle;
                  clientState->mState = SUBSCRIPTION_SETUP;
                  addState(*clientState);
               }
            }
         }
         else
         {
            // Change the dialogHandle as we switched from an
            // early to an established dialog
            // The SubscribeClientState was removed from the hashbag
            // above; change the key and put it back in.
            static_cast <UtlString&> (*clientState) = dialogHandle;
            clientState->mState = SUBSCRIPTION_SETUP;
            addState(*clientState);
         }
      }
      else
      {
         UtlString dialogString(dialogHandle);
         clientState = getState(dialogString);
      }
      // clientState is the subscription state in question,
      // or NULL if none was found.

      // If the response code is > 299, the reSUBSCRIBE failed,
      // but a prior SUBSCRIBE has not expired yet

      if (clientState)
      {
         if (expirationDate < now)
         {
            clientState->mState = SUBSCRIPTION_TERMINATED;
         }
         else
         {
            clientState->mState = SUBSCRIPTION_SETUP;
         }

         if (clientState->mpStateCallback)
         {
            (clientState->mpStateCallback)(clientState->mState,
                                           earlyDialogHandle,
                                           dialogHandle,
                                           clientState->mpApplicationData,
                                           responseCode, 
                                           responseText,
                                           expirationDate,
                                           subscribeResponse);
         }

         // We do not remove the subscription state.  It is the
         // application's job to explicitly call endSubscription.
      }
      unlock();
   }
   break;

   case SipRefreshManager::REFRESH_REQUEST_FAILED:
   { // Variable scope
      // Early or established dialog failed.  If the dialog was
      // established and the subscription had not expired yet
      // we would have recieved a DIALOG_ESTABLISHED with an
      // error response code.  So if the state is DIALOG_FAILED,
      // either the early dialog failed, or the subscription
      // has expired and the reSUBSCRIBE failed.  We do not hear
      // about SUBSCRIBEs that fail due to authorization unless
      // there is no matching credentials or the credentials did
      // not work.

      // Find the subscription state.  The dialog should not
      // have changed for this case so we use the established dialog
      // if it is provided, otherwise we use the early dialog
      UtlString dialogString(dialogHandle ? dialogHandle : earlyDialogHandle);
      lock();
      SubscribeClientState* clientState = getState(dialogString);

      if (clientState)
      {
         if (expirationDate < now)
         {
            clientState->mState = SUBSCRIPTION_TERMINATED;
         }
         else
         {
            clientState->mState = SUBSCRIPTION_SETUP;
         }

         if (clientState->mpStateCallback)
         {
            (clientState->mpStateCallback)(clientState->mState,
                                           earlyDialogHandle,
                                           dialogHandle,
                                           clientState->mpApplicationData,
                                           responseCode, 
                                           responseText,
                                           expirationDate,
                                           subscribeResponse);
         }

         // We do not remove the subscription state, that is the
         // applications job to explicitly call endSubscription
      }
      unlock();
   }
   break;

   // This should not happen
   case SipRefreshManager::REFRESH_REQUEST_UNKNOWN:
   default:
      OsSysLog::add(FAC_SIP, PRI_ERR, 
                    "SipSubscribeClient::refreshCallback invalid dialog state change: %d",
                    newState);
      break;
   }
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

void SipSubscribeClient::handleNotifyRequest(const SipMessage& notifyRequest)
{
    UtlString eventField;
    notifyRequest.getEventField(&eventField, NULL);
    // We could validate that the event field is
    // set and is the right event type, but mostly
    // we should not care as we know the event type
    // from the subscription.  We can be tolerant of
    // missing or malformed event headers in the
    // NOTIFY request.  However this does not support
    // multiple event types in the same dialog

    UtlString notifyDialogHandle;
    notifyRequest.getDialogHandle(notifyDialogHandle);

    // Is there an established dialog?
    UtlBoolean foundDialog = mpDialogMgr->dialogExists(notifyDialogHandle);
    // Is there an early dialog?
    // Even if there is an established dialog, we still need
    // the early dialog handle to pass to the callback
    // routines.
    // Construct the early dialog handle, keeping in mind that the
    // tags on the NOTIFY are the opposite of what they are on
    // the SUBSCRIBE
    UtlString earlyDialogHandle(notifyDialogHandle);
    ssize_t comma1 = earlyDialogHandle.index(',');
    ssize_t comma2 = earlyDialogHandle.index(',', comma1+1);
    earlyDialogHandle.remove(comma1, comma2-comma1);
    earlyDialogHandle.append(',');
    
    UtlBoolean foundEarlyDialog =
       mpDialogMgr->earlyDialogExists(earlyDialogHandle);

    enum SipDialogMgr::transactionSequence sequence = SipDialogMgr::NO_DIALOG;

#ifdef TEST_PRINT
    osPrintf("SipSubscribeClient::handleNotifyRequest looking for NOTIFY dialog: %s\n",
        notifyDialogHandle.data());
#endif

    if (foundDialog) 
    {
        sequence = 
            mpDialogMgr->isNewRemoteTransaction(notifyRequest);
    }

    // No established dialog
    else
    {
        // We can get a NOTIFY for an early dialog as there is a
        // race condition where the NOTIFY can pass the final 
        // response to the initial SUBSCRIBE
        // Is there an early dialog for this NOTIFY which should
        // be an established dialog (i.e. From and To tags)?

        if (foundEarlyDialog)
        {
            sequence = 
                mpDialogMgr->isNewRemoteTransaction(notifyRequest);
        }
        else
        {
           // Neither an established nor an early dialog.
           // Check to see if this is from another fork of a subscription
           // that has already been established.
           UtlString establishedDialogHandle;
           if (mpDialogMgr->getEstablishedDialogHandleFor(earlyDialogHandle,
                                                          establishedDialogHandle))
           {
              // establishedDialogHandle is the existing subscription that
              // is a sibling fork of the current NOTIFY.
              // Clone its state.
              SubscribeClientState* establishedState = getState(establishedDialogHandle);
              if (establishedState)
              {
                 OsSysLog::add(FAC_SIP, PRI_DEBUG, 
                               "SipSubscribeClient::handleNotifyRequest state duplicated from dialog: %s",
                               establishedDialogHandle.data());
                 SubscribeClientState* newState = new SubscribeClientState(*establishedState);
                 static_cast <UtlString&> (*newState) = notifyDialogHandle;
                 newState->mState = SUBSCRIPTION_SETUP;
                 addState(*newState);

                 // Insert the subscription state into the Refresh Manager
                 // (which will insert the dialog state into the Dialog
                 // Manager).
                 // Construct a SUBSCRIBE message for refreshing this
                 // subscription by copying the SUBSCRIBE from the other
                 // fork.
                 // Note that the Contact from the NOTIFY will be edited
                 // into the Dialog Manager's information when
                 // SipDialogMgr::updateDialog() is called below.
                 SipMessage subscribeRequest;
                 // This lookup should always succeed.
                 if (mpRefreshMgr->getRequest(establishedDialogHandle,
                                              subscribeRequest))
                 {
                    // Set the to-tag to match this NOTIFY's from-tag.
                    Url fromUrl;
                    notifyRequest.getFromUrl(fromUrl);
                    UtlString fromTag;
                    fromUrl.getFieldParameter("tag", fromTag);
                    subscribeRequest.setToFieldTag(fromTag);
                    UtlString dummy;
                    mpRefreshMgr->initiateRefresh(subscribeRequest,
                                                  this,
                                                  SipSubscribeClient::refreshCallback,
                                                  dummy,
                                                  // Do not send a SUBSCRIBE now.
                                                  TRUE);
                    sequence = SipDialogMgr::IN_ORDER;
                    // Adjust the flag variables to match what is now true.
                    foundDialog = TRUE;
                 }
                 else
                 {
                    // If the lookup fails, give up on the subscription
                    // this NOTIFY is trying to establish.
                    sequence = SipDialogMgr::NO_DIALOG;
                 }
              }
           }
        }
    }

    // Construct the response based on the request's status.
    SipMessage subscriptionResponse;

    switch (sequence)
    {
    case SipDialogMgr::NO_DIALOG:
       // NOTIFY does not match a dialog
       subscriptionResponse.setBadSubscriptionData(&notifyRequest);
    break;

    case SipDialogMgr::IN_ORDER:
    {
       // Request is a new transaction (i.e. cseq greater than
       // last remote transaction
       // Update the dialog
       mpDialogMgr->updateDialog(notifyRequest, notifyDialogHandle);

       // Get the SubscriptionClientState.
       SubscribeClientState* clientState = NULL;
       lock();

       if (!foundDialog && foundEarlyDialog)
       {
          // Found an early dialog.
          // Change the dialogHandle because we switched from an
          // early to an established dialog
          clientState = removeState(earlyDialogHandle);
            
          // Update the subscription state
          // Take the state out of the hashbag, change the key
          // and put it back in as it is not clear the hasbag 
          // will work correctly if you modify the key in place.
          if (clientState)
          {
             dynamic_cast <UtlString&> (*clientState) = notifyDialogHandle;
             clientState->mState = SUBSCRIPTION_SETUP;
             addState(*clientState);

             // invoke the subsription state call back to let
             // the application know the subscription is established
             if (clientState->mpStateCallback)
             {
                // Indicate that the subscription was established
                (clientState->mpStateCallback)(SUBSCRIPTION_SETUP,
                                               earlyDialogHandle,
                                               notifyDialogHandle,
                                               clientState->mpApplicationData,
                                               -1, // no response code
                                               NULL, // no response text
                                               -1, // do not know expiration
                                               NULL); // no response
             }
          }
          else
          {
             // There is a race condition which may cause the early dialog
             // to be promoted to established by RefreshManager thread
             // after this thread determined that the NOTIFY corresponds to
             // an early dialog
             clientState = getState(notifyDialogHandle);
          }
       }
       else
       {
          // Found an established dialog
          // Use the notify dialogHandle to get the subscription state
          clientState =  getState(notifyDialogHandle);
       }

       unlock();

       // invoke the Notify callback if a dialog exists
       if (clientState)
       {
          if (clientState->mpNotifyCallback)
          {
             (clientState->mpNotifyCallback)(earlyDialogHandle,
                                             notifyDialogHandle,
                                             clientState->mpApplicationData,
                                             &notifyRequest);
          }
          // Send an OK response; this NOTIFY matched a subscription state.
          subscriptionResponse.setOkResponseData(&notifyRequest);
       }
       else
       {
          // Could not find the subscription.
          subscriptionResponse.setBadSubscriptionData(&notifyRequest);
       }
    }
    break;

    case SipDialogMgr::LOOPED:
       subscriptionResponse.setInterfaceIpPort(notifyRequest.getInterfaceIp(), 
                                               notifyRequest.getInterfacePort());
       subscriptionResponse.setResponseData(&notifyRequest,
                                            SIP_LOOP_DETECTED_CODE,
                                            SIP_LOOP_DETECTED_TEXT);
       /** Set "Retry-After: 0" to prevent termination of the subscription.
        *  See RFC 3265, section 3.2.2.
        */
       subscriptionResponse.setHeaderValue(SIP_RETRY_AFTER_FIELD, "0");
       break;

    case SipDialogMgr::OUT_OF_ORDER:
       subscriptionResponse.setInterfaceIpPort(notifyRequest.getInterfaceIp(),
                                               notifyRequest.getInterfacePort());
       subscriptionResponse.setResponseData(&notifyRequest,
                                            SIP_OUT_OF_ORDER_CODE,
                                            SIP_OUT_OF_ORDER_TEXT);
       subscriptionResponse.setHeaderValue(SIP_RETRY_AFTER_FIELD, "0");
       break;
    }

    // Send the response.
    mpUserAgent->send(subscriptionResponse);
}

void SipSubscribeClient::addState(SubscribeClientState& clientState)
{
    mSubscriptions.insert(&clientState);
}

SubscribeClientState* SipSubscribeClient::getState(const UtlString& dialogHandle)
{
    SubscribeClientState* foundState =
       dynamic_cast <SubscribeClientState*> (mSubscriptions.find(&dialogHandle));
    if (foundState == NULL)
    {
        // Swap the tags around to see if it is keyed the other way
        UtlString reversedHandle;
        SipDialog::reverseTags(dialogHandle, reversedHandle);
        foundState =
           dynamic_cast <SubscribeClientState*> (mSubscriptions.find(&reversedHandle));
    }

    return (foundState);
}

SubscribeClientState* SipSubscribeClient::removeState(UtlString& dialogHandle)
{
    SubscribeClientState* foundState =
       dynamic_cast <SubscribeClientState*> (mSubscriptions.remove(&dialogHandle));
    if (foundState == NULL)
    {
        // Swap the tags around to see if it is keyed the other way
        UtlString reversedHandle;
        SipDialog::reverseTags(dialogHandle, reversedHandle);
        foundState =
           dynamic_cast <SubscribeClientState*> (mSubscriptions.remove(&reversedHandle));
    }

    return (foundState);
}

void SipSubscribeClient::lock()
{
    mSubscribeClientMutex.acquire();
}

void SipSubscribeClient::unlock()
{
    mSubscribeClientMutex.release();
}

/* ============================ FUNCTIONS ================================= */
