//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _Url_h_
#define _Url_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "utl/UtlDefs.h"
#include "utl/UtlString.h"
#include "utl/UtlRegex.h"
class UtlDList;

// DEFINES

// sip:/sips: URI parameters
#define SIP_GRUU_URI_PARAM                  "gr"
#define SIPX_DONE_URI_PARAM                 "x-sipX-done"
#define SIPX_NO_NAT_URI_PARAM               "x-sipX-nonat"
#define SIPX_PRIVATE_CONTACT_URI_PARAM      "x-sipX-privcontact"
#define SIPX_ROUTE_TO_REGISTRAR_URI_PARAM   "x-sipX-routetoreg"
#define SIPX_SIPXECS_LINEID_URI_PARAM  "sipxecs-lineid"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class NameValuePair ;

/// URL parser and constructor
/**
 * This object is used to parse and construct URL strings.  It
 * contains all of the parsed components of a URL.  It has the ability
 * to construct a serialized string of the object using the toString()
 * method.  It can also be used as a parser using the constructor
 * which accepts a string as input.  This is intended to be a
 * generic URL parser for all schema/protocol types.  It is currently
 * tested and known to work for sip, sips, http, https, ftp, and file type
 * URLs. There are accessors for the various parts of the URL
 * These parts appear in URLs such as the following:
 * @code
 * "display name"<protocol:[//]user:password@host:port;urlparm=value?headerParam=value>;fieldParam=value
 * @endcode
 * The routines for the various parts are:
 * - display name
 *   - getDisplayName()
 *   - setDisplayName()
 * - protocol
 *   - getUrlType()
 *   - setUrlType()
 * - user
 *   - getUserId()
 *   - setUserId()
 * - password
 *   - getPassword()
 *   - setPassword()
 * - host
 *   - getHostAddress()
 *   - setHostAddress()
 * - port
 *   - getHostPort()
 *   - setHostPort()
 * - URL parameter names and values
 *   - getUrlParameter()
 *   - getUrlParameters()
 *   - setUrlParameter()
 *   - removeUrlParameter()
 *   - removeUrlParameters()
 * - header (or CGI) parameter names and values
 *   - getHeaderParameter()
 *   - getHeaderParameters()
 *   - setHeaderParameter()
 *   - removeHeaderParameter()
 *   - removeHeaderParameters()
 * - field parameter names and values
 *   - getFieldParameter()
 *   - getFieldParameters()
 *   - setFieldParameter()
 *   - removeFieldParameter()
 *   - removeFieldParameters()
 * - properties
 *   - isGRUU()
 */

class Url
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   /// Identifiers for all supported URI schemes
   typedef enum
      {
         UnknownUrlScheme,  ///< invalid or unset scheme
         SipUrlScheme,      ///< sip:    (RFC 3261)
         SipsUrlScheme,     ///< sips:   (RFC 3261)
         HttpUrlScheme,     ///< http:   (RFC 2616)
         HttpsUrlScheme,    ///< https:  (RFC 2818)
         FtpUrlScheme,      ///< ftp:    (RFC 1738)
         FileUrlScheme,     ///< file:   (RFC 1738)
         MailtoUrlScheme,   ///< mailto: (RFC 2368)
         NUM_SUPPORTED_URL_SCHEMES
      }  Scheme;
   /**<
    * URI types are distinguished by the scheme names.
    * The scheme method translates names into these enumerations;
    * the schemeName method translates these into the canonical string for a URI.
    * The authoritative record of what schemes are defined is at:
    *   http://www.iana.org/assignments/uri-schemes
    * The general rules for URIs are in RFC 3986
    */

   /// Selector for the syntax form to be used to parse a URI.
   typedef enum
   {
      AddrSpec, ///< the form of a URI used in the request line
      NameAddr  ///< the form of a URI used in most header fields
   } UriForm;
   /**<
    * Unfortunately, a SIP URI cannot be parsed in a completely
    * context-free way because the semicolon is used both to delimit
    * url parameters and to delimit header field parameters.  The
    * ambiguity is resolved by the following rules per RFC 3261 section 20:
    *
    * - When the UriForm is NameAddr, if there are no angle brackets
    *   used as in 'sip:user@domain;foo=bar', then foo is a field parameter.
    *
    * - When the UriForm is AddrSpec, no angle brackets are allowed, so
    *   there can be no field parameters.  In 'sip:user@domain;foo=bar',
    *   foo is a url parameter.
    *
    * Normally, AddrSpec is used only when parsing a Url value taken from
    * the target URI in the request line.  For any header field value, NameAddr
    * should be used.
    */

   /// Default constructor from string
   Url(const UtlString& urlString,  ///< string to parse URL from
       UriForm     uriForm,         ///< context to be used to parse the uri
       UtlString*  nextUri   = NULL ///< anything after a trailing comma
       );
   /**<
    * The urlString value is parsed and the components saved.
    *
    * @note Check getScheme() after use to see if the urlString was valid!
    *
    * Since a constructor cannot return an error, and we don't like to raise exceptions,
    * the validity of the urlString input must be tested by checking the scheme of
    * the resulting Url object.  When the input was not successfully parsed, the
    * getScheme() method returns UnknownUrlScheme (even if the scheme specified by the
    * input was ok and the parse error was in some later part of the string).
    *
    * For a description of how the nextUri argument is used, see fromString.
    */

   /// Backward compatibilty constructor for (confusing) UtlBoolean isAddrSpec parameter.
   Url(const char* urlString = NULL, ///< string to parse URL from
       UtlBoolean isAddrSpec = FALSE /**< - TRUE if this is an addrspec (a Request-URI)
                                      *   - FALSE if this is a name-addr (URL from a header field) */
       );
   ///< @DEPRECATED because the use of UtlBoolean is confusing - use the three argument form.

    //! Copy constructor
    Url(const Url& rUrl);

    //! Destructor
    virtual
    ~Url();

/* ============================ MANIPULATORS ============================== */

     //! Assignment operator
    Url& operator=(const Url& rhs);

    //! Assignment from string
    Url& operator=(const char* urlString); // name-addr format
    /**<
     *  @DEPRECATED due to poor error handling - use the fromString method instead
     *  Parse the given null terminated string and set the
     *  URL parts as found in the string.
     *  Assumes urlString is in NameAddr format.
     */

    /// Set the value of this Url by parsing the given string.
    bool fromString(const UtlString& urlString,  ///< string to parse URL from
                    UriForm     uriForm,         ///< context to be used to parse the uri
                    UtlString*  nextUri   = NULL ///< text after a trailing comma
                    );
    /**<
     * This is used just like the constructor, but resets the values for an existing Url.
     * For convenience, it also @returns true iff the urlString parsed as valid.
     *
     * If nextUri is NULL, urlString must parse completely as a URI.
     * If uriForm is NameAddr, there may be preceding and following whitespace.
     * If uriForm is AddrSpec, whitespace is not allowed.
     * If nextUri is not NULL, urlString may be a suitable URI
     * followed by a comma and further text; the text following the
     * comma (and following whitespace, if uriForm == NameAddr) is
     * returned in *nextUri.
     *
     * Many header field are defined to allow multiple comma-separated Url values.
     * If the values have display name components that contain commas (legal), it
     * is safest to do a complete parse of the Urls.  The nextUri parameter provides
     * a straightforward way to accomplish this by returning any part of the input
     * string that follows a trailing comma (and following whitespace) after the first
     * url in the string has been parsed.
     * In the following example, assume that 'someMethod' fills its argument UtlString
     * with some header field value that may have multiple URL values (such as
     * Contact, Route, and Record-Route):
     *
     * @code
     * Url url;
     * UtlString inputString;
     * UtlString remainderString;
     * for ( inputString = ...;
     *          !inputString.isNull()
     *       && url.fromString(inputString, NameAddr, &remainderString);
     *       inputString = remainderString
     *      )
     * {
     *     ...operate on url...
     * }
     * @endcode
     */

    /// set the value of this url by parsing the given string.
    bool fromString(const UtlString& urlString,
                    UtlBoolean isAddrSpec = FALSE
                    )
    {
       return fromString(urlString, isAddrSpec ? AddrSpec : NameAddr, NULL);
    }
    ///< @DEPRECATED - use the form taking a UriForm for the second argument.

    //! Serialize this URL to a string in name-addr format, suitable for use
    //  as a field in a header.
    void toString(UtlString& urlString) const;

    //! Serialize this URL to a string in name-addr format, suitable for use
    //  as a field in a header.
    UtlString toString() const;

    //! Serialize this Url as a proper URI (with no <...>, display name,
    //  or field parameters).  This is also called 'addr-spec' in RFC 3261.
    void getUri(UtlString& Uri) const;

    //! Get a malloc'ed string containing the URI as a name-addr.
    /*! Must be free'd by the caller.  Suitable for use in a debugger.
     */
    char* getBytes() const;
    //! Debug dump of the internal structures to STDOUT.
    void dump();
    void kedump();

    //! Clear the contents of this URL
    void reset();

    //! Remove all of the URL, header and field parameters and values
    void removeParameters();

/* ============================ ACCESSORS ================================= */

    /// Construct the canonical identity.
    void getIdentity(UtlString& identity) const;
    /**<
     * In some applications this is used to compare if this
     * URL refers to the same destination.  The identity is:
     * @code
     * "user@host:port"
     * @endcode
     */

    /// Get the URL application layer protocol scheme string.
    void getUrlType(UtlString& urlProtocol) const;
    /**<
     * If you are going to make decisions based on the type,
     * it is more efficient to use getScheme to get the Scheme enumerated form
     * rather than comparing strings.
     */

    /// Set the URL application layer protocol using the scheme name string
    void setUrlType(const char* urlProtocol);
    ///< also see setScheme.

    /// Get the URL display name if present
    void getDisplayName(UtlString& displayName) const;

    /// Set the URL display name
    void setDisplayName(const char* displayName);

    /// Get the URL user identity if present
    void getUserId(UtlString& userId) const;

    /// Set the URL user identity
    void setUserId(const char* userId);

    /// Get the users password if present in the URL
    UtlBoolean getPassword(UtlString& userId) const;

    /// Set the users password in the URL
    void setPassword(const char* userId);
    /**<
     * Putting a password in a URL is a
     * <strong>really really bad idea</strong>.
     * RFC 3261 says:
     *
     * While the SIP and SIPS URI syntax allows this field to be
     * present, its use is NOT RECOMMENDED, because the passing
     * of authentication information in clear text (such as URIs)
     * has proven to be a security risk in almost every case where
     * it has been used.
     */

    /// Get the URL host name or IP address
    void getHostAddress(UtlString& address) const;

    /// Get the host and port together as a string "host:port"
    void getHostWithPort(UtlString& domain) const;

    /// Set the URL host name or IP address
    void setHostAddress(const char* address);

    /// Get the URL host port
    // (To get the "hostport" in RFC 3261 BNF, see ::getHostWithPort().)
    int getHostPort() const;
    ///< port == PORT_NONE specifies that no port number is present.

    /// Set the URL host port
    void setHostPort(int port);
    ///< port == PORT_NONE specifies that no port number is to be added.

    /// Get the file path from the URL
    UtlBoolean getPath(UtlString& path,
                       UtlBoolean getStyle = FALSE /**< TRUE will put header (or CGI) parameters
                                                    *   in path in the format needed for an HTTP
                                                    *   GET.  FALSE will form the path without
                                                    *   the header parameters as formated for a
                                                    *   HTTP POST. */

                       );


    /// Set the file path
    void setPath(const char* path);
    /**< @note the path should \a not contain header (or CGI) parameters @endnote */

    /// Get the named URL parameter value
    UtlBoolean getUrlParameter(const char* name,  ///< the parameter name to get
                               UtlString& value,  ///< the value of the named parameter
                               int index = 0
                               ) const;
    /**<
     * Gets the index occurrence of the named parameter (the same parameter name may
     * occur multiple times in a URL).
     * @return TRUE if the indicated parameter exists
     */

    /// Get the name and value of the URL parameter at the indicated index.
    UtlBoolean getUrlParameter(int urlIndex,    /**< the index indicting which URL parameter to
                                                 *   get (starting at 0 for the first one). */
                               UtlString& name,  ///< the parameter name at urlIndex
                               UtlString& value  ///< the value of the parameter at urlIndex
                               ) const;
    /**< @return TRUE if the indicated parameter exists. */

    /// Set the named URL parameter to the given value
    void setUrlParameter(const char* name, ///< the parameter name
                         const char* value ///< the value of the parameter
                         );
    /**< Adds the parameter if it does not exist, sets the value if
     *   it does exist.
     */

    /// Removes all of the URL parameters
    void removeUrlParameters();

    /// Removes all of the URL parameters with the given name
    void removeUrlParameter(const char* name);

    /// Gets all of the URL parameters and values
    UtlBoolean getUrlParameters(int iMaxReturn,     ///< the maximum number of items to return
                                UtlString *pNames,  /**< Pointer to a preallocated array of
                                                     *   UtlStrings.  If a null is specified,
                                                     *   the function will false and the
                                                     *   iActualReturn will contain the actual
                                                     *   number of parameters. */
                                UtlString *pValues, /**< Pointer to a preallocated array of
                                                     *   UtlStrings.  If a null is specified,
                                                     *   the function will return false and
                                                     *   the iActualReturn will contain the
                                                     *   actual number of parameters. */
                                int& iActualReturn  ///< The actual number of items returned
                                ) const;
    /**< @returns TRUE if values are returned otherwise FALSE */

    /// Get the named header parameter value
    UtlBoolean getHeaderParameter(const char* name, ///< the parameter name to get
                                  UtlString& value, ///< the value of the named parameter
                                  int index = 0     /**< gets the index occurance of the named
                                                     *   parameter (the same parameter name
                                                     *   may occur multiple times in the URL). */
                                  ) const;
    ///< @returns TRUE if the indicated parameter exists

    /// Get the name and value of the header parameter at the indicated index
    /*! \param headerIndex -
     * \param name -
     * \param value -
     * \
     */
    UtlBoolean getHeaderParameter(int headerIndex,      /**< the index indicating which header
                                                         *   parameter to get (starting at 0
                                                         *   for the first one). */
                                  UtlString& headerName, ///< parameter name at headerIndex
                                  UtlString& headerValue ///< value of parameter at headerIndex
                                  ) const;
    ///< @returns TRUE if the indicated parameter exists

    /// Set the named header parameter to the given value
    void setHeaderParameter(const char* name,  ///< the parameter name
                            const char* value  ///< the value of the parameter
                            );
    /**<
     * For sip and sips URLs, this sets the header parameter to the value if the header name
     * is unique according to SipMessage::isUrlHeaderUnique, overwriting any previous value.
     * If the header is not unique, then this appends the header parameter.
     *
     * For all other URL schemes, the parameter is always appended.
     *
     * To ensure that any previous value is replaced, call removeHeaderParameter(name) first.
     */

    /// Removes all of the header parameters
    void removeHeaderParameters();

    /// Removes all of the header parameters with the given name
    void removeHeaderParameter(const char* name);

    ///Gets all of the Header parameters
    UtlBoolean getHeaderParameters(int iMaxReturn,    ///< the maximum number of items to return
                                   UtlString *pNames, /**< Pointer to a preallocated collection of
                                                       *   UtlStrings.  If a null is specified,
                                                       *   the function will return false and
                                                       *   the iActualReturn will contain the
                                                       *   actual number of parameters. */
                                   UtlString *pValues,/**< Pointer to a preallocated collection of
                                                       *   UtlStrings.  If a null is specified,
                                                       *   the function will return false and
                                                       *   the iActualReturn will contain the
                                                       *   actual number of parameters.*/
                                   int& iActualReturn ///< The actual number of items returned
                                   ) const;
    ///< @returns TRUE if values are returned otherwise FALSE

    /// Get the named field parameter value
    /*! \param name -
     * \param value -
     * \param index -
     * \
     */
    UtlBoolean getFieldParameter(const char* name, ///< the parameter name to get
                                 UtlString& value, ///< the value of the named parameter
                                 int index = 0     ///< gets the index occurance of the named
                                                   /**< parameter (the same parameter name may
                                                      occur multiple times in the URL). */
                                 ) const;
    ///< @returns TRUE if the indicated parameter exists

    /// Get the name and value of the field parameter at the indicated location
    UtlBoolean getFieldParameter(int fieldIndex, /**< the index indicting which field parameter to
                                                  * get (starting at 0 for the first one). */
                                 UtlString& name, ///< the parameter name at fieldIndex
                                 UtlString& value ///< the value of the parameter at fieldIndex
                                 ) const;
    ///< @returns TRUE if the indicated parameter exists

    /// Set the named field parameter to the given value
    void setFieldParameter(const char* name, ///< the parameter name
                           const char* value ///< the value of the parameter
                           );
     /**< Adds the parameter if it does not exist, sets the value if
      *   it does exist. */

    /// Removes all of the field parameters
    void removeFieldParameters();

    /// Removes all of the field parameters with the given name
    void removeFieldParameter(const char* name);

    /// Gets all of the Header parameters
    UtlBoolean getFieldParameters(int iMaxReturn,    /**< the maximum number of items to return
                                                      *   UtlStrings.  If a null is specified,
                                                      *   the function will return false and
                                                      *   the iActualReturn will contain the
                                                      *   actual number of parameters. */
                                  UtlString *pNames, /**< Pointer to a preallocated collection of
                                                      *   UtlStrings.  If a null is specified, the
                                                      *   function will return false and the
                                                      *   iActualReturn will contain the actual
                                                      *   number of parameters.*/
                                  UtlString *pValues, /**< Pointer to a preallocated collection of
                                                       *   UtlStrings.  If a null is specified,
                                                       *   the function will return false and
                                                       *   the iActualReturn will contain the
                                                       *   actual number of parameters.*/
                                  int& iActualReturn ///< The actual number of items returned
                                  ) const;
    ///< @returns TRUE if values are returned otherwise FALSE

    /// Forces the presence of angle brackets (i.e. <>) in the URL
    //! when serialized
    void includeAngleBrackets();

    /// Remove the angle brackets (i.e. <>) from the URL
    void removeAngleBrackets();
    /**<
     * This does not really do anything - the toString function always puts
     * out a canonical form that does not include angle brackets if it is
     * possible to omit them.
     */

    /// Escape a string as a gen_value, which is what field-parameters use for values.
    static void gen_value_escape(UtlString& escapedText);

    /// Un-escape a string as a gen_value, which is what field-parameters use for values.
    static void gen_value_unescape(UtlString& escapedText);

/* ============================ INQUIRY =================================== */

    /// Is string all digits
    /*! \param dialedCharacters - null terminated string containing
     *       ascii test
     * \ return TRUE if the dialedCharacters are all digits
     */
   static UtlBoolean isDigitString(const char* dialedCharacters);

   /// Compare two URLs to see if the have the same user, host and port
   UtlBoolean isUserHostPortEqual(const Url& uri,
                                  int impliedPort = PORT_NONE
                                  ) const ;
   /**<
    * Follows the rules of RFC 3261 section 19.1.4, especially that
    * that no port specifies is NOT the same as the default port for
    * the URL type/protocol (but see the description of impliedPort).
    *
    * Assumes that host is not case sensitive (because DNS names are not by definition),
    * but that user id is case sensitive.
    *
    * If the impliedPort is some value other than PORT_NONE, then that port number
    * is considered to be equal to an unspecified port number.  For example:
    * @code
    *   Url implicitPortUrl("sip:user@example.com");
    *   UtlBoolean result;
    *
    *   Url explicitPortUrl("<sip:user@Example.COM:5060>;param=x");
    *   result = implicitPortUrl.isUserHostPortEqual(explicitPortUrl);
    *   // result is FALSE because an implicit port != 5060
    *
    *   Url unspecifiedPortUrl("<sip:user@Example.COM>;param=x");
    *   result = implicitPortUrl.isUserHostPortEqual(unspecifiedPortUrl);
    *   // result is TRUE, despite case difference in domain name
    *
    *   Url otherPortUrl("<sip:user@Example.COM:5999>;param=x");
    *   result = implicitPortUrl.isUserHostPortEqual(otherPortUrl, 5999);
    *   // result is TRUE because the specified implicit port is
    *   // considered to be that implied by the unspecified port in implicitPortUrl
    *
    *   result = implicitPortUrl.isUserHostPortEqual(otherPortUrl, 5060);
    *   // result is FALSE because the specified implicit port does not match
    * @endcode
    *
    * @return TRUE if the user Id, host and port are the same
    */

   /// Compare two URLs to see if the have the same user and host
   UtlBoolean isUserHostEqual(const Url& uri) const ;
   /**<
    * Assumes that host is not case sensitive, but that user id is case sensitive.
    * @return TRUE if the user Id and host are the same
    */

   /// Are angle brackets explicitly included
   UtlBoolean isIncludeAngleBracketsSet() const ;
   /**<
    * @note does not test if angle brackets are required or will be added
    * implicitly.
    * @returns TRUE if angle brackets were found when parsing or if they are
    *          explicitly set to be inserted during serialization.
    */

   /// Translate a scheme string (not including the terminating colon) to a Scheme enum.
   static Scheme scheme(const UtlString& schemeName);

   /// Get the canonical (lowercase) scheme string constant for a Scheme enum.
   static const char* schemeName(Scheme scheme);

   /// Get the enumerator for the URL scheme type (more convenient than getUrlType).
   Scheme getScheme() const;

   /// Set the scheme to be used (also see setUrlType).
   void setScheme(Scheme scheme);

   /// Is this a Globally Routable UA URI?
   bool isGRUU() const;
   ///< @returns true if it detectably has the properties of a GRUU

   /// Mark this as a Globally Routable UA URI
   void setGRUU(const UtlString& uniqueId /**< This is used as the value of the 'gr' Url
                                           *   parameter in a SIP URL.  If an empty UtlString
                                           *   is passed, the parameter is added with no value. */
                );
   /**<
    * This adds the parameter that identifies this URL as a GRUU
    * @see draft-ietf-sip-gruu-15.txt
    *
    * @note at present, may be called only for SipsUrlScheme or SipsUrlScheme URLs
    */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   /// parse a URL in string form into its component parts
   bool parseString(const char* urlString, ///< string to parse URL from
                    UriForm     uriForm,   ///< context to be used to parse the uri
                    UtlString*  nextUri    ///< anything after trailing comma
                    );

   Scheme    mScheme;

   UtlString mDisplayName;
   UtlString mUserId;
   UtlString mPassword;
   UtlBoolean mPasswordSet;
   UtlString mHostAddress;
   int mHostPort;
   UtlString mPath;

   bool              parseUrlParameters() const;//< lazy parser for url parameters
   mutable UtlString mRawUrlParameters;
   mutable UtlDList* mpUrlParameters;

   bool              parseHeaderOrQueryParameters() const;//< lazy parser for header or query parameters
   mutable UtlString mRawHeaderOrQueryParameters;
   mutable UtlDList* mpHeaderOrQueryParameters;

   mutable UtlDList* mpFieldParameters;


   UtlBoolean mAngleBracketsIncluded;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _Url_h_
