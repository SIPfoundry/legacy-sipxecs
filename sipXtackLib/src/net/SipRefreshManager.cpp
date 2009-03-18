//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <os/OsLock.h>
#include <os/OsDateTime.h>
#include <os/OsTimer.h>
#include <os/OsEventMsg.h>
#include <utl/UtlHashBagIterator.h>
#include <net/SipRefreshManager.h>
#include <net/SipUserAgent.h>
#include <net/SipDialogMgr.h>
#include <net/SipDialog.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

// Private class to contain subscription client state.
// UtlString is the dialog handle of the dialog or pseudo-dialog that
// is maintaining it.
class RefreshDialogState : public UtlString
{
public:

    RefreshDialogState();

    virtual ~RefreshDialogState();
    void toString(UtlString& dumpString);

    // UtlString::data contains the dialog handle.
    void* mpApplicationData;
    SipRefreshManager::RefreshStateCallback mpStateCallback;
    int mExpirationPeriodSeconds; // original expiration
    long mPendingStartTime; // epoch time in seconds
    long mExpiration; // epoch time in seconds
    SipMessage* mpLastRequest;
    SipRefreshManager::RefreshRequestState mRequestState;
    int mFailedResponseCode;
    UtlString mFailedResponseText;
    OsTimer* mpRefreshTimer;  // Fires when it is time to resend

    //! Dump the object's internal state.
    void dumpState();

    //! Convert RefreshRequestState to a printable string.
    static const char* refreshRequestStateText(SipRefreshManager::RefreshRequestState requestState);

private:
    //! DISALLOWED accendental copying
    RefreshDialogState(const RefreshDialogState& rRefreshDialogState);
    RefreshDialogState& operator=(const RefreshDialogState& rhs);

};

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

RefreshDialogState::RefreshDialogState()
{
    mpApplicationData = NULL;
    mpStateCallback = NULL;
    mExpirationPeriodSeconds = -1;
    mPendingStartTime = -1;
    mExpiration = -1;
    mpLastRequest = NULL;
    mRequestState = SipRefreshManager::REFRESH_REQUEST_UNKNOWN;
    mFailedResponseCode = -1;
    mpRefreshTimer = NULL;
}

void RefreshDialogState::toString(UtlString& dumpString)
{
    dumpString = "RefreshDialogState:\n\tmpData: ";
    dumpString.append(*this);
    dumpString.append("\n\tmpApplicationData: ");
    char numBuf[20];
    sprintf(numBuf, "%p", mpApplicationData);
    dumpString.append(numBuf);
    dumpString.append("\n\tmpStateCallback: ");
    sprintf(numBuf, "%p", mpStateCallback);
    dumpString.append(numBuf);
    dumpString.append("\n\tmExpirationPeriodSeconds: ");
    sprintf(numBuf, "%d", mExpirationPeriodSeconds);
    dumpString.append(numBuf);
    dumpString.append("\n\tmPendingStartTime: ");
    sprintf(numBuf, "%ld", mPendingStartTime);
    dumpString.append(numBuf);
    dumpString.append("\n\tmExpiration: ");
    sprintf(numBuf, "%ld", mExpiration);
    dumpString.append(numBuf);
    dumpString.append("\n\tmpLastRequest: ");
    sprintf(numBuf, "%p", mpLastRequest);
    dumpString.append(numBuf);
    dumpString.append("\n\tmRequestState: ");
    UtlString stateString;
    SipRefreshManager::refreshState2String(mRequestState, stateString);
    dumpString.append(stateString);
    dumpString.append("\n\tmFailedResponseCode: ");
    sprintf(numBuf, "%d", mFailedResponseCode);
    dumpString.append(numBuf);
    dumpString.append("\n\tmFailedResponseText: ");
    dumpString.append(mFailedResponseText ? mFailedResponseText : "");
    dumpString.append("\n\tmpRefreshTimer: ");
    sprintf(numBuf, "%p", mpRefreshTimer);
    dumpString.append(numBuf);
}

// Copy constructor NOT ALLOWED
RefreshDialogState::RefreshDialogState(const RefreshDialogState& rRefreshDialogState)
{
}

RefreshDialogState::~RefreshDialogState()
{
    if (mpLastRequest)
    {
       delete mpLastRequest;
       mpLastRequest = NULL;
    }

    if (mpRefreshTimer)
    {
       delete mpRefreshTimer;
       mpRefreshTimer = NULL;
    }
}

//Assignment operator NOT ALLOWED
RefreshDialogState& 
RefreshDialogState::operator=(const RefreshDialogState& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

// Constructor
SipRefreshManager::SipRefreshManager(SipUserAgent& userAgent, 
                                     SipDialogMgr& dialogMgr)
    : OsServerTask("SipRefreshManager-%d")
    , mRefreshMgrMutex(OsMutex::Q_FIFO)
{
    mpUserAgent = &userAgent;
    mpDialogMgr = &dialogMgr;
    mReceivingRegisterResponses = FALSE;
    mReceivingSubscribeResponses = FALSE;
    mDefaultExpiration = 3600;
}

// Copy constructor
SipRefreshManager::SipRefreshManager(const SipRefreshManager& rSipRefreshManager)
: mRefreshMgrMutex(OsMutex::Q_FIFO)
{
    // NOT ALLOWED
}


// Destructor
SipRefreshManager::~SipRefreshManager()
{
    // Do not delete *mpUserAgent or *mpDialogMgr.  They are not owned
    // by this SipRefreshManager and may be used elsewhere.

    // Stop receiving SUBSCRIBE responses
    mpUserAgent->removeMessageObserver(*(getMessageQueue()));

    // Wait until this OsServerTask has stopped or handleMethod
    // might access something we are about to delete here.
    waitUntilShutDown();

    // Unsubscribe to anything that is in the list
    stopAllRefreshes();
    // mRefreshes should now be empty
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
SipRefreshManager& 
SipRefreshManager::operator=(const SipRefreshManager& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

UtlBoolean SipRefreshManager::initiateRefresh(SipMessage& subscribeOrRegisterRequest,
                                              void* applicationData,
                                              const RefreshStateCallback refreshStateCallback,
                                              UtlString& earlyDialogHandle,
                                              UtlBoolean suppressFirstSend)
{
    UtlBoolean intitialRequestSent = FALSE;

    // Make sure we do not have an existing dialog or refresh session state
    // going for the given message
    UtlString messageDialogHandle;
    subscribeOrRegisterRequest.getDialogHandle(messageDialogHandle);
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "SipRefreshManager::initiateRefresh earlyDialogHandle = '%s', messageDialogHandle = '%s'",
                  earlyDialogHandle.data(), messageDialogHandle.data());

    UtlBoolean existingRefreshState = FALSE;
    UtlBoolean existingDialogState = FALSE;

    // It is OK if messageDialogHandle is an established dialog handle.
    // That happens when we are initializing a forked subscription
    // because we received a NOTIFY for it.

    OsLock localLock(mRefreshMgrMutex);
    // See if there is an early or established dialog for this message
    RefreshDialogState* s = getAnyDialog(messageDialogHandle);
    if (s)
    {
        existingRefreshState = TRUE;
        intitialRequestSent = FALSE;
        OsSysLog::add(FAC_SIP, PRI_ERR,
            "SipRefreshManager::initiateRefresh called with dialog handle '%s' that matches existing refresh state '%s'",
                      messageDialogHandle.data(),
                      s->data());
    }

    // The dialog should not exist either
    else if (mpDialogMgr->dialogExists(messageDialogHandle))
    {
        existingDialogState = TRUE;
        OsSysLog::add(FAC_SIP, PRI_ERR,
                      "SipRefreshManager::initiateRefresh called with dialog handle '%s' that matches existing dialog state",
            messageDialogHandle.data());
    }

    else if (mpDialogMgr->earlyDialogExistsFor(messageDialogHandle))
    {
        existingDialogState = TRUE;
        OsSysLog::add(FAC_SIP, PRI_ERR,
            "SipRefreshManager::initiateRefresh called with dialog handle '%s' that matches existing early dialog state",
            messageDialogHandle.data());
    }

    // Should not be any existing refresh or dialog states
    // for this message
    if(!existingRefreshState && !existingDialogState)
    {
        // Make sure we are registered to receive responses
        // for the message we are about to send
        UtlString method;
        subscribeOrRegisterRequest.getRequestMethod(&method);
        if(method.compareTo(SIP_REGISTER_METHOD) == 0)
        {
            lock();
            if(!mReceivingRegisterResponses)
            {
                mReceivingRegisterResponses = TRUE;
                // Register to receive REGISTER responses.
OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipRefreshManager::initiateRefresh Adding REGISTER response observer");
                mpUserAgent->addMessageObserver(*(getMessageQueue()), 
                                                SIP_REGISTER_METHOD,
                                                FALSE, // no requests
                                                TRUE, // yes responses
                                                TRUE, // incoming,
                                                FALSE, // outgoing
                                                NULL);
            }
            unlock();
        }
        else if(method.compareTo(SIP_SUBSCRIBE_METHOD) == 0)
        {
            UtlString eventType;
            subscribeOrRegisterRequest.getEventField(&eventType, NULL);
            // Check to see if we have already registered to
            // receive the event type
            lock();
            if(!mReceivingSubscribeResponses)
            {
                mReceivingSubscribeResponses = TRUE;
                // Register to receive SUBSCRIBE responses.
OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipRefreshManager::initiateRefresh Adding SUBSCRIBE observer");
                mpUserAgent->addMessageObserver(*(getMessageQueue()), 
                                                SIP_SUBSCRIBE_METHOD,
                                                FALSE, // no requests
                                                TRUE, // yes responses
                                                TRUE, // incoming,
                                                FALSE, // outgoing
                                                NULL); // event type is not used for filtering responses
            }
            unlock();
        }

        // Create a new refresh state
        int requestedExpiration = 0;  // returned from following call
        RefreshDialogState* state = createNewRefreshState(subscribeOrRegisterRequest,
                                                          messageDialogHandle,
                                                          applicationData,
                                                          refreshStateCallback,
                                                          requestedExpiration);

        // create a new dialog
        mpDialogMgr->createDialog(subscribeOrRegisterRequest, 
                                  TRUE, // message from this side
                                  messageDialogHandle);

        // Keep track of when we send this request to be refreshed
        long now = OsDateTime::getSecsSinceEpoch();
        state->mPendingStartTime = now;
        OsSysLog::add(FAC_SIP, PRI_DEBUG,
                      "SipRefreshManager::initiateRefresh %p->mPendingStartTime = %ld",
                      state, state->mPendingStartTime);

        // Set a timer  at which to resend the next refresh based upon the 
        // assumption that the request will succeed.  If we receive a 
        // failed response, we will cancel this timer and reschedule
        // a new timer based upon a smaller fraction of the requested 
        // expiration period 
        setRefreshTimer(*state, 
                        TRUE); // Resend with successful timeout
        OsTimer* resendTimer = state->mpRefreshTimer;

        OsSysLog::add(FAC_SIP, PRI_DEBUG,
                      "SipRefreshManager::initiateRefresh refreshTimer just being set.");

        // Mark the refresh state as having an outstanding request
        // and make a copy of the request.  The copy needs to be
        // attached to the state before the send in case the response
        // comes back before we return from the send.
        state->mRequestState = REFRESH_REQUEST_PENDING;
        state->mpLastRequest = new SipMessage(subscribeOrRegisterRequest);

        // Add the state to the container of refresh states
        // No need to lock this refreshMgr earlier as this is a new
        // state and no one can touch it until it is in the list.
        lock();
        mRefreshes.insert(state);
        unlock();
        // NOTE: at this point is is no longer safe to touch the state
        // without locking it again.  Avoid locking this SipRefreshManager object
        // when something can block (e.g. like calling SipUserAgent ::send)

        // Send the request (unless that is suppressed).
        // Is this correct?  Should we send the request first and only set
        // a timer if the request succeeds??
        if (!suppressFirstSend)
        {
           intitialRequestSent = mpUserAgent->send(subscribeOrRegisterRequest);
        }
        else
        {
           // Do not send a message, but pretend the send was successful.
           intitialRequestSent = TRUE;
           // Set the state as if we had received 2xx to an initial SUBSCRIBE.
           state->mRequestState = REFRESH_REQUEST_SUCCEEDED;
        }

        // We do not clean up the state even if the send fails.
        // The application must end the refresh as the refresh
        // manager should retry the send if it failed
        if(!intitialRequestSent)
        {
            // Need to lock this refresh mgr and make sure the state
            // still exists.  It could have been deleted while this
            // was unlocked above.
            lock();
            if(stateExists(state))
            {
                // It is now safe to touch the state again

                // Mark the state of the last request as having 
                // failed, so we know to resend when the timer
                // fires
                state->mRequestState = REFRESH_REQUEST_FAILED;

                // The expiration should still be set to zero

                // the initial send failed, cancel the timer and
                // fire the notification.  The handleMessage method
                // will see that the subscription or registration
                // has never succeeded and reshedule the timer for
                // a failure timeout.  We cannot reschedule the
                // timer here as there is a race condition between
                // deleting the timer here (in the applications context)
                // and in handleMessage in this refresh manager's 
                // context.
                // If the timer has not changed assume it is still safe
                // to touch it
                if(state->mpRefreshTimer == resendTimer)
                {
                    stopTimerForFailureReschedule(state->mpRefreshTimer);
                }

                // Do not notify the application that the request failed
                // when it occurs on the first invokation.  The application
                // will know by the return.
            }
            unlock();
        }
    }

    return(intitialRequestSent);
}


UtlBoolean SipRefreshManager::stopRefresh(const char* dialogHandle)
{
    UtlBoolean stateFound = FALSE;
    lock();
    // Find the refresh state
    UtlString dialogHandleString(dialogHandle);
    RefreshDialogState* state = getAnyDialog(dialogHandleString);

    // Remove the state so we can release the lock
    if (state)
    {
        mRefreshes.removeReference(state);
    }
    unlock();

    // If a matching state exists
    if (state)
    {
        // If the subscription or registration has not expired
        // or there is a pending request
        long now = OsDateTime::getSecsSinceEpoch();
        if (   state->mExpiration > now
            || state->mRequestState == REFRESH_REQUEST_PENDING)
        {
           // Send a terminating request.
           // We should not send a 0-expiration request if the request
           // is a SUBSCRIBE, as no subscription dialog has been
           // established within which we could send an unsubscribe.
           // (Any subscription that is created later will be
           // terminated when it sends a NOTIFY and we respond 481.)
           // But if the request is a REGISTER, we should send an
           // un-register, as REGISTERs are not within dialogs.
           if (state->mpLastRequest)
           {
              UtlString method;
              state->mpLastRequest->getRequestMethod(&method);
              if (method.compareTo(SIP_REGISTER_METHOD) == 0)
              {
                 // Reset the request with a zero expiration
                 setForResend(*state,
                              TRUE); // expire now

                 mpUserAgent->send(*(state->mpLastRequest));
              }
           }

           // Don't really need to set this stuff as we are
           // going to delete the state anyway
           state->mRequestState = REFRESH_REQUEST_PENDING;
           state->mPendingStartTime = now;
           state->mExpirationPeriodSeconds = 0;

           // Invoke the refresh state callback to indicate
           // the refresh has been expired
           UtlBoolean stateKeyIsEarlyDialog = SipDialog::isEarlyDialog(*state);
           (state->mpStateCallback)(state->mRequestState,
                                    stateKeyIsEarlyDialog ? state->data() : NULL,
                                    stateKeyIsEarlyDialog ? NULL : state->data(),
                                    state->mpApplicationData,
                                    -1, // responseCode
                                    NULL, // responseText,
                                    0, // zero means expires now
                                    NULL); // response
        }

        // Stop and delete the refresh timer
        state->mpRefreshTimer->stop();
        deleteTimerAndEvent(state->mpRefreshTimer);

        // Get rid of the dialog
        mpDialogMgr->deleteDialog(*state);

        // Fire and forget
        delete state;
        state = NULL;

        stateFound = TRUE;
    }

    return(stateFound);
}

void RefreshDialogState::dumpState()
{
   // indented 6

   UtlString requestURI;
   UtlString eventField;
   long now = OsDateTime::getSecsSinceEpoch();
   UtlString msg_text;
   ssize_t msg_length = 0;
   if (mpLastRequest)
   {
      mpLastRequest->getRequestUri(&requestURI);
      mpLastRequest->getEventField(&eventField);
      mpLastRequest->getBytes(&msg_text, &msg_length);
   }

   char refreshTimerText[100];
   if (mpRefreshTimer)
   {
      OsTimer::OsTimerState state;
      OsTimer::Time expiresAt;
      UtlBoolean periodic;
      OsTimer::Interval period;

      mpRefreshTimer->getFullState(state, expiresAt, periodic, period);

      sprintf(refreshTimerText,
              "%p %s/%+d/%s/%d",
              mpRefreshTimer,
              state == OsTimer::STARTED ? "STARTED" : "STOPPED",
              (int) ((expiresAt - OsTimer::now()) / 1000000),
              periodic ? "periodic" : "one-shot",
              (int) period);
   }
   else
   {
      sprintf(refreshTimerText,
              "%p",
              mpRefreshTimer);
   }

   OsSysLog::add(FAC_RLS, PRI_INFO,
                 "\t      RefreshDialogState %p request-URI = '%s', Event = '%s', mExpirationPeriodSeconds = %d, mPendingStartTime = %+d, mExpiration = %+d, mRequestState = '%s', mFailedResponseCode = %d, mFailedResponseText = '%s', mpRefreshTimer = %s, mpLastRequest = '%s'",
                 this, requestURI.data(), eventField.data(),
                 mExpirationPeriodSeconds, (int) (mPendingStartTime - now),
                 (int) (mExpiration - now),
                 refreshRequestStateText(mRequestState), mFailedResponseCode,
                 mFailedResponseText.data(),
                 refreshTimerText,
                 msg_text.data());
}

// Convert RefreshRequestState to a printable string.
const char* RefreshDialogState::refreshRequestStateText(SipRefreshManager::RefreshRequestState requestState)
{
   const char* ret;

   switch (requestState)
   {
   case SipRefreshManager::REFRESH_REQUEST_UNKNOWN:
      ret = "UNKNOWN";
      break;
   case SipRefreshManager::REFRESH_REQUEST_PENDING:
      ret = "PENDING";
      break;
   case SipRefreshManager::REFRESH_REQUEST_FAILED:
      ret = "FAILED";
      break;
   case SipRefreshManager::REFRESH_REQUEST_SUCCEEDED:
      ret = "SUCCEEDED";
      break;
   default:
      ret = "invalid value";
      break;
   }

   return ret;
};

void SipRefreshManager::stopAllRefreshes()
{
    //  Not sure if it is safe to take the lock on this
    // and keep it while we unsubscribe and unregister
    // everything.  There are some locking issues related
    // to handling incoming messages that might be a
    // problem.
    RefreshDialogState* dialogKey = NULL;
    lock();
    UtlHashBagIterator iterator(mRefreshes);
    while((dialogKey = (RefreshDialogState*) iterator()))
    {
        // Unsubscribe or unregister
        stopRefresh(*dialogKey);
        // Note that stopRefresh removes *dialogKey from mRefreshes and
        // deletes it, so we do not have to do so here.
    }
    unlock();
}

UtlBoolean SipRefreshManager::handleMessage(OsMsg &eventMessage)
{
    int msgType = eventMessage.getMsgType();
    int msgSubType = eventMessage.getMsgSubType();

    // Timer fired
    if(msgType == OsMsg::OS_EVENT &&
       msgSubType == OsEventMsg::NOTIFY)
    {
        intptr_t eventData = 0;
        RefreshDialogState* state = NULL;
        void* stateVoid;

        ((OsEventMsg&)eventMessage).getUserData(stateVoid);
        ((OsEventMsg&)eventMessage).getEventData(eventData);
        state = (RefreshDialogState*)stateVoid;

        lock();
        // If the state is not still in the list we cannot
        // touch it. It may have been deleted.
        if(state && stateExists(state))
        {
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "SipRefreshManager::handleMessage Timer fired, state = %p '%s', eventData = %ld",
                          state, state->data(), (long)eventData);

            // Refresh request failed, need to clean up and
            // schedule a refresh in a short/failed time period
            if(eventData == OS_INTERRUPTED)
            {
                // Clean up the timer and notifier
                deleteTimerAndEvent(state->mpRefreshTimer);

                // Create and set a new timer for the failed time out period
                setRefreshTimer(*state, 
                                FALSE);  // Resend with failure timeout
                OsSysLog::add(FAC_SIP, PRI_DEBUG,
                              "SipRefreshManager::handleMessage refreshTimer just being set for the failed timeout.");
            }

            // Normal timer fire, time to refresh
            else if(eventData != 0 &&
                    ((OsTimer*)eventData) == state->mpRefreshTimer)
            {
                // Clean up the timer and notifier
                deleteTimerAndEvent(state->mpRefreshTimer);

                // Legitimate states to reSUBSCRIBE or reREGISTER
                if(state->mRequestState == REFRESH_REQUEST_FAILED || 
                   state->mRequestState == REFRESH_REQUEST_SUCCEEDED)
                {
                    // Create and set a new timer for resending assuming
                    // the resend is successful.  If it fails we will
                    // cancel the timer and set a shorter timeout
                    setRefreshTimer(*state, 
                                    TRUE); // Resend with successful timeout

                    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                                  "SipRefreshManager::handleMessage refreshTimer just being set for the normal timeout.");

                    // reset the message for resend
                    setForResend(*state,
                                 FALSE); // do not expire now

                    // Keep track of when this refresh is sent so we know 
                    // when the new expiration is relative to.
                    state->mPendingStartTime = OsDateTime::getSecsSinceEpoch();
                    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                                  "SipRefreshManager::handleMessage %p->mPendingStartTime = %ld",
                                  state, state->mPendingStartTime);

                    // Do not want to keep the lock while we send the
                    // message as it could block.  Presumably it is better
                    // to incur the cost of copying the message????
                    SipMessage tempRequest(*(state->mpLastRequest));
                    
                    if (OsSysLog::willLog(FAC_SIP, PRI_DEBUG))
                    {
                       UtlString lastRequest;
                       ssize_t length;
                       state->mpLastRequest->getBytes(&lastRequest, &length);
                       OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipRefreshManager::handleMessage last request = '%s'",
                                     lastRequest.data());
                    }
                      
                    unlock();
                    mpUserAgent->send(tempRequest);
                    // do not need the lock any more, but this gives us
                    // clean locking symmetry.  DO NOT TOUCH state or
                    // any of its members BEYOND this point as it may 
                    // have been deleted
                    lock(); 
                }

                // This should not happen
                else
                {
                    OsSysLog::add(FAC_SIP, PRI_ERR,
                        "SipRefreshManager::handleMessage timer fired in unexpected state %d",
                        state->mRequestState);
                    // Dump the state into the log.
                    state->dumpState();

                    if(state->mRequestState == REFRESH_REQUEST_PENDING)
                    {
                       // Set a new, short timeout if the request is still
                       // pending.
                       setRefreshTimer(*state, 
                                       FALSE); // Resend with failed timeout
                       OsSysLog::add(FAC_SIP, PRI_DEBUG,
                                     "SipRefreshManager::handleMessage refreshTimer resetting timeout for the pending request.");
                    }
                }
            }

            // Bad -- do not know what happened
            else
            {
                OsSysLog::add(FAC_SIP, PRI_ERR,
                    "SipRefreshManager::handleMessage timer: %lx does not match state's timer: %p",
                    (long)eventData, state->mpRefreshTimer);
            }
        }
        unlock();
    }

    // SIP message
    else if (msgType == OsMsg::PHONE_APP &&
             msgSubType == SipMessage::NET_SIP_MESSAGE)
    {
        const SipMessage* sipMessage = ((SipMessageEvent&)eventMessage).getMessage();
        int messageType = ((SipMessageEvent&)eventMessage).getMessageStatus();

        // messageType can be:
        //    SipMessageEvent::TRANSPORT_ERROR for requests that do not get sent
        //            but we would have to register with the SipUserAgent to
        //            receive requests.
        //    SipMessageEvent::AUTHENTICATION_RETRY for 401 or 407 responses
        //            that are resent with credentials.  We get this message so
        //            that we can keep the dialog info. up to date.
        //    SipMessageEvent::APPLICATION normal messages
        // For now we will treat the APPLICATION and AUTHENTICATION_RETRY
        // identically.

        // If this is a SUBSCRIBE or REGISTER response
        UtlString method;
        int cseq;
        if (sipMessage) sipMessage->getCSeqField(&cseq, &method);
        if (sipMessage &&
            sipMessage->isResponse() &&
            (method.compareTo(SIP_SUBSCRIBE_METHOD) == 0 ||
             method.compareTo(SIP_REGISTER_METHOD) == 0))
        {
            UtlString dialogHandle;
            sipMessage->getDialogHandle(dialogHandle);
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "SipRefreshManager::handleMessage %s response %d, dialogHandle = '%s'",
                          method.data(), sipMessage->getResponseStatusCode(),
                          dialogHandle.data());

            UtlBoolean foundDialog = 
                mpDialogMgr->dialogExists(dialogHandle);
            UtlString earlyDialogHandle;
            UtlBoolean foundEarlyDialog = FALSE;
            UtlBoolean matchesLastLocalTransaction = FALSE;
            if (foundDialog)
            {
                matchesLastLocalTransaction = 
                    mpDialogMgr->isLastLocalTransaction(*sipMessage, 
                                                        dialogHandle);
            }
            else
            {
                foundEarlyDialog = 
                    mpDialogMgr->getEarlyDialogHandleFor(dialogHandle, 
                                                         earlyDialogHandle);
                if (foundEarlyDialog)
                {
                    matchesLastLocalTransaction =
                        mpDialogMgr->isLastLocalTransaction(*sipMessage, 
                                                            earlyDialogHandle);
                }
            }
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "SipRefreshManager::handleMessage foundDialog = %d, foundEarlyDialog = %d, matchesLastLocalTransaction = %d, earlyDialogHandle = '%s'",
                          foundDialog, foundEarlyDialog,
                          matchesLastLocalTransaction, earlyDialogHandle.data());

#ifdef TEST_PRINT
            UtlString refreshStateDump;
            dumpRefreshStates(refreshStateDump);
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "SipRefreshManager::handleMessage state dump: %s",
                          refreshStateDump.data());
#endif

            lock();
            // Find the refresh state for this response
            RefreshDialogState* state = NULL;
            if (foundDialog && matchesLastLocalTransaction)
            {
                state = (RefreshDialogState*) mRefreshes.find(&dialogHandle);
                // Check if the key has the tags reversed
                if (state == NULL)
                {
                    UtlString reversedDialogHandle;
                    SipDialog::reverseTags(dialogHandle, reversedDialogHandle);
                    state =
                       (RefreshDialogState*) 
                       mRefreshes.find(&reversedDialogHandle);
                }
            }
            else if (foundEarlyDialog && matchesLastLocalTransaction)
            {
                state = (RefreshDialogState*) mRefreshes.find(&earlyDialogHandle);
                // See if the key has the tags reversed
                if (state == NULL)
                {
                    UtlString reversedEarlyDialogHandle;
                    SipDialog::reverseTags(earlyDialogHandle, reversedEarlyDialogHandle);
                    state =
                       (RefreshDialogState*) 
                       mRefreshes.find(&reversedEarlyDialogHandle);
                }
            }

            if (state)
            {
                // Need to check for error responses vs. 2xx-class responses
                int responseCode = sipMessage->getResponseStatusCode();
                UtlString responseText;
                sipMessage->getResponseStatusText(&responseText);

                // Update the expiration members based on this response.
                int expirationPeriod = 0;

                // Provisional response, do nothing
                if (responseCode < SIP_2XX_CLASS_CODE)
                {
                    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                                  "SipRefreshManager::handleMessage Provisional response ignored");
                }

                // There was a resend with credentials for this
                // failed response (should be a 401 or 407 auth.
                // challenge.
                else if (messageType == SipMessageEvent::AUTHENTICATION_RETRY)
                {
                    // Do not stop the timer and do not change the
                    // state from PENDING to FAILED.
                   if (matchesLastLocalTransaction)
                   {
                      // Update the recorded CSeq in mpDialogMgr to account
                      // for the resend that was done by the SipUserAgent.
                      // This is needed so that the hoped-for 200 response
                      // to the resend is seen as matchesLastLocalTransaction
                      // and updates the subscription information.
                      int cseq;
                      UtlString method;
                      sipMessage->getCSeqField(&cseq, &method);
                      cseq++;
                      UtlString* handle =
                         foundDialog ? &dialogHandle : &earlyDialogHandle;
                      UtlBoolean ret =
                         mpDialogMgr->setNextLocalCseq(*handle, cseq);
                      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                                    "SipRefreshManager::handleMessage setNextLocalCseq('%s', %d) = %d",
                                    handle->data(), cseq, ret);
                   }
                   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                                 "SipRefreshManager::handleMessage Authentication retry response ignored");
                }

                // A success or failure response which will not be retried.
                // These change the state of the subscription.
                else
                {
                   // If the refresh state still records the early dialog,
                   // update it to the confirmed dialog.
                   // (And below we may update the Route's in the stored
                   // request.)
                   if (foundEarlyDialog && matchesLastLocalTransaction)
                   {
                       // Replace the state object's handle with the confirmed
                       // dialog handle and put it in the list under the new handle.
                       mRefreshes.removeReference(state);
                       *((UtlString*) state) = dialogHandle;
                       mRefreshes.insert(state);

                       // Update the stored request to have the new to-tag.
                       Url toUrl;
                       sipMessage->getToUrl(toUrl);
                       UtlString toTag;
                       toUrl.getFieldParameter("tag", toTag);
                       state->mpLastRequest->setToFieldTag(toTag);

                       OsSysLog::add(FAC_SIP, PRI_DEBUG,
                                     "SipRefreshManager::handleMessage Updating state %p to established dialog handle '%s' and to-tag '%s'",
                                     state, dialogHandle.data(), toTag.data());

                       // Update the state of the subscription dialog recorded
                       // in mpDialogMgr.
                       mpDialogMgr->updateDialog(*sipMessage);
                   }

                   // Success responses.
                   if (responseCode >= SIP_2XX_CLASS_CODE &&
                       responseCode < SIP_3XX_CLASS_CODE)
                   {
                       // Should we tolerate no Expires header in response?
                       // Currently assume that if Expires header is not
                       // set then we got what we asked for.
                       if (!getAcceptedExpiration(state, *sipMessage, 
                                                  expirationPeriod))
                       {
                           expirationPeriod = state->mExpirationPeriodSeconds;
                       }

                       // SUBSCRIBE or REGISTER gave us expiration seconds
                       // from when the request was sent.
                       if (expirationPeriod > 0)
                       {
                           // Calculate the new (often shorter) subscription refresh timer.
                           int nextResendSeconds = calculateResendTime(expirationPeriod, TRUE);

                           // Check if the accepted timer is less than the originally requested timer.
                           if (expirationPeriod < state->mExpirationPeriodSeconds)
                           { 
                               // Stop and restart the timer with the shorter timeout period.
                               state->mpRefreshTimer->stop();
                               OsTime timerTime(nextResendSeconds, 0);
                               state->mpRefreshTimer->oneshotAfter(timerTime);
                           }

                           state->mExpiration =
                              state->mPendingStartTime + nextResendSeconds;
                           state->mRequestState = REFRESH_REQUEST_SUCCEEDED;

                           // If the request is a SUBSCRIBE, and this
                           // response established a dialog, update
                           // the Route headers of the stored request
                           // based on the Record-Route headers of the
                           // 2xx response, and update the request-URI
                           // based on the Contact in the response.
                           
                           // If this is a 2xx response to a
                           // re-SUBSCRIBE, update the request-URI
                           // (since SUBSCRIBE is a target-refresh
                           // method).

                           // If it is a REGISTER, do not modify the
                           // stored request, since successive
                           // REGISTERS are a quasi-dialog and
                           // continue to be routed as the first
                           // message was.

                           // Use SipMessage::isRecordRouteAccepted to distinguish
                           // the REGISTER and SUBSCRIBE cases.
                           if (state->mpLastRequest->isRecordRouteAccepted())
                           {
                              // Copy Contact URI in response to
                              // request-URI in request.
                              UtlString contact;
                              sipMessage->getContactUri(0, &contact);
                              state->mpLastRequest->changeRequestUri(contact);

                              OsSysLog::add(FAC_SIP, PRI_DEBUG,
                                            "SipRefreshManager::handleMessage Updating request-URI to '%s'",
                                            contact.data());

                              if (foundEarlyDialog && matchesLastLocalTransaction)
                              {
                                 // Remove Route headers from the initial request.
                                 UtlString route;
                                 while (state->mpLastRequest->removeHeader(SIP_ROUTE_FIELD, 0))
                                 {
                                 }

                                 // Copy Record-Route headers from response to
                                 // Route headers in request (in reverse order).
                                 UtlString routeField;
                                 UtlString routeValue;
                                 for (int index = 0;
                                      sipMessage->getRecordRouteUri(index, &routeValue);
                                      index++)
                                 {
                                    if (index != 0)
                                    {
                                       routeField.prepend(",");
                                    }
                                    routeField.prepend(routeValue);
                                 }
                                 state->mpLastRequest->setRouteField(routeField);

                                 OsSysLog::add(FAC_SIP, PRI_DEBUG,
                                               "SipRefreshManager::handleMessage Updating Route headers to '%s'",
                                               routeValue.data());
                              }
                           }
                       }
                       // UnSUBSCRIBE or unREGISTER
                       else
                       {
                           state->mExpiration = 0;
                           // If the notifier gave us a 0 expiration,
                           // it has terminated the subscription.
                           state->mRequestState = REFRESH_REQUEST_FAILED;
                       }

                       // The request succeeded
                       OsSysLog::add(FAC_SIP, PRI_DEBUG,
                                     "SipRefreshManager::handleMessage SUBSCRIBE received success response, state = %p, expirationPeriod = %d, state->mExpiration = %ld",
                                     state, expirationPeriod, state->mExpiration);
                   }

                   // A failure response code other than authentication challenge.
                   // We don't care what  error -- It is the application's job
                   // to care.  End the subscription.
                   else
                   {
                       state->mFailedResponseCode = responseCode;
                       state->mFailedResponseText = responseText;
                       state->mRequestState = REFRESH_REQUEST_FAILED;
                       // Do not change the expiration time, it
                       // is what ever it was before the response was
                       // sent.
                       OsSysLog::add(FAC_SIP, PRI_DEBUG,
                                     "SipRefreshManager::handleMessage Failure");
                   }

                   // Invoke the callback to let the application
                   // know that the state changed
                   if (state->mpStateCallback)
                   {
                      (state->mpStateCallback)(state->mRequestState,
                                               earlyDialogHandle,
                                               dialogHandle,
                                               state->mpApplicationData,
                                               responseCode, // responseCode
                                               responseText, // responseText,
                                               state->mExpiration, // zero means expires now
                                               sipMessage); // response
                   }
                }

                // Check to see if the subscription was terminated, and thus
                // we should delete the state.
                if (state->mRequestState == REFRESH_REQUEST_FAILED)
                {
                   // Stop and delete the refresh timer
                   state->mpRefreshTimer->stop();
                   deleteTimerAndEvent(state->mpRefreshTimer);

                   // Get rid of the dialog
                   mRefreshes.removeReference(state);
                   mpDialogMgr->deleteDialog(*state);

                   // Delete the state object.
                   delete state;

                   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                                 "SipRefreshManager::handleMessage Deleted state %p",
                                 state);
                }
            }
            else
            {
               // Received an event for a dialog that we have no record of.
               OsSysLog::add(FAC_SIP, PRI_WARNING,
                             "SipRefreshManager::handleMessage Received event for unknown dialog handle '%s'",
                             dialogHandle.data());
            }
            unlock();
        }  // endif SUBSCRIBE or REGISTER response
    } // endif SipMessage event

    return (TRUE);
}


/* ============================ ACCESSORS ================================= */

void SipRefreshManager::refreshState2String(RefreshRequestState state, 
                                            UtlString& stateString)
{
    switch(state)
    {
    case REFRESH_REQUEST_UNKNOWN:
        stateString = "REFRESH_REQUEST_UNKNOWN";
        break;
    case REFRESH_REQUEST_PENDING:
        stateString = "REFRESH_REQUEST_PENDING";
        break;
    case REFRESH_REQUEST_FAILED:
        stateString = "REFRESH_REQUEST_FAILED";
        break;
    case REFRESH_REQUEST_SUCCEEDED:
        stateString = "REFRESH_REQUEST_SUCCEEDED";
        break;

    default:
        {
            stateString = "unknown: ";
            char numBuf[20];
            sprintf(numBuf, "%d", state);
            stateString.append(numBuf);
        }
        break;
    }
}

// Get a copy of the refresh request message for a given dialog handle.
UtlBoolean SipRefreshManager::getRequest(const UtlString& dialogHandle,
                                         SipMessage& message)
{
   // Look up the state for the dialog handle.
   RefreshDialogState* state = getAnyDialog(dialogHandle);
   if (state)
   {
      // Copy the message.
      message = *(state->mpLastRequest);
   }

   return state != NULL;
}

int SipRefreshManager::dumpRefreshStates(UtlString& dumpString)
{
    int count = 0;
    dumpString.remove(0);
    lock();
    UtlHashBagIterator iterator(mRefreshes);
    RefreshDialogState* state = NULL;
    UtlString oneStateDump;

    while((state = (RefreshDialogState*) iterator()))
    {
        state->toString(oneStateDump);
        dumpString.append(oneStateDump);
        count++;
    }
    unlock();
    return(count);
}


/* ============================ INQUIRY =================================== */

// Dump the object's internal state.
void SipRefreshManager::dumpState()
{
   lock();

   // indented 4

   OsSysLog::add(FAC_RLS, PRI_INFO,
                 "\t    SipRefreshManager %p, mReceivingRegisterResponses = %d, mReceivingSubscribeResponses = %d, mDefaultExpiration = %d",
                 this,
                 mReceivingRegisterResponses, mReceivingSubscribeResponses,
                 mDefaultExpiration);

   UtlHashBagIterator itor2(mRefreshes);
   RefreshDialogState* state;
   while ((state = dynamic_cast <RefreshDialogState*> (itor2())))
   {
      state->dumpState();
   }

   unlock();
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

void SipRefreshManager::lock()
{
    mRefreshMgrMutex.acquire();
}

void SipRefreshManager::unlock()
{
    mRefreshMgrMutex.release();
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

RefreshDialogState* SipRefreshManager::getAnyDialog(const UtlString& messageDialogHandle)
{
    RefreshDialogState* state = (RefreshDialogState*)
        mRefreshes.find(&messageDialogHandle);

    if (state == NULL)
    {
        UtlString reversedHandle;
        SipDialog::reverseTags(messageDialogHandle, reversedHandle);
        state = (RefreshDialogState*)
            mRefreshes.find(&reversedHandle);
    }

    // It did not match
    if (state == NULL)
    {
        // If this is an early dialog handle find out what the 
        // established dialog is      
        UtlString establishedDialogHandle;
        if (SipDialog::isEarlyDialog(messageDialogHandle) &&
            mpDialogMgr->getEstablishedDialogHandleFor(messageDialogHandle,
                                                       establishedDialogHandle))
        {
            state = (RefreshDialogState*) 
                mRefreshes.find(&establishedDialogHandle);
            if(state == NULL)
            {
                UtlString reversedEstablishedDialogHandle;
                SipDialog::reverseTags(establishedDialogHandle, reversedEstablishedDialogHandle);
                state = (RefreshDialogState*) 
                    mRefreshes.find(&reversedEstablishedDialogHandle);
            }
        }

        // If this is an established dialog, find out what the
        // early dialog handle was and see if we can find it
        else
        {
            UtlString earlyDialogHandle;
            mpDialogMgr->getEarlyDialogHandleFor(messageDialogHandle,
                                                 earlyDialogHandle);

            state = (RefreshDialogState*) 
                mRefreshes.find(&earlyDialogHandle);
            if(state == NULL)
            {
                UtlString reversedEarlyDialogHandle;
                SipDialog::reverseTags(earlyDialogHandle, reversedEarlyDialogHandle);
                state = (RefreshDialogState*) 
                    mRefreshes.find(&reversedEarlyDialogHandle);
            }
        }
    }

    return(state);
}

UtlBoolean SipRefreshManager::stateExists(RefreshDialogState* statePtr)
{
    // Our caller holds the lock.

    RefreshDialogState* state =
       (RefreshDialogState*) mRefreshes.findReference(statePtr);

    return(state != NULL);
}

RefreshDialogState* 
    SipRefreshManager::createNewRefreshState(SipMessage& subscribeOrRegisterRequest,
                                              UtlString& messageDialogHandle,
                                              void* applicationData,
                                              const RefreshStateCallback refreshStateCallback,
                                              int& requestedExpiration)
{
    RefreshDialogState* state = new RefreshDialogState();
    *((UtlString*) state) = messageDialogHandle;
    state->mpApplicationData = applicationData;
    state->mpStateCallback = refreshStateCallback;
    if(!getInitialExpiration(subscribeOrRegisterRequest, 
                             state->mExpirationPeriodSeconds)) // original expiration
    {
        state->mExpirationPeriodSeconds = mDefaultExpiration;
        subscribeOrRegisterRequest.setExpiresField(mDefaultExpiration);
    }
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "SipRefreshManager::createNewRefreshState %p->mExpirationPeriodSeconds = %d",
                  state, state->mExpirationPeriodSeconds);

    requestedExpiration = state->mExpirationPeriodSeconds;

    state->mPendingStartTime = 0;
    state->mExpiration = 0;
    state->mRequestState = REFRESH_REQUEST_UNKNOWN;
    state->mFailedResponseCode = 0;
    state->mFailedResponseText = NULL;
    state->mpRefreshTimer = NULL;  
    state->mpLastRequest = NULL;

    return(state);
}

void SipRefreshManager::setRefreshTimer(RefreshDialogState& state, 
                                        UtlBoolean isSuccessfulReschedule)
{
    // Create and set a new timer for the next time out period
    int nextResendSeconds = 
        calculateResendTime(state.mExpirationPeriodSeconds,
                            isSuccessfulReschedule);
    // If a signficant amount of time has passed since the prior
    // request was sent, decrease the error timeout a bit.
    // This is only a problem with the error case as in the
    // successful case we set the timer before sending the
    // request.
    if (!isSuccessfulReschedule)
    {
        long now = OsDateTime::getSecsSinceEpoch();
        if (state.mPendingStartTime > 0 &&
            now - state.mPendingStartTime > 5)
        {
            nextResendSeconds = nextResendSeconds - now + state.mPendingStartTime;
            if (nextResendSeconds < 30)
            {
                nextResendSeconds = 30;
            }
        }
    }

    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "SipRefreshManager::setRefreshTimer setting resend timeout in %d seconds",
                  nextResendSeconds);

    OsMsgQ* incomingQ = getMessageQueue();
    OsTimer* resendTimer = new OsTimer(incomingQ, &state);
    state.mpRefreshTimer = resendTimer;
    OsTime timerTime(nextResendSeconds, 0);
    resendTimer->oneshotAfter(timerTime);                
}

int SipRefreshManager::calculateResendTime(int requestedExpiration, 
                                           UtlBoolean isSuccessfulResend)
{
    int expiration;
    if (isSuccessfulResend)
    {
        expiration = (int)(0.55 * requestedExpiration);
    }
    else
    {
        expiration = (int)(0.1 * requestedExpiration);
    }

    // The transaction timeout is the minimum time we will return.
    int minRefresh = (mpUserAgent->getSipStateTransactionTimeout()) / 1000;
    if (expiration < minRefresh)
    {
        expiration = minRefresh;
    }

    return(expiration);
}

void SipRefreshManager::stopTimerForFailureReschedule(OsTimer* resendTimer)
{
    if(resendTimer)
    {
        resendTimer->stop();
        OsQueuedEvent* queuedEvent = 
            (OsQueuedEvent*) resendTimer->getNotifier();

        // If the queued event exists fire it now with an error status
        // to indicate that it should be resheduled with the (shorter)
        // error timeout.  Normally the timer is scheduled with the
        // timeout assuming that the reSUBSCRIBE or reREGISTER will
        // succeed.
        if(queuedEvent)
        {
            // Effectively make the timer fire now
            queuedEvent->signal(OS_INTERRUPTED);  // CANCELED
        }
    }
}

void SipRefreshManager::deleteTimerAndEvent(OsTimer*& timer)
{
    if (timer)
    {
        delete timer;
        timer = NULL;
    }
}

void SipRefreshManager::setForResend(RefreshDialogState& state, 
                                     UtlBoolean expireNow)
{
    if(state.mpLastRequest)
    {
        if (OsSysLog::willLog(FAC_SIP, PRI_DEBUG))
        {
           UtlString lastRequest;
           ssize_t length;
           state.mpLastRequest->getBytes(&lastRequest, &length);
           OsSysLog::add(FAC_SIP, PRI_DEBUG,
                         "SipRefreshManager::setForResend last request = '%s'",
                         lastRequest.data());
        }
       
        // Remove old vias
        state.mpLastRequest->removeTopVia();

        // Do not modify the Route headers in *state->mpLastRequest.
        // If it is a REGISTER, they are what was specified to ::initiateRefresh
        // (and should be maintained, as successive REGISTERS are a quasi-dialog
        // and continue to be routed as the first message was).
        // If it is a SUBSCRIBE, the Route headers were revised based on the
        // Record-Route's in the 2xx response that created the dialog.

        // Remove any credentials
        while(state.mpLastRequest->removeHeader(HTTP_AUTHORIZATION_FIELD, 0))
        {
        }
        while(state.mpLastRequest->removeHeader(HTTP_PROXY_AUTHORIZATION_FIELD, 0))
        {
        }

        // Remove transport state info
        state.mpLastRequest->resetTransport();

        // Set the dialog info and cseq
        mpDialogMgr->setNextLocalTransactionInfo(*(state.mpLastRequest));

        // Set the expiration
        if (expireNow)
        {
           state.mpLastRequest->setExpiresField(0);
        }

        // The Date header will be set by the SipUserAgent.
    }
}


UtlBoolean SipRefreshManager::getInitialExpiration(const SipMessage& sipRequest, 
                                                   int& expirationPeriod)
{
    UtlString method;
    UtlBoolean foundExpiration = FALSE;
    sipRequest.getRequestMethod(&method);

    if(method.compareTo(SIP_REGISTER_METHOD) == 0)
    {
        // Register could have it in the Contact header
        UtlString requestContactValue;
        if(sipRequest.getContactEntry(0 , &requestContactValue))
        {
            // Get the expires parameter for the contact if it exists
            Url contactUri(requestContactValue);
            UtlString contactExpiresParameter;
            if(contactUri.getFieldParameter(SIP_EXPIRES_FIELD, 
                                            contactExpiresParameter) &&
               !contactExpiresParameter.isNull())
            {
                foundExpiration = TRUE;

                // Convert to int
                expirationPeriod = atoi(contactExpiresParameter);
            }
        }
    }

    if(!foundExpiration)
    {
        // Not sure if we care if this is a request or response
        foundExpiration = sipRequest.getExpiresField(&expirationPeriod);
    }

    return(foundExpiration);
}

UtlBoolean SipRefreshManager::getAcceptedExpiration(RefreshDialogState* state,
                                                    const SipMessage& sipResponse, 
                                                    int& expirationPeriod)
{
    UtlString method;
    UtlBoolean foundExpiration = FALSE;
    int cseq;
    sipResponse.getCSeqField(&cseq, &method);

    // Look for an expiration time in a REGISTER response attached to
    // the contact.
    if (method.compareTo(SIP_REGISTER_METHOD) == 0)
    {
        // Get the presumably first contact in the REGISTER request
        // so that we can find the same contact in the response and
        // find out what the expiration is
        UtlString requestContact;
        Url requestContactUri;
        if(state && state->mpLastRequest &&
           state->mpLastRequest->getContactEntry(0, &requestContact))
        {
           requestContactUri = requestContact;
        }

        // Register could have it in the Contact header
        UtlString responseContactValue;
        int contactIndex = 0;
        while(sipResponse.getContactEntry(contactIndex , &responseContactValue))
        {
            // Get the expires parameter for the contact if it exists
            Url contactUri(responseContactValue);

            if(requestContactUri.isUserHostPortEqual(contactUri))
            {
                UtlString contactExpiresParameter;
                if(contactUri.getFieldParameter(SIP_EXPIRES_FIELD, 
                        contactExpiresParameter) &&
                   !contactExpiresParameter.isNull())
                {
                    foundExpiration = TRUE;

                    // Convert to int
                    expirationPeriod = atoi(contactExpiresParameter);
                }
            }
            contactIndex++;
        }
    }

    // If there wasn't an expiration of a REGISTER contact, look for an
    // Expires header.
    if (!foundExpiration)
    {
        foundExpiration = sipResponse.getExpiresField(&expirationPeriod);
    }

    return (foundExpiration);
}

/* ============================ FUNCTIONS ================================= */
