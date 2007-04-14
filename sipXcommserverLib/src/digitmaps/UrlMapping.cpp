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
#include <utl/UtlRegex.h>
#include <utl/UtlTokenizer.h>
#include <os/OsSysLog.h>
#include <net/Url.h>
#include <net/SipMessage.h>
#include <digitmaps/UrlMapping.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
static UtlString cUriKey("uri");
static UtlString cCallidKey("callid");
static UtlString cContactKey("contact");
static UtlString cExpiresKey("expires");
static UtlString cCseqKey("cseq");
static UtlString cQvalueKey("qvalue");

// CONSTANTS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
UrlMapping::UrlMapping() :
    mPrevMappingNode(NULL),
    mPrevMappingElement(NULL),
    mPrevHostMatchNode(NULL),
    mPrevUserMatchNode(NULL),
    mPrevPermMatchNode(NULL),
    mParseFirstTime(false),
    mDoc(NULL)
{
}

// Destructor
UrlMapping::~UrlMapping()
{
   if (mDoc != NULL)
   {
      delete mDoc ;
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
UrlMapping::loadMappingsString(const UtlString& contents,
                               const UtlString& mediaserver,
                               const UtlString& voicemail,
                               const UtlString& localhost)
{
    OsStatus currentStatus = OS_SUCCESS;

    if (mDoc != NULL)
    {
       delete mDoc ;
    }

    mDoc = new TiXmlDocument();
    if (mDoc->Parse(contents.data()))
    {
       OsSysLog::add(FAC_SIP, PRI_INFO, "UrlMapping::loadMappingsString - "
                     "loaded");

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
       OsSysLog::add( FAC_SIP, PRI_ERR, "UrlMapping::loadMappingsString - "
                     "failed to load" );
       currentStatus = OS_NOT_FOUND;
    }

    return currentStatus;
}

/* ============================ ACCESSORS ================================= */

OsStatus
UrlMapping::getPermissionRequired(const Url& requestUri,
                                  ResultSet& rPermissions)
{
    OsStatus currentStatus = OS_FAILED;

    // Get the "mappings" element.
    // It is a child of the document, and can be selected by name.
    mPrevMappingNode = mDoc->FirstChild( XML_TAG_MAPPINGS);
    if (!mPrevMappingNode)
    {
        OsSysLog::add( FAC_SIP, PRI_ERR, "UrlMapping::getPermissionRequired - "
                       "No " XML_TAG_MAPPINGS " node");
        return OS_FILE_READ_FAILED;
    }

    mPrevMappingElement = mPrevMappingNode->ToElement();
    if(!mPrevMappingElement)
    {
        OsSysLog::add( FAC_SIP, PRI_ERR, "UrlMapping::getPermissionRequired - "
                      "No child Node for Mappings");
        return OS_INVALID;
    }

    UtlBoolean doTransform = false;

    ResultSet registrations;

    currentStatus = parseHostMatchContainer(requestUri,
                                            registrations,
                                            doTransform,
                                            rPermissions,
                                            mPrevMappingNode);

    return currentStatus;
}

OsStatus
UrlMapping::getContactList(const Url& requestUri,
                           ResultSet& rRegistrations,
                           ResultSet& rPermissions )
{
    OsStatus currentStatus = OS_FAILED;

    // Get the "mappings" element.
    // It is a child of the document, and can be selected by name.
    mPrevMappingNode = mDoc->FirstChild( XML_TAG_MAPPINGS);
    if (!mPrevMappingNode)
    {
        OsSysLog::add( FAC_SIP, PRI_ERR, "UrlMapping::getContactList - "
                      "No " XML_TAG_MAPPINGS " node");
        return OS_FILE_READ_FAILED;
    }
    mPrevMappingElement = mPrevMappingNode->ToElement();
    if(!mPrevMappingElement)
    {
        OsSysLog::add( FAC_SIP, PRI_ERR, "UrlMapping::getContactList - "
                      "No child Node for Mappings");
        return OS_INVALID;
    }

    UtlBoolean doTransform = true;

    currentStatus = parseHostMatchContainer(requestUri,
                                            rRegistrations,
                                            doTransform,
                                            rPermissions,
                                            mPrevMappingNode);

    return currentStatus;
}

OsStatus
UrlMapping::parseHostMatchContainer(const Url& requestUri,
                                    ResultSet& rRegistrations,
                                    UtlBoolean& doTransform,
                                    ResultSet& rPermissions,
                                    TiXmlNode* mappingsNode,
                                    TiXmlNode* previousHostMatchNode)
{
    UtlString testHost;
    requestUri.getHostAddress(testHost);
    int testPort = requestUri.getHostPort();
    if(testPort == SIP_PORT)
    {
        testPort = 0;
    }

    UtlBoolean hostMatchFound = false;
    OsStatus userMatchFound = OS_FAILED;

    OsStatus currentStatus = OS_FAILED;
    TiXmlElement* mappingElement = mappingsNode->ToElement();

    TiXmlNode* hostMatchNode = previousHostMatchNode;
    while ( (hostMatchNode = mappingElement->IterateChildren(hostMatchNode))
            && userMatchFound != OS_SUCCESS)
    {
       if(hostMatchNode && hostMatchNode->Type() == TiXmlNode::ELEMENT)
       {
          TiXmlElement* hostMatchElement = hostMatchNode->ToElement();
          UtlString tagValue =  hostMatchElement->Value();
          if(tagValue.compareTo(XML_TAG_HOSTMATCH) == 0 )
          {
             //found hostmatch tag
             //check for host Match patterns in it
             TiXmlNode* hostPatternNode = NULL;

             for( hostPatternNode = hostMatchElement->FirstChild( XML_TAG_HOSTPATTERN);
                  hostPatternNode && userMatchFound != OS_SUCCESS;
                  hostPatternNode = hostPatternNode->NextSibling( XML_TAG_HOSTPATTERN ) )
             {
                if(hostPatternNode && hostPatternNode->Type() == TiXmlNode::ELEMENT)
                {
                   // found hostPattern tag
                   TiXmlElement* hostPatternElement = hostPatternNode->ToElement();
                   //get the host text value from it
                   TiXmlNode* hostPatternText = hostPatternElement->FirstChild();
                   if( hostPatternText && hostPatternText->Type() == TiXmlNode::TEXT)
                   {
                      TiXmlText* Xmlhost = hostPatternText->ToText();
                      if (Xmlhost)
                      {
                         UtlString host = Xmlhost->Value();
                         Url xmlUrl(host.data());
                         UtlString xmlHost;
                         xmlUrl.getHostAddress(xmlHost);
                         int xmlPort = xmlUrl.getHostPort();

                         if(   (xmlHost.compareTo(testHost, UtlString::ignoreCase) == 0)
                            && (xmlPort == SIP_PORT || xmlPort == testPort) )
                         {
                            hostMatchFound = true;
                            mPrevHostMatchNode = hostMatchNode;
                            userMatchFound = parseUserMatchContainer(requestUri,
                                                                     rRegistrations,
                                                                     doTransform,
                                                                     rPermissions,
                                                                     hostMatchNode);
                         }
                      }
                   }
                }
             } 
          }
       }
    }
    return currentStatus;
}

OsStatus
UrlMapping::parseUserMatchContainer(const Url& requestUri,
                                    ResultSet& rRegistrations,
                                    UtlBoolean& doTransform,
                                    ResultSet& rPermissions,
                                    TiXmlNode* hostMatchNode, //parent node
                                    TiXmlNode* previousUserMatchNode)
{

    UtlString testUser;
    requestUri.getUserId(testUser);

    OsStatus permssionMatchFound = OS_FAILED;
    UtlBoolean userMatchFound = false;

    TiXmlNode* userMatchNode = previousUserMatchNode;
    TiXmlElement* hostMatchElement = hostMatchNode->ToElement();

    while ( (userMatchNode = hostMatchElement->IterateChildren( userMatchNode))
            && (permssionMatchFound != OS_SUCCESS) )
    {
       if(userMatchNode && userMatchNode->Type() == TiXmlNode::ELEMENT)
       {
          UtlString tagValue = userMatchNode->Value();
          if(tagValue.compareTo(XML_TAG_USERMATCH) == 0 )
          {
             //found userPattern tag
             TiXmlElement* userMatchElement = userMatchNode->ToElement();
             TiXmlNode* userPatternNode = NULL;
             //get the user text value from it
             for( userPatternNode = userMatchElement->FirstChild( XML_TAG_USERPATTERN);
                  userPatternNode && permssionMatchFound != OS_SUCCESS;
                  userPatternNode = userPatternNode->NextSibling(XML_TAG_USERPATTERN ) )
             {
                if(userPatternNode && userPatternNode->Type() == TiXmlNode::ELEMENT)
                {
                   TiXmlElement* userPatternElement = userPatternNode->ToElement();

                   TiXmlNode* userPatternText = userPatternElement->FirstChild();
                   if(userPatternText && userPatternText->Type() == TiXmlNode::TEXT)
                   {
                      TiXmlText* XmlUser = userPatternText->ToText();
                      if (XmlUser)
                      {
                         UtlString userRE = XmlUser->Value();
                         UtlString regStr;
                         convertRegularExpression(userRE, regStr);
                         RegEx userExpression(regStr.data());
                         if (userExpression.Search(testUser.data(), testUser.length()))
                         {
                            UtlString vdigits;
                            getVDigits(userExpression, vdigits);
                            userMatchFound = true;
                            permssionMatchFound = parsePermMatchContainer(requestUri,
                                                                          vdigits,
                                                                          rRegistrations,
                                                                          doTransform,
                                                                          rPermissions,
                                                                          userMatchNode);
                         }
                      }
                   }
                }
             }
          }
       }
    }
    return permssionMatchFound;
}

OsStatus
UrlMapping::parsePermMatchContainer(const Url& requestUri,
                                    const UtlString& vdigits,
                                    ResultSet& rRegistrationResultSet,
                                    UtlBoolean& doTransformTag,
                                    ResultSet& rPermissions,
                                    TiXmlNode* userMatchNode,   //parent node
                                    TiXmlNode* previousPermMatchNode)
{
    OsStatus doTransformStatus = OS_FAILED;

    UtlBoolean permissionFound = false;
    UtlString permissionAuthType;

    UtlString requestUriStr;
    requestUri.toString(requestUriStr);

    TiXmlNode* permMatchNode = previousPermMatchNode;
    userMatchNode->ToElement();

    while ( (permMatchNode = userMatchNode->IterateChildren( permMatchNode))
            && (doTransformStatus != OS_SUCCESS) )
    {
       if(permMatchNode && permMatchNode->Type() == TiXmlNode::ELEMENT)
       {
          UtlString tagValue = permMatchNode->Value();
          if(tagValue.compareTo(XML_TAG_PERMISSIONMATCH) == 0 )
          {
             //practically there should always be only one permission match tag
             TiXmlElement* permissionMatchElement  = permMatchNode->ToElement();
             UtlBoolean permNodePresent = false;
             //get the user text value from it
             for( TiXmlNode*  permissionNode = permissionMatchElement->FirstChild( XML_TAG_PERMISSION);
                  permissionNode;
                  permissionNode = permissionNode->NextSibling(XML_TAG_PERMISSION ) )
             {
                permNodePresent = true;
                TiXmlElement* permissionElement = permissionNode->ToElement();
                //get attribules of permission
                UtlString* authType = (UtlString*) permissionElement->Attribute(XML_ATT_AUTHTYPE);
                // If the permission auth type is provided
                if(authType)
                {
                   permissionAuthType.append(authType->data());
                }

                //get permission Name
                TiXmlNode* permissionText = permissionElement->FirstChild();
                if(permissionText)
                {
                   UtlString permission = permissionText->Value();
                   // permissionName.append(permission.data());
                   UtlHashMap record;
                   UtlString* identityKey =
                      new UtlString ( "identity" );
                   UtlString* permissionKey =
                      new UtlString ( "permission" );
                   UtlString* identityValue =
                      new UtlString ( requestUriStr );
                   UtlString* permissionValue =
                      new UtlString ( permission );
                   record.insertKeyAndValue (
                      identityKey, identityValue );
                   record.insertKeyAndValue (
                      permissionKey, permissionValue );
                   rPermissions.addValue(record);
                   permissionFound = true;
                }
             }

             //if no permission node - then it means no permission required - allow all
             if((!permNodePresent || permissionFound ) && doTransformTag )
             {
                //if the premission matches in the permissions database
                //go ahead and get the transform tag
                doTransformStatus = doTransform(requestUri,
                                                vdigits,
                                                rRegistrationResultSet,
                                                permMatchNode);
             }
             else if(!permNodePresent || permissionFound)
             {
                doTransformStatus = OS_SUCCESS;
             }
          }
       }
    }
    return doTransformStatus;
}

OsStatus
UrlMapping::doTransform(const Url& requestUri,
                        const UtlString& vdigits,
                        ResultSet& rRegistrations,
                        TiXmlNode* permMatchNode)
{
    OsStatus currentStatus = OS_FAILED;
    TiXmlElement* permissionMatchElement  = permMatchNode->ToElement();
    TiXmlNode* transformNode = NULL;

    UtlString requestUriStr;
    UtlString tempContact;
    TiXmlNode* urlText = NULL;
    requestUri.toString(requestUriStr);
    //get the user text value from it
    for( transformNode = permissionMatchElement->FirstChild( XML_TAG_TRANSFORM);
         transformNode;
         transformNode = transformNode->NextSibling(XML_TAG_TRANSFORM ) )
    {
       currentStatus = OS_SUCCESS;
       if(transformNode && transformNode->Type() == TiXmlNode::ELEMENT)
       {
          TiXmlElement* transformElement = transformNode->ToElement();
          UtlBoolean urlTagFound = false;
          // URL Tag
          urlText = transformElement->FirstChild(XML_TAG_URL);
          if(urlText)
          {
             TiXmlNode* transformText = urlText->FirstChild();
             if(transformText && transformText->Type() == TiXmlNode::TEXT)
             {
                TiXmlText* XmlUrl = transformText->ToText();
                if (XmlUrl)
                {
                   UtlString url = XmlUrl->Value();
                   UtlString modfiedUrl;
                   replaceSymbols( url.data(), requestUri, vdigits, modfiedUrl );
                   tempContact.append(modfiedUrl);
                }
             }
             urlTagFound = true;
          }

          if(!urlTagFound)
          {
             Url tempContactUrl(requestUriStr);
             UtlString transformHost;
             UtlString transformUser;
             //host Tag
             urlText = transformElement->FirstChild(XML_TAG_HOST);
             if(urlText)
             {
                TiXmlNode* transformText = urlText->FirstChild();
                if(transformText && transformText->Type() == TiXmlNode::TEXT)
                {
                   TiXmlText* Xmlhost = transformText->ToText();
                   if (Xmlhost)
                   {
                      UtlString host = Xmlhost->Value();

                      UtlString modfiedHost;
                      replaceSymbols(host.data(), requestUri,vdigits, modfiedHost);

                      UtlString tempHost;
                      Url temp(modfiedHost);
                      temp.getHostAddress(tempHost);
                      int tempPort = temp.getHostPort();

                      tempContactUrl.setHostAddress(tempHost);
                      if( tempPort != 0)
                      {
                         tempContactUrl.setHostPort(tempPort);
                      }
                      // We have changed the domain; any transport restriction in the 
                      // original uri may now not match the capabilities of the new domain,
                      // so remove it.  The urlParams attribute can put in a new one if needed.
                      tempContactUrl.removeUrlParameter("transport");
                   }
                }
             }
             // User tag
             urlText = transformElement->FirstChild(XML_TAG_USER);
             if(urlText)
             {
                TiXmlNode* transformText = urlText->FirstChild();
                if(transformText && transformText->Type() == TiXmlNode::TEXT)
                {
                   TiXmlText* XmlUser = transformText->ToText();
                   if (XmlUser)
                   {
                      UtlString user = XmlUser->Value();
                      UtlString modfiedUserId;
                      replaceSymbols(user.data(), requestUri, vdigits, modfiedUserId);
                      tempContactUrl.setUserId(modfiedUserId);
                   }
                }
             }
             // Field parameters Tag
             UtlString name;
             UtlString value;
             urlText = transformElement->FirstChild(XML_TAG_FIELDPARAMS);
             if(urlText)
             {
                TiXmlNode* transformText = urlText->FirstChild();
                if(transformText && transformText->Type() == TiXmlNode::TEXT)
                {
                   TiXmlText* Xmlfieldparams = transformText->ToText();
                   if (Xmlfieldparams)
                   {
                      UtlString fieldparams = Xmlfieldparams->Value();
                      UtlString modFP;
                      replaceSymbols(fieldparams.data(), requestUri, vdigits, modFP);
                      //separate each name value pair and check for symbols to subtitute
                      UtlTokenizer nameValuePair(modFP);
                      UtlString token;
                      while( nameValuePair.next(token, ";") )
                      {
                         UtlString temp ;
                         token = token.strip(UtlString::both);
                         UtlTokenizer nv(token) ;
                         nv.next(temp, "=");
                         name = temp ;
                         nv.next(temp, "=");
                         value = temp ;

                         tempContactUrl.includeAngleBrackets();
                         tempContactUrl.setFieldParameter(name , value);
                      }
                   }
                }
             }
             // Url parameters Tag
             urlText = transformElement->FirstChild(XML_TAG_URLPARAMS);
             if(urlText)
             {
                TiXmlNode* transformText = urlText->FirstChild();
                if(transformText && transformText->Type() == TiXmlNode::TEXT)
                {
                   TiXmlText* XmlUrlparams = transformText->ToText();
                   if (XmlUrlparams)
                   {
                      UtlString urlparams = XmlUrlparams->Value();
                      UtlString modFP;
                      replaceSymbols(urlparams.data(), requestUri, vdigits, modFP);
                      //separate each name value pair and check for symbols to subtitute
                      UtlTokenizer nameValuePair(modFP);
                      UtlString token;
                      while( nameValuePair.next(token, ";") )
                      {
                         UtlString temp ;
                         token = token.strip(UtlString::both);
                         UtlTokenizer nv(token) ;
                         nv.next(temp, "=");
                         name = temp ;
                         nv.next(temp, "=");
                         value = temp ;

                         tempContactUrl.setUrlParameter(name , value);
                      }
                   }
                }
             }

             // Header parameters Tag
             urlText = transformElement->FirstChild(XML_TAG_HEADERPARAMS);
             if(urlText)
             {
                TiXmlNode* transformText = urlText->FirstChild();
                if(transformText && transformText->Type() == TiXmlNode::TEXT)
                {
                   TiXmlText* XmlHeaderparams = transformText->ToText();
                   if (XmlHeaderparams)
                   {
                      UtlString headerparams = XmlHeaderparams->Value();
                      UtlString modHP;
                      replaceSymbols(headerparams.data(), requestUri, vdigits, modHP);
                      //separate each name value pair and check for symbols to subtitute
                      UtlTokenizer nameValuePair(modHP);
                      UtlString token;
                      while( nameValuePair.next(token, ";") )
                      {
                         UtlString temp ;
                         token = token.strip(UtlString::both);
                         UtlTokenizer nv(token) ;
                         nv.next(temp, "=");
                         name = temp ;
                         nv.next(temp, "=");
                         value = temp ;

                         tempContactUrl.setHeaderParameter(name , value);
                      }

                   }
                }
             }
             tempContactUrl.toString(tempContact);
          }

          OsSysLog::add(FAC_SIP, PRI_DEBUG, "UrlMapping::doTransform "
                        "tempContact = '%s'", tempContact.data());

          UtlHashMap registrationRow;
          UtlString* uriValue =
             new UtlString ( requestUri.toString() );
          UtlString* callidValue =
             new UtlString ( " " );
          UtlString* contactValue =
             new UtlString ( tempContact );
          UtlInt* expiresValue =
             new UtlInt ( 0 );
          UtlInt* cseqValue =
             new UtlInt ( 0 );
          UtlString* qvalueValue =
             new UtlString ( "1.0" );

          // Memory Leak fixes, make shallow copies of static keys
          UtlString* uriKey = new UtlString( cUriKey );
          UtlString* callidKey = new UtlString( cCallidKey );
          UtlString* contactKey = new UtlString( cContactKey );
          UtlString* expiresKey = new UtlString( cExpiresKey );
          UtlString* cseqKey = new UtlString( cCseqKey );
          UtlString* qvalueKey = new UtlString( cQvalueKey );

          registrationRow.insertKeyAndValue (
             uriKey, uriValue);
          registrationRow.insertKeyAndValue (
             callidKey, callidValue);
          registrationRow.insertKeyAndValue (
             contactKey, contactValue);
          registrationRow.insertKeyAndValue (
             expiresKey, expiresValue);
          registrationRow.insertKeyAndValue (
             cseqKey, cseqValue);
          registrationRow.insertKeyAndValue (
             qvalueKey, qvalueValue);

          rRegistrations.addValue( registrationRow );
          tempContact.remove(0);
          urlText = NULL;
       }
    }
    return currentStatus;
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
    UtlString& vdigits)
{
    vdigits.remove(0);
    if ( userPattern.SubStrings() > 1 )
    {
        vdigits.append( userPattern.Match( 1 ) );
    }
}

void
UrlMapping::replaceAll(const UtlString& originalString ,
                       UtlString &modifiedString ,
                       const UtlString& replaceWhat,
                       const UtlString& replaceWith)
{
    UtlString tempString(originalString);
    modifiedString.append(originalString);
    unsigned int index = UTL_NOT_FOUND;
    while ( (index = tempString.index(replaceWhat, 0, UtlString::ignoreCase) ) != UTL_NOT_FOUND)
    {
        modifiedString.replace(index, replaceWhat.length(), replaceWith);
        tempString.remove(0);
        tempString.append(modifiedString);
    }
}

/*
#define XML_SYMBOL_USER             "{user}"
#define XML_SYMBOL_USER_ESCAPED     "{user-escaped}"
#define XML_SYMBOL_DIGITS           "{digits}"
#define XML_SYMBOL_DIGITS_ESCAPED   "{digits-escaped}"
#define XML_SYMBOL_HOST             "{host}"
#define XML_SYMBOL_HEADERPARAMS     "{headerparams}"
#define XML_SYMBOL_URLPARAMS        "{urlparams}"
#define XML_SYMBOL_URI              "{uri}"
#define XML_SYMBOL_LOCALHOST        "{localhost}"
#define XML_SYMBOL_MEDIASERVER      "{mediaserver}"
#define XML_SYMBOL_VOICEMAIL        "{voicemail}"
#define XML_SYMBOL_VDIGITS          "{vdigits}"
#define XML_SYMBOL_VDIGITS_ESCAPED  "{vdigits-escaped}"
*/

void UrlMapping::replaceSymbols(const UtlString &string,
                                const Url& requestUri,
                                const UtlString& vdigits,
                                UtlString& modifiedString)
{
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "UrlMapping::replaceSymbols string = '%s'", string.data());

    UtlString tempString(string);
    //get original uri
    UtlString uri;
    requestUri.toString(uri);
    //get host port
    UtlString host;
    requestUri.getHostAddress(host);
    int port = requestUri.getHostPort();
    char buff[16];
    sprintf(buff, "%d", port);
    host.append(":");
    host.append(buff);
    //get user
    UtlString user;
    requestUri.getUserId(user);
    UtlString user_escaped = user;
    HttpMessage::escape(user_escaped) ;
    UtlString vdigits_escaped = vdigits ;
    HttpMessage::escape(vdigits_escaped);

    // Figure how many Url parameter entries
    int iEntries = 0 ;
    UtlString urlparam;
    int i = 0;
    Url tempUrl(requestUri);
    tempUrl.getUrlParameters(0, NULL, NULL, iEntries) ;
    //construct a string
    if (iEntries > 0)
    {
        UtlString *pNames = new UtlString[iEntries] ;
        UtlString *pValues = new UtlString[iEntries] ;
        tempUrl.getUrlParameters(iEntries, pNames, pValues, iEntries) ;
        for( i=0; i < iEntries; i++)
        {
           if( i != 0)
           {
              urlparam.append(";");
           }
            urlparam.append(pNames[i]);
            urlparam.append("=");
            urlparam.append(pValues[i]);
        }
        delete []pValues ;
        delete []pNames;
    }

    // get header parameters
    // Figure how many Url parameter entries
    iEntries = 0 ;
    UtlString headerparam;
    i = 0;
    tempUrl.getHeaderParameters(0, NULL, NULL, iEntries) ;
    //construct a string
    if (iEntries > 0)
    {
        UtlString *pNames = new UtlString[iEntries] ;
        UtlString *pValues = new UtlString[iEntries] ;
        tempUrl.getHeaderParameters(iEntries, pNames, pValues, iEntries) ;
        for( i=0; i < iEntries; i++)
        {
           if( i != 0)
           {
              headerparam.append(";");
           }
            headerparam.append(pNames[i]);
            headerparam.append("=");
            headerparam.append(pValues[i]);
        }
        delete []pValues ;
        delete []pNames;
    }

    replaceAll( tempString, modifiedString , XML_SYMBOL_USER , user );
    tempString.remove(0);
    tempString.append(modifiedString);
    modifiedString.remove(0);

    replaceAll( tempString, modifiedString , XML_SYMBOL_USER_ESCAPED , 
                user_escaped );
    tempString.remove(0);
    tempString.append(modifiedString);
    modifiedString.remove(0);

    replaceAll( tempString, modifiedString, XML_SYMBOL_DIGITS , user);
    tempString.remove(0);
    tempString.append(modifiedString);
    modifiedString.remove(0);

    replaceAll( tempString, modifiedString, XML_SYMBOL_DIGITS_ESCAPED , 
                user_escaped);
    tempString.remove(0);
    tempString.append(modifiedString);
    modifiedString.remove(0);

    replaceAll( tempString, modifiedString, XML_SYMBOL_HOST , host);
    tempString.remove(0);
    tempString.append(modifiedString);
    modifiedString.remove(0);

    replaceAll( tempString, modifiedString, XML_SYMBOL_HEADERPARAMS , headerparam);
    tempString.remove(0);
    tempString.append(modifiedString);
    modifiedString.remove(0);

    replaceAll( tempString, modifiedString, XML_SYMBOL_URLPARAMS , urlparam);
    tempString.remove(0);
    tempString.append(modifiedString);
    modifiedString.remove(0);

    replaceAll( tempString, modifiedString, XML_SYMBOL_URI , uri);
    tempString.remove(0);
    tempString.append(modifiedString);
    modifiedString.remove(0);

    replaceAll( tempString, modifiedString, XML_SYMBOL_LOCALHOST , mLocalhost);
    tempString.remove(0);
    tempString.append(modifiedString);
    modifiedString.remove(0);

    replaceAll( tempString, modifiedString, XML_SYMBOL_VOICEMAIL , mVoicemail);
    tempString.remove(0);
    tempString.append(modifiedString);
    modifiedString.remove(0);

    replaceAll( tempString, modifiedString, XML_SYMBOL_VDIGITS , vdigits);
    tempString.remove(0);
    tempString.append(modifiedString);
    modifiedString.remove(0);

    replaceAll( tempString, modifiedString, XML_SYMBOL_VDIGITS_ESCAPED , 
                vdigits_escaped);
    tempString.remove(0);
    tempString.append(modifiedString);
    modifiedString.remove(0);

    replaceAll( tempString, modifiedString, XML_SYMBOL_MEDIASERVER , mMediaServer);

    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "UrlMapping::replaceSymbols modifiedString = '%s'",
                  modifiedString.data());
}
