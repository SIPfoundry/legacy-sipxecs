// Initial Version Copyright (C) David Becker (IANT GmbH), All Rights Reserved.
// Licensed to the User under the LGPL license.
//
// This library is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
// details.
// 
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include "os/OsLock.h"
#include "os/OsReadLock.h"
#include "os/OsWriteLock.h"
#include "os/OsConfigDb.h"
#include "os/OsLogger.h"
#include "os/OsFS.h"
#include "net/Url.h"

// APPLICATION INCLUDES

#include <sipxproxy/RouteState.h>
#include <sipxproxy/SipRouter.h>
#include "CallerAlertInfo.h"
#include <assert.h>

// DEFINES
#define FIELD_NAME "Alert-Info"
// CONSTANTS
const UtlString ONEXIST_DO_NOTHING = UtlString("0");
const UtlString ONEXIST_REPLACE = UtlString("1");
// TYPEDEFS
OsMutex        CallerAlertInfo::sSingletonLock(OsMutex::Q_FIFO);
CallerAlertInfo*   CallerAlertInfo::spInstance;

/// Factory used by PluginHooks to dynamically link the plugin instance
extern "C" AuthPlugin* getAuthPlugin(const UtlString& pluginName)
{
   OsLock singleton(CallerAlertInfo::sSingletonLock);

   if (!CallerAlertInfo::spInstance)
   {
      CallerAlertInfo::spInstance = new CallerAlertInfo(pluginName);
   }
   else
   {
      Os::Logger::instance().log(FAC_SIP, PRI_CRIT, "CallerAlertInfo[%s]: "
                    "it is invalid to configure more than one instance of the CallerAlertInfo plugin.",
                    pluginName.data());
      assert(false);
   }

   return CallerAlertInfo::spInstance;
}

/// constructor
CallerAlertInfo::CallerAlertInfo(const UtlString& pluginName ///< the name for this instance
                         )
   : AuthPlugin(pluginName),
     mConfigLock(OsRWMutex::Q_FIFO),
     mExternalText(""),
     mExternalEnabled(false),
     mInternalText(""),
     mInternalEnabled(false),
     mReplaceExisting(false),
     mpSipRouter( 0 )
{
   Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, "CallerAlertInfo[%s] started",
                 mInstanceName.data()
                 );
};

/// Nothing configurable
void
CallerAlertInfo::readConfig( OsConfigDb& configDb /**< a subhash of the individual configuration
                                               * parameters for this instance of this plugin. */
                             )
{
   /*
    * @note
    * The parent service may call the readConfig method at any time to
    * indicate that the configuration may have changed.  The plugin
    * should reinitialize itself based on the configuration that exists when
    * this is called.  The fact that it is a subhash means that whatever prefix
    * is used to identify the plugin (see PluginHooks) has been removed (see the
    * examples in PluginHooks::readConfig).
    */
   Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, "CallerAlertInfo[%s]::readConfig",
                 mInstanceName.data()
                 );
   OsWriteLock writeLock(mConfigLock);
   mExternalEnabled = configDb.getBoolean("EXTERNAL_ENABLED",false);
   configDb.get("EXTERNAL", mExternalText);
   mInternalEnabled = configDb.getBoolean("INTERNAL_ENABLED",false);
   configDb.get("INTERNAL", mInternalText);
   UtlString _onExisting;
   configDb.get("ON_EXISTING", _onExisting);
   if (_onExisting.compareTo(ONEXIST_REPLACE) == 0) {
      mReplaceExisting = true;
   } else {
      mReplaceExisting = false;
   }
}

AuthPlugin::AuthResult
CallerAlertInfo::authorizeAndModify(const UtlString& id,    /**< The authenticated identity of the
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
                                bool bSpiralingRequest, ///< spiraling indication 
                                UtlString&  reason      ///< rejection reason
                                )
{
   Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, "CallerAlertInfo[%s]::authorizeAndModify() begin",
                 mInstanceName.data()
                 );

   if (   (priorResult != DENY) // no point in modifying a request that won't be sent
       && (method.compareTo(SIP_INVITE_METHOD) == 0) //We only operate on INVITEs
       )   
   {
      
      OsReadLock readLock(mConfigLock);
      //Only checking if the header exists already
      const char* _existingHeader = request.getHeaderValue(0, FIELD_NAME);
      //If it exists we will only act if we are instructed to replace it.
      if (_existingHeader == NULL || mReplaceExisting) {
         // Check if the SIP message comes from the local domain
         Url _fromAddress;
         request.getFromUrl(_fromAddress);
         if (!mpSipRouter->isLocalDomain(_fromAddress)) {
            //if not then add Alert-Info External
            Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, "CallerAlertInfo[%s]::authorizeAndModify() found external call",
                          mInstanceName.data()
                          );
            if (mExternalEnabled) {              
               request.setHeaderValue(FIELD_NAME, mExternalText);
            }
         } else {
            //otherwise add Alert-Info Internal
            if (mInternalEnabled) {              
               request.setHeaderValue(FIELD_NAME, mInternalText);
            }
         }
      }
   }
   Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, "CallerAlertInfo[%s]::authorizeAndModify() end",
                 mInstanceName.data()
                 );
   return AuthPlugin::CONTINUE;
}

void CallerAlertInfo::announceAssociatedSipRouter( SipRouter* sipRouter )
{
   mpSipRouter = sipRouter;
}

/// destructor
CallerAlertInfo::~CallerAlertInfo()
{
   Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, "CallerAlertInfo[%s] stopped",
                 mInstanceName.data()
                 );
}
