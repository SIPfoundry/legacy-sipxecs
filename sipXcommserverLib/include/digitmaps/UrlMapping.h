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

/* ============================ MANIPULATORS ============================== */

    /// Read a mappings file into the XML DOM, providing translations for replacement tokens.
    OsStatus loadMappings(
        const UtlString& configFileName,
        const UtlString& mediaserver = "",
        const UtlString& voicemail = "",
        const UtlString& localhost = ""
                          );

/* ============================ ACCESSORS ================================= */

    /// Evaluate a request URI using authrules semantics, and return the set of permissions.
    OsStatus getPermissionRequired(const Url& requestUri,  ///< target to check
                                   ResultSet& rPermissions ///< required permissions (or'ed) 
                                   );

    /// Evaluate a request URI using mapping rules semantics, return contacts and permissions.
    OsStatus getContactList(const Url& requestUri,   ///< target to check
                            ResultSet& rContacts,    ///< contacts generated from first match
                            ResultSet& rPermissions  ///< permissions that target must have
                            );

    static void convertRegularExpression(const UtlString& source,
                                         UtlString& regExp);

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    TiXmlNode* mPrevMappingNode;
    TiXmlElement* mPrevMappingElement;
    TiXmlNode* mPrevHostMatchNode;
    TiXmlNode* mPrevUserMatchNode;
    TiXmlNode* mPrevPermMatchNode;
    TiXmlDocument *mDoc;
    UtlBoolean mParseFirstTime;
    Patterns *mPatterns ;
    UtlString mVoicemail;
    UtlString mLocalhost;
    UtlString mMediaServer;

    OsStatus parseHostMatchContainer(const Url& requestUri,
                                     ResultSet& rRegistratons,
                                     UtlBoolean& doTransform,
                                     ResultSet& rPermissions,
                                     TiXmlNode* mappingsNode,
                                     TiXmlNode* previousHostMatchNode = NULL
                                     );

    OsStatus parseUserMatchContainer(const Url& requestUri,
                                     ResultSet& rRegistratons,
                                     UtlBoolean& doTransform,
                                     ResultSet& rPermissions,
                                     TiXmlNode* hostMatchNode,
                                     TiXmlNode* previousUserMatchNode = NULL
                                     );

    OsStatus parsePermMatchContainer(const Url& requestUri,
                                     const UtlString& vdigits,
                                     ResultSet& rRegistratons,
                                     UtlBoolean& doTransform,
                                     ResultSet& rPermissions,
                                     TiXmlNode* userMatchNode,
                                     TiXmlNode* previousPermMatchNode = NULL
                                     );

    OsStatus doTransform(const Url& requestUri,
                         const UtlString& vdigits,
                         ResultSet& rRegistratons,
                         TiXmlNode* permMatchNode
                         );

    /// Get the name/value pair from an element; supports two syntaxes.
    bool getNamedAttribute(TiXmlElement* component, ///< the xml element to interpret
                           UtlString&    name,      ///< the content name
                           UtlString&    value      ///< the content value
                           );
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
                    );

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

/* ============================ INLINE METHODS ============================ */

#endif  // URLMAPPING_H

