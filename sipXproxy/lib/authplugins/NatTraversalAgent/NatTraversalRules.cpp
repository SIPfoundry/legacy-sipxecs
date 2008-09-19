// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>
#include <stdlib.h>

// APPLICATION INCLUDES
#include "utl/UtlSListIterator.h"
#include "os/OsSysLog.h"
#include "net/Url.h"
#include "NatTraversalRules.h"
#include "os/OsTime.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define STUN_QUERY_TIMEOUT_IN_MILLISECS (5000)

/* //////////////////////////// PUBLIC //////////////////////////////////// */
// Constructor
NatTraversalRules::NatTraversalRules()
   : mpDoc                 ( NULL ),
     mpPatterns            ( NULL ),
     mbNatTraveralEnabled  ( false ),
     mbSystemBehindNat     ( false ),
     mbAggressiveModeSet   ( true ),
     mMaxMediaRelaySessions( DEFAULT_MAX_MEDIA_RELAY_SESSIONS ),
     mbDiscoverPublicIpAddressViaStun( false ),
     mStunRefreshIntervalInSecs( 300 ),
     mpStunClient( 0 )
{
   UtlString hostIpAddress;
   OsSocket::getHostIp( &hostIpAddress );
   mPublicTransport.setAddress( hostIpAddress );
   mPublicTransport.setPort( DEFAULT_PUBLIC_PORT );
}

// Destructor
NatTraversalRules::~NatTraversalRules()
{
   delete mpDoc;
   delete mpPatterns;
   delete mpStunClient;

}

/* ============================ MANIPULATORS ============================== */
OsStatus NatTraversalRules::loadRules(const UtlString& configFileName )
{
   OsStatus currentStatus = OS_SUCCESS;

   // Destroy patterns, and start clean
   if (mpPatterns) delete mpPatterns ;
   mpPatterns = new Patterns() ;

   // Remove any old mappings first
   if(mpDoc) delete mpDoc;
   mpDoc = new TiXmlDocument( configFileName.data() );

   if( !mpDoc->LoadFile() )
   {
      UtlString parseError = mpDoc->ErrorDesc();
      OsSysLog::add( FAC_NAT, PRI_ERR, "ERROR parsing nattraversalrules '%s': %s"
                    ,configFileName.data(), parseError.data());

      currentStatus = OS_NOT_FOUND;
   }
   else
   {
      initializeNatTraversalInfo();
   }
   return currentStatus;
}

void NatTraversalRules::initializeNatTraversalInfo( void )
{
   mbNatTraveralEnabled    = false;
   mbSystemBehindNat       = false;
   
   TiXmlNode* pTopNode;
   TiXmlNode* pNode;

   // If we already have a StunClient object allocated, chances are that
   // it is doing STUN polling based on the old configuration data.  
   // Since we are reading in new configuration data here, stop that 
   // StunClient.  Another instance will get created if required.
   if( mpStunClient )
   {
      delete mpStunClient;
   }

   // get the 'nattraversal' node
   if( ( pTopNode = mpDoc->FirstChild( XML_TAG_NAT_TRAVERSAL ) ) )
   {
      // get the 'info' node
      if( ( pNode = pTopNode->FirstChild( XML_TAG_INFO ) ) )
      {
         TiXmlNode* pChildNode;
         // get the 'state' node
         
         if( ( pChildNode = pNode->FirstChild( XML_TAG_STATE ) ) && pChildNode->FirstChild() )
         {
            UtlString status = pChildNode->FirstChild()->Value();
            
            if( status.compareTo(XML_VALUE_ENABLED, UtlString::ignoreCase) == 0 )
            {  
               mbNatTraveralEnabled = true;
            }
         }
         else
         {
            OsSysLog::add(FAC_NAT, PRI_ERR, "NatTraversalRules::initializeNatTraversalInfo - No child Node named '%s'", XML_TAG_STATE );          
         }

         // get the 'behindnat' node
         if( ( pChildNode = pNode->FirstChild( XML_TAG_BEHIND_NAT ) ) && pChildNode->FirstChild() )
         {
            UtlString status = pChildNode->FirstChild()->Value();
            if( status.compareTo(XML_VALUE_TRUE, UtlString::ignoreCase) == 0 || status.compareTo(XML_VALUE_YES, UtlString::ignoreCase) == 0  )
            {  
               mbSystemBehindNat = true;
            }
         }
         else
         {
            OsSysLog::add(FAC_NAT, PRI_ERR, "NatTraversalRules::initializeNatTraversalInfo - No child Node named '%s'", XML_TAG_BEHIND_NAT );          
         }
   
         // get the 'relayaggressiveness' node
         if( ( pChildNode = pNode->FirstChild( XML_TAG_AGGRESSIVENESS ) ) && pChildNode->FirstChild() )
         {
            UtlString status = pChildNode->FirstChild()->Value();
            if( status.compareTo( XML_VALUE_AGGRESSIVE, UtlString::ignoreCase ) == 0 )
            {  
               mbAggressiveModeSet = true;
            }
            else
            {
               mbAggressiveModeSet = false;
            }
         }
         else
         {
            OsSysLog::add(FAC_NAT, PRI_ERR, "NatTraversalRules::initializeNatTraversalInfo - No child Node named '%s'", XML_TAG_AGGRESSIVENESS );          
         }
         
         // get the 'publicaddress' node
         if( ( pChildNode = pNode->FirstChild( XML_TAG_PUBLIC_ADDRESS ) ) && pChildNode->FirstChild() )
         {
            UtlString hostIpAddress;
            hostIpAddress = pChildNode->FirstChild()->Value();
            mPublicTransport.setAddress( hostIpAddress );
         }
         else
         {
            // No public IP address present in the rules file - default to host's IP address
            UtlString hostIpAddress;
            OsSocket::getHostIp( &hostIpAddress );
            Url url( hostIpAddress, TRUE );
            mPublicTransport.setAddress( hostIpAddress );
            OsSysLog::add(FAC_NAT, PRI_ERR, "NatTraversalRules::initializeNatTraversalInfo - No child Node named '%s', using host IP: '%s'", XML_TAG_PUBLIC_ADDRESS, mPublicTransport.getAddress().data() );
         }
         
         // get the 'publicport' node
         if( ( pChildNode = pNode->FirstChild( XML_TAG_PUBLIC_PORT ) ) && pChildNode->FirstChild() )
         {
            UtlString tempPublicPortString; 
            tempPublicPortString = pChildNode->FirstChild()->Value();
            mPublicTransport.setPort( atoi( tempPublicPortString.data() ) );
         }
         else
         {
            // No public port present in the rules file - initialize to default value
            mPublicTransport.setPort( DEFAULT_PUBLIC_PORT );
            OsSysLog::add(FAC_NAT, PRI_ERR, "NatTraversalRules::initializeNatTraversalInfo - No child Node named '%s', using default port %d", XML_TAG_PUBLIC_PORT, mPublicTransport.getPort() );
         }
         
         // get the 'proxyhostport' node
         if( ( pChildNode = pNode->FirstChild( XML_TAG_PROXY_HOST_PORT ) ) && pChildNode->FirstChild() )
         {
            UtlString hostport = pChildNode->FirstChild()->Value();
            Url url( hostport, TRUE );
            mProxyTransport.fromUrl( url );
         }
         else
         {
            OsSysLog::add(FAC_NAT, PRI_ERR, "NatTraversalRules::initializeNatTraversalInfo - No child Node named '%s'", XML_TAG_PUBLIC_ADDRESS );
         }
         
         // get the 'mediarelaypublicaddress' node
         if( ( pChildNode = pNode->FirstChild( XML_TAG_MR_PUBLIC_ADDRESS ) ) && pChildNode->FirstChild() )
         {
            mMediaRelayPublicAddress = pChildNode->FirstChild()->Value();
         }
         else
         {
            mMediaRelayPublicAddress = mPublicTransport.getAddress();
            OsSysLog::add(FAC_NAT, PRI_ERR, "NatTraversalRules::initializeNatTraversalInfo - No child Node named '%s', using public IP address of '%s'", XML_TAG_MR_PUBLIC_ADDRESS, mPublicTransport.getAddress().data() );
         }
         
         // get the 'mediarelaynativeaddress' node
         if( ( pChildNode = pNode->FirstChild( XML_TAG_MR_NATIVE_ADDRESS ) ) && pChildNode->FirstChild() )
         {
            mMediaRelayNativeAddress = pChildNode->FirstChild()->Value();
         }
         else
         {
            OsSysLog::add(FAC_NAT, PRI_ERR, "NatTraversalRules::initializeNatTraversalInfo - No child Node named '%s'", XML_TAG_MR_NATIVE_ADDRESS );
         }
         
         // get the 'mediarelayxml-rpc-port' node
         if( ( pChildNode = pNode->FirstChild( XML_TAG_MR_XMLRPC_PORT ) ) && pChildNode->FirstChild() )
         {
            UtlString tempMediaRelayXmlRpcPortString; 
            tempMediaRelayXmlRpcPortString = pChildNode->FirstChild()->Value();
            mMediaRelayXmlRpcPort = atoi( tempMediaRelayXmlRpcPortString.data() );
         }
         else
         {
            OsSysLog::add(FAC_NAT, PRI_ERR, "NatTraversalRules::initializeNatTraversalInfo - No child Node named '%s'", XML_TAG_MR_XMLRPC_PORT );
         }
         
         // get the 'maxmediarelaysessions' node
         if( ( ( pChildNode = pNode->FirstChild( XML_TAG_MR_MAX_SESSIONS1 ) ) ||
               ( pChildNode = pNode->FirstChild( XML_TAG_MR_MAX_SESSIONS2 ) ) 
             ) && pChildNode->FirstChild() )
         {
            UtlString tempMaxMediaRelaySessionsString; 
            tempMaxMediaRelaySessionsString = pChildNode->FirstChild()->Value();
            mMaxMediaRelaySessions = atoi( tempMaxMediaRelaySessionsString.data() );
         }
         else
         {
            mMaxMediaRelaySessions = DEFAULT_MAX_MEDIA_RELAY_SESSIONS;
            OsSysLog::add(FAC_NAT, PRI_ERR, "NatTraversalRules::initializeNatTraversalInfo - No child Node named '%s' or '%s'", XML_TAG_MR_MAX_SESSIONS1, XML_TAG_MR_MAX_SESSIONS2 );
         }
         
         // get the 'useSTUN' node
         if( ( pChildNode = pNode->FirstChild( XML_TAG_USE_STUN ) ) && pChildNode->FirstChild() )
         {
            UtlString status = pChildNode->FirstChild()->Value();
            if( status.compareTo(XML_VALUE_TRUE, UtlString::ignoreCase) == 0 || status.compareTo(XML_VALUE_YES, UtlString::ignoreCase) == 0  )
            {  
               mbDiscoverPublicIpAddressViaStun = true;
            }
         }
         else
         {
            OsSysLog::add(FAC_NAT, PRI_ERR, "NatTraversalRules::initializeNatTraversalInfo - No child Node named '%s'", XML_TAG_USE_STUN );
         }         
         
         // get the 'STUNRefreshInterval' node
         if( ( pChildNode = pNode->FirstChild( XML_TAG_STUN_REFRESH_INTERVAL ) ) && pChildNode->FirstChild() )
         {
            UtlString tempIntervalString; 
            tempIntervalString = pChildNode->FirstChild()->Value();
            mStunRefreshIntervalInSecs = atoi( tempIntervalString.data() );
         }
         else
         {
            OsSysLog::add(FAC_NAT, PRI_ERR, "NatTraversalRules::initializeNatTraversalInfo - No child Node named '%s'", XML_TAG_STUN_REFRESH_INTERVAL );
         }

         // get the 'STUNServer' node
         if( ( pChildNode = pNode->FirstChild( XML_TAG_STUN_SERVER ) ) && pChildNode->FirstChild() )
         {
            mStunServer = pChildNode->FirstChild()->Value();
         }
         else
         {
            OsSysLog::add(FAC_NAT, PRI_ERR, "NatTraversalRules::initializeNatTraversalInfo - No child Node named '%s'", XML_TAG_STUN_SERVER );
         }
      }
      else
      {
         OsSysLog::add(FAC_NAT, PRI_ERR, "NatTraversalRules::initializeNatTraversalInfo - No child Node named '%s'", XML_TAG_INFO );          
      }

      // get the 'localtopology' node
      if( ( pNode = pTopNode->FirstChild( XML_TAG_LOCAL_TOPOLOGY ) ) )
      {
         // get child under the 'localtopology' node
         if ( (pNode = pNode->FirstChild() ) )
         {
            // iterate through each child of 'localtopology' node
            do
            {
               // we only care about elements
               if( pNode->Type() == TiXmlNode::ELEMENT )
               {
                  UtlString matchPatternType = pNode->Value();
                  TiXmlNode* matchPatternText = pNode->FirstChild();
                  if( matchPatternText && matchPatternText->Type() == TiXmlNode::TEXT)
                  {
                     UtlString* pMatchPatternTextString = new UtlString( matchPatternText->Value() );
                     if( matchPatternType.compareTo( XML_TAG_IPV4SUBNET, UtlString::ignoreCase ) == 0 )
                     {
                        mlocalIpV4Subnets.append( pMatchPatternTextString );
                     }
                     else if( matchPatternType.compareTo( XML_TAG_DNSWILDCARD, UtlString::ignoreCase ) == 0 )
                     {
                        mlocalDnsWildcards.append( pMatchPatternTextString );
                     }
                     else
                     {
                        OsSysLog::add(FAC_NAT, PRI_ERR, "NatTraversalRules::initializeNatTraversalInfo - Unknown pattern type found: '%s'", matchPatternType.data() );          
                     }         
                  }
               }
            } while( ( pNode = pNode->NextSibling() ) );
         }
         else
         {
            OsSysLog::add(FAC_NAT, PRI_ERR, "NatTraversalRules::initializeNatTraversalInfo - No child Node for '%s'", XML_TAG_LOCAL_TOPOLOGY );          
         }         
      }
      else
      {
         OsSysLog::add(FAC_NAT, PRI_ERR, "NatTraversalRules::initializeNatTraversalInfo - No child Node named '%s'", XML_TAG_LOCAL_TOPOLOGY );          
      }
      
      // Now that we have extracted the configuration information, let's check if we 
      // need to discover our public IP address using STUN.  If so, launch a task that
      // will do so.
      if( mbDiscoverPublicIpAddressViaStun )
      {
         // we need to discover our public IP address using STUN.
         // Instantiate a new StunClient and ask for the public IP address.
         // If the request is successful, initialize the public transport with 
         // the response and start the polling mechanism to maintain it current.
         mpStunClient = new StunClient( mStunServer );
         
         UtlString publicIpAddress;
         if( mpStunClient->getPublicIpAddress( mStunServer, publicIpAddress ) )
         {
            announceStunResolvedPublicIpAddress( publicIpAddress );
         }
         else
         {
            UtlString hostIpAddress;
            OsSocket::getHostIp( &hostIpAddress );
            mPublicTransport.setAddress( hostIpAddress );
            OsSysLog::add(FAC_NAT, PRI_ERR, "NatTraversalRules::initializeNatTraversalInfo - failed to contact STUN server %s - using host IP %s as public", mStunServer.data(), mPublicTransport.getAddress().data() );
            
            if( mMediaRelayPublicAddress.isNull() )
            {
               // the admin did not explictly configure an IP address for the media relay, assume
               // that is was to be discovered via STUN as well.
               mMediaRelayPublicAddress = hostIpAddress;
            }
            publicIpAddress = hostIpAddress;
         }
         mpStunClient->maintainPublicIpAddressCurrent( this, mStunServer, mStunRefreshIntervalInSecs, publicIpAddress ); 
      }
   }
   else
   {
      OsSysLog::add(FAC_NAT, PRI_ERR, "NatTraversalRules::initializeNatTraversalInfo - No child Node named '%s'", XML_TAG_NAT_TRAVERSAL );
   }
}

void NatTraversalRules::announceStunResolvedPublicIpAddress( const UtlString& discoveredPublicIpAddress )
{
   //TODO: add mutual exclusion for accessing IP addresses.
   if( mbDiscoverPublicIpAddressViaStun )
   {
      mPublicTransport.setAddress( discoveredPublicIpAddress );
      
      if( mMediaRelayPublicAddress.isNull() )
      {
         // the admin did not explictly configure an IP address for the media relay, assume
         // that is was to be discovered via STUN as well.
         mMediaRelayPublicAddress = discoveredPublicIpAddress;
      }
      OsSysLog::add(FAC_NAT, PRI_INFO, "NatTraversalRules::announceStunResolvedPublicIpAddress - new public IP address %s discovered through STUN", discoveredPublicIpAddress.data() );
   }
}

bool NatTraversalRules::isEnabled( void ) const
{
   return mbNatTraveralEnabled;
}

bool NatTraversalRules::isBehindNat( void ) const
{
   return mbSystemBehindNat;
}

TransportData NatTraversalRules::getPublicTransportInfo( void ) const
{
   return mPublicTransport;
}

TransportData NatTraversalRules::getProxyTransportInfo( void ) const
{
   return mProxyTransport;
}

UtlString NatTraversalRules::getMediaRelayPublicAddress( void ) const
{
   return mMediaRelayPublicAddress;
}

UtlString NatTraversalRules::getMediaRelayNativeAddress( void ) const
{
   return mMediaRelayNativeAddress;
}

int NatTraversalRules::getMediaRelayXmlRpcPort( void ) const 
{
   return mMediaRelayXmlRpcPort;
}

int NatTraversalRules::getMaxMediaRelaySessions( void ) const
{
   return mMaxMediaRelaySessions;
}

bool NatTraversalRules::isAggressiveModeSet( void ) const
{
   return mbAggressiveModeSet;
}

bool NatTraversalRules::isConservativeModeSet( void ) const
{
   return !mbAggressiveModeSet;
}

bool NatTraversalRules::isPartOfLocalTopology( const UtlString& host, bool bCheckIpSubnets, bool bCheckDnsWidlcards ) const
{
   bool bHostIsPartOfLocalTopology = false;
   
   if( bCheckIpSubnets )
   {
      UtlSListIterator mlocalIpV4SubnetsIter( mlocalIpV4Subnets );
      UtlString* pIpV4Subnet;
      while( !bHostIsPartOfLocalTopology && ( pIpV4Subnet = dynamic_cast<UtlString*>(mlocalIpV4SubnetsIter() ) ) )
      {
         if( mpPatterns->IPv4subnet( host, *pIpV4Subnet ) )
         {
            bHostIsPartOfLocalTopology = true;
         }
      }
   }
   
   if( bCheckDnsWidlcards )
   {
      UtlSListIterator mlocalDnsWildcardsIter( mlocalDnsWildcards );
      UtlString* pDnsWildcard;
      while( !bHostIsPartOfLocalTopology && ( pDnsWildcard = dynamic_cast<UtlString*>(mlocalDnsWildcardsIter() ) ) )
      {
         if( mpPatterns->DnsWildcard( host, *pDnsWildcard ) )
         {
            bHostIsPartOfLocalTopology = true;
         }
      }
   }
   return bHostIsPartOfLocalTopology;
}

UtlString NatTraversalRules::getStunServer( void ) const
{
   return mStunServer;
}

int NatTraversalRules::getStunRefreshIntervalInSecs( void ) const
{
   return mStunRefreshIntervalInSecs;
}

NatTraversalRules::StunClient::StunClient( const UtlString& stunServer ) :
   mTimerMutex( OsMutex::Q_FIFO ),
   mSocket( STUN_PORT, stunServer ),
   mpNatTraversalRulesToKeepCurrent( 0 )
{
   mTimerMutex.acquire();
}

NatTraversalRules::StunClient::~StunClient()
{
}

bool NatTraversalRules::StunClient::getPublicIpAddress( const UtlString& stunServer, UtlString& discoveredPublicIpAddress )
{
   UtlString mappedAddress;
   int  mappedPort;
   OsTime timeout( STUN_QUERY_TIMEOUT_IN_MILLISECS );

   stunQueryAgent.setServer( stunServer );
   bool rc = stunQueryAgent.getMappedAddress( &mSocket, mappedAddress, mappedPort, 0, timeout );
   if( rc )
   {
      discoveredPublicIpAddress = mappedAddress;
      OsSysLog::add(FAC_NAT,PRI_INFO,"StunClient::getPublicIpAddress obtained public IP address %s from server %s", mappedAddress.data(), stunServer.data() );
   }
   else
   {
      discoveredPublicIpAddress.remove( 0 );
      OsSysLog::add(FAC_NAT,PRI_ERR,"StunClient::getPublicIpAddress failed to obtain mapping from server %s", stunServer.data() );
   }
   return rc;   
}

void NatTraversalRules::StunClient::maintainPublicIpAddressCurrent( NatTraversalRules* pNatTraversalRulesToKeepCurrent, 
                                     const UtlString& stunServer, int refreshIntervalInSecs, 
                                     const UtlString& publicIpAddressHint )
{
   mpNatTraversalRulesToKeepCurrent = pNatTraversalRulesToKeepCurrent;
   if( !publicIpAddressHint.isNull() )
   {
      mPublicIpAddressObtainedFromLastPoll = publicIpAddressHint;
   }
   start();
}

int NatTraversalRules::StunClient::run( void* runArg )
{
   UtlString discoveredPublicIpAddress; 
   OsStatus rc;
   while( !isShuttingDown() )
   {
      rc = mTimerMutex.acquire( mpNatTraversalRulesToKeepCurrent->getStunRefreshIntervalInSecs() * 1000 );
      if( rc == OS_WAIT_TIMEOUT )
      {
         if( getPublicIpAddress( mpNatTraversalRulesToKeepCurrent->getStunServer(), discoveredPublicIpAddress ) == true )
         {
            if( discoveredPublicIpAddress.compareTo( mPublicIpAddressObtainedFromLastPoll ) != 0 )
            {
               mPublicIpAddressObtainedFromLastPoll = discoveredPublicIpAddress;
               mpNatTraversalRulesToKeepCurrent->announceStunResolvedPublicIpAddress( discoveredPublicIpAddress );            
            }
         }
      }
   }   
   return 0;   
}
void NatTraversalRules::StunClient::requestShutdown( void )
{
   mTimerMutex.release();
   OsTask::requestShutdown();
}

