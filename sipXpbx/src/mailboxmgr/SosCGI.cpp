// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "utl/UtlString.h"
#include "net/Url.h"
#include "mailboxmgr/VXMLDefs.h"
#include "mailboxmgr/MailboxManager.h"
#include "mailboxmgr/SosCGI.h"
#include "xmlparser/tinyxml.h"

// DEFINES
#define E911_MAPPING_FILE     SIPX_CONFDIR "/e911rules.xml"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/* ============================ CREATORS ================================== */

SosCGI::SosCGI(const Url& from)
{
   from.getIdentity(mFrom);
   OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG, "SosCGI - Caller identity = %s\n", mFrom.data());
}


SosCGI::~SosCGI()
{}


OsStatus
SosCGI::execute(UtlString* out)
{

    // Parse the mapping file e911rules.xml
    OsStatus res = parseMappingFile (UtlString(E911_MAPPING_FILE));
    if (res == OS_FAILED)
    {
       OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_ERR, "SosCGI - parsing e911rules.xml failed!!!\n");
    }

    // Construct the dynamic VXML
    UtlString dynamicVxml = getVXMLHeader();
    dynamicVxml += "<form id=\"emergencyservice\">\n";
    dynamicVxml += "<property name=\"interdigittimeout\" value=\"3s\" />\n";
    dynamicVxml += "<property name=\"timeout\" value=\"7s\" />\n";
    dynamicVxml += "<transfer name=\"emergencyservicexfer\" dest=\"" + mSosUrl + "\" />\n";
    dynamicVxml += "</form>\n";
    dynamicVxml += VXML_END;

    // Write out the dynamic VXML script to be processed by OpenVXI
    if (out) 
    {
        out->remove(0);
        UtlString responseHeaders;
        MailboxManager::getResponseHeaders(dynamicVxml.length(), responseHeaders);

        out->append(responseHeaders.data());
        out->append(dynamicVxml.data());
    }
    return OS_SUCCESS;
}


OsStatus
SosCGI::parseMappingFile(const UtlString& mapFile)
{
    OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                  "SosCGI - Entering parseMappingFile('%s')\n",
                  mapFile.data());
    OsStatus result = OS_FAILED;

    TiXmlDocument doc(mapFile);

    // Verify that we can load the file (i.e it must exist)
    if(doc.LoadFile())
    {
       OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                     "SosCGI - doc.LoadFile() returned TRUE");

       TiXmlNode * rootNode = doc.FirstChild ("mappings");

       if (rootNode != NULL)
       {
          // Search in each group
          for (TiXmlNode *groupNode = rootNode->FirstChild("userMatch");
               groupNode; 
               groupNode = groupNode->NextSibling("userMatch"))
          {
             // Compare with each device in this group
             for (TiXmlNode *deviceNode = groupNode->FirstChild("userPattern");
                  deviceNode;
                  deviceNode = deviceNode->NextSibling("userPattern"))
             {
                if (deviceNode->Type() == TiXmlNode::ELEMENT)
                {
                   UtlString deviceName = (deviceNode->FirstChild())->Value();

                   // Compare it with the caller's identify
                   if(mFrom.compareTo(deviceName) == 0)
                   {
                      // The same device is found. Get the routing address
                      TiXmlNode *routeNode = groupNode->FirstChild("transform");
                      UtlString userName = ((routeNode->FirstChild("user"))->FirstChild())->Value();
                      UtlString hostName = ((routeNode->FirstChild("host"))->FirstChild())->Value();
                      TiXmlNode *urlParamsNode = routeNode->FirstChild("urlparams");
                      if (urlParamsNode != NULL)
                      {
                         UtlString urlParams = urlParamsNode->FirstChild()->Value();
                         mSosUrl = "&lt;sip:" + userName + "@" + hostName + ";" + urlParams + "&gt;";
                      }
                      else
                      {
                         mSosUrl = userName + "@" + hostName;
                      }
                      OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                                    "SosCGI - parseMappingFile:: Found the device %s and use url %s\n", mFrom.data(), mSosUrl.data());

                      return OS_SUCCESS;
                   }
                }
             }
          }

          // There is no match device, use the default routing address instead
          OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_INFO,
                        "SosCGI - parseMappingFile::Could not find the device in the mapping file = %s\n", mFrom.data());
          TiXmlNode *defaultGroupNode = rootNode->FirstChild("defaultMatch");
          TiXmlNode *routeNode = defaultGroupNode->FirstChild("transform");
          UtlString userName = ((routeNode->FirstChild("user"))->FirstChild())->Value();
          UtlString hostName = ((routeNode->FirstChild("host"))->FirstChild())->Value();
          TiXmlNode *urlParamsNode = routeNode->FirstChild("urlparams");
          if (urlParamsNode != NULL)
          {
             UtlString urlParams = urlParamsNode->FirstChild()->Value();
             mSosUrl = "&lt;sip:" + userName + "@" + hostName + ";" + urlParams + "&gt;";
          }
          else
          {
             mSosUrl = userName + "@" + hostName;
          }
          OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                        "SosCGI - using default url = %s\n", mSosUrl.data());

          return OS_SUCCESS;
       }
    }

    return result;
}

