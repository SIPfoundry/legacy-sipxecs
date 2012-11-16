// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//////////////////////////////////////////////////////////////////////////////
#include <string>

#include "CallerAlias.h"
#include <sipxproxy/SipRouter.h>
// SYSTEM INCLUDES
#include "os/OsLock.h"
#include "os/OsConfigDb.h"
#include "os/OsLogger.h"
#include "os/OsFS.h"
#include "net/Url.h"

#include <sipxproxy/RouteState.h>



// DEFINES
// CONSTANTS
const char* CallerAlias::CALLER_FROM_PARAM = "c";
const char* CallerAlias::ALIAS_FROM_PARAM  = "a";
const char* CallerAlias::CALLER_TAG_OFFSET_PARAM = "co";
const char* CallerAlias::ALIAS_TAG_OFFSET_PARAM  = "ao";

// TYPEDEFS
OsMutex        CallerAlias::sSingletonLock(OsMutex::Q_FIFO);
CallerAlias*   CallerAlias::spInstance;


/// Factory used by PluginHooks to dynamically link the plugin instance
extern "C" AuthPlugin* getAuthPlugin(const UtlString& pluginName)
{
   OsLock singleton(CallerAlias::sSingletonLock);

   if (!CallerAlias::spInstance)
   {
        CallerAlias::spInstance = new CallerAlias(pluginName);
   }
   else
   {
      Os::Logger::instance().log(FAC_SIP, PRI_CRIT, "CallerID[%s]: "
                    "it is invalid to configure more than one instance of the CallerAlias plugin.",
                    pluginName.data());
      assert(false);
   }

   return CallerAlias::spInstance;
}

/// constructor
CallerAlias::CallerAlias(const UtlString& pluginName ///< the name for this instance
) :
	AuthPlugin(pluginName)
{
	MongoDB::ConnectionInfo info(MongoDB::ConnectionInfo::connectionStringFromFile(), EntityDB::NS);
	mpEntityDb = new EntityDB(info);
}
;

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
   Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, "CallerID[%s]::readConfig",
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
       )   
   {
      UtlString callerFrom;
      UtlString callerFromTagOffsetStr;
      UtlString aliasFrom;
      UtlString aliasFromTagOffsetStr;
      UtlString originalFromTag;

      if (   !routeState.getParameter(mInstanceName.data(), CALLER_FROM_PARAM, callerFrom)
          || !routeState.getParameter(mInstanceName.data(), CALLER_TAG_OFFSET_PARAM, callerFromTagOffsetStr)
          || !routeState.getParameter(mInstanceName.data(), ALIAS_FROM_PARAM, aliasFrom)
          || !routeState.getParameter(mInstanceName.data(), ALIAS_TAG_OFFSET_PARAM, aliasFromTagOffsetStr)
          || !routeState.originalCallerFromTagValue(mInstanceName.data(), originalFromTag)
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
            mpSipRouter->ensureCanonicalDomain(fromUrl);
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
               Os::Logger::instance().log(FAC_SIP, PRI_WARNING,
                             "CallerID[%s]::check4andApplyAlias From uses unsupported scheme '%s'"
                             " - using null identity",
                             mInstanceName.data(),
                             Url::schemeName(fromUrlScheme)
                             );
               break;
            }

            /*
             * Determine whether the identity is one for which this proxy
             * is authoritative; if not, we will not use wildcard matches.
             */
            bool identityIsLocal = mpSipRouter->isLocalDomain(fromUrl);
            
            // now we have callerIdentity set; use for looking up each contact.
            Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                          "CallerID[%s]::check4andApplyAlias "
                          "\n  caller '%s' %s",
                          mInstanceName.data(),
                          callerIdentity.data(),
                          identityIsLocal ? "is local" : "is not local"
                          );

            /*
             * Examine the request URI,
             * checking for a caller alias set for its domain(including asssociated gateway sipxecsLineid)  with callerIdentity
             */

            UtlString sipxecsLineIdField;
            requestUri.getUrlParameter(SIPX_SIPXECS_LINEID_URI_PARAM, sipxecsLineIdField);

            Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                             "getUrlParameter: sipxecsLineid[%s]"
                             " in CallerID",
                             sipxecsLineIdField.data()
                             );

            UtlString targetDomain;
            requestUri.getHostWithPort(targetDomain);

            if (!(sipxecsLineIdField.isNull()))
            {
                targetDomain.append(";").append(SIPX_SIPXECS_LINEID_URI_PARAM).append("=").append(sipxecsLineIdField.data());
            }

            Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                          "CallerID::targetDomain [%s]",
                          targetDomain.data()
                          );

            // look up any caller alias for this identity and contact domain
            UtlString callerAlias;
            if (identityIsLocal && getCallerAlias(callerIdentity, targetDomain, callerAlias) )
            {
               // found a caller alias, so rewrite the From information
               /*
                * The From header requires special handling
                * - we need to preserve the tag, if any, from the original header
                */
               originalFromUrl.getFieldParameter("tag", originalFromTag);

               Url newFromUrl(callerAlias.data());
               newFromUrl.removeFieldParameter("tag"); // specifying a tag is a no-no
               if ( !originalFromTag.isNull() )
               {
                  newFromUrl.setFieldParameter("tag", originalFromTag.data());
               }
               UtlString newFromFieldValue;
               newFromUrl.toString(newFromFieldValue);
                   
               // log the change we are making before stripping the tag from the field values
               Os::Logger::instance().log( FAC_SIP, PRI_INFO,
                             "CallerID[%s]::check4andApplyAlias call %s set caller alias\n"
                             "  Original-From: %s\n"
                             "  Aliased-From:  %s",
                             mInstanceName.data(), callId.data(),
                             originalFromField.data(),
                             newFromFieldValue.data()
                             );

               // rewrite the caller identity with the aliased value
               request.setRawFromField(newFromFieldValue.data());

               // Factor the tag values out of the field values stored in the RouteState
               //  We do this because otherwise we'll end up encoding and sending two copies
               //  of the tag; since some phones send really long tag values (no one knows why),
               //  this can cause such large Record-Route headers that they cause interop problems.
               if ( ! originalFromTag.isNull() )
               {
                  // find the offset of the tag value in the callers from field
                  ssize_t callerFromTagOffset;
                  callerFromTagOffset = originalFromField.index(originalFromTag);
                  callerFromTagOffsetStr.appendNumber(callerFromTagOffset);
                  // strip the tag value from the original From value to be stored in the RouteState
                  originalFromField.replace(callerFromTagOffset, originalFromTag.length(), "");
                  
                  // find the offset of the tag value in the aliased from field
                  ssize_t aliasFromTagOffset;
                  aliasFromTagOffset = newFromFieldValue.index(originalFromTag);
                  aliasFromTagOffsetStr.appendNumber(aliasFromTagOffset);
                  // strip the tag value from the aliased From value to be stored in the RouteState
                  newFromFieldValue.replace(aliasFromTagOffset, originalFromTag.length(), "");
               }

               // save the original and new values so that we can fix them later
               routeState.setParameter(mInstanceName.data(),
                                       CALLER_FROM_PARAM,originalFromField);
               routeState.setParameter(mInstanceName.data(),
                                       CALLER_TAG_OFFSET_PARAM,callerFromTagOffsetStr);
               routeState.setParameter(mInstanceName.data(),
                                       ALIAS_FROM_PARAM,newFromFieldValue);
               routeState.setParameter(mInstanceName.data(),
                                       ALIAS_TAG_OFFSET_PARAM,aliasFromTagOffsetStr);
            }
            else
            {
               Os::Logger::instance().log( FAC_SIP, PRI_DEBUG,
                             "CallerID[%s]::check4andApplyAlias call %s found no caller id",
                             mInstanceName.data(), callId.data()
                             );
            }
         }
         else
         {
            Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                          "CallerID[%s]::authorizeAndModify "
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
            size_t tagOffset;
            
            if (routeState.directionIsCallerToCalled(mInstanceName.data()))
            {
               // replace the from tag value in the stored aliased header
               tagOffset = strtol(aliasFromTagOffsetStr.data(), NULL, 10);
               aliasFrom.insert(tagOffset, originalFromTag);

               // put the aliased header into the message
               request.setRawFromField(aliasFrom);
               Os::Logger::instance().log(FAC_AUTH, PRI_DEBUG, "CallerID[%s]::authorizeAndModify "
                             "call %s reset From",
                             mInstanceName.data(), callId.data()
                             );
            }
            else // direction is Called to Caller
            {
               // replace the from tag value in the stored original header
               tagOffset = strtol(callerFromTagOffsetStr.data(), NULL, 10);
               callerFrom.insert(tagOffset, originalFromTag);

               request.setRawToField(callerFrom.data());
               Os::Logger::instance().log(FAC_AUTH, PRI_DEBUG, "CallerID[%s]::authorizeAndModify "
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
   if (mpEntityDb != NULL) {
	   delete mpEntityDb;
	   mpEntityDb = 0;
   }
}

static std::string string_right(const std::string& str, size_t size)
{
  if (str.size() <= size)
    return str;

  size_t index = str.size() - size;

  return str.substr(index, size);
}

bool CallerAlias::getCallerAlias (
  const UtlString& identity,
  const UtlString& domain,
  UtlString& callerAlias_
) const
{
    EntityRecord userEntity;
    EntityRecord gatewayEntity;
    bool hasUserEntity = false;
    bool hasGatewayEntity = false;
    std::string callerAlias;
    OS_LOG_INFO(FAC_SIP, "CallerID::getCallerAlias - EntityDB::findByIdentity for identity=" << identity.str() << " domain=" << domain.str());

    hasUserEntity = mpEntityDb->findByIdentity(identity.str(), userEntity);
    hasGatewayEntity = mpEntityDb->findByIdentity(domain.str(), gatewayEntity);

    // if call is not routed through gw then nothing to do, return empty
    if (!hasGatewayEntity) {
        return false;
    }

    // ignore user and gateway options in case configured to transform extension
    if (gatewayEntity.callerId().transformExtension)
    {
        size_t loc = identity.str().find("@");
        if (loc != std::string::npos)
        {
            std::string userId = identity.str().substr(0, loc);
            //
            // Check if we need to truncate the userId to a certain length
            //
            if (gatewayEntity.callerId().extensionLength > 0 && userId.length() > (size_t)gatewayEntity.callerId().extensionLength)
                userId = string_right(userId, gatewayEntity.callerId().extensionLength);

            //
            // Now check if a prefix is specified
            //
            if (!gatewayEntity.callerId().extensionPrefix.empty())
            {
                std::string buff = gatewayEntity.callerId().extensionPrefix;
                buff += userId;
                userId = userId = buff;
            }

            callerAlias = "<sip:";
            callerAlias += userId;
            callerAlias += identity.str().substr(loc);
            callerAlias += ">";
        }
    }
    else
    {
      // apply user settings in case gateway not configured to ignore them
      if (hasUserEntity && !gatewayEntity.callerId().ignoreUserCalleId) {
          if (userEntity.callerId().enforcePrivacy)
          {
              callerAlias = "sip:anonymous@anonymous.invalid";
          }
          else if (!userEntity.callerId().id.empty())
          {
              callerAlias = userEntity.callerId().id;
          }
      } 

      // use gateway settings in case user didn't specify them or gateway configured to ignore user's calle id
      if (callerAlias.empty()) {
          if (gatewayEntity.callerId().enforcePrivacy)
          {
              callerAlias = "sip:anonymous@anonymous.invalid";
          }
          else if (!gatewayEntity.callerId().id.empty())
          {
              callerAlias = gatewayEntity.callerId().id;
          }
      }
    }
    
    if (!callerAlias.empty())
        callerAlias_ = callerAlias.c_str();
    else
        OS_LOG_DEBUG(FAC_SIP, "CallerID::getCallerAlias - No caller ID configured for identity=" << identity.str() << " domain=" << domain.str());
    
    return !callerAlias.empty();
}