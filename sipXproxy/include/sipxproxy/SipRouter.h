// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _SipRouter_h_
#define _SipRouter_h_

// SYSTEM INCLUDES
#include "sipdb/EntityDB.h"

// APPLICATION INCLUDES
#include <os/OsServerTask.h>
#include "os/OsThreadPool.h"
#include <sipXecsService/SipNonceDb.h>
#include <utl/PluginHooks.h>
#include <sipxproxy/AuthPlugin.h>
#include <net/SipBidirectionalProcessorPlugin.h>
#include <sipdb/RegDB.h>
#include <Poco/Semaphore.h>
#include <boost/thread.hpp>

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipUserAgent;
class OsConfigDb;
class SipMessage;
class RouteState;
class ForwardRules;
class SipOutputProcessor;


/// SipRouter implements the main message handling responsible for the forking
/// authorization and forwarding of SIP messages.
/**
 * The SipUserAgent sends a SipMessageEvent to the OsTask queue for this object;
 * for each such message:
 *
 * -# Extract the SipMessage from the OsMsg passed by the SipUserAgent and call
 *    the proxyMessage method to process it
 *  - proxyMessage does:
 *    -# Use SipMessage::normalizeProxyRoutes to get the route set into normal form,
 *       keeping a copy of any removed Route headers.
 *    -# Enforce Proxy-Require
 *    -# Check for and validate any user authentication in the message for this domain.
 *    -# Extract RouteState information (carried in Record-Route and Route headers)
 *       from the message.
 *    -# Invoke AuthPlugin::authorizeAndModify on the message for each configured AuthPlugin,
 *       stopping if any returns AuthPlugin::UNAUTHORIZED
 *       - An AuthPlugin may modify the message and/or the RouteState
 *    -# If all AuthPlugin objects return AuthPlugin::ALLOW_REQUEST, then proxyMessage
 *       returns true; if not, it sends a response before returning false:
 *       - '403 Forbidden' if there was valid user authentication in the message.
 *         (in this case, the reasonPhrase provided by AuthPlugin::authorizeAndModify
 *         is used if it is not null).
 *       - '407 Proxy Authentication Required' if there was no valid user authentication
 *         in the message.
 * -# If proxyMessage returned true, handleMessage passes it back to the SipUserAgent
 *    to be sent.
 *
 * Most of the actual decision making is delegated to the AuthPlugin classes; see the
 * AuthPlugin abstract base class for the interface and rules of operation.  This allows
 * modular extensions and changes to the authproxy operation without changing the basic
 * proxy behavior.
 */
class SipRouter : public OsServerTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:
    
  typedef boost::mutex mutex_critic_sec;
  typedef boost::lock_guard<mutex_critic_sec> mutex_critic_sec_lock;

   /// Default constructor
   SipRouter(SipUserAgent& sipUserAgent, 
             ForwardRules& forwardingRules,
             OsConfigDb&   configDb
          );

   virtual ~SipRouter();
   //:Destructor

   virtual UtlBoolean handleMessage(OsMsg& rMsg);

   /// Action to be taken by the caller of proxyAction method
   typedef enum
   {
      SendRequest,
      SendResponse,
      DoNothing
   } ProxyAction;
   
   /// Examine a request to be proxied, and either modify it for forwarding or create a response.
   ProxyAction proxyMessage(SipMessage&  request,  /**< request to be proxied
                                                    *   iff return is SendRequest */
                            SipMessage&  response  /**< response to be sent
                                                    *   iff return is SendResponse */
                            );
   /**< the caller must pass either request or response
    *   to SipUserAgent::send as directed by the return code
    */

   /// Adds to the request URI one or more parameters w/ or w/o values
   void addRuriParams(SipMessage& sipRequest, const UtlString& ruriParam);
   /**<
    * @param sipRequest The SIP request to be modified
    * @param ruriParam A string containing parameters to be added. The string must have the following format:
    *   "paramName1=paramValue1;...;paramNameN=paramValueN"
    * @note parameters with no value are accepted, like:
    *   "paramName1"
    */

   /// @returns true iff the domain of url is a valid form of the domain name for this proxy.
   bool isLocalDomain(const Url& url, ///< a url to be tested
                      bool bIncludeDomainAliases = true ///< also test for domain alias matches 
                      ) const;

   /// Adds a new host alias to the list of aliases for proxy.  This method
   /// is meant to be used by plugins that require special aliases.
   void addHostAlias( const UtlString& hostAliasToAdd );
   
   void addSipOutputProcessor( SipOutputProcessor *pProcessor );

   UtlBoolean removeSipOutputProcessor( SipOutputProcessor *pProcessor );
   
   /// Send a keepalive message to the specified address/port using the SipRouter's SipUserAgent.
   void sendUdpKeepAlive( SipMessage& keepAliveMsg, const char* serverAddress, int port );

   /// Get the canonical form of our SIP domain name.
   void getDomain(UtlString& canonicalDomain) const;
   
   /// Get the domain shared secret for signing.
   const SharedSecret* authSecret();

    /// Retrieve the SIP credential check values for a given identity and realm
    bool getCredential (
       const Url& uri,
       const UtlString& realm,
       UtlString& userid,
       UtlString& passtoken,
       UtlString& authType
                              ) const;

    /// Retrieve the SIP credential check values for a given userid and realm
    bool getCredential (
       const UtlString& userid,
       const UtlString& realm,
       Url& uri,
       UtlString& passtoken,
       UtlString& authType
                              ) const;

    /// Retrieve the location associated with an user
    bool getUserLocation (
       const UtlString& identity, ///< identity of the user
       UtlString& location        ///< location of the user or empty string
       ) const;
    /**<
     * Check if the user has a location in EntityDB and retrieves it.
     * @returns true if user has a location, false otherwise
     */


   /// If the fromUrl uses domain alias, change to original domain as identities are stored in credential database using mDomainName.
   void ensureCanonicalDomain(Url& url) const;

   UtlBoolean isRelayAllowed() const;

   SipUserAgent* getUserAgent() const;
   
/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:
   friend class SipRouterTest;

   /// Extract configuration parameters from the configuration file.
   void readConfig(OsConfigDb& configDb,  /// database to read for parameters
                   const Url&  defaultUri /// to be used for default realm and route
                   );

   /// Find the authenticated identity for a request (if any).
   bool isAuthenticated(const SipMessage& sipRequest, ///< message to be checked for authentication
                        UtlString& authUser           ///< returned authenticated identity uri
                        );
   /**<
    * Check the credentials in sipRequest for valid credentials for a
    * user in mRealm.
    * @returns true iff valid authentication is found (and sets authUser).
    */

   /** If sipRequest is a REGISTER that came from behind a NAT (indicated
    *  by 'received' parameter on topmost Via), add an initial signed Path URI
    *  for this proxy.
    *  @returns true iff sipRequest was modified.
    */
   bool addPathHeaderIfNATOrTlsRegisterRequest( SipMessage& sipRequest ) const;

   bool addNatMappingInfoToContacts( SipMessage& sipRequest ) const;
   
   /// Verifies if the proxy supports all the extensions listed in the Proxy-Require
   /// header of an incoming request.
   bool areAllExtensionsSupported( const SipMessage& sipRequest,   ///< request to be evluated
                                   UtlString& disallowedExtensions ///< unsupported extensions
                                   ) const;
   
   /// Create an authentication challenge.
   void authenticationChallenge(const SipMessage& sipRequest,///< message to be challenged. 
                                SipMessage& challenge        ///< challenge response.
                                );
   

   void handleRequest(SipMessage* pMsg);
   
   SipUserAgent* mpSipUserAgent;         ///< SIP stack interface
   bool          mAuthenticationEnabled; ///< based on SIPX_PROXY_AUTHENTICATE_ALGORITHM
   UtlString     mRealm;                 ///< realm for challenges - common to replicatants
   SipNonceDb    mNonceDb;               ///< generator for nonce values
   long          mNonceExpiration;       ///< nonce lifetime in seconds
   UtlString     mDomainName;            ///< for determining authority for addresses
   UtlString     mDomainAliases;         ///< for determining authority for addresses
   UtlString     mRouteHostPort;         ///< for writing Record-Route headers
   UtlString     mRouteHostSecurePort;
   SharedSecret* mSharedSecret;          ///< secret value to include in authentication hashes
   ForwardRules* mpForwardingRules;      ///< Holds to forwarding rules instructions
   PluginHooks   mAuthPlugins;           ///< decision making modules from configuration
   PluginHooks   mTransactionPlugins; 
   UtlBoolean    mEnsureTcpLifetime;     ///< If set, transport will be explicity specified in record routes.
   UtlBoolean    mRelayAllowed;     ///< If set, transport will be explicity specified in record routes.
   /// P-Asserted-Identity header is only applicable for INVITE, REFER, BYE,
   /// OPTIONS, NOTIFY, and SUBSCRIBE
   bool isPAIdentityApplicable(const SipMessage& sipRequest);

   //
   // This method will be call if a SIP message will no longer be spiraling through 
   // the authentication rules and is about to be sent out to the final destination.
   // This is normally the place where the code will evaluate extra rules
   // such as privacy.
   //
   void performPreRoutingChecks(SipMessage& sipRequest);
   // @cond INCLUDENOCOPY

   // There is no copy constructor.
   SipRouter(const SipRouter& rsipRouter);

   // There is no assignment operator.
   SipRouter& operator=(const SipRouter& rhs);

   // @endcond 

	 friend class SipBridgeRouter;

   EntityDB* mpEntityDb;
   RegDB* mpRegDb;
   OsThreadPool<SipMessage*> _threadPool;
   Poco::Semaphore _threadPoolSem;
   mutex_critic_sec _outboundMutex;
};

/* ============================ INLINE METHODS ============================ */

inline UtlBoolean SipRouter::isRelayAllowed() const
{
  return mRelayAllowed;
}

inline SipUserAgent* SipRouter::getUserAgent() const
{
  return mpSipUserAgent;
}


#endif  // _SipRouter_h_
