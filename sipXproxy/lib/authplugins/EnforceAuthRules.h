// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _ENFORCEAUTHRULES_H_
#define _ENFORCEAUTHRULES_H_

// SYSTEM INCLUDES
#include "os/OsRWMutex.h"

// APPLICATION INCLUDES
#include "sipdb/ResultSet.h"
#include "digitmaps/AuthRulesUrlMapping.h"
#include "AuthPlugin.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS
class EnforceAuthRulesTest;

extern "C" AuthPlugin* getAuthPlugin(const UtlString& name);

/// Enforces the rules specified by authrules.xml
/**
 * This is an AuthRules plugin called by SipRouter to enforce rules based on the
 * target uri and the permissions of the caller.
 */
class EnforceAuthRules : public AuthPlugin
{
  public:

   /// destructor
   virtual ~EnforceAuthRules();

   /// Called for any request - enforces the restrictions specified by authrules.
   virtual
      AuthResult authorizeAndModify(const UtlString& id,    /**< The authenticated identity of the
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
                                    bool bSpiralingRequest, ///< request spiraling indication 
                                    UtlString&  reason      ///< rejection reason
                                    );
   /**<
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

   /// Read (or re-read) the authorization rules.
   virtual void readConfig( OsConfigDb& configDb /**< a subhash of the individual configuration
                                                  * parameters for this instance of this plugin. */
                           );
   /**<
    * @note
    * The parent service may call the readConfig method at any time to
    * indicate that the configuration may have changed.  The plugin
    * should reinitialize itself based on the configuration that exists when
    * this is called.  The fact that it is a subhash means that whatever prefix
    * is used to identify the plugin (see PluginHooks) has been removed (see the
    * examples in PluginHooks::readConfig).
    */

  protected:
   friend class EnforceAuthRulesTest;
   
   /// @returns true iff at least one permission in grantedPermissions is in requiredPermissions
   bool isAuthorized(const ResultSet& requiredPermissions,
                     const ResultSet& grantedPermissions,
                     UtlString& matchedPermission,   ///< first required permission found
                     UtlString& unmatchedPermissions ///< requiredPermissions as a single string
                     );

  private:
   friend AuthPlugin* getAuthPlugin(const UtlString& name);

   /// Constructor - private so that only the factory can call it.
   EnforceAuthRules(const UtlString& instanceName ///< the configured name for this plugin instance
                    );

   OsRWMutex            mRulesLock;
   AuthRulesUrlMapping* mpAuthorizationRules;

// @cond INCLUDENOCOPY
   
   /// There is no copy constructor.
   EnforceAuthRules(const EnforceAuthRules& nocopyconstructor);

   /// There is no assignment operator.
   EnforceAuthRules& operator=(const EnforceAuthRules& noassignmentoperator);
// @endcond INCLUDENOCOPY
};

#endif // _ENFORCEAUTHRULES_H_
