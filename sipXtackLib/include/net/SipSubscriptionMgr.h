//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _SipSubscriptionMgr_h_
#define _SipSubscriptionMgr_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES

#include <os/OsDefs.h>
#include <os/OsMsgQ.h>
#include <os/OsMutex.h>
#include <utl/UtlDefs.h>
#include <utl/UtlHashMap.h>
#include <utl/UtlHashBag.h>
#include <net/SipDialogMgr.h>
#include <net/SipSubscribeServerEventHandler.h>
#include <net/SipSubscribeServer.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// FORWARD DECLARATIONS
class SipMessage;
class UtlString;
class SipDialogMgr;

// TYPEDEFS

//! Class for maintaining SUBSCRIBE dialog information in subscription server
/*!
 *
 * \par
 */
class SipSubscriptionMgr
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   /// Describes whether a subscription is to continue or be terminated.
   enum subscriptionChange
   {
      subscriptionContinues,
      subscriptionTerminated,
      // Subscription is to be ended, but subscriber is not to be notified.
      // Used when subscription will be continued by a later incarnation of
      // this server.
      subscriptionTerminatedSilently
   };

/* ============================ CREATORS ================================== */

    //! Default constructor
    SipSubscriptionMgr();

    //! Destructor
    virtual
    ~SipSubscriptionMgr();

/* ============================ MANIPULATORS ============================== */

    //! Asks the SipSubscriptionMgr to initialize itself and sets the address 
    // of the queue to which to send resend messages.  The SipSubscriptionMgr
    // may not be used until initialize() returns.
    virtual void initialize(OsMsgQ* pMsgQ);
   
    //! Add/Update subscription for the given SUBSCRIBE request
    /** The resourceId, eventTypeKey, and eventType are set based on
     *  the subscription.  If the subscription already exists, they
     *  are extracted from the subscription state.  If this is a new
     *  subscription, handler.getKeys() is called to compute them.
     */
    virtual UtlBoolean updateDialogInfo(/// the incoming SUBSCRIBE request
                                        const SipMessage& subscribeRequest,
                                        /// the canonical URI of the resource
                                        UtlString& resourceId,
                                        /// the event type key for the events
                                        UtlString& eventTypeKey,
                                        /// the event type for the events
                                        UtlString& eventType,
                                        /// the dialog handle for the subscription (out)
                                        UtlString& subscribeDialogHandle,
                                        /// TRUE if the subscription is new (out)
                                        UtlBoolean& isNew,
                                        /// TRUE if the subscription is not ongoing (out)
                                        UtlBoolean& isExpired,
                                        /** response for the SUBSCRIBE (out)
                                         *  Any errors will be logged into
                                         *  subscribeResponse.
                                         */
                                        SipMessage& subscribeResponse,
                                        /** Event-specific handler, which provides
                                         *  handler.getKeys().
                                         */
                                        SipSubscribeServerEventHandler& handler
       );

    //! Insert subscription dialog info without checking for the existence of the dialog
    /*! This method blindly inserts dialog information and should only be called from
     *  from the SipPersistentSubscriptionMgr.  It is intended to insert subscription
     *  information into memory from the IMDB.  (Thus, e.g., it does not randomize
     *  subscription times.)
     */
    virtual UtlBoolean insertDialogInfo(/// the incoming SUBSCRIBE request
                                        const SipMessage& subscribeRequest,
                                        /// the canonical URI of the resource
                                        const UtlString& resourceId,
                                        /// the event type key for the events
                                        const UtlString& eventTypeKey,
                                        /// the event type for the events
                                        const UtlString& eventType,
                                        /// the expiration epoch
                                        int expires,
                                        /// the last NOTIFY CSeq
                                        int notifyCSeq,
                                        /// the last version number to be sent
                                        int version,
                                        /// the dialog handle for the subscription (out)
                                        UtlString& subscribeDialogHandle,
                                        /// TRUE if the subscription is new (out)
                                        UtlBoolean& isNew);

    //! Set the subscription dialog information and cseq for the next NOTIFY request
    // Constructs the Subscription-State header using subscriptionStateFormat.
    // (See ::createNotifiesDialogInfo() for details.)
    // Returns resource, event-type, and accept-header about the subscription
    // if requested.
    virtual UtlBoolean getNotifyDialogInfo(const UtlString& subscribeDialogHandle,
                                           SipMessage& notifyRequest,
                                           const char* subscriptionStateFormat,
                                           UtlString* resourceId = NULL,
                                           UtlString* eventTypeKey = NULL,
                                           UtlString* eventType = NULL,
                                           UtlString* acceptHeaderValue = NULL);

    //! Construct a NOTIFY request for each subscription/dialog subscribed to the given resourceId and eventTypeKey
    /*! Allocates a SipMessage* array and allocates a SipMessage and sets the
     * dialog information for the NOTIFY request for each subscription.
     *  \param resourceId - resourceId and eventTypeKey should be appropriate
     *         strings.  If both are NULL, it means to select all subscriptions.
     *  \param eventTypeKey
     *  \param subscriptionStateFormat - a printf() format string that generates
     *         the desired Subscription-State header.  It may have one format
     *         specification for a 'long int' that is the number of seconds
     *         until the subscription expires.  It must generate no more than
     *         100 characters.
     *  \param numNotifiesCreated - number of pointers to NOTIFY requests in
     *         the returned notifyArray
     *  \param acceptHeaderValuesArray - allocated array holding a UtlString*
     *         for each subscription, containing the Accept header values
     *  \param notifyArray - allocated array holding a SipMessage* for
     *         each subscription, containing the generated NOTIFY requests
     */
    virtual void createNotifiesDialogInfo(const char* resourceId,
                                          const char* eventTypeKey,
                                          const char* subscriptionStateFormat,
                                          int& numNotifiesCreated,
                                          UtlString**& acceptHeaderValuesArray,
                                          SipMessage**& notifyArray);

    //! Construct a NOTIFY request for each subscription/dialog subscribed to the given eventType.
    /*! Allocates a SipMessage* array and allocates a SipMessage and sets the
     * dialog information for the NOTIFY request for each subscription.
     *  \param eventType - the event type to select
     *  \param subscriptionStateFormat - a printf() format string that generates
     *         the desired Subscription-State header.  It may have one format
     *         specification for a 'long int' that is the number of seconds
     *         until the subscription expires.  It must generate no more than
     *         100 characters.
     *  \param numNotifiesCreated - number of pointers to NOTIFY requests in
     *         the returned notifyArray
     *  \param acceptHeaderValuesArray - allocated array holding a UtlString*
     *         for each subscription, containing the Accept header values
     *  \param notifyArray - allocated array holding a SipMessage* for
     *         each subscription, containing the generated NOTIFY requests
     *  \param resourceIdArray - allocated array holding a char*
     *         for each subscription, containing the resource Id values.
     *  \param eventTypeKeyArray - allocated array holding a char*
     *         for each subscription, containing the event type key values.
     */
    virtual void createNotifiesDialogInfoEvent(const UtlString& eventType,
                                               const char* subscriptionStateFormat,
                                               int& numNotifiesCreated,
                                               UtlString**& acceptHeaderValuesArray,
                                               SipMessage**& notifyArray,
                                               UtlString**& resourceIdArray,
                                               UtlString**& eventTypeKeyArray);

    //! frees up the notifies created in createNotifiesDialogInfo
    static void freeNotifies(int numNotifies,
                             UtlString** acceptHeaderValues,
                             SipMessage** notifiesArray);

    //! End the dialog for the subscription indicated by the dialog handle
    /*! Finds a matching dialog and expires the subscription if it has
     *  not already expired.
     *  \param dialogHandle - a fully established SIP dialog handle
     *  \param change - describes whether the subscription should
     *         end silently - has no effect in SipSubscriptionMgr.
     *  Returns TRUE if a matching dialog was found regardless of
     *  whether the subscription was already expired or not.
     */
    virtual UtlBoolean endSubscription(const UtlString& dialogHandle,
                                       enum subscriptionChange change);

    //! Remove old subscriptions, ones that expired before the given time.
    virtual void removeOldSubscriptions(long oldEpochTimeSeconds);

    //! Get the current resend interval for a subscription.
    //  Returns 1,000,000 (which is larger than the max-resend value)
    //  if the subscription cannot be found.
    int getNextResendInterval(const UtlString& dialogHandle);

    //! Set the current resend interval for a subscription.
    void setNextResendInterval(const UtlString& dialogHandle,
                               int interval);

    //! Start the resend timer for a dialog, unless it is set to fire sooner.
    void startResendTimer(const UtlString& dialogHandle,
                          int interval);

    //! Get pointer to the dialog handle of a resend message.
    static const UtlString* getHandleOfResendEventMsg(const OsMsg& msg);

    //! Set stored value for the next NOTIFY CSeq and version.
    virtual void setNextNotifyCSeq(const UtlString& dialogHandleString,
                                   int nextLocalCseq,
                                   int version);

   /** Update the values that are saved in the IMDB of the NOTIFY CSeq
    *  (now in notifyRequest) and XML version (as specified by 'version').
     *  \param change - specifies whether the subscription is to
     *         continue or terminate
    */
   virtual void updateVersion(SipMessage& notifyRequest,
                              int version,
                              const UtlString& eventTypeKey);

   /// Perform substitutions in NOTIFY message content.
   /*  This routine retrieves the current content version number for the dialog
    *  of notifyRequest.  It then calls the application's substitution callback
    *  function (setContentInfo), which replaces the version number in the dialog message.
    *  On a successful replacement, it then increments the dialog's version number.
    *  Returns the version number used via the 'version' parameter
    *  and returns the eventTypeKey via the 'eventTypeKey' parameter.
    *  (These are needed to index the IMDB table of persistent subscriptions.)
    */
   virtual void updateNotifyVersion(SipContentVersionCallback setContentInfo,
                                    SipMessage& notifyRequest,
                                    int& version,
                                    UtlString& eventTypeKey);

    //! Set the minimum, default, and maximum subscription times that will be granted.
    UtlBoolean setSubscriptionTimes(int minExpiration,
                                    int mdefaultExpiration,
                                    int maxExpiration);
    /**< Sets the minimum, default, and maximum subscription times
     *   that will be granted by this SipSubscribeServer.
     *   Returns TRUE if the arguments are acceptable, returns FALSE
     *   (and makes no change) if the times are not acceptable.
     */

/* ============================ ACCESSORS ================================= */


    //! Get the dialog manager
    /*! WARNING: the application must be aware of the lifetime of
     *  the dialog manager, as no reference counting is done on
     *  the dialog manager.  The application is responsible for
     *  knowing when the dialog manager will go way.
     */
    SipDialogMgr* getDialogMgr();

    //! Get the minimum, default, and maximum subscription times that will be granted.
    void getSubscriptionTimes(int& minExpiration,
                              int& defaultExpiration,
                              int& maxExpiration);

/* ============================ INQUIRY =================================== */

    //! inquire if the dialog exists
    virtual UtlBoolean dialogExists(UtlString& dialogHandle);

    //! inquire if the dialog has already expired
    virtual UtlBoolean isExpired(UtlString& dialogHandle);

    /** get the next notify body "version" value that is allowed
     *  for a resource (as far as is known by this SipSubscriptionMgr).
     *  If no information is available, returns 0.
     */
    virtual int getNextAllowedVersion(const UtlString& resourceId);

    //! Dump the object's internal state.
    void dumpState();

    /// Initial resend interval, in seconds.
    static int sInitialNextResendInterval;
    /// Maximum resend interval, in seconds.
    static int sMaxNextResendInterval;

    // msgSubType for a ResendEventMsg.
    static int sEventResend;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    //! Copy constructor NOT ALLOWED
    SipSubscriptionMgr(const SipSubscriptionMgr& rSipSubscriptionMgr);

    //! Assignment operator NOT ALLOWED
    SipSubscriptionMgr& operator=(const SipSubscriptionMgr& rhs);

    //! lock for single thread use
    void lock();

    //! unlock for use
    void unlock();

    int mEstablishedDialogCount;
    OsMutex mSubscriptionMgrMutex;
    SipDialogMgr mDialogMgr;
    int mMinExpiration;
    int mDefaultExpiration;
    int mMaxExpiration;
    /// Queue to which to send resend messages.
    OsMsgQ* mpResendMsgQ;

    // Container for the SubscriptionServerState's, which are indexed
    // by dialog handles.
    UtlHashBag mSubscriptionStatesByDialogHandle;

    // Index to subscription states in mSubscriptionStatesByDialogHandle,
    // indexed by the resourceId and eventTypeKey.
    // Members are SubscriptionServerStateIndex's, which are indexed by
    // resourceId concatenated with eventTypeKey.
    UtlHashBag mSubscriptionStateResourceIndex;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipSubscriptionMgr_h_
