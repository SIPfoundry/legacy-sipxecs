//
// Copyright (C) 2008 Nortel Networks, certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef SUBSCRIPTIONAUTH_H_
#define SUBSCRIPTIONAUTH_H_
// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "AuthPlugin.h"
#include <set>

// DEFINES
#define DIALOG_EVENT_TYPE "dialog"
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS
//class SubscriptionAuthTest;
class SipRouter;

extern "C" AuthPlugin* getAuthPlugin(const UtlString& name);

/// Plug-in responsible for forcing the authentication of SUBSCRIBE requests
/// for event packages that require authentication, which are listed
/// in the PACKAGES_REQUIRING_AUTHENTICATION configuration value
/// found in the sipXproxy-config configuration file.  The creation of 
/// this plug-in was prompted by XECS-1606.
/// 
/// Note that this plug-in will return CONTINUE for all SUBSCRIBE requests
/// that do not need to be challenged by this plug-in.  Such a behavior enables 
/// other Auth plug-ins in the chain to impose the challenge of such SUBSCRIBE 
/// requests if they wish to do so based on their authentication policies by
///  returning DENY.

/**
 * This is an AuthRules plugin called by SipRouter.
 */
class SubscriptionAuth : public AuthPlugin
{
  public:

   /// destructor
   virtual ~SubscriptionAuth();

   /// Enforces authentication of SUBSCRIBE requests
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

   virtual void announceAssociatedSipRouter( SipRouter* sipRouter );

  protected:
   friend class SubscriptionAuthTest;

  private:
   friend AuthPlugin* getAuthPlugin(const UtlString& name);
   static const char* EventsRequiringAuthenticationKey;
   static const char* TargetsExemptedFromAuthenticationKey;

   SipRouter* mpSipRouter;
   UtlSList   mEventPackagesRequiringAuthentication;
   UtlSList   mTargetsExemptedFromAuthentication;

   /// determines whether or not the target of the request is exempted from
   /// authentication based on the user part of the R-URI and the configured list
   /// of exempted users.
   UtlBoolean isTargetExemptedFromAuthentication(const UtlString& targetUser) const; //< user-part of the request target 

   /// Constructor - private so that only the factory can call it.
   SubscriptionAuth(const UtlString& instanceName ///< the configured name for this plugin instance
                    );

// @cond INCLUDENOCOPY
   /// There is no copy constructor.
   SubscriptionAuth(const SubscriptionAuth& nocopyconstructor);
   /// There is no assignment operator.
   SubscriptionAuth& operator=(const SubscriptionAuth& noassignmentoperator);
// @endcond INCLUDENOCOPY
};

#endif /* SUBSCRIPTIONAUTH_H_ */
