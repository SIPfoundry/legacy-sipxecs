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
class SipRouter;
class OsConfigDb;
class SipMessage;

/**
 * SIP Registrar Plugin Hook Action.
 *
 */

/// Record-Route based actions.
/**
 * An AuthPlugin is an action invoked by the sipXproxy whenever a
 * SIP Message is passing through.   See SipRouter for the context of this call.
 *
 * This class is the abstract base from which all AuthPlugins must inherit.
 *
 * To configure a AuthPlugin into the sipXproxy, the sipXproxy-config
 * file should have a directive configuring the plugin library:
 * @code
 * SIPX_PROXY_HOOK_LIBRARY.[instance] : [path to libexampleauthplugin.so]
 * @endcode
 * Where [instance] is replaced by a unique plugin name, and the value
 * points to the libary that provides the plugin code.
 *
 * In addition to the class derived from this base, a AuthPlugin library must
 * provide a factory routine named getAuthPlugin with extern "C" linkage so
 * that the OsSharedLib mechanism can look it up in the dynamically loaded library
 * (looking up C++ symbols is problematic because of name mangling).
 *
 * The decision making of the plugin is in the authorizeAndModify method.
 *
 * @see SipRouter
 * @see Plugin
 * @see PluginHooks
 */
class AuthPlugin : public Plugin
{
  public:

   static const char* Prefix;  ///< the configuration file prefix = "SIPX_PROXY"
   static const char* Factory; ///< the factory routine name = "getAuthPlugin"

   /// destructor
   virtual ~AuthPlugin() {};

   /// Action to be taken by the proxy.
   typedef enum 
   {
      CONTINUE, /**< this plugin neither authorizes nor forbids
                 *   (but may have modified) this message */
      DENY,     ///< this request is not authorized - do not proxy
      ALLOW,    ///< this request is authorized - proxy the message (possibly modified)
   } AuthResult;
      
   /// Called by SipRouter::proxyMessage for each request to authorize and/or modify before sending.
   virtual
      AuthResult authorizeAndModify(const UtlString& id,     /**< The authenticated identity of the
                                                              *   request originator, if any
                                                              *   (the null string if not).
                                                              *   This is in the form of a SIP uri
                                                              *   identity value as used in the
                                                              *   credentials database (user@domain)
                                                              *   without the scheme or any
                                                              *   parameters.
                                                              */
                                    const Url&  requestUri,  ///< parsed target Uri
                                    RouteState& routeState,  ///< the state for this request.  
                                    const UtlString& method, ///< the request method
                                    AuthResult  priorResult, ///< results from earlier plugins.
                                    SipMessage& request,     ///< see below regarding modifying this
                                    bool bSpiralingRequest,  ///< true if request is still spiraling through proxy
                                                             ///< false if request is ready to be sent to target
                                    UtlString&  reason       ///< rejection reason
                                    ) = 0;
   /**<
    * This method may do any combination of:
    *
    * - Determine whether or not the request is authorized, using 
    *   any characteristics of the message.  For any identity, it
    *   should use the given identity.  The result of any plugins
    *   that have been called is passed in priorResult - if the
    *   priorResult is not CONTINUE, then the authorization returned
    *   by this plugin will not be used (because the earlier result
    *   takes precedence); the plugin may use this fact to skip
    *   any authorization processing it would otherwise perform.
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
    *   The SipRouter pointer provides access to information in the calling
    *   sipXproxy; the most useful of these is probably SipRouter::isLocalDomain.
    *
    *   authorizeAndModify should usually record the fact that a
    *   dialog forming request is authorized in a RouteState
    *   parameter so that any subsequent in-dialog requests can be
    *   authorized by just looking at the state.
    *
    *   If the final result of the AuthPlugin chain is DENY for any
    *   request, the proxy responds with either:
    *   - '403 Forbidden' if the request was authenticated
    *      In this case, if the 'reason' parameter is not null when
    *      returned, it's value is used as the reason text in the
    *      first line of the response (if it is null, 'Forbidden'
    *      is sent).
    *   - '407 Proxy Authentication Required' if no valid user
    *     authentication was found.
    * 
    * NOTE about the bSpiralingRequest parameter:
    *   This parameter is set to true by the caller if the request is
    *   to be sent next to this proxy (spiraled), and false otherwise.
    *   Plug-ins that are only concerned with requests that will be
    *   sent to a target other than this proxy and are not concerned
    *   with intermediate requests that are still spiraling through
    *   sipXproxy can use this flag to only process those requests
    *   that have finished spiraling. 
    */

   /// Read (or re-read) whatever configuration the plugin requires.
   virtual void readConfig( OsConfigDb& configDb /**< a subhash of the individual configuration
                                                  * parameters for this instance of this plugin. */
                           ) = 0;
   
   /// Used to announce the SIP Router instance that is logically associated with this Auth Plugin.
   /// Plugins that need to interact with their associated SIP Router can override this method
   /// and save the passed pointer for later use. 
   virtual void announceAssociatedSipRouter( SipRouter* sipRouter ){};

   /**<
    * @note
    * The parent service may call the readConfig method at any time to
    * indicate that the configuration may have changed.  The plugin
    * should reinitialize itself based on the configuration that exists when
    * this is called.  The fact that it is a subhash means that whatever prefix
    * is used to identify the plugin (see PluginHooks) has been removed (see the
    * examples in PluginHooks::readConfig).
    */

   /// Provide a string version of an AuthResult value for logging. 
   static const char* AuthResultStr(AuthResult result);
   
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
