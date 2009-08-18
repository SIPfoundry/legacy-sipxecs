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
#include <digitmaps/FallbackRulesUrlMapping.h>
#include "xmlparser/tinyxml.h"
#include "xmlparser/ExtractContent.h"
#include "digitmaps/Patterns.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
FallbackRulesUrlMapping::FallbackRulesUrlMapping()
{
}

// Destructor
FallbackRulesUrlMapping::~FallbackRulesUrlMapping()
{
}

/* ============================ MANIPULATORS ============================== */

OsStatus
FallbackRulesUrlMapping::loadMappings(const UtlString& configFileName )
{
    return UrlMapping::loadMappings( configFileName );
}

/* ============================ ACCESSORS ================================= */

OsStatus
FallbackRulesUrlMapping::getContactList(const Url& requestUri,   
                                        const UtlString& location,
                                        ResultSet& rContacts,  
                                        UtlString& callTag
                                       ) const
{
   OsStatus contactsSet = OS_FAILED;
   bool bLocationMatchFound = false;
   UtlString variableDigits;
   const TiXmlNode* pMatchingUserMatchContainerNode = 0;
   const TiXmlNode* pMatchingHostMatchContainerNode = 0;
   
   callTag = "UNK";
   if( getUserMatchContainerMatchingRequestURI(requestUri,
                                               variableDigits,                                                     
                                               pMatchingUserMatchContainerNode,
                                               pMatchingHostMatchContainerNode ) == OS_SUCCESS )
   {
      // Try to locate a callerLocationMatch section that matches the supplied location.
      const TiXmlElement* pMappingElement = pMatchingUserMatchContainerNode->ToElement();

      const TiXmlNode* pCallerLocationMatchNode = NULL;
      while ( (pCallerLocationMatchNode = pMappingElement->IterateChildren(pCallerLocationMatchNode))
              && !bLocationMatchFound )
      {
         if(pCallerLocationMatchNode && pCallerLocationMatchNode->Type() == TiXmlNode::ELEMENT)
         {
            const TiXmlElement* pCallerLocationMatchElement = pCallerLocationMatchNode->ToElement();
            UtlString tagValue =  pCallerLocationMatchElement->Value();

            if( tagValue.compareTo(XML_TAG_CALLTAG) == 0 )
            {
               // Found call tag element.  Read the text value for it.
               textContentShallow(callTag, pCallerLocationMatchNode);
            }

            if(tagValue.compareTo(XML_TAG_CALLERLOCATIONMATCH) == 0 )
            {
               //found callerLocationMatch tag
               //check for a location matching the supplied one
               const TiXmlNode* pLocationPatternNode = NULL;

               for( pLocationPatternNode = pCallerLocationMatchElement->FirstChild( XML_TAG_CALLERLOCATION );
                    pLocationPatternNode && !bLocationMatchFound;
                    pLocationPatternNode = pLocationPatternNode->NextSibling( XML_TAG_CALLERLOCATION ) )
               {
                  if(pLocationPatternNode && pLocationPatternNode->Type() == TiXmlNode::ELEMENT)
                  {
                     // found "callerLocation" tag
                     const TiXmlElement* pLocationPatternElement = pLocationPatternNode->ToElement();
                     //get the callerLocation text value from it
                     const TiXmlNode* pLocationText = pLocationPatternElement->FirstChild();
                     if( pLocationText && pLocationText->Type() == TiXmlNode::TEXT)
                     {
                        const TiXmlText* pXmlLocation = pLocationText->ToText();
                        if (pXmlLocation)
                        {
                           UtlString locationName = pXmlLocation->Value();
                           if( locationName == location )
                           {
                              bLocationMatchFound = true;
                              // we have found the location we are looking for, extract the transforms
                              // and add them to the result set.
                              contactsSet = doTransform(requestUri,
                                                        variableDigits,
                                                        rContacts,
                                                        pCallerLocationMatchNode);
                           }
                        }
                     }
                  }
               } 
            }
         }
      }
      
      // we finished iterating through all the callerLocationMatch element, let's check if we
      // found an entry for the location we were looking for.
      if( !bLocationMatchFound )
      {
         // we have not found the location we were looking for then use the default transforms 
         // that are located under the 'userMatch' element.
         contactsSet = doTransform(requestUri,
                                   variableDigits,
                                   rContacts,
                                   pMatchingUserMatchContainerNode);
      }
   }
   return contactsSet;
}
