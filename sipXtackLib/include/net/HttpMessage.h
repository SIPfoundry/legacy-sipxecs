//
// Copyright (C) 2007, 2010 Avaya, Inc., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
////////////////////////////////////////////////////////////////////////
//////


#ifndef _HttpMessage_h_
#define _HttpMessage_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <os/OsDefs.h>

#include <net/HttpBody.h>
#include <net/NameValuePair.h>
#include <os/OsSocket.h>
#include <os/OsTimeLog.h>
#include <os/OsMsgQ.h>
#include <utl/UtlHashMap.h>

// DEFINES
#define HTTP_NAME_VALUE_DELIMITER ':'
#define HEADER_LINE_PART_DELIMITER ' '
#define END_OF_LINE_DELIMITER "\r\n"

#define HTTP_DEFAULT_SOCKET_BUFFER_SIZE 10240

#define HTTP_LONG_INT_CHARS 20

#define HTTP_PROTOCOL_VERSION     "HTTP/1.0"
#define HTTP_PROTOCOL_VERSION_1_1 "HTTP/1.1"

// HTTP Methods
#define HTTP_GET_METHOD "GET"
#define HTTP_DELETE_METHOD "DELETE"
#define HTTP_PUT_METHOD "PUT"
#define HTTP_POST_METHOD "POST"

// Response codes and text
#define HTTP_OK_CODE 200
#define HTTP_OK_TEXT "OK"
#define HTTP_MOVED_PERMANENTLY_CODE 301
#define HTTP_MOVED_PERMANENTLY_TEXT "Moved Permanently"
#define HTTP_MOVED_TEMPORARILY_CODE 302
#define HTTP_MOVED_TEMPORARILY_TEXT "Moved Temporatily"
#define HTTP_UNAUTHORIZED_CODE 401
#define HTTP_UNAUTHORIZED_TEXT "Unauthorized"
#define HTTP_FORBIDDEN_CODE 403
#define HTTP_FORBIDDEN_TEXT "Forbidden"
#define HTTP_FILE_NOT_FOUND_CODE 404
#define HTTP_FILE_NOT_FOUND_TEXT "File Not Found"
#define HTTP_PROXY_UNAUTHORIZED_CODE 407
#define HTTP_PROXY_UNAUTHORIZED_TEXT "Proxy Authentication Required"
#define HTTP_SERVER_ERROR_CODE 500
#define HTTP_SERVER_ERROR_TEXT "Server Error"
#define HTTP_UNSUPPORTED_METHOD_CODE 501
#define HTTP_UNSUPPORTED_METHOD_TEXT "Not Implemented"
#define HTTP_OUT_OF_RESOURCES_CODE 503
#define HTTP_OUT_OF_RESOURCES_TEXT "Out of Resources"

// Field names
#define HTTP_ACCEPT_LANGUAGE_FIELD "ACCEPT-LANGUAGE"
#define HTTP_ACCEPT_ENCODING_FIELD "ACCEPT-ENCODING"
#define HTTP_AUTHORIZATION_FIELD "AUTHORIZATION"
#define HTTP_DATE_FIELD "DATE"
#define HTTP_CONTENT_DISPOSITION_FIELD "CONTENT-DISPOSITION"
#define HTTP_CONTENT_TRANSFER_ENCODING_FIELD "CONTENT-TRANSFER-ENCODING"
#define HTTP_TRANSFER_ENCODING_FIELD "TRANSFER-ENCODING"
#define HTTP_CONTENT_LENGTH_FIELD "CONTENT-LENGTH"
#define HTTP_CONTENT_TYPE_FIELD "CONTENT-TYPE"
#define HTTP_CONTENT_ID_FIELD "CONTENT-ID"
#define HTTP_LOCATION_FIELD "LOCATION"
#define HTTP_PROXY_AUTHENTICATE_FIELD "PROXY-AUTHENTICATE"
#define HTTP_PROXY_AUTHORIZATION_FIELD "PROXY-AUTHORIZATION"
#define HTTP_REFRESH_FIELD "REFRESH"
#define HTTP_USER_AGENT_FIELD "USER-AGENT"
#define HTTP_WWW_AUTHENTICATE_FIELD "WWW-AUTHENTICATE"
#define HTTP_HOST_FIELD  "HOST"
#define HTTP_ACCEPT_FIELD "ACCEPT"
#define HTTP_CONNECTION_FIELD "CONNECTION"

// Authentication Constants
//    these are by specification case-independant tokens,
//    but we always send the case as used in the examples in
//    the spec (rfc2617)
#define HTTP_BASIC_AUTHENTICATION "Basic"
#define HTTP_DIGEST_AUTHENTICATION "Digest"
#define HTTP_AUTHENTICATION_ALGORITHM_TOKEN "algorithm"
#   define HTTP_MD5_ALGORITHM "MD5"
#   define HTTP_MD5_SESSION_ALGORITHM "MD5-sess"
#define HTTP_AUTHENTICATION_CNONCE_TOKEN     "cnonce"
#define HTTP_AUTHENTICATION_DOMAIN_TOKEN "domain"
#define HTTP_AUTHENTICATION_REALM_TOKEN     "realm"
#define HTTP_AUTHENTICATION_NONCE_TOKEN     "nonce"
#define HTTP_AUTHENTICATION_NONCE_COUNT_TOKEN "nc"
#define HTTP_AUTHENTICATION_OPAQUE_TOKEN    "opaque"
#define HTTP_AUTHENTICATION_QOP_TOKEN  "qop"
#   define HTTP_QOP_AUTH "auth"
#   define HTTP_QOP_AUTH_INTEGRITY "auth-int"
#define HTTP_AUTHENTICATION_USERNAME_TOKEN  "username"
#define HTTP_AUTHENTICATION_URI_TOKEN       "uri"
#define HTTP_AUTHENTICATION_RESPONSE_TOKEN  "response"
#define HTTP_QOP_AUTH_DELIMITER ","

// HTTP_CONTENT_TRANSFER_ENCODING types
#define HTTP_CONTENT_TRANSFER_ENCODING_BINARY "binary"
#define HTTP_CONTENT_TRANSFER_ENCODING_BASE64 "base64"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const int HttpMessageRetries = 2;
const ssize_t HTTP_NOT_FOUND = -1;

// STRUCTS

// FORWARD DECLARATIONS
class UtlString;
class Url;
class HttpMessage;
class OsConnectionSocket;

// TYPEDEFS
//! Callback method used as part of HttpMessage::get.
//! If the callback method returns FALSE, then the retrieve is aborted.
typedef UtlBoolean (*GetDataCallbackProc)(char* pData,
                                         ssize_t iLength,
                                         void* pOptionalData,
                                         HttpMessage* pMsg);


//! HTTP message container
/*! This class is the container with methods for manipulation of a generic
 * HTTP message.  The message is composed of three parts:
 * -# The first header line
 * -# header fields and values (RFC 822 style)
 * -# the optional body (HttpBody)
 * \par
 * The first header line is composed of three white space delimited
 * parts.  The meaning of these parts depends upon whether the
 * the message is a request or a response.  Aside from the first
 * header line, the syntactic and symantic differences are very
 * message specific so a design decision was made to \a not use
 * different subclasses for requests and responses.  This also
 * avoids the multiple inheritance complexity that occurs if you
 * create subclasses for requests, responses and other message
 * types like SipMessage which also have requests and responses.
 * \par
 * The first header line parts can be accessed as follows:
 * - Requests:
 *    -# the request method is retreived via getRequestMethod()
 *    -# the request URI is retreived via getRequestUri()
 *    -# the request protocol is retrieved via getRequestProtocol()
 * - Responses:
 *    -# the response protocol is retrieved via getResponseProtocol()
 *    -# the response code is retrieved via getResponseStatusCode()
 *    -# the response status string is retrieved via getResponseStatusText()
 * \par
 * There are generic getters and setters for the header fields and values
 * However the design philosophy has been to create specialized get and
 * set methods for fields that require any special handling or parsing
 * (i.e. things that are more than opaque tokens or strings).  The generic
 * accessor methods are:
 * - getters:
 *     - getHeaderValue()
 * - setters:
 *     - setHeaderValue()
 *     - addHeaderField()
 *     - insertHeaderField()
 * \par
 * The final part of the HttpMessage is the body.  The body is
 * optional and may not be present.  The accessors for the body
 * (single or multipart) is getBody() and setBody()
 */
class HttpMessage : public UtlContainableAtomic
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

    enum HttpEndpointEnum
    {
        SERVER = 0,
        PROXY
    };

    enum AuthQopValues
    {
        AUTH_QOP_EMPTY = 0,
        AUTH_QOP_HAS_AUTH,
        AUTH_QOP_NOT_SUPPORTED   // Must be last (largest number) put supported values ahead of this line
    };

/* ============================ CREATORS ================================== */

    //! Construct from a string
        HttpMessage(const char* messageBytes = NULL,
                ssize_t byteCount = HTTP_NOT_FOUND);

    //! Construct from reading from the given socket
        HttpMessage(OsSocket* inSocket,
                ssize_t bufferSize = HTTP_DEFAULT_SOCKET_BUFFER_SIZE);

        //!Copy constructor
        HttpMessage(const HttpMessage& rHttpMessage);

    //! Assignment operator
    HttpMessage& operator=(const HttpMessage& rhs);

        //! Destructor
        virtual
        ~HttpMessage();

    /**
     * Get the ContainableType for a UtlContainable-derived class.
     */
    virtual UtlContainableType getContainableType() const;

    /** Class type used for runtime checking */
    static const UtlContainableType TYPE;

/* ============================ MANIPULATORS ============================== */

    //! Do an HTTP GET on the given URL
    /*! \param httpUrl - the url to get from the HTTP server.  The URL
     *         may contain a password & user id.
     * \param maxWaitMillSeconds - the maximum time to wait for the response
     * \param bPersistent - use persistent connections if true
     */
    int get(Url& httpUrl,
            int maxWaitMilliSeconds,
            bool bPersistent=true);

    //! Do an HTTP GET on the given URL
    /*! \param httpUrl - the url to get from the HTTP server.  The URL may contain a password & user id.  Note the only thing that may be required of the URL is the host and port as well as the user ID and password if there is a authentication challenge.
     * \param request - the complete HTTP request that will be sent to the HTTP server including the body.
     * \param maxWaitMillSeconds - the maximum time to wait for the response
     * \param bPersistent - use persistent connections if true
     */
    int get(Url& httpUrl,
            HttpMessage& request,
            int maxWaitMilliSeconds,
            bool bPersistent=false);


    //!Perform an HTTP GET on the specified URL and pass data to the
    //! specified callbackProc.
    /*!
     * \note If you specify a socket pointer, YOU ARE RESPONSIBLE for
     *       closing and deleting the socket!
     *
     * \param httpUrl - The url to get from the HTTP server.  The URL may
     *        contain a password & user id.
     * \param maxWaitMillSeconds - The maximum time to wait for the response.
     * \param callbackProc - Callback routine that will be invoked with the
     *        body data.
     * \param optionalData - Optional data passed to callbackProc
     * \param socket - Socket used to fetch data.  This is set before any
     *        data is pumped, however, if you specify this socket, it is
     *        the caller's responsibility to delete the OsSocket.
     */
    OsStatus get(Url& httpUrl,
                 int maxWaitMilliSeconds,
                 GetDataCallbackProc callbackProc,
                 void* optionalData = NULL,
                 OsConnectionSocket** socket = NULL) ;

    //! Read HTTP message from socket into the HttpMessage object.
    /*! Convenience function to read from a socket
     * \param inSocket - socket from which to read data
     * \param bufferSize - this is the size of the chunks of data to
     *   be read off the socket.  (It is not the total size of the
     *   message to be read.)
     * \param externalBuffer - a UtlString into which the HTTP message
     *   text will be accumulated and via which the text will be
     *   returned to the caller (in addition to returning it as the
     *   contents of the HttpMessage object).  If this is NULL, the
     *   message text is not returned.
     *   If externalBuffer contains a string, it is taken as the
     *   initial part of the HTTP message, to be followed by data read
     *   from inSocket.
     * \param maxContentLength - this is the maximum message size that
     *   will be read.  If the message content-length is larger, the
     *   socket is closed and the buffer is cleared.  (This is to protect
     *   against abusive senders.)
     * \return the number of bytes in the message that was read,
     *   or 0 to indicate an error.  If it is less than
     *   externalBuffer.length(), the remaining bytes are the start of
     *   another message.
     */
    int read(OsSocket* inSocket,
             ssize_t bufferSize = HTTP_DEFAULT_SOCKET_BUFFER_SIZE,
             UtlString* externalBuffer = NULL,
             int maxContentLength = 24000000);

    //! Will read bytes off the socket until the header of the message is
    //! believed received.
    int readHeader(OsSocket* inSocket, UtlString& buffer);

    //! Read the body of the specified data and deliever via the
    //! callback proc.
    int readBody(OsSocket* inSocket, int length, GetDataCallbackProc callbackProc, void* optionalData);

    //! Write message to socket
    /*! Convenience function to write to a socket
     * \return TRUE if the message was written successfully
     */
    UtlBoolean write(OsSocket* outSocket) const;

    /// Write just the headers and the terminating CRLF to outSocket
    UtlBoolean writeHeaders(OsSocket* outSocket) const;
    /**<
     * This method must only be used if the message has been marked as using
     * chunked encoding (by calling useChunkedBody with true).
     *
     * The chunks for the body should then be written by calling
     * writeChunk, followed by a call to writeEndOfChunks when all
     * chunks have been written.
     *
     * @return TRUE if the message was written successfully
     */

    /// Pass 'true' to use the 'chunked' transfer-encoding to delimit the body
    virtual void useChunkedBody(bool useChunked);
    /**<
     * It is an error to call this method if a body has already been attached to the message.
     *
     * The headers for the message must first be written using writeHeaders.
     * The chunks for the body should then be written by calling
     * writeChunk, followed by a call to writeEndOfChunks when all
     * chunks have been written.
     */

    /// Write a partial body using chunked encoding.
    UtlBoolean writeChunk(OsSocket* outSocket, const UtlString& chunk);

    /// Write the end-of-chunks marker to terminate a message sent with chunked encoding.
    UtlBoolean writeEndOfChunks(OsSocket* outSocket);

    //! Converts the given token to initial cap capitialization
    //! in the style used in RFC 822 header names
    static void cannonizeToken(UtlString& token);

    //! URL unescape the text to human readable form.
    static void unescape(UtlString& escapedText);

    //! Selectively URL escape the text if they are contained in
    //! tobeEscapedChars.
    static void escapeChars(UtlString& unEscapedText, UtlString& tobeEscapedChars);

    //! URL sscape the text.
    static void escape(UtlString& unEscapedText);

    //! Selectively URL escape the text if the char is tobeEscapedChar.
    static void escapeOneChar(UtlString& unEscapedText, char tobeEscapedChar);

    /*! @name Construction helper methods
     *  These methods are mostly for internal use as utilities
     * to the constructor methods
     */
    //@{
    //! Parse the first line of the request or response
    ssize_t parseFirstLine(const char* messageBytes, ssize_t byteCount);

        //! Parse the message from a byte buffer
        /*! This method reads the top header line, header feilds and
         * the message body if they are present from the buffer assuming
         * HTTP stream format.
     */
    void parseMessage(const char* messageBytes, ssize_t byteCount);

    //! Parses the bytes into a single or multipart body.
    void parseBody(const char* messageBodyBytes, ssize_t byteCount);

    //! returns: the number of bytes parsed
    static ssize_t parseHeaders(const char* headerBytes,
                            ssize_t messageLength,
                            UtlDList& headerNameValues);

    //! returns: the number of bytes in the message buffer which constitue the message header
    /*! The end of the headers is determined by the first blank line
     */
    static ssize_t findHeaderEnd(const char* messageBytes, ssize_t messageLength);

    //@}
/* ============================ ACCESSORS ================================= */

    static int smHttpMessageCount;
    static int getHttpMessageCount();

    const char* getFirstHeaderLine() const;

    //! Set the header line
    /*! \param newHeaderLine - gets copied
    */
    void setFirstHeaderLine(const char* newHeaderLine);

    //! Set the header line
    /*! \param method - method part of header line
     *  \param uri - URI part of header line
     *  \param protocol - protocol part of header line
    */
    void setFirstHeaderLine(const char* method,
                            const char* uri,
                            const char* protocol);

   //! Get part of header line
   /*! \param partIndex - index into space delimited first header line
    * \param part
    */
    void getFirstHeaderLinePart(ssize_t partIndex,
                                UtlString* part,
                                char separator = HEADER_LINE_PART_DELIMITER) const;

    //! @name Transport state accessors
    /*! \note: these are mostly only useful inside servers
     * that keep message state.
     */
    //@{
    // Manipulate transportTimeStamp, which gives the time the message was
    // last sent/resent.
    void setTransportTime(long timeStamp);
    long getTransportTime() const;
    // Set transportTimeStamp to the current time.
    void touchTransportTime();

    //! Used by transport to record the current resend interval (from the last
    //  time the message was sent to the time that the message is scheduled to
    //  be resent). (msec)
    void setResendInterval(int resendMSec);
    int getResendInterval() const;

    // Used by transport to record the number of times the message has
    // been sent/resent.
    int getTimesSent() const;
    void incrementTimesSent();
    void setTimesSent(int times = 0);

    // Used by tranport to record which protocol the message was sent over.
    void setSendProtocol(OsSocket::IpProtocolSocketType protocol = OsSocket::TCP);
    OsSocket::IpProtocolSocketType getSendProtocol() const;

    void setFirstSent();
    void setSendAddress(const char* address, int port);
    void getSendAddress(UtlString* address, int* port) const;

    // Resets the tranport state stuff
    void resetTransport();
    //@}

    //! @name Request callback methods
    /*! These methods are used to setup callback functions and data
     * which gets triggered when a response comes back
     */
    //@{

    //! Gets the queue on which responses from the same transaction
    //! are deposited
    OsMsgQ* getResponseListenerQueue();

    //! Sets the queue on which responses from the same transaction
    //! are deposited
    void setResponseListenerQueue(OsMsgQ* requestListenerQueue);

    //! Gets the data item to pass to the request listener
    void* getResponseListenerData();

    //! Sets the data item to pass to the request listener
    void setResponseListenerData(void* requestListenerData);
    //@}


    //! @name Generic header accessors
    /*! \note many specific header type accessors exist to parse
     * headers which have syntax that is not a simple token or string
     */
    //@{

    //! Find the number of occurrences of header fields with the given name.
    //  Note that this method does not implement the SIP convention of
    //  treating "Header: a, b" as equivalent to "Header: a\r\nHeader:b".
    //  This method will return 1 in the first case and 2 in the second,
    //  even though many of the SIP field accessors will extract two
    //  "values" for "Header" from a message containing either example.
    int getCountHeaderFields(const char* name = NULL) const;

    //! Get the value of the header field
    //! (i.e. second header line).
    /*! \param index - index into the header fields or if name is not null
     *        the index into the set of header fields having the given name
     * \param name - string containing the name of the header field
     * \return read-only string if a header field exists <br>
     *       NULL if no header field is found
     */
    const char* getHeaderValue(int index, const char* name = NULL) const;

    //! Sets the value of an existing header field
    /*! If the given name does not exist it creates a new one
     * \param name - header field name
     * \param newValue - the value which gets set for the field
     * \param index - the index into the number of fields with the
     *         given name
     * \note value is copied
     */
    void setHeaderValue(const char* name, const char* newValue, int index = 0);

    //! Adds a new header line
    /*! \param name - header field name
     * \param value - the value which gets set for the field
     * \note name and value are copied
     */
    void addHeaderField(const char* name, const char* value);


    //! Inserts a new header line, shifting existing headers at and
    //! after the secified location down by one
    /*! \param name - header field name
     * \param value - the value which gets set for the field
     * \param index - the location to insert in the list (first by default)
     * \note name and value are copied
     */
    void insertHeaderField(const char* name,
                          const char* value,
                          int index = 0);


    UtlBoolean removeHeader(const char* name, int index);
    //! Remove a header field and value
    /*! \param name - optional header field name NULL if indexing directly
     * \param index - header number or index into headers having the given
     *        name if provided
     */
    //@}

    //! @name Specialized header field accessors
    //@{
    void setContentType(const char* contentType);

    UtlBoolean getContentType(UtlString* contentType) const;

    void setContentLength(int contentLength);

    int getContentLength() const;

    void setUserAgentField(const char* userAgentFieldValue);

    void getUserAgentField(UtlString* userAgentFieldValue) const;

    void setRefresh(int seconds, const char* refreshUrl = NULL);

    UtlBoolean getDateField(long* epochDate) const;

    //! Sets the date field to the current time
    void setDateField();

    void setAcceptLanguageField(const char* acceptLanaguageFieldValue);

    void getAcceptLanguageField(UtlString* acceptLanaguageFieldValue) const;

    void setAcceptField(const char* acceptFieldValue);

    // Returns TRUE if the Accept header was present.
    UtlBoolean getAcceptField(UtlString& acceptValue) const;

    void setLocationField(const char* locationField);
    //@}

    //! @name Message body accessors
    //@{
    //! Get the body section of the message
    //  May return NULL if the message has no body section, which usually
    //  comes from parsing a message with a zero-length body.
    const HttpBody* getBody() const;

    /**
     * Attach the body section of the message. The body is NOT copied.
     * This message instance destructor will call delete on the body
     * pointer.
     */
    void setBody(HttpBody* newBody);
    //@}

    //! Get the bytes for the compete message
    /*! Suitable for streaming or sending over a socket
     * \param bytes - gets allocated and must be freed
     * \param length - the length of bytes
     */
    void getBytes(UtlString* bytes, ssize_t* length, bool includeBody = true) const;
    //! Get a malloc'ed string containing the text of the message.
    /*! Must be free'd by the caller.  Suitable for use in a debugger.
     */
    char* getBytes() const;
    //! Print the message to stdout (for debugging).
    void debugPrint(void) const;

   //! @name Authentication access methods
   //@{
    UtlBoolean getAuthenticationScheme(UtlString* scheme,
                                       HttpEndpointEnum authorizationEntity
                                       ) const;

    UtlBoolean getAuthenticateData(UtlString* scheme,
                                   UtlString* realm,
                                   UtlString* nonce,
                                   UtlString* opaque,
                                   UtlString* algorithm, // MD5 or MD5-sess
                                   UtlString* qop, // may be multiple values
                                   HttpEndpointEnum authorizationEntity,
                                   unsigned   index = 0
                                   ) const;


    static bool parseAuthenticateData(const UtlString& authenticationField,
                                      UtlString* scheme,
                                      UtlString* realm,
                                      UtlString* nonce,
                                      UtlString* opaque,
                                      UtlString* algorithm, // MD5 or MD5-sess
                                      UtlString* qop,
                                      UtlString* domain
                                      );

    void setAuthenticateData(const char* scheme, const char* realm,
                             const char* nonce, const char* opaque,
                             const char* domain = NULL,
                             enum HttpEndpointEnum authEntity = SERVER);

    void addAuthenticateField(const UtlString&      authenticationField,
                              enum HttpEndpointEnum authType
                              );

    bool getAuthenticateField(int index,
                              enum HttpEndpointEnum authEntity,
                              UtlString& authenticationField
                              ) const;

    // Authorization methods
    static void buildBasicAuthorizationCookie(const char* user,
                                                const char* password,
                                                UtlString* cookie);

    virtual void setRequestUnauthorized(const HttpMessage* request,
                            const char* authenticationScheme,
                            const char* authenticationRealm,
                            const char* authenticationNonce = NULL,
                            const char* authenticationOpaque = NULL,
                            const char* authenticationDomain = NULL);

    
    // Extract the authorization user and the user base (authorization user
    // field less the instrument indicator), as described for
    // HttpMessage::getDigestAuthorizationData.
    UtlBoolean getAuthorizationUser(UtlString* user,
                                    UtlString* userBase) const;

    UtlBoolean getAuthorizationScheme(UtlString* scheme) const;

    UtlBoolean getAuthorizationField(UtlString* authenticationField,
                                     HttpEndpointEnum authorizationEntity) const;

    void setBasicAuthorization(const char* user, const char* password,
                               HttpEndpointEnum authorizationEntity);

    UtlBoolean getBasicAuthorizationData(UtlString* encodedCookie) const;

    UtlBoolean getBasicAuthorizationData(UtlString* userId,
                                        UtlString* password) const;

    UtlBoolean verifyBasicAuthorization(const char* user,
                                   const char* password) const;

    // Digest Authorization
    void setDigestAuthorizationData(const char* user,
                                    const char* realm,
                                    const char* nonce,
                                    const char* uri,
                                    const char* response,
                                    const char* algorithm,
                                    const char* cnonce,
                                    const char* opaque,
                                    const char* qop,
                                    const char* nonceCount,
                                    HttpEndpointEnum authorizationEntity
                                    );

    /// Return data from the index-th (0-origin) Authorization/Proxy-Authorization header.
    // Use Authorization headers if authorizationEntity == SERVER, or
    // Proxy-Authorization headers if authorizationEntity == PROXY.
    // Return TRUE if the index-th value exists -- But it may not contain
    // the mandatory fields, and if it is not Digest type, all returned
    // strings will be empty.
    // Separate the 'user' presented in an authorization header into
    // the real user part (user_base) and the phone instrument identification
    // part (instrument).
    // The header's user value is examined to see if it is of the form
    // "xxx/yyy" (where yyy does not contain '/').  If not, user_base
    // is set to the entire value and instrument is set to NULL.  If
    // it is of that form, user_base is set to xxx, and instrument is
    // set to yyy.
    UtlBoolean getDigestAuthorizationData(UtlString* user,
                                          UtlString* realm = NULL,
                                          UtlString* nonce = NULL,
                                          UtlString* opaque = NULL,
                                          UtlString* response = NULL,
                                          UtlString* uri = NULL,
                                          UtlString* cnonce = NULL,
                                          UtlString* nonceCount = NULL,
                                          UtlString* qop = NULL,
                                          HttpEndpointEnum authorizationEntity = HttpMessage::PROXY,
                                          int index = 0,
                                          UtlString* user_base = NULL,
                                          UtlString* instrument = NULL) const;

    static void buildMd5UserPasswordDigest(const char* user,
                                           const char* realm,
                                           const char* password,
                                           UtlString& userPasswordDigest);

    static void buildMd5Digest(const char* userPasswordDigest,
                               const char* algorithm,
                               const char* nonce,
                               const char* cnonce,
                               const char* nonceCount,
                               const char* qop,
                               const char* method,
                               const char* uri,
                               const char* bodyDigest,
                               UtlString* responseToken);

    UtlBoolean verifyMd5Authorization(const char* userId,
                                      const char* password,
                                      const char* nonce,
                                      const char* realm,
                                      const char* cnonce,
                                      const char* nonceCount,
                                      const char* qop,
                                      const char* thisMessageMethod = NULL,
                                      const char* thisMessageUri = NULL,
                                      enum HttpEndpointEnum authEntity = SERVER) const;

    UtlBoolean verifyMd5Authorization(const char* userPasswordDigest,
                                      const char* nonce,
                                      const char* cnonce,
                                      const char* nonceCount,
                                      const char* qop,
                                      const char* thisMessageMethod = NULL,
                                      const char* thisMessageUri = NULL) const;

    //@}

    //! @name Response first header line access methods
    //@{
    void setResponseFirstHeaderLine(const char* protocol,
                                    int statusCode,
                                    const char* statusText);

    //! Get this response's message protocol
    void getResponseProtocol(UtlString* protocol) const;

    //! Get this response's status code
    int getResponseStatusCode() const;

    //! Get this response's status code text
    void getResponseStatusText(UtlString* text) const;
    //@}



        /*! @name Request first header line access methods
     */
    //@{
    void setRequestFirstHeaderLine(const char* method,
                                   const char* uri,
                                   const char* protocol);

    //! Get this request's method
    void getRequestMethod(UtlString* method) const;

    //! Get this request's URI
    void getRequestUri(UtlString* uri) const;

    //! Get this request's application layer protocol type
    void getRequestProtocol(UtlString* protocol) const;

    //! Change this request's URI
    void changeRequestUri(const char* newUri);
    //@}

     // Time logging
     void logTimeEvent(const char* eventName);
     void dumpTimeLog() const;

/* ============================ INQUIRY =================================== */

     static UtlBoolean isWholeMessage(const char* messageBuffer,
                              int bufferLength,
                              int& numberBytesChecked,
                              int& contentLength);

    UtlBoolean isFirstSend() const;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   UtlDList mNameValues;
   UtlString mFirstHeaderLine;
   UtlBoolean mHeaderCacheClean;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   HttpBody* body;
   bool mUseChunkedEncoding;

   // Note that these "transport parameters" are copied by the
   // assignment and copy constructors.

   // The time at which the message was last sent/resent.
   long transportTimeStamp;
   // The resend timeout that was set at the last resend, which is thus
   // the time from that resend until the resend timer fires.  (msec)
   int lastResendInterval;
   // The transport protocol that is being used to send this message.
   OsSocket::IpProtocolSocketType transportProtocol;
   // The number of times this message has been sent so far.
   int timesSent;

   UtlBoolean mFirstSent;
   UtlString mSendAddress;
   int mSendPort;
   OsMsgQ* mpResponseListenerQueue;
   void* mResponseListenerData;
#ifdef HTTP_TIMELOG
   OsTimeLog mTimeLog;
#endif

   //! Internal utility
   NameValuePair* getHeaderField(int index, const char* name = NULL) const;


};

/* ============================ INLINE METHODS ============================ */

#endif  // _HttpMessage_h_
