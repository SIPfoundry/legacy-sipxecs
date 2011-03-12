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
#include "os/OsSysLog.h"
#include "utl/UtlString.h"
#include "net/Url.h"
#include "net/SipMessage.h"
#include "digitmaps/Patterns.h"

#include "ForwardRules.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

/* //////////////////////////// PUBLIC //////////////////////////////////// */
// Constructor
ForwardRules::ForwardRules()
   : mDoc(NULL),
   mPatterns(NULL)
{
}

// Destructor
ForwardRules::~ForwardRules()
{
   delete mPatterns;
}
/* ============================ MANIPULATORS ============================== */
OsStatus ForwardRules::loadMappings(const UtlString configFileName,
                                  const UtlString mediaserver,
                                  const UtlString& voicemail,
                                  const UtlString& localhost)
{
   OsStatus currentStatus = OS_SUCCESS;

   // Remove any old mappings first
   if(mDoc) delete mDoc;

   // Destroy patterns, and start clean
   if (mPatterns) delete mPatterns ;
   mPatterns = new Patterns() ;

   mDoc = new TiXmlDocument( configFileName.data() );
   if( !mDoc->LoadFile() )
   {
      UtlString parseError = mDoc->ErrorDesc();

      OsSysLog::add( FAC_SIP, PRI_ERR, "ERROR parsing forwardingrules '%s': %s"
                    ,configFileName.data(), parseError.data());

      return OS_NOT_FOUND;
   }
   
   if(!voicemail.isNull())
      mVoicemail.append(voicemail);
   
   if(!localhost.isNull())
      mLocalhost.append(localhost);
   
   if(!mediaserver.isNull())
      mMediaServer.append(mediaserver);
   
   return currentStatus;
}

void ForwardRules::buildDefaultRules(const char* domain,
                                     const char* hostname,
                                     const char* ipAddress,
                                     const char* fqhn,
                                     int localPort)
{
    if(mDoc) delete mDoc;

    // Destroy patterns, and start clean
    if (mPatterns) delete mPatterns ;
    mPatterns = new Patterns() ;

    mDoc = new TiXmlDocument();
    buildDefaultRules(domain,
                      hostname,
                      ipAddress,
                      fqhn,
                      localPort,
                      *mDoc);
    mDoc->Print();
}

void ForwardRules::buildDefaultRules(const char* domain,
                                     const char* hostname,
                                     const char* ipAddress,
                                     const char* fqhn,
                                     int localPort,
                                     TiXmlDocument& xmlDoc)
{
    // Note: fqhn == fully qualified host name



    UtlString hostnamePort(hostname ? hostname : "localhost");
    UtlString domainPort(domain ? domain : "");
    UtlString ipAddressPort(ipAddress ? ipAddress : "127.0.0.1");
    UtlString fqhnPort(fqhn ? fqhn : "localhost");

    if(localPort == 5060) localPort = PORT_NONE;
    if(portIsValid(localPort))
    {
        char portString[40];
        sprintf(portString,":%d", localPort);
        hostnamePort.append(portString);
        domainPort.append(portString);
        ipAddressPort.append(portString);
        fqhnPort.append(portString);
    }

    UtlString sdsAddress(fqhn);
    sdsAddress.append(":5090");
    UtlString statusAddress(fqhn);
    statusAddress.append(":5110");
    UtlString regAddress(fqhn);
    regAddress.append(":5070");
    UtlString configAddress("sipuaconfig");
    UtlString configFqhnAddress(configAddress);
    configFqhnAddress.append(".");
    configFqhnAddress.append(domain);

    TiXmlElement routes("routes");
    TiXmlElement route("route");
    route.SetAttribute("mappingType", "local");

    TiXmlElement routeFromDomain("routeFrom");
    TiXmlText domainText(domainPort.data());

    TiXmlElement routeFromFqhn("routeFrom");
    TiXmlText fqhnText(fqhnPort.data());

    TiXmlElement routeFromHost("routeFrom");
    TiXmlText hostText(hostnamePort.data());

    TiXmlElement routeFromIp("routeFrom");
    TiXmlText ipText(ipAddressPort.data());

    TiXmlElement methodMatch("methodMatch");

    TiXmlElement methodPattern("methodPattern");
    TiXmlText subscribe("SUBSCRIBE");

    TiXmlElement fieldMatchConfig("fieldMatch");
    fieldMatchConfig.SetAttribute("fieldName", "Event");

    TiXmlElement fieldPatternConfig("fieldPattern");
    TiXmlText configEvent("sip-config");

    TiXmlElement routeToSds("routeTo");
    TiXmlText sdsText(sdsAddress.data());

    TiXmlElement fieldMatchStatus("fieldMatch");
    fieldMatchStatus.SetAttribute("fieldName", "Event");

    TiXmlElement fieldPatternStatus("fieldPattern");
    TiXmlText mwiEvent("message-summary*");

    TiXmlElement routeToStatus("routeTo");
    TiXmlText statusText(statusAddress.data());

    TiXmlElement routeToReg("routeTo");
    TiXmlText regText(regAddress.data());

    TiXmlElement routeConfig("route");

    TiXmlElement routeFromFqhnConfig("routeFrom");
    TiXmlText fqhnConfigText(configFqhnAddress.data());

    TiXmlElement routeFromConfig("routeFrom");
    TiXmlText configText(configAddress.data());

    // Link everything up in reverse order as it TinyXml 
    // makes copies
    routeFromDomain.InsertEndChild(domainText);
    route.InsertEndChild(routeFromDomain);
    routeFromHost.InsertEndChild(hostText);
    route.InsertEndChild(routeFromHost);
    routeFromFqhn.InsertEndChild(fqhnText);
    route.InsertEndChild(routeFromFqhn);
    routeFromIp.InsertEndChild(ipText);
    route.InsertEndChild(routeFromIp);

    methodPattern.InsertEndChild(subscribe);
    methodMatch.InsertEndChild(methodPattern);

    fieldPatternStatus.InsertEndChild(mwiEvent);
    fieldMatchStatus.InsertEndChild(fieldPatternStatus);
    routeToStatus.InsertEndChild(statusText);
    fieldMatchStatus.InsertEndChild(routeToStatus);
    methodMatch.InsertEndChild(fieldMatchStatus);

    fieldPatternConfig.InsertEndChild(configEvent);
    fieldMatchConfig.InsertEndChild(fieldPatternConfig);
    routeToSds.InsertEndChild(sdsText);
    fieldMatchConfig.InsertEndChild(routeToSds);
    methodMatch.InsertEndChild(fieldMatchConfig);

    routeToReg.InsertEndChild(regText);
    methodMatch.InsertEndChild(routeToReg);
    route.InsertEndChild(methodMatch);

    route.InsertEndChild(routeToReg);

    routeFromFqhnConfig.InsertEndChild(fqhnConfigText);
    routeConfig.InsertEndChild(routeFromFqhnConfig);

    routeFromConfig.InsertEndChild(configText);
    routeConfig.InsertEndChild(routeFromConfig);

    routeConfig.InsertEndChild(routeToReg);

    routes.InsertEndChild(route);
    routes.InsertEndChild(routeConfig);

    xmlDoc.InsertEndChild(routes);

}

/* ============================ CREATORS ================================== */
OsStatus ForwardRules::getRoute(const Url& requestUri,
                                const SipMessage& request,
                                UtlString& routeToString,
                                UtlString& mappingType,
                                bool& authRequired)

{
    OsStatus currentStatus = OS_FAILED;

    // Get the "routes" element.
    TiXmlNode* prevRouteNode = mDoc->FirstChild( XML_TAG_ROUTES);
    if (!prevRouteNode)
    {
        OsSysLog::add(FAC_SIP, PRI_ERR, "UrlMapping::loadMappings - No child Node for Mappings");
        return OS_FILE_READ_FAILED;
    }

    currentStatus = parseRouteMatchContainer(requestUri,
                                            request,
                                            routeToString,
                                            mappingType,
                                            authRequired,
                                            prevRouteNode);

    return currentStatus;

}
OsStatus ForwardRules::parseRouteMatchContainer(const Url& requestUri,
                                              const SipMessage& request,
                                              UtlString& routeToString,
                                              UtlString& mappingType,
                                              bool& authRequired,
                                              TiXmlNode* routesNode,
                                              TiXmlNode* previousRouteMatchNode)
{
   UtlString testHost;
   requestUri.getHostAddress(testHost);
   int testPort = requestUri.getHostPort();
   if(testPort == SIP_PORT)
   {
      testPort = PORT_NONE;
   }
   
   UtlBoolean routeMatchFound = false;
   OsStatus methodMatchFound = OS_FAILED;
   
  	TiXmlElement* routesElement = routesNode->ToElement();

   TiXmlNode* routeMatchNode = previousRouteMatchNode;
   // Iterate through routes container children looking for 
   // route tags
   while ( (routeMatchNode = routesElement->IterateChildren(routeMatchNode)) 
      && methodMatchFound != OS_SUCCESS)
   {
      // Skip non-elements
      if(routeMatchNode && routeMatchNode->Type() != TiXmlNode::ELEMENT)
      {
         continue;
      }

      // Skip non-route elements
      TiXmlElement* routeMatchElement = routeMatchNode->ToElement();
      UtlString tagValue =  routeMatchElement->Value();
      if(tagValue.compareTo(XML_TAG_ROUTEMATCH) != 0 )
      {
         continue;
      }

      mappingType.remove(0);
      routeToString.remove(0);
      const char* mappingTypePtr = 
          routeMatchElement->Attribute(XML_ATT_MAPPINGTYPE);

      //get the mapping Type attribute
      mappingType.append(mappingTypePtr ? mappingTypePtr : "");

      // Iterate through the route container's children looking
      // for routeFrom, routeIPv4subnet, or routeDnsWildcard elements
      TiXmlNode* routeFromPatternNode = NULL;
      for( routeFromPatternNode = routeMatchElement->FirstChildElement();
         routeFromPatternNode;
         routeFromPatternNode = routeFromPatternNode->NextSiblingElement() )
      {
         // Skip elements that aren't of the "domainMatches" family 
         enum {ret_from, ret_ip, ret_dns} routeElementType ;

         const char *name = routeFromPatternNode->Value() ;
         if (strcmp(name, XML_TAG_ROUTEFROM) == 0)
         {
            routeElementType = ret_from;
         }
         else if (strcmp(name, XML_TAG_ROUTEIPV4SUBNET) == 0)
         {
            routeElementType = ret_ip;
         }
         else if (strcmp(name, XML_TAG_ROUTEDNSWILDCARD) == 0)
         {
            routeElementType = ret_dns;
         }
         else
         {
            continue ;
         }


         //found "domainMatches" pattern tag
         TiXmlElement* routeFromPatternElement = routeFromPatternNode->ToElement();
         //get the text value from it
         TiXmlNode* routeFromPatternText = routeFromPatternElement->FirstChild();
         if(routeFromPatternText && routeFromPatternText->Type() == TiXmlNode::TEXT)
         {
            TiXmlText* Xmlpattern = routeFromPatternText->ToText();
            if (Xmlpattern)
            {
               UtlString pattern = Xmlpattern->Value();

               switch(routeElementType)
               {
                  case ret_from: // a routeFrom element matches host and port
                  {
                     Url xmlUrl(pattern.data());
                     UtlString xmlHost;
                     xmlUrl.getHostAddress(xmlHost);
                     int xmlPort = xmlUrl.getHostPort();

                     // See if the host and port of the routeFrom elelment
                     // match that of the URI
                     if( (xmlHost.compareTo(testHost, UtlString::ignoreCase) == 0) &&
                        ((xmlPort == SIP_PORT && testPort == PORT_NONE) ||
                         xmlPort == testPort) )
                     {
                        routeMatchFound = true;
                        OsSysLog::add(FAC_SIP, PRI_DEBUG, "ForwardRules::parseRouteMatchContainer - routeFrom %s matches %s", testHost.data(), pattern.data());
                     }
                  }
                  break ;

                  case ret_ip: // a routeIPv4subnet matches if the subnet does
                  {
                     // "pattern" is a subnet in CIDR notation (x.y.z.q/size)
                     // "testHost is supposed to be an IPv4 dotted quad

                     routeMatchFound = mPatterns->
                                          IPv4subnet(testHost, pattern);
                  }
                  break ;

                  case ret_dns: // a routeDnsWildcard matches if the domain name does
                  {
                     // "pattern" is a wildcard DNS (*.pingtel.com)
                     // "testHost is a FQDN
                     routeMatchFound = mPatterns->
                                          DnsWildcard(testHost, pattern);
  
                  }
                  break ;
               }

               if (routeMatchFound)
               {
                  previousRouteMatchNode = routeMatchNode;
                  // Find a match to the request method and recurse
                  // to find child element field(s) matches  and
                  // get the routeTo value
                  methodMatchFound = parseMethodMatchContainer(request,
                     routeToString,
                     authRequired,
                     routeMatchNode);

                  if( methodMatchFound == OS_SUCCESS)
                     break;
               }
            }
         }
      }
   }
   return methodMatchFound;
}

OsStatus ForwardRules::parseMethodMatchContainer(const SipMessage& request,
                                                 UtlString& routeToString,
                                                 bool& authRequired,
                                                 TiXmlNode* routeMatchNode,
                                                 TiXmlNode* previousMethodMatchNode)
{
 
   OsStatus fieldMatchFound = OS_FAILED;
   UtlBoolean methodMatchFound = false;
   UtlString method;
   request.getRequestMethod(&method);

   TiXmlNode* methodMatchNode = previousMethodMatchNode;
   TiXmlElement* routeMatchElement = routeMatchNode->ToElement();

   // Iterate through the children of the routeFrom container
   // looking for methodMatch elements
   while ( (methodMatchNode = routeMatchElement->IterateChildren( methodMatchNode)) 
      && (fieldMatchFound != OS_SUCCESS) ) 
   {
       // Skip non-elements
      if(methodMatchNode && methodMatchNode->Type() != TiXmlNode::ELEMENT)
      {
         continue;
      }

      // Skip non-methodMatch elements
      UtlString tagValue = methodMatchNode->Value();
      if(tagValue.compareTo(XML_TAG_METHODMATCH) != 0 )
      {
         continue;
      }

      //found methodPattern tag
      TiXmlElement* methodMatchElement = methodMatchNode->ToElement();
      TiXmlNode* methodPatternNode = NULL;
      // Iteratore through the children of the methodMatch element
      // looking for the first methodPattern element that matches
      for( methodPatternNode = methodMatchElement->FirstChild( XML_TAG_METHODPATTERN);
         methodPatternNode;
         methodPatternNode = methodPatternNode->NextSibling(XML_TAG_METHODPATTERN ) )
      {
         // Skip non-elements
         if(methodPatternNode && methodPatternNode->Type() != TiXmlNode::ELEMENT)
         {
            continue;
         }

         TiXmlElement* methodPatternElement = methodPatternNode->ToElement();

         // Get the value contained in the methodPattern element
         TiXmlNode* methodPatternText = methodPatternElement->FirstChild();
         if(methodPatternText && methodPatternText->Type() == TiXmlNode::TEXT)
         {
            TiXmlText* XmlMethod = methodPatternText->ToText();
            if (XmlMethod)
            {
                // If the method of the request matches the method in
                // the methodMatch element
               UtlString methodString = XmlMethod->Value();
               if (methodString.compareTo(method, UtlString::ignoreCase) == 0 )
               {
                  // Found a matching method, see if there is a fieldMatch
                  // with a fieldName attribute that matches the fields
                  // in the message
                  methodMatchFound = true;
                  fieldMatchFound = parseFieldMatchContainer(request,
                     routeToString,
                     authRequired,
                     methodMatchNode);

                  if(fieldMatchFound == OS_SUCCESS)
                  {
                     break;
                  }

                  // None of the fields matched, see if the methodMatch
                  // element has an immediate child routeTo element.
                  // This is the "default" if none of the fieldMatches
                  // matched.
                  else
                  {
                      fieldMatchFound = getRouteTo(routeToString, 
                          authRequired,
                          methodMatchElement);
                      if(fieldMatchFound == OS_SUCCESS)
                      {
                          break;
                      }
                  }
               }
            }
         }
      }
   }

   if(fieldMatchFound == OS_FAILED)
   {
      // if none of the method match were successfull or if no methodMatch node present
      // get the default routeTo for this routeNode.
      fieldMatchFound = getRouteTo(routeToString, authRequired,
            routeMatchNode);
   }
   return fieldMatchFound;
}

OsStatus ForwardRules::parseFieldMatchContainer(const SipMessage& request,
                                                UtlString& routeToString,
                                                bool& authRequired,
                                                TiXmlNode* methodMatchNode,
                                                TiXmlNode* previousFieldMatchNode)
{
  
   OsStatus getRouteFound = OS_FAILED;
   UtlBoolean fieldPatternFound = false;

   TiXmlNode* fieldMatchNode = previousFieldMatchNode;

   while ( (fieldMatchNode = methodMatchNode->IterateChildren( fieldMatchNode))
      && (getRouteFound != OS_SUCCESS)   ) 
   {

      UtlBoolean fieldNameMatches = false;
      UtlBoolean noFieldPatternRequired = false;

      if(fieldMatchNode && fieldMatchNode->Type() != TiXmlNode::ELEMENT)
      {
         continue;
      }

      UtlString tagValue = fieldMatchNode->Value();
      if(tagValue.compareTo(XML_TAG_FIELDMATCH) != 0 )
      {
         continue;
      }

      TiXmlElement* fieldMatchElement  = fieldMatchNode->ToElement();
      TiXmlNode* fieldPatternNode = NULL;
      UtlBoolean fieldPatternPresent = false;
      UtlString fieldName;
      const char* fieldValuePtr = NULL;

      //check for fieldName parameter , if present check if it matches
      if(fieldMatchElement->Attribute(XML_ATT_FIELDNAME))
      {
         UtlString fieldNameXml = fieldMatchElement->Attribute(XML_ATT_FIELDNAME);
         // If field name is specified, 
         // check if it exists in the message
         if(!fieldNameXml.isNull())
         {
             fieldValuePtr = 
                 request.getHeaderValue(0, fieldNameXml.data());

             if(fieldValuePtr)
             {
                fieldNameMatches = true;
             }
         }
      }
      else
      {
         noFieldPatternRequired = true;
      }

      if(fieldNameMatches && !noFieldPatternRequired)
      {
         //get the user text value from it
         for( fieldPatternNode = fieldMatchElement->FirstChild( XML_TAG_FIELDPATTERN);
            fieldPatternNode;
            fieldPatternNode = fieldPatternNode->NextSibling(XML_TAG_FIELDPATTERN ) )
         {
            fieldPatternPresent = true;
            TiXmlElement* fieldPatternElement = fieldPatternNode->ToElement();

            TiXmlNode* fieldPatternText = fieldPatternElement->FirstChild();
            if(fieldPatternText)
            {
               try
               {
                  RegEx fieldPatternXml(fieldPatternText->Value(),PCRE_ANCHORED);

                  if (fieldPatternXml.Search(fieldValuePtr))
                  {
                     fieldPatternFound = true;
                  }
               }
               catch(const char * ErrorMsg)
               {
                  OsSysLog::add(FAC_SIP, PRI_ERR,
                                "Illegal regular expression <fieldPattern>%s</fieldPattern>"
                                " in forwardingrules.xml: %s",
                                fieldPatternText->Value() ,ErrorMsg
                                );
               }
            }
         }
      }
      if( (fieldNameMatches && (fieldPatternFound || !fieldPatternPresent))
          || noFieldPatternRequired )
      {
            //get the routeTo field
         getRouteFound = getRouteTo(routeToString, authRequired,
            fieldMatchNode);
      }

   }
   return getRouteFound;
}

// Function to get a boolean configuration setting based on the Y/N value of
// a configuration parameter.  Snarfed from 
// sipXregistry/lib/redirect_plugins/SipRedirectorJoin.cpp, 
// probably both should be reconciled into a lib one day
static bool getYN(const char* boolText,
                        bool defaultValue)
{
   // Start with the default value.
   bool value = defaultValue;
   if (boolText)
   {
      // Examine the first character.
      switch (boolText[0])
      {
      case 'y':
      case 'Y':
      case 'T':
      case 't':
      case '1':
         // If the value starts with Y or 1, set the result to TRUE.
         value = true;
         break;
      case 'n':
      case 'N':
      case 'F':
      case 'f':
      case '0':
         // If the value starts with N or 0, set the result to FALSE.
         value = false;
         break;
      default:
         // Ignore all other values.
         break;
      }
   }
   return value;
}

OsStatus ForwardRules::getRouteTo(UtlString& RouteToString,
                                 bool& authRequired,
                                  TiXmlNode* nodeWithRouteToChild)
{
   
   OsStatus currentStatus = OS_FAILED;
   nodeWithRouteToChild->ToElement();
   TiXmlNode* routeToNode = NULL;
   TiXmlNode* routeToText = NULL;


   //get the user text value from it
   routeToNode = nodeWithRouteToChild->FirstChild(XML_TAG_ROUTETO);
   if(routeToNode)
   {
      if(routeToNode && routeToNode->Type() != TiXmlNode::ELEMENT)
      {
         return currentStatus;
      }

      TiXmlElement* routeToElement = routeToNode->ToElement();

      const char* authRequiredPtr = 
         routeToElement->Attribute(XML_ATT_AUTHREQUIRED);

      //set the authRequired attribute
      authRequired=getYN(authRequiredPtr, false) ; //defaults to false

      RouteToString.remove(0);
      routeToText = routeToElement->FirstChild();
      if(routeToText && routeToText->Type() == TiXmlNode::TEXT)
      {
         TiXmlText* routeTo = routeToText->ToText();
         if (routeTo)
         {
            RouteToString.append(routeTo->Value());
         }
      }
      // TinyXML compresses white space, so an all white space element becomes
      // zero length, but just in case, let's strip it ourselves.
      RouteToString.strip(UtlString::both);
      if (RouteToString.length() > 0)
      {
         // The route is not empty.  Declare success
         currentStatus = OS_SUCCESS;
      }
      else if (authRequired)
      {
         // The Route can be empty, as long as authRequired is set.
         // This means just route to the authProxy.
         currentStatus = OS_SUCCESS;
      }
      else
      {
         // Empty or missing route, and no authRequired...that's not allowed.
         currentStatus = OS_FAILED;
      }
   }
   return currentStatus;
}
