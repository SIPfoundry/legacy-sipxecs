// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//////////////////////////////////////////////////////////////////////////////
#ifndef _CALLERALIAS_H_
#define _CALLERALIAS_H_

// SYSTEM INCLUDES
#include "os/OsMutex.h"

// APPLICATION INCLUDES
#include "AuthPlugin.h"
#include "sipdb/CallerAliasDB.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class CallerAliasTest;

extern "C" AuthPlugin* getAuthPlugin(const UtlString& name);

/// Modify the From header of a request based on lookup in the caller-alias database.
/**
 * This AuthPlugin provides a caller aliasing feature by modifying the From header
 * in an initial request and maintaining the translation throughout the dialog.
 * It is intended to provide a way to modify the caller-id presented in a PSTN call.
 *
 * The translation is controlled by entries in the caller-alias sipdb, which has two key
 * columns: the caller identity and the target domain; and one output column: the From header
 * field value.
 *
 * On an initial request, the first test is for an exact match of the identity (user@domain,
 * with no parameters) in the From header of the request and the target domain (from the
 * request URI).  Note that the target domain for a PSTN call is the domain of some gateway
 * with an unique identifier "sipxecs-lineid" . The gateway could be a local device or a
 * remote SIP Trunk service.  The lookup is _not_ * affected by any Route on the request,
 * so a request to an ITSP that is Routed through a local SBC matches on the ITSP domain,
 * not that of the SBC.
 *
 * If an exact match is found, then the resulting From header from the output column
 * is substituted.  This is used to replace the identity of a specific user when calling
 * a specific domain.
 *
 * If no exact match is found, and if the existing From header identity is a user in the
 * domain of this proxy, then a second check is made in the caller-alias database for a
 * "wildcard" match: the caller identity used for this lookup is "*", and the target domain
 * from the request URI (as in the exact match test).  If this matches a row in the caller-alias
 * database, the From value from that row is substituted.  Note the exclusion: this wildcard
 * match is not attempted if the caller is not within the domain, so a call coming from outside
 * the domain that is forwarded by a local user is not aliased (the original outside
 * caller-id is maintained).
 *
 * If neither match finds a row in the caller-alias database, the From header is left unchanged.
 *
 * When a From header value is substituted, the entire value from the table is used, and the only
 * part of the original From header value that is preserved is the 'tag' field parameter.
 * Both the full original From header value and the aliased From header value are added to the
 * RouteState for the request.  This causes these values to be preserved in the route set.  As
 * subsequent requests and responses follow the route set through this proxy, this plugin
 * modifies the From header using these saved values so that each participant in the dialog
 * sees only the From header value they expect to see (the caller sees only the original value,
 * and the called party sees only the aliased value).
 */
class CallerAlias : public AuthPlugin
{
  public:

   virtual ~CallerAlias();

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

   /// Called for any request - provides caller alias facility.
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
   ///< See class description.

   virtual void announceAssociatedSipRouter( SipRouter* sipRouter );
   
  protected:
   friend class CallerAliasTest;
   friend class SipRouterTest;

   static OsMutex        sSingletonLock;  ///< lock to protext the spInstance
   static CallerAlias*   spInstance;      ///< only one CallerAlias may be configured
   static CallerAliasDB* spCallerAliasDB; ///< database instance handle

   static const char* CALLER_FROM_PARAM; ///< route parameter name for the original From header
   static const char* ALIAS_FROM_PARAM;  ///< route parameter name for the aliased From header
   static const char* CALLER_TAG_OFFSET_PARAM; ///< route parameter name for the tag offset in From
   static const char* ALIAS_TAG_OFFSET_PARAM; ///< route parameter name for the tag offset in To
   static const char* ALIAS_URI_SIPXECS_LINEID_PARAM; ///<internal URI parameter to identify the gateway chosen by call routing.
   
  private:
   friend AuthPlugin* getAuthPlugin(const UtlString& name);

   CallerAlias(const UtlString& instanceName ///< the configured name for this plugin instance
              );

   SipRouter* mpSipRouter;
};

#endif // _CALLERALIAS_H_
