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
#ifdef REPLACE_TEST
#include <xmlparser/TiXmlUtlStringWriter.h>
#endif // REPLACE_TEST

#include <os/OsSysLog.h>

#include <utl/UtlString.h>
#include <utl/UtlRegex.h>
#include <utl/UtlTokenizer.h>

#include <net/Url.h>
#include <net/SipMessage.h>

#include <digitmaps/UrlMapping.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
static UtlString cUriKey("uri");
static UtlString cCallidKey("callid");
static UtlString cContactKey("contact");
static UtlString cExpiresKey("expires");
static UtlString cCseqKey("cseq");
static UtlString cQvalueKey("qvalue");


// Constants used in SymbolMap for parsing replacement tokens
const char* XML_ATTRIBUTE_NAME           = "name";
const char* EQUAL_SIGN                   = "=";

const char* SIP_PARAMETER_TRANSPORT      = "transport";

const char* XML_SYMBOL_USER              = "{user}";
const char* XML_SYMBOL_USER_ESCAPED      = "{user-escaped}";
const char* XML_SYMBOL_DIGITS            = "{digits}";
const char* XML_SYMBOL_DIGITS_ESCAPED    = "{digits-escaped}";
const char* XML_SYMBOL_HOST              = "{host}";
const char* XML_SYMBOL_HEADERPARAMS      = "{headerparams}";
const char* XML_SYMBOL_URLPARAMS         = "{urlparams}";
const char* XML_SYMBOL_URI               = "{uri}";
const char* XML_SYMBOL_LOCALHOST         = "{localhost}";
const char* XML_SYMBOL_MEDIASERVER       = "{mediaserver}";
const char* XML_SYMBOL_VOICEMAIL         = "{voicemail}";
const char* XML_SYMBOL_VDIGITS           = "{vdigits}";
const char* XML_SYMBOL_VDIGITS_ESCAPED   = "{vdigits-escaped}";

const RegEx ComponentSymbols(
   "{(?:user(?:-escaped)?|digits(?:-escaped)?|host|(?:header|url)?params)}"
                             );
/*
 * The SymbolMap class encapsulates the replacement of all the magic tokens
 * that may be used in transform value content.
 */
class SymbolMap
{
private:
   UtlString mReplacementMediaserver;
   UtlString mReplacementVoicemail;
   UtlString mReplacementLocalhost;

   Url       mOriginalUrl;

   bool      mComponentsBuilt; ///< user and host
   UtlString mReplacementUser;
   UtlString mReplacementUserEscaped;
   UtlString mReplacementHost;

   bool      mUrlParamsBuilt;
   UtlString mReplacementUrlParams;

   bool      mHeaderParamsBuilt;
   UtlString mReplacementHeaderParams;

   void replaceEach(UtlString& value,
                    const UtlString& replaceWhat,
                    const UtlString& replaceWith)
      {
#        ifdef REPLACE_TEST
         OsSysLog::add(FAC_SIP, PRI_DEBUG, "UrlMapping SymbolMap::replaceEach('%s', '%s', '%s')",
                       value.data(), replaceWhat.data(), replaceWith.data());
#        endif // REPLACE_TEST

         UtlString modifiedString;
         size_t lastIndex;
         ssize_t index;
         for ( index = 0, lastIndex = 0;
               (index = value.index(replaceWhat, lastIndex, UtlString::ignoreCase)) != UTL_NOT_FOUND;
               lastIndex = index+replaceWhat.length()
              )
         {
            modifiedString.append(value, lastIndex, index-lastIndex);
            modifiedString.append(replaceWith);
         }

         if (lastIndex < value.length())
         {
            modifiedString.append(value, lastIndex, UtlString::UTLSTRING_TO_END);
         }

         value = modifiedString;
      }

   void buildComponents()
      {
         mOriginalUrl.getUserId(mReplacementUser);

         mReplacementUserEscaped = mReplacementUser;
         HttpMessage::escape(mReplacementUserEscaped);

         mOriginalUrl.getHostWithPort(mReplacementHost);

         mComponentsBuilt = true;
      }

   void buildUrlParams()
      {
         int iEntries = 0;
         // find out how many parameters there are...
         mOriginalUrl.getUrlParameters(0, NULL, NULL, iEntries) ;
         // construct a string with all url parameters
         if (iEntries > 0)
         {
            UtlString pNames[iEntries];
            UtlString pValues[iEntries];

            mOriginalUrl.getUrlParameters(iEntries, pNames, pValues, iEntries) ;
            for(int i=0; i < iEntries; i++)
            {
               if( i != 0)
               {
                  mReplacementUrlParams.append(";");
               }
               mReplacementUrlParams.append(pNames[i]);
               mReplacementUrlParams.append("=");
               mReplacementUrlParams.append(pValues[i]);
            }
         }
         mUrlParamsBuilt = true;
      }

   void buildHeaderParams()
      {
         int iEntries = 0;
         // find out how many parameters there are...
         mOriginalUrl.getHeaderParameters(0, NULL, NULL, iEntries) ;
         // construct a string with all url parameters
         if (iEntries > 0)
         {
            UtlString pNames[iEntries];
            UtlString pValues[iEntries];

            mOriginalUrl.getHeaderParameters(iEntries, pNames, pValues, iEntries) ;
            for(int i=0; i < iEntries; i++)
            {
               if( i != 0)
               {
                  mReplacementHeaderParams.append(";");
               }
               mReplacementHeaderParams.append(pNames[i]);
               mReplacementHeaderParams.append("=");
               mReplacementHeaderParams.append(pValues[i]);
            }
         }
         mHeaderParamsBuilt = true;
      }

public:
   SymbolMap(const Url& original,
             const UtlString& mediaserver,
             const UtlString& voicemail,
             const UtlString& localhost
             )
      : mReplacementMediaserver(mediaserver)
      , mReplacementVoicemail(voicemail)
      , mReplacementLocalhost(localhost)
      , mOriginalUrl(original)
      , mComponentsBuilt(false)
      , mUrlParamsBuilt(false)
      , mHeaderParamsBuilt(false)
      {
      }

   void replace(UtlString&       value,
                const UtlString& vdigits
                )
      {
#        ifdef REPLACE_TEST
         OsSysLog::add(FAC_SIP, PRI_DEBUG, "UrlMapping SymbolMap::replace('%s', '%s')",
                       value.data(), vdigits.data());
#        endif // REPLACE_TEST

         if (value.contains(XML_SYMBOL_MEDIASERVER))
         {
            replaceEach(value, XML_SYMBOL_MEDIASERVER, mReplacementMediaserver);
         }
         if (value.contains(XML_SYMBOL_VOICEMAIL))
         {
            replaceEach(value, XML_SYMBOL_VOICEMAIL,   mReplacementVoicemail);
         }
         if (value.contains(XML_SYMBOL_LOCALHOST))
         {
            replaceEach(value, XML_SYMBOL_LOCALHOST,   mReplacementLocalhost);
         }
         if (value.contains(XML_SYMBOL_URI))
         {
            UtlString replacementUri;
            mOriginalUrl.toString(replacementUri);
            replaceEach(value, XML_SYMBOL_URI,         replacementUri);
         }

         if (value.contains(XML_SYMBOL_VDIGITS))
         {
            replaceEach(value, XML_SYMBOL_VDIGITS,         vdigits);
         }
         if (value.contains(XML_SYMBOL_VDIGITS_ESCAPED))
         {
            UtlString vdigitsEscaped = vdigits ;
            HttpMessage::escape(vdigitsEscaped);
            replaceEach(value, XML_SYMBOL_VDIGITS_ESCAPED, vdigitsEscaped);
         }

         RegEx componentSymbols(ComponentSymbols);
         if (componentSymbols.Search(value))
         {
            if (!mComponentsBuilt)
            {
               buildComponents();
            }

            if (value.contains(XML_SYMBOL_USER))
            {
               replaceEach(value, XML_SYMBOL_USER,            mReplacementUser);
            }
            if (value.contains(XML_SYMBOL_USER_ESCAPED))
            {
               replaceEach(value, XML_SYMBOL_USER_ESCAPED,    mReplacementUserEscaped);
            }
            if (value.contains(XML_SYMBOL_DIGITS))
            {
               replaceEach(value, XML_SYMBOL_DIGITS,          mReplacementUser);
            }
            if (value.contains(XML_SYMBOL_DIGITS_ESCAPED))
            {
               replaceEach(value, XML_SYMBOL_DIGITS_ESCAPED,  mReplacementUserEscaped);
            }
            if (value.contains(XML_SYMBOL_HOST))
            {
               replaceEach(value, XML_SYMBOL_HOST,            mReplacementHost);
            }
         }

         if (value.contains(XML_SYMBOL_URLPARAMS))
         {
            if (!mUrlParamsBuilt)
            {
               buildUrlParams();
            }
            replaceEach(value, XML_SYMBOL_URLPARAMS,          mReplacementUrlParams);
         }

         if (value.contains(XML_SYMBOL_HEADERPARAMS))
         {
            if (!mHeaderParamsBuilt)
            {
               buildHeaderParams();
            }
            replaceEach(value, XML_SYMBOL_HEADERPARAMS,       mReplacementHeaderParams);
         }
      }
};

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
UrlMapping::UrlMapping() :
    mDoc(NULL),
    mParseFirstTime(false),
    mPatterns(NULL)
{
}

// Destructor
UrlMapping::~UrlMapping()
{
   if (mDoc != NULL)
   {
      delete mDoc ;
   }

   if (mPatterns != NULL)
   {
      delete mPatterns ;
   }
}

/* ============================ MANIPULATORS ============================== */

OsStatus
UrlMapping::loadMappings(const UtlString& configFileName,
                         const UtlString& mediaserver,
                         const UtlString& voicemail,
                         const UtlString& localhost)
{
    OsStatus currentStatus = OS_SUCCESS;

    if (mDoc != NULL)
    {
       delete mDoc ;
    }

    if (mPatterns != NULL)
    {
       delete mPatterns ;
    }
    mPatterns = new Patterns() ;

    mDoc = new TiXmlDocument(configFileName.data());
    if (mDoc->LoadFile())
    {
       OsSysLog::add(FAC_SIP, PRI_INFO, "UrlMapping::loadMappings - "
                     "loaded '%s'", configFileName.data());

       currentStatus = OS_SUCCESS;

       if(!voicemail.isNull())
       {
          mVoicemail.append(voicemail);
       }

       if(!localhost.isNull())
       {
          mLocalhost.append(localhost);
       }

       if(!mediaserver.isNull())
       {
          mMediaServer.append(mediaserver);
       }
    }
    else
    {
       OsSysLog::add( FAC_SIP, PRI_ERR, "UrlMapping::loadMappings - "
                     "failed to load '%s'", configFileName.data() );
       currentStatus = OS_NOT_FOUND;
    }

    return currentStatus;
}


OsStatus
UrlMapping::getUserMatchContainerMatchingRequestURI(const Url&  requestUri,
                                                    UtlString&  variableDigits,
                                                    const TiXmlNode*& prMatchingUserMatchContainerNode,
                                                    const TiXmlNode*& prMatchingHostMatchContainerNode,
                                                    const char* ruleType
                                                    ) const
{
    prMatchingUserMatchContainerNode = 0;
    variableDigits.remove(0);

    UtlString     testHost;
    TiXmlNode*    pPrevMappingNode;
    // Get the "mappings" element.
    // It is a child of the document, and can be selected by name.
    pPrevMappingNode = mDoc->FirstChild( XML_TAG_MAPPINGS);
    if (!pPrevMappingNode)
    {
        OsSysLog::add( FAC_SIP, PRI_ERR, "UrlMapping::getContactList - "
                      "No '%s' node",  XML_TAG_MAPPINGS);
        return OS_FILE_READ_FAILED;
    }
    if (!pPrevMappingNode->ToElement())
    {
        OsSysLog::add( FAC_SIP, PRI_ERR, "UrlMapping::getContactList - "
                      "node '%s' is not an element", XML_TAG_MAPPINGS );
        return OS_INVALID;
    }

    requestUri.getHostAddress(testHost);
    int testPort = requestUri.getHostPort();
    if(testPort == SIP_PORT)
    {
        testPort = 0;
    }

    OsStatus userMatchFound = OS_FAILED;
    TiXmlElement* pMappingElement = pPrevMappingNode->ToElement();

    TiXmlNode* pHostMatchNode = NULL;
    while ( (pHostMatchNode = pMappingElement->IterateChildren(pHostMatchNode))
            && userMatchFound != OS_SUCCESS)
    {
       if(pHostMatchNode && pHostMatchNode->Type() == TiXmlNode::ELEMENT)
       {
          TiXmlElement* pHostMatchElement = pHostMatchNode->ToElement();
          UtlString tagValue =  pHostMatchElement->Value();
          if(tagValue.compareTo(XML_TAG_HOSTMATCH) == 0 )
          {
             //found hostmatch tag

             //if ruleType is constrained, check for match
             if ( ruleType != NULL )
             {
                const TiXmlNode*  typeNode = pHostMatchNode->FirstChild("ruleType");
                if (!typeNode)
                {
                   continue;
                }
                UtlString currentRuleType = typeNode->FirstChild()->Value();
                if ( currentRuleType != ruleType )
                {
                   continue;
                }
             }

             //check for host Match patterns in it
             TiXmlNode* pHostPatternNode = NULL;

             for( pHostPatternNode = pHostMatchElement->FirstChild( XML_TAG_HOSTPATTERN );
                  pHostPatternNode && userMatchFound != OS_SUCCESS;
                  pHostPatternNode = pHostPatternNode->NextSibling( XML_TAG_HOSTPATTERN ) )
             {
                UtlBoolean hostMatchFound = false;

                if(pHostPatternNode && pHostPatternNode->Type() == TiXmlNode::ELEMENT)
                {
                   // found hostPattern tag
                   TiXmlElement* pHostPatternElement = pHostPatternNode->ToElement();
                   //get the host text value from it
                   TiXmlNode* pHostPatternText = pHostPatternElement->FirstChild();
                   if( pHostPatternText && pHostPatternText->Type() == TiXmlNode::TEXT)
                   {
                      TiXmlText* pXmlhost = pHostPatternText->ToText();
                      if (pXmlhost)
                      {
                         UtlString pattern = pXmlhost->Value();

                         // Get the "format" attribute to determine what
                         // type of pattern is to be searched
                         const char * xFormat =
                            pHostPatternElement->Attribute(XML_ATT_FORMAT);

                         UtlString fmt;
                         if (xFormat)
                         {
                            fmt.append(xFormat);
                         }
                         else
                         {
                            // Attribute "format" is missing,
                            // so default to 'url'
                            fmt.append(XML_SYMBOL_URL);
                         }

                         // format='url' matches host and port of a URL
                         if (fmt.compareTo(XML_SYMBOL_URL,
                                           UtlString::ignoreCase) == 0)
                         {
                            Url xmlUrl(pattern.data());
                            UtlString xmlHost;
                            xmlUrl.getHostAddress(xmlHost);
                            int xmlPort = xmlUrl.getHostPort();

                            if( xmlHost.compareTo(testHost, UtlString::ignoreCase) == 0
                                && (xmlPort == SIP_PORT || xmlPort == testPort) )
                            {
                               hostMatchFound = OS_SUCCESS;
                            }
                         }
                         // format='IPv4subnet' matches IP address if it is
                         // within the subnet specified in CIDR format
                         else if (fmt.compareTo(XML_SYMBOL_IPV4SUBNET,
                                                UtlString::ignoreCase) == 0)
                         {
                            if( mPatterns->IPv4subnet(testHost, pattern) == true )
                            {
                               hostMatchFound = OS_SUCCESS;
                            }
                         }
                         // format='DnsWildcard' matches FQDN
                         // if it ends with the correct domain
                         else if (fmt.compareTo(XML_SYMBOL_DNSWILDCARD,
                                                UtlString::ignoreCase) == 0)
                         {
                            if( mPatterns->DnsWildcard(testHost, pattern) == true )
                            {
                               hostMatchFound = OS_SUCCESS;
                            }
                         }

                         if (hostMatchFound)
                         {
                            prMatchingHostMatchContainerNode = pHostMatchNode;
                            userMatchFound = getUserMatchContainer(requestUri,
                                                                   pHostMatchNode,
                                                                   variableDigits,
                                                                   prMatchingUserMatchContainerNode);
                         }
                      }
                   }
                }
             }
          }
       }
    }
    return userMatchFound;
}

OsStatus
UrlMapping::getUserMatchContainer(const Url&             requestUri,
                                  const TiXmlNode* const pHostMatchNode,
                                  UtlString&             variableDigits,
                                  const TiXmlNode*&      prMatchingUserMatchContainerNode ) const
{
    UtlString testUser;
    requestUri.getUserId(testUser);

    OsStatus userMatchFound = OS_FAILED;

    const TiXmlElement* const pHostMatchElement = pHostMatchNode->ToElement();

    const TiXmlNode* pUserMatchNode = NULL;
    while ( (pUserMatchNode = pHostMatchElement->IterateChildren( pUserMatchNode ))
            && (userMatchFound != OS_SUCCESS) )
    {
       if(pUserMatchNode && pUserMatchNode->Type() == TiXmlNode::ELEMENT)
       {
          UtlString tagValue = pUserMatchNode->Value();
          if(tagValue.compareTo(XML_TAG_USERMATCH) == 0 )
          {
             //found userPattern tag
             const TiXmlElement* pUserMatchElement = pUserMatchNode->ToElement();
             const TiXmlNode* pUserPatternNode = NULL;
             //get the user text value from it
             for( pUserPatternNode = pUserMatchElement->FirstChild( XML_TAG_USERPATTERN );
                  pUserPatternNode && (userMatchFound != OS_SUCCESS);
                  pUserPatternNode = pUserPatternNode->NextSibling( XML_TAG_USERPATTERN ) )
             {
                if(pUserPatternNode && pUserPatternNode->Type() == TiXmlNode::ELEMENT)
                {
                   const TiXmlElement* pUserPatternElement = pUserPatternNode->ToElement();

                   const TiXmlNode* pUserPatternText = pUserPatternElement->FirstChild();
                   if(pUserPatternText && pUserPatternText->Type() == TiXmlNode::TEXT)
                   {
                      const TiXmlText* pXmlUser = pUserPatternText->ToText();
                      if (pXmlUser)
                      {
                         UtlString userRE = pXmlUser->Value();
                         UtlString regStr;
                         convertRegularExpression(userRE, regStr);
                         RegEx userExpression(regStr.data());
                         if (userExpression.Search(testUser.data(), testUser.length()))
                         {
                            getVDigits(userExpression, variableDigits);
                            prMatchingUserMatchContainerNode = pUserMatchNode;
                            userMatchFound = OS_SUCCESS;
                         }
                      }
                   }
                }
             }
          }
       }
    }
    return userMatchFound;
}

OsStatus
UrlMapping::doTransform(const Url& requestUri,
                        const UtlString& vdigits,
                        ResultSet& rContacts,
                        const TiXmlNode* permMatchNode) const
{
#  ifdef REPLACE_TEST
   {
      UtlString out;
      TiXmlUtlStringWriter w(&out);
      w << *permMatchNode;

      OsSysLog::add(FAC_SIP, PRI_DEBUG, "UrlMapping::doTransform "
                    "requestUri = '%s', vdigits = '%s', permMatchNode = '%s'",
                    requestUri.toString().data(),
                    vdigits.data(),
                    out.data());
   }
#  endif // REPLACE_TEST

   OsStatus returnStatus = OS_FAILED;  // will remain so unless at least one transform is valid

   // create the mapping for all the magic replacement tokens
   SymbolMap symbols(requestUri, mMediaServer, mVoicemail, mLocalhost);

   // loop over all transform elements - for each valid one, insert a contact into rContacts
   const TiXmlElement* permissionMatchElement  = permMatchNode->ToElement();
   const TiXmlNode* transformNode;
   for (transformNode = permissionMatchElement->FirstChild(XML_TAG_TRANSFORM);
        transformNode;
        transformNode = transformNode->NextSibling(XML_TAG_TRANSFORM)
        )
   {
      if(transformNode->Type() == TiXmlNode::ELEMENT)
      {
         Url transformedUrl(requestUri); // copy to modify

         /*
          * The transformState governs what sub-elements of a transform are valid;
          * order is significant.
          */
         enum {
            NoTransformsApplied,// any subelement is valid - initial state

            // the next 5 define the order for the component modifier elements
            UserTransformed,    // there may be only one user element
            HostTransformed,    // there may be only one host element
            UrlParamAdded,      // there may be multiple urlparams elements
            HeaderParamAdded,   // there may be multiple headerparams elements
            FieldParamAdded,    // there may be multiple fieldparams elements

            // if the url element is used, then none of above are allowed
            FullUrlTransformed, // if used, the url element must be the only transform subnode

            TransformError      // error state - invalid element seen
         } transformState = NoTransformsApplied;

         const TiXmlNode*    transformSubNode = NULL;
         while (   (transformState < TransformError)
                && (transformSubNode = transformNode->IterateChildren(transformSubNode))
                )
         {
            if(transformSubNode->Type() == TiXmlNode::ELEMENT)
            {
               const TiXmlElement* transformSubElement = transformSubNode->ToElement();
               UtlString elementType = transformSubElement->Value();

               if (   (NoTransformsApplied == transformState)
                   && (elementType.compareTo(XML_TAG_URL) ==0))
               {
                  // url element
                  const TiXmlNode* transformText = transformSubElement->FirstChild();
                  if(transformText && transformText->Type() == TiXmlNode::TEXT)
                  {
                     const TiXmlText* XmlUrl = transformText->ToText();
                     if (XmlUrl)
                     {
                        UtlString url = XmlUrl->Value();
                        symbols.replace(url, vdigits);

                        transformedUrl.fromString(url);
                        if (Url::UnknownUrlScheme != transformedUrl.getScheme())
                        {
                           // the transformed url was parsed successfully, so it's probably ok
                           transformState = FullUrlTransformed;
                        }
                        else
                        {
                           transformState = TransformError;
                           OsSysLog::add(FAC_SIP, PRI_ERR, "UrlMapping::doTransform "
                                         "invalid url '%s' from transform url at line %d; "
                                         "transform not used",
                                         url.data(), transformSubNode->Row()
                                         );
                        }
                     }
                  }
                  else
                  {
                     OsSysLog::add(FAC_SIP, PRI_ERR, "UrlMapping::doTransform "
                                   "skipped empty transform url at line %d; ",
                                   transformSubNode->Row()
                                   );
                  }
               }
               else if (   (transformState < UserTransformed)
                        && (elementType.compareTo(XML_TAG_USER)==0))
               {
                  // user element
                  const TiXmlNode* transformText = transformSubElement->FirstChild();
                  if(transformText && transformText->Type() == TiXmlNode::TEXT)
                  {
                     const TiXmlText* XmlUser = transformText->ToText();
                     if (XmlUser)
                     {
                        UtlString transformUser = XmlUser->Value();
                        symbols.replace(transformUser, vdigits);
                        transformedUrl.setUserId(transformUser);
                        transformState = UserTransformed;
                     }
                  }
               }
               else if (   (transformState < HostTransformed)
                        && (elementType.compareTo(XML_TAG_HOST)==0))
               {
                  // host element
                  const TiXmlNode* transformText = transformSubElement->FirstChild();
                  if(transformText && transformText->Type() == TiXmlNode::TEXT)
                  {
                     const TiXmlText* XmlHost = transformText->ToText();
                     if (XmlHost)
                     {
                        UtlString transformHost = XmlHost->Value();
                        symbols.replace(transformHost, vdigits);

                        UtlString justHost;
                        Url parsedHostPort(transformHost);
                        parsedHostPort.getHostAddress(justHost);

                        transformedUrl.setHostAddress(justHost);
                        transformedUrl.setHostPort(parsedHostPort.getHostPort());

                        // We have changed the domain; any transport restriction in the
                        // original uri may now not match the capabilities of the new domain,
                        // so remove it.  The urlParams attribute can put in a new one if needed.
                        transformedUrl.removeUrlParameter(SIP_PARAMETER_TRANSPORT);

                        transformState = HostTransformed;
                     }
                     else
                     {
                        OsSysLog::add(FAC_SIP, PRI_ERR, "UrlMapping::doTransform "
                                      "skipped empty host transform at line %d; "
                                      "host is required in a SIP url",
                                      transformSubNode->Row()
                                      );
                     }
                  }
               }
               else if (   (transformState <= UrlParamAdded)
                        && (elementType.compareTo(XML_TAG_URLPARAMS)==0))
               {
                  // Url parameters Tag
                  UtlString name;
                  UtlString value;
                  if(getNamedAttribute(transformSubElement, name, value))
                  {
                     symbols.replace(value, vdigits);
                     transformedUrl.setUrlParameter(name, value);
                     transformState = UrlParamAdded;
                  }
                  else
                  {
                     OsSysLog::add(FAC_SIP, PRI_ERR, "UrlMapping::doTransform "
                                   "invalid urlparams transform at line %d; "
                                   "transform skipped",
                                   transformSubNode->Row()
                                   );
                     transformState = TransformError;
                  }
               }
               else if (   (transformState <= HeaderParamAdded)
                        && (elementType.compareTo(XML_TAG_HEADERPARAMS)==0))
               {
                  // Header parameters Tag
                  UtlString name;
                  UtlString value;
                  if(getNamedAttribute(transformSubElement, name, value))
                  {
                     symbols.replace(value, vdigits);
                     transformedUrl.setHeaderParameter(name, value);
                     transformState = HeaderParamAdded;
                  }
                  else
                  {
                     OsSysLog::add(FAC_SIP, PRI_ERR, "UrlMapping::doTransform "
                                   "invalid headerparams transform at line %d; "
                                   "transform skipped",
                                   transformSubNode->Row()
                                   );
                     transformState = TransformError;
                  }
               }
               else if (   (transformState <= FieldParamAdded)
                        && (elementType.compareTo(XML_TAG_FIELDPARAMS)==0))
               {
                  // Field parameters Tag
                  UtlString name;
                  UtlString value;
                  if(getNamedAttribute(transformSubElement, name, value))
                  {
                     symbols.replace(value, vdigits);
                     transformedUrl.setFieldParameter(name, value);
                     transformState = FieldParamAdded;
                  }
                  else
                  {
                     OsSysLog::add(FAC_SIP, PRI_ERR, "UrlMapping::doTransform "
                                   "invalid fieldparams transform at line %d; "
                                   "transform skipped",
                                   transformSubNode->Row()
                                   );
                     transformState = TransformError;
                  }
               }
               else
               {
                  OsSysLog::add(FAC_SIP, PRI_ERR,
                                "UrlMapping::doTransform element '%s' is invalid at line %d",
                                elementType.data(), transformSubNode->Row()
                                );
                  transformState = TransformError;
               }
            }
            else
            {
               // non-element transformSubNode - ignore it
            }
         } // end of loop over transform sub-elements

         if (TransformError != transformState)
         {
            UtlHashMap contactRow;
            UtlString* uriValue    = new UtlString ( requestUri.toString() );
            UtlString* callidValue = new UtlString ( " " );

            UtlString* contactValue = new UtlString;
            transformedUrl.toString(*contactValue);

            OsSysLog::add(FAC_SIP, PRI_DEBUG, "UrlMapping::doTransform "
                          "adding '%s'", contactValue->data());

            UtlInt*    expiresValue = new UtlInt ( 0 );
            UtlInt*    cseqValue    = new UtlInt ( 0 );
            UtlString* qvalueValue = new UtlString ( "1.0" );

            // Make shallow copies of static keys
            UtlString* uriKey     = new UtlString( cUriKey );
            UtlString* callidKey  = new UtlString( cCallidKey );
            UtlString* contactKey = new UtlString( cContactKey );
            UtlString* expiresKey = new UtlString( cExpiresKey );
            UtlString* cseqKey    = new UtlString( cCseqKey );
            UtlString* qvalueKey  = new UtlString( cQvalueKey );

            contactRow.insertKeyAndValue (uriKey, uriValue);
            contactRow.insertKeyAndValue (callidKey, callidValue);
            contactRow.insertKeyAndValue (contactKey, contactValue);
            contactRow.insertKeyAndValue (expiresKey, expiresValue);
            contactRow.insertKeyAndValue (cseqKey, cseqValue);
            contactRow.insertKeyAndValue (qvalueKey, qvalueValue);

            rContacts.addValue( contactRow );

            // we found at least one valid transform, so call this good
            returnStatus = OS_SUCCESS;
         }
      }
      else
      {
         // some child other than transform - should not happen, but ignore
         OsSysLog::add(FAC_SIP, PRI_WARNING,
                       "UrlMapping::doTransform unrecognized node when fetching transform"
                       );
      }
   } // end of loop over transform elements

   return returnStatus;
}

void
UrlMapping::convertRegularExpression(const UtlString& source,
                                     UtlString& rRegExp)
{
    const char* sourceChar;
    UtlBoolean specialEscaped;
    UtlBoolean seenNonConstant = false;

    rRegExp.remove(0);
    rRegExp.append("^");

    for(sourceChar = source.data();
        *sourceChar;
        sourceChar++
        )
    {
        specialEscaped = false;

        if(*sourceChar == '\\')
        {
            specialEscaped = true;
            sourceChar++ ;
        }

        if(*sourceChar == '[')
        {
            if ( specialEscaped )
            {
                rRegExp.append("\\");
            }
            else if ( ! seenNonConstant )
            {
                rRegExp.append("(");
                seenNonConstant = true;
            }
            rRegExp.append(*sourceChar);
        }
        else if(*sourceChar == '.')
        {
            if ( specialEscaped )
            {
                rRegExp.append("\\.");
            }
            else
            {
                if ( ! seenNonConstant )
                {
                    rRegExp.append("(");
                    seenNonConstant = true;
                }
                rRegExp.append(".*");
            }
        }
        else if(*sourceChar == '+')
        {
           if ( specialEscaped )
           {
              rRegExp.append("\\");
           }
           rRegExp.append("\\+");
        }
        else if(*sourceChar == '$')
        {
           if ( specialEscaped )
           {
              rRegExp.append("\\");
           }
           rRegExp.append("\\$");
        }
        else if(*sourceChar == '?')
        {
           if ( specialEscaped )
           {
              rRegExp.append("\\");
           }
           rRegExp.append("\\?");
        }
        else if(*sourceChar == '*')
        {
           if ( specialEscaped )
           {
              rRegExp.append("\\");
           }
           rRegExp.append("\\*");
        }
        else if(*sourceChar == '(')
        {
           if ( specialEscaped )
           {
              rRegExp.append("\\");
           }
           rRegExp.append("\\(");
        }
        else if(*sourceChar == ')')
        {
           if ( specialEscaped )
           {
              rRegExp.append("\\");
           }
           rRegExp.append("\\)");
        }
        else if(*sourceChar == 'x')
        {
            if ( specialEscaped )
            {
                rRegExp.append("x");
            }
            else
            {
                if ( ! seenNonConstant )
                {
                    rRegExp.append("(");
                    seenNonConstant = true;
                }
                rRegExp.append('.');
            }
        }
        else if (*sourceChar == '\\')
        {
            rRegExp.append("\\\\");
        }
        else
        {
           rRegExp.append(*sourceChar);
        }
    }

    if ( seenNonConstant )
    {
       rRegExp.append(")");
    }
    rRegExp.append("$");
}

void UrlMapping::getVDigits(
    RegEx& userPattern,
    UtlString& vdigits) const
{
    vdigits.remove(0);
    if ( userPattern.SubStrings() > 1 )
    {
        vdigits.append( userPattern.Match( 1 ) );
    }
}


bool UrlMapping::getNamedAttribute(const TiXmlElement* component,
                                   UtlString&    name,
                                   UtlString&    value
                                   ) const
{
   bool foundNameValue = false;

   name.remove(0);
   value.remove(0);

   UtlString componentContent;   // get the content of the element into this
   const TiXmlNode* componentTextNode = component->FirstChild();
   if(componentTextNode && componentTextNode->Type() == TiXmlNode::TEXT)
   {
      const TiXmlText* componentText = componentTextNode->ToText();
      if (componentText)
      {
         componentContent = componentText->Value();
      }
   }

   // figure out whether this is the old or new syntax
   const char* nameAttrValue;
   if ((nameAttrValue = component->Attribute(XML_ATTRIBUTE_NAME)))
   {
      // this is the new syntax: <foo name='bar'>value</foo>
      name.append(nameAttrValue);
      value.append(componentContent);
      foundNameValue = true;
   }
   else
   {
      // this is the old syntax: <foo>bar=value</foo>
      ssize_t equalsOffset = componentContent.index(EQUAL_SIGN);
      if (UTL_NOT_FOUND != equalsOffset)
      {
         name.append(componentContent,0,equalsOffset);
         value.append(componentContent,equalsOffset+1,UtlString::UTLSTRING_TO_END);
         foundNameValue = true;
      }
      else
      {
         // assume that the whole thing is a name (an attribute with no value)
         name.append(componentContent);
         foundNameValue = true;
      }
   }

   return foundNameValue;
}
