//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _SipUserAgent_h_
#define _SipUserAgent_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <utl/UtlHashBag.h>
#include <os/OsServerTask.h>
#include <net/SipUserAgentBase.h>
#include <net/SipMessage.h>
#include <net/SipMessageEvent.h>
#include <net/SipTransactionList.h>
#include <net/SipUdpServer.h>
#include <os/OsQueuedEvent.h>
#include <net/SipOutputProcessor.h>

// DEFINES
#define SIP_DEFAULT_RTT     100 // Default T1 value (RFC 3261), in msec.
                                // Intended to be estimate of RTT of network.
#define SIP_MINIMUM_RTT     10  // Minimum T1 value allowed, in msec.
#define SIP_MAX_PORT_RANGE  10  // If a port is in use and the sip user agent
                                // is created with bUseNextAvailablePort set to
                                // true, this is the number of sequential ports
                                // to try.

// proxy, registrar, etc. UDP socket buffer size
#define SIPUA_DEFAULT_SERVER_UDP_BUFFER_SIZE 1000000

// proxy, registrar, etc. OsServerTask OsMsg queue size
#define SIPUA_DEFAULT_SERVER_OSMSG_QUEUE_SIZE 10000

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class OsConfigDb;
class OsQueuedEvent;
class OsTimer;
class SipSession;
class SipTcpServer;
class SipTlsServer;
class SipLineMgr;

//! Transaction and Transport manager for SIP stack
/*! Note SipUserAgent is perhaps not the best name for this class.
 * It is really the transaction and transport layers of
 * of a SIP stack which may be used in a User Agent or
 * server context.
 *
 * \par Using SipUserAgent
 *
 * The SipUserAgent is the primary interface for incoming
 * and outgoing SIP messages.  It handles all of the
 * reliability, resending, canceling and IP layer protocol
 * details.  Application developers that wish to send or
 * recieve SIP messages can use SipUserAgent without having
 * to worry about the transaction and transport details.
 * \par
 * Applications send SIP messages via the send() method.  Incoming
 * messages are received on an OsMsgQ to be handled by the
 * application.  The message queue must be registered with
 * the SipUserAgent via addMessageObserver() before they
 * can receive incoming messages.  Alternatively applications
 * that are only interested in a specific transaction can
 * pass the OsMsgQ as part of the send() method invocation.
 * Messages which fail to be sent due to transport problems
 * will be communicated back in one of three different ways:
 * -# send() returns a failure indication synchronously (e.g.
 *        due to unresolvable DNS name)
 * -# the send fails asynchronously (e.g. ICMP error) and puts
 *        a message in the OsMsgQ with a transport failure indication
 * -# the send succeeds, but the transaction fails or times out
 *        due to the lack of completion or responses to a request. In
 *        this case a message is put in the OsMsgQ with a transport
 *        failure indication.
 *
 * In the asynchronous cases where a message is put in the message
 * queue to indicate the failure, the original SIP message is attached
 * so that the application can determine which SIP message send failed.
 *
 * \par Internal Implementation Overview
 *
 * All state information will be contained in transactions
 * and/or the messages contained by a transaction.  The transaction
 * will keep track of what protocols to use and when as well as
 * when to schedule timers. send() will no longer be used for
 * resending.  It will only be used for the first time send.
 *
 * The flow for outgoing messages is something like the
 * following:
 * - 1) An application calls send() to send a SIP request or response
 * - 2) send() for requests: constructs a client transaction,
 *    for: responses finds an existing server transaction
 * - 3) send() asks the transaction how (i.e. protocol) and whether to
 *    send the message
 * - 4) send() calls the appropriate transport sender (e.g. sendUdp, sendTcp)
 * - 5A) If the send succeeded: send() asks the transaction to schedule
 *     a timeout for resending the message or failing the transaction
 * - 5B) If the send failed: send() asks the transaction whether to:
 *     - a) dispatch the transport error and mark the transaction state
 *         to indicate the failure.
 *     - b) try another protocol and repeat starting at step 4 above
 *
 * Timeouts are handled by handle message in the following flow:
 * - 1) The timeout expires and posts a SipMessageEvent on the
 *    SipUserAgent's queue which invokes handleMessage
 * - 2) handleMessage() finds the transaction for the timeout
 * - 3) handleMessage() asks the transaction if it should resend
 *    the message.
 * - 4A) The message may not be resent for one of a number of reasons:
 *     - a) a response or ACK was recieved
 *     - b) the transaction was canceled
 *     - c) It is time to give up and fail the transaction
 *     In the latter case an error must be dispatched to the
 *     application.  In the other cases nothing is done.
 * - 4B) If the message is to be resent, the transaction tells
 *     which protocol sender to use (as in steps 4 & 5 above
 *     for outbound messages).
 *
 * Inbound messages are still sent via dispatch.  The flow is now a
 * little different due to the use of transaction objects
 * - 1) dispatch() finds a matching transaction for non-transport error
 *    messages.
 * - 2A) If the message is a duplicate, it is dropped on the floor
 * - 2B) If the message is a new request, a server transaction is
 *     created
 * - 2C) If the message is a new response, for an exising client
 *     transaction, it is sent to the interested observers, with
 *     the original request attached.
 * - 2D) If the message is a response with no existing transaction,
 *     it is dropped on the floor ??I think it should be for UAC
 *     transactions anyway.  Proxy client response may need to be
 *     sent to observers??
 * - 3) If the message was not dropped on the floor by step 2A or 2D,
 *    the message is sent to the interested observers
 */

class SipUserAgent : public SipUserAgentBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

    friend class SipTransaction;
    friend class SipUdpServer;
    friend int SipUdpServer::run(void* runArg);

    enum EventSubTypes
    {
        UNSPECIFIED = 0,
        SHUTDOWN_MESSAGE = 10,
        SHUTDOWN_MESSAGE_EVENT
    };

    enum OptionsRequestHandlePref
    {
        HANDLE_OPTIONS_AUTOMATICALLY,
        PASS_OPTIONS_TO_CONSUMER
    };

/* ============================ CREATORS ================================== */

    //! Constructor
    /*! Sets up listeners on the defined ports and IP layer
     * protocols for incoming SIP messages.
     * \param sipTcpPort - port to listen on for SIP TCP messages.
     *        Specify PORT_DEFAULT to automatically select a port, or
     *        PORT_NONE to disable.
     * \param sipUdpPort - port to listen on for SIP UDP messages.
     *        Specify PORT_DEFAULT to automatically select a port, or
     *        PORT_NONE to disable.
     * \param sipTlsPort - port to listen on for SIP TLS messages.
     *        Specify PORT_DEFAULT to automatically select a port, or
     *        PORT_NONE to disable.
     * \param publicAddress - use this address in Via and Contact headers
     *        instead of the actual adress.  This is useful for address
     *        spoofing in a UA when behind a NAT
     * \param defaultUser - default user ID to use in Contacts which get
     *        inserted when missing on a UA.
     * \param defaultSipAddress - deprecated
     * \param sipProxyServers - server to which non-routed requests should
     *        be sent for next hop before going to the final destination
     * \param sipDirectoryServers - deprecated
     * \param sipRegistryServers - deprecated
     * \param authenticationScheme - authentication scheme to use when
     *        challenging on behalf of the UA (i.e. 401).  Valid values
     *        are NONE and DIGEST.
     * \param authenicateRealm - The authentication realm to use when
     *        sending 401 challenges.
     * \param authenticateDb - the authentication DB to use when
     *        authenticating incoming requests on behalf of the UA
     *        application (as a result of locally generated 401 challenges
     * \param authorizeUserIds - depricated by the SipLineMgr
     * \param authorizePasswords - depricated by the SipLineMgr
     * \param lineMgr - SipLineMgr object which is container for user
     *        definitions and their credentials.  This is used to
     *        authenticate incoming requests to the UA application.
     * \param sipFirstResendTimeout - T1 in RFC 3261, in msec
     *        Time from first send of a message (using UDP) to first resend.
     *        0 defaults to SIP_DEFAULT_RTT (defined above)
     *        minimum allowed value is SIP_MINIMUM_RTT (defined above)
     * \param defaultToUaTransactions - default transactions to be
     *        UA or PROXY.  TRUE means that this is a UA and associated
     *        validation should occur.  FALSE means that this is a
     *        PROXY and that minimal validation should occur.
     * \param readBufferSize - the default IP socket buffer size
     *        to use for the listener sockets.
     * \param queueSize - Size of the OsMsgQ to use for the queues
     *        internal to the SipUserAgent and subsystems.
     * \param bUseNextAvailablePort - When setting up the sip user
     *        agent using the designated sipTcp, sipUdp, and sipTls
     *        ports, select the next available port if the supplied
     *        port is busy.  If enable, this will attempt at most
     *        10 sequential ports.
     * \param doUaMessageChecks - check the acceptability of method,
     *        extensions, and encoding.  The default is TRUE; it may
     *        be set to false in applications such as a redirect server
     *        that will never actually send a 2xx response, so the
     *        checks might cause errors that the application should
     *        never generate.
     * \param forceSymmetricSignaling - impose that the same local
     *        IP:Port be used for sending and receiving SIP signaling
     * \param howTohandleOptionsRequest - Incoming OPTIONS requests can
     *        either be handled automatically by the SipUserAgent class
     *        or it can be passed up to the consumer. The default is to
     *        handle it locally.
     */
    SipUserAgent(int sipTcpPort = SIP_PORT,
                int sipUdpPort = SIP_PORT,
                int sipTlsPort = SIP_PORT+1,
                const char* publicAddress = NULL,
                const char* defaultUser = NULL,
                const char* defaultSipAddress = NULL,
                const char* sipProxyServers = NULL,
                const char* sipDirectoryServers = NULL,
                const char* sipRegistryServers = NULL,
                const char* authenticationScheme = NULL,
                const char* authenicateRealm = NULL,
                OsConfigDb* authenticateDb = NULL,
                OsConfigDb* authorizeUserIds = NULL,
                OsConfigDb* authorizePasswords = NULL,
                SipLineMgr* lineMgr = NULL,
                int sipFirstResendTimeout = SIP_DEFAULT_RTT,
                UtlBoolean defaultToUaTransactions = TRUE,
                int readBufferSize = -1,
                int queueSize = OsServerTask::DEF_MAX_MSGS,
                UtlBoolean bUseNextAvailablePort = FALSE,
                UtlBoolean doUaMessageChecks = TRUE,
                UtlBoolean forceSymmetricSignaling = TRUE,
                OptionsRequestHandlePref howTohandleOptionsRequest = HANDLE_OPTIONS_AUTOMATICALLY
                 );

    //! Destructor
    virtual
    ~SipUserAgent();
    // You must call ::shutdown(TRUE) before calling the destructor.

/* ============================ MANIPULATORS ============================== */

    //! Cleanly shuts down SipUserAgent.
    /*! This method can block until the shutdown is complete, or it can be
     * non-blocking.  When complete, the SipUserAgent can be deleted.
     * May be called repeatedly.
     * \sa isShutdownDone
     *
     * \param blockingShutdown - TRUE if this method should block until the
     * shutdown is complete, FALSE if this method should be non-blocking.
     */
    void shutdown(UtlBoolean blockingShutdown = TRUE);

    //! Enable stun lookups for UDP signaling.  Use a NULL szStunServer to
    //! disable
    virtual void enableStun(const char* szStunServer,
                            int refreshPeriodInSecs,
                            int stunOptions,
                            OsNotification* pNotification = NULL,
                            const char* szIp = NULL) ;

    //! For internal use only
    virtual UtlBoolean handleMessage(OsMsg& eventMessage);

    //! Deprecated (Add a SIP message recipient)
    virtual void addMessageConsumer(OsServerTask* messageConsumer);

    //! Add a SIP message observer for receiving SIP messages meeting the
    //! given filter criteria
    /*! SIP messages will be added to the \a messageQueue if they meet
     * the given filter criteria.
     *
     * \param messageQueue - the queue on which an SipMessageEvent is
     *        dispatched
     * \param sipMethod - the specific method type of the requests or
     *        responses to be observed.  NULL or a null string indicates
     *        all methods.
     * \param wantRequests - want to observe SIP requests
     * \param wantResponses - want to observe SIP responses
     * \param wantIncoming - want to observe SIP messages originating
     *        from the network.
     * \param wantOutGoing - (not implemented) want to observe SIP
     *        messages originating from locally.
     * \param eventName - want to observe SUBSCRIBE or NOTIFY requests
     *        having the given event type
     *        eventName is ignored when matching responses.
     * \param pSession - want to observe SIP message with the
     *        specified session (call-id, to url, from url)
     * \param observerData - data to be attached to SIP messages queued
     *        on the observer
     */
    void addMessageObserver(OsMsgQ& messageQueue,
                              const char* sipMethod = NULL,
                              UtlBoolean wantRequests = TRUE,
                              UtlBoolean wantResponses = TRUE,
                              UtlBoolean wantIncoming = TRUE,
                              UtlBoolean wantOutGoing = FALSE,
                              const char* eventName = NULL,
                              SipSession* pSession = NULL,
                              void* observerData = NULL);


    //! Removes all SIP message observers for the given message/queue
    //! observer
    /*! This undoes what addMessageObserver() does.
     * \param messageQueue - All observers dispatching to this message queue
     *         will be removed if the pObserverData is NULL or matches.
     * \param pObserverData - If null, all observers that match the message
     *        queue will be removed.  Otherwise, only observers that match
     *        both the message queue and observer data will be removed.
     * \return TRUE if one or more observers are removed otherwise FALSE.
     */
    UtlBoolean removeMessageObserver(OsMsgQ& messageQueue,
                                    void* pObserverData = NULL);

    //! Adds a new SipOutputProcessor to the list of processors
    //! that will get notified when an outgoing SIP message is about
    //! to be sent.  Upon successful addtion of a processor, it will
    //! start receiving notifications from the SIP stack via its
    //! SipOutputProcessor::handleOutputMessge() method when SIP
    //! messages are sent out.
    //! Please refer to comments in SipOutputProcessor.h for information
    //! about threading and blocking considerations.
    //!
    //! \param pProcessor - Pointer to SipOutputProcessor-derived observer.
    //!
    //! Notes on how addSipOutputProcessor differs from addMessageObserver:
    //!        The SIP output processor differs in many ways from the message
    //!        observers that can be registered via addMessageObserver():
    //!        #1- Output processor sees messages in the outgoing direction only
    //!            just as they are about to be sent while the current
    //!            implementation of the 'message observer' only sees
    //!            incoming messages.
    //!        #2- 'message observer' only gives application access to a
    //!            limited subset of responses while the output processor
    //!            allows an application to see all responses
    //!        #3- For the subset of responses that can be seen by an
    //!            application via the 'message observer', it cannot
    //!            effectively modify them as they are already sent
    //!            to their destination when the application sees them.
    //!            In contrast, the output processor allows modification
    //!            of all messages being sent.
    //!        #4- 'message observers' get notified of incoming messages
    //!            on their message loop thread which makes processing of
    //!            observed messages thread-safe.  On the other hand, the
    //!            output processor feature generates callbacks in the context
    //!            of the thread that is making the send() request to the
    //!            SipUserAgent instance being monitored.  This means
    //!            that applications processing 'output processor'
    //!            callbacks must provide their own thread safety mechanisms.
    //!       #5 - When using unreliable transport (UDP), the Output processor
    //!            generates a notification for each retransmission of the message.
    //!            The 'message observer' sits above the retransmission
    //!            layer and as such will send a single notification per
    //!            observable message.
    //!
    void addSipOutputProcessor( SipOutputProcessor *pProcessor );

    //! Removes a previously added SipOutputProcessor from the list of
    //! processors that will get notified when an outgoing SIP message is
    //! about to be sent.
    UtlBoolean removeSipOutputProcessor( SipOutputProcessor *pProcessor );

    // See comments in SipUserAgentBase.h
    virtual void executeAllSipOutputProcessors( SipMessage& message,
                                                const char* address,
                                                int port );

    //! Send a SIP message over the net

    /*! This method sends the SIP message as dictated by policy and
     * the address specified in the message.  This method understands
     * SIP transactions and will resolve addresses and re-send messages
     * as necessary.
     * Most applications will register a OsMsgQ via
     * addMessageObserver() prior to calling send and so should call
     * send with only one argument.
     * \note If the application does register the message queue via
     * addMessageObserver() it should not pass the message queue as
     * an argument to send or it will receive multiple copies of the
     * incoming responses.
     * \param message - the sip message to be sent
     * \param responseListener - the optional queue on which to place
     *        SipMessageEvents containing SIP responses from the same
     *        transaction as the request sent in message
     * \param responseListenerData - optional data to be passed back
     *        with responses
     * Returns true if the message was accepted for sending.  If true
     *        is returned, any send failure will be reported asynchronously.
     */
    virtual UtlBoolean send(SipMessage& message,
                            OsMsgQ* responseListener = NULL,
                            void* responseListenerData = NULL);

    //! Dispatch the SIP message to the message consumer(s)
    /*! This is typically only used by the SipUserAgent and its sub-system.
     * So unless you know what you are doing you should not be using this
     * method. All incoming SIP message need to be dispatched via the
     * user agent server so that it can provide the reliablity for UDP
     * (i.e. resend requests when no response is received)
     * \param messageType - is as define by SipMessageEvent::MessageStatusTypes
     *        APPLICATION type are normal incoming messages
     *        TRANSPORT_ERROR type are notification of failures to
     *        send messages
     */
    // Takes ownership of '*message'.
    virtual void dispatch(SipMessage* message,
                          int messageType = SipMessageEvent::APPLICATION);

    void allowMethod(const char* methodName, const bool bAllow = true);

    void allowExtension(const char* extension);

    void getSupportedExtensions(UtlString& extensionsString);

    //! Add the specified Require header "extension" for clients.
    void requireExtension(const char* extension);

    //! Get the extensions Require'd for clients, blank if there are none.
    void getRequiredExtensions(UtlString& extensionsString);

    //! Set the SIP proxy servers for the user agent.
    /*! This method will clear any existing proxy servers before
     *  resetting this list.  NOTE: As of 12/2004, only the first
     *  proxy server is used.  Please consider using DNS SRV in
     *  until fully implemented.
     */
    void setProxyServers(const char* sipProxyServers);

/* ============================ ACCESSORS ================================= */

    //! Enable or disable the outbound use of rport (send packet to actual
    //! port -- not advertised port).
    UtlBoolean setUseRport(UtlBoolean bEnable) ;

    //! Is use report set?
    UtlBoolean getUseRport() const ;

    //! Get the manually configured public address
    UtlBoolean getConfiguredPublicAddress(UtlString* pIpAddress, int* pPort) ;

    //! Get the local address and port
    UtlBoolean getLocalAddress(UtlString* pIpAddress, int* pPort) ;

    //! Get the NAT mapped address and port
    UtlBoolean getNatMappedAddress(UtlString* pIpAddress, int* pPort) ;

    //! Get the SipUserAgent's contact URI.
    void getContactURI(UtlString& contact);

    void setIsUserAgent(UtlBoolean isUserAgent);

    /// Provides a string to be appended to the standard User-Agent header.
    void setUserAgentHeaderProperty( const char* property );
    /**<
     * The property is added between "<product>/<version>" and the platform (eg "(VxWorks)")
     * The value should be formated either as "token/token", "token", or "(string)"
     * with no leading or trailing space.
     */

    //! Set the limit of allowed hops a message can make
    void setMaxForwards(int maxForwards);

    //! Get the limit of allowed hops a message can make
    int getMaxForwards();

    //! Allow or disallow recursion and forking of 3xx class requests
    void setForking(UtlBoolean enabled);

    void getFromAddress(UtlString* address, int* port, UtlString* protocol);

    void getViaInfo(int protocol,
                    UtlString& address,
                    int& port);

    void getDirectoryServer(int index, UtlString* address,
                            int* port, UtlString* protocol);

    void getProxyServer(int index, UtlString* address,
                        int* port, UtlString* protocol);

    //! Print diagnostics
    void printStatus();

    void startMessageLog(int newMaximumLogSize = 0);

    void stopMessageLog();

    void clearMessageLog();

    virtual void logMessage(const char* message, int messageLength);

    void getMessageLog(UtlString& logData);

    int getSipStateTransactionTimeout();

    // Manipulate mDefaultExpiresSeconds, the default time to let a transaction live.
    int getDefaultExpiresSeconds() const;
    void setDefaultExpiresSeconds(int expiresSeconds);

    // Manipulate mDefaultSerialExpiresSeconds, the default time to
    // let a transaction live if it is a serially-forked child.
    int getDefaultSerialExpiresSeconds() const;
    void setDefaultSerialExpiresSeconds(int expiresSeconds);

    //! Tells the User Agent whether or not to append
    //! the platform name onto the User Agent string
    void setIncludePlatformInUserAgentName(const bool bInclude);

    //! Period of time a TCP socket can remain idle before it is removed
    void setMaxTcpSocketIdleTime(int idleTimeSeconds);

    //! Get the maximum number of DNS SRV records to pursue in the
    //! case of failover
    int getMaxSrvRecords() const;

    //! Set the maximum number of DNS SRV records to pursue in the
    //! case of failover
    void setMaxSrvRecords(int numRecords);

    //! Get the number of seconds to wait before trying the next DNS SRV record
    int getDnsSrvTimeout();

    //! Set the number of seconds to wait before trying the next DNS SRV record
    void setDnsSrvTimeout(int timeout);

    //! Add other DNS names or IP addresses which are considered to
    //! refer to this SipUserAgent.
    /*! Used with routing decisions to determine whether routes
     * are targeted to this SIP server or not.
     * \param aliases - space or comma separated of the format:
     *        "sip:host:port" or "host:port"
     */
    void setHostAliases(const UtlString& aliases);

    //! Flag to recurse only one contact in a 300 response
    void setRecurseOnlyOne300Contact(UtlBoolean recurseOnlyOne);
    /***< @note this is a 300 not 3xx class response.@endnote */

    //! Flag to return Vias in too many hops response to request with max-forwards == 0
    void setReturnViasForMaxForwards(UtlBoolean returnVias);

    //! Get a copy of the original request that was sent corresponding
    //! to this incoming response
    /*! \returns NULL if not found.  Caller MUST free the copy of the
     * request when done
     */
    SipMessage* getRequest(const SipMessage& response);

    int getUdpPort() const ;
      //! Get the local UDP port number (or PORT_NONE if disabled)

    int getTcpPort() const ;
     //! Get the local TCP port number (or PORT_NONE if disabled)

    int getTlsPort() const ;
      //! Get the local Tls port number (or PORT_NONE if disabled)

    void setUserAgentName(const UtlString& name);
      //! Sets the User Agent name sent with outgoing sip messages.

    const UtlString& getUserAgentName() const;
      //! Sets the User Agent name sent with outgoing sip messages.

/* ============================ INQUIRY =================================== */

    virtual UtlBoolean isMessageLoggingEnabled();

    UtlBoolean isMethodAllowed(const char* method);

    UtlBoolean isExtensionAllowed(const char* extension) const;

    UtlBoolean isExtensionRequired(const char* extension) const;

    UtlBoolean isForkingEnabled();

    UtlBoolean isMyHostAlias(const Url& route) const;

    UtlBoolean recurseOnlyOne300Contact();

    UtlBoolean isOk(OsSocket::IpProtocolSocketType socketType);

    UtlBoolean isOk();
    //: Determine if the user agent is ok (all the protocol handlers are Ok)

    UtlBoolean isSymmetricSignalingImposed();

    //! Find out if SipUserAgent has finished shutting down.
    /*! Useful when using the non-blocking form of \ref shutdown.
     *
     * \returns TRUE if SipUserAgent has finished shutting down, FALSE otherwise.
     */
    UtlBoolean isShutdownDone();

    /** Apply the recorded User-Agent/Server header value as a User-Agent header,
     *  if the message does not already have one.
     */
    void setUserAgentHeader(SipMessage& message);

    /** Apply the recorded User-Agent/Server header value as a Server header.
     *  if the message does not already have one.
     */
    void setServerHeader(SipMessage& message);

    /// Call either setServerHeader or setUserAgentHeader, as appropriate based on isUserAgent.
    void setSelfHeader(SipMessage& message);

    SipContactDb& getContactDb() { return mContactDb; }

    //! Adds a contact record to the contact db
    const bool addContactAddress(ContactAddress& contactAddress);

    //! Gets all contact addresses for this user agent
    void getContactAddresses(ContactAddress* pContacts[], int &numContacts);

    //! SPECIAL CASE ONLY -
    // This is used only to forward in-dialog ACKs which the proxy
    // would otherwise have to route back to itself, causing a loop.
    // Instead, such ACKs are sent to the redirector/locater and
    // given a new ReqUri, then sent circling back to the proxy.
    UtlBoolean sendStatelessAck(SipMessage& request,
                                UtlString& address,
                                int port,
                                OsSocket::IpProtocolSocketType protocol);

    /** Send a UDP message symmetrically, that is, so the source port is
     *  the SipUserAgent's UDP listening port. */
    UtlBoolean sendSymmetricUdp(SipMessage& message,       ///< the message
                                const char* serverAddress, ///< destination address
                                int port                   ///< destination port
       );

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    /// constuct the value to be used in either user-agent or server header.
    void selfHeaderValue(UtlString& self);

    void getAllowedMethods(UtlString* allowedMethods);

    void whichExtensionsNotAllowed(const SipMessage* message,
                                   UtlString* disallowedExtensions) const;

    UtlBoolean checkMethods(SipMessage* message);

    UtlBoolean checkExtensions(SipMessage* message);

    UtlBoolean sendStatelessResponse(SipMessage& response);

    UtlBoolean sendStatelessRequest(SipMessage& request,
                                    const UtlString& address,
                                    int port,
                                    OsSocket::IpProtocolSocketType protocol,
                                    const UtlString& branchId);

    /** Send the message to the specified serverAddress/port using
     *  the specified transport protocol.
     *  These methods are "below" the level that tracks SIP transactions;
     *  they simply send the message (or fail).
     */
    UtlBoolean sendTls(SipMessage* message,
                       const char* serverAddress,
                       int port);

    UtlBoolean sendTcp(SipMessage* message,
                       const char* serverAddress,
                       int port);

    UtlBoolean sendUdp(SipMessage* message,
                       const char* serverAddress,
                       int port);

    // Get mReliableTransportTimeoutMs, the time from first send via TCP to first resend (msec).
    int getReliableTransportTimeout();

    // Get mUnreliableTransportTimeoutMs, the time from first send via UDP to first resend (msec).
    int getUnreliableTransportTimeout();

    // Get mMaxResendTimeoutMs, the maximum that will be used for a timeout interval.
    int getMaxResendTimeout();

    UtlBoolean shouldAuthenticate(SipMessage* message) const;

    UtlBoolean authorized(SipMessage* request,
                          const char* uri = NULL) const;

    void addAuthentication(SipMessage* message) const;

    UtlBoolean resendWithAuthorization(SipMessage* response,
                                       SipMessage* request,
                                       int* messageType,
                                       int authorizationEntity);

    UtlBoolean doesMaddrMatchesUserAgent(SipMessage& message) ;

    void setInviteTransactionTimeoutSeconds(int expiresSeconds);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    SipTcpServer* mSipTcpServer;
    SipUdpServer* mSipUdpServer;
    SipTlsServer* mSipTlsServer;
    SipTransactionList mSipTransactions;
    UtlString defaultSipUser;
    UtlString defaultSipAddress;
    UtlString proxyServers;
    UtlString directoryServers;
    UtlString registryServers;
    int registryPeriod;
    int lastRegisterSeqNum;
    UtlString registerCallId;
    UtlString sipIpAddress;
    UtlString mConfigPublicAddress ;
    int mSipPort;
    UtlDList allowedSipMethods;
    UtlDList allowedSipExtensions;
    UtlDList requiredSipExtensions;
    // The specific name for this compnent, to be added to the User-Agent
    // or Server headers.
    UtlString mUserAgentHeaderProperties;
    UtlHashBag mMyHostAliases;
    UtlHashBag mMessageObservers;
    UtlSortedList mOutputProcessors;
    OsRWMutex mMessageLogRMutex;
    OsRWMutex mMessageLogWMutex;
    OsRWMutex mOutputProcessorMutex;

    // Timers (in seconds or milliseconds)
    // Time allowed before first resend of a message, in msec.
    // T1 in RFC 3261.
    // The time from first send via UDP to first resend (msec).
    int mUnreliableTransportTimeoutMs;
    // The upper limit for timeout intervals.
    // (Timeout intervals double each time, but are limited to
    // mMaxResendTimeoutMs.)
    int mMaxResendTimeoutMs;
    // The time from first send via TCP (or other reliable transport) to first resend (msec).
    int mReliableTransportTimeoutMs;
    int mTransactionStateTimeoutMs;
    // The default time to let a transaction live.
    int mDefaultExpiresSeconds;
    // The default time to let a transaction live if it is a
    // serially-forked child.
    int mDefaultSerialExpiresSeconds;
    // Minimum secs needed for an INVITE transaction so that a called phone
    // can be answered.
    int mMinInviteTransactionTimeout;
    int mMaxTcpSocketIdleTime; // Time after which unused TCP sockets are removed (secs)
    int mDnsSrvTimeout; // Time to give up & try the next DNS SRV record (secs)

    int mMaxSrvRecords; // Max num of DNS SRV records to use before giving up


    UtlString defaultUserAgentName;
    long mLastCleanUpTime;
    UtlString mAuthenticationScheme;
    UtlString mAuthenticationRealm;
    OsConfigDb* mpAuthenticationDb;
    OsConfigDb* mpAuthorizationUserIds;
    OsConfigDb* mpAuthorizationPasswords;
    SipLineMgr* mpLineMgr;
    int mMaxMessageLogSize;
    UtlString mMessageLog;
    /** TRUE when this SipUserAgent is functioning as a UA,
     *  FALSE when it is functioning as a proxy.
     */
    UtlBoolean mIsUaTransactionByDefault;
    UtlBoolean mForkingEnabled;
    int mMaxForwards;
    UtlBoolean mRecurseOnlyOne300Contact;
    UtlBoolean mReturnViasForMaxForwards;
    UtlBoolean mbUseRport;
    bool mbIncludePlatformInUserAgentName;  // whether or not the platform name should
                                            // be appended to the user agent name

    /** check the acceptability of method, extensions, and encoding.
     * The default is TRUE; it may be set to false in applications such as a redirect server
     * that will never actually send a 2xx response, so the checks might cause errors that
     * the application should never generate.
     */
    UtlBoolean mDoUaMessageChecks;
    UtlBoolean mbForceSymmetricSignaling;
    OptionsRequestHandlePref mHandleOptionsRequests;

    void garbageCollection();

    void queueMessageToInterestedObservers(SipMessageEvent& event,
                                           const UtlString& method);
    void queueMessageToObservers(SipMessage* message,
                                 int messageType);

    //! timer that sends events to the queue periodically
    OsTimer* mpTimer;

    //! flags used during shutdown
    UtlBoolean mbShuttingDown;
    UtlBoolean mbShutdownDone;

    //! Disabled copy constructor
    SipUserAgent(const SipUserAgent& rSipUserAgent);

    //! Disabled assignment operator
    SipUserAgent& operator=(const SipUserAgent& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipUserAgent_h_
