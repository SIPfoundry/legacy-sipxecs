// Initial Version Copyright (C) David Becker (IANT GmbH), All Rights Reserved.
// Licensed to the User under the LGPL license.
//
// This library is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
// details.
// 
//////////////////////////////////////////////////////////////////////////////
#ifndef _CALLERALERTINFO_H_
#define _CALLERALERTINFO_H_

// SYSTEM INCLUDES
#include "os/OsMutex.h"
#include "os/OsRWMutex.h"

// APPLICATION INCLUDES
#include <sipxproxy/AuthPlugin.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class CallerAlertInfo;

extern "C" AuthPlugin* getAuthPlugin(const UtlString& name);

/**
 * Proxy AuthPlugin CallerAlertInfo
 * This plug-in adds the Alert-Info header field to all INVITE messages passing through.
 * If the message originates from the local domain the message is marked as internal,
 * otherwise it is marked as external.
 */

class CallerAlertInfo : public AuthPlugin
{
  public:

   virtual ~CallerAlertInfo();

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

   /// Called for any request
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

   static OsMutex        sSingletonLock;  ///< lock to protext the spInstance
   static CallerAlertInfo*   spInstance;  ///< only one CallerAlertInfo may be configured

  private:
   friend AuthPlugin* getAuthPlugin(const UtlString& name);

   CallerAlertInfo(const UtlString& instanceName ///< the configured name for this plugin instance
              );

   OsRWMutex  mConfigLock;      ///< lock to protect the configuration
   
   // Plug-in configuration
   UtlString  mExternalText;    ///< Alert-Info value for external calls
   bool       mExternalEnabled; ///< Whether Alert-Info should be appended to external calls
   UtlString  mInternalText;    ///< Alert-Info value for internal calls
   bool       mInternalEnabled; ///< Whether Alert-Info should be appended to internal calls
   bool       mReplaceExisting; ///< If the SIP request already contains an Alert-Info, should we replace it?

   SipRouter* mpSipRouter;
};

#endif // _CALLERALERTINFO_H_
