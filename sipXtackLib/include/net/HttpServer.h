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
#define MAX_PERSISTENT_HTTP_CONNECTIONS  5

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

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class HttpServer : public OsTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   friend class HttpConnection;

/* ============================ CREATORS ================================== */

   HttpServer(OsServerSocket *pSocket, OsConfigDb* userPasswordDb,
                       const char* realm, OsConfigDb* validIpAddressDB = NULL,
                       bool bPersistentConnection = false);
     //:Default constructor

   virtual
   ~HttpServer();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

    virtual int run(void* runArg);

    // Request processors
    static void processPostFile(const HttpRequestContext& requestContext,
                                const HttpMessage& request,
                                HttpMessage*& response);
    static int doPostFile(const HttpRequestContext& requestContext,
                                const HttpMessage& request,
                                HttpMessage*& response,
                                                                UtlString& status);

    static void processFileRequest(const HttpRequestContext& requestContext,
                                const HttpMessage& request,
                                HttpMessage*& response);

    // Error request processors
    static void processNotSupportedRequest(const HttpRequestContext& requestContext,
                                const HttpMessage& request,
                                HttpMessage*& response);
    static void processFileNotFound(const HttpRequestContext& requestContext,
                                const HttpMessage& request,
                                HttpMessage*& response);

    static void processUserNotAuthorized(const HttpRequestContext& requestContext,
                                     const HttpMessage& request,
                                     HttpMessage*& response,
                                     const char* text = 0);

    static void createHtmlResponse(int responseCode, const char* responseCodeText,
                   const char* htmlBodyText, HttpMessage*& response);

    static void testCgiRequest(const HttpRequestContext& requestContext,
                                const HttpMessage& request,
                                HttpMessage*& response);

    static UtlBoolean mapUri(OsConfigDb& configDb, const char* uri, UtlString& mappedUri);

    void addUriMap(const char* fromUri, const char* toUri);

    typedef void RequestProcessor(const HttpRequestContext& requestContext,
                                  const HttpMessage& request,
                                  HttpMessage*& response
                                  );
    
    void addRequestProcessor(const char* fileUrl, RequestProcessor* requestProcessor);

    void addHttpService(const char* fileUrl, HttpService* service);

    /// set permission for access to mapped file names
    void allowFileAccess(bool fileAccess ///< true => allow access, false => disallow access
                         );
    
    void setPasswordDigest(const char* user, const char* password,
                           UtlString& userPasswordDigest);


        void setPasswordDigest(const char* user, const char* passwordDigest);
          //: Sets the password, given an already digested password.



        void getDigest(const char* user, const char* password,
                                   UtlString& userPasswordDigest) ;

        void setPasswordBasic(const char* user, const char* password);

        void removeUser(const char* user, const char* password) ;

        static void constructFileList(UtlString & indexText, UtlString uri, UtlString uriFilename) ;

        //get current http server status
        OsStatus getStatus();


/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    void processRequest(const HttpMessage& request,          ///< request to be dispatched
                        HttpMessage*& response,              ///< build response in this message
                        const OsConnectionSocket* connection ///< for access to security info
                        );

    UtlBoolean processRequestIpAddr(const UtlString& remoteIp,
       const HttpMessage& request,
       HttpMessage*& response);


    UtlBoolean isRequestAuthorized(const HttpMessage& request,
                                  HttpMessage*& response,
                                  UtlString& userId);

    void processPutRequest(const HttpRequestContext& requestContext,
                           const HttpMessage& request,
                           HttpMessage*& response);

    void getFile(const char* fileName, HttpBody*& body);

    void putFile(const char* fileName, HttpBody& body);

    UtlBoolean findRequestProcessor(const char* fileUri,
                                    RequestProcessor*& requestProcessor
                                    );

    UtlBoolean findHttpService(const char* fileUri, HttpService*& service);

    void loadValidIpAddrList();
    
/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   HttpServer(const HttpServer& rHttpServer);
     //:Copy constructor (disabled)
   HttpServer& operator=(const HttpServer& rhs);
     //:Assignment operator (disabled)
   OsStatus httpStatus;
   int mServerPort;
   OsServerSocket* mpServerSocket;
   OsConfigDb * mpUserPasswordDigestDb;
   OsConfigDb * mpUserPasswordBasicDb;
   OsConfigDb * mpValidIpAddressDB;
   OsConfigDb mUriMaps;
   OsConfigDb * mpNonceDb;
   UtlString mRealm;
   UtlHashMap mRequestProcessorMethods;
   UtlHashMap mHttpServices;
   bool       mAllowMappedFiles;
   UtlHashBag mValidIpAddrList;
   UtlBoolean mbPersistentConnection;
   int mHttpConnections;
   UtlSList* mpHttpConnectionList;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _HttpServer_h_
