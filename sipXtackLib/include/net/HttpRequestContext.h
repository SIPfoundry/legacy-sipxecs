//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _HttpRequestContext_h_
#define _HttpRequestContext_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "utl/UtlString.h"
#include "utl/UtlSList.h"
#include "net/HttpMessage.h"

// DEFINES


// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class HttpBody;

/// Provides all the information available on a particular HTTP request.
class HttpRequestContext
{

public:

   /// Keys for the getEnvironmentVariable access routine.
    enum RequestEnvironmentVariables
    {
        HTTP_ENV_RAW_URL = 0,   ///< The full request uri
        HTTP_ENV_UNMAPPED_FILE, ///< The file part of the raw URL
        HTTP_ENV_MAPPED_FILE,   ///< The file part of the url mapped to the real location
        HTTP_ENV_QUERY_STRING,  /**< The query part of the URL (if GET)
                                 *   Note: the individual variables are retreiveable
                                 *   using getCgiVariables */
        HTTP_ENV_SERVER_NAME,   ///< The Server name part of the URL
        HTTP_ENV_REQUEST_METHOD,///< The request method (i.e. GET, PUT, POST)
        HTTP_ENV_USER,          ///< The user name (if this request required authorization)

        HTTP_ENV_LAST ///< a dummy value indicating the last var (MUST BE LAST)
    };

/* ============================ CREATORS ================================== */

   /// Construct the context for an HTTP request.
   HttpRequestContext( const char* requestMethod = NULL
                      ,const char* rawUrl = NULL
                      ,const char* mappedFile = NULL
                      ,const char* serverName = NULL
                      ,const char* userId = NULL
                      ,OsConnectionSocket* connection = NULL
                      );

   /// Destructor
   virtual ~HttpRequestContext();

   /// Extracts the CGI variables from a POST request body.
   void extractPostCgiVariables(const HttpBody& body);

   typedef void (*UnEscapeFunction)(UtlString&);
   /// Parse variables from the requestUri parameters
   static void parseCgiVariables(const char* queryString,
                                 UtlList& cgiVariableList,
                                 const char* pairSeparator = "&",
                                 const char* namValueSeparator = "=",
                                 UtlBoolean nameIsCaseInsensitive = TRUE,
                                 UnEscapeFunction unescape = &HttpMessage::unescape);
   /**<
    * If nameIsCaseInsensitive == TRUE, puts NameValuePairInsensitive's
    * into cgiVariableList rather than NameValuePair's.
    */

   /// Copy constructor
   HttpRequestContext(const HttpRequestContext& rHttpRequestContext);

   /// Assignment operator
   HttpRequestContext& operator=(const HttpRequestContext& rhs);

   /// Tests whether or not the HTTP Method is the give value.
   bool methodIs(const char* method) const;

   /// Get the request URI as normalized and mapped by the HttpServer.
   void getMappedPath(UtlString& path) const;

   /// Get Environment and context variables related to this request
   void getEnvironmentVariable(enum RequestEnvironmentVariables envVariable,
                               UtlString& value
                               ) const;

   /// Get CGI/Form variables provided in this POST or GET request.
   UtlBoolean getCgiVariable(const char* name, UtlString& value, int occurance = 0) const;
   /**<
    * As it is possible to have multiple occurances of a named value
    * the occurance argument indicates which occurance.  The default is the first.
    * @returns TRUE/FALSE if the occurance of the named variable exists
    */

   /// Get the name and value of the variable at the given index
   UtlBoolean getCgiVariable(int index, UtlString& name, UtlString& value) const;

   /// Test whether or not the client connection is encrypted.
   bool isEncrypted() const;

   /// Test whether or not the given name is the SSL client that sent this request.
   bool isTrustedPeer( const UtlString&  peername ///< name of the peer to check
                      ) const;
   /**<
    * This tests the host identity provided by the SSL handshake; it does not
    * test the HTTP user identity if any.
    *
    * @NOTE All subjectAltName values are case-folded to lower case: @see OsSSL::peerIdentity
    *
    * @TODO This should allow selection limited by the name type
    *
    * @returns
    * - true if the connection is SSL and the peername matches a name in the peer certificate.
    * - false if not.
    */

   /// Direct access to socket for the request.
   OsConnectionSocket* socket() const;

protected:

private:

   /// Parse the CGI/form variables from the &,= delineated name, value pairs and unescape the name and values.
   void parseCgiVariables(const char* queryString);

   UtlSList   mCgiVariableList;
   bool       mUsingInsensitive;
   UtlString  mEnvironmentVars[HTTP_ENV_LAST];
   bool       mPeerCertTrusted;
   UtlSList   mPeerIdentities;

   OsConnectionSocket* mConnection;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _HttpRequestContext_h_
