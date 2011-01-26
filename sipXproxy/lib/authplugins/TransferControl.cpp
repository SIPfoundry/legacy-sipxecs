// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include "os/OsSysLog.h"

// APPLICATION INCLUDES
#include "net/Url.h"
#include "net/SipMessage.h"
#include "net/SipXauthIdentity.h"
#include "SipRouter.h"
#include "TransferControl.h"

// DEFINES
// CONSTANTS

const char* SIP_METHOD_URI_PARAMETER = "method";

// TYPEDEFS
// FORWARD DECLARATIONS

/// Factory used by PluginHooks to dynamically link the plugin instance
extern "C" AuthPlugin* getAuthPlugin(const UtlString& pluginName)
{
   return new TransferControl(pluginName);
}

/// constructor
TransferControl::TransferControl(const UtlString& pluginName ///< the name for this instance
   )
   : AuthPlugin(pluginName),
     mpSipRouter(NULL)
{
   OsSysLog::add(FAC_SIP,PRI_INFO,"TransferControl plugin instantiated '%s'",
                 mInstanceName.data());
};

void 
TransferControl::announceAssociatedSipRouter( SipRouter* sipRouter )
{
   mpSipRouter = sipRouter;
}

/// Read (or re-read) the authorization rules.
void
TransferControl::readConfig( OsConfigDb& configDb /**< a subhash of the individual configuration
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
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "TransferControl[%s]::readConfig",
                 mInstanceName.data()
                 );
}

AuthPlugin::AuthResult
TransferControl::authorizeAndModify(const UtlString& id,    /**< The authenticated identity of the
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
   
   // get the call-id to use in logging
   UtlString callId;
   request.getCallIdField(&callId);

   if (DENY != priorResult)
   {
      if (method.compareTo(SIP_REFER_METHOD) == 0)
      {
         UtlString targetStr;
         if (request.getReferToField(targetStr))
         {
            Url target(targetStr, Url::NameAddr);  // parse the target URL

            UtlString targetMethod; 
            if (   Url::SipUrlScheme == target.getScheme() 
                /* REFER can create requests other than INVITE: we don't care about those       *
                 * so check that the method is INVITE or is unspecified (INVITE is the default) */
                && (   ! target.getUrlParameter(SIP_METHOD_URI_PARAMETER, targetMethod)
                    || (0==targetMethod.compareTo(SIP_INVITE_METHOD, UtlString::ignoreCase))
                    ))
            {
               if (id.isNull())
               {
                  // UnAuthenticated REFER. Do challenge the REFER to confirm the 
                  // identity of the transferor.  Note:  prior to XECS-2487, we used to challenge
                  // only the unauthenticated REFERs that didn't carry a Replaces header.
                  // The fix for XECS-2487 now requires that all unauthenticated REFERs
                  // be challenged so that consultative transfers get routed properly
                  // when user-based gateway section is used.  See tracker for the details
                  if (mpSipRouter->isLocalDomain(target))
                  {
                     OsSysLog::add(FAC_AUTH, PRI_INFO, "TransferControl[%s]::authorizeAndModify "
                                   "challenging transfer in call '%s'",
                                   mInstanceName.data(), callId.data()
                                   );
                     result = DENY; // we need an identity to attach to the Refer-To URI
                  }
                  else
                  {
                     /*
                      * This is a transfer to a target outside our domain, so let it go
                      * unchallenged.  See XECS-806
                      */
                     OsSysLog::add(FAC_AUTH, PRI_DEBUG, "TransferControl[%s]::authorizeAndModify "
                                   "allowing foriegn transfer in call '%s'",
                                   mInstanceName.data(), callId.data()
                                   );
                     // Add the References to the refer-to. Adding the callId field as a reference
                     // header (will be used in resulting INVITE) in the Refer-To provides us
                     // with enough information to be able to logically tie the calls together.
                     // Useful for CDR records.  
                     UtlString refcallId(callId);
                     refcallId.append(";rel=refer");
                     target.setHeaderParameter(SIP_REFERENCES_FIELD, refcallId.data());
                  
                     OsSysLog::add(FAC_AUTH, PRI_DEBUG, "TransferControl[%s]::authorizeAndModify "
                                   "adding Reference field [%s] to refer-to",
                                   mInstanceName.data(), callId.data()
                                  );
                     request.setReferToField(target.toString().data());

                     result = ALLOW;
                  }
               }
               else
               {
                  // Authenticated REFER 

                  // annotate the refer-to with the authenticated controller identity
                  SipXauthIdentity controllerIdentity;
                  controllerIdentity.setIdentity(id);
                  controllerIdentity.encodeUri(target);

                  // add the References to the refer-to.
                  UtlString refcallId(callId);
                  refcallId.append(";rel=refer");
                  target.setHeaderParameter(SIP_REFERENCES_FIELD, refcallId.data());
                  
                  OsSysLog::add(FAC_AUTH, PRI_DEBUG, "TransferControl[%s]::authorizeAndModify "
                                "adding Reference field [%s] to refer-to",
                                mInstanceName.data(), callId.data()
                               );
                  request.setReferToField(target.toString().data());


               }
            }
            else
            {
               OsSysLog::add(FAC_AUTH, PRI_WARNING, "TransferControl[%s]::authorizeAndModify "
                             "unrecognized refer target '%s' for call '%s'",
                             mInstanceName.data(), targetStr.data(), callId.data()
                             );
            }
         }
         else
         {
            // REFER without a Refer-To header... incorrect, but just ignore it.
            OsSysLog::add(FAC_AUTH, PRI_WARNING,
                          "TransferControl[%s]::authorizeAndModify "
                          "REFER method without Refer-To in call '%s'",
                          mInstanceName.data(), callId.data()
                          );
         }
      }
      else if (method.compareTo(SIP_INVITE_METHOD) == 0)
      {
         UtlString targetCallId;
         UtlString targetFromTag;
         UtlString targetToTag;

         if (request.getReplacesData(targetCallId, targetToTag, targetFromTag))
         {
            /*
             * This is an INVITE with Replaces: probably either the completion
             * of a call pickup or a consultative transfer.
             * In any case, it will not create a new call - just connect something
             * to an existing call - so we don't need to make any new authorization
             * decisions.
             */
            result = ALLOW;
         }
         else
         {
            // INVITE without Replaces: is not a transfer - ignore it.
         }
      }
      else
      {
         // neither REFER nor INVITE, so is not a transfer - ignore it.
      }
   }
   else
   {
      // Some earlier plugin already denied this - don't waste time figuring it out.
      OsSysLog::add(FAC_AUTH, PRI_DEBUG, "TransferControl[%s]::authorizeAndModify "
                    "prior authorization result %s for call %s",
                    mInstanceName.data(), AuthResultStr(priorResult), callId.data()
                    );
   }
   
   return result;
}


/// destructor
TransferControl::~TransferControl()
{
}
