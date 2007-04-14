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
//#include <...>

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

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class HttpRequestContext
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
    enum RequestEnvironmentVariables
    {
        // Environment variables:
        HTTP_ENV_RAW_URL = 0,   // The url provided in the request
        HTTP_ENV_UNMAPPED_FILE, // The file part of the raw URL
        HTTP_ENV_MAPPED_FILE,   // The file part of the url mapped to the real location
        HTTP_ENV_QUERY_STRING,  // The query part of the URL (if GET) Note: the individual variables are retreiveable via getCgiVariables
        HTTP_ENV_SERVER_NAME,   // The Server name part of the URL
        HTTP_ENV_REQUEST_METHOD,// The request method (i.e. GET, PUT, POST)
        HTTP_ENV_USER,          // The user name (if this request required authorization)


        HTTP_ENV_LAST // Note: this is a dummy variable indicating the last var
    };

/* ============================ CREATORS ================================== */

   /// Construct the context for an HTTP request.
   HttpRequestContext( const char* requestMethod = NULL
                      ,const char* rawUrl = NULL
                      ,const char* mappedFile = NULL
                      ,const char* serverName = NULL
                      ,const char* userId = NULL
                      ,const OsConnectionSocket* connection = NULL
                      );
     //:Default constructor


   virtual
   ~HttpRequestContext();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   void extractPostCgiVariables(const HttpBody& body);
   // Extracts the CGI variables from the request body.

   typedef void (*UnEscapeFunction)(UtlString&);
   static void parseCgiVariables(const char* queryString,
                                 UtlList& cgiVariableList,
                                 const char* pairSeparator = "&",
                                 const char* namValueSeparator = "=",
                                 UtlBoolean nameIsCaseInsensitive = TRUE,
                                 UnEscapeFunction unescape =
                                     &HttpMessage::unescape);
   // If nameIsCaseInsensitive == TRUE, puts NameValuePairInsensitive's
   // into cgiVariableList rather than NameValuePair's.

   HttpRequestContext(const HttpRequestContext& rHttpRequestContext);
     //:Copy constructor
   HttpRequestContext& operator=(const HttpRequestContext& rhs);
     //:Assignment operator

/* ============================ ACCESSORS ================================= */

   void getEnvironmentVariable(enum RequestEnvironmentVariables envVariable, UtlString& value) const;
   //: Get Environment and context variables related to this request
   // See the RequestEnvironmentVariables enumeration for the complete list.

   UtlBoolean getCgiVariable(const char* name, UtlString& value, int occurance = 0) const;
   //: Get CGI/Form variables provided in this POST or GET request.
   // As it is possible to have multiple occurances of a named value
   // the occurance argument indicates which occurance.  The default is the first.
   //! returns: TRUE/FALSE if the occurance of the named variable exists

   UtlBoolean getCgiVariable(int index, UtlString& name, UtlString& value) const;
   //: Get the name and value of the variable at the given index

   /// Test whether or not the client connection is encrypted.
   bool isEncrypted() const;

   /// Test whether or not the given name is the SSL client that sent this request.
   bool isTrustedPeer( const UtlString& peername ) const;
   /**<
    * This tests the host identity provided by the SSL handshake; it does not
    * test the HTTP user identity.
    * @returns
    * - true if the connection is SSL and the peername matches a name in the peer certificate.
    * - false if not.
    */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   void parseCgiVariables(const char* queryString);
   //: Parse the CGI/form variables from the &,= delineated name, value pairs and unescape the name and values.

   UtlSList mCgiVariableList;
   bool     mUsingInsensitive;
   UtlString mEnvironmentVars[HTTP_ENV_LAST];
   bool     mConnectionEncrypted;
   bool     mPeerCertTrusted;
   UtlSList mPeerIdentities;
   
};

/* ============================ INLINE METHODS ============================ */

#endif  // _HttpRequestContext_h_
