// 
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef FALLBACKRULESURLMAPPING_H
#define FALLBACKRULESURLMAPPING_H

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
 * This class interprets the rules encoded by the fallbackrules XML schema
 * (see the description elements in the schem files for the structure and
 * contents of the file):
 *
 * - fallbackrules: sipXcommserverLib/meta/fallbackrules.xml
 *   Specifies a set of rules for transforming a target URI into one or
 *   more new targets based on the request-URI and the location of the caller.
 */

class FallbackRulesUrlMapping : public UrlMapping
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   FallbackRulesUrlMapping();

    virtual ~FallbackRulesUrlMapping();
    //:Destructor

/* ============================ MANIPULATORS ============================== */

    /// Read a mappings file into the XML DOM, providing translations for replacement tokens.
    OsStatus loadMappings( const UtlString& configFileName );

/* ============================ ACCESSORS ================================= */

   /// Evaluate a request URI using forwarding rules semantics as well and as the
   /// location of the caller and return corresponding contacts 
   OsStatus getContactList(const Url& requestUri,    ///< target to check
                           const UtlString& location,///< location of the requesting user 
                           ResultSet& rContactsr,      ///< contacts generated from first match
                           UtlString& callTag        ///< call tag for contacts.
                           ) const;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
};

/* ============================ INLINE METHODS ============================ */

#endif  // FALLBACKRULESURLMAPPING_H

