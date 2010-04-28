//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>

#ifdef __pingtel_on_posix__
#include <stdlib.h>
#endif

// APPLICATION INCLUDES
#include "os/OsSysLog.h"
#include "utl/UtlDListIterator.h"
#include "utl/UtlRegex.h"
#include "net/Url.h"
#include "net/NameValueTokenizer.h"
#include "net/NameValuePairInsensitive.h"
#include "net/SipMessage.h"
#include "net/HttpRequestContext.h"

#undef TIME_PARSE
#if TIME_PARSE
#   include "os/OsTimeLog.h"
#   define LOG_TIME(x) timeLog.addEvent(x)
#else
#   define LOG_TIME(x) /* x */
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

/* =========================================================================
 * IMPORTANT NOTE:
 *   If you change any of the following regular expressions, enable the
 *   verbose form of the PARSE macro in ../test/net/UrlTest.cpp and check
 *   to see if the parsing times are reasonable.  It's pretty easy to
 *   cause very deep recursions, which can be both a performance problem
 *   and can cause crashes due to stack overflow.
 * ========================================================================= */

#define DQUOTE "\""
#define LWS "\\s+"
#define SWS "\\s*"
// The pattern which matches a single backslash character.
// The pattern is two backslash characters.
// In C++, this must be written as four backslash characters.
#define SLASH "\\\\"

#define SIP_TOKEN "[a-zA-Z0-9.!%*_+`'~-]++"

// The following two patterns are largely parallel.  The first does not
// include any substring-capturing (...) to speed up matching, since
// substring-capturing is slow.

// SipTokenSequenceOrQuoted - used to validate display name values in setDisplayName
//   does not capture any substrings - this is important to avoid recursion
const RegEx SipTokenSequenceOrQuoted("^(?:" SIP_TOKEN "(?:" LWS SIP_TOKEN ")*"
                                       "|" DQUOTE "(?:[^" SLASH DQUOTE "]++"
                                                   "|" SLASH "."
                                                  ")*"
                                           DQUOTE
                                     ")$");

// DisplayName - used to parse display name from a url string
//    $1 matches an unquoted string
//    $2 matches a quoted string but without the quotes
//       Do Not Change This To Include The Quotes - that causes the regex
//       processor to recurse, possibly very very deeply.
//       Instead, we add the quotes back in explicitly in later processing.
// Does not include any leading or trailing whitespace.
const RegEx DisplayName( SWS "(?:(" SIP_TOKEN "(?:" LWS SIP_TOKEN ")*)"
                              "|" DQUOTE "((?:[^" SLASH DQUOTE "]++"
                                           "|" SLASH "."
                                           ")*)"
                                  DQUOTE
                             ")"
                         // This context constraint does not change what
                         // matches are successful, but speeds up failure
                         // cases.
                         "(?=" SWS "<)"
                        );

// AngleBrackets
//   allows and matches leading whitespace
//   $0 matches any leading whitespace, the angle brackets, and the contents
//   $1 matches just the contents
const RegEx AngleBrackets( SWS "<([^>]+)>" );

/* SupportedSchemes - matches any supported scheme name followed by ':'
 *   allows leading whitespace
 *   $0 matches the scheme name with the colon and any leading whitespace
 *   $1 matches the scheme name without the colon
 *
 * IMPORTANT
 *    The number and order of the strings in the following two constants MUST match the
 *    the number and order of the enum values in the Url::Scheme type.
 *
 *    The expression
 *        SupportedSchemes.Matches()
 *            - SUPPORTED_SCHEMES_FIRST_MATCHES
 *            + SUPPORTED_SCHEMES_FIRST_VALUE
 *    
 *    is used to compute Url::Scheme of the Url, so if the above rule is broken, the
 *    scheme recognition will not work.
 *
 *    Similarly, the Scheme value is used as an index into SchemeName, so the translation
 *    to a string will be wrong if that is not kept correct.
 */
#define SUPPORTED_SCHEMES "(?i:(UNKNOWN-URL-SCHEME)|(sip)|(sips)|(http)|(https)|(ftp)|(file)|(mailto))"
// The value returned by regex.Matches() if the first alternative in
// "RegEx regex(SUPPORTED_SCHEMES)" matches.
#define SUPPORTED_SCHEMES_FIRST_MATCHES 2
// The Scheme value for the first alternative in SUPPORTED_SCHEMES.
#define SUPPORTED_SCHEMES_FIRST_VALUE UnknownUrlScheme

const RegEx SupportedScheme( SUPPORTED_SCHEMES ":" );
const RegEx SupportedSchemeSWS( SWS SUPPORTED_SCHEMES SWS ":" );
const RegEx SupportedSchemeExact( "^" SUPPORTED_SCHEMES "$" );
const char* SchemeName[ Url::NUM_SUPPORTED_URL_SCHEMES ] =
{
   "UNKNOWN-URL-SCHEME",
   "sip",
   "sips",
   "http",
   "https",
   "ftp",
   "file",
   "mailto",
};

// UsernameAndPassword
//   requires and matches the trailing '@'
//   $1 matches user
//   $2 matches password
const RegEx UsernameAndPassword(
   "("
      "(?:"
         "[a-zA-Z0-9_.!~*'()&=+$,;?/-]++"
       "|"
         "%[0-9a-fA-F]{2}"
      ")+"
    ")"
   "(?:" ":"
      "("
        "(?:"
            "[a-zA-Z0-9_.!~*'()&=+$,-]++"
         "|"
            "%[0-9a-fA-F]{2}"
        ")*"
      ")"
    ")?"
    "@"
                                   );

// Host Address and Port
//   does not allow leading whitespace
//   $0 matches host:port
//   $1 matches host
//   $2 matches port
#define DOMAIN_LABEL "(?:[a-zA-Z0-9](?:[a-zA-Z0-9-]*[a-zA-Z0-9])?)"
const RegEx HostAndPort(
   "("
     "(?:" DOMAIN_LABEL "\\.)*" DOMAIN_LABEL "\\.?" // DNS name
      "|"
        "(?:[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3})" // IPv4 address
      "|"
        "(?:\\[[0-9a-fA-F:.]++\\])" // IPv6 address
     ")"
   "(?:" ":" "([0-9]{1,6}))?" // port number
                        );

// UrlPath
//   does not allow whitespace
//   does not require, but matches a trailing '?'
//   $0 matches path?
//   $1 matches path
const RegEx UrlPath( "([^?,\\s]++)[?,]?" );

// UrlParams
//   allows leading whitespace
//   is terminated by but does not require a trailing '?' or '>'
//   $0 matches ;params
//   $1 matches params
const RegEx UrlParams( SWS ";([^?>,]++)" );

// FieldParamName
//   allows leading whitespace
//   is terminated by end of string
//   $0 matches ;name
//   $1 matches name
const RegEx FieldParamName( SWS ";" SWS "(" SIP_TOKEN ")" SWS );

// FieldParamEqualsUnquotedValue
//   is terminated by end of string
//   $0 matches =value
const RegEx FieldParamEqualsUnquotedValue(
   "=" SWS "(" SIP_TOKEN
   "|(?:" // host
   "(?:" DOMAIN_LABEL "\\.)*" DOMAIN_LABEL "\\.?" // DNS name
   "|" "(?:[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3})" // IPv4 address
   "|" "(?:\\[[0-9a-fA-F:.]++\\])" // IPv6 address
   ")"
   ")"
                                    );

// FieldParamValueOkUnquoted
//   $0 matches any value that can be an unquoted field parameter value
const RegEx FieldParamValueOkUnquoted(
   "^(?:" SIP_TOKEN
   "|(?:" // host
   "(?:" DOMAIN_LABEL "\\.)*" DOMAIN_LABEL "\\.?" // DNS name
   "|" "(?:[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3})" // IPv4 address
   "|" "(?:\\[[0-9a-fA-F:.]++\\])" // IPv6 address
   ")"
   ")$"
                                      );

// FieldParamEqualsQuotedValue
//   is terminated by end of string
//   $0 matches ="value" including quotes
const RegEx FieldParamEqualsQuotedValue(
   "=" SWS "(" DQUOTE "(?:[^\\" DQUOTE "]++" "|" SLASH "." ")*" DQUOTE ")" // quoted-string
                                  );
// HeaderOrQueryParams
//   allows leading whitespace
//   is terminated by but does not require a trailing '>'
//   $0 matches ?params
//   $1 matches params
const RegEx HeaderOrQueryParams( SWS "\\?([^,>]++)" );

// AllDigits
const RegEx AllDigits("^\\+?[0-9*]++$");

// Comma separator between multiple values in name-addrs
const RegEx CommaSeparator(SWS "," SWS);

// Regexps that describe what may follow a name-addr/addr-spec in various
// contexts.
const RegEx End("$");           // addr-spec not in list
const RegEx EndComma("$|,");    // addr-spec in list
const RegEx EndSws(SWS "$");    // name-addr not in list
const RegEx EndSwsComma(SWS "$|" SWS "," SWS); // name-addr in list
const RegEx EndUrl(SWS "$");

// STATIC VARIABLE INITIALIZATIONS

#ifndef min
#define min(x,y) (((x) < (y)) ? (x) : (y))
#endif

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
Url::Url(const UtlString& urlString, ///< string to parse URL from
         UriForm          uriForm,   ///< context to be used to parse the uri
         UtlString*       nextUri    ///< anything after trailing comma
         ) :
   mpUrlParameters(NULL),
   mpHeaderOrQueryParameters(NULL),
   mpFieldParameters(NULL)
{
   reset();
   if (urlString && *urlString)
   {
      parseString(urlString, uriForm, nextUri);
   }
}

Url::Url(const char* urlString, UtlBoolean isAddrSpec) :
   mpUrlParameters(NULL),
   mpHeaderOrQueryParameters(NULL),
   mpFieldParameters(NULL)
{
   reset();
   if (urlString && *urlString)
   {
      parseString(urlString, isAddrSpec ? AddrSpec : NameAddr, NULL);
   }
}

bool Url::fromString(const UtlString& urlString, ///< string to parse URL from
                     UriForm          uriForm,   ///< context to be used to parse the uri
                     UtlString*       nextUri    ///< anything after trailing comma
                     )
{
   reset();

   return parseString(urlString.data(), uriForm, nextUri);
}

// Copy constructor
Url::Url(const Url& rUrl) :
   mpUrlParameters(NULL),
   mpHeaderOrQueryParameters(NULL),
   mpFieldParameters(NULL)
{
   reset();
   *this = rUrl;
}

// Destructor
Url::~Url()
{
    removeParameters();
}

void Url::removeParameters()
{
   removeUrlParameters();
   removeFieldParameters();
   removeHeaderParameters();
}

void Url::reset()
{
    mScheme = SipUrlScheme;
    mDisplayName.remove(0);
    mUserId.remove(0);
    mPassword.remove(0);
    mPasswordSet = FALSE;
    mHostAddress.remove(0);
    mHostPort = PORT_NONE;
    mPath.remove(0);
    mAngleBracketsIncluded = FALSE;
    removeParameters();
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
Url&
Url::operator=(const Url& rhs)
{
   if (this != &rhs) // handle the assignment to self case
   {
      reset();

      // Copy the members
      mScheme = rhs.mScheme;
      mDisplayName = rhs.mDisplayName;
      mUserId = rhs.mUserId;
      mPassword = rhs.mPassword;
      mPasswordSet = rhs.mPasswordSet;
      mHostAddress = rhs.mHostAddress;
      mHostPort = rhs.mHostPort;
      mPath = rhs.mPath;
      mAngleBracketsIncluded = rhs.mAngleBracketsIncluded;

      if (rhs.mpUrlParameters)
      {
         mpUrlParameters = new UtlDList;

         UtlDListIterator paramIterator(*rhs.mpUrlParameters);
         NameValuePairInsensitive* rhsParam;
         while ((rhsParam = dynamic_cast<NameValuePairInsensitive*>(paramIterator())))
         {
            mpUrlParameters->append(new NameValuePairInsensitive(*rhsParam));
         }
      }
      else
      {
         mRawUrlParameters = rhs.mRawUrlParameters;
      }

      if (rhs.mpHeaderOrQueryParameters)
      {
         mpHeaderOrQueryParameters = new UtlDList;

         UtlDListIterator paramIterator(*rhs.mpHeaderOrQueryParameters);
         NameValuePairInsensitive* rhsParam;
         while ((rhsParam = dynamic_cast<NameValuePairInsensitive*>(paramIterator())))
         {
            mpHeaderOrQueryParameters->append(new NameValuePairInsensitive(*rhsParam));
         }
      }
      else
      {
         mRawHeaderOrQueryParameters = rhs.mRawHeaderOrQueryParameters;
      }

      if (rhs.mpFieldParameters)
      {
         mpFieldParameters = new UtlDList;

         UtlDListIterator paramIterator(*rhs.mpFieldParameters);
         NameValuePairInsensitive* rhsParam;
         while ((rhsParam = dynamic_cast<NameValuePairInsensitive*>(paramIterator())))
         {
            mpFieldParameters->append(new NameValuePairInsensitive(*rhsParam));
         }
      }
   }

   return *this;
}

Url& Url::operator=(const char* urlString)
{
   reset();

   if (urlString && *urlString)
   {
      parseString(urlString, NameAddr, NULL);
   }

   return *this;
}
/* ============================ ACCESSORS ================================= */


Url::Scheme Url::getScheme() const
{
   return mScheme;
}

void Url::getUrlType(UtlString& urlProtocol) const
{
    urlProtocol = SchemeName[mScheme];
}

void Url::setScheme(Url::Scheme scheme)
{
   mScheme = scheme;
}

void Url::setUrlType(const char* urlProtocol)
{
   if (urlProtocol)
   {
      UtlString schemeName(urlProtocol);

      mScheme = scheme(schemeName);

      if ( UnknownUrlScheme == mScheme )
      {
         OsSysLog::add(FAC_SIP, PRI_ERR, "Url::setUrlType unsupported Url scheme '%s'",
                       urlProtocol
                       );
      }
   }
   else
   {
      // no urlProtocol value passed
      OsSysLog::add(FAC_SIP, PRI_CRIT, "Url::setUrlType Url scheme NULL");
      mScheme = UnknownUrlScheme;
   }
}

void Url::getDisplayName(UtlString& displayName) const
{
    displayName = mDisplayName;
    if (isDigitString(mDisplayName.data()))
    {
       NameValueTokenizer::frontBackTrim(&displayName, "\"");
    }
}

void Url::setDisplayName(const char* displayName)
{
   mDisplayName.remove(0);

   if (displayName && strlen(displayName))
   {
       RegEx tokenSequenceOrQuoted(SipTokenSequenceOrQuoted);
       if (tokenSequenceOrQuoted.Search(displayName))
       {
          mDisplayName = displayName;
       }
       else
       {
          OsSysLog::add(FAC_SIP, PRI_CRIT, "Url::setDisplayName '%s' invalid", displayName);
       }
   }
}

void Url::getUserId(UtlString& userId) const
{
   userId = mUserId;
}

void Url::setUserId(const char* userId)
{
   if (userId)
   {
      mUserId = userId;
   }
   else
   {
      mUserId.remove(0);
   }
}

UtlBoolean Url::getPassword(UtlString& password) const
{
    password = mPassword;
    return(mPasswordSet);
}

void Url::setPassword(const char* password)
{
    if (password)
    {
        mPassword = password;
        mPasswordSet = TRUE;
    }
    else
    {
        mPassword.remove(0);
        mPasswordSet = FALSE;
    }
}

void Url::getHostAddress(UtlString& address) const
{
    address = mHostAddress;
}

void Url::getHostWithPort(UtlString& domain) const
{
   getHostAddress(domain);
   if (mHostPort != PORT_NONE)
   {
      char portNum[7];
      sprintf(portNum, ":%d", mHostPort);
      domain.append(portNum);
   }
}

void Url::setPath(const char* path)
{
   if (path)
   {
      mPath = path;
   }
   else
   {
      mPath.remove(0);
   }
}

UtlBoolean Url::getPath(UtlString& path, UtlBoolean getStyle)
{
    path = mPath;

    // If the desire is to have an HTTP GET style path with CGI variables
    if(getStyle)
    {
       if (   (   mpHeaderOrQueryParameters
               || const_cast<Url*>(this)->parseHeaderOrQueryParameters()
               )
           && mpHeaderOrQueryParameters->entries()
           )
        {
            UtlDListIterator headerParamIterator(*mpHeaderOrQueryParameters);
            NameValuePairInsensitive* headerParam = NULL;
            UtlString headerParamValue ;
            UtlBoolean firstHeader = TRUE;

            while ((headerParam = dynamic_cast<NameValuePairInsensitive*>(headerParamIterator())))
            {
                // Add separator for first header parameter
                if(firstHeader)
                {
                    path.append("?", 1);
                    firstHeader = FALSE;
                }
                else
                {
                    path.append("&", 1);
                }

                path.append(*headerParam);
                headerParamValue = headerParam->getValue();
                if(!headerParamValue.isNull())
                {
                    path.append("=", 1);
                    HttpMessage::escape(headerParamValue);

                    path.append(headerParamValue);
                }
            }
        } // endif mpHeaderOrQueryParameters
    }

    return(!mPath.isNull());
}

void Url::setHostAddress(const char* address)
{
    if (address)
        mHostAddress = address;
    else
        mHostAddress.remove(0);
}

int Url::getHostPort() const
{
    return(mHostPort);
}

void Url::setHostPort(int port)
{
    mHostPort = port;
}

UtlBoolean Url::getUrlParameter(const char* name, UtlString& value, int index) const
{
    int foundIndex = 0;
    UtlBoolean found = FALSE;
    value = "";
    if(mpUrlParameters || parseUrlParameters())
    {
        UtlDListIterator urlParamIterator(*mpUrlParameters);
        NameValuePairInsensitive* urlParam = NULL;

        UtlString paramName;

        while (!found && (urlParam = dynamic_cast<NameValuePairInsensitive*>(urlParamIterator())))
        {
            paramName = *urlParam;
            if(paramName.compareTo(name, UtlString::ignoreCase) == 0)
            {
                if(index == foundIndex)
                {
                    found = TRUE;
                    value = urlParam->getValue();
                }
                else
                {
                   foundIndex++;
                }
            }
        }
    }
    return(found);
}

UtlBoolean Url::getUrlParameter(int urlIndex, UtlString& name, UtlString& value) const
{
    NameValuePairInsensitive* url = NULL;

    if (   (urlIndex >= 0)
        && (mpUrlParameters || parseUrlParameters())
        && (((int)(mpUrlParameters->entries())) > urlIndex)
        )
    {
       url = (NameValuePairInsensitive*) mpUrlParameters->at(urlIndex);
    }

    if(url)
    {
        name = *url;
        value = url->getValue();
    }

    return(NULL != url);
}

UtlBoolean Url::getUrlParameters(int iMaxReturn, UtlString* pNames,
                                 UtlString *pValues, int& iActualReturn) const
{
    if(! (mpUrlParameters || parseUrlParameters()))
    {
        iActualReturn = 0;
    }

    // If the pNames or pValue is null, return false and set the actual return
    // to the actual number of items.
    else if ((pNames == NULL) || (pValues == NULL))
    {
        iActualReturn = mpUrlParameters->entries() ;
        return FALSE ;
    }
    else
    {
        iActualReturn = min(iMaxReturn, ((int)(mpUrlParameters->entries()))) ;

        for (int i=0; i<iActualReturn; i++)
        {
            NameValuePairInsensitive* pair = (NameValuePairInsensitive*) mpUrlParameters->at(i) ;
            pNames[i] = *pair;
            pValues[i] = pair->getValue() ;
        }
    }
    return (iActualReturn > 0) ;
}

void Url::setUrlParameter(const char* name, const char* value)
{
    NameValuePairInsensitive* nv = new NameValuePairInsensitive(name ? name : "",
        value ? value : "");

    // ensure that mpUrlParameters is initialized
    if (! (mpUrlParameters || parseUrlParameters()))
    {
       mpUrlParameters = new UtlDList;
    }

    NameValuePairInsensitive* existingParam = dynamic_cast<NameValuePairInsensitive*>(mpUrlParameters->find(nv));

    if (existingParam)
    {
       existingParam->setValue(value);
       delete nv;
    }
    else
    {
       mpUrlParameters->append(nv);
    }

}

UtlBoolean Url::getHeaderParameter(const char* name, UtlString& value, int index) const
{
    int foundIndex = 0;
    UtlBoolean found = FALSE;
    value = "";
    if(mpHeaderOrQueryParameters || parseHeaderOrQueryParameters())
    {
        UtlDListIterator headerParamIterator(*mpHeaderOrQueryParameters);
        NameValuePairInsensitive* headerParam = NULL;

        UtlString paramName;

        while (!found && (headerParam = dynamic_cast<NameValuePairInsensitive*>(headerParamIterator())))
        {
            paramName = *headerParam;
            if(paramName.compareTo(name, UtlString::ignoreCase) == 0)
            {
                if(index == foundIndex)
                {
                    found = TRUE;
                    value = headerParam->getValue();
                }
                else
                {
                   foundIndex++;
                }
            }
        }
    }
    return(found);
}

UtlBoolean Url::getHeaderParameters(int iMaxReturn, UtlString* pNames,
                                    UtlString *pValues, int& iActualReturn) const
{
    if(!(mpHeaderOrQueryParameters || parseHeaderOrQueryParameters()))
    {
        iActualReturn = 0;
    }

    // If the pValue is null, return false and set the actual return to the actual
    // number of items.
    else if (pValues == NULL || pNames == NULL)
    {
        iActualReturn = mpHeaderOrQueryParameters->entries() ;
        return FALSE ;
    }
    else
    {
        iActualReturn = min(iMaxReturn, ((int)(mpHeaderOrQueryParameters->entries()))) ;

        for (int i=0; i<iActualReturn; i++)
        {
            NameValuePairInsensitive *pair = (NameValuePairInsensitive*) mpHeaderOrQueryParameters->at(i) ;
            pNames[i] = *pair;
            pValues[i] = pair->getValue() ;
        }
    }
    return (iActualReturn > 0) ;
}


void Url::removeUrlParameters()
{
   if(mpUrlParameters)
   {
      mpUrlParameters->destroyAll();
      delete mpUrlParameters;
      mpUrlParameters = NULL;
   }
   else
   {
      mRawUrlParameters.remove(0);
   }
}

void Url::removeUrlParameter(const char* name)
{
    if(mpUrlParameters || parseUrlParameters())
    {
        NameValuePairInsensitive nv(name ? name : "", NULL);

        UtlContainable* matchingParam;
        while((matchingParam = mpUrlParameters->find(&nv)))
        {
           mpUrlParameters->removeReference(matchingParam);
           delete matchingParam;
        }
    }
}

void Url::getUri(UtlString& urlString) const
{
   // Insert the scheme
    urlString = schemeName(mScheme);
    urlString.append(":",1);

    switch(mScheme)
    {
    case FileUrlScheme:
    case FtpUrlScheme:
    case HttpUrlScheme:
    case HttpsUrlScheme:
       urlString.append("//",2);
       break;

    case SipUrlScheme:
    case SipsUrlScheme:
    case MailtoUrlScheme:
    default:
       break;
    }

    // Add the user
    if (FileUrlScheme != mScheme) // no user defined in a file url
    {
       if(!mUserId.isNull())
       {
          urlString.append(mUserId);
          if(!mPassword.isNull() || mPasswordSet)
          {
             urlString.append(":", 1);
             urlString.append(mPassword);
          }
          urlString.append("@", 1);
       }
    }

    // Add the host
    urlString.append(mHostAddress);
    if(portIsValid(mHostPort))
    {
       char portBuffer[20];
       sprintf(portBuffer, ":%d", mHostPort);
       urlString.append(portBuffer);
    }

    // Add the path
    switch(mScheme)
    {
    case FileUrlScheme:
    case FtpUrlScheme:
    case HttpUrlScheme:
    case HttpsUrlScheme:
       if(!mPath.isNull())
       {
          urlString.append(mPath);
       }
       break;

    case SipUrlScheme:
    case SipsUrlScheme:
    case MailtoUrlScheme:
    default:
       break;
    }

    // Add the URL parameters
    if (   (   mpUrlParameters
            || const_cast<Url*>(this)->parseUrlParameters()
            )
        && mpUrlParameters->entries()
        )
    {
        UtlDListIterator urlParamIterator(*mpUrlParameters);
        NameValuePairInsensitive* urlParam = NULL;
        UtlString urlParamValue;

        while ((urlParam = dynamic_cast<NameValuePairInsensitive*>(urlParamIterator())))
        {
            urlString.append(";", 1);
            urlString.append(*urlParam);
            urlParamValue = urlParam->getValue();
            if(!urlParamValue.isNull())
            {
                urlString.append("=", 1);
                HttpMessage::escape(urlParamValue);
                urlString.append(urlParamValue);
            }
        }
    }

    // Add the header parameters
    if (   (   mpHeaderOrQueryParameters
            || const_cast<Url*>(this)->parseHeaderOrQueryParameters()
            )
        && mpHeaderOrQueryParameters->entries()
        )
    {
        UtlDListIterator headerParamIterator(*mpHeaderOrQueryParameters);
        NameValuePairInsensitive* headerParam = NULL;
        UtlString headerParamValue;
        UtlBoolean firstHeader = TRUE;

        while ((headerParam = dynamic_cast<NameValuePairInsensitive*>(headerParamIterator())))
        {
            // Add separator for first header parameter
            if(firstHeader)
            {
                urlString.append("?", 1);
                firstHeader = FALSE;
            }
            else
            {
                urlString.append("&", 1);
            }

            urlString.append(*headerParam);
            headerParamValue = headerParam->getValue();
            if(!headerParamValue.isNull())
            {
                urlString.append("=", 1);
                HttpMessage::escape(headerParamValue);
                urlString.append(headerParamValue);
            }
        }
    }
}

void Url::setHeaderParameter(const char* name, const char* value)
{
   if ( name && *name )
   {
      NameValuePairInsensitive* nv = new NameValuePairInsensitive(name, value ? value : "");

      // ensure that mpHeaderOrQueryParameters is initialized
      if (! (mpHeaderOrQueryParameters || parseHeaderOrQueryParameters()))
      {
         mpHeaderOrQueryParameters = new UtlDList;
      }

      if (   (   SipUrlScheme  == mScheme
              || SipsUrlScheme == mScheme
              )
          && ( SipMessage::isUrlHeaderUnique(name) )
          )
      {
         removeHeaderParameter(name);
      }

      // for all other cases, assume that duplicate query parameters are ok
      mpHeaderOrQueryParameters->append(nv);
   }
}

UtlBoolean Url::getHeaderParameter(int headerIndex, UtlString& name, UtlString& value) const
{
    NameValuePairInsensitive* header = NULL;

    if (   (headerIndex >= 0)
        && (mpHeaderOrQueryParameters || parseHeaderOrQueryParameters())
        && (((int)(mpHeaderOrQueryParameters->entries())) > headerIndex)
        )
    {
       header = dynamic_cast<NameValuePairInsensitive*>(mpHeaderOrQueryParameters->at(headerIndex));
    }

    if(header)
    {
        name = *header;
        value = header->getValue();
    }

    return(NULL != header);
}

void Url::removeHeaderParameters()
{
   if(mpHeaderOrQueryParameters)
   {
      mpHeaderOrQueryParameters->destroyAll();
      delete mpHeaderOrQueryParameters;
      mpHeaderOrQueryParameters = NULL;
   }
   mRawHeaderOrQueryParameters.remove(0);
}

void Url::removeHeaderParameter(const char* name)
{
    if(mpHeaderOrQueryParameters || parseHeaderOrQueryParameters())
    {
        NameValuePairInsensitive nv(name ? name : "", NULL);

        UtlContainable* matchingParam;
        while((matchingParam=mpHeaderOrQueryParameters->find(&nv)))
        {
           mpHeaderOrQueryParameters->removeReference(matchingParam);
           delete matchingParam;
        }
    }
}

UtlBoolean Url::getFieldParameter(const char* name, UtlString& value, int index) const
{
    int foundIndex = 0;
    UtlBoolean found = FALSE;
    value = "";
    if(mpFieldParameters)
    {
        UtlDListIterator fieldParamIterator(*mpFieldParameters);
        NameValuePairInsensitive* fieldParam = NULL;

        UtlString paramName;

        while (!found && (fieldParam = dynamic_cast<NameValuePairInsensitive*>(fieldParamIterator())))
        {
            paramName = *fieldParam;
            if(paramName.compareTo(name, UtlString::ignoreCase) == 0)
            {
                if(index == foundIndex)
                {
                    found = TRUE;
                    value = fieldParam->getValue();
                }
                else
                {
                   foundIndex++;
                }
            }
        }
    }
    return(found);
}

UtlBoolean Url::getFieldParameter(int fieldIndex, UtlString& name, UtlString& value) const
{
    NameValuePairInsensitive* field = NULL;

    if (   fieldIndex >= 0
        && (mpFieldParameters)
        && ((int)(mpFieldParameters->entries())) > fieldIndex
        )
    {
       field = (NameValuePairInsensitive*) mpFieldParameters->at(fieldIndex);
    }

    if(field)
    {
        name = *field;
        value = field->getValue();
    }

    return(NULL != field);
}

UtlBoolean Url::getFieldParameters(int iMaxReturn, UtlString* pNames,
                                   UtlString *pValues, int& iActualReturn) const
{
    if(!mpFieldParameters)
    {
        iActualReturn = 0;
    }

    // If the pValue is null, return false and set the actual return to the actual
    // number of items.
    else if (pNames == NULL || pValues == NULL)
    {
        iActualReturn = mpFieldParameters->entries() ;
        return FALSE ;
    }
    else
    {
        iActualReturn = min(iMaxReturn, ((int)(mpFieldParameters->entries()))) ;

        for (int i=0; i<iActualReturn; i++)
        {
            NameValuePairInsensitive *pair = (NameValuePairInsensitive*) mpFieldParameters->at(i) ;
            pNames[i] = *pair;
            pValues[i] = pair->getValue() ;

        }
    }
    return (iActualReturn > 0) ;
}


void Url::setFieldParameter(const char* name, const char* value)
{
   if (name && *name != '\000')
   {
      // ensure that mpFieldParameters is initialized
      if (!mpFieldParameters)
      {
         mpFieldParameters = new UtlDList;
      }

      // create a new object to serve as a search key for an existing one (value is unimportant now)
      NameValuePairInsensitive* nv = new NameValuePairInsensitive(name, "");

      NameValuePairInsensitive* existingParam = dynamic_cast<NameValuePairInsensitive*>(mpFieldParameters->find(nv));

      if (existingParam)
      {
         delete nv;
         existingParam->setValue(value);
      }
      else
      {
         if ( value && *value != '\000' )
         {
            nv->setValue(value);
         }
         mpFieldParameters->append(nv);
      }
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_CRIT, "Url::setFieldParameter passed a null name");
      assert(false);
   }
}

void Url::removeFieldParameters()
{
   if(mpFieldParameters)
   {
      mpFieldParameters->destroyAll();
      delete mpFieldParameters;
      mpFieldParameters = NULL;
   }
}

void Url::removeFieldParameter(const char* name)
{
    if(mpFieldParameters)
    {
        NameValuePairInsensitive nv(name ? name : "", NULL);

        UtlContainable* matchingParam;
        while((matchingParam=mpFieldParameters->find(&nv)))
        {
           mpFieldParameters->removeReference(matchingParam);
           delete matchingParam;
        }
    }
}

void Url::includeAngleBrackets()
{
    mAngleBracketsIncluded = TRUE;
}

void Url::removeAngleBrackets()
{
    mAngleBracketsIncluded = FALSE;
}

UtlString Url::toString() const
{
    UtlString str;
    toString(str);
    return str;
}

void Url::toString(UtlString& urlString) const
{
   UtlBoolean isNameAddr = FALSE;

   // This is a replace operation; clear the storage string
   urlString.remove(0);

   if ( !mDisplayName.isNull() )
   {
      urlString.append(mDisplayName);
      isNameAddr = TRUE;
   }

   bool haveUrlParams = (   (   mpUrlParameters
                             || const_cast<Url*>(this)->parseUrlParameters()
                             )
                         && mpUrlParameters->entries()
                         );
   bool haveHdrParams = (   (   mpHeaderOrQueryParameters
                             || const_cast<Url*>(this)->parseHeaderOrQueryParameters()
                             )
                         && mpHeaderOrQueryParameters->entries()
                         );

   bool haveFldParams = (   mpFieldParameters
                         && mpFieldParameters->entries()
                         );

   // If this should be nameAddr as opposed to addrSpec
   // (i.e. do we need anglebrackets)
   if (   isNameAddr                             // There was a Display name
       || mAngleBracketsIncluded                 // Explicit setting from the caller
       || haveFldParams
       || (   ( SipUrlScheme == mScheme || SipsUrlScheme == mScheme )
           && ( haveUrlParams || haveHdrParams )
           )
       )
   {
       urlString.append("<", 1);
       isNameAddr = TRUE;
   }

   UtlString theAddrSpec;
   const_cast<Url*>(this)->getUri(theAddrSpec);
   urlString.append(theAddrSpec);

   // Add the terminating angle bracket
   if(isNameAddr)
   {
      urlString.append(">", 1);
   }

   // Add the field parameters
   if(haveFldParams)
   {
      UtlDListIterator fieldParamIterator(*mpFieldParameters);
      NameValuePairInsensitive* fieldParam = NULL;
      UtlString fieldParamValue;

      while ((fieldParam = dynamic_cast<NameValuePairInsensitive*>(fieldParamIterator())))
      {
         urlString.append(";", 1);
         urlString.append(*fieldParam);
         fieldParamValue = fieldParam->getValue();
         if(!fieldParamValue.isNull())
         {
            urlString.append("=", 1);
            Url::gen_value_escape(fieldParamValue);
            urlString.append(fieldParamValue);
         }
      }
   }
}

// Get a malloc'ed string containing the URI as a name-addr.
char* Url::getBytes() const
{
   UtlString buffer;

   // Write into the UtlString.
   toString(buffer);

   char* ret = (char*) malloc(buffer.length() + 1);
   assert(ret);
   memcpy(ret, buffer.data(), buffer.length() + 1);

   return ret;
}

void Url::dump()
{
    UtlString proto;
    getUrlType(proto);
    printf("Url type: '%s'\n", proto.data());

    UtlString disp;
    getDisplayName(disp);
    printf("DisplayName: '%s'\n", disp.data());

    UtlString user;
    getUserId(user);
    printf("UserId: '%s'\n", user.data());

    UtlString pwd;
    getPassword(pwd);
    printf("Password: '%s'\n", pwd.data());

    UtlString server;
    getHostAddress(server);
    printf("Address: '%s'\n", server.data());

    int port = getHostPort();
    printf("Port: %d\n", port);

    UtlString callId;
    getHeaderParameter("call-id", callId);
    printf("Call-Id: '%s'\n", callId.data());

    UtlString name;
    UtlString value;
    int index = 0;
    printf("\nHeader Parameters:\n");
    while(getHeaderParameter(index, name, value))
    {
        printf("'%s'='%s'\n", name.data(), value.data());
        index++;
    }

    index = 0;
    printf("\nField Parameters:\n");
    while(getFieldParameter(index, name, value))
    {
        printf("'%s'='%s'\n", name.data(), value.data());
        index++;
    }

    index = 0;
    printf("\nURL Parameters:\n");
    while(getUrlParameter(index, name, value))
    {
        printf("'%s'='%s'\n", name.data(), value.data());
        index++;
    }
}

void Url::kedump()
{
    UtlString proto;
    getUrlType(proto);
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "Url::kedump Url type: '%s'", proto.data());

    UtlString disp;
    getDisplayName(disp);
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "Url::kedump DisplayName: '%s'", disp.data());

    UtlString user;
    getUserId(user);
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "Url::kedump UserId: '%s'", user.data());

    UtlString pwd;
    getPassword(pwd);
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "Url::kedump Password: '%s'", pwd.data());

    UtlString server;
    getHostAddress(server);
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "Url::kedump Address: '%s'", server.data());

    int port = getHostPort();
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "Url::kedump Port: %d", port);

    UtlString callId;
    getHeaderParameter("call-id", callId);
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "Url::kedump Call-Id: '%s'", callId.data());

    UtlString name;
    UtlString value;
    int index = 0;
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "Url::kedump Header Parameters:");
    while(getHeaderParameter(index, name, value))
    {
        OsSysLog::add(FAC_SIP, PRI_DEBUG,
                      "Url::kedump '%s'='%s'", name.data(), value.data());
        index++;
    }

    index = 0;
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "Url::kedump Field Parameters:");
    while(getFieldParameter(index, name, value))
    {
        OsSysLog::add(FAC_SIP, PRI_DEBUG,
                      "Url::kedump '%s'='%s'", name.data(), value.data());
        index++;
    }

    index = 0;
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "Url::kedump URL Parameters:");
    while(getUrlParameter(index, name, value))
    {
        OsSysLog::add(FAC_SIP, PRI_DEBUG,
                      "Url::kedump '%s'='%s'", name.data(), value.data());
        index++;
    }
}

/* ============================ INQUIRY =================================== */

UtlBoolean Url::isDigitString(const char* dialedCharacters)
{
    // Must be digits or *
    RegEx allDigits(AllDigits);
    return allDigits.Search(dialedCharacters);
}


UtlBoolean Url::isIncludeAngleBracketsSet() const
{
    return mAngleBracketsIncluded ;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

// Special value used inside Url::parseString() to indicate that no
// recognizable scheme could be found at the start of the string.
// UnknownUrlScheme is used to indicate that "UNKNOWN-URL-SCHEME:" was
// found at the start.  Before returning, parseString turns
// UnrecognizableUrlScheme into UnknownUrlScheme.
static const Url::Scheme UnrecognizableUrlScheme =
   Url::NUM_SUPPORTED_URL_SCHEMES;

bool Url::parseString(const char* urlString, ///< string to parse URL from
                      UriForm     uriForm,   ///< which context should be used to parse the uri
                      UtlString*  nextUri    ///< any leftover value following a trailing comma
                      )
{
   // If uriForm == AddrSpec:
   //                userinfo@hostport;uriParameters?headerParameters
   // If uriForm == NameAddr:
   //    DisplayName<userinfo@hostport;urlParameters?headerParameters>;fieldParameters
   //    or:
   //    userinfo@hostport;fieldParameters

#  ifdef TIME_PARSE
   OsTimeLog timeLog;
   LOG_TIME("start    ");
#  endif
   // ensure that the leftover value is cleared out in any case
   if (nextUri)
   {
      nextUri->remove(0);
   }

   // Try to catch when a name-addr is passed but we are expecting an
   // addr-spec -- many name-addr's start with '<' or '"', but of course
   // addr-spec's (or any URI) cannot.
   if (AddrSpec == uriForm && (urlString[0] == '<' || urlString[0] == '"'))
   {
      OsSysLog::add(FAC_SIP, PRI_ERR, "Url::parseString "
                    "Invalid addr-spec found (probably name-addr format): '%s'",
                    urlString);
   }

   int workingOffset = 0; // begin at the beginning...

   ssize_t afterAngleBrackets = UTL_NOT_FOUND;

   if (AddrSpec == uriForm)
   {
      mAngleBracketsIncluded = FALSE;
   }
   else // ! addr-spec
   {
      // Is there a display name on the front?
      mDisplayName.remove(0);
      LOG_TIME("display   <");
      RegEx displayName(DisplayName);
      if (   displayName.SearchAt(urlString, workingOffset)
          && displayName.MatchStart(0) == workingOffset
         )
      {
         LOG_TIME("display   > ");
         switch (displayName.Matches() /* number of substrings that matched */)
         {
         case 2: // matched unquoted sequence of tokens
            displayName.MatchString(&mDisplayName, 1);
            break;

         case 3: // matched a double quoted string
            // see performance note on DisplayName
            mDisplayName.append("\"");
            displayName.MatchString(&mDisplayName, 2);
            mDisplayName.append("\"");
            break;

         default:
            OsSysLog::add(FAC_SIP, PRI_ERR, "Url::parseString display name invalid (matches: %d)\n"
                          "   %s value: '%s'",
                          displayName.Matches(),
                          uriForm == NameAddr ? "NameAddr" :
                          uriForm == AddrSpec ? "AddrSpec" :
                          "(unknown form)",
                          urlString);
         }

         // does not include whitespace or the '<'
         workingOffset = displayName.AfterMatch(0);
      }

      // Are there angle brackets around the URI?
      LOG_TIME("angles   < ");
      RegEx angleBrackets(AngleBrackets);
      if (   angleBrackets.SearchAt(urlString, workingOffset)
          && angleBrackets.MatchStart(0) == workingOffset
         )
      {
         LOG_TIME("angles   > ");
         // yes, there are angle brackets
         workingOffset = angleBrackets.MatchStart(1); // inside the angle brackets
         afterAngleBrackets = angleBrackets.AfterMatch(0); // following the '>'

         /*
          * Note: We do not set mAngleBracketsIncluded just because we saw them
          *       That is only used for explicit control from the outside.
          *       The local knowledge of whether or not there are angle brackets
          *       is whether or not afterAngleBrackets == UTL_NOT_FOUND
          */
      }
   }

   /*
    * AMBIGUITY - there is a potential ambiguity when parsing real URLs.
    *
    * Consider the url 'foo:333' - it could be:
    *       scheme 'foo' host '333' ('333' is a valid local host name - bad idea, but legal)
    *   or  host   'foo' port '333' (and scheme 'sip' is implied)
    *
    * Now make it worse by using 'sips' as a hostname:
    *   'sips:333'
    *       scheme 'sips' host '333'
    *   or  host   'sips' port '333' (and scheme 'sip' is implied)
    *
    * We resolve the first case by treating anything left of the colon as a scheme if
    * it is one of the supported schemes.  Otherwise, we set the scheme to the
    * default (sip) and go on so that it will be parsed as a hostname.  This does not
    * do the right thing for the (host 'sips' port '333') case, but they get what
    * they deserve for not writing the scheme explicitly (which they are supposed
    * to do).
    */

   // Parse the scheme (aka URI type)
   LOG_TIME("scheme   < ");
   RegEx supportedScheme(AddrSpec == uriForm ?
                         SupportedScheme :
                         SupportedSchemeSWS);
   if (   (supportedScheme.SearchAt(urlString,workingOffset))
       && (supportedScheme.MatchStart(0) == workingOffset)
       )
   {
      LOG_TIME("scheme   > ");
      // the scheme name matches one of the supported schemes
      // The first alternative in SUPPORTED_SCHEMES will cause
      // RegEx::Matches() to return SUPPORTED_SCHEMES_FIRST_MATCHES,
      // the second will return SUPPORTED_SCHEMES_FIRST_MATCHES+1, etc.
      // The first alternative is for scheme SUPPORTED_SCHEMES_FIRST_VALUE,
      // etc.
      mScheme = static_cast <Scheme> (
      	      supportedScheme.Matches()
                  - SUPPORTED_SCHEMES_FIRST_MATCHES
                  + SUPPORTED_SCHEMES_FIRST_VALUE
	      );
      workingOffset = supportedScheme.AfterMatch(0); // past the ':'
   }
   else
   {
      /*
       * It did not match one of the supported scheme names so proceed
       * on the assumption that the text is a host and "sip:" is
       * implied.  Leave the workingOffset where it is (before the
       * token).  The code below, through the parsing of host and port,
       * treats this as an implicit 'sip:' url; if it parses OK up to
       * that point, it resets the scheme to SipsUrlScheme.
       */
      mScheme = UnrecognizableUrlScheme;
   }

   // skip over any '//' following the scheme for the ones we know use that
   switch (mScheme)
   {
   case FileUrlScheme:
   case FtpUrlScheme:
   case HttpUrlScheme:
   case HttpsUrlScheme:
      if (0==strncmp("//", urlString+workingOffset, 2))
      {
         workingOffset += 2;
      }
      break;

   case UnknownUrlScheme:
   case SipUrlScheme:
   case SipsUrlScheme:
   case MailtoUrlScheme:
   case UnrecognizableUrlScheme:
   default:
      break;
   }

   if (FileUrlScheme != mScheme) // no user part in file urls
   {
      // Parse the username and password
      LOG_TIME("userpass   < ");
      RegEx usernameAndPassword(UsernameAndPassword);
      if (   usernameAndPassword.SearchAt(urlString, workingOffset)
          && usernameAndPassword.MatchStart(0) == workingOffset
          )
      {
         LOG_TIME("userpass   > ");
         usernameAndPassword.MatchString(&mUserId, 1);
         usernameAndPassword.MatchString(&mPassword, 2);
         workingOffset = usernameAndPassword.AfterMatch(0);
      }
      else
      {
         // username and password are optional, so not finding them is ok
         // leave workingOffset where it is
      }
   }

   // Parse the hostname and port
   LOG_TIME("hostport   < ");
   RegEx hostAndPort(HostAndPort);
   if (   (hostAndPort.SearchAt(urlString,workingOffset))
       && (hostAndPort.MatchStart(0) == workingOffset)
       )
   {
      LOG_TIME("hostport   > ");
      hostAndPort.MatchString(&mHostAddress,1);
      UtlString portStr;
      if (hostAndPort.MatchString(&portStr,2))
      {
         mHostPort = atoi(portStr.data());
      }

      workingOffset = hostAndPort.AfterMatch(0);

      if (UnrecognizableUrlScheme == mScheme)
      {
         /*
          * Resolve AMBIGUITY
          *   Since we were able to parse this as a host and port, it
          *   is now safe to set the scheme to the implied 'sip:'.
          * Note that if the URI started with "UNKNOWN-URL-SCHEME:",
          * mScheme == UnknownUrlScheme, and this assignment is not done.
          */
         mScheme = SipUrlScheme;
      }
   }
   else
   {
      if (FileUrlScheme != mScheme) // no host is ok in a file URL
      {
         /*
          * This is not a file URL, so not having a recognizable host name is invalid.
          *
          * Since we may have been called from a constructor, there is no way to
          * return an error, but at this point we know this is bad, so instead
          * we just log an error and set the scheme to the unknown url type and
          * clear any components that might have been set.
          */
         OsSysLog::add(FAC_SIP, PRI_ERR,
                       "Url::parseString no valid host found at char %d in '%s', "
                       "uriForm = %s",
                       workingOffset, urlString,
                       (AddrSpec == uriForm ? "addr-spec" :
                        NameAddr == uriForm ? "name-addr" : "INVALID")
                       );
         mScheme = UnknownUrlScheme;
         mDisplayName.remove(0);
         mUserId.remove(0);
         mPassword.remove(0);
      }
   }

   // At this point, we have decided whether or not to assume an
   // implicit "sip:", so we no longer need to distinguish URIs
   // without a recognizable scheme from those that are explicitly
   // marked "UNKNOWN-URL-SCHEME:".
   if (UnrecognizableUrlScheme == mScheme)
   {
      mScheme = UnknownUrlScheme;
   }

   // Next is a path if http, https, or ftp,
   //      OR url parameters if sip or sips.
   // There can be no Url parameters for http, https, or ftp
   //    because semicolon is a valid part of the path value
   switch (mScheme)
   {
   case FileUrlScheme:
   case FtpUrlScheme:
   case HttpUrlScheme:
   case HttpsUrlScheme:
   {
      // this is an http, https, or ftp URL, so get the path
      LOG_TIME("path   < ");
      RegEx urlPath(UrlPath);
      if (   (urlPath.SearchAt(urlString, workingOffset))
          && (urlPath.MatchStart(0) == workingOffset)
          )
      {
         LOG_TIME("path   > ");
         urlPath.MatchString(&mPath,1);
         workingOffset = urlPath.AfterMatch(1);
      }
#     ifdef _WIN32
      {
         // Massage Data under Windows:  C|/foo.txt --> C:\foo.txt
         mPath.replace('|', ':');
         mPath.replace('/', '\\');
      }
#     endif
   }
   break;

   case SipUrlScheme:
   case SipsUrlScheme:
   {
      // it may have url parameters of the form ";" param "=" value ...
      //                iff it meets the right conditions:
      if (   AddrSpec == uriForm                 // in addr-spec, any param is a url param
          || afterAngleBrackets != UTL_NOT_FOUND // inside angle brackets there may be a url param
          )
      {
         LOG_TIME("urlparm   < ");
         RegEx urlParams(UrlParams);
         if (   (urlParams.SearchAt(urlString, workingOffset))
             && (urlParams.MatchStart(0) == workingOffset)
             )
         {
            LOG_TIME("urlparm   > ");
            urlParams.MatchString(&mRawUrlParameters, 1);
            workingOffset = urlParams.AfterMatch(1);

            // actual parsing of the parameters is in parseUrlParameters
            // so that it only happens if someone asks for them.
         }
      }
   }
   break;

   case MailtoUrlScheme:
   default:
      // no path component
      break;
   }

   if (UnknownUrlScheme != mScheme)
   {
      // Parse any header or query parameters
      LOG_TIME("hdrparm   < ");
      RegEx headerOrQueryParams(HeaderOrQueryParams);
      if(   (headerOrQueryParams.SearchAt(urlString, workingOffset))
         && (headerOrQueryParams.MatchStart(0) == workingOffset)
         )
      {
         LOG_TIME("hdrparm   > ");
         headerOrQueryParams.MatchString(&mRawHeaderOrQueryParameters, 1);
         workingOffset = headerOrQueryParams.AfterMatch(0);

         // actual parsing of the parameters is in parseHeaderOrQueryParameters
         // so that it only happens if someone asks for them.
      }

      // Parse the field parameters
      if (NameAddr == uriForm) // can't have field parameters in an AddrSpec
      {
         // If '<...>' was seen, workingOffset should be just before '>'.
         if (afterAngleBrackets != UTL_NOT_FOUND)
         {
            if ((ssize_t) (workingOffset+1) == afterAngleBrackets)
            {
               // Advance to after '>'.
               workingOffset = afterAngleBrackets;
            }
            else
            {
               mScheme = UnknownUrlScheme;
            }
         }

         // If there was no trouble about '>', continue with parsing
         // field parameters.
         LOG_TIME("fldparm   < ");
         RegEx commaSeparator(CommaSeparator);
         RegEx endUrl(EndUrl);
         RegEx fieldParamNameEquals(FieldParamName);
         RegEx fieldParamEqualsUnquotedValue(FieldParamEqualsUnquotedValue);
         RegEx fieldParamEqualsQuotedValue(FieldParamEqualsQuotedValue);

         bool finishedFieldParams = false;
         while ( !finishedFieldParams && UnknownUrlScheme != mScheme )
         {
            if (   (endUrl.SearchAt(urlString, workingOffset))
                && (endUrl.MatchStart(0) == workingOffset))
            {
               workingOffset = endUrl.AfterMatch(0);
               finishedFieldParams = true;
            }
            else if (   (commaSeparator.SearchAt(urlString, workingOffset))
                     && (commaSeparator.MatchStart(0) == workingOffset) )
            {
               // Do not advance workingOffset, so that it remains
               // pointing at the comma separator.  The code below
               // that checks that the URI parsing ended at the
               // correct location needs to have workingOffset
               // pointing at the comma separator.
               finishedFieldParams = true;
            }
            else if (   (fieldParamNameEquals.SearchAt(urlString, workingOffset))
                     && (fieldParamNameEquals.MatchStart(0) == workingOffset) )
            {
               UtlString fieldParamName;
               UtlString fieldParamValue;
               fieldParamNameEquals.MatchString(&fieldParamName, 1);
               workingOffset = fieldParamNameEquals.AfterMatch(0);

               if (   (fieldParamEqualsUnquotedValue.SearchAt(urlString, workingOffset))
                   && (fieldParamEqualsUnquotedValue.MatchStart(0) == workingOffset) )
               {
                  fieldParamEqualsUnquotedValue.MatchString(&fieldParamValue, 1);
                  workingOffset = fieldParamEqualsUnquotedValue.AfterMatch(0);
               }
               else if (   (fieldParamEqualsQuotedValue.SearchAt(urlString, workingOffset))
                        && (fieldParamEqualsQuotedValue.MatchStart(0) == workingOffset) )
               {
                  fieldParamEqualsQuotedValue.MatchString(&fieldParamValue, 1);
                  workingOffset = fieldParamEqualsQuotedValue.AfterMatch(0);
               }

               gen_value_unescape(fieldParamName);
               gen_value_unescape(fieldParamValue);

               if (!mpFieldParameters)
               {
                  mpFieldParameters = new UtlDList();
               }

               mpFieldParameters->append(
                  new NameValuePairInsensitive(fieldParamName.data(), fieldParamValue.data()));
            }
            else
            {
               OsSysLog::add(FAC_SIP, PRI_ERR,
                             "Url::parseString error "
                             "- expected end of url or field parameter ';name=' "
                             "at offset %d in '%s'",
                             workingOffset, urlString
                             );

               mScheme=UnknownUrlScheme;
            }
         }
         LOG_TIME("fldparm   > ");
      }
   }

   if (UnknownUrlScheme != mScheme)
   {
      // At this point, the parse has reached the end of the URI, or the end
      // of what could be parsed.  Based on uriForm and nextUri, determine
      // if the parse is successful and return the "remainder of the string"
      // value.
      {
         // Select the regexp that describes allowed following characters.
         const RegEx* test =
            AddrSpec == uriForm ?
            (nextUri ? &EndComma : &End) :
            (nextUri ? &EndSwsComma : &EndSws);
         RegEx re(*test);

         // Match the regexp.
         if (   (re.SearchAt(urlString, workingOffset))
             && (re.MatchStart(0) == workingOffset)
             )
         {
            // If the match succeeded and nextUri != NULL, store into nextUri.
            if (nextUri)
            {
               re.AfterMatchString(nextUri);
            }
         }
         else
         {
            // If the match failed, mark the Url as invalid.
            mScheme = UnknownUrlScheme;
         }
      }
   }

#  ifdef TIME_PARSE
     UtlString timeDump;
   timeLog.getLogString(timeDump);
   printf("\n%s\n", timeDump.data());
#  endif

   return UnknownUrlScheme != mScheme;
}

UtlBoolean Url::isUserHostPortEqual(const Url &url,
                                    int impliedPort
                                    ) const
{
   // Compare the relevant components of the URI.
   int otherPort;
   int myPort;
   if (impliedPort == PORT_NONE)
   {
      // strict checking - no implied port values
      otherPort = url.mHostPort;
      myPort    = mHostPort;
   }
   else
   {
      // sloppy checking - an unspecified port is considered to be impliedPort
      otherPort = ( url.mHostPort == PORT_NONE ) ? impliedPort : url.mHostPort;
      myPort    = ( mHostPort     == PORT_NONE ) ? impliedPort : mHostPort;
   }

   return ((myPort == otherPort) && isUserHostEqual(url));
}

UtlBoolean Url::isUserHostEqual(const Url &url) const
{

   return (   (mHostAddress.compareTo(url.mHostAddress, UtlString::ignoreCase) == 0)
           && (mUserId.compareTo(url.mUserId) == 0));
}


void Url::getIdentity(UtlString &identity) const
{
   identity.remove(0);
   identity.append(mUserId);
   identity.append("@", 1);
   UtlString lowerHostAddress(mHostAddress);
   lowerHostAddress.toLower();
   identity.append(lowerHostAddress);

   // If the port designates an actual port, it must be specified.
   if(portIsValid(mHostPort))
   {
      char portBuffer[20];
      sprintf(portBuffer, ":%d", mHostPort);
      identity.append(portBuffer);
   }
}

/// Translate a scheme string (not including the terminating colon) to a Scheme enum.
Url::Scheme Url::scheme(const UtlString& schemeName)
{
   Scheme theScheme;

   RegEx supportedSchemeExact(SupportedSchemeExact);
   if (supportedSchemeExact.Search(schemeName.data()))
   {
      // The first alternative in SUPPORTED_SCHEMES will cause
      // RegEx::Matches() to return 2, the second will return 3, etc.
      // So subtract 2 from ::Matches() and add the code for the first
      // alternative to generate the Scheme value.
      theScheme = static_cast <Scheme> (
         supportedSchemeExact.Matches()
             - SUPPORTED_SCHEMES_FIRST_MATCHES
             + SUPPORTED_SCHEMES_FIRST_VALUE
         );
   }
   else
   {
      theScheme = UnknownUrlScheme;
   }
   return theScheme;
}


/// Get the canonical (lowercase) name of a supported Scheme.
const char* Url::schemeName(Url::Scheme scheme)
{
   const char* theName;
   if (scheme > UnknownUrlScheme && scheme < NUM_SUPPORTED_URL_SCHEMES)
   {
      theName = SchemeName[scheme];
   }
   else
   {
      theName = SchemeName[UnknownUrlScheme];
   }
   return theName;
}


/* //////////////////////////// PRIVATE /////////////////////////////////// */

bool Url::parseUrlParameters() const
{
   if (!mpUrlParameters && !mRawUrlParameters.isNull())
   {
      mpUrlParameters = new UtlDList();

      HttpRequestContext::parseCgiVariables(mRawUrlParameters,
                                            *mpUrlParameters, ";", "=",
                                            TRUE, &HttpMessage::unescape);
      mRawUrlParameters.remove(0);
   }

   return mpUrlParameters != NULL;
}

bool Url::parseHeaderOrQueryParameters() const
{
   if (!mpHeaderOrQueryParameters && !mRawHeaderOrQueryParameters.isNull())
   {
      mpHeaderOrQueryParameters = new UtlDList();

      HttpRequestContext::parseCgiVariables(mRawHeaderOrQueryParameters,
                                            *mpHeaderOrQueryParameters, "&", "=",
                                            TRUE, &HttpMessage::unescape);
      mRawHeaderOrQueryParameters.remove(0);
   }

   return mpHeaderOrQueryParameters != NULL;
}

void Url::gen_value_unescape(UtlString& escapedText)
{
#if 0
   printf("Url::gen_value_unescape before escapedText = '%s'\n",
          escapedText.data());
#endif

    int numUnescapedChars = 0;
    const char* unescapedTextPtr = escapedText;
    // The number of unescaped characters is always less than
    // or equal to the number of escaped characters.  Therefore
    // we will cheat a little and used the escapedText as
    // the destination to directly write characters in place
    // as the append method is very expensive
    char* resultPtr = new char[escapedText.length() + 1];

    // Skip initial whitespace, which may be before the starting double-quote
    // of a quoted string.  Tokens and hosts are not allowed to start with
    // whitespace.
    while (*unescapedTextPtr &&
           (*unescapedTextPtr == ' ' || *unescapedTextPtr == '\t'))
    {
       // Consume the whitespace character.
       unescapedTextPtr++;
       numUnescapedChars++;
    }

    // Examine the first character to see if it is a double-quote.
    if (*unescapedTextPtr == '"')
    {
       // Skip the initial double-quote.
       unescapedTextPtr++;
       while (*unescapedTextPtr)
       {
          // Substitute a (backslash-)quoted-pair.
          if (*unescapedTextPtr == '\\')
          {
             // Get the next char.
             unescapedTextPtr++;
             // Don't get deceived if there is no next character.
             if (*unescapedTextPtr)
             {
                // The next character is copied unchanged.
                resultPtr[numUnescapedChars] = *unescapedTextPtr;
                numUnescapedChars++;
             }
          }
          // A double-quote without backslash ends the string.
          else if (*unescapedTextPtr == '"')
          {
             break;
          }
          // Char is face value.
          else
          {
             resultPtr[numUnescapedChars] = *unescapedTextPtr;
             numUnescapedChars++;
          }
          // Go to the next character
          unescapedTextPtr++;
       }
    }
    else
    {
       // It is a token or host, and can be copied unchanged.
       while (*unescapedTextPtr)
       {
          resultPtr[numUnescapedChars] = *unescapedTextPtr;
          numUnescapedChars++;
          // Go to the next character
          unescapedTextPtr++;
       }
    }

    // Copy back into the UtlString.
    resultPtr[numUnescapedChars] = '\0';
    escapedText.replace(0, numUnescapedChars, resultPtr);
    escapedText.remove(numUnescapedChars);
    delete[] resultPtr;
   
#if 0
   printf("Url::gen_value_unescape after escapedText = '%s'\n",
          escapedText.data());
#endif
}

void Url::gen_value_escape(UtlString& unEscapedText)
{
   // Check if there are any characters in unEscapedText that need to be
   // escaped in a field parameter value.
   
   RegEx fieldParamValueOkUnquoted(FieldParamValueOkUnquoted);

   UtlString fieldValue;
   if (!fieldParamValueOkUnquoted.Search(unEscapedText))
   {
      // Temporary string to construct the escaped value in.
      UtlString escapedText;
      // Pre-size it to the size of the un-escaped test, plus 2 for
      // the starting and ending double-quotes.
      escapedText.capacity((size_t) unEscapedText.length() + 2);
      const char* unescapedTextPtr = unEscapedText.data();

      // Start with double-quote.
      escapedText.append("\"");

      // Process each character of the un-escaped value.
      while(*unescapedTextPtr)
      {
         char unEscapedChar = *unescapedTextPtr;
         if (unEscapedChar == '"' || unEscapedChar == '\\')
         {
            // Construct a little 2-character string and append it.
            char escapedChar[2];
            escapedChar[0] = '\\';
            escapedChar[1] = *unescapedTextPtr;
            escapedText.append(&unEscapedChar, 2);
        }
        else
        {
           // Append the character directly.
           escapedText.append(&unEscapedChar, 1);
        }
         // Consider the next character.
         unescapedTextPtr++;
      }

      // End with double-quote.
      escapedText.append("\"");

      // Write the escaped string into the argumemt.
      unEscapedText = escapedText;
   }
}


/// Is this a Globally Routable UA URI?
bool Url::isGRUU() const
{
   // for now we just test to see if it has the standard gruu marker parameter
   UtlString valueNotUsed;
   return (   ( SipUrlScheme == mScheme || SipsUrlScheme == mScheme )
           && getUrlParameter(SIP_GRUU_URI_PARAM, valueNotUsed)
           );
}

/// Mark as being a Globally Routable UA URI
void Url::setGRUU(const UtlString& uniqueId)
{
   if ( SipUrlScheme == mScheme || SipsUrlScheme == mScheme )
   {
      setUrlParameter(SIP_GRUU_URI_PARAM, uniqueId.data());
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_CRIT, "Url::setGRUU called for an invalid URL scheme (%s)",
                    schemeName(mScheme)
                    );
      assert(false);
   }
}


/* ============================ FUNCTIONS ================================= */
