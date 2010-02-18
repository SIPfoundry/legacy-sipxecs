//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _HttpServer_h_
#define _HttpServer_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <utl/UtlInt.h>
#include <utl/UtlHashMap.h>
#include <utl/UtlHashBag.h>
#include <os/OsTask.h>
#include <os/OsConfigDb.h>
#include <net/HttpConnection.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class HttpMessage;
class HttpBody;
class OsServerSocket;
class OsConnectionSocket;
class HttpRequestContext;
class HttpService;
class HttpConnection;

/// Implement an HTTP server interface.
/**
 * This class listens on the accept socket passed to the constructor, accepting new
 * connections and creating an HttpConnection object for each.  If the constructor
 * specified that connections should be persistent, the HttpServer maintains the list
 * of open connections and enforces the MAX_PERSISTENT_HTTP_CONNECTIONS limit.
 *
 * Actual HTTP requests are parsed from the HttpConnection by HttpMessage::read,
 * and then the HttpConnection object passes them to HttpServer::processRequest.
 *
 * Prior to attempting to process the request itself, the HttpServer:
 *
 * - Optionally enforces any restrictions on the remote IP address.  If the validIpAddressDB
 *   parameter passed to the HttpServer constructor is non-NULL and not empty, then the
 *   IP address of the requestor must appear in that database, or the request is rejected
 *   with a 403 Forbidden response.
 *
 *   IP addresses are NOT a good authentication mechanism.  See HttpRequestContext for
 *   a description of how to use TLS peer authentication for strong authentication.
 *
 * - Path is checked for any '..' elements and rejected if they are present.
 *
 * - Applies any mappings configured using addUriMap (see addUriMap and mapUri).
 *
 * - Constructs an HttpRequestContext object to provide all the information about the request.
 *
 * There are three ways to handle an HTTP request; the mapped path is used select the
 * processor for the request from among these method in this order:
 *
 * -# Create a static method whose signature matches HttpServer::RequestProcessor and
 *    register the method using addRequestProcessor.  This means of providing an HTTP
 *    service is relatively simple, but provides no context except the request itself.
 *    Using this method, you must specify the exacty path that this processor is
 *    is responsible for.  This mechanism may only be used for GET and POST methods.
 *
 * -# Create a subclass of HttpService and register an object of that class using addHttpService.
 *    The registered object will be called using its version of HttpService::processRequest,
 *    passed the request, an empty response, and the HttpRequestContext.
 *    Using this method, the service is responsible for any extension of the registered
 *    path (see addHttpService).
 *    This means of providing a service is best when you need additional contextual
 *    information in the service object or want to use additional path information.
 *    This mechanism may be used for GET, PUT, DELETE, and POST methods.
 *
 */
class HttpServer : public OsTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

    /// Create an HttpServer interface on an existing socket.
    HttpServer(OsServerSocket* pSocket,    /**< The listening socket on which new connection
                                            *   requests are received.  This may be
                                            *   OsServerSocket or OsSSLServerSocket.
                                            */
               OsConfigDb*     validIpAddressDB = NULL, //< optional requestor address restriction
               bool            bPersistentConnection = false
               );

    virtual ~HttpServer();

    /// OsServerTask main loop implementation.
    virtual int run(void* runArg);

    /// Translate URI prefixes.
    static UtlBoolean mapUri(UtlHashMap& uriMaps,  ///< database of prefix translations.
                             const char* uri,      ///< normalized input request uri path
                             UtlString& mappedUri  ///< result of translation, if any
                             );
    /**<
     * @returns TRUE iff a translation was found
     *
     * The uri is iteratively checked against the database of mappings, stripping one path
     * component (separated by '/') at each iteration.
     * Each database check looks for a match of the (possibly shortened) path, and if an exact
     * match is found, then the matching portion is replaced by the translation from the map
     * and any previously stripped path components are appended to it.
     *
     * This operates only on prefixes - you cannot modify only the middle or the end of a path,
     * and it operates only on separator boundaries.
     * For example, the translation "/a/b" => "/xy" will not modify the uri "/a/bc".
     *
     * If no mappings are found, mappedUri is set to the input uri with no changes.
     */

    /// Add a prefix map translation step for the request URI path.
    void addUriMap(const char* fromUri, ///< normalized path prefix as received in the request URI
                   const char* toUri    ///< translation for the above prefix
                   );
    /**<
     * This provides a mapping to be applied by the mapUri method before the request URI is
     * resolved to a processing mechanism.  Both values MUST begin with '/' and may not end
     * with '/' (at present this is not checked - the translation just doesn't work if you don't
     * follow the rule).
     */

    /// Signature of processor to be passed to addRequestProcessor.
    typedef void RequestProcessor(const HttpRequestContext& requestContext,
                                  const HttpMessage& request,
                                  HttpMessage*& response
                                  );

    /// Specify a RequestProcessor to be called when the path exactly matches the fileUrl.
    void addRequestProcessor(const char* fileUrl,
                             RequestProcessor* requestProcessor
                             );

    /// Specify an HttpService to be called when the path is a prefix match for the fileUrl.
    void addHttpService(const char* fileUrl, /**< path prefix this service is registered for
                                              * Must begin with '/'
                                              * Must not end with '/' unless it is exactly "/"
                                              */
                        HttpService* service ///< service object
                        );
    /**<
     * The fileUrl parameter specifies a prefix that must start with a '/' and must
     * exactly match the initial components of the HTTP Request URI.
     * So, registering "/one" matches "/one/two/three" but not "/" or "/two" or "/onetwo".
     */

    /// Get current http server status
    OsStatus getStatus();

    /// Is the server socket used by this HttpServer ok?
    UtlBoolean isSocketOk() const ;

protected:
    friend class HttpConnection;
    friend class HttpServerTest;

    void processRequest(const HttpMessage& request,    ///< request to be dispatched
                        HttpMessage*& response,        ///< build response in this message
                        OsConnectionSocket* connection ///< meta-data regarding the request
                        );

    UtlBoolean processRequestIpAddr(const UtlString& remoteIp,
                                    const HttpMessage& request,
                                    HttpMessage*& response
                                    );

private:

    static void processPostFile(const HttpRequestContext& requestContext,
                                const HttpMessage& request,
                                HttpMessage*& response
                                );

    static int doPostFile(const HttpRequestContext& requestContext,
                          const HttpMessage& request,
                          HttpMessage*& response,
                          UtlString& status
                          );

    // Error request processors
    static void processNotSupportedRequest(const HttpRequestContext& requestContext,
                                           const HttpMessage& request,
                                           HttpMessage*& response
                                           );

    static void processFileNotFound(const HttpRequestContext& requestContext,
                                    const HttpMessage& request,
                                    HttpMessage*& response
                                    );

    static void createHtmlResponse(int responseCode,
                                   const char* responseCodeText,
                                   const char* htmlBodyText,
                                   HttpMessage*& response
                                   );

    static void testCgiRequest(const HttpRequestContext& requestContext,
                               const HttpMessage& request,
                               HttpMessage*& response
                               );

    void getFile(const char* fileName,
                 HttpBody*& body
                 );

    void putFile(const char* fileName,
                 HttpBody& body
                 );

    UtlBoolean findRequestProcessor(const char* fileUri,
                                    RequestProcessor*& requestProcessor
                                    );

    UtlBoolean findHttpService(const char* fileUri,
                               HttpService*& service
                               );

    void loadValidIpAddrList();

   static const int MAX_PERSISTENT_HTTP_CONNECTIONS;

   OsStatus        httpStatus;
   int             mServerPort;
   OsServerSocket* mpServerSocket;
   OsConfigDb*     mpValidIpAddressDB;
   UtlHashMap      mUriMaps;
   UtlHashMap      mRequestProcessorMethods;
   UtlHashMap      mHttpServices;
   UtlHashBag      mValidIpAddrList;
   UtlBoolean      mbPersistentConnection;
   int             mHttpConnections;
   UtlSList*       mpHttpConnectionList;

   // @cond INCLUDENOCOPY
   // There is no copy constructor.
   HttpServer(const HttpServer& rHttpServer);
   // There is no assignment operator.
   HttpServer& operator=(const HttpServer& rhs);
   // @endcond

};

/* ============================ INLINE METHODS ============================ */

#endif  // _HttpServer_h_
