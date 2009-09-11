//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////
// MwiPlugin.h: interface for the MwiPlugin class.

#ifndef MWIPLUGIN_H
#define MWIPLUGIN_H

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <statusserver/SubscribeServerPluginBase.h>

// DEFINES
#define XML_TAG_VOICEMAILCGI "voicemail-cgi-url"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class TiXmlElement;
class Notifier;

/**
 */
class MwiPlugin : public SubscribeServerPluginBase
{
public:
    /**
     *
     * @param eventType
     * @param pluginElement
     * @param notifier
     */
    MwiPlugin( const UtlString& eventType,
               const TiXmlElement& pluginElement,
               Notifier* notifier );

    /**
     */
    virtual ~MwiPlugin();

    /**
     *
     * @param request
     * @param response
     * @param authenticatedUser
     * @param authenticatedRealm
     * @param domain   The domain should not need to
     *                 be sent int.  This is a temporary bogus work around
     *                 for stuff that should be done in the SubscribeServer NOT
     *                 in the plugin.
     *
     * @return
     */
    OsStatus handleSubscribeRequest(
        const SipMessage& request,
        SipMessage& response,
        const char* authenticatedUser,
        const char* authenticatedRealm,
        const char* domain);

    /**
     *
     * @param requestContext
     * @param request
     * @param response
     *
     * @return
     */
    OsStatus handleEvent(
        const HttpRequestContext& requestContext,
        const HttpMessage& request,
        HttpMessage& response );

    /**
     *
     * @param request
     * @param response
     *
     * @return
     */
    OsStatus handleNotifyResponse (
        const SipMessage& request,
        const SipMessage& response);

    /** */
    void terminatePlugin();

protected:
    Notifier*           mNotifier;
    UtlString            mVoicemailCGIUrl;

};
#endif // MWIPLUGIN_H
