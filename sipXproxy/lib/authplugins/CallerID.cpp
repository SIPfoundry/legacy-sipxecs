// SYSTEM INCLUDES
#include "os/OsLock.h"
#include "os/OsConfigDb.h"
#include "os/OsSysLog.h"
#include "os/OsFS.h"
#include "net/Url.h"

// APPLICATION INCLUDES

#include "RouteState.h"
#include "SipRouter.h"
#include "CallerID.h"
#include "sipXecsService/SipXecsService.h"

// DEFINES
// CONSTANTS

const char* INTERNAL_CID_FILENAME = "cidrules.xml";
const char* ENABLE_PARAM="ENABLE_INTERNAL_CID";
const char* ENABLE_TRUE="true";

// TYPEDEFS
OsMutex        CallerID::sSingletonLock(OsMutex::Q_FIFO);
CallerID*   CallerID::spInstance;

/// Factory used by PluginHooks to dynamically link the plugin instance
extern "C" AuthPlugin* getAuthPlugin(const UtlString& pluginName)
{
   OsLock singleton(CallerID::sSingletonLock);

   if (!CallerID::spInstance)
   {
         CallerID::spInstance = new CallerID(pluginName);
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_CRIT, "CallerID[%s]: "
                    "it is invalid to configure more than one instance of the CallerID plugin.",
                    pluginName.data());
      assert(false);
   }

   return CallerID::spInstance;
}

/// constructor
CallerID::CallerID(const UtlString& pluginName ///< the name for this instance
                         )
   : AuthPlugin(pluginName),
     mpSipRouter( 0 )
{

};

/// Nothing configurable outside the database right now
void
CallerID::readConfig( OsConfigDb& configDb /**< a subhash of the individual configuration
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
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "CallerID[%s]::readConfig",
                 mInstanceName.data()
                 );

   bEnabled = configDb.getBoolean(ENABLE_PARAM,false);
   fileName = SipXecsService::Path(SipXecsService::ConfigurationDirType, INTERNAL_CID_FILENAME);
   mDoc = new TiXmlDocument( fileName.data() );
   if(bEnabled)
   {
      OsSysLog::add(FAC_SIP, PRI_CRIT, "CallerID: CallerID[%s] plugin is now enabled",  mInstanceName.data());
      if( !mDoc->LoadFile() ) {
         UtlString parseError = mDoc->ErrorDesc();
         OsSysLog::add( FAC_SIP, PRI_ERR, "ERROR parsing cidrules '%s': %s disabling CallerID plugin",fileName.data(), parseError.data());
         bEnabled = false;
      }
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_CRIT, "CallerID: CallerID[%s] plugin is now disabled", mInstanceName.data());
   }
}

AuthPlugin::AuthResult
CallerID::authorizeAndModify(const UtlString& id,    /**< The authenticated identity of the
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
   if(!bEnabled)
   {
      return AuthPlugin::CONTINUE;
   }


/* Is this an INVITE request? */
   if (method.compareTo(SIP_INVITE_METHOD) == 0)
   {

      if (priorResult != DENY) // no point in modifying a request that won't be sent
      {
         if (   routeState.isMutable()
             && routeState.directionIsCallerToCalled(mInstanceName.data())
             ) // a new dialog?
         {   // FIXME

         UtlString originalFromField, cNewFromUrl;
         request.getFromField(&originalFromField);
         Url originalFromUrl(originalFromField);
         UtlString cCallerNumber;
         originalFromUrl.getUserId(cCallerNumber);
         bool identityIsLocal = mpSipRouter->isLocalDomain(originalFromUrl);
         if (identityIsLocal) {

            OsSysLog::add(FAC_SIP, PRI_DEBUG, "Processing caller number %s", cCallerNumber.data());

            TiXmlNode* itemsNode = mDoc->FirstChild( XML_TAG_ITEMS );
            TiXmlNode* itemNode;
            for ( itemNode = itemsNode->FirstChild(); itemNode != 0; itemNode= itemNode->NextSibling()) {
               TiXmlElement* itemElement = itemNode->ToElement();
               const char* extension=itemElement->Attribute( XML_ATT_EXTENSION );
               if (!extension) {
                  continue;
               }

               try {

                  if (cCallerNumber.compareTo(extension) == 0) {
                     const char* rewrite=itemElement->Attribute( XML_ATT_REWRITE );
                     if (!rewrite) {
                        continue;
                     }

                     OsSysLog::add( FAC_SIP, PRI_DEBUG, "parsing cidrules match extension '%s' and rewrite '%s'", extension, rewrite);

                     originalFromUrl.setUserId(rewrite);
                     originalFromUrl.toString(cNewFromUrl);
                     request.setRawFromField(cNewFromUrl.data());
                     return AuthPlugin::CONTINUE;
                  } else {
                     OsSysLog::add( FAC_SIP, PRI_DEBUG, "no match for number'%s' in extension '%s'", cCallerNumber.data(), extension);
                  }
               } catch(const char * ErrorMsg) {
                  OsSysLog::add(FAC_SIP, PRI_ERR, "failed to match cid for extension %s error %s", extension ,ErrorMsg);
               }
            }		
         }

      }
      } else {
         OsSysLog::add(FAC_SIP, PRI_DEBUG, "CallerID[%s]::authorizeAndModify not mutable - no rewrite", mInstanceName.data());
      }
   }

   return AuthPlugin::CONTINUE;
}

void CallerID::announceAssociatedSipRouter( SipRouter* sipRouter )
{
   mpSipRouter = sipRouter;
}

/// destructor
CallerID::~CallerID()
{

}
