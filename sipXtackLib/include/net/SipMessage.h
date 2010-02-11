//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
////////////////////////////////////////////////////////////////////////
//////

#ifndef _SipMessage_h_
#define _SipMessage_h_

// SYSTEM INCLUDES
#include <limits.h>

// APPLICATION INCLUDES
#include <utl/UtlHashBag.h>

#include <net/HttpMessage.h>
#include <net/SdpBody.h>
#include <net/SdpCodec.h>
#include <net/Url.h>

class UtlHashMap;
class SipUserAgent;

// DEFINES

// SIP extensions
// Keep in alphabetical order by extension.
#define SIP_EVENTLIST_EXTENSION "eventlist"
#define SIP_JOIN_EXTENSION "join"
#define SIP_PATH_EXTENSION "path"
#define SIP_REPLACES_EXTENSION "replaces"
#define SIP_CALL_CONTROL_EXTENSION "sip-cc"
#define SIP_SESSION_TIMER_EXTENSION "timer"

// SIP Methods
// Keep in alphabetical order by method.
#define SIP_ACK_METHOD "ACK"
#define SIP_BYE_METHOD "BYE"
#define SIP_CANCEL_METHOD "CANCEL"
#define SIP_INFO_METHOD "INFO"
#define SIP_INVITE_METHOD "INVITE"
#define SIP_MESSAGE_METHOD "MESSAGE"
#define SIP_NOTIFY_METHOD "NOTIFY"
#define SIP_OPTIONS_METHOD "OPTIONS"
#define SIP_PRACK_METHOD "PRACK"
#define SIP_PUBLISH_METHOD "PUBLISH"
#define SIP_REFER_METHOD "REFER"
#define SIP_REGISTER_METHOD "REGISTER"
#define SIP_SUBSCRIBE_METHOD "SUBSCRIBE"
#define SIP_UPDATE_METHOD "UPDATE"

// SIP Fields
// Keep in alphabetical order by field name.
// Put "SHORT" versions immediately after long versions.
#define SIP_ACCEPT_FIELD "ACCEPT"
#define SIP_ACCEPT_ENCODING_FIELD HTTP_ACCEPT_ENCODING_FIELD
#define SIP_ACCEPT_LANGUAGE_FIELD HTTP_ACCEPT_LANGUAGE_FIELD
#define SIP_ALLOW_FIELD "ALLOW"
#define SIP_ALLOW_EVENTS_FIELD "ALLOW-EVENTS"
#define SIP_ALSO_FIELD "ALSO"
#define SIP_CALLID_FIELD "CALL-ID"
#define SIP_SHORT_CALLID_FIELD "I"
#define SIP_CONFIG_ALLOW_FIELD "CONFIG_ALLOW"
#define SIP_CONFIG_REQUIRE_FIELD "CONFIG_REQUIRE"
#define SIP_CONTACT_FIELD "CONTACT"
#define SIP_SHORT_CONTACT_FIELD "M"
#define SIP_CONTENT_LENGTH_FIELD HTTP_CONTENT_LENGTH_FIELD
#define SIP_SHORT_CONTENT_LENGTH_FIELD "L"
#define SIP_CONTENT_TYPE_FIELD HTTP_CONTENT_TYPE_FIELD
#define SIP_SHORT_CONTENT_TYPE_FIELD "C"
#define SIP_CONTENT_ENCODING_FIELD "CONTENT-ENCODING"
#define SIP_SHORT_CONTENT_ENCODING_FIELD "E"
#define SIP_CSEQ_FIELD "CSEQ"
#define SIP_EVENT_FIELD "EVENT"
#define SIP_SHORT_EVENT_FIELD "O"
#define SIP_EXPIRES_FIELD "EXPIRES"
#define SIP_FROM_FIELD "FROM"
#define SIP_SHORT_FROM_FIELD "F"
#define SIP_GR_FIELD "GR"
#define SIP_MAX_FORWARDS_FIELD "MAX-FORWARDS"
#define SIP_MIN_EXPIRES_FIELD "MIN-EXPIRES"
#define SIP_PATH_FIELD "PATH"
#define SIP_PROXY_REQUIRE_FIELD "PROXY-REQUIRE"
#define SIP_Q_FIELD "Q"
#define SIP_RACK_FIELD "RACK"
#define SIP_REASON_FIELD "REASON"
#define SIP_RECORD_ROUTE_FIELD "RECORD-ROUTE"
#define SIP_REFER_TO_FIELD "REFER-TO"
#define SIP_SHORT_REFER_TO_FIELD "R"
#define SIP_REFERENCES_FIELD "REFERENCES"
#define SIP_REFERRED_BY_FIELD "REFERRED-BY"
#define SIP_SHORT_REFERRED_BY_FIELD "B"
#define SIP_REPLACES_FIELD "REPLACES"
#define SIP_REQUEST_DISPOSITION_FIELD "REQUEST-DISPOSITION"
#define SIP_REQUESTED_BY_FIELD "REQUESTED-BY"
#define SIP_REQUIRE_FIELD "REQUIRE"
#define SIP_RETRY_AFTER_FIELD "RETRY-AFTER"
#define SIP_ROUTE_FIELD "ROUTE"
#define SIP_RSEQ_FIELD "RSEQ"
#define SIP_SERVER_FIELD "SERVER"
#define SIP_SESSION_EXPIRES_FIELD "SESSION-EXPIRES"
#define SIP_IF_MATCH_FIELD "SIP-IF-MATCH"
#define SIP_ETAG_FIELD "SIP-ETAG"
#define SIP_SUBJECT_FIELD "SUBJECT"
#define SIP_SHORT_SUBJECT_FIELD "S"
#define SIP_SUBSCRIPTION_STATE_FIELD "SUBSCRIPTION-STATE"
#define SIP_SUPPORTED_FIELD "SUPPORTED"
#define SIP_SHORT_SUPPORTED_FIELD "K"
#define SIP_TO_FIELD "TO"
#define SIP_SHORT_TO_FIELD "T"
#define SIP_UNSUPPORTED_FIELD "UNSUPPORTED"
#define SIP_USER_AGENT_FIELD HTTP_USER_AGENT_FIELD
#define SIP_VIA_FIELD "VIA"
#define SIP_SHORT_VIA_FIELD "V"
#define SIP_WARNING_FIELD "WARNING"
#define SIP_SIPX_AUTHIDENTITY "X-sipX-Authidentity"
#define SIP_SIPX_FROM_CALLER_DIRECTION_HEADER "X-sipX-FromCaller"
#define SIP_SIPX_NAT_HANDLED "X-sipX-handled"
#define SIP_SIPX_SESSION_CONTEXT_ID_HEADER "X-sipX-Id"
#define SIP_SIPX_NAT_ROUTE_FIELD "X-sipX-Nat-Route"
#define SIP_SIPX_SPIRAL_HEADER "X-sipX-Spiral"
#define SIP_SIPX_CALL_DEST_FIELD "sipXecs-CallDest"

///custom fields
#define SIP_LINE_IDENTIFIER "LINEID"
#define SIPX_IMPLIED_SUB "sipx-implied" ///< integer expiration duration for subscription
// Response codes and text
#define SIP_1XX_CLASS_CODE 100

#define SIP_TRYING_CODE 100
#define SIP_TRYING_TEXT "Trying"

#define SIP_RINGING_CODE 180
#define SIP_RINGING_TEXT "Ringing"

#define SIP_QUEUED_CODE 182
#define SIP_QUEUED_TEXT "Queued"

#define SIP_EARLY_MEDIA_CODE 183
#define SIP_EARLY_MEDIA_TEXT "Session Progress"

#define SIP_2XX_CLASS_CODE 200

#define SIP_OK_CODE 200
#define SIP_OK_TEXT "OK"

#define SIP_ACCEPTED_CODE 202
#define SIP_ACCEPTED_TEXT "Accepted"

#define SIP_3XX_CLASS_CODE 300

#define SIP_MULTI_CHOICE_CODE 300
#define SIP_MULTI_CHOICE_TEXT "Multiple Choices"

#define SIP_PERMANENT_MOVE_CODE 301
#define SIP_PERMANENT_MOVE_TEXT "Moved Permanently"

#define SIP_TEMPORARY_MOVE_CODE 302
#define SIP_TEMPORARY_MOVE_TEXT "Moved Temporarily"

#define SIP_USE_PROXY_CODE 305
#define SIP_USE_PROXY_TXT "Use Proxy"

#define SIP_4XX_CLASS_CODE 400

#define SIP_BAD_REQUEST_CODE 400
#define SIP_BAD_REQUEST_TEXT "Bad Request"

#define SIP_FORBIDDEN_CODE 403
#define SIP_FORBIDDEN_TEXT "Forbidden"

#define SIP_NOT_FOUND_CODE 404
#define SIP_NOT_FOUND_TEXT "Not Found"

#define SIP_BAD_METHOD_CODE 405
#define SIP_BAD_METHOD_TEXT "Method Not Allowed"

#define SIP_PROXY_AUTH_REQ_CODE 407
#define SIP_PROXY_AUTH_REQ_TEXT "Proxy Authentication Required"

#define SIP_REQUEST_TIMEOUT_CODE 408
#define SIP_REQUEST_TIMEOUT_TEXT "Request timeout"

#define SIP_CONDITIONAL_REQUEST_FAILED_CODE 412
#define SIP_CONDITIONAL_REQUEST_FAILED_TEXT "Conditional Request Failed"

#define SIP_BAD_MEDIA_CODE 415
#define SIP_BAD_MEDIA_TEXT "Unsupported Media Type or Content Encoding"

#define SIP_UNSUPPORTED_URI_SCHEME_CODE 416
#define SIP_UNSUPPORTED_URI_SCHEME_TEXT "Unsupported URI Scheme"

#define SIP_BAD_EXTENSION_CODE 420
#define SIP_BAD_EXTENSION_TEXT "Extension Not Supported"

#define SIP_EXTENSION_REQUIRED_CODE 421
#define SIP_EXTENSION_REQUIRED_TEXT "Extension Required"

#define SIP_TOO_BRIEF_CODE 423
#define SIP_TOO_BRIEF_TEXT "Registration Too Brief"
#define SIP_SUB_TOO_BRIEF_TEXT "Subscription Too Brief"

#define SIP_TEMPORARILY_UNAVAILABLE_CODE 480
#define SIP_TEMPORARILY_UNAVAILABLE_TEXT "Temporarily Unavailable"

#define SIP_BAD_TRANSACTION_CODE 481
#define SIP_BAD_TRANSACTION_TEXT "Transaction Does Not Exist"
#define SIP_BAD_SUBSCRIPTION_CODE 481
#define SIP_BAD_SUBSCRIPTION_TEXT "Subscription Does Not Exist"

#define SIP_LOOP_DETECTED_CODE 482
#define SIP_LOOP_DETECTED_TEXT "Loop Detected"

#define SIP_TOO_MANY_HOPS_CODE 483
#define SIP_TOO_MANY_HOPS_TEXT "Too many hops"

#define SIP_BAD_ADDRESS_CODE 484
#define SIP_BAD_ADDRESS_TEXT "Address Incomplete"

#define SIP_BUSY_CODE 486
#define SIP_BUSY_TEXT "Busy Here"

#define SIP_REQUEST_TERMINATED_CODE 487
#define SIP_REQUEST_TERMINATED_TEXT "Request Terminated"

#define SIP_REQUEST_NOT_ACCEPTABLE_HERE_CODE 488
#define SIP_REQUEST_NOT_ACCEPTABLE_HERE_TEXT "Not Acceptable Here"

#define SIP_BAD_EVENT_CODE 489
#define SIP_BAD_EVENT_TEXT "Requested Event Type Is Not Supported"

#define SIP_5XX_CLASS_CODE 500

#define SIP_SERVER_INTERNAL_ERROR_CODE 500
#define SIP_SERVER_INTERNAL_ERROR_TEXT "Internal Server Error"
#define SIP_OUT_OF_ORDER_CODE 500
#define SIP_OUT_OF_ORDER_TEXT "Out Of Order"

#define SIP_UNIMPLEMENTED_METHOD_CODE 501
#define SIP_UNIMPLEMENTED_METHOD_TEXT "Not Implemented"

#define SIP_SERVICE_UNAVAILABLE_CODE 503
#define SIP_SERVICE_UNAVAILABLE_TEXT "Service Unavailable"

#define SIP_BAD_VERSION_CODE 505
#define SIP_BAD_VERSION_TEXT "Version Not Supported"

#define SIP_6XX_CLASS_CODE 600

#define SIP_GLOBAL_BUSY_CODE 600
#define SIP_GLOBAL_BUSY_TEXT "Busy Everywhere"

#define SIP_DECLINE_CODE 603
#define SIP_DECLINE_TEXT "Declined"

// There is no 7xx class.  This symbol defines the end of the 6xx class.
#define SIP_7XX_CLASS_CODE 700

// Warning codes
#define SIP_WARN_INCOMPAT_PROTO_CODE          300
#define SIP_WARN_INCOMPAT_PROTO_TEXT          "Incompatible network protocol"
#define SIP_WARN_INCOMPAT_ADDRESS_CODE        301
#define SIP_WARN_INCOMPAT_ADDRESS_TEXT        "Incompatible network address formats"
#define SIP_WARN_INCOMPAT_TRANSPORT_CODE      302
#define SIP_WARN_INCOMPAT_TRANSPORT_TEXT      "Incompatible transport protocol"
#define SIP_WARN_INCOMPAT_BANDWIDTH_CODE      303
#define SIP_WARN_INCOMPAT_BANDWIDTH_TEXT      "Incompatible bandwidth units"
#define SIP_WARN_MEDIA_NAVAIL_MEDIA_TYPE_CODE 304
#define SIP_WARN_MEDIA_NAVAIL_MEDIA_TYPE_TEXT "Media type not available"
#define SIP_WARN_MEDIA_INCOMPAT_MEDIA_CODE    305
#define SIP_WARN_MEDIA_INCOMPAT_MEDIA_TEXT    "Insufficient compatible media types"
#define SIP_WARN_INCOMPAT_ATTRIBUTE_CODE      306
#define SIP_WARN_INCOMPAT_ATTRIBUTE_TEXT      "Attribute not understood"
#define SIP_WARN_INCOMPAT_SDP_PARAM_CODE      307
#define SIP_WARN_INCOMPAT_SDP_PARAM_TEXT      "Session description parameter not understood"
#define SIP_WARN_NAVAIL_MULTICAST_CODE        330
#define SIP_WARN_NAVAIL_MULTICAST_TEXT        "Multicast not available"
#define SIP_WARN_NAVAIL_UNICAST_CODE          331
#define SIP_WARN_NAVAIL_UNICAST_TEXT          "Unicast not available"
#define SIP_WARN_NAVAIL_BANDWIDTH_CODE        370
#define SIP_WARN_NAVAIL_BANDWIDTH_TEXT        "Insufficient bandwidth"
#define SIP_WARN_MISC_CODE                    399
#define SIP_WARN_MISC_TEXT                    "<Provide your own warning text>"

// Transport stuff
#define SIP_PORT 5060
#define SIP_TLS_PORT 5061
#define SIP_PROTOCOL_VERSION "SIP/2.0"
#define SIP_SUBFIELD_SEPARATOR " "
#define SIP_SUBFIELD_SEPARATORS "\t "
#define SIP_MULTIFIELD_SEPARATOR ","
#define SIP_SINGLE_SPACE " "
#define SIP_MULTIFIELD_SEPARATORS "\t ,"
#define SIP_TRANSPORT_UDP "UDP"
#define SIP_TRANSPORT_TCP "TCP"
#define SIP_TRANSPORT_TLS "TLS"
#define SIP_URL_TYPE "SIP:"
#define SIPS_URL_TYPE "SIPS:"
#define SIP_DEFAULT_MAX_FORWARDS 20

// Subfield constant
#define BOTTOM_SUBFIELD (INT_MAX)

// Caller preference request dispostions tokens
#define SIP_DISPOSITION_QUEUE "QUEUE"

// NOTIFY method event types
#define SIP_EVENT_MESSAGE_SUMMARY           "message-summary"
#define SIP_EVENT_SIMPLE_MESSAGE_SUMMARY    "simple-message-summary"
#define SIP_EVENT_CHECK_SYNC                "check-sync"
#define SIP_EVENT_REFER                     "refer"
#define SIP_EVENT_CONFIG                    "sip-config"
#define SIP_EVENT_UA_PROFILE                "ua-profile"

// NOTIFY Subscription-State values
#define SIP_SUBSCRIPTION_ACTIVE             "active"
#define SIP_SUBSCRIPTION_PENDING            "pending"
#define SIP_SUBSCRIPTION_TERMINATED         "terminated"

// The following are used for the REFER NOTIFY message body contents
#define CONTENT_TYPE_SIP_APPLICATION        "application/sip"
#define CONTENT_TYPE_MESSAGE_SIPFRAG        "message/sipfrag"
#define CONTENT_TYPE_SIMPLE_MESSAGE_SUMMARY "application/simple-message-summary"
#define CONTENT_TYPE_XPRESSA_SCRIPT         "text/xpressa-script"

#define SIP_REFER_SUCCESS_STATUS "SIP/2.0 200 OK\r\n"
#define SIP_REFER_FAILURE_STATUS "SIP/2.0 503 Service Unavailable\r\n"

// Reason header text strings
#define SIP_REASON_CALL_ANSWERED_ELSEWHERE "Call answered elsewhere"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipTransaction;

/// Specialization of HttpMessage to contain and manipulate SIP messages
/**
 * @see HttpMessage for the descriptions of the general constructs
 * manipulators and accessors for the three basic parts of a SIP
 * message.  A message can be queried as to whether it is a request or a
 * response via the isResponse method.
 *
 * @nosubgrouping
 */
class SipMessage : public HttpMessage
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   // See sipXcall's CpCallManager for more defines

   enum EventSubTypes
   {
       NET_UNSPECIFIED = 0,
       NET_SIP_MESSAGE
   };

/* ============================ CREATORS ================================== */

    //! Construct from a buffer
    SipMessage(const char* messageBytes = NULL,
               ssize_t byteCount = -1);

    //! Construct from bytes read from a socket
    SipMessage(OsSocket* inSocket,
               ssize_t bufferSize = HTTP_DEFAULT_SOCKET_BUFFER_SIZE);

    //! Copy constructor
    SipMessage(const SipMessage& rSipMessage);

    //! Assignment operator
    SipMessage& operator=(const SipMessage& rhs);

    virtual
    ~SipMessage();
    //:Destructor

    /**
     * Get the ContainableType for a UtlContainable-derived class.
     */
    virtual UtlContainableType getContainableType() const;

    /** Class type used for runtime checking */
    static const UtlContainableType TYPE;

/* ============================ MANIPULATORS ============================== */

    static UtlBoolean getShortName( const char* longFieldName,
                                   UtlString* shortFieldName );

    static UtlBoolean getLongName( const char* shortFieldName,
                                  UtlString* longFieldName );

    void replaceShortFieldNames();

/* ============================ ACCESSORS ================================= */

    //! @name SIP URL manipulators
    //@{
    //! Generate a SIP name-addr from the specified components.
    static void buildSipUri(UtlString* url,
                            const char* address,
                            int port = PORT_NONE,
                            const char* protocol = NULL,
                            const char* user = NULL,
                            const char* userLabel = NULL,
                            const char* tag = NULL);

    static void buildReplacesField(UtlString& replacesField,
                                   const char* callId,
                                   const char* fromField,
                                   //< name-addr from which to extract the from-tag
                                   const char* toField
                                   //< name-addr from which to extract the to-tag
       );

    static UtlBoolean parseParameterFromUri(const char* uri,
                                           const char* parameterName,
                                           UtlString* parameterValue);

    static void setUriParameter(UtlString* uri, const char* parameterName,
                                const char* parameterValue);

    static void parseAddressFromUri(const char* uri, UtlString* address,
                                    int* port, UtlString* protocol,
                                    UtlString* user = NULL,
                                    UtlString* userLabel = NULL,
                                    UtlString* tag = NULL);

    static void ParseContactFields(const SipMessage *sipResponse,
                                   const SipMessage* ipRequest,
                                   const UtlString& subFieldName,
                                   int& subFieldValue);

    static void setUriTag(UtlString* uri, const char* tagValue);

    static UtlBoolean getViaTag(const char* viaField,
                           const char* tagName,
                           UtlString& tagValue);
    //@}

    //! @name SIP specific first header line accessors
    //@{
    void setSipRequestFirstHeaderLine(const char* method,
                                      const char* uri,
                                      const char* protocolVersion = SIP_PROTOCOL_VERSION);

    void changeUri(const char* uri);

    void getUri(UtlString* address,
                int* port,
                UtlString* protocol,
                UtlString* user = NULL) const;
    //@}


    //! @name Request builder methods
    /*! The following set of methods are used for building
     * requests in this message
     */
    //@{
    void setAckData(const char* uri,
                    const char* fromAddress,
                    const char* toAddress,
                    const char* callId,
                    int sequenceNumber = 1);

    void setAckData(const SipMessage* inviteResponse,
                    const SipMessage* inviteRequest,
                    const char* contactUri = NULL,
                    int sessionExpiresSeconds = 0);

    void setAckErrorData(const SipMessage* inviteResponse);

    void setByeData(const char* uri,
                    const char* fromAddress,
                    const char* toAddress,
                    const char* callId,
                    const char* localContact,
                    int sequenceNumber = 1);

    void setByeData(const SipMessage* inviteResponse,
                    const char * lastRespContact,
                    UtlBoolean byeToCallee,
                    int localSequenceNumber,
                    const char* routeField,
                    const char* alsoInviteUri,
                    const char* localContact);

    void setCancelData(const char* fromAddress, const char* toAddress,
                       const char* callId,
                       int sequenceNumber = 1);

    void setCancelData(const SipMessage* inviteResponse,
                       const UtlString* reasonValue = NULL);

    void setInviteData(const char* fromAddress,
                       const char* toAddress,
                       const char * farEndContact,
                       const char* contactUrl,
                       const char* callId,
                       const char* rtpAddress,
                       int rtpAudioPort,
                       int rtcpAudioPort,
                       int rtpVideoPort,
                       int rtcpVideoPort,
                       SdpSrtpParameters* srtpParams,
                       int sequenceNumber = 1,
                       int numRtpCodecs = 0,
                       SdpCodec* rtpCodecs[] = NULL,
                       int sessionReinviteTimer = 0,
                       const char* headerPAI = NULL);

    void setInviteData(const char* fromField,
                       const char* toField,
                       const char* farEndContact,
                       const char* contactUrl,
                       const char* callId,
                       int sequenceNumber = 1,
                       int sessionReinviteTimer = 0,
                       const char* headerPAI = NULL);

    void setInviteData(const SipMessage* previousInvite,
                       const char* newUri);

    void setReinviteData(SipMessage* invite,
                         const char* farEndContact,
                         const char* contactUrl,
                         UtlBoolean inviteFromThisSide,
                         const char* routeField,
                         const char* rtpAddress,
                         int rtpAudioPort,
                         int rtcpaudioPort,
                         int rtpVideoPort,
                         int rtcpVideoPort,
                         int sequenceNumber,
                         int numRtpCodecs,
                         SdpCodec* rtpCodecs[],
                         SdpSrtpParameters* srtpParams,
                         int sessionReinviteTimer);

    void setReinviteData(SipMessage* invite,
                         const char* farEndContact,
                         const char* contactUrl,
                         UtlBoolean inviteFromThisSide,
                         const char* routeField,
                         int sequenceNumber,
                         int sessionReinviteTimer);

    void setOptionsData(const SipMessage* inviteRequest,
                        const char* LastRespContact,
                        UtlBoolean optionsToCallee,
                        int localCSequenceNumber,
                        const char* routeField,
                        const char* localContact);

    void setNotifyData(const SipMessage *subscribeRequest,
                       int localCSequenceNumber,
                       const char* route,
                       const char* stateField = NULL,
                       const char* eventField = NULL,
                       const char* id = NULL);

    void setNotifyData( const char* uri,
                        const char* fromField,
                        const char* toField,
                        const char* callId,
                        int lastNotifyCseq,
                        const char* eventtype,
                        const char* id,
                        const char* state,
                        const char* contact,
                        const char* routeField);

    void setSubscribeData(const char* uri,
                          const char* fromField, ///< name-addr
                          const char* toField,   ///< name-addr
                          const char* callId,
                          int cseq,
                          const char* eventField,
                          const char* acceptField,
                          const char* id,
                          const char* contact,   ///< may be NULL
                          const char* routeField,
                          int expiresInSeconds);

    void setEnrollmentData(const char* uri,
                           const char* fromField,
                           const char* toField,
                           const char* callId,
                           int CSequenceNum,
                           const char* contactUrl,
                           const char* protocolField,
                           const char* profileField,
                           int expiresInSeconds = -1);

    void setVoicemailData(const char* fromField,
                           const char* toField,
                           const char* Uri,
                           const char* contactUrl,
                           const char* callId,
                           int CSeq,
                           int subscribePeriod = -1);


    void setReferData(const SipMessage* inviteResponse,
                    UtlBoolean byeToCallee,
                    int localSequenceNumber,
                    const char* routeField,
                    const char* contactUrl,
                    const char* remoteContactUrl,
                    const char* transferTargetAddress,
                    const char* targetCallId);

    void setRegisterData(const char* registererUri,      ///< From
                         const char* registerAsUri,      ///< To
                         const char* registrarServerUri, ///< request-URI
                         const char* takeCallsAtUri,     ///< Contact
                         const char* callId,
                         int sequenceNumber,
                         int expiresInSeconds = -1       ///< Expires
       );

    void setRequestData(const char* method,
                        const char* uri,         ///< request-URI
                        const char* fromAddress, ///< name-addr
                        const char* toAddress,   ///< name-addr
                        const char* callId,
                        int sequenceNumber = 1,
                        const char* contactUrl = NULL);

    //! Set a PUBLISH request
    void setPublishData( const char* uri,
                         const char* fromField,
                         const char* toField,
                         const char* callId,
                         int cseq,
                         const char* eventField,
                         const char* id,
                         const char* sipIfMatchField,
                         int expiresInSeconds);


    /// Apply any header parameters in the request uri to the message
    void applyTargetUriHeaderParams();
    /**
     * There are some special rules implemented by this routine:
     *
     * - The header must not be forbidden by isUrlHeaderAllowed
     *
     * - If the header is a From, then any tag parameter on that
     *   new From header value is removed, and the tag parameter
     *   from the original From header is inserted.  The original
     *   From header value is inserted in an X-Original-From header.
     *
     * - If the header is a Route, it is forced to be a loose route
     *   and inserted as the topmost Route.
     *
     * - If it a unique header per isUrlHeaderUnique, then the new
     *   value replaces the old one.
     *
     * - otherwise, the new header is just added.to the message.
     */

    //@}

    void addSdpBody(const char* rtpAddress,
                    int rtpAudioPort,
                    int rtcAudiopPort,
                    int rtpVideoPort,
                    int rtcpVideoPort,
                    int numRtpCodecs,
                    SdpCodec* rtpCodecs[],
                    SdpSrtpParameters* srtpParams);

    //! Accessor to get SDP body, optionally decrypting it if key info. is provided
    /*
     *  \param derPkcs12PrivateKey - DER format pkcs12 container for the
     *         private key and public key/Certificate for a recipent who is
     *         allowed to decrypt this pkcs7 (S/MIME) encapsulated body.
     *  \param derPkcs12PrivateKeyLength - length in bytes of derPkcs12PrivateKey
     *  \param pkcs12SymmetricKey - symetric key used to protect (encrypt) the
     *         derPkcs12PrivateKey (the private key is contained in a
     *         pkcs12 in an encrypted format to protect it from theft).
     *  \param pkcs12SymmetricKeyLength - the length in bytes of
     *         pkcs12SymmetricKey.
     */
    const SdpBody* getSdpBody(const char* derPkcs12 = NULL,
                              int derPkcs12Length = 0,
                              const char* pkcs12SymmetricKey = NULL) const;

    //! Check whether the message has a valid SDP body
    /*
     *  \param derPkcs12PrivateKey - DER format pkcs12 container for the
     *         private key and public key/Certificate for a recipent who is
     *         allowed to decrypt this pkcs7 (S/MIME) encapsulated body.
     *  \param derPkcs12PrivateKeyLength - length in bytes of derPkcs12PrivateKey
     *  \param pkcs12SymmetricKey - symetric key used to protect (encrypt) the
     *         derPkcs12PrivateKey (the private key is contained in a
     *         pkcs12 in an encrypted format to protect it from theft).
     *  \param pkcs12SymmetricKeyLength - the length in bytes of
     *         pkcs12SymmetricKey.
     */
    UtlBoolean hasSdpBody(const char* derPkcs12 = NULL,
                          int derPkcs12Length = 0,
                          const char* pkcs12SymmetricKey = NULL) const;



    //! @name Response builders
    /*! The following methods are used to build responses
     * in this message
     */
    //@{
    void setResponseData(const SipMessage* request,
                         int responseCode,
                         const char* responseText,
                         const char* localContact = NULL,
                         UtlBoolean echoRecordRouteIfApplicable = TRUE);

    void setResponseData(int statusCode, const char* statusText,
                         const char* fromAddress,
                         const char* toAddress,
                         ///< A to-tag will be generated if none is supplied.
                         const char* callId,
                         int sequenceNumber,
                         const char* sequenceMethod,
                         const char* localContact = NULL);

    void setOkResponseData(const SipMessage* request,
                           const char* localContact = NULL);

    void setRequestTerminatedResponseData(const SipMessage* request);

    virtual void setRequestUnauthorized(const SipMessage* request,
                                const char* authenticationScheme,
                                const char* authenticationRealm,
                                const char* authenticationNonce = NULL,
                                const char* authenticationOpaque = NULL,
                                enum HttpEndpointEnum authEntity = SERVER);

    void setTryingResponseData(const SipMessage* request);

    /// Generate a 488 response (no compatible codecs).
    void setInviteBadCodecs(const SipMessage* inviteRequest,
                            SipUserAgent* ua
                            /**< SipUserAgent from which to extract the
                             *   agent identification for the Warning:
                             *   header.
                             */
       );

    void setRequestBadMethod(const SipMessage* request,
                             const char* allowedMethods);

    void setRequestUnimplemented(const SipMessage* request);

    void setRequestBadExtension(const SipMessage* request,
                                const char* unsuportedExtensions);

    void setRequestBadAddress(const SipMessage* request);

    void setRequestBadVersion(const SipMessage* request);

    void setRequestBadRequest(const SipMessage* request);

    void setRequestBadUrlType(const SipMessage* request);

    void setRequestBadContentEncoding(const SipMessage* request,
                             const char* allowedEncodings);

    void setInviteRingingData(const char* fromAddress, const char* toAddress,
                              const char* callId,
                              int sequenceNumber = 1);

    void setInviteRingingData(const SipMessage* inviteRequest,
                              const char* localContact = NULL);

    void setQueuedResponseData(const SipMessage* inviteRequest);

    void setForwardResponseData(const SipMessage* inviteRequest,
                                const char* forwardAddress);

    void setInviteBusyData(const char* fromAddress, const char* toAddress,
                       const char* callId,
                       int sequenceNumber = 1);

    void setBadTransactionData(const SipMessage* inviteRequest);

    void setBadSubscriptionData(const SipMessage* inviteRequest);

    void setLoopDetectedData(const SipMessage* inviteRequest);

    void setInviteBusyData(const SipMessage* inviteRequest);

    void setInviteOkData(const char* fromAddress,
                         const char* toAddress,
                         const char* callId,
                         const SdpBody* inviteSdp,
                         const char* rtpAddress,
                         int rtpAudioPort,
                         int rtcpAudioPort,
                         int rtpVideoPort,
                         int rtcpVideoPort,
                         int numRtpCodecs,
                         SdpCodec* rtpCodecs[],
                         SdpSrtpParameters& srtpParams,
                         int sequenceNumber = 1,
                         const char* localContact = NULL);

    void setInviteOkData(const SipMessage* inviteRequest,
                         const char* rtpAddress,
                         int rtpAudioPort,
                         int rtcpAudioPort,
                         int rtpVideoPort,
                         int rtcpVideoPort,
                         int numRtpCodecs,
                         SdpCodec* rtpCodecs[],
                         SdpSrtpParameters& srtpParams,
                         int maxSessionExpiresSeconds,
                         const char* localContact = NULL);

    void setByeErrorData(const SipMessage* byeRequest);

    /// set the body of a response to contain the received request as message/sipfrag
    void setRequestDiagBody(SipMessage request);

    void setReferOkData(const SipMessage* referRequest);

    void setReferDeclinedData(const SipMessage* referRequest);

    void setReferFailedData(const SipMessage* referRequest);

    /// Builds a response with the codes and a message/sipfrag body containing the request headers.
    void setDiagnosticSipFragResponse(const SipMessage& message,
                                      int               responseCode,
                                      const char*       responseText,
                                      int               warningCode,
                                      const char*       warningText,
                                      const UtlString&  address
                                      ///< warn-agent: hostport or token, *not* URI
       );

    //@}


    //! @name Specialized header field accessors.
    //! Using BOTTOM_SUBFIELD as the subfieldIndex returns the last subfield
    //@{
    UtlBoolean getFieldSubfield(const char* fieldName, int subfieldIndex,
                                UtlString* subfieldValue) const;

    // Modify the value of a specific subfield
    UtlBoolean setFieldSubfield(const char* fieldName,
                                int addressIndex,
                                const UtlString& newSubfieldValue);

    // Get the Contact field, but in URI (addr-spec) format (despite
    // that the Contact: header value has name-addr format).
    UtlBoolean getContactUri(int addressIndex, UtlString* uri) const;

    // Get the Contact field as a string (as the value appears in the
    // header).
    UtlBoolean getContactField(int addressIndex,
                               UtlString& contactField) const;

    UtlBoolean getContactEntry(int addressIndex,
                               UtlString* uriAndParameters) const;

    UtlBoolean getContactAddress(int addressIndex,
                                 UtlString* contactAddress,
                                 int* contactPort,
                                 UtlString* protocol,
                                 UtlString* user = NULL,
                                 UtlString* userLabel = NULL) const;

    void setViaFromRequest(const SipMessage* request);

    /// fills in parameters in topmost via based on actual received information.
    void setReceivedViaParams(const UtlString& fromIpAddress, ///< actual sender ip
                              int              fromPort       ///< actual sender port
                              );

    void addVia(const char* domainName,
                int port,
                const char* protocol,
                const char* branchId = NULL,
                const bool bIncludeRport = false) ;

    void addViaField(const char* viaField, UtlBoolean afterOtherVias = TRUE);

    // Set the specified Via-parameter to the specified value on the
    // top (first) Via in the message.
    void setTopViaTag(const char* tagValue,
                      const char* tagName = "received");

    // Set the specified Via-parameter to the specified value on the
    // specified Via in the message.
    UtlBoolean setViaTag(const char* tagValue,
                         const char* tagName,
                         int subFieldIndex);

    void setCallIdField(const char* callId = NULL);

    void setCSeqField(int sequenceNumber, const char* method);
    void incrementCSeqNumber();

    void setFromField(const char* fromField);

    void setFromField(const char* fromAddress, int fromPort,
                      const char* fromProtocol = NULL,
                      const char* fromUser = NULL,
                      const char* fromLabel = NULL);
    void setRawToField(const char* toField);

    void setRawFromField(const char* fromField);

    void setToField(const char* toAddress, int toPort,
                    const char* fromProtocol = NULL,
                    const char* toUser = NULL,
                    const char* toLabel = NULL);
    void setToFieldTag(const char* tagValue);

    void setToFieldTag(int tagValue);

    void setFromFieldTag(const char* tagValue);

    void setExpiresField(int expiresInSeconds);

    void setAcceptField( const char* acceptValue );

    void setAcceptEncodingField( const char* acceptEncodingValue );

    void setAcceptLanguageField( const char* acceptLanguageValue );

    void setRequireField( const char* requireValue );

    void setRetryAfterField( const char* retryAfterValue );

    void setUnsupportedField( const char* unsupportedValue );

    void setMinExpiresField(int minimumExpiresInSeconds);

    void setContactField(const char* contactField, int index = 0);

    void setRequestDispositionField(const char* dispositionField);

    void addRequestDisposition(const char* dispositionToken);

    /// Set the Warning header field.
    void setWarningField(int code, ///< warn-code
                         const char* hostname,
                         ///< warn-agent: hostport or token, *not* URI
                         const char* text
                         ///< warn-text (will be quoted in Warning header)
       );

    void getFromLabel(UtlString* fromLabel) const;

    void getToLabel(UtlString* toLabel) const;

    /// Get the From header as a character string.
    void getFromField(UtlString* fromField) const;

    /// Get the From header as a string containing the URI, that is, an addr-spec.
    void getFromUri(UtlString* uri) const;

    /// Get the From header as a name-addr.
    void getFromUrl(Url& url) const;

    void getFromAddress(UtlString* fromAddress, int* fromPort, UtlString* protocol,
                        UtlString* user = NULL, UtlString* userLabel = NULL,
                        UtlString* tag = NULL) const;

    UtlBoolean getResponseSendAddress(UtlString& address,
                                     int& port,
                                     UtlString& protocol) const;

    static void convertProtocolStringToEnum(const char* protocolString,
                         OsSocket::IpProtocolSocketType& protocolEnum);

    static void convertProtocolEnumToString(OsSocket::IpProtocolSocketType protocolEnum,
                                            UtlString& protocolString);

    UtlBoolean getWarningCode(int* warningCode, int index = 0) const;

    // Retrieves the index-th Via: header as it appears in the message,
    // but does not parse Via: headers for ",".
    // You probably want to use getViaFieldSubField().
    UtlBoolean getViaField(UtlString* viaField, int index) const;

    // Retrieves the index-th logical Via: header value, folding together
    // all the Via: headers and parsing ",".
    // Using BOTTOM_SUBFIELD as the subfieldIndex returns the last subfield
    UtlBoolean getViaFieldSubField(UtlString* viaSubField, int subFieldIndex) const;

    void getTopVia(UtlString* viaAddress,
                   int* viaPort,
                   UtlString* protocol,
                   int* recievedPort = NULL,
                   UtlBoolean* receivedSet = NULL,
                   UtlBoolean* maddrSet = NULL,
                   UtlBoolean* receivedPortSet = NULL) const;

    UtlBoolean removeTopVia();

    /// Get the To header as a character string.
    void getToField(UtlString* toField) const;

    /// Get the To header as a string containing the URI, that is, an addr-spec.
    void getToUri(UtlString* uri) const;

    /// Get the To header as a name-addr.
    void getToUrl(Url& url) const;

    void getToAddress(UtlString* toAddress,
                      int* toPort,
                      UtlString* protocol,
                      UtlString* user = NULL,
                      UtlString* userLabel = NULL,
                      UtlString* tag = NULL) const;

    void getCallIdField(UtlString* callId) const;

    void getDialogHandle(UtlString& dialogHandle) const;
    void getDialogHandleReverse(UtlString& dialogHandle) const;

    void getReferencesField(UtlString* references) const;

    UtlBoolean getCSeqField(int* sequenceNum, UtlString* sequenceMethod) const;

    UtlBoolean getRequireExtension(int extensionIndex, UtlString* extension) const;

    UtlBoolean getProxyRequireExtension(int extensionIndex, UtlString* extension) const;

    void addRequireExtension(const char* extension);

    UtlBoolean getContentEncodingField(UtlString* contentEncodingField) const;

    /// Retrieve the event type, id, and other params from the Event Header
    UtlBoolean getEventField(UtlString* eventType,
                             UtlString* eventId = NULL, //< set to the 'id' parameter value if not NULL
                             UtlHashMap* params = NULL  //< holds parameter name/value pairs if not NULL
                             ) const;

    UtlBoolean getEventField(UtlString& eventField) const;

    void setEventField(const char* eventField);

    UtlBoolean getExpiresField(int* expiresInSeconds) const;

    UtlBoolean getRequestDispositionField(UtlString* dispositionField) const;

    UtlBoolean getRequestDisposition(int tokenIndex,
                                    UtlString* dispositionToken) const;

    UtlBoolean getSessionExpires(int* sessionExpiresSeconds) const;

    void setSessionExpires(int sessionExpiresSeconds);

    UtlBoolean getSupportedField(UtlString& supportedField) const;

    void setSupportedField(const char* supportedField);

    //! Test whether "token" is present in the Supported: header.
    UtlBoolean isInSupportedField(const char* token) const;

    UtlBoolean getRequireField(UtlString& requireField) const;

    //! Test whether "token" is present in the Require: header.
    UtlBoolean isInRequireField(const char* token) const;

    //! Test whether "token" is present in the specified "header" field.
    UtlBoolean isInSpecifiedHeaderField(const char* token, const char* header) const;

    //! Get the SIP-IF-MATCH field from the PUBLISH request
    UtlBoolean getSipIfMatchField(UtlString& sipIfMatchField) const;

    //! Set the SIP-IF-MATCH field for a PUBLISH request
    void setSipIfMatchField(const char* sipIfMatchField);

    //! Get the SIP-ETAG field from the response to a PUBLISH request
    UtlBoolean getSipETagField(UtlString& sipETagField) const;

    //! Set the SIP-ETAG field in a response to the PUBLISH request
    void setSipETagField(const char* sipETagField);

    const UtlString& getInterfaceIp() const;
    int getInterfacePort() const;
    void setInterfaceIpPort(const char* szIP, int port);


    //@}

    //! @name SIP Routing header field accessors and manipulators
    //@{
    UtlBoolean getMaxForwards(int& maxForwards) const;

    void setMaxForwards(int maxForwards);

    void decrementMaxForwards();

    UtlBoolean getRecordRouteField(int index,
                                  UtlString* recordRouteField) const;

    UtlBoolean getRecordRouteUri(int index, UtlString* recordRouteUri) const;

    void setRecordRouteField(const char* recordRouteField, int index);

    void addRecordRouteUri(const char* recordRouteUri);

    // isClientMsgStrictRouted returns whether or not a message
    //    is set up such that it requires strict routing.
    //    This is appropriate only when acting as a UAC
    UtlBoolean isClientMsgStrictRouted() const;

    UtlBoolean getRouteField(UtlString* routeField) const;

    UtlBoolean getRouteUri(int index, UtlString* routeUri) const;

    void addRouteUri(const char* routeUri);

    void addLastRouteUri(const char* routeUri);

    UtlBoolean getLastRouteUri(UtlString& routeUri,
                              int& lastIndex);

    UtlBoolean removeRouteUri(int index, UtlString* routeUri);

    void setRouteField(const char* routeField);

    UtlBoolean buildRouteField(UtlString* routeField) const;

    /// That method is used to add to a message a special 'NAT' route that can
    /// be used to alter the destination of the message.  This route is
    /// carried in the X-sipX-Nat-Route header.  More specifically,
    /// if present in a message, the NAT route takes precedence over the
    /// Request-URI and specifies the endpoint that will be the recipient
    /// of the message provided that the message contains no Route: headers.
    ///
    /// Notes:
    ///  1- NAT route takes precedence over the message's Request-URI
    ///  2- The message's Route set takes precedence over the NAT route
    ///  3- The stack removes any NAT route headers from all incoming messages
    ///     to remove the possibility that malicious endpoints exploit that
    ///     functionality to steer their messages to endpoints (PSTN gateways)
    ///     that they normally do not have access to.
    ///  4- The stack does not remove the NAT route from outgoing messages to
    ///     allow for easier troubleshooting using network traces.
    void setSipXNatRoute(const char* routeUri);

    bool getSipXNatRoute(UtlString* uriString);

    void removeSipXNatRoute( void );

    /// Adjust route values as required when receiving at a proxy.
    void normalizeProxyRoutes(const SipUserAgent* sipUA, ///< used to check isMyHostAlias
                              Url& requestUri,           ///< returns normalized request uri
                              UtlSList* removedRoutes = NULL // route headers popped
                              );
    /**<
     * Check the request URI and the topmost route
     *   - Detect and correct for any strict router upstream
     *     as specified by RFC 3261 section 16.4 Route Information Preprocessing
     *   - Pop off the topmost route until it is not me
     *
     * If the removedRoutes is non-NULL, then any removed routes are returned
     *   on this list (in the order returned - topmost first) as UtlString objects.
     *   The caller is responsible for deleting these UtlStrings (a call to
     *   removedRoutes->destroyAll will delete them all).
     */

    //@}

    UtlBoolean getPathUri(int index, UtlString* pathUri) const;

    /// Add a URI to the Path header at the beginning.
    void addPathUri(const char* routeUri
                    ///< URI or a name-addr with <...>
       );

    /// Add a URI to the Path header at the end.
    void addLastPathUri(const char* pathUri
                        ///< URI or a name-addr with <...>
       );

    //! @name Call control header field accessors
    //@{
    //! Deprecated
    UtlBoolean getAlsoUri(int index, UtlString* alsoUri) const;
    //! Deprecated
    UtlBoolean getAlsoField(UtlString* alsoField) const;
    //! Deprecated
    void setAlsoField(const char* alsoField);
    //! Deprecated
    void addAlsoUri(const char* alsoUri);

    void setRequestedByField(const char* requestedByUri);

    UtlBoolean getRequestedByField(UtlString& requestedByField) const;

    void setReferToField(const char* referToField);

    UtlBoolean getReferToField(UtlString& referToField) const;

    void setReferredByField(const char* referredByField);

    UtlBoolean getReferredByField(UtlString& referredByField) const;

    UtlBoolean getReferredByUrls(UtlString* referrerUrl = NULL,
                      UtlString* referredToUrl = NULL) const;

    void setAllowField(const char* allowField);

    UtlBoolean getAllowField(UtlString& allowField) const;

    void setAllowEventsField(const char* allowField);

    UtlBoolean getAllowEventsField(UtlString& allowField) const;

    UtlBoolean getReplacesData(UtlString& callId,
                              UtlString& toTag,
                              UtlString& fromTag) const;

    /// @returns true if the message has either a User-Agent or Server header
    bool hasSelfHeader() const;

    // SERVER-header accessors
    void getServerField(UtlString* serverFieldValue) const;
    void setServerField(const char* serverField);

    //@}

    // This method is needed to cover the symetrical situation which is
    // specific to SIP authorization (i.e. authentication and authorize
    // fields may be in either requests or responses
    UtlBoolean verifyMd5Authorization(const char* userId,
                                      const char* password,
                                      const char* nonce,
                                      const char* realm,
                                      const char* uri = NULL,
                                      enum HttpEndpointEnum authEntity = SERVER) const;

    //! @name DNS SRV state accessors
    /*! \note this will be deprecated
     */
    //@{
        //SDUA
    UtlBoolean getDNSField(UtlString * Protocol,
                          UtlString * Address,
                          UtlString * Port) const;
    void setDNSField( const char* Protocol,
                     const char* Address,
                     const char* Port);

    void clearDNSField();
    //@}

    void setReasonField(const char* reasonString);

    //! Accessor to store transaction reference
    /*! \note the transaction may be NULL
     */
    void setTransaction(SipTransaction* transaction);

    //! Accessor to get transaction reference
    /*! \note the transaction may be NULL
     */
    SipTransaction* getSipTransaction() const;

/* ============================ INQUIRY =================================== */

    //! Returns TRUE if this a SIP response
    //! as opposed to a request.
    UtlBoolean isResponse() const;

    //! @ Transaction and session related inquiry methods
    //@{
    UtlBoolean isSameMessage(const SipMessage* message,
                            UtlBoolean responseCodesMustMatch = FALSE) const;

    //! Is message part of a server or client transaction?
    /*! \param isOutgoing - the message is to be sent as opposed to received
     */
    UtlBoolean isServerTransaction(UtlBoolean isOutgoing) const;

    //! Returns TRUE if *message is part of the same dialog as *this.
    //! Function uses RFC 3261 rules for matching dialogs. If no
    //! to/from tags are found, the function will compare the entire
    //! To/From fields (backwards compatibility with RFC 2543).
    //! NOTE:
    //! The function compares the To fields of the two messages,
    //! and the From fields of the two messages.  If *message would
    //! have these fields swapped relative to *this, the caller is
    //! responsible for swapping the To/From fields first.
    UtlBoolean isSameSession(const SipMessage* message) const;

    //! Returns TRUE if the URI tags match. If we are comparing
    //! URIs from To fields (comparingToUrl == TRUE), then the
    //! function takes into account that dialog-forming messages have
    //! no To-tags, so a URI with no tag matches any URI.
    //! If comparingToUrl == FALSE and no tags are present in both URIs
    //! then the function will compare the entire URIs (RFC 2543).
    static UtlBoolean isSameSession(const Url& firstUrl,
                                    const Url& secondUrl,
                                    UtlBoolean comparingToUrl = FALSE);

    UtlBoolean isResponseTo(const SipMessage* message) const;
    UtlBoolean isAckFor(const SipMessage* inviteResponse) const;

    //SDUA
    UtlBoolean isInviteFor(const SipMessage* inviteRequest) const;
    UtlBoolean isSameTransaction(const SipMessage* message) const;
    //@}

    //
    UtlBoolean isRequestDispositionSet(const char* dispositionToken) const;

    UtlBoolean isRequireExtensionSet(const char* extension) const;

    //! Indicates whether or not the SIP message can accept a
    //! Record-route header.  Applications wishing to add a
    //! Record-route header to a SIP message should call this
    //! method first to see if the SIP method allows it.
    UtlBoolean isRecordRouteAccepted( void ) const;

    //! Is this a header parameter we want to allow users or apps. to
    //  pass through in the URL
    static UtlBoolean isUrlHeaderAllowed(const char*);

    //! Does this header allow multiple values, or only one.
    static UtlBoolean isUrlHeaderUnique(const char*);

    static void parseViaParameters( const char* viaField
                                   ,UtlContainer& viaParameterList
                                   );

    /// This is here to block access to the HTTP method - not valid for SIP
    virtual void useChunkedBody(bool useChunked);

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    static SdpBody* convertToSdpBody(const HttpBody* httpBody);

    SipTransaction* mpSipTransaction;

    UtlString mInterfaceIp;
    int       mInterfacePort;

    //SDUA
    UtlString m_dnsProtocol ;
    UtlString m_dnsAddress ;
    UtlString m_dnsPort ;

    // Class for the singleton object that carries the field properties
    class SipMessageFieldProps
       {
         private:
          friend class SipMessage;

          SipMessageFieldProps();
          ~SipMessageFieldProps();

          UtlHashBag mShortFieldNames;
          UtlHashBag mLongFieldNames;
          // Headers that may not be referenced in a URI header parameter.
          UtlHashBag mDisallowedUrlHeaders;
          // Headers that do not take a list of values.
          UtlHashBag mUniqueUrlHeaders;

          void initNames();
          void initDisallowedUrlHeaders();
          void initUniqueUrlHeaders();
       };

    // Singleton object to carry the field properties.
    static SipMessageFieldProps sSipMessageFieldProps;

};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipMessage_h_
