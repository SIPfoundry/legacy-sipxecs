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
#include "RequestLinter.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Factory used by PluginHooks to dynamically link the plugin instance
extern "C" AuthPlugin* getAuthPlugin(const UtlString& pluginName)
{
   return new RequestLinter(pluginName);
}

/// constructor
RequestLinter::RequestLinter(const UtlString& pluginName ///< the name for this instance
                                   )
   : AuthPlugin(pluginName),
     mpSipRouter( 0 )
{
   OsSysLog::add(FAC_SIP,PRI_INFO,"RequestLinter plugin instantiated '%s'",
                 mInstanceName.data());
};

/// Read (or re-read) the authorization rules.
void
RequestLinter::readConfig( OsConfigDb& configDb /**< a subhash of the individual configuration
                                                    * parameters for this instance of this plugin. */
                             )
{
   // no configuration to read...
}

void 
RequestLinter::announceAssociatedSipRouter( SipRouter* pSipRouter )
{
   mpSipRouter = pSipRouter;
}

AuthPlugin::AuthResult
RequestLinter::authorizeAndModify(const UtlString& id,    /**< The authenticated identity of the
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
   if( bSpiralingRequest == false )
   {
      removeOurPAssertedIdentityHeader( request );
   }
   return result;
}


bool 
RequestLinter::removeOurPAssertedIdentityHeader( SipMessage& request )
{
   bool bRemoved = false;
   
   if( mpSipRouter )
   {
      int index = 0;
      const char* pPAssertedIdentity;
      
      while( ( pPAssertedIdentity = request.getHeaderValue( index, SipXauthIdentity::PAssertedIdentityHeaderName ) ) ) 
      {
         Url pAssertedIdentityUrl( pPAssertedIdentity );
         // check if the P-Asserted-Identity is for our domain.
         if( mpSipRouter->isLocalDomain( pAssertedIdentityUrl ) )
         {
            // P-Asserted-Identity found for our domain, remove it.
            request.removeHeader( SipXauthIdentity::PAssertedIdentityHeaderName, index );
            // note: do not advance the index.  The removal of the header effectivly
            // shifts all the remaining headers up so no need for increment to get at the next one.
            bRemoved = true;
         }
         else
         {
            // advance the index to evaluate the next P-Asserted-Identity
            index++;
         }
      }
   }
   return bRemoved;
}

/// destructor
RequestLinter::~RequestLinter()
{
}
