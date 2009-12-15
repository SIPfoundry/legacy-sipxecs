//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$

//////////////////////////////////////////////////////////////////////////////

#ifndef _SipSubscribeClient_h_
#define _SipSubscribeClient_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES

#include <os/OsDefs.h>
#include <os/OsServerTask.h>
#include <utl/UtlHashBag.h>
#include <net/SipRefreshManager.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// FORWARD DECLARATIONS

class ReestablishRequestMsg;
class SipMessage;
class SipUserAgent;
class SipDialogMgr;
class SipRefreshManager;
class SubscriptionGroupState;
class SubscriptionDialogState;

// TYPEDEFS

//! Class for maintaining the subscriber role of SIP subscriptions
/** Once a SipSubscribeClient has been created, it can be directed to
 *  create and maintain subscriptions.  SUBSCRIBE messages are sent
 *  using addSubscription.  Each use of addSubscription creates an
 *  early dialog, and any subscriptions that are established are
 *  created as established dialogs.  The creation and destruction of
 *  early and established dialogs is reported through a callback
 *  function, and the contents of any NOTIFYs received in the
 *  subscriptions is reported through another callback function.
 *
 *  Normally, if an established subscription is ended due to error or
 *  action of the notifier, SipSubscribeClient attempts to reestablish it.
 *  In addition, each group of subscription dialogs is terminated and
 *  reestablished on a long time period (12 to 24 hours).
 *  Both of the above behaviors can be suppressed for a subscription by
 *  optional arguments to addSubscription.
 */
class SipSubscribeClient : public OsServerTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

    // These 'friend' declarations could probably be eliminated by a couple of
    // suitable accessors.
    friend class ReestablishRequestMsg;
    friend class SubscriptionStartingNotification;
    friend class SubscriptionRestartNotification;

    enum SubscriptionState
    {
        SUBSCRIPTION_INITIATED, // Early dialog
        SUBSCRIPTION_SETUP,     // Established dialog
        SUBSCRIPTION_TERMINATED // Ended dialog
    };

    /// Type of the callback function for reporting subscription state changes.
    /** Note SipSubscribeClient does automatically refresh subscriptions, but
     *  if the subscription is involuntarily ended (by the notifier or otherwise),
     *  SipSubscribeClient will not attempt to reestablish it.
     */
    typedef void (*SubscriptionStateCallback)
       (SipSubscribeClient::SubscriptionState newState,
        const char* earlyDialogHandle,
        /**< The earlyDialogHandle which was returned by the relevant
         *   addSubscription call.  This is the early dialog handle of
         *   the initial SUBSCRIBE message, but will differ from the
         *   early dialog handle of any reestablishment SUBSCRIBE.
         */
        const char* dialogHandle,
        ///< the established dialog handle of the subscription
        void* applicationData,
        ///< the applicationData supplied to addSubscription
        int responseCode,
        const char* responseText,
        /**< responseCode and responseTest will be from the relevant failure
         *   response, or -1 and NULL, respectively.
         */
        long expiration,
        ///< current expiration time of the subscription, or -1 if not known
        //   See documentation of the expiration parameter of
        //   SipRefreshManager::RefreshStateCallback for details.
        const SipMessage* subscribeResponse
        ///< relevent response message, or NULL
        //   Ownership is retained by SipSubscribeClient.
        );

    /// Type of the callback function for reporting received NOTIFYs.
    /** Note that NOTIFYs are generally reported in the order they are
     *  received.  Duplicated NOTIFYs are not reported, as the duplicated
     *  messages are removed in the SIP stack.  Similarly, out-of-order
     *  NOTIFYs are rejected in the SIP stack and are not reported.
     *  This provides the desired behavior if the notifications for a
     *  subscription give a complete update of the subscription's
     *  state.  If notifications only give partial updates, and so
     *  missed NOTIFYs must be detected, the NOTIFYs need to contain
     *  serial numbers (as many SIP event packages do), as the CSeq
     *  numbers cannot be relied upon to be sequential.
     *  (If the application needs a full update, it must direct
     *  SipSubscribeClient to terminate the subscription and begin a
     *  new one.)
     *  If the callback returns true, its caller will send a 200 response to the NOTIFY.
     *  If the callback returns false, its caller will send no response, and the callback
     *  is responsible for sending an appropriate response itself.
     *
     */
    typedef bool (*NotifyEventCallback)
       (const char* earlyDialogHandle,
        ///< handle of the early dialog generated by the original SUBSCRIBE.
        const char* dialogHandle,
        ///< handle of the established dialog generated by the first NOTIFY.
        void* applicationData,
        ///< the applicationData supplied to addSubscription
        const SipMessage* notifyRequest
        ///< the NOTIFY request (does not become owned by the callback)
        //   Ownership is retained by SipSubscribeClient.
          );

/* ============================ CREATORS ================================== */

    //! Default Dialog constructor
    SipSubscribeClient(SipUserAgent& userAgent,
                       SipDialogMgr& dialogMgr,
                       SipRefreshManager& refreshMgr);

    //! Destructor
    virtual
    ~SipSubscribeClient();

/* ============================ MANIPULATORS ============================== */

    //! Create a SIP event subscription based on values specified to create a SUBSCRIBE message
    /*!
     *  Returns TRUE if the SUBSCRIBE request was sent and the
     *  Subscription state proceeded to SUBSCRIPTION_INITIATED.
     *  Returns FALSE if the SUBSCRIBE request was not able to
     *  be sent, the subscription state is set to SUSCRIPTION_FAILED.
     */
    UtlBoolean addSubscription(const char* resourceId,
                               /**< The request-URI for the SUBSCRIBE.
                                *   The request-URI as it arrives at the
                                *   notifier will (conventionally) be used
                                *   (without parameters) as the
                                *   resourceId to subscribe to.
                                */
                               const char* eventHeaderValue,
                               //< value of the Event header
                               const char* acceptHeaderValue,
                               //< value of the Accept header
                               const char* fromFieldValue,
                               //< value (name-addr) of the From header
                               const char* toFieldValue,
                               //< value (name-addr) of the To header
                               const char* contactFieldValue,
                               /**< Value of the Contact header.
                                *   Must route to this UA.
                                */
                               int subscriptionPeriodSeconds,
                               //< subscription period to request
                               void* applicationData,
                               //< application data to pass to the callback funcs.
                               const SubscriptionStateCallback subscriptionStateCallback,
                               //< callback function for begin/end of subscriptions (or NULL)
                               const NotifyEventCallback notifyEventsCallback,
                               //< callback function for incoming NOTIFYs (or NULL)
                               UtlString& earlyDialogHandle,
                               /**< returned handle for the early
                                *   dialog created by the SUBSCRIBE
                                */
                               bool reestablish = true,
                               /**< if true, reestablish the subscription if one
                                *   of the subscriptions fails.
                                */
                               bool restart = true
                               /**< if true, restart the subscription on a long
                                *   time cycle (12 to 24 hours)
                                */
       );

    //! Create a SIP event subscription based on a provided SUBSCRIBE request
    /*!
     *  Returns TRUE if the SUBSCRIBE request was sent and the
     *  Subscription state proceeded to SUBSCRIPTION_INITIATED.
     *  Returns FALSE if the SUBSCRIBE request was not able to
     *  be sent, the subscription state is set to SUSCRIPTION_FAILED.
     */
    UtlBoolean addSubscription(SipMessage* subscriptionRequest,
                               ///< becomes owned by SipSubscribeClient
                               void* applicationData,
                               const SubscriptionStateCallback subscriptionStateCallback,
                               const NotifyEventCallback notifyEventsCallback,
                               UtlString& earlyDialogHandle,
                               bool reestablish = true,
                               bool restart = true);

    //! End the SIP event subscription indicated by the dialog handle
    /*  This ends only the subscription dialog indicated by the handle,
     *  the other subscriptions in the group remain.
     *  Does not trigger the reestablishment of the subscription.
     */
    UtlBoolean endSubscriptionDialog(const UtlString& dialogHandle);

    //! End the SIP event subscription indicated by the dialog handle
    /*  This ends only the subscription dialog indicated by the handle,
     *  the other subscriptions in the group remain.
     *  Used when the notifier has ended the subscription (by sending
     *  a NOTIFY with "Subscription-State: terminated"), so we do
     *  not need to send an un-SUBSCRIBE.
     *  Caller MUST hold the lock!  (unlike ::endSubscriptionDialog)
     */
    UtlBoolean endSubscriptionDialogByNotifier(const UtlString& dialogHandle);

    //! End the SIP event subscription group indicated by the early dialog handle
    /*! Ends any established or early dialog subscriptions created by the
     *  addSubscription() that returned the early dialog handle.
     */
    UtlBoolean endSubscriptionGroup(const UtlString& dialogHandle);

    //! End all subscriptions known by the SipSubscribeClient.
    void endAllSubscriptions();

    /** Change the subscription expiration period for all subscriptions
     *  in a group..
     *  Send a new SUBSCRIBE with a different timeout value, and set
     *  refresh timers appropriately.  Future re-SUBSCRIBEs will use
     *  the new timeout value.
     */
    UtlBoolean changeSubscriptionTime(const char* earlyDialogHandle,
                                      int subscriptionPeriodSeconds);

    //! Handler for NOTIFY requests
    UtlBoolean handleMessage(OsMsg &eventMessage);

/* ============================ ACCESSORS ================================= */

    //! Get a string representation of an 'enum SubscriptionState' value.
    static void getSubscriptionStateEnumString(enum SubscriptionState stateValue,
                                               UtlString& stateString);

/* ============================ INQUIRY =================================== */

    //! Get a count of the subscription groups which currently exist.
    int countSubscriptionGroups();

    //! Get a count of the subscription dialogs which currently exist.
    int countSubscriptionDialogs();

    //! Dump the object's internal state.
    void dumpState();

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    //! Callback to handle subscription state changes from the refresh manager
    /*! RefreshStateCallback
     */
    static void refreshCallback(SipRefreshManager::RefreshRequestState newState,
                                const char* earlyDialogHandle,
                                const char* dialogHandle,
                                void* subscribeClientPtr,
                                int responseCode,
                                const char* responseText,
                                long expiration, // epoch seconds
                                const SipMessage* subscribeResponse);

    //! Method version of callback function for Refresh Manager.
    void refreshCallback(SipRefreshManager::RefreshRequestState newState,
                         const char* earlyDialogHandle,
                         const char* dialogHandle,
                         int responseCode,
                         const char* responseText,
                         long expiration, // epoch seconds
                         const SipMessage* subscribeResponse);

    //! Handle incoming notify request
    void handleNotifyRequest(const SipMessage& notifyRequest);

    //! Add the state for a subscription group to mSubscriptionGroups.
    /*  Assumes external locking
     */
    void addGroupState(SubscriptionGroupState* groupState);

    //! Add the state for a subscription dialog to mSubscriptionDialogs.
    /*  Assumes external locking
     */
    void addDialogState(SubscriptionDialogState* dialogState);

    //! find the state from mSubscriptionGroups with the specified current handle
    /*  Assumes external locking
     */
    SubscriptionGroupState* getGroupStateByOriginalHandle(const UtlString& dialogHandle);

    //! find the state from mSubscriptionGroups with the specified original handle
    /*  Assumes external locking
     */
    SubscriptionGroupState* getGroupStateByCurrentHandle(const UtlString& dialogHandle);

    //! find the state from mSubscriptionDialogs that matches the handle
    /*  Assumes external locking
     */
    SubscriptionDialogState* getDialogState(const UtlString& dialogHandle);

    /** Recalculate the current early dialog handle from the SUBSCRIBE request
     *  and revise mSubscriptionGroupsByCurrentHandle.
     *  Assumes external locking
     */
    void reindexGroupState(SubscriptionGroupState* groupState);

    //! remove the state from mSubscriptionGroups with the specified original handle
    /*  Assumes external locking
     */
    SubscriptionGroupState* removeGroupStateByOriginalHandle(const UtlString& dialogHandle);

    //! remove the state from mSubscriptionGroups with the specified current handle
    /*  Assumes external locking
     */
    SubscriptionGroupState* removeGroupStateByCurrentHandle(const UtlString& dialogHandle);

    //! remove the state from mSubscriptionDialogs that matches the dialog
    /*  Assumes external locking
     */
    SubscriptionDialogState* removeDialogState(const UtlString& dialogHandle);

    //! Copy constructor NOT ALLOWED
    SipSubscribeClient(const SipSubscribeClient& rSipSubscribeClient);

    //! Assignment operator NOT ALLOWED
    SipSubscribeClient& operator=(const SipSubscribeClient& rhs);

    SipUserAgent* mpUserAgent;
    SipDialogMgr* mpDialogMgr;
    SipRefreshManager* mpRefreshManager;

    /** SubscriptionGroupState objects carrying the information for
     *  each subscription group.
     */
    UtlHashBag mSubscriptionGroups;
    /** Index of mSubscriptionGroups by the current early dialog handle
     *  (mCurrentEarlyDialogHandle).
     *  Note that mSubscriptionGroupsByCurrentHandle does not own either the
     *  keys or the values of this UtlHashMap.
     */
    UtlHashMap mSubscriptionGroupsByCurrentHandle;
    /** SubscriptionGroupDialog objects carrying the information for
     *  each subscription dialog.
     */
    UtlHashBag mSubscriptionDialogs;
    /** SIP event types that we are currently a SipUserAgent message observer
     *  for.
     */
    UtlHashBag mEventTypes;

    // See sipXtackib/doc/developer/SipSubscribeClient-Locking.txt for
    // how the locks are used.
    // However, in this class we do not need the "object lock", so we use
    // only one OsBSem.
    OsBSem mSetSem;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipSubscribeClient_h_
