//
//
// Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>
#include <stdlib.h>

// APPLICATION INCLUDES
#include <os/OsSysLog.h>
#include <utl/UtlString.h>
#include <net/Url.h>
#include <digitmaps/EmergencyRulesUrlMapping.h>
#include "xmlparser/tinyxml.h"
#include "xmlparser/ExtractContent.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const char* EMERGENCY_RULE = "Emergency";


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
EmergencyRulesUrlMapping::EmergencyRulesUrlMapping()
{
}

// Destructor
EmergencyRulesUrlMapping::~EmergencyRulesUrlMapping()
{
}

/* ============================ MANIPULATORS ============================== */

OsStatus
EmergencyRulesUrlMapping::loadMappings(const UtlString& configFileName,
                         const UtlString& mediaserver,
                         const UtlString& voicemail,
                         const UtlString& localhost)
{
    return UrlMapping::loadMappings( configFileName, mediaserver, voicemail, localhost );
}

/* ============================ ACCESSORS ================================= */

bool
EmergencyRulesUrlMapping::getMatchedRule(const Url& requestUri,
                                         UtlString& rNameStr,
                                         UtlString& rDescriptionStr
                                         ) const
{
   bool numberMatched = false;
   UtlString variableDigits;
   const TiXmlNode* pMatchingUserMatchContainerNode = 0;
   const TiXmlNode* pMatchingHostMatchContainerNode = 0;

   if (getUserMatchContainerMatchingRequestURI(requestUri,
                                               variableDigits,
                                               pMatchingUserMatchContainerNode,
                                               pMatchingHostMatchContainerNode,
                                               EMERGENCY_RULE) == OS_SUCCESS )
   {
      // save the name
      if ( pMatchingHostMatchContainerNode != 0 )
      {
         numberMatched = true;

         const TiXmlNode*  nameNode = pMatchingHostMatchContainerNode->FirstChild("name");
         if (nameNode)
         {
            textContentShallow(rNameStr, nameNode);
         }

         // save the description
         const TiXmlNode*  descriptionNode = pMatchingHostMatchContainerNode->FirstChild("description");
         if (descriptionNode)
         {
            textContentShallow(rDescriptionStr, descriptionNode);
         }
      }

   }
   return numberMatched;
}

