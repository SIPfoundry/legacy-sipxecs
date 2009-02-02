//
//
// Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef EMERGENCYRULESURLMAPPING_H
#define EMERGENCYRULESURLMAPPING_H

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsStatus.h"
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
 * This class interprets the rules encoded by the authrules XML schema,
 * looking only at Emergency rules
 * (see the description elements in the schema files for the structure and
 * contents of the file):
 *
 * - AuthRules: sipXcommserverLib/meta/authrules.xml
 *   Specifies a set of rules including dial rule types.
 *   If a user dials an emergency number, en email notification is sent.
 *   This mode is used in the proxy (specifically the EmergencyRules AuthPlugin).
 *
 */
class EmergencyRulesUrlMapping : public UrlMapping
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   EmergencyRulesUrlMapping();

   /// destructor
   virtual ~EmergencyRulesUrlMapping();

/* ============================ MANIPULATORS ============================== */

    /// Read a mappings file into the XML DOM, providing translations for replacement tokens.
    virtual OsStatus loadMappings(
        const UtlString& configFileName,
        const UtlString& mediaserver = "",
        const UtlString& voicemail = "",
        const UtlString& localhost = ""
                          );

/* ============================ ACCESSORS ================================= */

    /// Evaluate a request URI using emergencyrules semantics, and return rule details.
    bool getMatchedRule(const Url& requestUri,                 ///< target to check
                                   UtlString& rNameStr,        ///< name of the rule that matched
                                   UtlString& rDescriptionStr  ///< description of the rule that matched
                                   ) const;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

};

/* ============================ INLINE METHODS ============================ */

#endif  // EMERGENCYRULESURLMAPPING_H

