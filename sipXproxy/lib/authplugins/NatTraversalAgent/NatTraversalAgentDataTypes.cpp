//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include "os/OsSysLog.h"
#include "os/OsDateTime.h"
// APPLICATION INCLUDES
#include "NatTraversalAgentDataTypes.h"
#include "NatTraversalRules.h"
#include "MediaRelay.h"
#include "SipRouter.h"
#include "net/SdpBody.h"
#include "sipdb/RegistrationDB.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS


NativeTransportData::NativeTransportData( const Url& url )
{
   mTransportType.remove( 0 );
   mTransportType.append( "Private" );
   fromUrl( url );
}

void NativeTransportData::fromUrl( const Url& url )
{
   UtlString mappingInformationString;
   if( (const_cast<Url&>(url)).getUrlParameter( SIPX_PRIVATE_CONTACT_URI_PARAM, mappingInformationString, 0 ) )
   {
      // A native IP address is carried by our custom x-sipX-privcontact, use it to init transport data.
       Url tempUrl;
       tempUrl.fromString( mappingInformationString, true );
       TransportData::fromUrl( tempUrl );
   }
   else
   {
      // None of our proprietary headers are present.  The URL contains the native IP address then - use
      // it to initialize the transport data.
      TransportData::fromUrl( url );
   }
}

PublicTransportData::PublicTransportData( const Url& url )
{
   mTransportType.remove( 0 );
   mTransportType.append( "Public" );
   fromUrl( url );
}

void PublicTransportData::fromUrl( const Url& url )
{
   UtlString dummy;
   if( (const_cast<Url&>(url)).getUrlParameter( SIPX_PRIVATE_CONTACT_URI_PARAM, dummy, 0 ) ||
       (const_cast<Url&>(url)).getUrlParameter( SIPX_NO_NAT_URI_PARAM, dummy, 0 ) )
   {
      // The contact header has one of our proprietary URL parameters therefore indicating that the
      // IP, port & transport in the contact represent the UA's public IP address.  Use it to set the
      // public transport data values encapsulated by this class.
      TransportData::fromUrl( url );
   }
   else
   {
      // No custom headers are available to help us guess the public IP address of the device.
      // Set public IP address as unknown.
      mAddress.remove( 0 );
      mAddress.append( UNKNOWN_IP_ADDRESS_STRING );
   }
}

TransportData::TransportData() :
   mTransportType( UNKNOWN_TRANSPORT_STRING ),
   mAddress( UNKNOWN_IP_ADDRESS_STRING ),
   mPort( UNKNOWN_PORT_NUMBER ),
   mTransportProtocol( UNKNOWN_TRANSPORT_STRING )
{
}

TransportData::TransportData( const Url& url ) :
   mTransportType( UNKNOWN_TRANSPORT_STRING )
{
   fromUrl( url );
}

TransportData::TransportData( const UtlString& ipAddress,
                              uint16_t portNumber,
                              const UtlString& transportProtocol ) :
   mTransportType( UNKNOWN_TRANSPORT_STRING ),
   mAddress( ipAddress ),
   mPort( portNumber ),
   mTransportProtocol( transportProtocol )
{
}

const UtlContainableType TransportData::TYPE = "TransportData";

UtlContainableType TransportData::getContainableType() const
{
   return TransportData::TYPE;
}

unsigned TransportData::hash() const
{
   unsigned hash = mPort;
   size_t ipAddressLength = mAddress.length();
   if( ipAddressLength > 1 )
   {
      hash *= mAddress( ipAddressLength - 1 );
      hash *= mAddress( ipAddressLength - 2 );
   }
   return hash;
}

int TransportData::compareTo(UtlContainable const *rhs) const
{
   int result = -1;
   if ( rhs->isInstanceOf( TransportData::TYPE ) )
   {
      TransportData* pRhsTransportData = (TransportData*)rhs;

      if( mPort == pRhsTransportData->mPort )
      {
         if( ( result = mAddress.compareTo( pRhsTransportData->getAddress() ) ) == 0 )
         {
            result = mTransportProtocol.compareTo( pRhsTransportData->getTransportProtocol() );
         }
      }
      else
      {
         result = mPort < pRhsTransportData->mPort ? -1 : 1;
      }
   }
   return result;
}

void TransportData::fromUrl( const Url& url )
{
   url.getHostAddress( mAddress );
   mPort = url.getHostPort();
   if( mPort == PORT_NONE )
   {
      UtlString urlType;
      url.getUrlType( urlType );
      mPort = urlType.compareTo( SIPS_URL_TYPE, UtlString::ignoreCase ) == 0 ? SIP_TLS_PORT : SIP_PORT;
   }
   if( ! (const_cast<Url&>(url)).getUrlParameter( "transport", mTransportProtocol, 0 ) )
   {
      mTransportProtocol = "udp";
   }
}

void TransportData::toUrlString( UtlString& outputString ) const
{
   char portString[21];
   sprintf( portString, "%d", mPort );

   outputString.remove(0);
   outputString.append( mAddress );
   outputString.append( ":" );
   outputString.append( portString );
   outputString.append( ";transport=" );
   outputString.append( mTransportProtocol );
}

//GETTERS
const UtlString& TransportData::getAddress( void ) const
{
   return mAddress;
}

int TransportData::getPort( void ) const
{
   return mPort;
}

const UtlString& TransportData::getTransportProtocol( void ) const
{
   return mTransportProtocol;
}

const UtlString& TransportData::getTransportDataType( void ) const
{
   return mTransportType;
}

//SETTERS
void TransportData::setAddress( const UtlString& address )
{
   mAddress = address;
}

void TransportData::setPort( int port )
{
   mPort = port;
}

void TransportData::setTransportProtocol( const UtlString& transportProtocol )
{
   mTransportProtocol = transportProtocol;
}

bool TransportData::isEqual( const TransportData& rhs ) const
{
   return ( mAddress.compareTo( rhs.getAddress() ) == 0 ) &&
          ( mTransportProtocol.compareTo( rhs.getTransportProtocol() ) == 0 ) &&
          ( mPort == rhs.getPort() );
}

bool TransportData::isInitialized( void ) const
{
   return mAddress.compareTo( UNKNOWN_IP_ADDRESS_STRING );
}

EndpointDescriptor::EndpointDescriptor( const Url& url, const NatTraversalRules& natRules ) :
   mNativeTransport( url ),
   mPublicTransport( url )
{
   mLocation = computeLocation( natRules );
}

EndpointDescriptor::EndpointDescriptor( const Url& url, const NatTraversalRules& natRules, const RegistrationDB* pRegistrationDB ) :
   mNativeTransport( url ),
   mPublicTransport( url )
{
   mLocation = computeLocation( natRules );
   if( mLocation == UNKNOWN && pRegistrationDB )
   {
      // we could not establish the location of the user however we got supplied with
      // a pointer to the registration DB.  Search the Registration DB looking for a
      // Contact entry matching the supplied URI's user@hostport hoping to find
      // the location markers that are needed to establish the location of the endpoint.

      // extract user@hostport information from Request-URI
      UtlString stringToMatch;
      url.getIdentity( stringToMatch );

      int timeNow = OsDateTime::getSecsSinceEpoch();
      UtlSList resultList;
      pRegistrationDB->getUnexpiredContactsFieldsContaining( stringToMatch, timeNow, resultList );
      UtlString* pMatchingContact;
      if( !resultList.isEmpty() )
      {
         pMatchingContact = (UtlString*)resultList.first();
         Url urlWithLocationInformation( *pMatchingContact );
         mPublicTransport.fromUrl( urlWithLocationInformation );
         mNativeTransport.fromUrl( urlWithLocationInformation );

         OsSysLog::add(FAC_NAT, PRI_INFO, "EndpointDescriptor::EndpointDescriptor[1]: Retrieved location info for UNKNOWN user from regDB:'%s'",
               pMatchingContact->data() );

         //update location information based on new information
         mLocation = computeLocation( natRules );
      }
      else
      {
         // no match for the supplied identity - make one last attempt to recover location markers
         // from the registration DB by trying to look for a contact that has the URL's hostport matching
         // a DB entry's sipx-privcontact and the URL's user matching a DB entry's contact user part
         UtlString userIdToMatch;
         url.getUserId( userIdToMatch );
         
         UtlString host;
         int port;
         url.getHostAddress( host );
         port = url.getHostPort();
         UtlString tmpStringToMatch = SIPX_PRIVATE_CONTACT_URI_PARAM;
         tmpStringToMatch.append('='); 
         tmpStringToMatch.append( host );
         if( port != PORT_NONE )
         {
            tmpStringToMatch.append("%3A");   // %3A is the escaped version of the '=' character.
            tmpStringToMatch.appendNumber( port );
         }

OsSysLog::add(FAC_NAT, PRI_INFO, "bobjoly %s", tmpStringToMatch.data() );

         
         pRegistrationDB->getUnexpiredContactsFieldsContaining( tmpStringToMatch, timeNow, resultList );
         UtlSListIterator iter( resultList );
         UtlContainable* pEntry;
         while( ( pEntry = iter() ) != NULL )
         {
            pMatchingContact = (UtlString*)pEntry;
OsSysLog::add(FAC_NAT, PRI_INFO, "bobjoly got one possible match %s", pMatchingContact->data() );
            
            Url urlWithLocationInformation( *pMatchingContact );
            UtlString userId;
            urlWithLocationInformation.getUserId( userId );
            if( userId.compareTo( userIdToMatch ) == 0 )
            {  
               mPublicTransport.fromUrl( urlWithLocationInformation );
               mNativeTransport.fromUrl( urlWithLocationInformation );
   
               OsSysLog::add(FAC_NAT, PRI_INFO, "EndpointDescriptor::EndpointDescriptor[2]: Retrieved location info for UNKNOWN user from regDB:'%s'",
                     pMatchingContact->data() );
   
               //update location information based on new information
               mLocation = computeLocation( natRules );
               break;
            }
         }
      }
      resultList.destroyAll();
   }
}


const TransportData& EndpointDescriptor::getNativeTransportAddress( void ) const
{
   return mNativeTransport;
}

const TransportData& EndpointDescriptor::getPublicTransportAddress( void ) const
{
   return mPublicTransport;
}

LocationCode EndpointDescriptor::getLocationCode( void ) const
{
   return mLocation;
}

LocationCode EndpointDescriptor::computeLocation( const NatTraversalRules& natRules )
{
   LocationCode computedLocation = UNKNOWN;

   // Check is the Public IP address in known...
   if( mPublicTransport.isInitialized() )
   {
      // the Public IP address can be determined.  Figure out the location of the
      // endpoint based on it.
      if( mPublicTransport.getAddress() == mNativeTransport.getAddress() )
      {
         // public IP address matches the endpoint's native IP address.  This means that
         // there are no NATs between the endpoint and the sipXecs.  Three possibilities exist:
         //  o Endpoint is in same subnet as sipXecs and sipXecs is behind NAT => location is LOCAL_NATED
         //  o Endpoint is in same subnet as sipXecs and sipXecs is NOT behind a NAT => location is PUBLIC
         //  o Endpoint is NOT in same subnet as sipXecs => location is PUBLIC
         if( natRules.isPartOfLocalTopology( mPublicTransport.getAddress(), true, true ) )
         {
            if( natRules.isBehindNat() )
            {
               computedLocation = LOCAL_NATED;
            }
            else
            {
               computedLocation = PUBLIC;
            }
         }
         else
         {
            computedLocation = PUBLIC;
         }
      }
      else
      {
         // public and native IP addresses do not match.  This means that the endpoint is in a remote
         // network that is fronted by a NAT. Location is therefore remote NATed.
         computedLocation = REMOTE_NATED;
      }
   }
   else
   {

      // the public IP address is not known.  Examine the endpoint's native IP address to find out if
      // it resides within our local private network
      if( natRules.isPartOfLocalTopology( mNativeTransport.getAddress(), true, true ) )
      {
         // the endpoint is actually on this machine.  It could be for VM, AA, ACD or some
         // other media server endpoint.  Check it the sipXecs is behind a NAT...
         if( natRules.isBehindNat() )
         {
            // The endpoint is sipXecs and it is behind a NAT, location is therefore LOCAL_NATED
            computedLocation = LOCAL_NATED;
         }
         else
         {
            // The endpoint is sipXecs and it is NOT behind a NAT, location is therefore PUBLIC
            computedLocation = PUBLIC;
         }
      }
      else
      {
         // we have not information about the location of the public address, Location is therefore UNKNOWN
         computedLocation = UNKNOWN;
      }
   }
   return computedLocation;
}

void EndpointDescriptor::toString( UtlString& outputString ) const
{
   UtlString tmpString;
   outputString.remove(0);
   outputString.append( "Native Transport: ");
   mNativeTransport.toUrlString( tmpString );
   outputString.append( tmpString );
   outputString.append( "; Public Transport: " );
   mPublicTransport.toUrlString( tmpString );
   outputString.append( tmpString );

   switch( mLocation )
   {
   case PUBLIC:
      outputString.append( "; Location = PUBLIC" );
      break;
   case REMOTE_NATED:
      outputString.append( "; Location = REMOTE_NATED" );
      break;
   case LOCAL_NATED:
      outputString.append( "; Location = LOCAL_NATED" );
      break;
   case UNKNOWN:
   default:
      outputString.append( "; Location = UNKNOWN" );
      break;
   }
}

MediaEndpoint::MediaEndpoint( const MediaEndpoint& referenceMediaEndpoint )
{
   *this = referenceMediaEndpoint;
}

MediaEndpoint::MediaEndpoint( const SdpBody& sdpBody, size_t mediaDescriptionIndex )
{
   setData( sdpBody, mediaDescriptionIndex );
}

MediaEndpoint::MediaEndpoint() :
   mAddress ( "" ),
   mRtpPort ( 0 ),
   mRtcpPort( 0 )
{
}

bool MediaEndpoint::setData( const SdpBody& sdpBody, size_t mediaDescriptionIndex )
{
   UtlString newAddress;
   int newRtpPort;
   int newRtcpPort;
   bool bNewDataDiffersFromOld;

   sdpBody.getMediaAddress(  mediaDescriptionIndex, &newAddress ) ;
   sdpBody.getMediaPort(     mediaDescriptionIndex, &newRtpPort);
   sdpBody.getMediaRtcpPort( mediaDescriptionIndex, &newRtcpPort );

   if( mRtpPort  == newRtpPort  &&
       mRtcpPort == newRtcpPort &&
       mAddress  == newAddress )
   {
      bNewDataDiffersFromOld = false;
   }
   else
   {
      bNewDataDiffersFromOld = true;
      mRtpPort  = newRtpPort;
      mRtcpPort = newRtcpPort;
      mAddress  = newAddress;
   }
   return bNewDataDiffersFromOld;
}

const UtlString& MediaEndpoint::getAddress ( void ) const
{
   return mAddress;
}

int MediaEndpoint::getRtpPort( void ) const
{
   return mRtpPort;
}

int MediaEndpoint::getRtcpPort( void ) const
{
   return mRtcpPort;
}

MediaEndpoint& MediaEndpoint::operator=( const MediaEndpoint& rhs )
{
   if( this != &rhs )
   {
      mAddress  = rhs.mAddress;
      mRtpPort  = rhs.mRtpPort;
      mRtcpPort = rhs.mRtcpPort;
   }
   return *this;
}

MediaDescriptor::MediaDescriptor( const MediaDescriptor& referenceMediaDescriptor )
{
   *this = referenceMediaDescriptor;
}

MediaDescriptor::MediaDescriptor( const SdpBody& sdpBody, size_t index, EndpointRole endpointRole )  :
   mMediaDescriptionIndex               ( index ),
   mCurrentMediaRelayHandle             ( INVALID_MEDIA_RELAY_HANDLE ),
   mTentativeInitialMediaRelayHandle    ( INVALID_MEDIA_RELAY_HANDLE ),
   mTentativeNonInitialMediaRelayHandle ( INVALID_MEDIA_RELAY_HANDLE )
{
   setMediaTypeAndDirectionalityData( sdpBody, index );
   setEndpointData( sdpBody, index, endpointRole );
}

const UtlString& MediaDescriptor::getType( void ) const
{
   return mType;
}

MediaDirectionality MediaDescriptor::getDirectionality( void ) const
{
   return mDirectionality;
}

void MediaDescriptor::setDirectionalityOverride( MediaDirectionality override )
{
   mDirectionalityOverride = override;
}

MediaDirectionality MediaDescriptor::getDirectionalityOverride( void ) const
{
   return mDirectionalityOverride;
}

void MediaDescriptor::setMediaTypeAndDirectionalityData( const SdpBody& sdpBody, size_t index )
{
   sdpBody.getMediaType( index, &mType );
   mDirectionality         = sdpDirectionalityAttributeToMediaDirectionalityValue( sdpBody, index );
   mDirectionalityOverride = NOT_A_DIRECTION;
}

MediaDirectionality MediaDescriptor::sdpDirectionalityAttributeToMediaDirectionalityValue( const SdpBody& sdpBody, size_t index )
{
   MediaDirectionality directionalityValue;
   if( sdpBody.getMediaAttribute( index, "sendrecv" ) )
   {
      directionalityValue = SEND_RECV;
   }
   else if( sdpBody.getMediaAttribute( index, "sendonly" ) )
   {
      directionalityValue = SEND_ONLY;
   }
   else if( sdpBody.getMediaAttribute( index, "recvonly" ) )
   {
      directionalityValue = RECV_ONLY;
   }
   else if( sdpBody.getMediaAttribute( index, "inactive" ) )
   {
      directionalityValue = INACTIVE;
   }
   else
   {
      directionalityValue = SEND_RECV;
   }
   return directionalityValue;
}

void MediaDescriptor::mediaDirectionalityValueToSdpDirectionalityAttribute( const MediaDirectionality valueToConvert, UtlString& conversion )
{
   switch( valueToConvert )
   {
      case SEND_RECV:
         conversion = "sendrecv";
         break;
      case SEND_ONLY:
         conversion = "sendonly";
         break;
      case RECV_ONLY:
         conversion = "recvonly";
         break;
      case INACTIVE:
         conversion = "inactive";
         break;
      default:
         conversion = "unknown_direction";
         break;
   }
}

const MediaEndpoint& MediaDescriptor::getEndpoint( EndpointRole endpointRole ) const
{
   if( endpointRole == CALLER )
   {
      return getCallerEndpoint();
   }
   else
   {
      return getCalleeEndpoint();
   }
}

bool MediaDescriptor::setEndpointData( const SdpBody& sdpBody,
                                       size_t index,
                                       EndpointRole endpointRole )
{
   if( endpointRole == CALLER )
   {
      return setCallerEndpointData( sdpBody, index );
   }
   else
   {
      return setCalleeEndpointData( sdpBody, index );
   }
}

const MediaEndpoint& MediaDescriptor::getCallerEndpoint( void ) const
{
   return mCaller;
}

bool MediaDescriptor::setCallerEndpointData( const SdpBody& sdpBody, size_t index )
{
   return mCaller.setData( sdpBody, index );
}

const MediaEndpoint& MediaDescriptor::getCalleeEndpoint( void ) const
{
   return mCallee;
}

bool MediaDescriptor::setCalleeEndpointData( const SdpBody& sdpBody, size_t index )
{
   return mCallee.setData( sdpBody, index );
}

void MediaDescriptor::setCurrentMediaRelayHandle( const tMediaRelayHandle handle )
{
   mCurrentMediaRelayHandle = handle;
}

tMediaRelayHandle MediaDescriptor::getCurrentMediaRelayHandle( void ) const
{
   return mCurrentMediaRelayHandle;
}

void MediaDescriptor::clearCurrentMediaRelayHandle( void )
{
   mCurrentMediaRelayHandle = INVALID_MEDIA_RELAY_HANDLE;
}


void MediaDescriptor::setTentativeInitialMediaRelayHandle( const tMediaRelayHandle handle )
{
   mTentativeInitialMediaRelayHandle = handle;
}

tMediaRelayHandle MediaDescriptor::getTentativeInitialMediaRelayHandle( void ) const
{
   return mTentativeInitialMediaRelayHandle;
}

void MediaDescriptor::clearTentativeInitialMediaRelayHandle( void )
{
   mTentativeInitialMediaRelayHandle = INVALID_MEDIA_RELAY_HANDLE;
}

void MediaDescriptor::setTentativeNonInitialMediaRelayHandle( const tMediaRelayHandle handle )
{
   mTentativeNonInitialMediaRelayHandle = handle;
}

tMediaRelayHandle MediaDescriptor::getTentativeNonInitialMediaRelayHandle( void ) const
{
   return mTentativeNonInitialMediaRelayHandle;
}

void MediaDescriptor::clearTentativeNonInitialMediaRelayHandle( void )
{
   mTentativeNonInitialMediaRelayHandle = INVALID_MEDIA_RELAY_HANDLE;
}

MediaDescriptor& MediaDescriptor::operator=( const MediaDescriptor& rhs )
{
   if( this != &rhs )
   {
      mType                                = rhs.mType;
      mMediaDescriptionIndex               = rhs.mMediaDescriptionIndex;
      mDirectionality                      = rhs.mDirectionality;
      mDirectionalityOverride              = rhs.mDirectionalityOverride;
      mCaller                              = rhs.mCaller;
      mCallee                              = rhs.mCallee;
      mCurrentMediaRelayHandle             = rhs.mCurrentMediaRelayHandle;
      mTentativeInitialMediaRelayHandle    = rhs.mTentativeInitialMediaRelayHandle;
      mTentativeNonInitialMediaRelayHandle = rhs.mTentativeNonInitialMediaRelayHandle;
   }
   return *this;
}

MediaRelaySession::MediaRelaySession( const tMediaRelayHandle& uniqueHandle,
                                      int callerPort,
                                      int calleePort,
                                      MediaBridgePair *pAssociatedMediaBridgePair,
                                      bool isaCloneOfAnotherMediaRelaySession ) :
   mUniqueHandle( uniqueHandle ),
   mCallerRtpPort( callerPort ),
   mCalleeRtpPort( calleePort ),
   mbIsaCloneOfAnotherMediaRelaySession( isaCloneOfAnotherMediaRelaySession ),
   mpAssociatedMediaBridgePair( pAssociatedMediaBridgePair ),
   mLinkCount( 1 )
{
   // Figure out if the RTP ports have been swapped between the caller and the callee.
   // This can only happen when a MediaRelaySession gets cloned.
   if( mbIsaCloneOfAnotherMediaRelaySession )
   {
      // By convention Sym1 is used to track the caller's information.  Check to see
      // if the port of Sym1 of the RTP bridge matches the caller port passed as a
      // parameter.  If so, no swapped has happened.
      mbCallerAndCalleeRtpPortsSwapped =
         ( callerPort == pAssociatedMediaBridgePair->getRtpBridge()->getEndpoint1Sym()->getPort() ) ? false : true;
   }
}

int MediaRelaySession::getRtpRelayPort( EndpointRole endpointRole ) const
{
   if( endpointRole == CALLER )
   {
      return mCallerRtpPort;
   }
   else
   {
      return mCalleeRtpPort;
   }
}

bool MediaRelaySession::isaCloneOfAnotherMediaRelaySession( void ) const
{
   return mbIsaCloneOfAnotherMediaRelaySession;
}

ssize_t MediaRelaySession::getLinkCount( void ) const
{
   return mLinkCount;
}

ssize_t MediaRelaySession::incrementLinkCount( void )
{
   return ++mLinkCount;
}

ssize_t MediaRelaySession::decrementLinkCount( void )
{
   if( mLinkCount )
   {
      mLinkCount--;
   }
   return mLinkCount;
}

void MediaRelaySession::setPacketProcessingStats( const PacketProcessingStatistics& newStats )
{
   mPacketProcessingStats = newStats;
}

const UtlContainableType MediaRelaySession::TYPE = "MediaRelaySession";

UtlContainableType MediaRelaySession::getContainableType() const
{
   return MediaRelaySession::TYPE;
}

unsigned MediaRelaySession::hash() const
{
   const intptr_t handle = mUniqueHandle;
   return handle;
}

int MediaRelaySession::compareTo(UtlContainable const *rhs ) const
{
   int result = -1;
   if ( rhs->isInstanceOf( MediaRelaySession::TYPE ) )
   {
      if( (intptr_t)mUniqueHandle == (intptr_t)(((MediaRelaySession*)rhs)->mUniqueHandle ) )
      {
         result = 0;
      }
      else
      {
         result = ( (intptr_t)mUniqueHandle > (intptr_t)(((MediaRelaySession*)rhs)->mUniqueHandle ) );
      }
   }
   return result;
}

PacketProcessingStatistics::PacketProcessingStatistics( void ) :
   mNumberOfPacketsProcessed( 0 ),
   mEpochTimeOfLastPacketsProcessed( OsDateTime::getSecsSinceEpoch() )
{
}
