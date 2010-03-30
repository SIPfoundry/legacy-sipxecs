//
// Copyright (C) 2010 Avaya Inc., certain elements licensed under a Contributor Agreement.
// Copyright (C) 2009 Nortel Networks, certain elements licensed under a Contributor Agreement.
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <net/SipDialog.h>
#include <net/SipDialogMgr.h>
#include <net/SipRefreshManager.h>
#include <net/SipUserAgent.h>
#include <os/OsDateTime.h>
#include <os/OsEventMsg.h>
#include <os/OsLock.h>
#include <os/OsMsgQ.h>
#include <os/OsTimer.h>
#include <os/OsUnLock.h>
#include <utl/UtlHashBagIterator.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// msgSubType for an OsMsg that requests calling a callback routine
#define SIP_CALLBACK     (OsEventMsg::USER_START)
// msgSubType for an OsMsg for the firing of a refresh timer
#define EVENT_REFRESH    (OsEventMsg::USER_START + 1)

// STATIC VARIABLE INITIALIZATIONS

#define TRANSIENT_ERROR_LIMIT 10
// Resend time in seconds after first transient error.
#define TRANSIENT_RESEND_TIME_BASE 1

/// The maximum number of consecutive transient failures that will be tolerated.
// A further transient failure will be treated as a non-transient failure.
static const int sTransientErrorLimit = TRANSIENT_ERROR_LIMIT;

/** The resend delays to use after the corresponding number of consecutive
 *  transient failures.
 */
static int sTransientResendDelay[TRANSIENT_ERROR_LIMIT + 1] ={
   0,                           // element 0 is never used
   TRANSIENT_RESEND_TIME_BASE,  // 1 second, as defined above
   TRANSIENT_RESEND_TIME_BASE * 2,
   TRANSIENT_RESEND_TIME_BASE * 4,
   TRANSIENT_RESEND_TIME_BASE * 8,
   TRANSIENT_RESEND_TIME_BASE * 16,
   TRANSIENT_RESEND_TIME_BASE * 32,
   TRANSIENT_RESEND_TIME_BASE * 60, // adjusted to 1 minute from 64 seconds
   TRANSIENT_RESEND_TIME_BASE * 120,
   TRANSIENT_RESEND_TIME_BASE * 240,
   TRANSIENT_RESEND_TIME_BASE * 300, // adjusted to 5 minutes from 8 minutes
};

// LOCAL FUNCTIONS

//! Calculate the time in seconds from now when a refresh should occur.
/** Time is calculated based on the expiration provided in the response.
 *  Time is corrected based on when the request was sent.
 */
static int calculateResendTime(int providedExpiration,
                               ///< Expiration provided in response.
                               int minimumRefresh
                               ///< Minimum refresh time that will be returned.
   );


/// OsMsg subclass for the firing of mRefreshTimer - A refresh should be sent.
class RefreshEventMsg : public OsMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   /* ============================ CREATORS ================================== */

   RefreshEventMsg(const UtlString& handle /**< handle of the RefreshDialogState
                                             *   for which to send the refresh */
      ) :
      OsMsg(OS_EVENT, EVENT_REFRESH),
      mHandle(handle)
      {
      }

   virtual
   ~RefreshEventMsg()
      {
      }
   //:Destructor

   /* ============================ MANIPULATORS ============================== */

   virtual OsMsg* createCopy(void) const
      {
         return new RefreshEventMsg(mHandle);
      };
   //:Create a copy of this msg object (which may be of a derived type)

   /* ============================ ACCESSORS ================================= */

   // Get pointer to the handle value.
   UtlString* getHandle()
      {
         return &mHandle;
      }

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   RefreshEventMsg(const RefreshEventMsg& rRefreshEventMsg);
   //:Copy constructor (not implemented for this class)

   RefreshEventMsg& operator=(const RefreshEventMsg& rhs);
   //:Assignment operator (not implemented for this class)

   /// Handle of the RefreshDialogState for which to send the refresh.
   UtlString mHandle;

};


// Private class to contain refresh state.
// UtlString is the dialog handle of the dialog or pseudo-dialog that
// is maintaining the refresh.
class RefreshDialogState : public UtlString
{
public:

   RefreshDialogState(const UtlString& handle,
                      ///< the dialog handle for this refresh
                      SipRefreshManager* pRefMgr
                      ///< the governing SipRefreshManager
      );

   virtual ~RefreshDialogState();

   // UtlString::data contains the dialog handle.

   /// Clean the state and attached request so that the request can be resent.
   void resetStateForResend(UtlBoolean expireNow
                            ///< If true, set requested expiration to 0.
      );

   //! Set the state's refresh timer to fire when it is time to resend.
   //  If isSuccessful is true, the expiration and specified
   //  minimum are used to determine the time after mPendingStartTime
   //  when refresh should be done: mRefreshTimer is set for that time,
   //  and mExpiration is set for when the granted time expires.
   //  If isSuccessful is false, the expiration argument is taken as
   //  when to retry (in seconds from now), and mRefreshTimer is set for
   //  that time.  (mExpiration is not updated, as no new grant has been given).
   void setRefreshTimer(UtlBoolean isSuccessful,
                        ///< Determines if this is success or failure processing.
                        int expiration,
                        ///< The granted expiration period.
                        int minimumRefresh
                        ///< Minimum refresh that will be used.
      );

   //! Dump contents of RefreshDialogState as a string.
   void toString(UtlString& dumpString);

   /// Application data provided to ::initiateRefresh.
   void* mpApplicationData;
   /// Callback function for changes in subscription state.
   SipRefreshManager::RefreshStateCallback mpStateCallback;
   /// The expiration time to request.
   int mExpirationPeriodSeconds;
   /// When the last message was sent.
   long mPendingStartTime;
   /// When the subscription/registration will expire.
   //  Values -1, 0, and less than current time have special meanings.
   //  See SipRefreshManager.h.
   long mExpiration;
   /// The last request sent for this refresh.
   SipMessage* mpLastRequest;
   /// The current state of this refresh.
   SipRefreshManager::RefreshRequestState mRequestState;
   /// Fires when it is time to refresh.
   OsTimer mRefreshTimer;
   /** The number of consecutive 'transient' failure responses that have been
    *  received for resends.
    */
   int mTransientErrorCount;

   //! Dump the object's internal state.
   void dumpState();

private:
   //! DISALLOWED accidental copying
   RefreshDialogState(const RefreshDialogState& rRefreshDialogState);
   RefreshDialogState& operator=(const RefreshDialogState& rhs);
};

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

RefreshDialogState::RefreshDialogState(const UtlString& handle,
                                       SipRefreshManager* pRefMgr) :
   UtlString(handle),
   mpApplicationData(NULL),
   mpStateCallback(NULL),
   mExpirationPeriodSeconds(-1),
   mPendingStartTime(-1),
   mExpiration(-1),
   mpLastRequest(NULL),
   mRequestState(SipRefreshManager::REFRESH_REQUEST_UNKNOWN),
   mRefreshTimer(new RefreshEventMsg(static_cast <const UtlString> (*this)),
                 pRefMgr->getMessageQueue())
{
}

// Copy constructor NOT ALLOWED
// RefreshDialogState::RefreshDialogState(const RefreshDialogState& rRefreshDialogState)

// Assignment operator NOT ALLOWED
// RefreshDialogState&
// RefreshDialogState::operator=(const RefreshDialogState& rhs)

RefreshDialogState::~RefreshDialogState()
{
    if (mpLastRequest)
    {
       delete mpLastRequest;
       mpLastRequest = NULL;
    }

    mRefreshTimer.stop(TRUE);
}

// Clean the state and attached request so that the request can be resent.
void RefreshDialogState::resetStateForResend(UtlBoolean expireNow)
{
   // Remove the Via that was added when the request was last sent.
   mpLastRequest->removeTopVia();

   // Do not modify the Route headers in *mpLastRequest.
   // If it is a REGISTER, they are what was specified to ::initiateRefresh
   // (and should be maintained, as successive REGISTERS are a quasi-dialog
   // and continue to be routed as the first message was).
   // If it is a SUBSCRIBE, the Route headers were revised based on the
   // Record-Route's in the 2xx response that created the dialog.

   // Remove any credentials
   while (mpLastRequest->removeHeader(HTTP_AUTHORIZATION_FIELD, 0))
   {
      // null
   }
   while (mpLastRequest->removeHeader(HTTP_PROXY_AUTHORIZATION_FIELD, 0))
   {
      // null
   }

   // Remove transport state info
   mpLastRequest->resetTransport();

   // Set the expiration to 0 if we are to terminate the registration/subscription.
   if (expireNow)
   {
      mpLastRequest->setExpiresField(0);
   }

   // The Date header will be set by the SipUserAgent.

   if (OsSysLog::willLog(FAC_SIP, PRI_DEBUG))
   {
      UtlString lastRequest;
      ssize_t length;
      mpLastRequest->getBytes(&lastRequest, &length);
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
		    "RefreshDialogState::resetStateForResend last request set to '%s'",
		    lastRequest.data());
   }
}

void RefreshDialogState::setRefreshTimer(UtlBoolean isSuccessful,
                                         int expiration,
                                         int minimumRefresh)
{
   int nextResendSeconds;

   if (isSuccessful)
   {
      // 'expiration' is the granted time from a success response.

      // Calculate the time from mPendingStartTime when we want to refresh.
      nextResendSeconds = calculateResendTime(expiration, minimumRefresh);

      // Correct the time to be based on now.
      nextResendSeconds -=
         OsDateTime::getSecsSinceEpoch() - mPendingStartTime;

      // Set the state's recorded expiration time.
      mExpiration = mPendingStartTime + expiration;
   }
   else
   {
      // 'expiration' is the time (from now) to set into the timer.
      nextResendSeconds = expiration;
   }

   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "RefreshDialogState::setRefreshTimer setting resend timeout in %d seconds",
                 nextResendSeconds);

   OsTime timerTime(nextResendSeconds, 0);
   mRefreshTimer.oneshotAfter(timerTime);
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
}


// Private OsMsg type to carry requests to call application callback function.
class CallbackRequestMsg : public OsMsg
{
public:

   typedef enum {
      CALLBACK = 0,
      CALLBACK_AND_DELETE,
      DELETE
   } Action;
   //!enumcode: CALLBACK - call the callback function
   //!enumcode: CALLBACK_AND_DELETE - call the callback function, then delete *state
   //!enumcode: DELETE - delete *state

   // Constructor
   CallbackRequestMsg(RefreshDialogState* state, ///< the state object
                      Action action,             ///< the action to take
                      const SipMessage* message, /**< pointer to the message involved
                                                  *   or NULL.
                                                  *   (owned by CallbackRequestMsg).
                                                  */
                      const UtlString& earlyDialogHandle,
                                                 ///< early dialog handle to report
                      const UtlString& dialogHandle
                                                 ///< dialog handle to report
      );

   // Destructor
   virtual ~CallbackRequestMsg();

   /// Do the callback described by this object, if 'action' specifies it.
   void callback(SipDialogMgr& dialogMgr
                 ///< use to look up early dialog handles
      );

private:
   //! DISALLOWED accidental copying
   CallbackRequestMsg(const CallbackRequestMsg& rCallbackRequestMsg);
   CallbackRequestMsg& operator=(const CallbackRequestMsg& rhs);

   //! DISALLOWED intentional copying
   //  (Will abort at run-time.)
   virtual OsMsg* createCopy(void) const;

   /// Pointer to the relevant RefreshDialogState.
   //  If mDelete is true, *mpState is owned by this object.
   //  *mpState contains most of the information needed to call the callback.
   RefreshDialogState* mpState;
   /// The action to be performed.
   //  (Deletion is done by our destructor.)
   Action mAction;
   /// The response message involved.
   //  Owned by this object.
   const SipMessage* mpMessage;
   /// Early dialog handle to report
   UtlString mEarlyDialogHandle;
   /// Dialog handle to report
   UtlString mDialogHandle;
};

// Constructor
CallbackRequestMsg::CallbackRequestMsg(RefreshDialogState* state,
                                       Action action,
                                       const SipMessage* message,
                                       const UtlString& earlyDialogHandle,
                                       const UtlString& dialogHandle) :
   OsMsg(OsMsg::OS_EVENT, SIP_CALLBACK),
   mpState(state),
   mAction(action),
   mpMessage(message),
   mEarlyDialogHandle(earlyDialogHandle),
   mDialogHandle(dialogHandle)
{
}

CallbackRequestMsg::~CallbackRequestMsg()
{
   // Delete *state if we are supposed to delete it.
   if (mAction == CALLBACK_AND_DELETE || mAction == DELETE)
   {
      delete mpState;
   }
   // Delete *message if it exists.
   if (mpMessage)
   {
      delete mpMessage;
   }
}

// Do the callback described by this object, if specified by mAction.
void CallbackRequestMsg::callback(SipDialogMgr& dialogMgr)
{
   if (mAction == CALLBACK || mAction == CALLBACK_AND_DELETE)
   {
      UtlString responseText;
      int responseCode = -1;

      // If there is a message, get the response code and text.
      if (mpMessage)
      {
         mpMessage->getResponseStatusText(&responseText);
         responseCode = mpMessage->getResponseStatusCode();
      }

      if (OsSysLog::willLog(FAC_SIP, PRI_DEBUG))
      {
         UtlString m;
         ssize_t l;
         if (mpMessage)
         {
            mpMessage->getBytes(&m, &l);
         }
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "CallbackRequestMsg::callback "
                       "mpState = %p, mpState->mRequestState = %s, mEarlyDialogHandle = '%s', mDialogHandle = '%s', mpState->mpApplicationData = %p, mpState->mExpiration = %ld, mpMessage = '%s'",
                       mpState,
                       SipRefreshManager::refreshRequestStateText(mpState->mRequestState),
                       mEarlyDialogHandle.data(),
                       mDialogHandle.data(),
                       mpState->mpApplicationData,
                       mpState->mExpiration,
                       m.data());
      }
      // the callback may need the earlyDialogHandle
      if (mEarlyDialogHandle.isNull())
      {
         dialogMgr.getEarlyDialogHandleFor(mDialogHandle,
                                           mEarlyDialogHandle);
      }
      mpState->mpStateCallback(mpState->mRequestState,
                               mEarlyDialogHandle.data(),
                               mDialogHandle.data(),
                               mpState->mpApplicationData,
                               mpMessage ? responseCode : -1,
                               mpMessage ? responseText.data() : NULL,
                               mpState->mExpiration,
                               mpMessage);
   }
}

// Dynamic copy routine (unused)
OsMsg* CallbackRequestMsg::createCopy(void) const
{
   assert(FALSE);
   return NULL;
}


// Constructor
SipRefreshManager::SipRefreshManager(SipUserAgent& userAgent,
                                     SipDialogMgr& dialogMgr)
    : OsServerTask("SipRefreshManager-%d")
    , mpUserAgent(&userAgent)
    , mpDialogMgr(&dialogMgr)
    , mDefaultExpiration(3600)
    , mSemaphore(OsBSem::Q_FIFO, OsBSem::FULL)
    , mReceivingRegisterResponses(FALSE)
    , mReceivingSubscribeResponses(FALSE)
{
}

// Copy constructor NOT ALLOWED
// SipRefreshManager::SipRefreshManager(const SipRefreshManager& rSipRefreshManager)

// Destructor
SipRefreshManager::~SipRefreshManager()
{
    // Do not delete *mpUserAgent or *mpDialogMgr.  They are not owned
    // by this SipRefreshManager and may be used elsewhere.

    // Stop receiving REGISTER/SUBSCRIBE responses.
    mpUserAgent->removeMessageObserver(*(getMessageQueue()));

    // Unsubscribe to anything that is in the list
    stopAllRefreshes();
    // mRefreshes should now be empty

    // Wait until this OsServerTask has stopped or handleMethod
    // might access something we are about to delete here.
    waitUntilShutDown();
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator NOT ALLOWED
// SipRefreshManager&
// SipRefreshManager::operator=(const SipRefreshManager& rhs)

UtlBoolean SipRefreshManager::initiateRefresh(SipMessage* subscribeOrRegisterRequest,
                                              void* applicationData,
                                              const RefreshStateCallback refreshStateCallback,
                                              UtlString& earlyDialogHandle,
                                              UtlBoolean suppressFirstSend)
{
   RefreshDialogState* state = NULL;

   if (OsSysLog::willLog(FAC_SIP, PRI_DEBUG))
   {
      UtlString b;
      ssize_t l;
      subscribeOrRegisterRequest->getBytes(&b, &l);
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipRefreshManager::initiateRefresh  "
                    "refreshStateCallback = %p, suppressFirstSend = %d, subscribeOrRegisterRequest = '%s'",
                    refreshStateCallback, suppressFirstSend, b.data());
   }

   // Set to true if attempt to send subscribeOrRegisterRequest failed.
   // (Since initially we haven't attempted to send it, the initial
   // value is false.)
   UtlBoolean initialRequestSendFailed = FALSE;

   // Make sure we do not have an existing dialog or refresh session state
   // going for the given message
   subscribeOrRegisterRequest->getDialogHandle(earlyDialogHandle);
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipRefreshManager::initiateRefresh earlyDialogHandle = '%s'",
                 earlyDialogHandle.data());

   UtlBoolean existingRefreshState = FALSE;
   UtlBoolean existingDialogState = FALSE;

   {
      // Seize lock.
      OsLockUnlockable lock(mSemaphore);

      // Check for state for this handle in this SipRefreshManager.
      RefreshDialogState* s = getAnyDialog(earlyDialogHandle);
      if (s)
      {
         existingRefreshState = TRUE;
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "SipRefreshManager::initiateRefresh called with dialog handle '%s' that matches existing refresh state '%s'",
                       earlyDialogHandle.data(),
                       s->data());
      }
      // Check for early or established dialog state in *mpDialogMgr.
      else if (mpDialogMgr->dialogExists(earlyDialogHandle))
      {
         existingDialogState = TRUE;
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "SipRefreshManager::initiateRefresh called with dialog handle '%s' that matches existing dialog state",
                       earlyDialogHandle.data());
      }
      else if (mpDialogMgr->earlyDialogExistsFor(earlyDialogHandle))
      {
         existingDialogState = TRUE;
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "SipRefreshManager::initiateRefresh called with dialog handle '%s' that matches existing early dialog state",
                       earlyDialogHandle.data());
      }

      // Should not be any existing refresh or dialog states
      // for this message
      if (!existingRefreshState && !existingDialogState)
      {
         // Make sure we are registered to receive responses
         // for the message we are about to send
         UtlString method;
         subscribeOrRegisterRequest->getRequestMethod(&method);
         if (method.compareTo(SIP_REGISTER_METHOD) == 0)
         {
            if (!mReceivingRegisterResponses)
            {
               mReceivingRegisterResponses = TRUE;
               // Register to receive REGISTER responses.
               mpUserAgent->addMessageObserver(*(getMessageQueue()),
                                               SIP_REGISTER_METHOD,
                                               FALSE, // no requests
                                               TRUE, // yes responses
                                               TRUE, // incoming,
                                               FALSE, // outgoing
                                               NULL);
            }
         }
         else if (method.compareTo(SIP_SUBSCRIBE_METHOD) == 0)
         {
            // Check to see if we have already registered to
            // receive the event type
            if (!mReceivingSubscribeResponses)
            {
               mReceivingSubscribeResponses = TRUE;
               // Register to receive SUBSCRIBE responses.
               mpUserAgent->addMessageObserver(*(getMessageQueue()),
                                               SIP_SUBSCRIBE_METHOD,
                                               FALSE, // no requests
                                               TRUE, // yes responses
                                               TRUE, // incoming,
                                               FALSE, // outgoing
                                               NULL); // event type is not used for filtering responses
            }
         }

         // Create a new refresh state.
         int requestedExpiration;  // returned from following createNewRefreshState
         state = createNewRefreshState(*subscribeOrRegisterRequest,
                                       earlyDialogHandle,
                                       applicationData,
                                       refreshStateCallback,
                                       requestedExpiration);

         // Create a new dialog in *mpDialogMgr.
         mpDialogMgr->createDialog(*subscribeOrRegisterRequest,
                                   TRUE, // message from this side
                                   earlyDialogHandle);

         // Keep track of when we sent this request, so we can more
         // accurately calculate when it needs to be refreshed.
         state->mPendingStartTime = OsDateTime::getSecsSinceEpoch();
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "SipRefreshManager::initiateRefresh %p->mPendingStartTime = %ld",
                       state, state->mPendingStartTime);

         // Set the request state correctly.
         // If suppressFirstSend is specified, we immediately transition into
         // REFRESH_REQUEST_SUCCEEDED, as if we had received a 2xx response.
         // A registration can be terminated before the response to the initial
         // REGISTER, so it is in state REFRESH_REQUEST_SUCCEEDED.
         // But an initial SUBSCRIBE can't be terminated by sending a SUBSCRIBE
         // with "Expires: 0" until we receive a success response to the initial
         // SUBSCRIBE and thus know the to-tag.  So it is in state
         // REFRESH_REQUEST_PENDING.
         state->mRequestState =
            (suppressFirstSend ||
             method.compareTo(SIP_REGISTER_METHOD) == 0) ?
            REFRESH_REQUEST_SUCCEEDED :
            REFRESH_REQUEST_PENDING;
         // Record the request as mpLastRequest.
         // The request needs to be attached to the state before the
         // send() in case the response comes back before we return
         // from the send.
         // ::initiateRefresh() passes ownership of
         // *subscribeOrRegisterRequest to state->mpLastRequest, so
         // we do not copy the request here.
         state->mpLastRequest = subscribeOrRegisterRequest;

         // Add the state to the container of refresh states
         mRefreshes.insert(state);

         // Schedule the callback for the new state.
         UtlBoolean stateKeyIsEarlyDialog = SipDialog::isEarlyDialog(*state);
         const char *key = state->data();
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "SipRefreshManager::initiateRefresh %p->mRequestState = %d",
                       state, state->mRequestState);
         postMessageP(
            new CallbackRequestMsg(state,
                                   CallbackRequestMsg::CALLBACK,
                                   NULL,     // no response
                                   stateKeyIsEarlyDialog ? key : NULL,
                                   stateKeyIsEarlyDialog ? NULL : key
               ));

         // Send the request (unless that is suppressed).
         if (!suppressFirstSend)
         {
            // Temporarily free the lock.
            OsUnLock unlock(lock);

            // Since we are holding no locks, it doesn't matter if we block here,
            // so we can call SipUserAgent::send() directly.
            // SipUserAgent::send() does not take ownership of
            // subscribeOrRegisterRequest.
            initialRequestSendFailed =
               !mpUserAgent->send(*subscribeOrRegisterRequest);
            if (initialRequestSendFailed)
            {
               OsSysLog::add(FAC_SIP, PRI_WARNING,
                             "SipRefreshManager::initiateRefresh SipUserAgent::send() returned false");
            }

            // Note that 'state' may now be invalid, as another thread may have
            // terminated the refresh while we weren't holding the locks.
         }
         else
         {
            // Set the expiration timer as if we received a success response.
            state->setRefreshTimer(TRUE,
                                   requestedExpiration,
                                   mpUserAgent->getSipStateTransactionTimeout() / 1000
               );

            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "SipRefreshManager::initiateRefresh setting success timer due to suppressFirstSend");
         }
      }
      else
      {
         // A state for the same dialog identifiers already existed.
         initialRequestSendFailed = TRUE;
      }
   }

   // We do not set a timer here, but instead wait for the response to do so.

   return !initialRequestSendFailed;
}


UtlBoolean SipRefreshManager::stopRefresh(const char* dialogHandle,
                                          UtlBoolean noSend)
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipRefreshManager::stopRefresh dialogHandle = '%s', noSend = %d",
                 dialogHandle, noSend);

   RefreshDialogState* state;

   {
      // Seize lock.
      OsLock lock(mSemaphore);

      // Find the refresh state
      UtlString dialogHandleString(dialogHandle);
      state = getAnyDialog(dialogHandleString);

      if (state)
      {
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "SipRefreshManager::stopRefresh "
                       "state = %p, state->mRequestState = %s",
                       state,
                       refreshRequestStateText(state->mRequestState));

         // Stop the refresh timer.
         state->mRefreshTimer.stop(TRUE);

         // Remove the state from mRefreshes.
         mRefreshes.removeReference(state);

         // Do not delete the dialog from *mpDialogMgr yet, because
         // ::prepareForResend will need the dialog state to update
         // the CSeq.
      }
   }

   // We now have the only pointer to *state and we are not holding the lock.
   if (state)
   {
      // mRequestState tells whether sending a terminating request is
      // required.
      if (state->mRequestState == REFRESH_REQUEST_SUCCEEDED)
      {
         // Send a terminating request.
         if (state->mpLastRequest && !noSend)
         {
            // Reset the request for resending.
            // Set its expiration to 0.
            prepareForResend(*state,
                             TRUE); // expire now

            mpUserAgent->send(*state->mpLastRequest);
         }
      }

      // Remove the dialog from the Dialog Manager now that the dialog state
      // is no longer needed.
      mpDialogMgr->deleteDialog(*state);

      // The action we will specify in the CallbackRequestMsg.
      CallbackRequestMsg::Action action;
      // mRequestState tells whether a callback is needed to report termination.
      if (state->mRequestState != REFRESH_REQUEST_FAILED)
      {
         // Change the state.
         state->mRequestState = REFRESH_REQUEST_FAILED;

         // Set the action.
         action = CallbackRequestMsg::CALLBACK_AND_DELETE;
      }
      else
      {
         // Set the action.
         action = CallbackRequestMsg::DELETE;
      }

      // Send a message to our thread to call the refresh state callback.
      // Since we are holding the only pointer to *state (except
      // for other CallbackRequestMsg's that will be processed
      // before this one), we can safely request it to be deleted.
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipRefreshManager::stopRefresh %p->mRequestState = %d",
                    state, state->mRequestState);
      postMessageP(
         new CallbackRequestMsg(state,
                                action,  // the proper action
                                NULL,    // no message
                                // use the provided handle as both early and established
                                dialogHandle,
                                dialogHandle
            ));

      // The refresh state will be deleted by our thread when it handles
      // the message.
   }

   return state != NULL;
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
      mpLastRequest->getEventField(eventField);
      mpLastRequest->getBytes(&msg_text, &msg_length);
   }

   char refreshTimerText[100];

   OsTimer::OsTimerState state;
   OsTimer::Time expiresAt;
   UtlBoolean periodic;
   OsTimer::Interval period;

   mRefreshTimer.getFullState(state, expiresAt, periodic, period);
   sprintf(refreshTimerText,
           "%p %s/%+d/%s/%d",
           &mRefreshTimer,
           state == OsTimer::STARTED ? "STARTED" : "STOPPED",
           (int) ((expiresAt - OsTimer::now()) / 1000000),
           periodic ? "periodic" : "one-shot",
           (int) period);

   OsSysLog::add(FAC_RLS, PRI_INFO,
                 "\t      RefreshDialogState %p request-URI = '%s', Event = '%s', mExpirationPeriodSeconds = %d, mPendingStartTime = %+d, mExpiration = %+d, mRequestState = '%s', mRefreshTimer = %s, mpLastRequest = '%s'",
                 this, requestURI.data(), eventField.data(),
                 mExpirationPeriodSeconds, (int) (mPendingStartTime - now),
                 (int) (mExpiration - now),
                 SipRefreshManager::refreshRequestStateText(mRequestState),
                 refreshTimerText,
                 msg_text.data());
}

void SipRefreshManager::stopAllRefreshes()
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipRefreshManager::stopAllRefreshes entered");

   UtlHashBagIterator iterator(mRefreshes);
   UtlString key;
   UtlBoolean found;
   int count = 0;

   // Repeatedly get the key of the first state in mRefreshes
   // and terminate that refresh.
   do {
      {
         // Seize lock.
         OsLockUnlockable lock(mSemaphore);

         found = !mRefreshes.isEmpty();
         if (found)
         {
            iterator.reset();
            // Copy the key of the refresh into a local variable so that we
            // can release the lock.
            key = *static_cast <UtlString*>
               (dynamic_cast <RefreshDialogState*>
                (iterator()));
         }
      }

      if (found)
      {
         stopRefresh(key);
         count++;
      }
   } while (found);

   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipRefreshManager::stopAllRefreshes exited %d entries",
                 count);
}

UtlBoolean SipRefreshManager::changeRefreshTime(const char* earlyDialogHandle,
                                                int expirationPeriodSeconds)
{
   RefreshDialogState* state;

   {
      // Seize lock.
      OsLockUnlockable lock(mSemaphore);

      state = getAnyDialog(earlyDialogHandle);
      if (state)
      {
         {
            // Stop the timer.
            state->mRefreshTimer.stop(TRUE);

            state->mExpirationPeriodSeconds = expirationPeriodSeconds;
            state->mpLastRequest->setExpiresField(expirationPeriodSeconds);

            // Reset the message for resend
            prepareForResend(*state,
                             FALSE); // not immediate expiration

            // Keep track of when this refresh is sent so we know
            // when the new expiration is relative to.
            state->mPendingStartTime = OsDateTime::getSecsSinceEpoch();
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "SipRefreshManager::changeRefreshTime %p->mPendingStartTime = %ld",
                          state, state->mPendingStartTime);

            // Send the message.
            {
               OsUnLock unlock(lock);

               mpUserAgent->send(*(state->mpLastRequest));
               // SipUserAgent::send does not take ownership of the message.
            }

            if (OsSysLog::willLog(FAC_SIP, PRI_DEBUG))
            {
               UtlString lastRequest;
               ssize_t length;
               state->mpLastRequest->getBytes(&lastRequest, &length);
               OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipRefreshManager::changeRefreshTime last request = '%s'",
                             lastRequest.data());
            }
         }
      }
   }

   return state != NULL;
}

UtlBoolean SipRefreshManager::changeCurrentRefreshTime(const char* earlyDialogHandle,
                                                       int expirationPeriodSeconds)
{
   RefreshDialogState* state;

   {
      // Seize lock.
      OsLock lock(mSemaphore);

      state = getAnyDialog(earlyDialogHandle);
      if (state)
      {
         {
            // Stop the timer.
            state->mRefreshTimer.stop(TRUE);

            // Keep track of when this refresh is sent so we know
            // when the new expiration is relative to.
            state->mPendingStartTime = OsDateTime::getSecsSinceEpoch();
            state->setRefreshTimer(TRUE,
                                   expirationPeriodSeconds,
                                   mpUserAgent->getSipStateTransactionTimeout() / 1000
               );

            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "SipRefreshManager::changeCurrentRefreshTime %p->now + %d",
                          state,
                          (int) (state->mExpiration - state->mPendingStartTime));
         }
      }
   }

   return state != NULL;
}

UtlBoolean SipRefreshManager::handleMessage(OsMsg& eventMessage)
{
    int msgType = eventMessage.getMsgType();
    int msgSubType = eventMessage.getMsgSubType();

    // Incoming SIP message
    if (msgType == OsMsg::PHONE_APP &&
        msgSubType == SipMessage::NET_SIP_MESSAGE)
    {
       handleSipMessage(dynamic_cast <SipMessageEvent&> (eventMessage));
    }
    // Request to call a callback function.
    else if (msgType == OsMsg::OS_EVENT &&
             msgSubType == SIP_CALLBACK)
    {
       // Call the callback function.
       (dynamic_cast <CallbackRequestMsg&> (eventMessage)).callback(*mpDialogMgr);
       // Our caller deletes eventMessage, which frees everything owned by
       // that CallbackRequestMsg.
    }
    // Refresh timer fired - A refresh should be sent.
    else if (msgType == OsMsg::OS_EVENT &&
             msgSubType == EVENT_REFRESH)
    {
       // Call handleRefreshEvent on the handle.
       RefreshEventMsg* m = dynamic_cast <RefreshEventMsg*> (&eventMessage);
       assert(m != 0);
       handleRefreshEvent(*m->getHandle());
    }

    return TRUE;
}

// Handle an incoming SIP message.
void SipRefreshManager::handleSipMessage(SipMessageEvent& eventMessage)
{
   const SipMessage* sipMessage = eventMessage.getMessage();
   int messageType = eventMessage.getMessageStatus();

   // messageType can be:
   //    SipMessageEvent::TRANSPORT_ERROR for requests that do not get sent
   //            (This case is only theoretical, because in this case, the
   //            attached SipMessage is the failed request, and we have
   //            not registered with mpUserAgent to receive requests.)
   //    SipMessageEvent::AUTHENTICATION_RETRY for 401 or 407 responses
   //            that mpUserAgent has resent with credentials.
   //            When we get this message we update the dialog information
   //            (increment the recorded CSeq).
   //    SipMessageEvent::APPLICATION for normal incoming messages, that is,
   //            other responses.

   // If this is a SUBSCRIBE or REGISTER response
   UtlString method;
   int cseq;
   sipMessage->getCSeqField(&cseq, &method);

   if (OsSysLog::willLog(FAC_SIP, PRI_DEBUG))
   {
      UtlString b;
      ssize_t l;
      sipMessage->getBytes(&b, &l);
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipRefreshManager::handleSipMessage sipMessage = '%s'",
                    b.data());
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipRefreshManager::handleSipMessage sipMessage %p, cseq = %d, method = '%s', isResponse = %d",
                    sipMessage, cseq, method.data(), sipMessage->isResponse());
   }

   if (sipMessage->isResponse() &&
       (method.compareTo(SIP_SUBSCRIBE_METHOD) == 0 ||
        method.compareTo(SIP_REGISTER_METHOD) == 0))
   {
      UtlString dialogHandle;
      sipMessage->getDialogHandle(dialogHandle);
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipRefreshManager::handleSipMessage %s CSeq %d, dialogHandle = '%s', response = %d",
                    method.data(), cseq, dialogHandle.data(),
                    sipMessage->getResponseStatusCode());

      UtlBoolean foundDialog = mpDialogMgr->dialogExists(dialogHandle);
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
                    "SipRefreshManager::handleSipMessage foundDialog = %d, foundEarlyDialog = %d, matchesLastLocalTransaction = %d, earlyDialogHandle = '%s'",
                    foundDialog, foundEarlyDialog,
                    matchesLastLocalTransaction, earlyDialogHandle.data());

#ifdef TEST_PRINT
      UtlString refreshStateDump;
      dumpRefreshStates(refreshStateDump);
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipRefreshManager::handleSipMessage state dump: %s",
                    refreshStateDump.data());
#endif

      // Seize lock.
      OsLock lock(mSemaphore);

      // Find the refresh state for this response
      RefreshDialogState* state = NULL;
      if (foundDialog && matchesLastLocalTransaction)
      {
         state =
            dynamic_cast <RefreshDialogState*> (mRefreshes.find(&dialogHandle));
         // Check if the key has the tags reversed
         if (state == NULL)
         {
            UtlString reversedDialogHandle;
            SipDialog::reverseTags(dialogHandle, reversedDialogHandle);
            state =
               dynamic_cast <RefreshDialogState*>
               (mRefreshes.find(&reversedDialogHandle));
         }
      }
      else if (foundEarlyDialog && matchesLastLocalTransaction)
      {
         state =
            dynamic_cast <RefreshDialogState*> (mRefreshes.find(&earlyDialogHandle));
         // See if the key has the tags reversed
         if (state == NULL)
         {
            UtlString reversedEarlyDialogHandle;
            SipDialog::reverseTags(earlyDialogHandle, reversedEarlyDialogHandle);
            state =
               dynamic_cast <RefreshDialogState*>
               (mRefreshes.find(&reversedEarlyDialogHandle));
         }
      }

      if (state)
      {
         // Process the response based on the response code.
         int responseCode = sipMessage->getResponseStatusCode();
         UtlString responseText;
         sipMessage->getResponseStatusText(&responseText);
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "SipRefreshManager::handleSipMessage response code = %d, text = '%s'",
                       responseCode, responseText.data());

         // Provisional response, do nothing
         if (responseCode < SIP_2XX_CLASS_CODE)
         {
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "SipRefreshManager::handleSipMessage Provisional response ignored");
         }

         // This event reports that mpUserAgent resent the request with
         // credentials after receiving an authentication challenge (401 or
         // 407).
         else if (messageType == SipMessageEvent::AUTHENTICATION_RETRY)
         {
            if (matchesLastLocalTransaction)
            {
               // Update the recorded CSeq in mpDialogMgr to account
               // for the resend that was done by the SipUserAgent.
               // This is needed so that the hoped-for 200 response
               // to the resend is seen as matchesLastLocalTransaction
               // and updates the subscription information.
               cseq++;
               UtlString* handle =
                  foundDialog ? &dialogHandle : &earlyDialogHandle;
               UtlBoolean ret =
                  mpDialogMgr->setNextLocalCseq(*handle, cseq);
               OsSysLog::add(FAC_SIP, PRI_DEBUG,
                             "SipRefreshManager::handleSipMessage setNextLocalCseq('%s', %d) = %d",
                             handle->data(), cseq, ret);
            }
            else
            {
               OsSysLog::add(FAC_SIP, PRI_DEBUG,
                             "SipRefreshManager::handleSipMessage Authentication retry response not in sequence, ignored");
            }
         }

         // A success or failure response which will not be retried
         // by mUserAgent.
         // These change the state of the subscription.
         else
         {
            // Test if this is a 'transient' failure response, and if so,
            // if the count of consecutive transient failures is small enough
            // that we will retry.
            // Also, do not do this processing on an initial SUBSCRIBE, as
            // resending an out-of-dialog SUBSCRIBE could collide with
            // subscriptions that were set up.
            // Response codes 408 and 482 are considered transient.
            // See XX-4591 and the sipX-dev thread starting at
            // http://list.sipfoundry.org/archive/sipx-dev/msg18258.html
            if (   (   responseCode == SIP_REQUEST_TIMEOUT_CODE
                    || responseCode == SIP_LOOP_DETECTED_CODE)
                && !(   method.compareTo(SIP_SUBSCRIBE_METHOD) == 0
                     && state->mRequestState == REFRESH_REQUEST_PENDING)
                && ++state->mTransientErrorCount <= sTransientErrorLimit
                && (unsigned long)state->mExpiration > OsDateTime::getSecsSinceEpoch() )
            {
               // A transient failure to be retried.

               OsSysLog::add(FAC_SIP, PRI_DEBUG,
                             "SipRefreshManager::handleSipMessage transient failure to be retried "
                             "responseCode = %d, method = '%s', "
                             "dialogHandle = '%s', mTransientErrorCount = %d, "
                             "mExpiration = %lu",
                             responseCode, method.data(),
                             dialogHandle.data(), state->mTransientErrorCount,
                             (unsigned long) state->mExpiration);

               // Stop the refresh timer.
               state->mRefreshTimer.stop(TRUE);

               // Set the refresh timer to the appropriate (short) retry
               // interval.
               state->setRefreshTimer(
                  FALSE,
                  sTransientResendDelay[state->mTransientErrorCount],
                  mpUserAgent->getSipStateTransactionTimeout() / 1000);
            }
            else
            {
               // All other success and failure responses.

               // Reset the count of transient failures.
               state->mTransientErrorCount = 0;

               // If the refresh state still records the early dialog,
               // update it to the confirmed dialog.
               // Do not do this for REGISTER requests, which never transition
               // to an established dialog.
               // (And below we may update the Route's in the stored
               // request.)
               if (foundEarlyDialog && matchesLastLocalTransaction &&
                   method.compareTo(SIP_REGISTER_METHOD) != 0)
               {
                  // Replace the state object's handle with the confirmed
                  // dialog handle and put it in the list under the new handle.
                  mRefreshes.removeReference(state);
                  *(dynamic_cast <UtlString*> (state)) = dialogHandle;
                  mRefreshes.insert(state);

                  // Update the stored request to have the new to-tag.
                  Url toUrl;
                  sipMessage->getToUrl(toUrl);
                  UtlString toTag;
                  toUrl.getFieldParameter("tag", toTag);
                  state->mpLastRequest->setToFieldTag(toTag);

                  OsSysLog::add(FAC_SIP, PRI_DEBUG,
                                "SipRefreshManager::handleSipMessage Updating state %p to established dialog handle '%s' and to-tag '%s'",
                                state, dialogHandle.data(), toTag.data());

                  // Update the state of the dialog recorded in mpDialogMgr.
                  mpDialogMgr->updateDialog(*sipMessage);
               }

               // Success responses.
               if (responseCode >= SIP_2XX_CLASS_CODE &&
                   responseCode < SIP_3XX_CLASS_CODE)
               {
                  int expirationPeriod = -1;

                  // An expiration time is required in the response, but
                  // we assume that if the expires value is not present
                  // then we got what we requested.
                  if (!getAcceptedExpiration(state, *sipMessage, expirationPeriod))
                  {
                     if (OsSysLog::willLog(FAC_SIP, PRI_WARNING))
                     {
                        UtlString b;
                        ssize_t l;
                        state->mpLastRequest->getBytes(&b, &l);
                        OsSysLog::add(FAC_SIP, PRI_WARNING,
                                      "SipRefreshManager::handleSipMessage Response did not provide expiration: %s",
                                      b.data());
                     }
                     expirationPeriod = state->mExpirationPeriodSeconds;
                  }

                  // Separate non-zero expirations from zero expirations.
                  if (expirationPeriod > 0)
                  {
                     // Stop the refresh timer.
                     state->mRefreshTimer.stop(TRUE);

                     // Set the refresh timer based on the provided time.
                     state->setRefreshTimer(TRUE,
                                            expirationPeriod,
                                            mpUserAgent->getSipStateTransactionTimeout() / 1000);

                     // Mark the refresh as successfully established.
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

                     // For future extensibility, use
                     // SipMessage::isRecordRouteAccepted to distinguish
                     // the REGISTER and SUBSCRIBE cases.
                     if (state->mpLastRequest->isRecordRouteAccepted())
                     {
                        // Copy Contact URI in response to
                        // request-URI in request.
                        UtlString contact;
                        sipMessage->getContactUri(0, &contact);
                        state->mpLastRequest->changeRequestUri(contact);

                        OsSysLog::add(FAC_SIP, PRI_DEBUG,
                                      "SipRefreshManager::handleSipMessage Updating request-URI to '%s'",
                                      contact.data());

                        if (foundEarlyDialog && matchesLastLocalTransaction)
                        {
                           // Remove Route headers from the initial request.
                           UtlString route;
                           while (state->mpLastRequest->removeHeader(SIP_ROUTE_FIELD, 0))
                           {
                              // null
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
                                         "SipRefreshManager::handleSipMessage Updating Route headers to '%s'",
                                         routeValue.data());
                        }
                     }
                  }
                  // This is an unSUBSCRIBE or unREGISTER (a 2xx response with expiration 0).
                  else
                  {
                     state->mExpiration = 0;
                     // If the notifier gave us a 0 expiration,
                     // it has terminated the subscription.
                     state->mRequestState = REFRESH_REQUEST_FAILED;

                     // Stop the refresh timer.
                     state->mRefreshTimer.stop(TRUE);
                  }

                  // The request succeeded.
                  OsSysLog::add(FAC_SIP, PRI_DEBUG,
                                "SipRefreshManager::handleSipMessage SUBSCRIBE received success response, state = %p, expirationPeriod = %d, state->mExpiration = %ld",
                                state, expirationPeriod, state->mExpiration);
               }

               // A failure response code other than authentication challenge.
               // We don't care what error -- It is the application's job
               // to care.  End the subscription.
               else
               {
                  state->mRequestState = REFRESH_REQUEST_FAILED;
                  state->mExpiration = 0;
                  OsSysLog::add(FAC_SIP, PRI_DEBUG,
                                "SipRefreshManager::handleSipMessage Failure");

                  // Stop the refresh timer.
                  state->mRefreshTimer.stop(TRUE);
               }

               // Request a callback to let the application know the state changed.
               OsSysLog::add(FAC_SIP, PRI_DEBUG,
                             "SipRefreshManager::handleSipMessage %p->mRequestState = %s",
                             state,
                             SipRefreshManager::refreshRequestStateText(state->mRequestState));
               postMessageP(
                  new CallbackRequestMsg(state,
                                         CallbackRequestMsg::CALLBACK,
                                         sipMessage,
                                         earlyDialogHandle,
                                         dialogHandle
                     ));
               // Ownership of *sipMessage has passed to the callback request,
               // so detach it from eventMessage.
               eventMessage.setMessage(NULL);
            }
         }
      }
      else
      {
         // Received an event for a dialog that we have no record of.
         OsSysLog::add(FAC_SIP, PRI_WARNING,
                       "SipRefreshManager::handleSipMessage Received event for dialog handle '%s' but cannot find refresh state for it",
                       dialogHandle.data());
      }
   } // endif SUBSCRIBE or REGISTER response

   // *sipMessage remains owned by eventMessage, and will be deleted by
   // the caller of SipRefreshManager::handleMessage.
   // But if we passed ownership of *sipMessage onward, we cleared the pointer
   // in eventMessage.
}

// The timer event routine for mRefreshTimer's.
// When a RefreshDialogState::mRefreshTimer fires, it queues to SipRefreshManager
// a RefreshEventMsg carrying a copy of the dialog handle of the
// RefreshDialogState.  SipRefreshManager::handleMessage eventually processes
// the RefreshEventMsg, and calls ::handleRefreshEvent.
// Thus, ::handleRefreshEvent may be called after the RefreshDialogState has
// been deleted or its state has been modified.  Fortunately, there is no
// functional failure if we just send a refresh message anyway.
OsStatus SipRefreshManager::handleRefreshEvent(const UtlString& handle)
{
   // Pointer to SipMessage (if any) to send after releasing the locks.
   // *message_to_send is owned by us.
   SipMessage* message_to_send = NULL;

   {
      // Seize lock.
      OsLock lock(mSemaphore);

      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipRefreshManager::handleRefreshEvent Timer fired, handle = '%s'",
                    handle.data());

      RefreshDialogState* state = getAnyDialog(handle);

      if (state)
      {
         // Legitimate states in which to see a timer fire to start
         // a re-SUBSCRIBE or re-REGISTER attempt.
         if (state->mRequestState == SipRefreshManager::REFRESH_REQUEST_FAILED ||
             state->mRequestState == SipRefreshManager::REFRESH_REQUEST_SUCCEEDED)
         {
            // Clean the message for resend.
            prepareForResend(*state,
                             FALSE); // do not expire now

            // Keep track of when this refresh is sent so we know
            // when the new expiration is relative to.
            state->mPendingStartTime = OsDateTime::getSecsSinceEpoch();
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "SipRefreshManager::handleRefreshEvent %p->mPendingStartTime = %ld",
                          state, state->mPendingStartTime);

            // Do not want to keep the lock while we send the message
            // as it could block, so we copy the message and send it after
            // we release the locks.
            message_to_send = new SipMessage(*(state->mpLastRequest));

            if (OsSysLog::willLog(FAC_SIP, PRI_DEBUG))
            {
               UtlString lastRequest;
               ssize_t length;
               state->mpLastRequest->getBytes(&lastRequest, &length);
               OsSysLog::add(FAC_SIP, PRI_DEBUG,
                             "SipRefreshManager::handleRefreshEvent last request = '%s'",
                             lastRequest.data());
            }
         }
         // This should not happen
         else
         {
            OsSysLog::add(FAC_SIP, PRI_ERR,
                          "SipRefreshManager::handleRefreshEvent "
                          "timer fired in unexpected state %s",
                          SipRefreshManager::refreshRequestStateText(state->mRequestState));
            // Dump the state into the log.
            state->dumpState();
         }
      }
      else
      {
         OsSysLog::add(FAC_SIP, PRI_WARNING,
                       "SipRefreshManager::handleRefreshEvent "
                       "no state found to match handle '%s'",
                       handle.data());
      }      
   }

   // Now that the locks are released, send the message, if we have one to send.
   if (message_to_send)
   {
       mpUserAgent->send(*message_to_send);
       // SipUserAgent::send() doesn't take ownership of sipMessage,
       // so we must delete it.
       delete message_to_send;
   }

   return OS_SUCCESS;
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

   // Seize lock.
   OsLock lock(mSemaphore);

   UtlHashBagIterator iterator(mRefreshes);
   RefreshDialogState* state = NULL;
   UtlString oneStateDump;

   while((state = dynamic_cast <RefreshDialogState*> (iterator())))
   {
      state->toString(oneStateDump);
      dumpString.append(oneStateDump);
      count++;
   }

   return(count);
}


/* ============================ INQUIRY =================================== */

// Dump the object's internal state.
void SipRefreshManager::dumpState()
{
   // Seize lock.
   OsLock lock(mSemaphore);

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
}

// Convert RefreshRequestState to a printable string.
const char* SipRefreshManager::refreshRequestStateText(SipRefreshManager::RefreshRequestState requestState)
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

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

RefreshDialogState* SipRefreshManager::getAnyDialog(const UtlString& messageDialogHandle)
{
   RefreshDialogState* state =
      dynamic_cast <RefreshDialogState*> (mRefreshes.find(&messageDialogHandle));

   if (state == NULL)
   {
      UtlString reversedHandle;
      SipDialog::reverseTags(messageDialogHandle, reversedHandle);
      state =
         dynamic_cast <RefreshDialogState*> (mRefreshes.find(&reversedHandle));
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
         state =
            dynamic_cast <RefreshDialogState*>
            (mRefreshes.find(&establishedDialogHandle));
         if(state == NULL)
         {
            UtlString reversedEstablishedDialogHandle;
            SipDialog::reverseTags(establishedDialogHandle, reversedEstablishedDialogHandle);
            state =
               dynamic_cast <RefreshDialogState*>
               (mRefreshes.find(&reversedEstablishedDialogHandle));
         }
      }

      // If this is an established dialog, find out what the
      // early dialog handle was and see if we can find it
      else
      {
         UtlString earlyDialogHandle;
         mpDialogMgr->getEarlyDialogHandleFor(messageDialogHandle,
                                              earlyDialogHandle);

         state =
            dynamic_cast <RefreshDialogState*>
            (mRefreshes.find(&earlyDialogHandle));
         if(state == NULL)
         {
            UtlString reversedEarlyDialogHandle;
            SipDialog::reverseTags(earlyDialogHandle, reversedEarlyDialogHandle);
            state =
               dynamic_cast <RefreshDialogState*>
               (mRefreshes.find(&reversedEarlyDialogHandle));
         }
      }
   }

   return(state);
}

UtlBoolean SipRefreshManager::stateExists(RefreshDialogState* statePtr)
{
   // Our caller holds the lock.

   RefreshDialogState* state =
      dynamic_cast <RefreshDialogState*> (mRefreshes.findReference(statePtr));

   return(state != NULL);
}

RefreshDialogState*
    SipRefreshManager::createNewRefreshState(SipMessage& subscribeOrRegisterRequest,
                                              UtlString& messageDialogHandle,
                                              void* applicationData,
                                              const RefreshStateCallback refreshStateCallback,
                                              int& requestedExpiration)
{
   RefreshDialogState* state =
      new RefreshDialogState(messageDialogHandle, this);
   state->mpApplicationData = applicationData;
   // refreshStateCallback may not be NULL, because we need to be able to tell
   // the application that a refresh has failed, and the application needs to
   // call stopRefresh to purge the refresh state.
   assert(refreshStateCallback);
   state->mpStateCallback = refreshStateCallback;
   // Set the expiration time to mDefaultExpiration if the request
   // does not contain an expiration time.
   if (!getInitialExpiration(subscribeOrRegisterRequest,
                             state->mExpirationPeriodSeconds)) // original expiration
   {
      state->mExpirationPeriodSeconds = mDefaultExpiration;
      subscribeOrRegisterRequest.setExpiresField(mDefaultExpiration);
   }
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipRefreshManager::createNewRefreshState %p->mExpirationPeriodSeconds = %d",
                 state, state->mExpirationPeriodSeconds);

   // Zero the count of consecutively seen transient failure responses.
   state->mTransientErrorCount = 0;

   requestedExpiration = state->mExpirationPeriodSeconds;

   return state;
}

// Adjust the SipDialogMgr, the RefreshDialogState, and the RefreshDialogState's
// attached request in preparation for retransmitting the request.
void SipRefreshManager::prepareForResend(RefreshDialogState& state,
                                         UtlBoolean expireNow)
{
   if (state.mpLastRequest)
   {
      // Update state and its attached request.
      state.resetStateForResend(expireNow);

      // Increment the CSeq recorded in SipDialogMgr.
      // Copy the dialog information into the stored request.
      mpDialogMgr->setNextLocalTransactionInfo(*state.mpLastRequest);
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_ERR,
                    "SipRefreshManager::prepareForResend called with a state (%p '%s') that has no last request recorded",
                    &state, state.data());
   }
}

// Get the expiration from the initial SUBSCRIBE or REGISTER request.
UtlBoolean SipRefreshManager::getInitialExpiration(const SipMessage& sipRequest,
                                                   int& expirationPeriod)
{
   UtlString method;
   UtlBoolean foundExpiration = FALSE;
   sipRequest.getRequestMethod(&method);

   if (method.compareTo(SIP_REGISTER_METHOD) == 0)
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

   if (!foundExpiration)
   {
      // Not sure if we care if this is a request or response
      foundExpiration = sipRequest.getExpiresField(&expirationPeriod);
   }

   return foundExpiration;
}

// Get the expiration from the SUBSCRIBE or REGISTER response.
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
      // Get the first contact in the REGISTER request so that we
      // can find the same contact in the response and find out what
      // its expiration is.
      // :TODO: Fix this, because the first contact may not be the one we are
      // registering.
      UtlString requestContact;
      Url requestContactUri;
      if(state && state->mpLastRequest &&
         state->mpLastRequest->getContactEntry(0, &requestContact))
      {
         requestContactUri = requestContact;
      }

      // The response could have the expiration time in the Contact header.
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

   // If there wasn't an expires parameter of a REGISTER contact, look for an
   // Expires header.
   if (!foundExpiration)
   {
      foundExpiration = sipResponse.getExpiresField(&expirationPeriod);
   }

   return (foundExpiration);
}

/* ============================ FUNCTIONS ================================= */

static int calculateResendTime(int requestedExpiration,
                               int minimumRefresh)
{
   // Basic refresh is 65% of the granted expiration time.
   int expiration = (int)(0.65 * requestedExpiration);

   // The transaction timeout is the minimum time we will return.
   if (expiration < minimumRefresh)
   {
      expiration = minimumRefresh;
   }

   return expiration;
}
