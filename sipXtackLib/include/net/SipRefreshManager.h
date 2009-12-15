//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _SipRefreshManager_h_
#define _SipRefreshManager_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES

#include <net/SipMessageEvent.h>
#include <os/OsDefs.h>
#include <os/OsEventMsg.h>
#include <os/OsServerTask.h>
#include <utl/UtlHashBag.h>
#include <net/SipDialog.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// FORWARD DECLARATIONS
class SipMessage;
class SipUserAgent;
class SipDialogMgr;
class RefreshDialogState;
class OsTimer;

// TYPEDEFS

//! Class for refreshing SIP subscriptions and registrations
/*! This is currently verified for SUBSCRIPTIONS ONLY.
 *  This class is intended to replace the SipRefreshMgr class.
 *
 * \par
 */
class SipRefreshManager : public OsServerTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

    enum RefreshRequestState
    {
       REFRESH_REQUEST_UNKNOWN,  // not yet started
       REFRESH_REQUEST_PENDING,  // an initial request has been sent but has not
                                 // received a final response, and therefore
                                 // a terminating request (with "Expires: 0")
                                 // cannot yet be sent
                                 // (Can only happen with SUBSCRIBE.)
       REFRESH_REQUEST_FAILED,   // last request received a non-remediable
                                 // failure response
       REFRESH_REQUEST_SUCCEEDED // last request received a success response,
                                 // or was an initial request that does not
                                 // require waiting before sending a terminating
                                 // request
                                 // (The latter can only happen with REGISTER.)
    };

    /*! Typedef defining the signature of the callback used to inform
     *  applications of refresh subscription and registration state
     *  changes.
     *  \param requestState - the state of the last SUBSCRIBE or NOTIFY
     *         transaction.  Note: this is not the same as the dialog state.
     *         The dialog state is indicted by the fact that expiration
     *         is in the future (i.e. subscribed or registered) or in the
     *         past or zero (i.e. expired).
     *         Note that a callback with requestState == REFRESH_REQUEST_FAILED
     *         requires a call of stopRefresh to purge the state information
     *         from the SipRefreshManager.
     *  \param earlyDialogHandle - always provided (which is changed
     *         from previous versions of this method, which only provided
     *         earlyDialogHandle while the refresh was being established)
     *         the dialog just changed to an established dialog otherwise NULL
     *  \param dialogHandle - provided if dialog is established otherwise NULL
     *  \param applicationData - pass back of application data provided upon
     *         adding SUBSCRIBE or REGISTER to be refreshed
     *  \param responseCode - SIP response code 200-299 Success, > 300 failure
     *         Has no meaning if subscribeResponse is NULL
     *         NOTE: a failure code does not mean that the subscription or
     *         registration has expired.  The previous SUBSCRIBE or REGISTER
     *         may have succeeded and may not have expired yet.
     *  \param responseText - SIP response text. NULL if subscribeResponse is NULL
     *  \param expiration - epoch time in seconds when expiration of subscription
     *         or registration will occur.  A value of zero indicates that
     *         a unSUBSCRIBE or unREGISTER occurred and that it has expired
     *         and that no further attempts will be made to reSUBSCRIBE or
     *         reREGISTER.  Usually zero indicates that the application stopped
     *         the refresh.  A value of -1 indicates that no requests have
     *         succeeded yet. A value greater than zero and less than the
     *         current epoch time indicates the SUBSCRIBE or REGISTER has
     *         expired, most likely because of a request failure.
     *  \param subscribeResponse - SIP SUBSCRIBE or REGISTER response which
     *         stimulated the state change.  May be NULL.  Ownership retained
     *         by SipRefreshManager.
     *
     *  All callbacks are called from the SipRefreshManager's thread.  In
     *  particular, no locks will be held by the thread when the callback is
     *  called.
     *  All refreshes eventually generate callbacks, if only to report the
     *  failure of the initial request.
     */
    typedef void (*RefreshStateCallback) (SipRefreshManager::RefreshRequestState requestState,
                                       const char* earlyDialogHandle,
                                       const char* dialogHandle,
                                       void* applicationData,
                                       int responseCode,
                                       const char* responseText,
                                       long expiration, // epoch seconds
                                       const SipMessage* response);

/* ============================ CREATORS ================================== */

    //! Constructor
    SipRefreshManager(SipUserAgent& userAgent,
                      SipDialogMgr& dialogMgr);


    //! Destructor
    virtual
    ~SipRefreshManager();


/* ============================ MANIPULATORS ============================== */

    //! Send message and keep request refreshed (i.e. subscribed or registered)
    /*!
     *  The request can be out-of-dialog, or in-dialog (e.g.,
     *  generated from a NOTIFY from a fork of a SUBSCRIBE from which
     *  we did not receive a response).
     *
     *  SipRefreshManager sends the request and re-sends it periodically
     *  as indicated by the accepted expirations seen in responses.
     *  Failure responses due to authentication challenges are re-sent
     *  immediately.  Failure responses due to defined transient error
     *  conditions are retried with exponential backoff.  (Except not for
     *  initial SUBSCRIBE requests, as a retry might collide with established
     *  subscriptions.)
     *  The state of the refresh dialog/quasi-dialog is reported via the
     *  callback function.
     *
     *  Certain immediate error conditions (generally, if the dialog identifiers
     *  of subscribeOrRegisterRequest match a currently active refresh) cause
     *  initiateRefresh to return false, in which case no state is created
     *  and there will be no callback.  Otherwise, initiateRefresh returns
     *  true (immediately upon sending the request), a state is
     *  created, but success of the attempted request can only be determined
     *  by monitoring the callbacks.
     *
     *  The caller of this method must explicitly call stopRefresh
     *  to clean up the refresh state, even if the callback function
     *  has reported REFRESH_REQUEST_FAILED.
     *
     *  State changes for the refresh are always reported via the callback
     *  function, including the initial state set by initiateRefresh and
     *  the transition to REFRESH_REQUEST_FAILED caused by calling stopRefresh
     *  on a refresh that has not terminated.
     */
    UtlBoolean initiateRefresh(SipMessage* subscribeOrRegisterRequest,
                               ///< becomes owned by SipRefreshManager
                               void* applicationData,
                               const RefreshStateCallback refreshStateCallback,
                               UtlString& earlyDialogHandle,
                               UtlBoolean suppressFirstSend = FALSE);

    //! End the SIP refresh (registration or subscription) indicated by
    /*! the dialog handle.
     *  If the given dialogHandle is an early dialog it will end any
     *  established subscriptions that match the early dialog handle.
     *  Typically the application should use the established dialog
     *  handle.
     *  This method can be used to end one of the dialogs if multiple
     *  subscription dialogs were created as a result of a single
     *  subscribe request.  The application will get multiple
     *  REFRESH_REQUEST_SUCCEEDED RefreshStateCallback events when
     *  multiple dialogs are created as a result of a single SUBSCRIBE.
     *  To end one of the subscriptions, the application should use
     *  the confirmed dialogHandle provided by the SubscriptionStateCallback.
     *  Only a single registration can occur as a result of sending
     *  a REGISTER request.
     *  If stopRefresh stops a (quasi)dialog that has not been stopped
     *  before, the state callback will be called to signal its stopping.
     */
    UtlBoolean stopRefresh(const char* dialogHandle,
                           UtlBoolean noSend = FALSE
                           ///< if true, do not send terminating request
       );

    //! Stop refreshing, unregister and unsubscribe all
    void stopAllRefreshes();

    //! Change the refresh period.
    /*! Change the Expires value of the stored request,
     *  send a request with the new Expires value,
     *  and start a new refresh timer
     */
    UtlBoolean changeRefreshTime(const char* earlyDialogHandle,
                                 int expirationPeriodSeconds);

    //! Change the refresh period for this refresh only.
    UtlBoolean changeCurrentRefreshTime(const char* earlyDialogHandle,
                                        int expirationPeriodSeconds);

    //! Handler for SUBSCRIBE and REGISTER responses
    UtlBoolean handleMessage(OsMsg &eventMessage);

    /// Handle an incoming SIP message.
    void handleSipMessage(SipMessageEvent& eventMessage);

    /// Process a refresh timer event.
    OsStatus handleRefreshEvent(UtlString& handle);

/* ============================ ACCESSORS ================================= */

    //! Get a copy of the refresh request message for a given dialog handle.
    //  Returns true if the state was found.
    UtlBoolean getRequest(const UtlString& dialogHandle, SipMessage& message);

    //! Debugging method to get an dump of all refresh states
    int dumpRefreshStates(UtlString& dumpString);

    //! Get a string representation of the refresh state enumeration
    static void refreshState2String(RefreshRequestState state, UtlString& stateString);

/* ============================ INQUIRY =================================== */

    //! Get a count of the subscriptions and registration which have been added
    int countRefreshSessions() const;

    //! Dump the object's internal state.
    void dumpState();

    //! Convert RefreshRequestState to a printable string.
    static const char* refreshRequestStateText(SipRefreshManager::RefreshRequestState requestState);

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    //! Copy constructor NOT ALLOWED
    SipRefreshManager(const SipRefreshManager& rSipRefreshManager);

    //! Assignment operator NOT ALLOWED
    SipRefreshManager& operator=(const SipRefreshManager& rhs);

    //! Accessor to get the state that matches either an established or early dialog
    RefreshDialogState* getAnyDialog(const UtlString& messageDialogHandle);

    //! Accessor to verify state is still in the mRefreshes container
    UtlBoolean stateExists(RefreshDialogState* statePtr);

    //! helper function to construct and initialize refresh state
    RefreshDialogState* createNewRefreshState(SipMessage& subscribeOrRegisterRequest,
                                              UtlString& messageDialogHandle,
                                              void* applicationData,
                                              const RefreshStateCallback refreshStateCallback,
                                              int& requestedExpiration);

    //! Delete the given timer and its associated notifier.
    static void deleteTimerAndEvent(OsTimer*& timer);

    //! Clean the state and attached request so that the request can be resent.
    void prepareForResend(RefreshDialogState& state,
                          ///< the refresh state containing the request to resend
                          UtlBoolean expireNow
                          /**< If TRUE, the subscription/registration is
                           *   to be terminated, so the expiration time will be
                           *   set to 0.
                           */
       );

    //! Get the expiration from the initial SUBSCRIBE or REGISTER request.
    static UtlBoolean getInitialExpiration(const SipMessage& sipRequest,
                                           int& expirationPeriod);

    //! Get the expiration from the SUBSCRIBE or REGISTER response.
    static UtlBoolean getAcceptedExpiration(RefreshDialogState* refreshState,
                                            const SipMessage& sipResponse,
                                            int& expirationPeriod);

    /// The SipUserAgent to be used.
    SipUserAgent* mpUserAgent;
    /// The SipDialogMgr to be used.
    SipDialogMgr* mpDialogMgr;
    /// The default subscription expiration to be used.
    int mDefaultExpiration;

    // See sipXtackib/doc/developer/SipRefreshManager-Locking.txt for
    // how the locks are used.
    /// Lock to protect mRefreshes.
    OsBSem mSetSem;
    /// Lock to protect individual objects in mRefreshes.
    OsBSem mObjectSem;

    // RefreshDialogState for each subscription and registration being
    // maintained, indexed by the dialog handle of the dialog or
    // pseudo-dialog that is maintaining it.
    UtlHashBag mRefreshes;

    /// True if we have registered a message observer for REGISTER responses.
    UtlBoolean mReceivingRegisterResponses;
    /// True if we have registered a message observer for SUBSCRIBE responses.
    UtlBoolean mReceivingSubscribeResponses;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipRefreshManager_h_
