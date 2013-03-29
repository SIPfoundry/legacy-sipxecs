//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _SipTransaction_h_
#define _SipTransaction_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES


#include <os/OsDefs.h>
#include <os/OsSocket.h>
#include <os/OsMsgQ.h>
#include <net/BranchId.h>
#include <net/Url.h>
#include <net/SipSrvLookup.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipMessage;
class SipUserAgent;
class SipTransactionList;
class OsEvent;
class OsTimer;

/** SipTransaction correlates requests and responses.
 *
 * CallId  + 's' or 'c' (for server or client) is used as
 * the key for the hash (i.e. stored as the string/data in
 * the parent UtlString.
 */
class SipTransaction : public UtlString {
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

    /*** States of a transaction.
     * @note
     *   See RFC 3261 for a definition and description
     *   of these transaction states and when the transitions occur.
     * @endnote
     */
    enum transactionStates {
        TRANSACTION_UNKNOWN,           ///< not yet set
        TRANSACTION_LOCALLY_INIITATED, ///< No messages sent (usually client)
        TRANSACTION_CALLING,           ///< Request sent
        TRANSACTION_PROCEEDING,        ///< Provisional response received
        TRANSACTION_COMPLETE,          ///< Final response received
        TRANSACTION_CONFIRMED,         ///< ACK recieved for 300-699 response classes
        TRANSACTION_TERMINATED,        ///< Ready to be garbage collected
        NUM_TRANSACTION_STATES         /**< used for array allocation and limit checks.
                                        * New values must be added before this one, and must also
                                        * be added to the string constants in the stateString
                                        * method. */
    };

    /// The relationship of a message to a transaction
    enum messageRelationship {
        MESSAGE_UNKNOWN,          ///< Relationship not yet determined, or error
        MESSAGE_UNRELATED,        ///< A with different Call-Id, To or From
        MESSAGE_SAME_SESSION,     ///< But not part of this TX or related branches
        MESSAGE_DIFFERENT_BRANCH, ///< Same Call-Id, to, from, cseq but different TX
        MESSAGE_REQUEST,          ///< The request to this TX
        MESSAGE_PROVISIONAL,      ///< A provision response to this TX
        MESSAGE_FINAL,            ///< The first final response to this TX
        MESSAGE_NEW_FINAL,        ///< A different final response for this TX
        MESSAGE_CANCEL,           ///< A cancel for this TX
        MESSAGE_CANCEL_RESPONSE,
        MESSAGE_ACK,              ///< An ACK for this non-2xx TX
        MESSAGE_2XX_ACK,          ///< An ACK assocated with this TX (but considered a different TX)
        MESSAGE_2XX_ACK_PROXY,    ///< An ACK assocated with this TX (but considered a different TX), to be sent to next hop
        MESSAGE_DUPLICATE,        ///< A duplicate message for this TX
        NUM_RELATIONSHIPS         /**< used for array allocation and limit checks.
                                   * New values must be added before this one, and must also
                                   * be added to the string constants in the relationshipString
                                   * method. */
    };

    /// The relative priority of a particular response
    enum ResponsePriority {
        RESP_PRI_CHALLENGE,     ///< Highest priority may need to be challenged
        RESP_PRI_CANCEL,        ///< next lower priority
        RESP_PRI_3XX,           ///< next lower priority
        RESP_PRI_4XX,           ///< next lower priority
        RESP_PRI_404,           ///< next lower priority
        RESP_PRI_5XX,           ///< next lower priority
        RESP_PRI_6XX,           ///< next lower priority
        RESP_PRI_LOWEST,        ///< lowest priority
        RESP_PRI_NOMATCH,       ///< Should only happen if no response exists
        NUM_PRIORITIES          /**< used for array allocation and limit checks.
                                   * New values must be added before this one. */
    };

/* ============================ CREATORS ================================== */

    /// Create a new transaction
    SipTransaction(SipMessage* initialMsg = NULL,     ///< message whose state this tracks
                   UtlBoolean isOutgoing = TRUE,          ///< direction
                   UtlBoolean userAgentTransaction = TRUE,///< local initiated
                   BranchId*  parentBranch = NULL         ///< for use in loop detection
                   );
    /**<
     * When this is an out going request, this is a client
     * transaction.  The via header field MUST be added before
     * constructing this transaction as this sets the branch ID.
     */

    virtual
    ~SipTransaction();
    //:Destructor

/* ============================ MANIPULATORS ============================== */

    UtlBoolean handleOutgoing(SipMessage& outgoingMessage,
                              ///< Message to be sent.
                              // (Ownership is not taken by SipTransaction.
                              // message is modified.)
                              SipUserAgent& userAgent,
                              SipTransactionList& transactionList,
                              enum messageRelationship relationship);

    void handleResendEvent(const SipMessage& outgoingMessage,
                           SipUserAgent& userAgent,
                           enum messageRelationship relationship,
                           SipTransactionList& transactionList,
                           // Time until the next resend should happen (msec) (output)
                           int& nextTimeout,
                           SipMessage*& delayedDispatchedMessage);

    void handleExpiresEvent(const SipMessage& outgoingMessage,
                            SipUserAgent& userAgent,
                            enum messageRelationship relationship,
                            SipTransactionList& transactionList,
                            // Time until the next resend should happen (msec) (output)
                            int& nextTimeout,
                            SipMessage*& delayedDispatchedMessage,
                            /// TRUE if Timer C event -- timer can be extended by 101-199 responses
                            bool extendable
       );

    UtlBoolean handleIncoming(SipMessage& incomingMessage,
                             SipUserAgent& userAgent,
                             enum messageRelationship relationship,
                             SipTransactionList& transactionList,
                             SipMessage*& delayedDispatchedMessage);

    void removeTimer(OsTimer* timer);

    void stopTimers();
    void deleteTimers();

/* ============================ Deprecated ============================== */

    void linkChild(SipTransaction& child);

    void unlinkChild(SipTransaction* pChild);

    void toString(UtlString& dumpString,
                  UtlBoolean dumpMessagesAlso);
    //: Serialize the contents of this

    void justDumpTransactionTree(void);

    void dumpTransactionTree(UtlString& dumpstring,
                             UtlBoolean dumpMessagesAlso);
    //: Serialize the contents of all the transactions in this tree
    // The parent is found first and then all children are serialized
    // recursively

    void dumpChildren(UtlString& dumpstring,
                      UtlBoolean dumpMessagesAlso);
    //: Serialize the contents of all the child transactions to this transaction
    // All children are serialized recursively


/* ============================ ACCESSORS ================================= */

    static const char* stateString(enum transactionStates state);

    static const char* relationshipString(enum messageRelationship relationship);

    static void buildHash(const SipMessage& message,
                          UtlBoolean isOutgoing,
                          UtlString& hash);

    SipTransaction* getTopMostParent() const;

    void getCallId(UtlString& callId) const;

    enum transactionStates getState() const;

    long getStartTime() const;

    long getTimeStamp() const;

    void touch();
    void touchBelow(int newDate);

    SipMessage* getRequest();

    SipMessage* getLastProvisionalResponse();

    SipMessage* getLastFinalResponse();

    void cancel(SipUserAgent& userAgent,
                SipTransactionList& transactionList);
    //: cancel any outstanding client transactions (recursively on children)

    void markBusy();

    void markAvailable();

    void notifyWhenAvailable(OsEvent* availableEvent);
    //: The given event is signaled when this transaction is not busy

    void signalNextAvailable();

    void signalAllAvailable();

/* ============================ INQUIRY =================================== */

    UtlBoolean isServerTransaction() const;
    //: Inquire if this transaction is a server as opposed to a client transaction

    //! Inquiry as to whether this transaction is a recursed DNS SRV child
    UtlBoolean isDnsSrvChild() const;

    UtlBoolean isUaTransaction() const;
    //: Inquire if transaction is UA based or proxy
    // Note this is different than server vs client transaction

    UtlBoolean isChildSerial();
    //: Inquire as to whether child transaction will be serial or all parallel searched
    // If all immediate child transactions have the same
    // Q value FALSE is returned

    UtlBoolean isEarlyDialogWithMedia();
    //: Tests to see if this is an existing early dialog with early media
    // If transaction has not yet been completed and there was early media
    // (determined by the presence of SDP in a provisional response

    UtlBoolean isChildEarlyDialogWithMedia();
    //: Are any of the children in an early dialog with media

    UtlBoolean isMethod(const char* methodToMatch) const;
    //: see if this tranaction is of the given method type

    enum messageRelationship whatRelation(const SipMessage& message,
                                          UtlBoolean isOutgoing) const;
    //: Check if the given message is part of this transaction.
    //< returns enum messageRelationship values.   :
    //<   Note: Per rfc-3261, if the proper via branch-id prefix is present, we only try to match branch ids.
    //<         If the prefix is not present, we do all the work needed to compare to- and from- tags explicitly.
    //<
    //< - MESSAGE_UNRELATED ---------- if match fails for any of:
    //<                                  {call id, via branch-id (if no 3261 prefix must match from-tag AND to-tag)}
    //< - MESSAGE_SAME_SESSION ------- match ALL of {call id, via branch-id (if no 3261 prefix must match from-tag AND to-tag)} but NOT Cseq
    //< - MESSAGE_DIFFERENT_BRANCH --- match ALL of {call id, via branch-id (if no 3261 prefix must match from-tag AND to-tag), Cseq}
    //<                                   BUT isServerTransaction() value of message doesn't match the transaction's value
    //<                                     (one is a client transaction (is outbound request or inbound response)
    //                                        and the other is a server transaction (is inbound request or outbound response) )
    //<                            --- match ALL of {call id, via branch-id (if no 3261 prefix must match from-tag AND to-tag), Cseq}
    //<                                   BUT it is NOT true that-
    //<                                  - the branch ids of this message and this transaction match
    //<                                  - OR the message branch id matches the parent's branch AND the RequestURIs match
    //<                                  - OR it is an ACK request for this client UA(no more vias) which had a 2xx final response
    //<                                  - OR it is an ACK request for this server which had a 2xx final response
    //<                                  - OR it is an ACK request which had a 2xx final response, to be forwarded by proxy
    //<                                  - OR it is a CANCEL request for this client UA(no more vias)
    //<
    //< The remaining responses are only set AFTER checking that none of the cases above occur!
    //< - MESSAGE_REQUEST ---------------- is a request other than ACK or CANCEl with no previous request
    //< - MESSAGE_PROVISIONAL ------------ is a provisional response (1xx)
    //< - MESSAGE_FINAL ------------------ is a first response to any request
    //< - MESSAGE_NEW_FINAL -------------- is a second (or later) response to an invite request with identical status but different to-tag
    //<                                        (different fork)
    //<                              ----- is a second (or later) response to any request but with a different status code
    //< - MESSAGE_CANCEL ----------------- is CANCEL request, regardless whether there is a previous request of any kind
    //< - MESSAGE_CANCEL_RESPONSE -------- is a response to CANCEL request
    //< - MESSAGE_ACK -------------------- is ACK request for transaction with 3xx or greater (non-2xx) response
    //<                              ----- is ACK request for transaction with no final response
    //<                              ----- is ACK request for transaction with no previous request
    //< - MESSAGE_2XX_ACK ---------------- is ACK request for "server" transaction with 2xx response
    //< - MESSAGE_2XX_ACK_PROXY ---------- is ACK request for "proxy" transaction with 2xx response
    //< - MESSAGE_DUPLICATE -------------- is a second (or later) response to an invite request with identical to-tag and status (same fork)
    //<                              ----- is a second (or later) response to other-than-invite with identical status
    //<                              ----- is a second (or later) request with identical method
    //<                              ----- is a second (or later) request with different method but not ACK or CANCEL


    UtlBoolean isBusy();
    //: is this transaction being used (e.g. locked)

    //UtlBoolean isDuplicateMessage(SipMessage& message,
    //                             UtlBoolean checkIfTransactionMatches = TRUE);
    //: Check to see if this request or response has already been received by this transaction

    UtlBoolean isUriChild(Url& uri);
    // Does this URI already exist as an immediate child to this transaction
    // Search through each of the children and see if the child
    // transaction's URI matches.

    UtlBoolean isUriRecursed(Url& uri);
    // Has this URI been recursed anywhere in this transaction tree already
    // Start looking at the parent

    UtlBoolean isUriRecursedChildren(UtlString& uriString);
    // Has this URI been recursed anywhere at or below in this transaction tree already
    // Look at or below the current transaction in the transaction tree

    void setCancelReasonValue(const char* protocol,
                              int responseCode,
                              const char* reasonText = NULL);
    // Set the data to be used to generate a Reason header for any CANCEL
    // generated for any child transaction.

    UtlSList& childTransactions();

    bool isMarkedForDeletion() const;
    void markForDeletion();
/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    void handleChildTimeoutEvent(SipTransaction& child,
                                 const SipMessage& outgoingMessage,
                                 SipUserAgent& userAgent,
                                 enum messageRelationship relationship,
                                 SipTransactionList& transactionList,
                                 // Time until the next resend should happen (msec) (output)
                                 int& nextTimeout,
                                 SipMessage*& delayedDispatchedMessage);
    //: tells the parent transaction the result of the timeout event

    UtlBoolean handleChildIncoming(//SipTransaction& child,
                                  SipMessage& incomingMessage,
                                  SipUserAgent& userAgent,
                                  enum messageRelationship relationship,
                                  SipTransactionList& transactionList,
                                  UtlBoolean childSaysShouldDispatch,
                                  SipMessage*& delayedDispatchedMessage);
    //: Tells the parent transaction the result of the incoming message
    //! returns: TRUE/FALSE as to whether the message should be dispatched to applications

    UtlBoolean startSequentialSearch(SipUserAgent& userAgent,
                                    SipTransactionList& transactionList);
    //: Checks to see if a final response can be sent or if sequential search should be started

    UtlBoolean recurseChildren(SipUserAgent& userAgent,
                              SipTransactionList& transactionList);
    //: Starts search on any immediate children of the highest unpursued Q value

    UtlBoolean recurseDnsSrvChildren(SipUserAgent& userAgent,
                              SipTransactionList& transactionList,
                              SipMessage* pRequest = 0);
    //: Starts search on any immediate DNS SRV children of the highest unpursued Q value

    /// Copy unique realms from any proxy challenges in the response into realmList
    void getChallengeRealms(const SipMessage& response, UtlSList& realmList);
    ///< This is for use only within findBestResponse, to filter duplicate realms

    UtlBoolean findBestResponse(SipMessage& bestResponse);
    // Finds the best final response to return the the server transaction

    UtlBoolean findBestChildResponse(SipMessage& bestResponse, int responseFoundCount);

    enum ResponsePriority findRespPriority(int responseCode);


    enum messageRelationship addResponse(SipMessage*& response,
                                         UtlBoolean isOutGoing,
                                         enum messageRelationship relationship = MESSAGE_UNKNOWN);
    //: Adds the provisional or final response to the transaction

    void cancelChildren(SipUserAgent& userAgent,
                        SipTransactionList& transactionList);
    //: Cancels children transactions on a server transaction

    void doMarkBusy(int markValue);

    OsSocket::IpProtocolSocketType getPreferredProtocol();
    //: Determine best protocol, based on message size

    

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    SipTransaction(const SipTransaction& rSipTransaction);
    //:Copy constructor (disabled)
    SipTransaction& operator=(const SipTransaction& rhs);
    //:Assignment operator (disabled)

    // Resend a message when the resent timer has expired.  Also schedules
    // the next resend.
    UtlBoolean doResend(SipMessage& resendMessage,
                        SipUserAgent& userAgent,
                        // Time until the next resend should happen (msec) (output)
                        int& nextTimeoutMs);

    // Do the first transmission of a message, including for requests,
    // scheduling the timeout that triggers the first send.
    UtlBoolean doFirstSend(SipMessage& message,
                          enum messageRelationship relationship,
                          SipUserAgent& userAgent,
                          UtlString& toAddress,
                          int& port,
                          OsSocket::IpProtocolSocketType& toProtocol);

    void prepareRequestForSend(SipMessage& request,
                               SipUserAgent& userAgent,
                               UtlBoolean& addressRequiresDnsSrvLookup,
                               UtlString& toAddress,
                               int& port,
                               OsSocket::IpProtocolSocketType& toProtocol);

    // CallId  + 's' or 'c' (for server or client) is used as
    // the key for the hash (i.e. stored as the string/data in
    // the parent UtlString
    UtlString mCallId;
    BranchId* mpBranchId;
    UtlString mRequestUri;
    Url mFromField;
    Url mToField;
    UtlString mRequestMethod;
    int mCseq;
    UtlBoolean mIsServerTransaction; ///< TRUE = server, FALSE = client
    UtlBoolean mIsUaTransaction;     ///< UA or proxy transaction, TRUE unless created by proxy SipUserAgent or
                                     ///<   if SUA::send message already has a VIA header
    UtlString mCancelReasonValue;    ///< Value of Reason header to be sent in CANCEL of any children (or null)

    // Address and transport that have been established for this transaction.
    UtlString mSendToAddress;
    int mSendToPort;
    OsSocket::IpProtocolSocketType mSendToProtocol;

    server_t* mpDnsDestinations;        ///< list obtained from DNS server, can contain 0 valid destinations
    SipMessage* mpRequest;
    SipMessage* mpLastProvisionalResponse;
    SipMessage* mpLastFinalResponse;
    SipMessage* mpAck;
    SipMessage* mpCancel;
    SipMessage* mpCancelResponse;
    SipTransaction* mpParentTransaction;
    UtlSList mChildTransactions;
    long mTransactionCreateTime;         ///< When this thing was created
    long mTransactionStartTime;          /**<  When the request was sent/received
                                          * i.e. went to TRANSACTION_CALLING state */
    long mTimeStamp;                     ///< When this was last used
    enum transactionStates mTransactionState;
    UtlBoolean mDispatchedFinalResponse; ///< For UA recursion
    UtlBoolean mProvisionalSdp;          ///< early media
    UtlSList mTimers;                    /**< A list of all outstanding timers
                                          *   started by this transaction. */
    /**< SipTransaction Timer Usage
      * In this comment, "transaction" refers to the SipTransaction object in the code, not an RFC3261 transaction.
      * Timer objects contain the corresponding SipMessage to be used when events are processed.
      *
      * Two timers are possible -
      *
      * 1- transaction resend timer
      * --- posts TRANSACTION_RESEND event on timeout
      * --- initially set in doFirstSend, can be set again in handleResendEvent
      * --- initial value is set from SipUserAgent variables, default is  SIP_DEFAULT_RTT, can be overridden in SUA::+
      * --- for resend, value is set according to RFC3261 rules
      *
      * --- TRANSACTION_RESEND Timeout behavior ---
      * ------  Resend message according to RFC3261 rules.
      *
      * 2- transaction expires timer
      * --- posts TRANSACTION_EXPIRATION event
      * --- can be set in doFirstSend or recurseDnsSrvChildren
      * --- default values are set in SipUserAgent::+, can be overridden
      * --- more complicated than resend timer
      *
      * --- All client transactions for request messages fall into two classes:
      *
      * --- 1. Transactions that are created to send a request to a request-URI.  These transactions execute the RFC 3263 process
      * --- and create a class 2 transaction child for each address/port that is a destination for the request-URI.  Class 1 transactions
      * --- carry the timer(s) that enforce "request expiration", that is, the Expires header and Timer C for INVITEs.
      * --- These transactions' timers are set in recurseDnsSrvChildren.
      *
      * --- 2. Transactions that are created to send a request to an address/port.  These transactions are children of a class 1
      * --- transaction.  These transactions carry the timer that watches whether the address/port is responding at all, or
      * --- whether the request should be sent to an alternative address/port (by a sibling class 2 transaction).  These transactions'
      * --- timers are set in doFirstSend.
      *
      * --- Class 2 transactions, whose timers are set in doFirstSend:
      * ----- Due to various transaction state variables, these timers are generally ignored once any response is received.
      * ---------- see mIsDnsSrvChild and the transaction state value
      * ------ Only set when sending request and this is not a server transaction
      * ------ in all cases, the max value is SipUserAgent::mDefaultExpiresSeconds
      * ---------- default is DEFAULT_SIP_TRANSACTION_EXPIRES (180s), can override, see proxy(), SIPX_PROXY_DEFAULT_EXPIRES
      * ------ smaller values are set based on SipTransaction variables:
      * --------  for a serial child transaction resulting from DNS lookup, value is set to mDnsSrvTimeout
      * -------------- default is (4s), can override, see proxy(), SIPX_PROXY_DNSSRV_TIMEOUT
      * --------  for any other transaction when message has an Expires header, value is set to the Expires header value
      * --------  for serial child transaction and no Expires header, value is set to mDefaultSerialExpiresSeconds
      * -------------- default is DEFAULT_SIP_SERIAL_EXPIRES (20s), can override, see proxy(), SIPX_PROXY_DEFAULT_SERIAL_EXPIRES
      *
      * --- Class 1 transactions, whose timers are set in recurseDnsSrvChildren:
      * ----- These transactions can have two timers.  One is a "Timer C" timer which will be extended if a 101-199 response has
      * ----- been received since the timer was last set/extended.  The other is absolute and is not affected by provisional
      * ----- responses.
      * ------ for transactions tied to INVITE messages, the Timer C timer is SipUserAgent::mDefaultExpiresSeconds
      * ---------- mDefaultExpiresSeconds is initialized as DEFAULT_SIP_TRANSACTION_EXPIRES (180s);
      * ---------- in sipXproxy, it is changed to the config value SIPX_PROXY_DEFAULT_EXPIRES (if provided)
      * ------ non-INVITE transactions do not get a Timer C timer.
      * ------ for all transactions, the absolute timer is set if any of the following provides an expiration time (the timer
      * ------ is set to the minimum of them):
      * ---------- for transactions tied to non-INVITE messages, SipUserAgent::mTransactionStateTimeoutMs/1000
      * ------------ default is (8s), no override is provided in sipXproxy
      * ---------- for any INVITE transaction, when message has an Expires header, the Expires header value
      * ---------- for serial child transaction and no Expires header in the associated request, SipUserAgent::mDefaultSerialExpiresSeconds
      * ------------ mDefaultSerialExpiresSeconds is initialized as DEFAULT_SIP_SERIAL_EXPIRES (20s);
      * ------------ in sipXproxy, it is changed to the config value SIPX_PROXY_DEFAULT_SERIAL_EXPIRES (if provided)
      *
      * --- TRANSACTION_EXPIRATION/TRANSACTION_EXPIRATION_TIMER_C event behavior ---
      * ------ Ignore timeout if attached SipMessage is a response.
      * ------ Do not send CANCEL if:
      * ---------- tx is mIsDnsChild and a final or provisional response has occurred
      * ---------- tx is in a serial search tree and has received provisional SDP
      * ---------- tx state is COMPLETED or CONFIRMED (transaction has finished its own work)
      * ------ Do not send CANCEL and extend timer if:
      * ---------- None of the previous cases are true
      * ---------- AND this is a TRANSACTION_EXPIRATION_TIMER_C event
      * ---------- AND a provisional response > 100 has been received since the previous TRANSACTION_EXPIRATION_TIMER_C expiration.
      * ------ After making CANCEL decision (and sending CANCEL if required), find the top of the transaction tree.
      * ---------- Step through the tree, if any transactions have more to do, nothing further is done.
      * ---------- If all transactions have reached an end state, find the best response and send it if needed.
      * */

    // Recursion members
    UtlBoolean mIsCanceled;
    UtlBoolean mIsRecursing;   ///< TRUE if any braches have not be pursued
    UtlBoolean mIsDnsSrvChild; ///< This Child Transaction pursues one of the Server_T objects of the parent CT
                               ///  This transaction is prepared to actually a send a Sip message
    double mQvalue;            ///< Recurse order (q-value).  Larger values are processed first; equal values are recursed in parallel.
    int mExpires;              ///< The value of the Expires header of the initial INVITE message, or -1 if none.
                               //   (initialMsg parameter of SipTransaction::().)
                               //   Maximum time (seconds) to wait for a final response
    UtlBoolean mIsBusy;
    /** TRUE if a provisional response 101-199 has been processed since the last time
     *  the Timer C expiration timer was set/extended.
     *  Set by processing a response 101-199; when expiration timer fires.
     *  Upon a TRANSACTION_EXPIRATION_TIMER_C event, if this flag is set, the
     *  CANCEL is not done and this flag is cleared.
     *  TRANSACTION_EXPIRATION events are not affected by this flag; they always
     *  cause CANCEL.
     */
    UtlBoolean mProvoExtendsTimer;
    UtlString mBusyTaskName;
    UtlSList* mWaitingList;    /**< Events waiting until this is available
                                * Note only a parent tx should have a waiting list */
    bool _markedForDeletion;
    public: static UtlBoolean SendTryingForNist;

public:
  static UtlBoolean enableTcpResend;
};

/* ============================ INLINE METHODS ============================ */

inline UtlSList& SipTransaction::childTransactions()
{
  return mChildTransactions;
}

inline bool SipTransaction::isMarkedForDeletion() const
{
  return _markedForDeletion;
}

inline void SipTransaction::markForDeletion()
{
  _markedForDeletion = true;
}

#endif // _SipTransaction_h_
