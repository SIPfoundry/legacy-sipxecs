// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//////////////////////////////////////////////////////////////////////////////

// APPLICATION INCLUDES
#include "DummyAuthPlugIn.h"
#include "os/OsSysLog.h"

// TYPEDEFS
// FORWARD DECLARATIONS

/// Factory used by PluginHooks to dynamically link the plugin instance
extern "C" AuthPlugin* getAuthPlugin(const UtlString& pluginName)
{
   return new DummyAuthPlugin(pluginName);
}

/// constructor
DummyAuthPlugin::DummyAuthPlugin(const UtlString& pluginName )
   : AuthPlugin(pluginName),
     mbDenyNextRequest( false )
{
   OsSysLog::add(FAC_SIP,PRI_INFO,"DummyAuthPlugin plugin instantiated '%s'",
                                 pluginName.data());
};


AuthPlugin::AuthResult
DummyAuthPlugin::authorizeAndModify(const UtlString& id,    /**< The authenticated identity of the
                                                              *   request originator, if any (the null
                                                              *   string if not).
                                                              *   This is in the form of a SIP uri
                                                              *   identity value as used in the
                                                              *   credentials database (user@domain)
                                                              *   without the scheme or any parameters.
                                                              */
                                     const Url&  requestUri, ///< parsed target Uri
                                     RouteState& routeState, ///< the state for this request.  
                                     const UtlString& method,///< the request method
                                     AuthResult  priorResult,///< results from earlier plugins.
                                     SipMessage& request,    ///< see AuthPlugin wrt modifying
                                     bool bSpiralingRequest, ///< request spiraling indication 
                                     UtlString&  reason      ///< rejection reason
                                     )
{
   mLastAuthenticatedId = id;
   if( mbDenyNextRequest )
   {
      mbDenyNextRequest = false;
      priorResult = AuthPlugin::DENY;
   }
   mLastAuthResult      = priorResult;
   return priorResult;

}

/// destructor
DummyAuthPlugin::~DummyAuthPlugin(){}
