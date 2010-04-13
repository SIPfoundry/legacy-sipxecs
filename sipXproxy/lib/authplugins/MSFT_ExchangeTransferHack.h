// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _MSFT_EXCHANGETRANSFERHACK_H_
#define _MSFT_EXCHANGETRANSFERHACK_H_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "AuthPlugin.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MSFT_ExchangeTransferHackTest;

extern "C" AuthPlugin* getAuthPlugin(const UtlString& name);

/// Modifies an incorrectly constructed REFER from MSFT Exchange so that it works 
/**
 * The problem is that the Refer-To URI has a domain-part that is taken
 * from the callers Contact address rather than the domain of the proxy. If
 * that contact address refers to a B2BUA that can resolve it based just on
 * the user part, this works - but that is not going to be true in most
 * proxy-based configurations, including ours. For full details, see the
 * discussion in the microsoft forum url in the External Issue Reference.
 *
 * This AuthPlugin is a workaround; it recognizes when a REFER: 
 *
 * - uses the same domain in the request uri and the refer-to uri
 * - has a User-Agent header that starts matches what Exchange sends (configurable)
 *
 * having recognized this, it rewrites the domain of the Refer-To URI to be
 * the domain of the proxy.
 *
 * See XECS-872
 */
class MSFT_ExchangeTransferHack : public AuthPlugin
{
  public:

   /// destructor
   virtual ~MSFT_ExchangeTransferHack();

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
   friend class MSFT_ExchangeTransferHackTest;

   static const char* RecognizerConfigKey;

  private:
   friend AuthPlugin* getAuthPlugin(const UtlString& name);

   /// Constructor - private so that only the factory can call it.
   MSFT_ExchangeTransferHack(const UtlString& instanceName ///< the configured name for this plugin instance
                    );

   RegEx*   mUserAgentRegEx;
   SipRouter* mpSipRouter;

// @cond INCLUDENOCOPY
   /// There is no copy constructor.
   MSFT_ExchangeTransferHack(const MSFT_ExchangeTransferHack& nocopyconstructor);
   /// There is no assignment operator.
   MSFT_ExchangeTransferHack& operator=(const MSFT_ExchangeTransferHack& noassignmentoperator);
// @endcond INCLUDENOCOPY
};

#endif // _MSFT_EXCHANGETRANSFERHACK_H_
