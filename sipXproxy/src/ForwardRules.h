// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _ForwardRules_h_
#define _ForwardRules_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "utl/UtlString.h"
#include "os/OsStatus.h"
#include "xmlparser/tinyxml.h"
#include "digitmaps/Patterns.h"

// DEFINES
#define XML_TAG_ROUTES              "routes"
#define XML_TAG_ROUTEMATCH          "route"
#define XML_ATT_MAPPINGTYPE         "mappingType"
#define XML_TAG_ROUTEFROM           "routeFrom"
#define XML_TAG_ROUTEIPV4SUBNET     "routeIPv4subnet"
#define XML_TAG_ROUTEDNSWILDCARD    "routeDnsWildcard"
#define XML_TAG_METHODMATCH         "methodMatch"
#define XML_TAG_METHODPATTERN       "methodPattern"
#define XML_TAG_FIELDMATCH          "fieldMatch"
#define XML_ATT_FIELDNAME           "fieldName"
#define XML_TAG_FIELDPATTERN        "fieldPattern"
#define XML_TAG_ROUTETO             "routeTo"
#define XML_ATT_AUTHREQUIRED        "authRequired"

class UtlString;
class TiXmlNode;
class Url;
class SipMessage;

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

class ForwardRules 
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
	
/* ============================ CREATORS ================================== */

   ForwardRules();

   virtual ~ForwardRules();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */
   OsStatus loadMappings(const UtlString configFileName,
                         const UtlString mediaserver = "",
                         const UtlString& voicemail = "",
                         const UtlString& localhost = "");
   
   void buildDefaultRules(const char* domain,
                         const char* hostname,
                         const char* ipAddress,
                         const char* fqhn,
                         int localPort);

   static void buildDefaultRules(const char* domain,
                                 const char* hostname,
                                 const char* ipAddress,
                                 const char* fqhn,
                                 int localPort,
                                 TiXmlDocument& xmlDoc);

   OsStatus getRoute(const Url& requestUri,
      const SipMessage& request,
      UtlString& RouteToString,
      UtlString& mappingType,
      bool& authRequired);
   
/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   TiXmlDocument *mDoc;
   Patterns *mPatterns ;
   UtlString mVoicemail;
   UtlString mLocalhost;
   UtlString mMediaServer;

   OsStatus parseRouteMatchContainer(const Url& requestUri,
      const SipMessage& request,
      UtlString& RouteToString,
      UtlString& mappingType,
      bool& authRequired,
      TiXmlNode* routesNode,
      TiXmlNode* previousRouteMatchNode = NULL);
   
   OsStatus parseMethodMatchContainer(const SipMessage& request,
      UtlString& RouteToString,
      bool& authRequired,
      TiXmlNode* routeMatchNode,
      TiXmlNode* previousMethodMatchNode = NULL);

   OsStatus parseFieldMatchContainer(const SipMessage& request,
      UtlString& RouteToString,
      bool& authRequired,
      TiXmlNode* methodMatchNode,
      TiXmlNode* previousFieldMatchNode = NULL);

   OsStatus getRouteTo(UtlString& RouteToString,
      bool& authRequired,
      TiXmlNode* fieldMatchNode);


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   
/*////////////////////////////////////////////////////////////////////////// */
};

/* ============================ INLINE METHODS ============================ */

#endif  // _ForwardRules_h_
