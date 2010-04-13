// 
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
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
#include <utl/UtlRegex.h>
#include <utl/UtlTokenizer.h>
#include <net/Url.h>
#include <net/SipMessage.h>
#include <digitmaps/AuthRulesUrlMapping.h>
#include "xmlparser/tinyxml.h"
#include "digitmaps/Patterns.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
AuthRulesUrlMapping::AuthRulesUrlMapping()
{
}

// Destructor
AuthRulesUrlMapping::~AuthRulesUrlMapping()
{
}

/* ============================ MANIPULATORS ============================== */

OsStatus
AuthRulesUrlMapping::loadMappings(const UtlString& configFileName,
                         const UtlString& mediaserver,
                         const UtlString& voicemail,
                         const UtlString& localhost)
{
    return UrlMapping::loadMappings( configFileName, mediaserver, voicemail, localhost );
}

/* ============================ ACCESSORS ================================= */

OsStatus
AuthRulesUrlMapping::getPermissionRequired(const Url& requestUri,
                                           ResultSet& rPermissions) const
{
   OsStatus permissionsSet = OS_FAILED; 
   UtlString variableDigits;
   const TiXmlNode* pMatchingUserMatchContainerNode = 0;
   const TiXmlNode* pMatchingHostMatchContainerNode = 0;
   
   if( getUserMatchContainerMatchingRequestURI(requestUri,
                                               variableDigits,                                                     
                                               pMatchingUserMatchContainerNode,
                                               pMatchingHostMatchContainerNode) == OS_SUCCESS )
   {
      permissionsSet = parsePermMatchContainer( requestUri, 
                                                variableDigits, 
                                                rPermissions, 
                                                pMatchingUserMatchContainerNode );
   }
   return permissionsSet;
}

OsStatus
AuthRulesUrlMapping::parsePermMatchContainer(const Url& requestUri,
                                             const UtlString& vdigits,
                                             ResultSet& rPermissions,
                                             const TiXmlNode* pUserMatchNode ) const
{
    OsStatus permissionsSet = OS_FAILED;

    UtlString requestUriStr;
    requestUri.toString(requestUriStr);

    const TiXmlNode* pPermMatchNode = NULL;
    while ( (pPermMatchNode = pUserMatchNode->IterateChildren( pPermMatchNode ))
            && (permissionsSet != OS_SUCCESS) )
    {
       if(pPermMatchNode && pPermMatchNode->Type() == TiXmlNode::ELEMENT)
       {
          UtlString tagValue = pPermMatchNode->Value();
          if(tagValue.compareTo( XML_TAG_PERMISSIONMATCH ) == 0 )
          {
             //practically there should always be only one permission match tag
             const TiXmlElement* pPermissionMatchElement  = pPermMatchNode->ToElement();
             //get the user text value from it
             for( const TiXmlNode* pPermissionNode = pPermissionMatchElement->FirstChild( XML_TAG_PERMISSION );
                  pPermissionNode;
                  pPermissionNode = pPermissionNode->NextSibling( XML_TAG_PERMISSION ) )
             {
                //get permission Name
                const TiXmlElement* pPermissionElement = pPermissionNode->ToElement();
                const TiXmlNode* pPermissionText = pPermissionElement->FirstChild();
                if(pPermissionText)
                {
                   UtlString permission = pPermissionText->Value();
                   // permissionName.append(permission.data());
                   UtlHashMap record;
                   UtlString* pIdentityKey =
                      new UtlString ( "identity" );
                   UtlString* pPermissionKey =
                      new UtlString ( "permission" );
                   UtlString* pIdentityValue =
                      new UtlString ( requestUriStr );
                   UtlString* pPermissionValue =
                      new UtlString ( permission );
                   record.insertKeyAndValue (
                      pIdentityKey, pIdentityValue );
                   record.insertKeyAndValue (
                      pPermissionKey, pPermissionValue );
                   rPermissions.addValue(record);
                   permissionsSet = OS_SUCCESS;
                }
             }
          }
       }
    }
    return permissionsSet;
}

