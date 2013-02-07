// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <set>

#include <boost/algorithm/string.hpp>
#include "sipdb/EntityDB.h"
#include <sipxproxy/SipRouter.h>
#include "os/OsReadLock.h"
#include "os/OsWriteLock.h"
#include "os/OsConfigDb.h"
#include "os/OsLogger.h"
#include "os/OsFS.h"
#include "sipXecsService/SipXecsService.h"
#include "sipXecsService/SharedSecret.h"

// APPLICATION INCLUDES
#include "EnforceAuthRules.h"


// DEFINES
// CONSTANTS
const char DEFAULT_AUTH_RULES_FILENAME[] = "authrules.xml";
const char RULES_FILENAME_CONFIG_PARAM[] = "RULES";
const char IDENTITY_VALIDITY_CONFIG_NAME[] = "IDENTITY_VALIDITY_SECONDS";

const unsigned int DefaultSignatureValiditySeconds = 10;

// TYPEDEFS
// FORWARD DECLARATIONS

/// Factory used by PluginHooks to dynamically link the plugin instance
extern "C" AuthPlugin* getAuthPlugin(const UtlString& pluginName)
{
   return new EnforceAuthRules(pluginName);
}

/// constructor
EnforceAuthRules::EnforceAuthRules(const UtlString& pluginName ///< the name for this instance
                                   )
   : AuthPlugin(pluginName)
   , mRulesLock(OsRWMutex::Q_FIFO)
   , mpAuthorizationRules(NULL),
    mpSipRouter(0),
    mpEntityDb(0)
{
    Os::Logger::instance().log(FAC_SIP,PRI_INFO,"EnforceAuthRules plugin instantiated '%s'",
                 mInstanceName.data());
	MongoDB::ConnectionInfo info(MongoDB::ConnectionInfo::connectionStringFromFile(), EntityDB::NS);
	mpEntityDb = new EntityDB(info);
};

/// Read (or re-read) the authorization rules.
void
EnforceAuthRules::readConfig( OsConfigDb& configDb /**< a subhash of the individual configuration
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
   Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, "EnforceAuthRules[%s]::readConfig",
                 mInstanceName.data()
                 );
   OsWriteLock writeLock(mRulesLock);

#if 0
   int validitySeconds = 0;
   if (OS_NOT_FOUND==configDb.get(IDENTITY_VALIDITY_CONFIG_NAME, validitySeconds))
   {
      validitySeconds = DefaultSignatureValiditySeconds;
      Os::Logger::instance().log(FAC_SIP, PRI_INFO, "EnforceAuthRules[%s]::readConfig "
                    "no value found for %s: defauted to '%d' seconds",
                    mInstanceName.data(), IDENTITY_VALIDITY_CONFIG_NAME, validitySeconds
                    );
   }
   else
   {
      Os::Logger::instance().log(FAC_SIP, PRI_INFO, "EnforceAuthRules[%s]::readConfig "
                    "set SipXauthIdentity validity interval to '%d' seconds",
                    mInstanceName.data(), validitySeconds
                    );
   }
   // Set signature validity interval for SipXauthIdentity
   SipXauthIdentity::setSignatureValidityInterval(OsTime(validitySeconds,0));
#endif // 0

   if (mpAuthorizationRules)
   {
      delete mpAuthorizationRules;
   }

   mpAuthorizationRules = new AuthRulesUrlMapping();
   
   UtlString fileName;
   if ( ! configDb.get(RULES_FILENAME_CONFIG_PARAM, fileName) )
   {
      OsPath defaultPath =
         SipXecsService::Path(SipXecsService::ConfigurationDirType, DEFAULT_AUTH_RULES_FILENAME);

      fileName = defaultPath;
      
      Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, "EnforceAuthRules[%s]::readConfig "
                    " no rules file configured; trying '%s'",
                    mInstanceName.data(), fileName.data()
                    );
   }

   if (OS_SUCCESS == mpAuthorizationRules->loadMappings(fileName))
   {
      Os::Logger::instance().log(FAC_SIP, PRI_INFO, "EnforceAuthRules[%s]::readConfig "
                    " successfully loaded rules file '%s'.",
                    mInstanceName.data(), fileName.data()
                    );
   }
   else
   {
      Os::Logger::instance().log(FAC_SIP, PRI_ERR, "EnforceAuthRules[%s]::readConfig "
                    " error loading rules file '%s': enforcement disabled.",
                    mInstanceName.data(), fileName.data()
                    );
      delete mpAuthorizationRules;
      mpAuthorizationRules = NULL;
   }
}

AuthPlugin::AuthResult
EnforceAuthRules::authorizeAndModify(const UtlString& id,    /**< The authenticated identity of the
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
      
   /*
    * This plugin uses one state parameter to record that the dialog has already been
    * processed by authrules; it does not have any value.  If that parameter is present
    * in a request and routeState::isMutable returns false, then this method does not
    * evaluate the authrules, it just returns ALLOW.
    *
    * If the state parameter is not found, or routeState::isMutable returns true (indicating
    * that this not an in-dialog request), then the authrules are evaluated.
    *
    * - If the rules allow the request, the state parameter is set to record that
    *   and ALLOW is returned.
    * - If the rules do not allow the request, this method sets the reason phrase
    *   to describe what permission is needed for the request to succeed and returns
    *   DENY.
    */

   UtlString unused;
   
   // get the call-id to use in logging
   UtlString callId;
   request.getCallIdField(&callId);

   if (priorResult == CONTINUE)
   {
      OsReadLock readLock(mRulesLock);

      if (mpAuthorizationRules)
      {
         ResultSet  requiredPermissions;
         mpAuthorizationRules->getPermissionRequired(requestUri, requiredPermissions);

         if (requiredPermissions.isEmpty())
         {
            result = ALLOW;
            Os::Logger::instance().log(FAC_AUTH, PRI_INFO, "EnforceAuthRules[%s]::authorizeAndModify "
                          " no permission required for call %s",
                          mInstanceName.data(), callId.data()

            if (!mpSipRouter->isRelayAllowed())
            {
              Url fromUri;
              request.getFromUrl(fromUri);
              if (mpSipRouter && !mpSipRouter->isLocalDomain(fromUri, true))
              {
                //
                // Checking if it wants to relay
                //
                if (!mpSipRouter->isLocalDomain(requestUri, true))
                {
                  return DENY;
                }
              }
            }
         }
         else if (id.isNull())
         {
            /*
             * Some permission is required, but we cannot look up permissions without
             * an identity.  Let this request fail, which will cause a challenge.
             * When there is no identity, good security practice dictates that we _not_
             * supply any information about what is needed, so do not set the reason.
             */
            result = DENY;
            Os::Logger::instance().log(FAC_AUTH, PRI_DEBUG, "EnforceAuthRules[%s]::authorizeAndModify "
                          " request not authenticated but requires some permission. Call-Id = '%s'",
                          mInstanceName.data(), callId.data() 
                          );
         }
         else
         {
            // some permission is required and caller is authenticated, so see if they have it

            

            UtlString unmatchedPermissions;
            UtlString matchedPermission;
            if (isAuthorized(id, requiredPermissions,
                             matchedPermission, unmatchedPermissions)
                )
            {
               result = ALLOW;
               Os::Logger::instance().log(FAC_AUTH, PRI_DEBUG, "EnforceAuthRules[%s]::authorizeAndModify "
                             " id '%s' authorized by '%s'",
                             mInstanceName.data(), id.data(), matchedPermission.data()
                             );
            }
            else
            {
               result = DENY;
               Os::Logger::instance().log(FAC_AUTH, PRI_WARNING,
                             "EnforceAuthRules[%s]::authorizeAndModify "
                             " call '%s' requires '%s'",
                             mInstanceName.data(), callId.data(), unmatchedPermissions.data()
                             );
               // since the user is at least a valid user, help them debug the configuration
               // by telling them what permissions would allow this request.
               reason.append("Requires ");
               reason.append(unmatchedPermissions);
            }
         }
      }
      else
      {
         // no rules configured, so all requests are allowed
         result = CONTINUE;
      }
   }
   else
   {
      // another plug-in already provided an authoritative result for this request so
      // don't waste time figuring it out.
      Os::Logger::instance().log(FAC_AUTH, PRI_DEBUG, "EnforceAuthRules[%s]::authorizeAndModify "
                    "prior authorization result %s for call %s - rules skipped",
                    mInstanceName.data(), AuthResultStr(priorResult), callId.data()
                    );
   }
   
   return result;
}

/// @returns true iff at least one permission in grantedPermissions is in requiredPermissions
bool EnforceAuthRules::isAuthorized(const UtlString& id,
     const ResultSet& requiredSet,
     UtlString& matchedPermission,   ///< first required permission found
     UtlString& unmatchedPermissions ///< requiredPermissions as a single string
 )
{
    if (id.isNull())
      return false;

    bool authorized = false;
    matchedPermission.remove(0);
    unmatchedPermissions.remove(0);

    OS_LOG_INFO(FAC_SIP, "EnforceAuthRules::isAuthorized - EntityDB::findByIdentity");

    EntityRecord entity;
    std::string identity = id.str();
    if (!mpEntityDb->findByIdentity(identity, entity))
        return false;

    std::set<std::string> grantedPermissions = entity.permissions();
    std::set<std::string> requiredPermissions;
    UtlSListIterator requiredRecs(requiredSet);
    UtlHashMap* requiredRecord;
    UtlString permissionKey("permission");
    while((requiredRecord = dynamic_cast<UtlHashMap*>(requiredRecs())))
    {
      UtlString* reqPermission = dynamic_cast<UtlString*>(requiredRecord->findValue(&permissionKey));
      if (reqPermission && !reqPermission->isNull())
      {
         requiredPermissions.insert(reqPermission->data());
      }
    }

    if (requiredPermissions.size() == 0)
      return true;

    for (std::set<std::string>::const_iterator iter = requiredPermissions.begin();
        iter != requiredPermissions.end(); iter++)
    {
        std::string permission = *iter;
        boost::to_lower(permission);
        if (permission == "validuser")
        {
            authorized = true;
            if (!matchedPermission.isNull())
            {
               matchedPermission.append("+");
            }
            matchedPermission.append(iter->c_str());
        }
        else
        {
            std::set<std::string>::const_iterator grantedIter;
            grantedIter = grantedPermissions.find(*iter);
            if (grantedIter != grantedPermissions.end())
            {
                authorized = true;
                if (!matchedPermission.isNull())
                {
                   matchedPermission.append("+");
                }
                matchedPermission.append(iter->c_str());
            }
            else
            {
                if (!unmatchedPermissions.isNull())
                {
                   unmatchedPermissions.append("|");
                }
                unmatchedPermissions.append(iter->c_str());
            }
        }
        
    }

    return authorized;
}

/// destructor
EnforceAuthRules::~EnforceAuthRules()
{
   { // write lock scope
      OsWriteLock writeLock(mRulesLock);
      if (mpAuthorizationRules)
      {
         delete mpAuthorizationRules;
         mpAuthorizationRules = NULL;
      }
   }
   if (mpEntityDb != NULL)
   {
	   delete mpEntityDb;
	   mpEntityDb = NULL;
   }
}


void EnforceAuthRules::announceAssociatedSipRouter(SipRouter* sipRouter)
{
    mpSipRouter = sipRouter;
}
