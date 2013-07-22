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
#include <sipxproxy/AuthPlugin.h>

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
   
   /// Boolean indicator that returns true if the plugin wants to process requests
   /// that requires no authentication
   virtual bool willModifyTrustedRequest() const;
   
   /// This method is called by the proxy if willModifyRequest() flag is set to true
   /// giving this plugin the opportunity to modify the request even if it requires
   /// no authentication
   virtual void modifyTrustedRequest(
                                    const Url&  requestUri,  ///< parsed target Uri
                                    SipMessage& request,     ///< see below regarding modifying this
                                    bool bSpiralingRequest  ///< true if request is still spiraling through pr
                                    );
   
  protected:
   friend class TransferControlTest;

   /// Add in the REFER request's Refer-To header a X-SipX-Location-Info header containing user location and gateway line-id
   void addLocationInfo(
       const UtlString& id, ///< The authenticated identity of the request originator, expected to not be null
       Url& target          ///< Refer-To url where to add location info
       );
   /**<
    * This methods will construct the X-SipX-Location-Info header and adds it to Refer-To url.
    * the X-SipX-Location-Info header can contain two parameters:
    * - location of the refer originator. In case the user has no location then this parameter
    * will not be added;
    * - line-id of the initial gateway used as destination for call from the referror to the transfer target.
    * In case support for multiple gateways per location is NOT enabled this parameter will not be added.
    * @note In case the parameters above are not available then the header will not be added at all
    */
   
   static const char* RecognizerConfigKey1;
   static const char* RecognizerConfigKey2;
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
