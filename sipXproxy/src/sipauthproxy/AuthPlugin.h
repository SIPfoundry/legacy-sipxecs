// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _AUTHPLUGIN_H_
#define _AUTHPLUGIN_H_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "utl/UtlString.h"
#include "utl/Plugin.h"
#include "RouteState.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipAaa;
class OsConfigDb;
class SipMessage;

/**
 * SIP Registrar Plugin Hook Action.
 *
 */

/// Record-Route based actions.
/**
 * An AuthPlugin is an action invoked by the sipXauthproxy whenever a
 * SIP Request is passing through.   See SipAaa for the context of this call.
 *
 * This class is the abstract base from which all AuthPlugins must inherit.
 *
 * To configure a AuthPlugin into the sipauthproxy, the authproxy-config
 * file should have a directive configuring the plugin library:
 * @code
 * SIP_AUTHPROXY_HOOK_LIBRARY.[instance] : [path to libexampleauthplugin.so]
 * @endcode
 * Where [instance] is replaced by a unique plugin name, and the value
 * points to the libary that provides the plugin code.
 *
 * In addition to the class derived from this base, a AuthPlugin library must
 * provide a factory routine named getAuthPlugin with extern "C" linkage so
 * that the OsSharedLib mechanism can look it up in the dynamically loaded library
 * (looking up C++ symbols is problematic because of name mangling).
 *
 * @see Plugin
 * @see PluginHooks
 */
class AuthPlugin : public Plugin
{
  public:

   static const char* Prefix;  ///< the configuration file prefix = "SIP_AUTHPROXY"
   static const char* Factory; ///< the factory routine name = "getAuthPlugin"

   /// destructor
   virtual ~AuthPlugin() {};

   /// Action to be taken by the proxy.
   typedef enum 
   {
      ALLOW_REQUEST,   ///< proxy message as it exists (possibly modified)
      UNAUTHORIZED     ///< request not authorized - do not proxy
   } AuthResult;
      
   /// Called by SipAaa::proxyMessage for each request to authorize and/or modify before sending.
   virtual
      AuthResult authorizeAndModify(const SipAaa* sipAaa,  ///< for access to proxy information
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
                                    SipMessage& request,    ///< see below regarding modifying this
                                    UtlString&  reason      ///< rejection reason
                                    ) = 0;
   /**<
    * This method may do any combination of:
    *
    * - Determine whether or not the request is authorized for
    *   the given identity.  If not, it should return UNAUTHORIZED.
    *   If any plugin returns UNAUTHORIZED, no further plugins
    *   are consulted; the proxy immediately returns either:
    *   - '403 Forbidden' if the request was authenticated
    *      In this case, if the 'reason' parameter is not null when
    *      returned, it's value is used as the reason text in the
    *      first line of the response (if it is null, 'Forbidden'
    *      is sent).
    *   - '407 Proxy Authentication Required' if no valid user
    *     authentication was found.
    *
    * - Modify the message.
    *   This should be done with great care, as it can introduce
    *   interoperability problems.  
    *   The plugin MUST NOT:
    *   - Modify the request line, including the target URI
    *   - Add a Record-Route header of its own (see below).
    *   - Create any persistent state other than modifying
    *     the routeState as passed in (At present there is no
    *     plugin interface defined to clean up such internal
    *     state.  This is because it represents a potential DoS
    *     attack - the attacker just sends requests that will
    *     eventually fail, but which create lots of state.
    *     Until we are confident that we can prevent this _and_
    *     see a real need for such internal state, this restriction
    *     applies).
    *   
    * - Record state to be retained for the dialog in routeState
    *   Modifying the routeState will work only if the request is creating a new
    *   dialog, because the value is saved in a url parameter of
    *   the route added by this proxy in its Record-Route; for
    *   a dialog-forming request, the saved state becomes a
    *   part of the route set for the dialog and is available
    *   in the Route header of subsequent in-dialog requests,
    *   but in an in-dialog request the Record-Route is ignored.
    *   The plugin can use RouteState::isMutable to find out
    *   whether or not the state can be modified.
    *
    *   An AuthPlugin MUST NOT call RouteState::update (it is
    *   called by the authproxy after all plugins have completed).
    *
    *   The sipAaa pointer provides access to information in the calling
    *   authproxy; the most useful of these is probably SipAaa::isLocalDomain.
    *
    *   authorizeAndModify should usually record the fact that a
    *   dialog forming request is authorized in a RouteState
    *   parameter so that any subsequent in-dialog requests can be
    *   authorized by just looking at the state.
    */

   /// Read (or re-read) whatever configuration the plugin requires.
   virtual void readConfig( OsConfigDb& configDb /**< a subhash of the individual configuration
                                                  * parameters for this instance of this plugin. */
                           ) = 0;
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

   /// constructor
   AuthPlugin(const UtlString& instanceName ///< the configured name for this plugin instance
              ) :
      Plugin(instanceName)
   {
   };
    

  private:
// @cond INCLUDENOCOPY
   
      /// There is no copy constructor.
      AuthPlugin(const AuthPlugin& nocopyconstructor);

      /// There is no assignment operator.
      AuthPlugin& operator=(const AuthPlugin& noassignmentoperator);
// @endcond INCLUDENOCOPY

};

#endif // _AUTHPLUGIN_H_
