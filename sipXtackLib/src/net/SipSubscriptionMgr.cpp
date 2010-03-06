//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <utl/UtlString.h>
#include <utl/UtlHashBagIterator.h>
#include <os/OsSysLog.h>
#include <os/OsTimer.h>
#include <os/OsDateTime.h>
#include <net/SipPublishContentMgr.h>
#include <net/SipSubscriptionMgr.h>
#include <net/SipMessage.h>
#include <net/SipDialogMgr.h>
#include <net/SipDialog.h>
#include <net/NetMd5Codec.h>
#include <net/CallId.h>

// Private class whose instances contain the state of a single subscription.
class SubscriptionServerState : public UtlString
{
public:
    SubscriptionServerState(const UtlString& dialogHandle);

    virtual ~SubscriptionServerState();

    // Parent UtlString contains the dialog handle
    UtlString mResourceId;
    UtlString mEventTypeKey;
    UtlString mEventType;
    UtlString mAcceptHeaderValue;
    long mExpirationDate;       // expiration time
    int mDialogVer;             // the last value used in a 'version' attribute
    SipMessage* mpLastSubscribeRequest;
    OsTimer* mpExpirationTimer;

    //! Dump the object's internal state.
    void dumpState();

private:
    //! DISALLOWED accidental copying
    SubscriptionServerState(const SubscriptionServerState& rSubscriptionServerState);
    SubscriptionServerState& operator=(const SubscriptionServerState& rhs);
};

class SubscriptionServerStateIndex : public UtlString
{
public:
    SubscriptionServerStateIndex(const UtlString& resourceId,
                                 const UtlString& eventTypeKey,
                                 SubscriptionServerState* state);

    virtual ~SubscriptionServerStateIndex();

    // Parent UtlString is the resourceId and eventTypeKey.
    SubscriptionServerState* mpState;

private:
    //! DISALLOWED accidental copying
    SubscriptionServerStateIndex(const SubscriptionServerStateIndex& rSubscriptionServerStateIndex);
    SubscriptionServerStateIndex& operator=(const SubscriptionServerStateIndex& rhs);
};

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// Initial expiration parameters.
#define INITIAL_MIN_EXPIRATION 32
#define INITIAL_MAX_EXPIRATION 86400
#define INITIAL_DEFAULT_EXPIRATION 3600
// Limit expiration parameters.
#define LIMIT_MIN_EXPIRATION 32
#define LIMIT_MAX_EXPIRATION 86400

// Used to separate the resourceId from the eventTypeKey in
// SubscriptionServerStateIndex value.
#define CONTENT_KEY_SEPARATOR "\001"

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

SubscriptionServerState::SubscriptionServerState(const UtlString& dialogHandle)
   : UtlString(dialogHandle)
   , mExpirationDate(-1)
   , mDialogVer(-1)
   , mpLastSubscribeRequest(NULL)
   , mpExpirationTimer(NULL)
{
}
SubscriptionServerState::~SubscriptionServerState()
{
    if(mpLastSubscribeRequest)
    {
        delete mpLastSubscribeRequest;
        mpLastSubscribeRequest = NULL;
    }

    if(mpExpirationTimer)
    {
        // Timer should have been stopped and the the task upon
        // which the fired timer queues its message need to have
        // synchronized to make sure it does not get touched after
        // it is deleted here.
        delete mpExpirationTimer;
        mpExpirationTimer = NULL;
    }
}

// Dump the object's internal state.
void SubscriptionServerState::dumpState()
{
   // indented 6

   OsSysLog::add(FAC_RLS, PRI_INFO,
                 "\t      SubscriptionServerState %p UtlString = '%s', mResourceId = '%s', mEventTypeKey = '%s', "
                 "mAcceptHeaderValue = '%s', mExpirationDate = %+d, mDialogVer = %d",
                 this, data(), mResourceId.data(), mEventTypeKey.data(),
                 mAcceptHeaderValue.data(),
                 (int) (mExpirationDate - OsDateTime::getSecsSinceEpoch()),
                 mDialogVer);
}

SubscriptionServerStateIndex::SubscriptionServerStateIndex(const UtlString& resourceId,
                                                           const UtlString& eventTypeKey,
                                                           SubscriptionServerState* state)
{
   append(resourceId);
   append(CONTENT_KEY_SEPARATOR);
   append(eventTypeKey);
   mpState = state;
}

SubscriptionServerStateIndex::~SubscriptionServerStateIndex()
{
    // Do not delete mpState, it is freed elsewhere.
}

// Constructor
SipSubscriptionMgr::SipSubscriptionMgr()
   : mEstablishedDialogCount(0)
 , mSubscriptionMgrMutex(OsMutex::Q_FIFO)
 , mMinExpiration(INITIAL_MIN_EXPIRATION)
 , mDefaultExpiration(INITIAL_DEFAULT_EXPIRATION)
 , mMaxExpiration(INITIAL_MAX_EXPIRATION)
{
}


// Copy constructor NOT IMPLEMENTED
SipSubscriptionMgr::SipSubscriptionMgr(const SipSubscriptionMgr& rSipSubscriptionMgr)
: mSubscriptionMgrMutex(OsMutex::Q_FIFO)
{
}


// Destructor
SipSubscriptionMgr::~SipSubscriptionMgr()
{
    // Iterate through and delete all the dialogs
    // TODO:
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
SipSubscriptionMgr&
SipSubscriptionMgr::operator=(const SipSubscriptionMgr& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

UtlBoolean SipSubscriptionMgr::updateDialogInfo(const SipMessage& subscribeRequest,
                                                UtlString& resourceId,
                                                UtlString& eventTypeKey,
                                                UtlString& eventType,
                                                UtlString& subscribeDialogHandle,
                                                UtlBoolean& isNew,
                                                UtlBoolean& isSubscriptionExpired,
                                                SipMessage& subscribeResponse,
                                                SipSubscribeServerEventHandler& handler)
{
    isNew = FALSE;
    UtlBoolean subscriptionSucceeded = FALSE;
    UtlString dialogHandle;
    subscribeRequest.getDialogHandle(dialogHandle);
    SubscriptionServerState* state = NULL;
    int expiration = -1;
    isSubscriptionExpired = TRUE;

    // Double check the sanity of the class attributes

    if(mMaxExpiration < mMinExpiration)
    {
        // This is an error case. Switch the values so that we do not
        // run into any negative expiration times.
        int tmp = mMaxExpiration;
        mMaxExpiration = mMinExpiration;
        mMinExpiration = tmp;

        OsSysLog::add(FAC_SIP, PRI_WARNING,
            "Swapping values as mMinExpiration (%d) is greater than mMaxExpiration (%d)",
            mMinExpiration, mMaxExpiration);
    }

    if(mMaxExpiration < mDefaultExpiration)
    {
        // This is an error case. Switch the values so that we do not
        // run into any negative expiration times.
        int tmp = mMaxExpiration;
        mMaxExpiration = mDefaultExpiration;
        mDefaultExpiration = tmp;

        OsSysLog::add(FAC_SIP, PRI_WARNING,
            "Swapping values as mDefaultExpiration (%d) is greater than mMaxExpiration (%d)",
            mDefaultExpiration, mMaxExpiration);
    }

    // Set the expires period randomly
    int spreadFloor = mMinExpiration*2;
    if(!subscribeRequest.getExpiresField(&expiration))
    {
        // no expires field
        // spread it between the default expiration and max allowed expiration
        expiration = (  (rand() % (mMaxExpiration - mDefaultExpiration))
                       + mDefaultExpiration);
    }
    else if ( expiration >= mMaxExpiration )
    {
        if (mMaxExpiration > spreadFloor)
        {
            // - spread it between the spreadFloor and the max allowed expiration
            expiration = (  (rand() % (mMaxExpiration - spreadFloor))
                           + spreadFloor);
        }
        else
        {
            // Max Expiration is smaller than the spreadFloor, hence
            // spread it between the min and the max allowed expiration
            expiration = (  (rand() % (mMaxExpiration - mMinExpiration))
                           + mMinExpiration);
        }
    }
    else if ( expiration > spreadFloor )
    {
        // a normal (long) expiration
        // - spread it between the spreadFloor and the longest they asked for
        expiration = (  (rand() % (expiration - spreadFloor))
                       + spreadFloor);
    }
    else if ( expiration > mMinExpiration )
    {
        // a short but greater than minimum expiration
        // - spread it between the min and the longest they asked for
        expiration = (  (rand() % (expiration - mMinExpiration))
                       + mMinExpiration);
    }
    // If requested expiration == mMinExpiration, leave it unchanged.
    // Cases where the expiration is less than mMinExpiration are handled below.
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "SipSubscriptionMgr::updateDialogInfo calculated expiration = %d",
                  expiration);

    // If this is an early dialog we need to make it an established dialog.
    if(SipDialog::isEarlyDialog(dialogHandle))
    {
        UtlString establishedDialogHandle;
        if(mDialogMgr.getEstablishedDialogHandleFor(dialogHandle, establishedDialogHandle))
        {
            OsSysLog::add(FAC_SIP, PRI_WARNING,
                "Incoming early SUBSCRIBE dialog: %s matches established dialog: %s",
                dialogHandle.data(), establishedDialogHandle.data());
        }

        // make up a To tag and set it
        UtlString toTag;
        CallId::getNewTag(toTag);

        // Acceptable expiration, create a subscription and dialog
        if(expiration >= mMinExpiration ||
           expiration == 0 ||
           // :WORKAROUND:  Also allow expiration == 1, to support the
           // 1-second subscriptions the pick-up agent makes because
           // current Snom phones do not respond to 0-second subscriptions.
           // See XPB-399 and ENG-319.
           expiration == 1)
        {
            // Call the event-specific function to determine the resource ID
            // and event type key for this SUBSCRIBE.
            handler.getKeys(subscribeRequest,
                            resourceId,
                            eventTypeKey,
                            eventType);

            // Create a dialog and subscription state even if
            // the expiration is zero as we need the dialog info
            // to route the one-time NOTIFY.  The immediately
            // expired dialog will be garbage collected.

            SipMessage* subscribeCopy = new SipMessage(subscribeRequest);
            subscribeCopy->setToFieldTag(toTag);

            // Re-get the dialog handle now that the To tag is set
            subscribeCopy->getDialogHandle(dialogHandle);

            // Create the dialog
            mDialogMgr.createDialog(*subscribeCopy, FALSE, dialogHandle);
            isNew = TRUE;

            // Create a subscription state
            state = new SubscriptionServerState(dialogHandle);
            state->mEventTypeKey = eventTypeKey;
            state->mEventType = eventType;
            state->mpLastSubscribeRequest = subscribeCopy;
            state->mResourceId = resourceId;
            if (!subscribeCopy->getAcceptField(state->mAcceptHeaderValue))
            {
               // No Accept header seen, set special value allowing any
               // content type.
               state->mAcceptHeaderValue = SipPublishContentMgr::acceptAllTypes;
            }

            long now = OsDateTime::getSecsSinceEpoch();
            state->mExpirationDate = now + expiration;

            // TODO: currently the SipSubsribeServer does not handle timeout
            // events to send notifications that the subscription has ended.
            // So we do not set a timer at the end of the subscription
            state->mpExpirationTimer = NULL;

            // Create the index by resourceId and eventTypeKey key
            SubscriptionServerStateIndex* stateKey =
               new SubscriptionServerStateIndex(resourceId, eventTypeKey, state);

            subscribeResponse.setResponseData(subscribeCopy,
                                              SIP_ACCEPTED_CODE,
                                              SIP_ACCEPTED_TEXT,
                                              NULL);
            subscribeResponse.setExpiresField(expiration);
            subscribeCopy->getDialogHandle(subscribeDialogHandle);

            lock();
            mSubscriptionStatesByDialogHandle.insert(state);
            mSubscriptionStateResourceIndex.insert(stateKey);
            if (OsSysLog::willLog(FAC_SIP, PRI_DEBUG))
            {
               UtlString requestContact;
               subscribeRequest.getContactField(0, requestContact);
               OsSysLog::add(FAC_SIP, PRI_DEBUG,
                             "SipSubscriptionMgr::updateDialogInfo insert early-dialog subscription for dialog handle '%s', key '%s', "
                             "contact '%s', mExpirationDate %ld, eventTypeKey '%s'",
                             state->data(), stateKey->data(),
                             requestContact.data(), state->mExpirationDate, state->mEventTypeKey.data());
            }

            // Not safe to touch these after we unlock
            stateKey = NULL;
            state = NULL;
            subscribeCopy = NULL;
            unlock();

            subscriptionSucceeded = TRUE;

            // One time subscribe?
            isSubscriptionExpired = expiration == 0;
        }
        // Expiration too small
        else
        {
            // Set expiration too small error
            subscribeResponse.setResponseData(&subscribeRequest,
                                                SIP_TOO_BRIEF_CODE,
                                                SIP_SUB_TOO_BRIEF_TEXT);
            subscribeResponse.setMinExpiresField(mMinExpiration);
            isSubscriptionExpired = TRUE;
        }
    }

    // Not an early dialog handle -- The dialog for this message should already exist
    else
    {
        // Acceptable expiration, create a subscription and dialog
        if(expiration >= mMinExpiration ||
           expiration == 0)
        {
            // Update the dialog state
            mDialogMgr.updateDialog(subscribeRequest, dialogHandle);

            // Get the subscription state and update that
            // TODO:  This assumes that no one reuses the same dialog
            // to subscribe to more than one event type.  mSubscriptionStatesByDialogHandle
            // will need to be changed to a HashBag and we will need to
            // search through to find a matching event type
            lock();
            state = (SubscriptionServerState*)
                mSubscriptionStatesByDialogHandle.find(&dialogHandle);
            if(state)
            {
                // Update the expiration time.
                long now = OsDateTime::getSecsSinceEpoch();
                state->mExpirationDate = now + expiration;
                // Record this SUBSCRIBE as the latest SUBSCRIBE request.
                if(state->mpLastSubscribeRequest)
                {
                    delete state->mpLastSubscribeRequest;
                }
                state->mpLastSubscribeRequest = new SipMessage(subscribeRequest);
                subscribeRequest.getAcceptField(state->mAcceptHeaderValue);

                // Set our Contact to the same request URI that came in
                UtlString contact;
                subscribeRequest.getRequestUri(&contact);

                // Add the angle brackets to Contact, since it is a name-addr.
                Url url(contact);
                url.includeAngleBrackets();
                contact = url.toString();

                subscribeResponse.setResponseData(&subscribeRequest,
                                                SIP_ACCEPTED_CODE,
                                                SIP_ACCEPTED_TEXT,
                                                contact);
                subscribeResponse.setExpiresField(expiration);
                subscriptionSucceeded = TRUE;
                isSubscriptionExpired = FALSE;
                subscribeDialogHandle = dialogHandle;

                // Set the resource information so our caller can generate a NOTIFY.
                resourceId = state->mResourceId;
                eventTypeKey = state->mEventTypeKey;
            }

            // No state, but SUBSCRIBE had a to-tag.
            else
            {
               // Unknown subscription.
               subscribeResponse.setResponseData(&subscribeRequest,
                                                 SIP_BAD_SUBSCRIPTION_CODE,
                                                 SIP_BAD_SUBSCRIPTION_TEXT);
            }
            unlock();
        }

        // Expiration too small
        else
        {
            // Set expiration too small error
            subscribeResponse.setResponseData(&subscribeRequest,
                                                SIP_TOO_BRIEF_CODE,
                                                SIP_SUB_TOO_BRIEF_TEXT);
            subscribeResponse.setMinExpiresField(mMinExpiration);
            isSubscriptionExpired = isExpired(dialogHandle);
        }
    }

    return(subscriptionSucceeded);
}


UtlBoolean SipSubscriptionMgr::insertDialogInfo(const SipMessage& subscribeRequest,
                                                const UtlString& resourceId,
                                                const UtlString& eventTypeKey,
                                                const UtlString& eventType,
                                                int expires,
                                                int notifyCSeq,
                                                int version,
                                                UtlString& subscribeDialogHandle,
                                                UtlBoolean& isNew)
{
    if (OsSysLog::willLog(FAC_SIP, PRI_DEBUG))
    {
       UtlString request;
       ssize_t len;
       subscribeRequest.getBytes(&request, &len);

       OsSysLog::add(FAC_SIP, PRI_DEBUG,
                     "SipSubscriptionMgr::insertDialogInfo "
                     "resourceId = '%s', eventTypeKey = '%s', eventType = '%s', "
                     "expires = %d, notifyCSeq = %d, version = %d, "
                     "subscribeRequest = '%s'",
                     resourceId.data(), eventTypeKey.data(), eventType.data(),
                     expires, notifyCSeq, version,
                     request.data());
    }

    isNew = FALSE;
    UtlBoolean subscriptionSucceeded = FALSE;
    UtlString dialogHandle;
    subscribeRequest.getDialogHandle(dialogHandle);
    SubscriptionServerState* state = NULL;

    // If this is an early dialog we need to make it an established dialog.
    if (SipDialog::isEarlyDialog(dialogHandle))
    {
        OsSysLog::add(FAC_SIP, PRI_DEBUG,
                      "SipSubscriptionMgr::insertDialogInfo is an early dialog handle");

        UtlString establishedDialogHandle;
        if (mDialogMgr.getEstablishedDialogHandleFor(dialogHandle, establishedDialogHandle))
        {
            OsSysLog::add(FAC_SIP, PRI_WARNING,
                "Incoming early SUBSCRIBE dialog: %s matches established dialog: %s",
                dialogHandle.data(), establishedDialogHandle.data());
        }

        // make up a To tag and set it
        UtlString toTag;
        CallId::getNewTag(toTag);

        // Create a dialog and subscription state even if
        // the expiration is zero as we need the dialog info
        // to route the one-time NOTIFY.  The immediately
        // expired dialog will be garbage collected.

        SipMessage* subscribeCopy = new SipMessage(subscribeRequest);
        subscribeCopy->setToFieldTag(toTag);

        // Re-get the dialog handle now that the To tag is set
        subscribeCopy->getDialogHandle(dialogHandle);

        // Create the dialog
        mDialogMgr.createDialog(*subscribeCopy, FALSE, dialogHandle);
        // Set the recorded CSeq of the last NOTIFY.
        mDialogMgr.setNextLocalCseq(dialogHandle, notifyCSeq);
        isNew = TRUE;

        // Create a subscription state
        state = new SubscriptionServerState(dialogHandle);
        state->mEventTypeKey = eventTypeKey;
        state->mEventType = eventType;
        state->mpLastSubscribeRequest = subscribeCopy;
        state->mResourceId = resourceId;
        subscribeCopy->getAcceptField(state->mAcceptHeaderValue);
        state->mExpirationDate = expires;
        state->mDialogVer = version;

        // TODO: currently the SipSubscribeServer does not handle timeout
        // events to send notifications that the subscription has ended.
        // So we do not set a timer at the end of the subscription
        state->mpExpirationTimer = NULL;

        // Create the index by resourceId and eventTypeKey key
        SubscriptionServerStateIndex* stateKey =
           new SubscriptionServerStateIndex(resourceId, eventTypeKey, state);

        subscribeCopy->getDialogHandle(subscribeDialogHandle);

        lock();
        mSubscriptionStatesByDialogHandle.insert(state);
        mSubscriptionStateResourceIndex.insert(stateKey);
        if (OsSysLog::willLog(FAC_SIP, PRI_DEBUG))
        {
           UtlString requestContact;
           subscribeRequest.getContactField(0, requestContact);
           OsSysLog::add(FAC_SIP, PRI_DEBUG,
                         "SipSubscriptionMgr::insertDialogInfo insert early-dialog subscription for dialog handle '%s', key '%s', contact '%s', mExpirationDate %ld",
                         state->data(), stateKey->data(),
                         requestContact.data(), state->mExpirationDate);
        }

        // Not safe to touch these after we unlock
        stateKey = NULL;
        state = NULL;
        subscribeCopy = NULL;
        unlock();

        subscriptionSucceeded = TRUE;
    }

    // Not an early dialog handle -- The dialog for this message may already exist
    else
    {
        // Update the dialog state
        mDialogMgr.updateDialog(subscribeRequest, dialogHandle);

        // Get the subscription state and update that
        // TODO:  This assumes that no one reuses the same dialog
        // to subscribe to more than one event type.  mSubscriptionStatesByDialogHandle
        // will need to be changed to a HashBag and we will need to
        // search through to find a matching event type
        lock();
        state = (SubscriptionServerState*)
            mSubscriptionStatesByDialogHandle.find(&dialogHandle);
        if (state)
        {
           OsSysLog::add(FAC_SIP, PRI_DEBUG,
                         "SipSubscriptionMgr::insertDialogInfo "
                         "is an established dialog handle, state found");

            // Set the recorded CSeq of the last NOTIFY.
            mDialogMgr.setNextLocalCseq(dialogHandle, notifyCSeq);

            state->mExpirationDate = expires;
            state->mDialogVer = version;
            if(state->mpLastSubscribeRequest)
            {
                delete state->mpLastSubscribeRequest;
            }
            state->mpLastSubscribeRequest = new SipMessage(subscribeRequest);
            subscribeRequest.getAcceptField(state->mAcceptHeaderValue);

            // Set the contact to the same request URI that came in
            UtlString contact;
            subscribeRequest.getRequestUri(&contact);

            // Add the angle brackets for contact
            Url url(contact);
            url.includeAngleBrackets();
            contact = url.toString();

            subscriptionSucceeded = TRUE;
            subscribeDialogHandle = dialogHandle;
        }

        // No state, but SUBSCRIBE had a to-tag.
        else
        {
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "SipSubscriptionMgr::insertDialogInfo "
                          "is an established dialog handle, no state found");

            SipMessage* subscribeCopy = new SipMessage(subscribeRequest);
            // Create the dialog
            mDialogMgr.createDialog(*subscribeCopy, FALSE, dialogHandle);
            // Set the recorded CSeq of the last NOTIFY.
            mDialogMgr.setNextLocalCseq(dialogHandle, notifyCSeq);
            isNew = TRUE;

            // Create a subscription state
            state = new SubscriptionServerState(dialogHandle);
            state->mEventTypeKey = eventTypeKey;
            state->mEventType = eventType;
            state->mpLastSubscribeRequest = subscribeCopy;
            state->mResourceId = resourceId;
            subscribeCopy->getAcceptField(state->mAcceptHeaderValue);

            state->mExpirationDate = expires;
            state->mDialogVer = version;
            // TODO: currently the SipSubscribeServer does not handle timeout
            // events to send notifications that the subscription has ended.
            // So we do not set a timer at the end of the subscription
            state->mpExpirationTimer = NULL;

            // Create the index by resourceId and eventTypeKey key
            SubscriptionServerStateIndex* stateKey =
               new SubscriptionServerStateIndex(resourceId, eventTypeKey, state);
            mSubscriptionStatesByDialogHandle.insert(state);
            mSubscriptionStateResourceIndex.insert(stateKey);
            if (OsSysLog::willLog(FAC_SIP, PRI_DEBUG))
            {
               UtlString requestContact;
               subscribeRequest.getContactField(0, requestContact);
               OsSysLog::add(FAC_SIP, PRI_DEBUG,
                     "SipSubscriptionMgr::insertDialogInfo insert subscription for key '%s', contact '%s', mExpirationDate %ld",
                     stateKey->data(), requestContact.data(), state->mExpirationDate);
            }

            // Not safe to touch these after we unlock
            stateKey = NULL;
            state = NULL;
            subscribeCopy = NULL;

            // Set the contact to the same request URI that came in
            UtlString contact;
            subscribeRequest.getRequestUri(&contact);

            // Add the angle brackets for contact
            Url url(contact);
            url.includeAngleBrackets();
            contact = url.toString();

            subscriptionSucceeded = TRUE;
        }
        unlock();
    }

    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "SipSubscriptionMgr::insertDialogInfo "
                  "subscribeDialogHandle = '%s', isNew = %d, ret = %d",
                  subscribeDialogHandle.data(), isNew, subscriptionSucceeded);
    return subscriptionSucceeded;
}

UtlBoolean SipSubscriptionMgr::getNotifyDialogInfo(const UtlString& subscribeDialogHandle,
                                                   SipMessage& notifyRequest,
                                                   const char* subscriptionStateFormat,
                                                   UtlString* resourceId,
                                                   UtlString* eventTypeKey,
                                                   UtlString* eventType,
                                                   UtlString* acceptHeaderValue)
{
    UtlBoolean notifyInfoSet = FALSE;
    lock();
    SubscriptionServerState* state =
       dynamic_cast <SubscriptionServerState*>
       (mSubscriptionStatesByDialogHandle.find(&subscribeDialogHandle));

    if (state)
    {
        notifyInfoSet = mDialogMgr.setNextLocalTransactionInfo(notifyRequest,
                                                               SIP_NOTIFY_METHOD,
                                                               subscribeDialogHandle);

        // Set the event header, if we know what it is.
        if (state->mpLastSubscribeRequest)
        {
            UtlString eventHeader;
            state->mpLastSubscribeRequest->getEventField(eventHeader);
            notifyRequest.setEventField(eventHeader);
        }

        // Set the Subscription-State header.
        long expires =
           state->mExpirationDate - OsDateTime::getSecsSinceEpoch();
        char buffer[101];
        sprintf(buffer,
                // The caller does not know the expiration time.
                // If expires <=0, terminate for time-out.
                // Otherwise, use the format given by the caller.
                (expires > 0 ? subscriptionStateFormat : "terminated;reason=timeout"),
                expires);
        notifyRequest.setHeaderValue(SIP_SUBSCRIPTION_STATE_FIELD, buffer, 0);

        // Return information about the subscription.
        if (resourceId)
        {
           *resourceId = state->mResourceId;
        }
        if (eventTypeKey)
        {
           *eventTypeKey = state->mEventTypeKey;
        }
        if (eventType)
        {
           *eventType = state->mEventType;
        }
        if (acceptHeaderValue)
        {
           *acceptHeaderValue = state->mAcceptHeaderValue;
        }
    }
    else
    {
       OsSysLog::add(FAC_SIP, PRI_ERR,
                     "SipSubscriptionMgr::getNotifyDialogInfo No subscription state found for handle '%s'",
                     subscribeDialogHandle.data());
    }

    unlock();

    return(notifyInfoSet);
}

// Construct a NOTIFY request for each subscription/dialog subscribed
// to the given resourceId and eventTypeKey
void SipSubscriptionMgr::createNotifiesDialogInfo(const char* resourceId,
                                                  const char* eventTypeKey,
                                                  const char* subscriptionStateFormat,
                                                  int& numNotifiesCreated,
                                                  UtlString**& acceptHeaderValuesArray,
                                                  SipMessage**& notifyArray)
{
   UtlString contentKey(resourceId);
   contentKey.append(CONTENT_KEY_SEPARATOR);
   contentKey.append(eventTypeKey);

   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipSubscriptionMgr::createNotifiesDialogInfo try to find contentKey '%s' in mSubscriptionStateResourceIndex (%zu entries)",
                 contentKey.data(), mSubscriptionStateResourceIndex.entries());

   lock();

#if 0 // Enable for very detailed logging of searching for the NOTIFY info.
   if (OsSysLog::willLog(FAC_SIP, PRI_DEBUG))
   {
      UtlHashBagIterator iterator(mSubscriptionStateResourceIndex);
      UtlString* contentTypeIndex;
      while ((contentTypeIndex = dynamic_cast <SubscriptionServerStateIndex*> (iterator())))
      {
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "SipSubscriptionMgr::createNotifiesDialogInfo element '%s'",
                       contentTypeIndex->data());
      }
   }
#endif // 0

   // Select the desired subset of the subscriptions, or all of them.
   UtlHashBagIterator iterator(mSubscriptionStateResourceIndex,
                               &contentKey);
   int count = 0;
   int index = 0;
   acceptHeaderValuesArray = NULL;
   notifyArray = NULL;

   while (iterator())
   {
      count++;
   }

   SubscriptionServerStateIndex* subscriptionIndex = NULL;
   acceptHeaderValuesArray = new UtlString*[count];
   notifyArray = new SipMessage*[count];
   iterator.reset();
   long now = OsDateTime::getSecsSinceEpoch();

   while ((subscriptionIndex =
           dynamic_cast <SubscriptionServerStateIndex*> (iterator())))
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipSubscriptionMgr::createNotifiesDialogInfo now %ld, mExpirationDate %ld",
                    now, subscriptionIndex->mpState->mExpirationDate);

      // Should not happen, the container is supposed to be locked
      if (index >= count)
      {
         OsSysLog::add(FAC_SIP, PRI_ERR,
                       "SipSubscriptionMgr::createNotifiesDialogInfo iterator elements count changed from: %d to %d while locked",
                       count, index);
      }
      // Should not happen, the index should be created and
      // deleted with the state
      else if (subscriptionIndex->mpState == NULL)
      {
         OsSysLog::add(FAC_SIP, PRI_ERR,
                       "SipSubscriptionMgr::createNotifiesDialogInfo SubscriptionServerStateIndex with NULL mpState");
      }

      // If not expired yet
      else if (subscriptionIndex->mpState->mExpirationDate >= now)
      {
         // Get the accept value.
         acceptHeaderValuesArray[index] =
            new UtlString(subscriptionIndex->mpState->mAcceptHeaderValue);
         // Create the NOTIFY message.
         notifyArray[index] = new SipMessage;
         mDialogMgr.setNextLocalTransactionInfo(*(notifyArray[index]),
                                                SIP_NOTIFY_METHOD,
                                                // This is a SubscriptionServerState,
                                                // whose superclass UtlString contains
                                                // the dialog handle for the subscription.
                                                static_cast <const UtlString> (*(subscriptionIndex->mpState)));

         // Set the event header, if we know what it is.
         UtlString eventHeader;
         if(subscriptionIndex->mpState->mpLastSubscribeRequest)
         {
            subscriptionIndex->mpState->mpLastSubscribeRequest->getEventField(eventHeader);
         }
         notifyArray[index]->setEventField(eventHeader);

         // Set the Subscription-State header.
         char buffer[101];
         sprintf(buffer, subscriptionStateFormat,
                 (long) (subscriptionIndex->mpState->mExpirationDate - now));
         notifyArray[index]->setHeaderValue(SIP_SUBSCRIPTION_STATE_FIELD,
                                            buffer, 0);
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "SipSubscriptionMgr::createNotifiesDialogInfo index %d, mAcceptHeaderValue '%s', getEventField '%s'",
                       index, acceptHeaderValuesArray[index]->data(),
                       eventHeader.data());

         if (OsSysLog::willLog(FAC_SIP, PRI_DEBUG))
         {
            UtlString s;
            ssize_t i;
            notifyArray[index]->getBytes(&s, &i);
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "SipSubscriptionMgr::createNotifiesDialogInfo notifyArray[%d] = '%s'",
                          index, s.data());
         }

         index++;
      }
   }

   unlock();

   numNotifiesCreated = index;

   return;
}

void SipSubscriptionMgr::createNotifiesDialogInfoEvent(const UtlString& eventType,
                                                       const char* subscriptionStateFormat,
                                                       int& numNotifiesCreated,
                                                       UtlString**& acceptHeaderValuesArray,
                                                       SipMessage**& notifyArray,
                                                       UtlString**& resourceIdArray,
                                                       UtlString**& eventTypeKeyArray)
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipSubscriptionMgr::createNotifiesDialogInfoEvent mSubscriptionStateResourceIndex (%zu entries)",
                 mSubscriptionStateResourceIndex.entries());

   lock();

#if 0 // Enable for very detailed logging of searching for the NOTIFY info.
   if (OsSysLog::willLog(FAC_SIP, PRI_DEBUG))
   {
      UtlHashBagIterator iterator(mSubscriptionStateResourceIndex);
      UtlString* contentTypeIndex;
      while ((contentTypeIndex = dynamic_cast <SubscriptionServerStateIndex*> (iterator())))
      {
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "SipSubscriptionMgr::createNotifiesDialogInfoEvent element '%s'",
                       contentTypeIndex->data());
      }
   }
#endif // 0

   // Select the desired subset of the subscriptions, or all of them.
   UtlHashBagIterator iterator(mSubscriptionStatesByDialogHandle);
   int count = 0;
   int index = 0;
   acceptHeaderValuesArray = NULL;
   notifyArray = NULL;
   resourceIdArray = NULL;
   eventTypeKeyArray = NULL;

   SubscriptionServerState* subscription = NULL;
   while ((subscription =
           dynamic_cast <SubscriptionServerState*> (iterator())))
   {
      // Select subscriptions with the right eventType.
      if (subscription->mEventType.compareTo(eventType) == 0)
      {
         count++;
      }
   }

   acceptHeaderValuesArray = new UtlString*[count];
   notifyArray = new SipMessage*[count];
   resourceIdArray = new UtlString*[count];
   eventTypeKeyArray = new UtlString*[count];

   iterator.reset();
   long now = OsDateTime::getSecsSinceEpoch();

   while ((subscription =
           dynamic_cast <SubscriptionServerState*> (iterator())))
   {
      // Select subscriptions with the right eventType.
      if (subscription->mEventType.compareTo(eventType) == 0)
      {
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "SipSubscriptionMgr::createNotifiesDialogInfoEvent now %ld, mExpirationDate %ld",
                       now, subscription->mExpirationDate);
         
         // Should not happen, the container is supposed to be locked
         if (index >= count)
         {
            OsSysLog::add(FAC_SIP, PRI_ERR,
                          "SipSubscriptionMgr::createNotifiesDialogInfoEvent iterator elements count changed from: %d to %d while locked",
                          count, index);
         }
         // If not expired yet
         else if (subscription->mExpirationDate >= now)
         {
            // Get the accept value.
            acceptHeaderValuesArray[index] =
               new UtlString(subscription->mAcceptHeaderValue);
            // Create the NOTIFY message.
            notifyArray[index] = new SipMessage;
            mDialogMgr.setNextLocalTransactionInfo(*(notifyArray[index]),
                                                   SIP_NOTIFY_METHOD,
                                                   // This is a SubscriptionServerState,
                                                   // whose superclass UtlString contains
                                                   // the dialog handle for the subscription.
                                                   static_cast <const UtlString> (*(subscription)));
            
            // Set the event header, if we know what it is.
            UtlString eventHeader;
            if (subscription->mpLastSubscribeRequest)
            {
               subscription->mpLastSubscribeRequest->getEventField(eventHeader);
            }
            notifyArray[index]->setEventField(eventHeader);
            
            // Set the Subscription-State header.
            char buffer[101];
            sprintf(buffer, subscriptionStateFormat,
                    (long) (subscription->mExpirationDate - now));
            notifyArray[index]->setHeaderValue(SIP_SUBSCRIPTION_STATE_FIELD,
                                               buffer, 0);
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "SipSubscriptionMgr::createNotifiesDialogInfoEvent index %d, mAcceptHeaderValue '%s', getEventField '%s'",
                          index, acceptHeaderValuesArray[index]->data(),
                          eventHeader.data());
            
            // Set the resourceId, eventTypeKey, and eventType.
            resourceIdArray[index] = new UtlString(subscription->mResourceId);
            eventTypeKeyArray[index] = new UtlString(subscription->mEventTypeKey);

            if (OsSysLog::willLog(FAC_SIP, PRI_DEBUG))
            {
               UtlString s;
               ssize_t i;
               notifyArray[index]->getBytes(&s, &i);
               OsSysLog::add(FAC_SIP, PRI_DEBUG,
                             "SipSubscriptionMgr::createNotifiesDialogInfoEvent notifyArray[%d] = '%s'",
                             index, s.data());
            }
            
            index++;
         }
      }
   }

   unlock();

   numNotifiesCreated = index;

   return;
}

void SipSubscriptionMgr::freeNotifies(int numNotifies,
                                      UtlString** acceptHeaderValues,
                                      SipMessage** notifiesArray)
{
   for (int index = 0; index < numNotifies; index++)
   {
      if (acceptHeaderValues[index])
      {
         delete acceptHeaderValues[index];
      }
      if (notifiesArray[index])
      {
         delete notifiesArray[index];
      }
   }
   delete[] acceptHeaderValues;
   delete[] notifiesArray;
}

UtlBoolean SipSubscriptionMgr::endSubscription(const UtlString& dialogHandle,
                                               enum subscriptionChange change)
{
    UtlBoolean subscriptionFound = FALSE;

    lock();
    SubscriptionServerState* state =
       dynamic_cast <SubscriptionServerState*>
       (mSubscriptionStatesByDialogHandle.find(&dialogHandle));
    
    if (state)
    {
        SubscriptionServerStateIndex* stateIndex = NULL;
        UtlString contentKey(state->mResourceId);
        contentKey.append(CONTENT_KEY_SEPARATOR);
        contentKey.append(state->mEventTypeKey);
        UtlHashBagIterator iterator(mSubscriptionStateResourceIndex, &contentKey);
        while((stateIndex =
               dynamic_cast <SubscriptionServerStateIndex*> (iterator())))
        {
            if(stateIndex->mpState == state)
            {
                mSubscriptionStatesByDialogHandle.removeReference(state);
                mSubscriptionStateResourceIndex.removeReference(stateIndex);
                if (OsSysLog::willLog(FAC_SIP, PRI_DEBUG))
                {
                    UtlString requestContact;
                    state->mpLastSubscribeRequest->getContactField(0, requestContact);
                    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                                 "SipSubscriptionMgr::endSubscription Delete subscription for dialog handle '%s', key '%s', contact '%s', mExpirationDate %ld",
                                 state->data(), stateIndex->data(),
                                 requestContact.data(), state->mExpirationDate);
                }

                delete state;
                delete stateIndex;
                subscriptionFound = TRUE;

                break;
            }
        }

        // Could not find the state index that corresponds to the state
        // Should not happen, there should always be one of each
        if (!subscriptionFound)
        {
            OsSysLog::add(FAC_SIP, PRI_ERR,
                "SipSubscriptionMgr::endSubscription Could not find subscription in mSubscriptionStateResourceIndex for content key '%s', dialog handle '%s'",
                          contentKey.data(),
                          dialogHandle.data());
        }
    }
    else
    {
       OsSysLog::add(FAC_SIP, PRI_ERR,
                     "SipSubscriptionMgr::endSubscription Could not find subscription in mSubscriptionStatesByDialogHandle for dialog handle '%s'",
                     dialogHandle.data());
    }

    unlock();

    // Remove the dialog
    mDialogMgr.deleteDialog(dialogHandle);

    return(subscriptionFound);
}

void SipSubscriptionMgr::removeOldSubscriptions(long oldEpochTimeSeconds)
{
    lock();
    UtlHashBagIterator iterator(mSubscriptionStateResourceIndex);
    SubscriptionServerStateIndex* stateIndex = NULL;
    while((stateIndex =
           dynamic_cast <SubscriptionServerStateIndex*> (iterator())))
    {
        if(stateIndex->mpState)
        {
            if(stateIndex->mpState->mExpirationDate < oldEpochTimeSeconds)
            {
                if (OsSysLog::willLog(FAC_SIP, PRI_DEBUG))
	        {
		   UtlString requestContact;
		   stateIndex->mpState->mpLastSubscribeRequest->
		      getContactField(0, requestContact);
		   OsSysLog::add(FAC_SIP, PRI_DEBUG,
				 "SipSubscriptionMgr::removeOldSubscriptions delete subscription for dialog handle '%s', key '%s', contact '%s', mExpirationDate %ld",
                                 stateIndex->mpState->data(),
				 stateIndex->data(), requestContact.data(),
				 stateIndex->mpState->mExpirationDate);
                }
                mDialogMgr.deleteDialog(*(stateIndex->mpState));
                mSubscriptionStatesByDialogHandle.removeReference(stateIndex->mpState);
                delete stateIndex->mpState;
                stateIndex->mpState = NULL;
                mSubscriptionStateResourceIndex.removeReference(stateIndex);
                delete stateIndex;
            }
        }
        else
        {
            OsSysLog::add(FAC_SIP, PRI_ERR,
                "SipSubscriptionMgr::removeOldSubscriptions SubscriptionServerStateIndex with NULL mpState, deleting");
            mSubscriptionStateResourceIndex.removeReference(stateIndex);
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "SipSubscriptionMgr::removeOldSubscriptions delete subscription for key '%s'",
                          stateIndex->data());
            delete stateIndex;
        }
    }

    unlock();
}

// Set stored value for the next NOTIFY CSeq.
void SipSubscriptionMgr::setNextNotifyCSeq(
   const UtlString& dialogHandleString,
   int nextLocalCseq,
   int version)
{
   // version is not stored.
   mDialogMgr.setNextLocalCseq(dialogHandleString, nextLocalCseq);
}

// Store the NOTIFY cseq now in notifyRequest and the specified version.
void SipSubscriptionMgr::updateVersion(SipMessage& notifyRequest,
                                       int version,
                                       const UtlString& eventTypeKey)
{
   // Does nothing.
}

// Update the NOTIFY message content by calling the application's
// substitution callback function.
void SipSubscriptionMgr::updateNotifyVersion(SipContentVersionCallback setContentInfo,
                                             SipMessage& notifyRequest,
                                             int& version,
                                             UtlString& eventTypeKey)
{
   UtlString dialogHandle;
   notifyRequest.getDialogHandleReverse(dialogHandle);

   // Initialize the 'last version number' to 0 and the 'found
   // subscription state' to NULL.
   version = 0;
   SubscriptionServerState* state = NULL;

   // Try to find the subscription state for the dialog, otherwise use
   // the default values set above.
   if (!dialogHandle.isNull())
   {
      state =
         dynamic_cast <SubscriptionServerState*>
         (mSubscriptionStatesByDialogHandle.find(&dialogHandle));

      if (state != NULL)
      {
         // Increment the saved "last XML version number".
         // Keep that value for insertion into the XML.
         version = ++state->mDialogVer;
         eventTypeKey = state->mEventTypeKey;

         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "SipSubscriptionMgr::updateNotifyVersion "
                       "dialogHandle = '%s', new mDialogVer = %d, eventTypeKey = '%s'",
                       dialogHandle.data(), state->mDialogVer, eventTypeKey.data());
      }
      else
      {
         OsSysLog::add(FAC_SIP, PRI_ERR, "SipSubscriptionMgr::updateNotifyVersion Unable to find dialog state for handle '%s'",
                       dialogHandle.data());
      }
   }

   // Call the application "string variable replacement" callback routine.
   if (setContentInfo != NULL)
   {
      setContentInfo(notifyRequest, version);
   }
}

// Set the minimum, default, and maximum subscription times that will be granted.
UtlBoolean SipSubscriptionMgr::setSubscriptionTimes(int minExpiration,
                                                    int defaultExpiration,
                                                    int maxExpiration)
{
   UtlBoolean ret = FALSE;

   // Check that the arguments are properly ordered.
   if (!(minExpiration <= defaultExpiration &&
         defaultExpiration <= maxExpiration))
   {
        OsSysLog::add(FAC_SIP, PRI_WARNING,
                      "Arguments to SipSubscriptionMgr::setSubscriptionTimes not in order: minExpiration = %d, defaultExpiration = %d, maxExpiration = %d",
                      minExpiration, defaultExpiration, maxExpiration);
   }
   // Check that the arguments are inside the allowed bounds.
   else if (!(LIMIT_MIN_EXPIRATION <= minExpiration &&
              maxExpiration <= LIMIT_MAX_EXPIRATION))
   {
        OsSysLog::add(FAC_SIP, PRI_WARNING,
                      "Arguments to SipSubscriptionMgr::setSubscriptionTimes not within allowed range: minExpiration = %d, maxExpiration = %d, allowed range = [%d, %d]",
                      minExpiration, maxExpiration,
                      LIMIT_MIN_EXPIRATION, LIMIT_MAX_EXPIRATION);
   }
   // Acceptable arguments.
   else
   {
      mMinExpiration = minExpiration;
      mDefaultExpiration = defaultExpiration;
      mMaxExpiration = maxExpiration;
      ret = TRUE;
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipSubscriptionMgr::setSubscriptionTimes set mMinExpiration = %d, mDefaultExpiration = %d, mMaxExpiration = %d",
                    mMinExpiration, mDefaultExpiration, mMaxExpiration);
   }

   return ret;
}

/* ============================ ACCESSORS ================================= */

SipDialogMgr* SipSubscriptionMgr::getDialogMgr()
{
    return &mDialogMgr;
}

// Get the minimum, default, and maximum subscription times that will be granted.
void SipSubscriptionMgr::getSubscriptionTimes(int& minExpiration,
                                              int& defaultExpiration,
                                              int& maxExpiration)
{
   minExpiration = mMinExpiration;
   defaultExpiration = mDefaultExpiration;
   maxExpiration = mMaxExpiration;
}

/* ============================ INQUIRY =================================== */

UtlBoolean SipSubscriptionMgr::dialogExists(UtlString& dialogHandle)
{
    UtlBoolean subscriptionFound = FALSE;

    lock();
    SubscriptionServerState* state = (SubscriptionServerState*)
        mSubscriptionStatesByDialogHandle.find(&dialogHandle);
    if(state)
    {
        subscriptionFound = TRUE;
    }
    unlock();

    return(subscriptionFound);
}

UtlBoolean SipSubscriptionMgr::isExpired(UtlString& dialogHandle)
{
    UtlBoolean subscriptionExpired = TRUE;

    lock();
    SubscriptionServerState* state = (SubscriptionServerState*)
        mSubscriptionStatesByDialogHandle.find(&dialogHandle);
    if(state)
    {
        long now = OsDateTime::getSecsSinceEpoch();

        if(now <= state->mExpirationDate)
        {
            subscriptionExpired = FALSE;
        }
    }
    unlock();

    return(subscriptionExpired);
}

/** get the next notify body "version" value that is allowed
 *  for a resource (as far as is known by this SipSubscriptionMgr).
 *  If no information is available, returns 0.
 */
int SipSubscriptionMgr::getNextAllowedVersion(const UtlString& resourceId)
{
   // The non-persistent SipSubscriptionMgr retains no information
   // about version numbers.
   // :TODO: It probably ought to, though.
   return 0;
}

// Dump the object's internal state.
void SipSubscriptionMgr::dumpState()
{
   lock();

   // indented 4

   OsSysLog::add(FAC_RLS, PRI_INFO,
                 "\t    SipSubscriptionMgr %p",
                 this);

   UtlHashBagIterator itor(mSubscriptionStatesByDialogHandle);
   SubscriptionServerState* ss;
   while ((ss = dynamic_cast <SubscriptionServerState*> (itor())))
   {
      ss->dumpState();
   }

   unlock();
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */


void SipSubscriptionMgr::lock()
{
    mSubscriptionMgrMutex.acquire();
}

void SipSubscriptionMgr::unlock()
{
    mSubscriptionMgrMutex.release();
}

/* ============================ FUNCTIONS ================================= */
