// 
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef MAPPINGRULESURLMAPPING_H
#define MAPPINGRULESURLMAPPING_H

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include "os/OsStatus.h"
#include "sipdb/ResultSet.h"
#include "xmlparser/tinyxml.h"
#include "digitmaps/Patterns.h"
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
class Url;
class RegEx;

/**
 * This class interprets the rules encoded by the mappingrules XML schema
 * (see the description elements in the schem files for the structure and
 * contents of the file):
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
class MappingRulesUrlMapping : public UrlMapping
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   MappingRulesUrlMapping();

    virtual ~MappingRulesUrlMapping();
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

   /// Evaluate a request URI using mapping rules semantics, return contacts and permissions.
   OsStatus getContactList(const Url& requestUri,   ///< target to check
                           ResultSet& rContacts,    ///< contacts generated from first match
                           ResultSet& rPermissions, ///< permissions that target must have
                           UtlString& callTag       ///< call tag for contacts.
                           ) const;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   OsStatus parsePermMatchContainer(const Url& requestUri,
                                    const UtlString& vdigits,
                                    ResultSet& rRegistratons,
                                    ResultSet& rPermissions,
                                    UtlString& callTag,
                                    const TiXmlNode* pUserMatchNode
                                    ) const;

};

/* ============================ INLINE METHODS ============================ */

#endif  // MAPPINGRULESURLMAPPING_H

