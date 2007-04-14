// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include "os/OsReadLock.h"
#include "os/OsWriteLock.h"
#include "os/OsConfigDb.h"
#include "os/OsSysLog.h"
#include "os/OsFS.h"

// APPLICATION INCLUDES
#include "EnforceAuthRules.h"
#include "sipdb/CredentialDB.h"
#include "sipdb/PermissionDB.h"
#include "sipdb/AuthexceptionDB.h"

// DEFINES
// CONSTANTS
const char DEFAULT_AUTH_RULES_FILENAME[] = "authrules.xml";
const char RULES_FILENAME_CONFIG_PARAM[] = "RULES";
const char RULES_ENFORCED_ROUTE_PARAM[] = "auth";

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
   , mpAuthorizationRules(NULL)
{
   OsSysLog::add(FAC_SIP,PRI_INFO,"EnforceAuthRules plugin instantiated '%s'",
                 mInstanceName.data());
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
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "EnforceAuthRules[%s]::readConfig",
                 mInstanceName.data()
                 );
   OsWriteLock writeLock(mRulesLock);

   if (mpAuthorizationRules)
   {
      delete mpAuthorizationRules;
   }

   mpAuthorizationRules = new UrlMapping();
   
   UtlString fileName;
   if ( ! configDb.get(RULES_FILENAME_CONFIG_PARAM, fileName) )
   {
      OsPath workingDirectory ;
      if ( OsFileSystem::exists(SIPX_CONFDIR) )
      {
         workingDirectory = SIPX_CONFDIR;
         OsPath path(workingDirectory);
         path.getNativePath(workingDirectory);

      } else
      {
         OsPath path;
         OsFileSystem::getWorkingDirectory(path);
         path.getNativePath(workingDirectory);
      }
      fileName = workingDirectory + OsPathBase::separator + DEFAULT_AUTH_RULES_FILENAME;

      OsSysLog::add(FAC_SIP, PRI_DEBUG, "EnforceAuthRules[%s]::readConfig "
                    " no rules file configured; trying '%s'",
                    mInstanceName.data(), fileName.data()
                    );
   }

   if (OS_SUCCESS == mpAuthorizationRules->loadMappings(fileName))
   {
      OsSysLog::add(FAC_SIP, PRI_INFO, "EnforceAuthRules[%s]::readConfig "
                    " successfully loaded rules file '%s'.",
                    mInstanceName.data(), fileName.data()
                    );
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_ERR, "EnforceAuthRules[%s]::readConfig "
                    " error loading rules file '%s': enforcement disabled.",
                    mInstanceName.data(), fileName.data()
                    );
      delete mpAuthorizationRules;
      mpAuthorizationRules = NULL;
   }
}

AuthPlugin::AuthResult
EnforceAuthRules::authorizeAndModify(const SipAaa* sipAaa,  ///< for access to proxy information
                                     const UtlString& id, /**< The authenticated identity of the
                                                           *   request originator, if any (the null
                                                           *   string if not).
                                                           *   This is in the form of a SIP uri
                                                           *   identity value as used in the
                                                           *   credentials database (user@domain)
                                                           *   without the scheme or any parameters.
                                                           */
                                     const Url&  requestUri, ///< parsed target Uri
                                     RouteState& routeState, ///< the state for this request.  
                                     SipMessage& request,    ///< see AuthPlugin regarding modifying
                                     UtlString&  reason      ///< rejection reason
                                     )
{
   AuthResult result = UNAUTHORIZED;
   
   /*
    * This plugin uses one state parameter to record that the dialog has already been
    * processed by authrules; it does not have any value.  If that parameter is present
    * in a request and routeState::isMutable returns false, then this method does not
    * evaluate the authrules, it just returns ALLOW_REQUEST.
    *
    * If the state parameter is not found, or routeState::isMutable returns true (indicating
    * that this not an in-dialog request), then the authrules are evaluated.
    *
    * - If the rules allow the request, the state parameter is set to record that
    *   and ALLOW_REQUEST is returned.
    * - If the rules do not allow the request, this method sets the reason phrase
    *   to describe what permission is needed for the request to succeed and returns
    *   UNAUTHORIZED.
    */

   UtlString unused;
   
   // get the call-id to use in logging
   UtlString callId;
   request.getCallIdField(&callId);

   if (!routeState.getParameter(mInstanceName.data(), RULES_ENFORCED_ROUTE_PARAM, unused))
   {
      UtlString userid;
      requestUri.getUserId(userid);

      if ( ! AuthexceptionDB::getInstance()->isException( userid ) )
      {
         OsReadLock readLock(mRulesLock);

         if (mpAuthorizationRules && !request.isResponse())
         {
            UtlString method;
            request.getRequestMethod(&method);

            if (method.compareTo(SIP_ACK_METHOD) != 0) // don't authenticate ACK
            {
               ResultSet  requiredPermissions;
               mpAuthorizationRules->getPermissionRequired(requestUri, requiredPermissions);
         
               if (requiredPermissions.isEmpty())
               {
                  OsSysLog::add(FAC_AUTH, PRI_INFO, "EnforceAuthRules[%s]::authorizeAndModify "
                                " no permission required for call %s",
                                mInstanceName.data(), callId.data()
                                );
                  result = ALLOW_REQUEST;
               }
               else if (id.isNull())
               {
                  /*
                   * Some permission is required, but we cannot look up permissions without
                   * an identity.  Let this request fail, which will cause a challenge.
                   * When there is no identity, good security practice dictates that we _not_
                   * supply any information about what is needed, so do not set the reason.
                   */
                  OsSysLog::add(FAC_AUTH, PRI_DEBUG, "EnforceAuthRules[%s]::authorizeAndModify "
                                " request not authenticated but requires some permission: %s",
                                mInstanceName.data(), callId.data() 
                                );
               }
               else
               {
                  // some permission is required and caller is authenticated, so see if they have it
                  ResultSet grantedPermissions;
                  Url identity(id);
                  PermissionDB::getInstance()->getPermissions(identity, grantedPermissions);

                  UtlString unmatchedPermissions;
                  UtlString matchedPermission;

                  if (isAuthorized(requiredPermissions, grantedPermissions,
                                   matchedPermission, unmatchedPermissions)
                      )
                  {
                     OsSysLog::add(FAC_AUTH, PRI_DEBUG, "EnforceAuthRules[%s]::authorizeAndModify "
                                   " id '%s' authorized by '%s'",
                                   mInstanceName.data(), id.data(), matchedPermission.data()
                                   );

                     result = ALLOW_REQUEST;
                  }
                  else
                  {
                     OsSysLog::add(FAC_AUTH, PRI_WARNING,
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
               // this is an ACK
               result = ALLOW_REQUEST;
            }
         }
         else
         {
            // no rules configured, so all requests are allowed
            result = ALLOW_REQUEST;
         }
      }
      else
      {
         // requestUri was found in the authexceptions db, so let it go
         OsSysLog::add(FAC_AUTH, PRI_DEBUG, "EnforceAuthRules[%s]::authorizeAndModify "
                       " target '%s' allowed as an exception for %s",
                       mInstanceName.data(), userid.data(), callId.data()
                       );
         result = ALLOW_REQUEST;
      }

      if (   ALLOW_REQUEST == result
          && routeState.isMutable()
          )
      {
         routeState.setParameter(mInstanceName.data(),RULES_ENFORCED_ROUTE_PARAM,unused);
      }
   }
   else
   {
      /*
       * There is a valid RULES_ENFORCED_ROUTE_PARAM on this request,
       * so there is no need to check it.
       */
      OsSysLog::add(FAC_AUTH, PRI_DEBUG, "EnforceAuthRules[%s]::authorizeAndModify "
                    "valid previous authorization found for call %s",
                    mInstanceName.data(), callId.data()
                    );
      result = ALLOW_REQUEST;
   }
   
   return result;
}

bool EnforceAuthRules::isAuthorized(
   const ResultSet& requiredPermissions,
   const ResultSet& grantedPermissions,
   UtlString& matchedPermission,
   UtlString& unmatchedPermissions)
{
   bool authorized = false;
   UtlString identityKey("identity");
   UtlString permissionKey("permission");

   matchedPermission.remove(0);
   unmatchedPermissions.remove(0);

   /*
    * The ResultSet structure is inefficient to search repeatedly,
    * so walk the grantedPermissions once, saving a pointer to each
    * of the permissions in the temporary list 'thisUserPermissions'.
    * NOTE: the UtlString is still 'owned' by the ResultSet, so do not
    * destroy anything from thisUserPermissions
    */
   UtlSList thisUserPermissions;
   UtlSListIterator grantedRecords(grantedPermissions);
   UtlHashMap* grantedRecord;
   while((grantedRecord = dynamic_cast<UtlHashMap*>(grantedRecords())))
   {      
      UtlString* grantedPermission
         = dynamic_cast<UtlString*>(grantedRecord->findValue(&permissionKey));
      if (grantedPermission)
      {
         thisUserPermissions.insert(grantedPermission);
      }
   }
   
   // thisUserPermissions now has an easily searched list of the permissions for this user.
   
   UtlSListIterator requiredRecords(requiredPermissions);
   UtlHashMap* requiredRecord;
   while((requiredRecord = dynamic_cast<UtlHashMap*>(requiredRecords())))
   {
      UtlString requiredPermission
         = *dynamic_cast<UtlString*>(requiredRecord->findValue(&permissionKey));

      /*
       * Special case - this routine is only called for authenticated users,
       *                and all authenticated users have 'ValidUser' permission,
       *                so this is a pass.
       */
      if (requiredPermission.compareTo("ValidUser", UtlString::ignoreCase) == 0)
      {
         authorized = true;
         if (!matchedPermission.isNull())
         {
            matchedPermission.append("+");
         }
         matchedPermission.append(requiredPermission);
      }
      else
      {
         // normal check to see if the permission is granted to this user
         if (thisUserPermissions.contains(&requiredPermission))
         {
            authorized = true;
            if (!matchedPermission.isNull())
            {
               matchedPermission.append("+");
            }
            matchedPermission.append(requiredPermission);
         }
         else
         {
            if (!unmatchedPermissions.isNull())
            {
               unmatchedPermissions.append("|");
            }
            unmatchedPermissions.append(requiredPermission);
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
}
