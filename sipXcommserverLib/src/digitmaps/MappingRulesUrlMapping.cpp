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
#include <digitmaps/MappingRulesUrlMapping.h>
#include "xmlparser/tinyxml.h"
#include "xmlparser/ExtractContent.h"
#include "digitmaps/Patterns.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MappingRulesUrlMapping::MappingRulesUrlMapping()
{
}

// Destructor
MappingRulesUrlMapping::~MappingRulesUrlMapping()
{
}

/* ============================ MANIPULATORS ============================== */

OsStatus
MappingRulesUrlMapping::loadMappings(const UtlString& configFileName,
                                     const UtlString& mediaserver,
                                     const UtlString& voicemail,
                                     const UtlString& localhost
                                     )
{
    return UrlMapping::loadMappings( configFileName, mediaserver, voicemail, localhost );
}

/* ============================ ACCESSORS ================================= */

OsStatus
MappingRulesUrlMapping::getContactList(const Url& requestUri,   
                                       ResultSet& rContacts,    
                                       ResultSet& rPermissions,
                                       UtlString& callTag
                                       ) const
{
   OsStatus contactsSet = OS_FAILED; 
   UtlString variableDigits;
   const TiXmlNode* pMatchingUserMatchContainerNode = 0;
   const TiXmlNode* pMatchingHostMatchContainerNode = 0;
   
   if( getUserMatchContainerMatchingRequestURI(requestUri,
                                               variableDigits,                                                     
                                               pMatchingUserMatchContainerNode,
                                               pMatchingHostMatchContainerNode ) == OS_SUCCESS )
   {
      contactsSet = parsePermMatchContainer( requestUri, 
                                             variableDigits, 
                                             rContacts,
                                             rPermissions, 
                                             callTag,
                                             pMatchingUserMatchContainerNode );
   }
   return contactsSet;
}

OsStatus
MappingRulesUrlMapping::parsePermMatchContainer(const Url& requestUri,
                                                const UtlString& vdigits,
                                                ResultSet& rContactResultSet,
                                                ResultSet& rPermissions,
                                                UtlString& callTag,
                                                const TiXmlNode* pUserMatchNode   //parent node
                                                ) const
{
    OsStatus doTransformStatus = OS_FAILED;

    UtlBoolean bPermissionFound = false;

    UtlString requestUriStr;
    callTag = "UNK";
    requestUri.toString(requestUriStr);

    const TiXmlNode* pPermMatchNode = NULL;
    while ( (pPermMatchNode = pUserMatchNode->IterateChildren( pPermMatchNode ) )
            && (doTransformStatus != OS_SUCCESS) )
    {
       if(pPermMatchNode && pPermMatchNode->Type() == TiXmlNode::ELEMENT)
       {
          UtlString tagValue = pPermMatchNode->Value();

          if( tagValue.compareTo(XML_TAG_CALLTAG) == 0 )
          {
            // Found call tag element.  Read the text value for it.
            textContentShallow(callTag, pPermMatchNode);
          }
          if( tagValue.compareTo(XML_TAG_PERMISSIONMATCH) == 0 )
          {
             //practically there should always be only one permission match tag
             const TiXmlElement* pPermissionMatchElement = pPermMatchNode->ToElement();
             UtlBoolean bPermNodePresent = false;
             //get the user text value from it
             for( const TiXmlNode*  pPermissionNode = pPermissionMatchElement->FirstChild( XML_TAG_PERMISSION );
                  pPermissionNode;
                  pPermissionNode = pPermissionNode->NextSibling( XML_TAG_PERMISSION ) )
             {
                bPermNodePresent = true;
                const TiXmlElement* pPermissionElement = pPermissionNode->ToElement();
                //get permission Name
                const TiXmlNode* pPermissionText = pPermissionElement->FirstChild();
                if(pPermissionText)
                {
                   UtlString permission = pPermissionText->Value();
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
                   bPermissionFound = true;
                }
             }

             //if no permission node - then it means no permission required - allow all
             if((!bPermNodePresent || bPermissionFound ) )
             {
                //if the premission matches in the permissions database
                //go ahead and get the transform tag
                doTransformStatus = doTransform(requestUri,
                                                vdigits,
                                                rContactResultSet,
                                                pPermMatchNode);
             }
          }
       }
    }
    return doTransformStatus;
}

