//
// Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _EMERGENCYNOTIFY_H_
#define _EMERGENCYNOTIFY_H_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "AuthPlugin.h"
#include "digitmaps/EmergencyRulesUrlMapping.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS
class EmergencyNotifyTest;

extern "C" AuthPlugin* getAuthPlugin(const UtlString& name);

/**
 * This is an AuthRules plugin called by SipRouter.
 * Its job is to generate a notification when an emergency number is dialled.
 */
class EmergencyNotify : public AuthPlugin
{
  public:

   /// destructor
   virtual ~EmergencyNotify();

   /// Called for any request - generates a notification when an emergency number is dialled
   virtual
      AuthResult authorizeAndModify(const UtlString& id, /**< The authenticated identity of the
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
                                    bool bSpiralingRequest,
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

  protected:
   friend class EmergencyRulesTest;
   bool getMatchedRule(const Url& requestUri,                 ///< target to check
                                  UtlString& rNameStr,        ///< name of the rule that matched
                                  UtlString& rDescriptionStr  ///< description of the rule that matched
                                  ) const;

  private:
   friend AuthPlugin* getAuthPlugin(const UtlString& name);

   /// Constructor - private so that only the factory can call it.
   EmergencyNotify(const UtlString& instanceName ///< the configured name for this plugin instance
                    );

   OsRWMutex   mRulesLock;
   EmergencyRulesUrlMapping* mpEmergencyRules;

// @cond INCLUDENOCOPY
   /// There is no copy constructor.
   EmergencyNotify(const EmergencyNotify& nocopyconstructor);
   /// There is no assignment operator.
   EmergencyNotify& operator=(const EmergencyNotify& noassignmentoperator);
// @endcond INCLUDENOCOPY
};

#endif // _EMERGENCYNOTIFY_H_
