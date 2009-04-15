// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _TRANSFERCONTROL_H_
#define _TRANSFERCONTROL_H_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "AuthPlugin.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS
class TransferControlTest;
class SipRouter;

extern "C" AuthPlugin* getAuthPlugin(const UtlString& name);

/// Enforces the rules specified by authrules.xml
/**
 * This is an AuthRules plugin called by SipRouter.
 * It's job is to make exceptions for transfers that might otherwise
 * be prevented by the other authorization rules.
 */
class TransferControl : public AuthPlugin
{
  public:

   /// destructor
   virtual ~TransferControl();

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
   friend class TransferControlTest;
   
  private:
   friend AuthPlugin* getAuthPlugin(const UtlString& name);
   
   SipRouter* mpSipRouter;

   /// Constructor - private so that only the factory can call it.
   TransferControl(const UtlString& instanceName ///< the configured name for this plugin instance
                    );

// @cond INCLUDENOCOPY
   /// There is no copy constructor.
   TransferControl(const TransferControl& nocopyconstructor);
   /// There is no assignment operator.
   TransferControl& operator=(const TransferControl& noassignmentoperator);
// @endcond INCLUDENOCOPY
};

#endif // _TRANSFERCONTROL_H_
