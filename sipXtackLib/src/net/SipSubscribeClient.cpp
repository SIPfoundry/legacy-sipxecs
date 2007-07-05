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
#include <utl/UtlHashMapIterator.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

// Private class to contain subscription client states
class SubscribeClientState : public UtlString
{
public:
    // The parent UtlString contains the dialogHandle as a key

    SubscribeClientState();

    virtual ~SubscribeClientState();

    void toString(UtlString& dumpString);

    // UtlString::data contains the Dialog Handle;
    SipSubscribeClient::SubscriptionState mState;
    void* mpApplicationData;
    SipSubscribeClient::SubscriptionStateCallback mpStateCallback;
    SipSubscribeClient::NotifyEventCallback mpNotifyCallback;

private:
    //! DISALLOWED accendental copying
    SubscribeClientState(const SubscribeClientState& rSubscribeClientState);
    SubscribeClientState& operator=(const SubscribeClientState& rhs);
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

// Constructor
SipSubscribeClient::SipSubscribeClient(SipUserAgent& userAgent, 
                                       SipDialogMgr& dialogMgr,
                                       SipRefreshManager& refreshMgr)
    : OsServerTask("SipSubscribeClient-%d")
    , mSubcribeClientMutex(OsMutex::Q_FIFO)
{
    mpUserAgent = &userAgent;
    mpDialogMgr = &dialogMgr;
    mpRefreshMgr = &refreshMgr;
    mCallIdCount = 0;
    mTagCount = 0;
}

// Copy constructor
SipSubscribeClient::SipSubscribeClient(const SipSubscribeClient& rSipSubscribeClient)
: mSubcribeClientMutex(OsMutex::Q_FIFO)
{
}


// Destructor
SipSubscribeClient::~SipSubscribeClient()
{
    // Do not delete mpUserAgent, mpDialogMgr or mpRefreshMgr.  They
    // may be used else where and need to be deleted outside the
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

   return *this;
}

UtlBoolean SipSubscribeClient::addSubscription(const char* resourceId,
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
    return(addSubscription(subscribeRequest, 
                    applicationData,
                    subscriptionStateCallback,
                    notifyEventsCallback,
                    earlyDialogHandle));
}

UtlBoolean SipSubscribeClient::addSubscription(SipMessage& subscriptionRequest,
                                               void* applicationData,
                                               const SubscriptionStateCallback subscriptionStateCallback,
                                               const NotifyEventCallback notifyEventsCallback,
                                               UtlString& earlyDialogHandle)
{
    // Verify the from tag is set
    Url fromUrl;
    subscriptionRequest.getFromUrl(fromUrl);
    UtlString fromTag;
    fromUrl.getFieldParameter("tag",fromTag);
    if(fromTag.isNull())
    {
        UtlString fromFieldValue;
        fromUrl.toString(fromFieldValue);
        CallId::getNewTag("", fromTag);
        fromUrl.setFieldParameter("tag", fromTag);
        fromUrl.toString(fromFieldValue);
        subscriptionRequest.setRawFromField(fromFieldValue);
    }

    // Get the event type and make sure we are registered to
    // receive NOTIFY requests for this event type
    UtlString eventType;
    subscriptionRequest.getEventField(&eventType, NULL, NULL);
    // If this event type is not in the list, we need to register
    // to receive SUBSCRIBE responses for this event type
    lock();
    if(mEventTypes.find(&eventType) == NULL)
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
    clientState->mState = SUBSCRIPTION_UNKNOWN;
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
                                      this,  // this comes back as app. data in callback
                                      refreshCallback,
                                      earlyDialogHandle);

    return(initialSendOk);
}

UtlBoolean SipSubscribeClient::endSubscription(const char* dialogHandle)
{
    UtlBoolean foundSubscription = FALSE;
    UtlString matchDialog(dialogHandle);
    lock();
    SubscribeClientState* clientState = removeState(matchDialog);
    unlock();

    if(clientState)
    {
        foundSubscription = TRUE;
        // If there is a state change of interest and
        // there is a callback function
        if(clientState->mState != SUBSCRIPTION_FAILED &&
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
        clientState = NULL;
    }

    // Did not find a dialog to match
    else
    {
        // dialogHandle may be a handle to an early dialog.
        // See if we can find the dialog matching an early dialog handle.
        // It is possible that there is more than one dialog that matches
        // this early dialog handle.
        UtlString earlyDialogHandle;
        while(mpDialogMgr->getEarlyDialogHandleFor(matchDialog, earlyDialogHandle))
        {
            lock();
            clientState = removeState(earlyDialogHandle);
            unlock();

            if(clientState)
            {
                foundSubscription = TRUE;
                // If there is a state change of interest and
                // there is a callback function
                if(clientState->mState != SUBSCRIPTION_FAILED &&
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
            }
        }
    }

    // Stop the refresh and unsubscribe
    UtlBoolean foundRefreshSubscription = mpRefreshMgr->stopRefresh(dialogHandle);

    return(foundSubscription || foundRefreshSubscription);
}

void SipSubscribeClient::endAllSubscriptions()
{
    SubscribeClientState* clientState = NULL;
    SubscribeClientState* dialogKey = NULL;
    UtlString earlyDialogHandle;
    // Not sure if we can take the lock and hold it while we
    // unsubscribe.  The refreshMgr is going to invoke call backs.
    // If this lock blocks the same thread from taking the lock,
    // we will have a deadlock.  Would like to take the lock once
    // to prevent additions to the mSubscription container until
    // we are all done unsubscribing.
    lock();
    UtlHashMapIterator iterator(mSubscriptions);
    while((dialogKey = (SubscribeClientState*) iterator()))
    {
        clientState = removeState(*dialogKey);

        if(clientState)
        {
            // If there is a state change of interest and
            // there is a callback function
            if(clientState->mState != SUBSCRIPTION_FAILED &&
               clientState->mpStateCallback)
            {
                mpDialogMgr->getEarlyDialogHandleFor(*dialogKey, earlyDialogHandle);

                // Indicate that the subscription was terminated
                (clientState->mpStateCallback)(SUBSCRIPTION_TERMINATED,
                    clientState->mState == SUBSCRIPTION_INITIATED ? earlyDialogHandle.data() : NULL,
                    clientState->mState == SUBSCRIPTION_SETUP ? dialogKey->data() : NULL,
                    clientState->mpApplicationData,
                    -1, // no response code
                    NULL, // no response text
                    0, // expires now
                    NULL); // no response
            }

            // Unsubscribe and stop refreshing the subscription
            mpRefreshMgr->stopRefresh(*dialogKey);

            delete clientState;
            clientState = NULL;
            dialogKey = NULL;  // dialogKey and state should be the same object
        }


    }
    unlock();
}

UtlBoolean SipSubscribeClient::handleMessage(OsMsg &eventMessage)
{
    int msgType = eventMessage.getMsgType();
    int msgSubType = eventMessage.getMsgSubType();

    // SIP message
    if(msgType == OsMsg::PHONE_APP &&
       msgSubType == SipMessage::NET_SIP_MESSAGE)
    {
        const SipMessage* sipMessage = ((SipMessageEvent&)eventMessage).getMessage();

        // If this is a NOTIFY request
        UtlString method;
        if(sipMessage) sipMessage->getRequestMethod(&method);
        if(sipMessage)
        {
            if(method.compareTo(SIP_NOTIFY_METHOD) == 0 &&
               !sipMessage->isResponse())
            {
                handleNotifyRequest(*sipMessage);
            }
            //else if(method.compareTo(SIP_SUBSCRIBE_METHOD) == 0 &&
            //        sipMessage->isResponse())
            // Subscribe responses should go to the refreshManager
            // where a callback will be used to notify the subscribe
            // client of the outcome via refreshCallback.

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

    return(TRUE);
}

/* ============================ ACCESSORS ================================= */

int SipSubscribeClient::countSubscriptions()
{
    int count = 0;
    lock();
    count = mSubscriptions.entries();
    unlock();
    return(count);
}

int SipSubscribeClient::dumpStates(UtlString& dumpString)
{
    int count = 0;
    dumpString.remove(0);
    UtlString oneClientDump;
    SubscribeClientState* clientState = NULL;
    lock();
    UtlHashMapIterator iterator(mSubscriptions);
    while((clientState = (SubscribeClientState*) iterator()))
    {
        clientState->toString(oneClientDump);
        dumpString.append(oneClientDump);

        count++;
    }
    unlock();

    return(count);
}

void SipSubscribeClient::getSubscriptionStateEnumString(enum SubscriptionState stateValue, 
                                                        UtlString& stateString)
{
    switch(stateValue)
    {
    case SUBSCRIPTION_UNKNOWN:
        stateString = "SUBSCRIPTION_UNKNOWN";
        break;

    case SUBSCRIPTION_INITIATED: // Early dialog
        stateString = "SUBSCRIPTION_INITIATED";
        break;

    case SUBSCRIPTION_SETUP:     // Established dialog
        stateString = "SUBSCRIPTION_SETUP";
        break;

    case SUBSCRIPTION_FAILED:    // Failed dialog setup or refresh
        stateString = "SUBSCRIPTION_FAILED";
        break;

    case SUBSCRIPTION_TERMINATED:
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
    if(subscribeClientPtr)
    {
        long now = OsDateTime::getSecsSinceEpoch();
        SipSubscribeClient* subClient = (SipSubscribeClient*) subscribeClientPtr;
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

                subClient->lock();
                SubscribeClientState* clientState = NULL;

                // See if we can find an state for the early dialog
                if(earlyDialogHandle && *earlyDialogHandle)
                {
                    UtlString dialogString(earlyDialogHandle);
                    clientState = 
                        subClient->removeState(dialogString);

                    // If we could not find the state for the early dialog
                    // it must have been a second dialog created from the
                    // same early dialog.
                    // TODO: copy subscription state and create a second
                    // (or subsequent) subscription.
                    if(clientState == NULL)
                    {
                        OsSysLog::add(FAC_SIP, PRI_ERR, 
                            "SipSubscribeClient::refreshCallback failed to find early dialog: %s",
                            earlyDialogHandle);
                    }
                    else
                    {
                        // Change the dialogHandle as we switched from an
                        // early to an established dialog

                        // Update the subscription state
                        // Take the state out of the hashbag, change the key
                        // and put it back in as it is not clear the hasbag 
                        // will work correctly if you modify the key in place.
                        *((UtlString*)clientState) = dialogHandle;
                        clientState->mState = SUBSCRIPTION_SETUP;
                        subClient->addState(*clientState);
                    }
                }

                else
                {
                    UtlString dialogString(dialogHandle);
                    clientState = 
                        subClient->getState(dialogString);
                }

                // If the response code is > 299, the reSUBSCRIBE failed,
                // but a prior SUBSCRIBE has not expired yet

                if(clientState)
                {
                    if(expirationDate < now)
                    {
                        clientState->mState = SUBSCRIPTION_TERMINATED;
                    }
                    else
                    {
                        clientState->mState = SUBSCRIPTION_SETUP;
                    }

                    if(clientState->mpStateCallback)
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
                subClient->unlock();

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
                UtlString dailogString(dialogHandle ? dialogHandle : earlyDialogHandle);
                subClient->lock();
                SubscribeClientState* clientState = subClient->getState(dailogString);

                if(clientState)
                {
                    if(expirationDate < now)
                    {
                        clientState->mState = SUBSCRIPTION_TERMINATED;
                    }
                    else
                    {
                        clientState->mState = SUBSCRIPTION_SETUP;
                    }

                    if(clientState->mpStateCallback)
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
                subClient->unlock();
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
    UtlString earlyDialogHandle;
    UtlBoolean foundEarlyDialog =
       mpDialogMgr->getEarlyDialogHandleFor(notifyDialogHandle,
                                            earlyDialogHandle);

    UtlBoolean subscriptionFound = FALSE;
    UtlBoolean newTransaction = FALSE;

#ifdef TEST_PRINT
    osPrintf("SipSubscribeClient::handleNotifyRequest looking for NOTIFY dialog: %s\n",
        notifyDialogHandle.data());
#endif

    if(foundDialog) 
    {
        newTransaction = 
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

        if(foundEarlyDialog)
        {
            newTransaction = 
                mpDialogMgr->isNewRemoteTransaction(notifyRequest);
        }
    }

    // Request is a new transaction (i.e. cseq greater than
    // last remote transaction
    if(newTransaction)
    {
        // Update the dialog
        mpDialogMgr->updateDialog(notifyRequest, notifyDialogHandle);

        // Get the SubScriptionState 
        SubscribeClientState* clientState = NULL;
        lock();

        if(!foundDialog && foundEarlyDialog)
        {
            // Change the dialogHandle as we switched from an
            // early to an established dialog
            clientState = removeState(earlyDialogHandle);
            
            // Update the subscription state
            // Take the state out of the hashbag, change the key
            // and put it back in as it is not clear the hasbag 
            // will work correctly if you modify the key in place.
            if(clientState)
            {
                *((UtlString*)clientState) = notifyDialogHandle;
                clientState->mState = SUBSCRIPTION_SETUP;
                addState(*clientState);

                // invoke the subsription state call back to let
                // the application know the subscription is established
                if(clientState->mpStateCallback)
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

        // Established dialog
        else
        {
            // Use the notify dialogHandle to get the subscription state
            clientState =  getState(notifyDialogHandle);
        }

        // invoke the Notify callback if it exists
        if(clientState)
        {
            subscriptionFound = TRUE;

            if(clientState->mpNotifyCallback)
            {
                (clientState->mpNotifyCallback)(earlyDialogHandle,
                                          notifyDialogHandle,
                                          clientState->mpApplicationData,
                                          &notifyRequest);
            }
        }
        unlock();
    }

    // NOTIFY does not match a dialog
    if(!subscriptionFound)
    {
        SipMessage noSubscriptionResponse;
        noSubscriptionResponse.setBadTransactionData(&notifyRequest);
        mpUserAgent->send(noSubscriptionResponse);
    }

    // Send a OK response this NOTIFY matched a subscription state
    else
    {
        SipMessage notifyOk;
        notifyOk.setOkResponseData(&notifyRequest);
        mpUserAgent->send(notifyOk);
    }
}

void SipSubscribeClient::addState(SubscribeClientState& clientState)
{
    mSubscriptions.insert(&clientState);
}

SubscribeClientState* SipSubscribeClient::getState(const UtlString& dialogHandle)
{
    SubscribeClientState* foundState = (SubscribeClientState*) 
                mSubscriptions.find(&dialogHandle);
    if(foundState == NULL)
    {
        // Swap the tags around to see if it is keyed the other way
        UtlString reversedHandle;
        SipDialog::reverseTags(dialogHandle, reversedHandle);
        foundState = (SubscribeClientState*) 
                mSubscriptions.find(&reversedHandle);
    }

    return(foundState);
}

SubscribeClientState* SipSubscribeClient::removeState(UtlString& dialogHandle)
{
    SubscribeClientState* foundState = (SubscribeClientState*) 
                mSubscriptions.remove(&dialogHandle);
    if(foundState == NULL)
    {
        // Swap the tags around to see if it is keyed the other way
        UtlString reversedHandle;
        SipDialog::reverseTags(dialogHandle, reversedHandle);
        foundState = (SubscribeClientState*) 
                mSubscriptions.remove(&reversedHandle);
    }

    return(foundState);
}

void SipSubscribeClient::lock()
{
    mSubcribeClientMutex.acquire();
}

void SipSubscribeClient::unlock()
{
    mSubcribeClientMutex.release();
}

/* ============================ FUNCTIONS ================================= */
