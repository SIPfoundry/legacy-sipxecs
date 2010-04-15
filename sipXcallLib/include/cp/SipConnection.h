//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _SipConnection_h_
#define _SipConnection_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <cp/Connection.h>
#include <net/SipContactDb.h>
#include <utl/UtlHashMapIterator.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipUserAgent;
class SipMessage;
class SdpCodec;
class SdpCodecFactory;


//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class SipConnection : public Connection
{
    /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

    enum ReinviteStates
    {
        ACCEPT_INVITE = 0,
        REINVITED,
        REINVITING
    };

    /* ============================ CREATORS ================================== */

    SipConnection(const char* outboundLineAddress,
        UtlBoolean isEarlyMediaFor180Enabled = TRUE,
        CpCallManager* callMgr = NULL,
        CpCall* call = NULL,
        CpMediaInterface* mediaInterface = NULL,
        SipUserAgent* sipUA = NULL,
        int offeringDelayMilliSeconds = IMMEDIATE,
        int sessionReinviteTimer = 0,
        int availableBehavior = RING,
        const char* forwardUnconditionalUrl = NULL,
        int busyBehavior = BUSY,
        const char* forwardOnBusy = NULL);
    //:Default constructor

    virtual
        ~SipConnection();
    //:Destructor

    /* ============================ MANIPULATORS ============================== */

    virtual UtlBoolean dequeue(UtlBoolean callInFocus);

    virtual UtlBoolean send(SipMessage& message,
                        OsMsgQ* responseListener = NULL,
                        void* responseListenerData = NULL);

    virtual UtlBoolean dial(const char* dialString,
                            const char* callerId,
                            const char* callId,
                            const char* callController = NULL,
                            const char* originalCallConnection = NULL,
                            UtlBoolean requestQueuedCall = FALSE,
                            const void* pDisplay = NULL,
                            const char* originalCallId = NULL,
						    const char* paiAddress = NULL);
    //! param: requestQueuedCall - indicates that the caller wishes to have the callee queue the call if busy

    /// Initiate transfer on transfer controller connection in the original call.
    virtual UtlBoolean originalCallTransfer(
       UtlString&  transferTargetAddress,
       const char* transferControllerAddress,
       const char* targetCallId,
       bool        holdBeforeTransfer = true
                                            );
    /**
     * If fromAddress or toAddress are NULL it is assumed to be a blind transfer.
     */

    virtual UtlBoolean targetCallBlindTransfer(const char* transferTargetAddress,
        const char* transferControllerAddress);
    // Communicate blind transfer on transfer controller connection in
    // the target call.  This is signaled by the transfer controller in the
    // original call.

    virtual UtlBoolean transfereeStatus(int connectionState, int response);
    // Method to communicate status to original call on transferee side

    virtual UtlBoolean transferControllerStatus(int connectionState, int response);
    // Method to communicate status to target call on transfer
    // controller side

    virtual UtlBoolean answer(const void* hWnd = NULL);

    virtual UtlBoolean hangUp();

    virtual UtlBoolean hold();

    virtual UtlBoolean offHold();

    virtual UtlBoolean renegotiateCodecs();

    virtual UtlBoolean sendKeepAlive(UtlBoolean useOptionsForKeepalive);

    virtual UtlBoolean accept(int forwardOnNoAnswerTimeOut);

    virtual UtlBoolean reject(int errorCode, const char* errorText);

    virtual UtlBoolean redirect(const char* forwardAddress);

    virtual UtlBoolean sendInfo(UtlString contentType, UtlString sContent);

    virtual UtlBoolean processMessage(OsMsg& eventMessage,
        UtlBoolean callInFocus, UtlBoolean onHook);

    void setCallerId();

    virtual UtlBoolean getRemoteAddress(UtlString* remoteAddress) const;
    virtual UtlBoolean getRemoteAddress(UtlString* remoteAddress,
        UtlBoolean leaveFieldParmetersIn) const;

    static UtlBoolean processNewFinalMessage(SipUserAgent* sipUa,
        OsMsg* eventMessage);

    void setContactType(ContactType eType) ;
    void setContactId(ContactId contactId) { mContactId = contactId; }


    /* ============================ ACCESSORS ================================= */

    virtual UtlBoolean getSession(SipSession& session);

    virtual OsStatus getFromField(UtlString* fromField);

    virtual OsStatus getToField(UtlString* toField);

    virtual OsStatus getInvite(SipMessage* message);

    int getNextCseq();

    /* ============================ INQUIRY =================================== */

    static UtlBoolean shouldCreateConnection(SipUserAgent& sipUa,
        OsMsg& eventMessage,
        SdpCodecFactory* codecFactory = NULL);

    virtual UtlBoolean willHandleMessage(OsMsg& eventMessage) const;

    virtual UtlBoolean isConnection(const char* callId,
        const char* toTag,
        const char* fromTag,
        UtlBoolean  strictCompare) const;

    virtual UtlBoolean isSameRemoteAddress(Url& remoteAddress) const;
    virtual UtlBoolean isSameRemoteAddress(Url& remoteAddress,
        UtlBoolean tagsMustMatch) const;

    /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    ContactType selectCompatibleContactType(const SipMessage& request) ;
    //: Select a compatible contact given the URI

    void updateContact(Url* pContactUrl, ContactType eType) ;

    static UtlBoolean requestShouldCreateConnection(const SipMessage* sipMsg,
        SipUserAgent& sipUa,
        SdpCodecFactory* codecFactory);

    UtlBoolean doOffHold(UtlBoolean forceReInvite);

    UtlBoolean doHangUp(const char* dialString = NULL,
        const char* callerId = NULL);

    void buildFromToAddresses(const char* dialString,
        const char* callerId,
        const char* callerDisplayName,
        UtlString& fromAddress,
        UtlString& goodToAddress) const;

    void buildLocalContact(Url fromAddress,
        UtlString& localContact) ;//for outbound call
    void buildLocalContact(UtlString& localContact) ;//when getting inbound calls

    UtlBoolean extendSessionReinvite();

    // SIP Request handlers
    UtlBoolean processRequest(const SipMessage* request,
        UtlBoolean callInFocus, UtlBoolean onHook);
    void processInviteRequest(const SipMessage* request);
    void processReferRequest(const SipMessage* request);
    void processAckRequest(const SipMessage* request);
    void processByeRequest(const SipMessage* request);
    void processCancelRequest(const SipMessage* request);
    void processNotifyRequest(const SipMessage* request);

    // SIP Response handlers
    UtlBoolean processResponse(const SipMessage* response,
        UtlBoolean callInFocus, UtlBoolean onHook);
    void processInviteResponse(const SipMessage* request);
    void processReferResponse(const SipMessage* request);
    void processOptionsResponse(const SipMessage* request);
    void processByeResponse(const SipMessage* request);
    void processCancelResponse(const SipMessage* request);
    void processNotifyResponse(const SipMessage* request);

    /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    SipUserAgent* sipUserAgent;
    UtlString mFromTag;
    SipMessage* mInviteMsg;
    UtlBoolean      mbCancelling;
    UtlBoolean mWaitingForKeepAliveResponse;
    SdpBody* mPrevSdp;

    SipMessage* mReferMessage;
    UtlBoolean inviteFromThisSide;
    UtlString mLastRequestMethod;
    UtlString mRemoteContact; // last contact field from other side / name-addr
    UtlString mRemoteContactUri; //last contact field from the other side / addr-spec
    Url mFromUrl; // SIP address for the local side
    Url mToUrl;  //  SIP address for the remote side
    UtlString mRemoteUriStr;  //  SIP uri string for the remote side
    UtlString mLocalUriStr;  //  SIP uri string for the local side
    UtlHashMap mProvisionalToTags; // Key:To Tag, Value:Remote Contact

    int lastLocalSequenceNumber;
    int lastRemoteSequenceNumber;
    int reinviteState;
    UtlString mRouteField;
    int mDefaultSessionReinviteTimer;
    int mSessionReinviteTimer;
    UtlString mAllowedRemote;  // Methods supported by the otherside
    UtlBoolean mIsReferSent;   // used to determine whether to send ack when sequence number is smaller
    UtlBoolean mIsAcceptSent;   // used to determine whether to accept ack when sequence number is smaller
    int mHoldCompleteAction;
    UtlBoolean mIsEarlyMediaFor180;
    UtlString mLineId; //line identifier for incoming calls.
    UtlString mLocalContact;    ///< The local Contact: field value - a URI in name-addr format.
    ContactType mContactType ;
    ContactId mContactId;
    UtlBoolean mDropping ;

    UtlBoolean getInitialSdpCodecs(const SipMessage* sdpMessage,
        SdpCodecFactory& supportedCodecsArray,
        int& numCodecsInCommon,
        SdpCodec** &codecsInCommon,
        UtlString& remoteAddress,
        int& remotePort,
        int& remoteRtcpPort,
        SdpDirectionality* directionality) const;

    virtual void proceedToRinging(const SipMessage* inviteMessage,
                                  SipUserAgent* sipUserAgent,
                                  int availableBehavior);

    UtlBoolean isMethodAllowed(const char* method);

    void doBlindRefer();



    SipConnection& operator=(const SipConnection& rhs);
    //:Assignment operator (disabled)

    SipConnection(const SipConnection& rSipConnection);
    //:Copy constructor (disabled)


};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipConnection_h_
