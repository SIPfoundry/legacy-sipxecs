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

#include <os/OsDefs.h>
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
 *  This class is intended to deprecate the SipRefreshMgr class.
 *
 * \par 
 */
class SipRefreshManager : public OsServerTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

    enum RefreshRequestState
    {
       REFRESH_REQUEST_UNKNOWN,  // not yet initialized
       REFRESH_REQUEST_PENDING,  // a SUBSCRIBE has been sent but has not
                                 // received a final response
       REFRESH_REQUEST_FAILED,   // last SUBSCRIBE received a non-remediable
                                 // failure response
       REFRESH_REQUEST_SUCCEEDED // last SUBSCRIBE received a success response
    };

    /*! Typedef defining the signature of the callback used to inform
     *  applications of refresh subscription and registration state
     *  changes.
     *  \param requestState - the state of the last SUBSCRIBE or NOTIFY
     *         transaction.  Note: this is not the same as the dialog state.
     *         The dialog state is indicted by the fact that expiration
     *         is in the future (i.e. subscribed or registered) or in the
     *         past or zero (i.e. expired).
     *  \param earlyDialogHandle - provided if still an early dialog or if
     *         the dialog just changed to an established dialog otherwise NULL
     *  \param dialogHandle - provided if dialog is established otherwise NULL
     *  \param applicationData - pass back of application data provided upon
     *         adding SUBSCRIBE or REGISTRATION to be refreshed
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
     *         reREGISTER.  Usual zero indicates that the application stopped
     *         the refresh.  A value of -1 indicates that no requests have 
     *         succeeded yet. A value greater than zero and less than the 
     *         current epoch time indicates the SUBSCRIBE or REGISTER has 
     *         expired most likely because of a request failure.
     *  \param subscribeResponse - SIP SUBSCRIBE or REGISTER response which
     *         stimulated the state change.  May be NULL.
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
     *  Returns TRUE if the request was sent and the 
     *  refresh state proceeded to REFRESH_INITIATED.
     *  Returns FALSE if the request was not able to
     *  be sent, the refresh state is set to REFRESH_FAILED.
     *  The caller of this method must explictly call stopRefresh
     *  to clean up the refresh state even if this method fails.
     *  This method may fail if the dialog or refresh state already
     *  exists or if the request immediately fails to send.  The
     *  refresh manager may attempt to resend the request to
     *  subscribe or register even if it fails the first time while
     *  this method is invoked, but most error responses to the
     *  request terminate the refresh state.
     */
    UtlBoolean initiateRefresh(SipMessage& subscribeOrRegisterRequest,
                               void* applicationData,
                               const RefreshStateCallback refreshStateCallback,
                               UtlString& earlyDialogHandle,
                               UtlBoolean suppressFirstSend = FALSE);

    //! End the SIP refresh (registration or subscription) indicated by 
    /*! the dialog handle.  If the given dialogHandle is an early dialog it
     *  will end any established subscriptions.  Typically 
     *  the application SHOULD use the established dialog handle.  This
     *  method can also be used to end one of the dialogs if multiple
     *  subsription dialogs were created as a result of a single 
     *  subscribe request.  The application will get multiple 
     *  REFRESH_SETUP RefreshStateCallback events when
     *  multiple dialogs are created as a result of a single SUBSCRIBE.
     *  To end one of the subscriptions the application should use
     *  the setup dialogHandle provided by the SubscriptionStateCallback.
     *  Only a single registration can occur as a result of sending
     *  a REGISTER request.
     */
    UtlBoolean stopRefresh(const char* dialogHandle);

    //! Stop refreshing, unregister and unsubscribe all
    void stopAllRefreshes();

    //! Handler for SUBSCRIBE and REGISTER responses
    UtlBoolean handleMessage(OsMsg &eventMessage);

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

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    //! lock for single thread use
    void lock();
    //! lock for single thread use
    void unlock();

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

    //! Create a new timer and set it to fire when its time to resend
    void setRefreshTimer(RefreshDialogState& state, 
                         UtlBoolean isSuccessfulReschedule);

    //! Calculate the time in seconds when a refresh should occur
    /*! Assume that the register or subscribe will succeed and that
     *  we should send the refresh safely before the expiration
     */
    int calculateResendTime(int requestedExpiration, 
                            UtlBoolean isSuccessfulResend);

    //! Stop the resend timer to indicate that it should be rescheduled with a short, failure timeout
    void stopTimerForFailureReschedule(OsTimer* timer);

    //! Delete the given timer and its associated notiifer
    static void deleteTimerAndEvent(OsTimer*& timer);

    //! set the given state and attached request so that it can be resent
    void setForResend(RefreshDialogState& state, 
                      UtlBoolean expireNow);

    //! Get the expiration from the initial SUBSCRIBE or REGISTER request
    static UtlBoolean getInitialExpiration(const SipMessage& sipRequest, 
                                           int& expirationPeriod);

    //! Get the expiration from the accepted SUBSCRIBE or REGISTER response
    static UtlBoolean getAcceptedExpiration(RefreshDialogState* refreshState,
                                            const SipMessage& sipResponse, 
                                            int& expirationPeriod);

    OsMutex mRefreshMgrMutex; // used to lock this 
    SipUserAgent* mpUserAgent;
    SipDialogMgr* mpDialogMgr;
    // RefreshDialogState for each subscription and registration being
    // maintained, indexed by the dialog handle of the dialog or
    // pseudo-dialog that is maintaining it.
    UtlHashBag mRefreshes;
    // TRUE if we have registered a message observer for REGISTER responses.
    UtlBoolean mReceivingRegisterResponses;
    // TRUE if we have registered a message observer for SUBSCRIBE responses.
    UtlBoolean mReceivingSubscribeResponses;
    int mDefaultExpiration;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipRefreshManager_h_
