// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include "os/OsSysLog.h"
#include "os/OsConfigDb.h"
#include "utl/UtlRegex.h"

// APPLICATION INCLUDES
#include "net/Url.h"
#include "net/SipMessage.h"
#include "SipRouter.h"
#include "MSFT_ExchangeTransferHack.h"

// DEFINES
// CONSTANTS
const char* MSFT_ExchangeTransferHack::RecognizerConfigKey = "USERAGENT";

// TYPEDEFS
// FORWARD DECLARATIONS

/// Factory used by PluginHooks to dynamically link the plugin instance
extern "C" AuthPlugin* getAuthPlugin(const UtlString& pluginName)
{
   return new MSFT_ExchangeTransferHack(pluginName);
}

/// constructor
MSFT_ExchangeTransferHack::MSFT_ExchangeTransferHack(const UtlString& pluginName ///< the name for this instance
                                                     )
   : AuthPlugin(pluginName)
   , mUserAgentRegEx(NULL)
   , mpSipRouter(NULL)
{
   OsSysLog::add(FAC_SIP,PRI_INFO,"MSFT_ExchangeTransferHack plugin instantiated '%s'",
                 mInstanceName.data());
};

void 
MSFT_ExchangeTransferHack::announceAssociatedSipRouter( SipRouter* sipRouter )
{
   mpSipRouter = sipRouter;
}

/// Read (or re-read) the configuration.
void
MSFT_ExchangeTransferHack::readConfig( OsConfigDb& configDb /**< a subhash of the individual configuration
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
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "MSFT_ExchangeTransferHack[%s]::readConfig",
                 mInstanceName.data()
                 );

   if (mUserAgentRegEx)
   {
      delete mUserAgentRegEx;
      mUserAgentRegEx = NULL;
   }

   UtlString recognizer;
   if (configDb.get(RecognizerConfigKey, recognizer) && !recognizer.isNull())
   {
      OsSysLog::add( FAC_SIP, PRI_INFO
                    ,"MSFT_ExchangeTransferHack[%s]::readConfig "
                    " recognizer %s : '%s'"
                    ,mInstanceName.data(), RecognizerConfigKey
                    ,recognizer.data()
                    );
      
      try
      {
         mUserAgentRegEx = new RegEx(recognizer.data());
      }
      catch(const char* compileError)
      {
         OsSysLog::add( FAC_SIP, PRI_ERR
                       ,"MSFT_ExchangeTransferHack[%s]::readConfig "
                       " Invalid recognizer expression '%s' : %s"
                       ,mInstanceName.data()
                       ,recognizer.data()
                       ,compileError
                       );
         mUserAgentRegEx = NULL;
      }
   }
   else
   {
      OsSysLog::add( FAC_SIP, PRI_NOTICE
                    ,"MSFT_ExchangeTransferHack[%s]::readConfig "
                    " no recognizer '%s'"
                    ,mInstanceName.data(), RecognizerConfigKey
                    );
   }
}

AuthPlugin::AuthResult
MSFT_ExchangeTransferHack::authorizeAndModify(const UtlString& id,
                                              const Url&  requestUri,
                                              RouteState& routeState,
                                              const UtlString& method,
                                              AuthResult  priorResult,
                                              SipMessage& request,
                                              bool bSpiralingRequest,  
                                              UtlString&  reason
                                              )
{
   AuthResult result = CONTINUE; // we modify, but never make an authorization decision

   if (mUserAgentRegEx) // if not configured, do nothing
   {
      // get the call-id to use in logging
      UtlString callId;
      request.getCallIdField(&callId);

      /*
       * Note: a REFER from Exchange, with the bug we are hunting here, will always look like
       * it is tranferring to some foreign domain.  This will cause the TransferControl AuthPlugin
       * to ALLOW it.  If TransferControl starts challenging REFER from Exchange, that may mean
       * that MSFT has fixed the bug this plugin is meant to compensate for.
       */
      if (DENY != priorResult) // ignore anything some other plugin has already nixed
      {
         if (method.compareTo(SIP_REFER_METHOD) == 0) // we only care about REFER
         {
            UtlString userAgent;
            request.getUserAgentField( &userAgent );

            if (mUserAgentRegEx->Search(userAgent)) // does this look like Exchange?
            {
               UtlString targetStr;
               if (request.getReferToField(targetStr)) // get the address of the transfer target
               {
                  Url target(targetStr);
                  if (Url::SipUrlScheme == target.getScheme()) // target address parsed ok?
                  {
                     // check whether or not this is REFER with Replaces
                     UtlString targetDialog;
                     if (!target.getHeaderParameter(SIP_REPLACES_FIELD, targetDialog))
                     {
                        /*
                         * This is a REFER without Replaces from Exchange
                         * so check the domain parts of the two URIs to see if they match.
                         */

                        // Get the domain part of the transfer-target URI
                        UtlString targetDomain;
                        target.getHostWithPort(targetDomain);

                        // Get the domain part of the request URI
                        UtlString requestDomain;
                        requestUri.getHostWithPort(requestDomain);

                        if (targetDomain.compareTo(requestDomain, UtlString::ignoreCase) == 0)
                        {
                           // The domains are the same; this is the bug we're looking for...

                           UtlString correctDomain;
                           mpSipRouter->getDomain(correctDomain);
                           target.setHostAddress(correctDomain);
                           target.setHostPort(PORT_NONE);

                           UtlString modifiedTarget;
                           target.toString(modifiedTarget);
                           request.setReferToField(modifiedTarget.data());

                           OsSysLog::add(FAC_AUTH, PRI_INFO,
                                         "MSFT_ExchangeTransferHack[%s]::authorizeAndModify "
                                         "corrected transfer target domain in call '%s'\n"
                                         "changed '@%s' -> '@%s'",
                                         mInstanceName.data(), callId.data(),
                                         targetDomain.data(), correctDomain.data()
                                         );
                        }
                        else
                        {
                           // oh my god... did MSFT fix Exchange?
                           OsSysLog::add(FAC_AUTH, PRI_DEBUG,
                                         "MSFT_ExchangeTransferHack[%s]::authorizeAndModify "
                                         "request and target domain differ in '%s'; not modified",
                                         mInstanceName.data(), callId.data()
                                         );
                        }
                     }
                     else
                     {
                        // This is a REFER with Replaces from Exchange
                        // I don't expect this to happen, but if it does then don't mess with it.
                        OsSysLog::add(FAC_AUTH, PRI_INFO,
                                      "MSFT_ExchangeTransferHack[%s]::authorizeAndModify "
                                      "allowing REFER with Replaces in call '%s' to '%s'; no action",
                                      mInstanceName.data(), callId.data(), targetDialog.data()
                                      );
                     }
                  }
                  else
                  {
                     OsSysLog::add(FAC_AUTH, PRI_WARNING,
                                   "MSFT_ExchangeTransferHack[%s]::authorizeAndModify "
                                   "unrecognized refer target '%s' for call '%s'",
                                   mInstanceName.data(), targetStr.data(), callId.data()
                                   );
                  }
               }
               else
               {
                  // REFER without a Refer-To header... incorrect, but just ignore it.
                  OsSysLog::add(FAC_AUTH, PRI_WARNING,
                                "MSFT_ExchangeTransferHack[%s]::authorizeAndModify "
                                "REFER method without Refer-To in call '%s'",
                                mInstanceName.data(), callId.data()
                                );
               }
            }
            else
            {
               OsSysLog::add(FAC_AUTH, PRI_DEBUG,
                             "MSFT_ExchangeTransferHack[%s]::authorizeAndModify "
                             "User-Agent '%s' does not match recognizer in %s",
                             mInstanceName.data(), userAgent.data(), callId.data()
                             );
            }
         }
         else
         {
            // not a REFER - ignore it.
         }
      }
      else
      {
         // Some earlier plugin already decided on this - don't waste time figuring it out.
         OsSysLog::add(FAC_AUTH, PRI_DEBUG, "MSFT_ExchangeTransferHack[%s]::authorizeAndModify "
                       "prior authorization result %s for call %s",
                       mInstanceName.data(), AuthResultStr(priorResult), callId.data()
                       );
      }
   }
   return result;
}


/// destructor
MSFT_ExchangeTransferHack::~MSFT_ExchangeTransferHack()
{
   if (mUserAgentRegEx)
   {
      delete mUserAgentRegEx;
      mUserAgentRegEx = NULL;
   }
}
