//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef URLMAPPING_H
#define URLMAPPING_H

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include "os/OsStatus.h"
#include "sipdb/ResultSet.h"
#include "xmlparser/tinyxml.h"
#include "digitmaps/Patterns.h"

// DEFINES
#define XML_TAG_MAPPINGS             ("mappings")

#define XML_TAG_HOSTMATCH            ("hostMatch")
#define XML_TAG_HOSTPATTERN          ("hostPattern")
#define XML_ATT_FORMAT               ("format")
#define XML_SYMBOL_URL               ("url")
#define XML_SYMBOL_IPV4SUBNET        ("IPv4subnet")
#define XML_SYMBOL_DNSWILDCARD       ("DnsWildcard")

#define XML_TAG_USERMATCH            ("userMatch")
#define XML_TAG_USERPATTERN          ("userPattern")

#define XML_TAG_PERMISSIONMATCH      ("permissionMatch")
#define XML_TAG_PERMISSION           ("permission")
#define XML_ATT_AUTHTYPE             ("authType")

#define XML_TAG_CALLERLOCATIONMATCH  ("callerLocationMatch")
#define XML_TAG_CALLERLOCATION       ("callerLocation")

#define XML_PERMISSION_911           ("emergency-dialing")
#define XML_PERMISSION_1800          ("1800-dialing")
#define XML_PERMISSION_1900          ("1900-dialing")
#define XML_PERMISSION_1877          ("1877-dialing")
#define XML_PERMISSION_1888          ("1888-dialing")
#define XML_PERMISSION_1             ("domestic-dialing")
#define XML_PERMISSION_011           ("international-dialing")

#define XML_TAG_TRANSFORM            ("transform")
#define XML_TAG_URL                  ("url")
#define XML_TAG_HOST                 ("host")
#define XML_TAG_USER                 ("user")
#define XML_TAG_FIELDPARAMS          ("fieldparams")
#define XML_TAG_URLPARAMS            ("urlparams")
#define XML_TAG_HEADERPARAMS         ("headerparams")
#define XML_TAG_CALLTAG              ("callTag")

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class TiXmlNode;
class Url;
class RegEx;

/**
 * This class interprets the rules encoded by two XML schemas (see
 * the description elements in the schema files for the structure and
 * contents of the files):
 *
 * - AuthRules: sipXcommserverLib/meta/authrules.xml
 *   Specifies a set of rules for what permissions are required to send a
 *   request to a particular target URI.  In this mode, the permissions
 *   returned (see getPermissionRequired) must be checked against those
 *   that the authorizing party has - if the authorizing party has any one
 *   of the required permissions, then the call is allowed.  This mode is
 *   is used in the proxy (specifically the EnforceAuthRules AuthPlugin).
 *
 * - MappingRules: sipXcommserverLib/meta/urlmap.xml
 *   Specifies a set of rules for transforming a target URI into one or
 *   more new targets.  In this mode, the permissions returned (see
 *   getContactList) must be checked against those that the target has.
 *   In this mode, 'capabilities' or 'attributes' would better describe
 *   the real function of the 'permissions'; if the target has any one
 *   of the 'permissions', then the contact set is used.  This mode is
 *   is used in the redirect server (specifically the SipRedirectorMapping
 *   plugin).
 */
class UrlMapping
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

    UrlMapping();

    virtual ~UrlMapping();
    //:Destructor

    /// Read a mappings file into the XML DOM, providing translations for replacement tokens.
    virtual OsStatus loadMappings(const UtlString& configFileName,
                        const UtlString& mediaserver = "",
                        const UtlString& voicemail = "",
                        const UtlString& localhost = ""
                        );

   static void convertRegularExpression(const UtlString& source,
                                        UtlString& regExp);


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    // returns a pointer to the <userMatch> XML node that matches the
    // user and domain patterns of the supplied request-URI
    OsStatus getUserMatchContainerMatchingRequestURI(const Url& requestUri,
                                                     UtlString& variableDigits,
                                                     const TiXmlNode*& prMatchingUserMatchContainerNode,
                                                     const TiXmlNode*& prMatchingHostMatchContainerNode,
                                                     const char* ruleType = NULL  ///< e.g. Emergency to match only emerg rules
                                                     ) const;

    OsStatus doTransform(const Url& requestUri,
                         const UtlString& vdigits,
                         ResultSet& rRegistratons,
                         const TiXmlNode* permMatchNode
                         ) const;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   TiXmlDocument *mDoc;
   UtlBoolean mParseFirstTime;
   Patterns *mPatterns ;
   UtlString mVoicemail;
   UtlString mLocalhost;
   UtlString mMediaServer;

   OsStatus getUserMatchContainer(const Url&              requestUri,
                                  const TiXmlNode* const  pHostMatchNode,
                                  UtlString&              variableDigits,
                                  const TiXmlNode*&       prMatchingUserMatchContainerNode
                                  ) const;

   /// Get the name/value pair from an element; supports two syntaxes.
   bool getNamedAttribute(const TiXmlElement* component, ///< the xml element to interpret
                          UtlString&    name,            ///< the content name
                          UtlString&    value            ///< the content value
                          ) const;
   /**<
    * @returns true iff a name/value pair was found.
    * Supports two syntaxes; suppose that an attribute 'bar' exists.
    * It may be encoded in either of two ways:
    * @code
    * Old Syntax:
    *            <foo>bar=value</foo>
    * New Syntax:
    *            <foo name='bar'>value</foo>
    * @endcode
    * The New syntax is preferred because it eliminates some obnoxious parsing ambiguities.
    */

   void getVDigits(RegEx& userPattern,
                   UtlString& vdigits
                   ) const;


};

/* ============================ INLINE METHODS ============================ */

#endif  // URLMAPPING_H
