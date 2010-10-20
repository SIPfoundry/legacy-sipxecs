//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _NatTraversalRules_h_
#define _NatTraversalRules_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "digitmaps/Patterns.h"
#include "os/OsStatus.h"
#include "os/OsTask.h"
#include "os/OsStunQueryAgent.h"
#include "os/OsStunDatagramSocket.h"
#include "utl/UtlString.h"
#include "utl/UtlSList.h"
#include "xmlparser/tinyxml.h"
#include "NatTraversalAgentDataTypes.h"

// DEFINES
 //XML tags
#define XML_TAG_NAT_TRAVERSAL         "nattraversal"
#define XML_TAG_INFO                  "info"
#define XML_TAG_STATE                 "state"
#define XML_TAG_PUBLIC_ADDRESS        "publicaddress"
#define XML_TAG_PUBLIC_PORT           "publicport"
#define XML_TAG_PROXY_HOST_PORT       "proxyunsecurehostport"
#define XML_TAG_LOCAL_TOPOLOGY        "localtopology"
#define XML_TAG_IPV4SUBNET            "ipV4subnet"
#define XML_TAG_DNSWILDCARD           "dnsWildcard"
#define XML_TAG_BEHIND_NAT            "behindnat"
#define XML_TAG_MR_PUBLIC_ADDRESS     "mediarelayexternaladdress"
#define XML_TAG_MR_NATIVE_ADDRESS     "mediarelaynativeaddress"
#define XML_TAG_MR_XMLRPC_PORT        "mediarelayxml-rpc-port"
#define XML_TAG_MR_PORT_RANGE         "port-range"
#define XML_TAG_USE_STUN              "useSTUN"
#define XML_TAG_STUN_SERVER           "stun-server-address"
#define XML_TAG_STUN_REFRESH_INTERVAL "rediscovery-time"
#define XML_TAG_AGGRESSIVENESS        "relayaggressiveness"
#define XML_TAG_SECURE_XMLRPC         "secureXMLRPC"

//XML values
#define XML_VALUE_ENABLED           "enabled"
#define XML_VALUE_TRUE              "true"
#define XML_VALUE_YES               "yes"
#define XML_VALUE_AGGRESSIVE        "Aggressive"
#define XML_VALUE_CONSERVATIVE      "Conservative"

//DEFAULTS
#define DEFAULT_MAX_MEDIA_RELAY_SESSIONS (50)
#define DEFAULT_PUBLIC_PORT              (5060)
#define DEFAULT_MR_XMLRPC_PORT           (9090)

// OTHER
#define PORTS_PER_MEDIA_RELAY_SESSIONS   (4)   // RHS RTP & RTCP + LHS RTP & RTCP
#define NAT_TRAVERSAL_SHARE              (0.5) // NAT Traversal gets half of the media relay ports

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * Class that knows how to parse the NatTraversalRules.xml file and
 * extract its information.  The class offers an API to allow apps
 * to retrieve the information elements of that file.  Note that
 * there are two ways to specifiy the public IP address of a system
 * through the web GUI.  One is to manually enter an IP address
 * and the other is automatically discover it through STUN.  The
 * NatTraversalRule class hides that detail from its owner
 * by implementing a STUN client that is used to automatically
 * discover the public IP address when a STUN server is provided.
 * The owner can find out the public IP addresses of the system
 * via getPublicTransportInfo() and getMediaRelayPublicAddress()
 * without really knowing if that data was explicitly configured
 * or discovered through STUN.
 */
class NatTraversalRules
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   NatTraversalRules();

   virtual ~NatTraversalRules();

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */
   OsStatus loadRules(const UtlString& configFileName );

   bool isEnabled( void ) const;

   bool isBehindNat( void ) const;

   TransportData getPublicTransportInfo( void ) const;

   TransportData getProxyTransportInfo( void ) const;

   UtlString     getMediaRelayPublicAddress( void ) const;

   UtlString     getMediaRelayNativeAddress( void ) const;

   int           getMediaRelayXmlRpcPort( void ) const;

   int           getMaxMediaRelaySessions( void ) const;

   bool          isAggressiveModeSet( void ) const;

   bool          isConservativeModeSet( void ) const;

   bool          isXmlRpcSecured( void ) const;

   bool isPartOfLocalTopology(const UtlString& host,
                              bool checkIpSubnets = true,
                              bool checkDnsWidlcards = true ) const;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   /**
    * Class that gets used by NatTraversalRules class to resolve the public IP
    * address through STUN whenever the nattraversalrules.xml config file calls
    * for it.
    * */
   class StunClient : public OsTask
   {
      public:
         StunClient( const UtlString& stunServer );
         virtual ~StunClient();

         bool getPublicIpAddress( UtlString& discoveredPublicIpAddress );
         void maintainPublicIpAddressCurrent( NatTraversalRules* pNatTraversalRulesToKeepCurrent,
                                              int refreshIntervalInSecs,
                                              const UtlString& publicIpAddressHint );
         virtual void requestShutdown( void );

      protected:
         virtual int run( void* runArg );

      private:
         OsMutex mTimerMutex;               // mutex used to generate heartbeat;
         OsStunQueryAgent stunQueryAgent;
         OsStunDatagramSocket mSocket;
         NatTraversalRules* mpNatTraversalRules;
         NatTraversalRules* mpNatTraversalRulesToKeepCurrent;
         UtlString mPublicIpAddressObtainedFromLastPoll;
         UtlString mStunServerName;
         bool mbStunServerIsValid;  // whether STUN server was resolved to an IP address
   };

   TiXmlDocument* mpDoc;
   Patterns*      mpPatterns;
   bool           mbNatTraveralEnabled;
   bool           mbSystemBehindNat;
   bool           mbAggressiveModeSet;
   UtlSList       mlocalIpV4Subnets;
   UtlSList       mlocalDnsWildcards;
   TransportData  mPublicTransport;
   TransportData  mProxyTransport;
   UtlString      mMediaRelayPublicAddress;
   bool           mbMediaRelayPublicAddressProvidedInConfig;
   UtlString      mMediaRelayNativeAddress;
   int            mMediaRelayXmlRpcPort;
   int            mMaxMediaRelaySessions;
   bool           mbDiscoverPublicIpAddressViaStun;
   UtlString      mStunServer;
   int            mStunRefreshIntervalInSecs;
   StunClient*    mpStunClient;
   bool           mbXmlRpcOverSecureTransport;

   void initializeNatTraversalInfo( void );

   // Private methods to be called by StunClient
   UtlString     getStunServer( void ) const;

   int           getStunRefreshIntervalInSecs( void ) const;

   void announceStunResolvedPublicIpAddress( const UtlString& discoveredPublicIpAddress );
};

/* ============================ INLINE METHODS ============================ */

#endif  // _NatTraversalRules_h_
