// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include "os/OsLock.h"
#include "os/OsConfigDb.h"
#include "os/OsSysLog.h"
#include "os/OsFS.h"

// APPLICATION INCLUDES
#include "sipdb/CallerAliasDB.h"

#include "RouteState.h"
#include "SipRouter.h"
#include "CallerAlias.h"

// DEFINES
// CONSTANTS
const char* CallerAlias::CALLER_FROM_PARAM = "caller";
const char* CallerAlias::ALIAS_FROM_PARAM  = "alias";

// TYPEDEFS
OsMutex        CallerAlias::sSingletonLock(OsMutex::Q_FIFO);
CallerAlias*   CallerAlias::spInstance;
CallerAliasDB* CallerAlias::spCallerAliasDB;

/// Factory used by PluginHooks to dynamically link the plugin instance
extern "C" AuthPlugin* getAuthPlugin(const UtlString& pluginName)
{
   OsLock singleton(CallerAlias::sSingletonLock);

   if (!CallerAlias::spInstance)
   {
      CallerAlias::spCallerAliasDB = CallerAliasDB::getInstance();
      if (CallerAlias::spCallerAliasDB)
      {
         CallerAlias::spInstance = new CallerAlias(pluginName);
      }
      else
      {
         OsSysLog::add(FAC_SIP, PRI_CRIT, "CallerAlias[%s] "
                       "no CallerAliasDB found - aliases disabled",
                       pluginName.data());
      }
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_CRIT, "CallerAlias[%s]: "
                    "it is invalid to configure more than one instance of the CallerAlias plugin.",
                    pluginName.data());
      assert(false);
   }

   return CallerAlias::spInstance;
}

/// constructor
CallerAlias::CallerAlias(const UtlString& pluginName ///< the name for this instance
                         )
   : AuthPlugin(pluginName),
     mpSipRouter( 0 )
{
};

/// Nothing configurable outside the database right now
void
CallerAlias::readConfig( OsConfigDb& configDb /**< a subhash of the individual configuration
                                               * parameters for this instance of this plugin. */
                             )
{
   /*
    * @note
    * The parent service may call the readConfig method at any time to
    * indicate that the configuration may have changed.  The plugin
    * should reinitialize itself based on the configuration that exists when
    * this is called.  The fact that it is a subhash means that whatever prefix
    * is used to identify the plugin (see PluginHooks) has been removed (see the
    * examples in PluginHooks::readConfig).
    */
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "CallerAlias[%s]::readConfig",
                 mInstanceName.data()
                 );
}

AuthPlugin::AuthResult
CallerAlias::authorizeAndModify(const UtlString& id,    /**< The authenticated identity of the
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
                                SipMessage& request,    ///< see AuthPlugin regarding modifying
                                bool bSpiralingRequest, ///< spiraling indication 
                                UtlString&  reason      ///< rejection reason
                                )
{
   // get the call-id to use in logging
   UtlString callId;
   request.getCallIdField(&callId);

   if (   (priorResult != DENY) // no point in modifying a request that won't be sent
       && (spCallerAliasDB)     // there is a caller alias database? (should always be true)
       )   
   {
      UtlString callerFrom;
      UtlString aliasFrom;
      
      if (   !routeState.getParameter(mInstanceName.data(), CALLER_FROM_PARAM, callerFrom)
          || !routeState.getParameter(mInstanceName.data(), ALIAS_FROM_PARAM, aliasFrom)
          )
      {
         if (   routeState.isMutable()
             && routeState.directionIsCallerToCalled(mInstanceName.data())
             ) // a new dialog?
         {
            /*
             * Get the callers identity by getting the caller URI and:
             *    remove all parameters
             *    remove the scheme name
             */
            UtlString callerIdentity;

            UtlString originalFromField;
            request.getFromField(&originalFromField);
            Url originalFromUrl(originalFromField);
            
            /*
             * Extract the from identity as a key for the caller alias table
             * Start with the From header field (someday we should use the Identity if present)
             */
            Url fromUrl(originalFromUrl);
            fromUrl.removeParameters(); // parameters are not relevant for this 
         
            Url::Scheme fromUrlScheme = fromUrl.getScheme();
            switch (fromUrlScheme)
            {
            case Url::SipsUrlScheme:
               // sips and sip are equivalent for identity purposes,
               //   so just set to sip 
               fromUrl.setScheme(Url::SipUrlScheme);
               //   and fall through to extract the identity...

            case Url::SipUrlScheme:
               // case Url::TelUrlScheme: will go here, since 'tel' and 'sip' are the same length
               fromUrl.getUri(callerIdentity);
               callerIdentity.remove(0,4 /* strlen("sip:") */); // strip off the scheme name
               break;

            default:
               // for all other schemes, treat identity as null
               OsSysLog::add(FAC_SIP, PRI_WARNING,
                             "CallerAlias[%s]::check4andApplyAlias From uses unsupported scheme '%s'"
                             " - using null identity",
                             mInstanceName.data(),
                             fromUrl.schemeName(fromUrlScheme)
                             );
               break;
            }

            /*
             * Determine whether the identity is one for which this proxy
             * is authoritative; if not, we will not use wildcard matches.
             */
            bool identityIsLocal = mpSipRouter->isLocalDomain(fromUrl);
            
            // now we have callerIdentity set; use for looking up each contact.
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "CallerAlias[%s]::check4andApplyAlias "
                          "\n  caller '%s' %s",
                          mInstanceName.data(),
                          callerIdentity.data(),
                          identityIsLocal ? "is local" : "is not local"
                          );

            /*
             * Examine the request URI,
             *   checking for a caller alias set for its domain with callerIdentity
             */
            UtlString targetDomain;
            requestUri.getHostWithPort(targetDomain);

            // look up any caller alias for this identity and contact domain
            UtlString callerAlias;
            UtlString nullId; // empty string for wildcard matches
            if (spCallerAliasDB->getCallerAlias(callerIdentity, targetDomain, callerAlias)
                || (   identityIsLocal
                    && spCallerAliasDB->getCallerAlias(nullId, targetDomain, callerAlias)
                    )
                )
            {
               // found a caller alias, so rewrite the From information
               /*
                * The From header requires special handling
                * - we need to preserve the tag, if any, from the original header
                */
               UtlString originalFromTag;
               originalFromUrl.getFieldParameter("tag", originalFromTag);

               Url newFromUrl(callerAlias.data());
               newFromUrl.removeFieldParameter("tag"); // specifying a tag is a no-no
               if ( !originalFromTag.isNull() )
               {
                  newFromUrl.setFieldParameter("tag", originalFromTag.data());
               }
               UtlString newFromFieldValue;
               newFromUrl.toString(newFromFieldValue);
                   
               // rewrite the caller identity with the aliased value
               request.setRawFromField(newFromFieldValue.data());

               // save the original and new values so that we can fix them later
               routeState.setParameter(mInstanceName.data(),CALLER_FROM_PARAM,originalFromField);
               routeState.setParameter(mInstanceName.data(),ALIAS_FROM_PARAM,newFromFieldValue);

               OsSysLog::add( FAC_SIP, PRI_INFO,
                             "CallerAlias[%s]::check4andApplyAlias call %s set caller alias\n"
                             "  Original-From: %s\n"
                             "  Aliased-From:  %s",
                             mInstanceName.data(), callId.data(),
                             originalFromField.data(),
                             newFromFieldValue.data()
                             );
            }
            else
            {
               OsSysLog::add( FAC_SIP, PRI_DEBUG,
                             "CallerAlias[%s]::check4andApplyAlias call %s found no alias",
                             mInstanceName.data(), callId.data()
                             );
            }
         }
         else
         {
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "CallerAlias[%s]::authorizeAndModify "
                          "not mutable - no rewrite",
                          mInstanceName.data()                          
                          );
         }
      }
      else // the callerFrom and aliasFrom parameters were found
      {
         /*
          * This request has had its From rewritten, so fix either the From
          * or the To depending on which direction this request is going.
          */
         if (!request.isResponse()) // can't modify responses, so don't bother
         {
            if (routeState.directionIsCallerToCalled(mInstanceName.data()))
            {
               request.setRawFromField(aliasFrom);
               OsSysLog::add(FAC_AUTH, PRI_DEBUG, "CallerAlias[%s]::authorizeAndModify "
                             "call %s reset From",
                             mInstanceName.data(), callId.data()
                             );
            }
            else // direction is Called to Caller
            {
               request.setRawToField(callerFrom.data());
               OsSysLog::add(FAC_AUTH, PRI_DEBUG, "CallerAlias[%s]::authorizeAndModify "
                             "call %s reset To",
                             mInstanceName.data(), callId.data()
                             );
            }
         }
      }
   }
   return AuthPlugin::CONTINUE;
}

void CallerAlias::announceAssociatedSipRouter( SipRouter* sipRouter )
{
   mpSipRouter = sipRouter;
}

/// destructor
CallerAlias::~CallerAlias()
{
   spCallerAliasDB->releaseInstance();
   spCallerAliasDB = NULL;
}
