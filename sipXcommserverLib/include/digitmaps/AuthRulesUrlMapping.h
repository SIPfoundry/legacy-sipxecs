// 
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef AUTHRULESURLMAPPING_H
#define AUTHRULESURLMAPPING_H

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include "os/OsStatus.h"
#include "sipdb/ResultSet.h"
#include "digitmaps/UrlMapping.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class TiXmlNode;

/**
 * This class interprets the rules encoded by the authrules XML schema
 * (see the description elements in the schem files for the structure and
 * contents of the file):
 *
 * - AuthRules: sipXcommserverLib/meta/authrules.xml
 *   Specifies a set of rules for what permissions are required to send a
 *   request to a particular target URI.  The permissions returned
 *   (see getPermissionRequired) must be checked against those
 *   that the authorizing party has - if the authorizing party has any one
 *   of the required permissions, then the call is allowed.  This mode is
 *   is used in the proxy (specifically the EnforceAuthRules AuthPlugin).
 *
 */
class AuthRulesUrlMapping : public UrlMapping
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   AuthRulesUrlMapping();

    virtual ~AuthRulesUrlMapping();
    //:Destructor

/* ============================ MANIPULATORS ============================== */

    /// Read a mappings file into the XML DOM, providing translations for replacement tokens.
    virtual OsStatus loadMappings(
        const UtlString& configFileName,
        const UtlString& mediaserver = "",
        const UtlString& voicemail = "",
        const UtlString& localhost = ""
                          );

/* ============================ ACCESSORS ================================= */

    /// Evaluate a request URI using authrules semantics, and return the set of permissions.
    OsStatus getPermissionRequired(const Url& requestUri,  ///< target to check
                                   ResultSet& rPermissions ///< required permissions (or'ed) 
                                   ) const;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

private:
   OsStatus
   parsePermMatchContainer( const Url& requestUri,
                            const UtlString& vdigits,
                            ResultSet& rPermissions,
                            const TiXmlNode* pUserMatchNode ) const;


};

/* ============================ INLINE METHODS ============================ */

#endif  // AUTHRULESURLMAPPING_H

