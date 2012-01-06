#ifndef _CALLERID_H_
#define _CALLERID_H_

// SYSTEM INCLUDES
#include "os/OsMutex.h"

// APPLICATION INCLUDES
#include "AuthPlugin.h"
#include "xmlparser/tinyxml.h"

// DEFINES
#define XML_TAG_ITEMS          "items"
#define XML_TAG_ITEM           "item"
#define XML_ATT_EXTENSION      "extension"
#define XML_ATT_REWRITE        "rewrite"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//FIXME
//class CallerAliasTest;

extern "C" AuthPlugin* getAuthPlugin(const UtlString& name);

class CallerID : public AuthPlugin
{
  public:

   virtual ~CallerID();

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
   friend class SipRouterTest;

   static OsMutex        sSingletonLock;
   static CallerID*   spInstance;


  private:
   bool			bEnabled;
   UtlString		fileName;
   TiXmlDocument *mDoc;

   friend AuthPlugin* getAuthPlugin(const UtlString& name);

   CallerID(const UtlString& instanceName ///< the configured name for this plugin instance
              );

   SipRouter* mpSipRouter;
};

#endif // _CALLERID_H_
