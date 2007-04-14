// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _MODIFYCALLERALIAS_H_
#define _MODIFYCALLERALIAS_H_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "AuthPlugin.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Changes From headers in INVITE-initiated dialogs according to the caller-alias database.
/**
 * This is an AuthRules plugin called by SipAaa::proxyMessage.
 *
 * The rules in the caller-alias database are used to rewrite the From field
 * for outgoing calls.
 *
 * Two parameters are set:
 * - Caller has the original value of the From header.
 *   In the event of a spiraled request, this value is not modified.
 * - Callee has the new value as determined by the caller-alias database.
 *   This may be rewritten in a spiraled request, so that only the final value
 *   is used (this is probably a pathological situation, so it is logged as a
 *   WARNING).
 *
 * This uses AuthPlugin::requestIsFromCaller to determine which
 * value should be placed in the From field.
 */
class ModifyCallerAlias : public AuthPlugin
{
  public:

   /// destructor
   virtual ~ModifyCallerAlias();

   /// Called for any request to be proxied.
   virtual
      AuthResult isAuthorized(const UtlString& id,    /**< The authenticated identity of the request
                                                       * originator, if any (the null string if not).
                                                       * This is in the form of a SIP uri identity
                                                       * value as used in the credentials database
                                                       * (user@domain) without any parameters.
                                                       */
                              const Url&  requestUri, ///< parsed target Uri
                              RouteState& routeState, ///< the recorde state for this request.  
                              SipMessage& request     ///< see below regarding modifying this
                              ) const = 0;
   /**<
    * This plugin always returns ALLOW_REQUEST; it's only effect, if any, is to
    * modify the From header and the accompanying RouteState.
    */

  protected:

  private:
   /// Constructor - private so that only the factory can call it.
   ModifyCallerAlias(const UtlString& instanceName ///< the configured name for this plugin instance
                   );

// @cond INCLUDENOCOPY
   
   /// There is no copy constructor.
   ModifyCallerAlias(const ModifyCallerAlias& nocopyconstructor);

   /// There is no assignment operator.
   ModifyCallerAlias& operator=(const ModifyCallerAlias& noassignmentoperator);
// @endcond INCLUDENOCOPY
};

#endif // _MODIFYCALLERALIAS_H_
