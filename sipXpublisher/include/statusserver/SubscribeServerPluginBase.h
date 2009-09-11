//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef SUBSCRIBESERVERPLUGINBASE_H
#define SUBSCRIBESERVERPLUGINBASE_H

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <os/OsStatus.h>

// DEFINES
// These are defined here until we create an application
// include which contains these
#define XML_TAG_SUBSCRIBE_SERVER_PLUGINS  "subscribe-server-plugins"
#define XML_TAG_SUBSCRIBE_PLUGINS         "subscribe-plugin"
#define XML_TAG_PERMISSIONMATCH "permissionMatch"
#define XML_TAG_PERMISSION      "permission"
#define XML_TAG_EVENT_TYPE      "event-type"
#define XML_TAG_LOAD_LIBARY     "load-library"
#define XML_TAG_PLUGIN_FACTORY  "plugin-factory"
#define XML_TAG_PLUGIN_DATA     "plugin-data"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipMessage;
class HttpMessage;
class HttpRequestContext;
class TiXmlNode;
class Notifier;

class SubscribeServerPluginBase
{
public:


    /**
     *
     * @param request
     * @param response
     * @param authenticatedUser
     * @param authenticatedRealm
     * @param domain
     *
     * @return
     */
    virtual OsStatus handleSubscribeRequest (
        const SipMessage& request,
        SipMessage& response,
        const char* authenticatedUser,
        const char* authenticatedRealm,
        const char* domain) = 0;

    /**
     *
     * @param requestContext
     * @param request
     * @param response
     *
     * @return
     */
    virtual OsStatus handleEvent (
        const HttpRequestContext& requestContext,
        const HttpMessage& request,
        HttpMessage& response ) = 0;

    /**
     *
     * @param request
     * @param response
     *
     * @return
     */
    virtual OsStatus handleNotifyResponse (
        const SipMessage& request,
        const SipMessage& response) = 0;

    /**
     */
    virtual void terminatePlugin() = 0;

    virtual ~SubscribeServerPluginBase()
    {
    }


protected:


};



//////////////////////////////////////////////////////////////////////////////////////////

#endif // SUBSCRIBESERVERPLUGINBASE_H
