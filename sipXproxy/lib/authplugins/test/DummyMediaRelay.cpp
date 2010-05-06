// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "MediaRelay.h"

// STATIC INITIALIZATIONS
const UtlContainableType Sym::TYPE                 = "Sym";
const UtlContainableType Bridge::TYPE              = "Bridge";
const UtlContainableType MediaBridgePair::TYPE     = "MediaBridgePair";
const UtlContainableType AsynchMediaRelayMsg::TYPE = "AsynchMediaRelayMsg";

// DEFINES

// TEST VARIABLES 
static bool sbFakeReceptionOfPackets = false;

// TEST FUNCTIONS
void SetFakeReceptionOfPacketsFlag( bool bFakeReceptionOfPackets )
{
   sbFakeReceptionOfPackets = bFakeReceptionOfPackets;
}

MediaRelay::MediaRelay() :
   mbSignedInWithSymmitron( false ),
   mAsynchMediaRelayRequestSender( this ),
   mMutex( OsMutex::Q_FIFO ),
   mGenericTimer( *this ),
   mGenericTimerTickCounter( 0 ),
   mbPollForSymmitronRecovery( false )

{
   mRelaySessionHandle = 0;
}

bool MediaRelay::initialize( const  UtlString& publicAddress, 
                             const  UtlString& nativeAddress,
                             bool   bXmlRpcSecured,                             
                             bool   isPartOfsipXLocalPrivateNetwork,
                             int    xmlRpcPort,
                             size_t maxMediaRelaySessions )
{
   mPublicAddress = publicAddress;
   mNativeAddress = nativeAddress;
   mbIsPartOfsipXLocalPrivateNetwork = isPartOfsipXLocalPrivateNetwork;
   mXmlRpcPort = xmlRpcPort;
   mMaxMediaRelaySessions = maxMediaRelaySessions;
   mRelaySessionHandle = 0;
   return true;
}

MediaRelay::~MediaRelay()
{
}

bool MediaRelay::allocateSession( tMediaRelayHandle& relayHandle, int& endpoint1RelayRtpPort, int& endpoint2RelayRtpPort )
{
   endpoint1RelayRtpPort = 10000 + ( mRelaySessionHandle * 2 );
   endpoint2RelayRtpPort = 11000 + ( mRelaySessionHandle * 2 );
   relayHandle = mRelaySessionHandle++;

   UtlString dummyId("11111");
   UtlString mediaRelayPrivateIp("192.168.0.2");
   Sym* pCallerRtpSym = new Sym( dummyId, mediaRelayPrivateIp, endpoint1RelayRtpPort );
   Sym* pCallerRtcpSym = new Sym( dummyId, mediaRelayPrivateIp, endpoint1RelayRtpPort + 1 );
   Sym* pCalleeRtpSym = new Sym( dummyId, mediaRelayPrivateIp, endpoint2RelayRtpPort );
   Sym* pCalleeRtcpSym = new Sym( dummyId, mediaRelayPrivateIp, endpoint2RelayRtpPort + 1 );
   Bridge* pCallerBridge = new Bridge( dummyId, pCallerRtpSym, pCalleeRtpSym );
   Bridge* pCalleeBridge = new Bridge( dummyId, pCallerRtcpSym, pCalleeRtcpSym );
   MediaBridgePair* pMediaBridgePair = new MediaBridgePair( pCallerBridge, pCalleeBridge );
   
   MediaRelaySession* pMediaRelaySession = new MediaRelaySession( relayHandle, endpoint1RelayRtpPort, endpoint2RelayRtpPort, pMediaBridgePair );
   tMediaRelayHandle* pHandle = new tMediaRelayHandle( relayHandle );
   mActiveMediaRelaySessions.insertKeyAndValue( pHandle, pMediaRelaySession );
   return true;
}

tMediaRelayHandle MediaRelay::cloneSession( const tMediaRelayHandle& relayHandleToClone, bool doSwapCallerAndCallee )
{
   const MediaRelaySession* pMediaRelaySessionToClone;
   tMediaRelayHandle relayHandleOfClone = INVALID_MEDIA_RELAY_HANDLE;
   
   if( ( pMediaRelaySessionToClone = getSessionByHandle( relayHandleToClone ) ) )
   {
      int callerRtpPort = pMediaRelaySessionToClone->getRtpRelayPort( CALLER );
      int calleeRtpPort = pMediaRelaySessionToClone->getRtpRelayPort( CALLEE );

      if( doSwapCallerAndCallee )
      {
         int tempPort  = callerRtpPort;
         callerRtpPort = calleeRtpPort;
         calleeRtpPort = tempPort; 
      }
      
      relayHandleOfClone = mRelaySessionHandle++;
      MediaRelaySession* pClonedMediaRelaySession = new MediaRelaySession( relayHandleOfClone, callerRtpPort, calleeRtpPort, 
                                                                           pMediaRelaySessionToClone->getAssociatedMediaBridgePair(), true );
      tMediaRelayHandle* pHandle = new tMediaRelayHandle( relayHandleOfClone );
      mActiveMediaRelaySessions.insertKeyAndValue( pHandle, pClonedMediaRelaySession );
   } 
   return relayHandleOfClone;

}

bool MediaRelay::deallocateSession( const tMediaRelayHandle& handle )
{
   
   if( handle != INVALID_MEDIA_RELAY_HANDLE )
   {
      const MediaRelaySession* pMediaRelaySession;
      
      if( ( pMediaRelaySession = getSessionByHandle( handle ) ) )
      {
         mActiveMediaRelaySessions.destroy( &handle );
      }
   }
   return true;
}

bool MediaRelay::setDirectionMode( const tMediaRelayHandle& handle, MediaDirectionality mediaRelayDirectionMode )
{
   return TRUE;
}

bool MediaRelay::linkSymToEndpoint( const tMediaRelayHandle& relayHandle, 
                                    const UtlString& endpointIpAddress, 
                                    int endpointRtpPort,
                                    int endpointRtcpPort,
                                    EndpointRole ownerOfSymToLink )
{
   return true;
}

bool MediaRelay::getPacketProcessingStatsForMediaRelaySession( const tMediaRelayHandle& handle, PacketProcessingStatistics& stats )
{
   if( sbFakeReceptionOfPackets )
   {
      stats.mEpochTimeOfLastPacketsProcessed = OsDateTime::getSecsSinceEpoch();
      stats.mNumberOfPacketsProcessed += 1000;
   }
   else
   {
      stats.mEpochTimeOfLastPacketsProcessed = 1;
   }
   return true;
}

OsStatus MediaRelay::signal( intptr_t eventData )
{
   return OS_SUCCESS;      
}

ssize_t MediaRelay::incrementLinkCountOfMediaRelaySession( const tMediaRelayHandle& handle )
{
   ssize_t result = 0;
   MediaRelaySession* pMediaRelaySession = 0;
   
   if( ( pMediaRelaySession = getSessionByHandle( handle ) ) )
   {
      result = pMediaRelaySession->incrementLinkCount();
   }
   return result;
}

int MediaRelay::getRtpRelayPortForMediaRelaySession( const tMediaRelayHandle& handle, EndpointRole endpointRole )
{
   int rtpPort = PORT_NONE;
   MediaRelaySession* pMediaRelaySession = 0;
   
   if( ( pMediaRelaySession = getSessionByHandle( handle ) ) )
   {
      rtpPort = pMediaRelaySession->getRtpRelayPort( endpointRole );
   }
   return rtpPort;
}

MediaRelaySession* MediaRelay::getSessionByHandle( const tMediaRelayHandle& handle )
{
   return (MediaRelaySession*)( mActiveMediaRelaySessions.findValue( &handle ) );
}

Sym::Sym( UtlString& id, UtlString& localAddress, int port ) :
   mId( id ),
   mLocalAddress( localAddress ),
   mPort( port )
{   
}

UtlString Sym::getId( void ) const
{
   return mId;
}

UtlString Sym::getLocalAddress( void ) const
{
   return mLocalAddress;
}

int Sym::getPort( void ) const
{
   return mPort;
}
UtlContainableType Sym::getContainableType() const
{
   return Sym::TYPE;
}

unsigned Sym::hash() const
{
   return directHash();
}

// evaluation is performed so that syms will be ordered by local IP, then
// by port number then by Id.
int Sym::compareTo(UtlContainable const *rhs ) const
{
   Sym *rhsSym = (Sym*)rhs;
   int rc;
   if( ( rc = mLocalAddress.compareTo( rhsSym->getLocalAddress() ) ) == 0 )
   {
      if( mPort != rhsSym->getPort() )
      {
         rc = ( mPort < rhsSym->getPort() ? -1 : 1 );
      }
      else
      {
         rc = mId.compareTo( rhsSym->getId() );
      }
         
   }
   return rc; 
}

Bridge::Bridge( UtlString& id, Sym* pEndpoint1Sym, Sym* pEndpoint2Sym ) :
   mId( id ),
   mpEndpoint1Sym( pEndpoint1Sym ),
   mpEndpoint2Sym( pEndpoint2Sym )
{
}

UtlString Bridge::getId( void ) const
{
   return mId;
}

const Sym* Bridge::getEndpoint1Sym( void ) const
{
  return mpEndpoint1Sym;
}

const Sym* Bridge::getEndpoint2Sym( void ) const
{
   return mpEndpoint2Sym;
}

UtlContainableType Bridge::getContainableType() const
{
   return Bridge::TYPE;
}

unsigned Bridge::hash() const
{
   return directHash();
}

int Bridge::compareTo(UtlContainable const *rhs ) const
{
   return mId.compareTo( ((Bridge*)rhs)->mId ); 
}

MediaBridgePair::MediaBridgePair( Bridge* pRtpBridge, Bridge* pRtcpBridge ) :
   mpRtpBridge(  pRtpBridge ),
   mpRtcpBridge( pRtcpBridge )
{
}
   
const Bridge* MediaBridgePair::getRtpBridge( void ) const
{ 
   return mpRtpBridge; 
}

const Bridge* MediaBridgePair::getRtcpBridge( void ) const
{ 
   return mpRtcpBridge;
}

UtlContainableType MediaBridgePair::getContainableType() const
{
   return MediaBridgePair::TYPE;
}

unsigned MediaBridgePair::hash() const
{
   return directHash();
}

int MediaBridgePair::compareTo(UtlContainable const *rhs ) const
{
   int result; 
   if( getRtpBridge() == static_cast<const MediaBridgePair*>(rhs)->getRtpBridge() )
   {
      if( getRtcpBridge() == static_cast<const MediaBridgePair*>(rhs)->getRtcpBridge() )
      {
         result = 0;
      }
      else
      {
         result = ( getRtcpBridge() > static_cast<const MediaBridgePair*>(rhs)->getRtcpBridge() ? 1 : -1 );  
      }
   }
   else
   {
      result = ( getRtpBridge() > static_cast<const MediaBridgePair*>(rhs)->getRtpBridge() ? 1 : -1 );  
   }
   return result;
}

AsynchMediaRelayRequestSender::AsynchMediaRelayRequestSender( MediaRelay* pOwningMediaRelay ) :
   OsServerTask( "AsynchMediaRelayRequestSender-%d" ),
   mpOwningMediaRelay( pOwningMediaRelay )
{
}

AsynchMediaRelayRequestSender::~AsynchMediaRelayRequestSender()
{
}


UtlBoolean AsynchMediaRelayRequestSender::handleMessage( OsMsg& rMsg )
{
   bool handled = TRUE;
   switch ( rMsg.getMsgType() )
   {
   case OsMsg::OS_SHUTDOWN:
      OsTask::requestShutdown();
      break;
   }
   return handled;
}

