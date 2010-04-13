// 
// Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "SipRouter.h"
#include "os/OsSysLog.h"
#include "net/SipXauthIdentity.h"
#include "CallDestination.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Factory used by PluginHooks to dynamically link the plugin instance
extern "C" AuthPlugin* getAuthPlugin(const UtlString& pluginName)
{
   return new CallDestination(pluginName);
}

/// constructor
CallDestination::CallDestination(const UtlString& pluginName ///< the name for this instance
                                   )
   : AuthPlugin(pluginName),
     mpSipRouter( 0 )
{
   OsSysLog::add(FAC_SIP,PRI_INFO,"CallDestination plugin instantiated '%s'",
                 mInstanceName.data());
};

/// Read (or re-read) the authorization rules.
void
CallDestination::readConfig( OsConfigDb& configDb /**< a subhash of the individual configuration
                                                    * parameters for this instance of this plugin. */
                             )
{
   // no configuration to read...
}

void 
CallDestination::announceAssociatedSipRouter( SipRouter* pSipRouter )
{
   mpSipRouter = pSipRouter;
}

AuthPlugin::AuthResult
CallDestination::authorizeAndModify(const UtlString& id,    /**< The authenticated identity of the
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
   AuthResult result = CONTINUE;
   if (method.compareTo(SIP_INVITE_METHOD) == 0) {
      moveCallDestToRecordRoute(request);
   }
   return result;
}


bool 
CallDestination::moveCallDestToRecordRoute( SipMessage& request )
{
   bool bRemoved = false;
   
   UtlString inviteUriStr;
   UtlString headercalldest;
      
   // Get the Command header and see if there are any Call Destination tags in it.
   // If there are tags, remove them from the header and add them to the Record-Route.
   request.getRequestUri(&inviteUriStr);
   Url inviteUri(inviteUriStr, Url::AddrSpec); 
   if (inviteUri.getUrlParameter(SIP_SIPX_CALL_DEST_FIELD, headercalldest, 0)) {
      UtlString recordroute;
         
      inviteUri.removeUrlParameter(SIP_SIPX_CALL_DEST_FIELD);
      request.changeUri(inviteUri.toString().data());
      request.getRecordRouteField(0, &recordroute);
      Url recordrouteUri(recordroute, Url::NameAddr); 
      UtlString routecalldest;
      if (recordrouteUri.getUrlParameter(SIP_SIPX_CALL_DEST_FIELD, routecalldest, 0)) {
         // Parameter exists.  Need to append to it.
         headercalldest = routecalldest + "," + headercalldest;
         recordrouteUri.setUrlParameter(SIP_SIPX_CALL_DEST_FIELD, headercalldest.data());
      }
      else {
         // Parameter doesn't exist.  Add it.
         recordrouteUri.setUrlParameter(SIP_SIPX_CALL_DEST_FIELD, headercalldest.data());
      }
      request.setRecordRouteField(recordrouteUri.toString().data(), 0);
      bRemoved = true;
   }
   return bRemoved;
}

/// destructor
CallDestination::~CallDestination()
{
}
