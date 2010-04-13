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
#include "NatTraversalRules.h"
#include "alarm/Alarm.h"
#include "net/XmlRpcRequest.h"
#include "net/XmlRpcResponse.h"
#include "os/OsLock.h"
#include "os/OsSysLog.h"
#include "os/OsProcess.h"
#include "os/OsDateTime.h"
#include "os/OsTask.h"
#include "utl/UtlSListIterator.h"
#include "utl/UtlHashMapIterator.h"
#include "utl/UtlBool.h"

// STATIC INITIALIZATIONS
const UtlContainableType Sym::TYPE                 = "Sym";
const UtlContainableType Bridge::TYPE              = "Bridge";
const UtlContainableType MediaBridgePair::TYPE     = "MediaBridgePair";
const UtlContainableType AsynchMediaRelayMsg::TYPE = "AsynchMediaRelayMsg";

// DEFINES

// common name part of our handle
#define COMMON_HANDLE_NAME "ntap-mediarelay"

// sipXBridge XML-RPC methods
#define SIGN_IN_METHOD             "sipXrelay.signIn"
#define SIGN_OUT_METHOD            "sipXrelay.signOut"
#define CREATE_SYMS_METHOD         "sipXrelay.createSyms"
#define DESTROY_SYM_METHOD         "sipXrelay.destroySym"
#define PAUSE_SYM_METHOD           "sipXrelay.pauseSym"
#define RESUME_SYM_METHOD          "sipXrelay.resumeSym"
#define SET_DESTINATION_SYM_METHOD "sipXrelay.setDestination"
#define REMOVE_SYM_METHOD          "sipXrelay.removeSym"
#define ADD_SYM_METHOD             "sipXrelay.addSym"
#define CREATE_BRIDGE_METHOD       "sipXrelay.createBridge"
#define START_BRIDGE_METHOD        "sipXrelay.startBridge"
#define PAUSE_BRIDGE_METHOD        "sipXrelay.pauseBridge"
#define RESUME_BRIDGE_METHOD       "sipXrelay.resumeBridge"
#define GET_SYM_STATS_METHOD       "sipXrelay.getSymStatistics"
#define PING_METHOD                "sipXrelay.ping"
#define SET_SYM_TIMEOUT_METHOD     "sipXrelay.setTimeout"
#define GET_BRIDGE_STATS_METHOD    "sipXrelay.getBridgeStatistics"

// sipXBridge XML-RPC response member names
#define STATUS_CODE           "status-code"
#define ERROR_CODE            "faultCode"
#define ERROR_INFO            "faultString"
#define INSTANCE_HANDLE       "instance-handle"
#define BRIDGE_ID             "bridge-id"
#define SYM_SESSION           "sym"
#define SYM_ID                "id"
#define SYM_RECEIVER_DATA     "receiver"
#define SYM_RECEIVER_IP       "ipAddress"
#define SYM_RECEIVER_PORT     "port"
#define SYM_SESSION_STATS     "sym-stats"
#define CURRENT_TIME_OF_DAY   "current-time-of-day"
#define BRIDGE_STATE          "bridge-state"
#define CREATION_TIME         "creation-time"
#define LAST_PACKET_RECEIVED  "last-packet-received"
#define SESSION_STATE         "sym-state"
#define PACKETS_RECEIVED      "packets-received"
#define PACKETS_SENT          "packets-sent"
#define PACKETS_PROCESSED     "packets-processed"

// Status codes
#define OK                    "ok"
#define ERROR                 "error"

// Error codes
#define CONNECTION_FAILED    -6
#define SUCCESS              -1
#define ILLEGAL_ARGUMENT      0
#define HANDLE_NOT_FOUND      1
#define PROCESSING_ERROR      2
#define SESSION_NOT_FOUND     3
#define ILLEGAL_STATE         5
#define PORTS_NOT_AVAILABLE   6

// Parity codes
#define EVEN  1
#define ODD   2

// Misc
#define MAX_FAILED_CONNECTION_RECOVERY_ATTEMPTS                (1)
#define GENERIC_TIMER_IN_SECS                                  (30)
#define GENERIC_TIMER_TICKS_BEFORE_PING                        (1)
#define GENERIC_TIMER_TICKS_BEFORE_SYMMITRON_RECONNECT_ATTEMPT (1)
#define GENERIC_TIMER_TICKS_BEFORE_BRIDGE_STAT_QUERY           (10)
#define DEFAULT_RTP_KEEP_ALIVE_IN_MILLISECS                    (20000)
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

MediaRelay::MediaRelay() :
   mbSignedInWithSymmitron( false ),
   mAsynchMediaRelayRequestSender( this ),
   mMutex( OsMutex::Q_FIFO ),
   mGenericTimer( *this ),
   mGenericTimerTickCounter( 0 ),
   mbPollForSymmitronRecovery( false )
{
   // start the timer that will periodically ping the symmitron and query bridge stats
   OsTime genericTimerPeriod( GENERIC_TIMER_IN_SECS, 0 );
   mGenericTimer.periodicEvery( genericTimerPeriod, genericTimerPeriod );

   // seed random number generator
   srand( (unsigned)time(NULL) );   
}

bool MediaRelay::initialize( const  UtlString& publicAddress,
                             const  UtlString& nativeAddress,
                             bool   bXmlRpcSecured,
                             bool   isPartOfsipXLocalPrivateNetwork,
                             int    xmlRpcPort,
                             size_t maxMediaRelaySessions )
{
   OsLock lock( mMutex );
   mbSignedInWithSymmitron = false;
   mPublicAddress = publicAddress;
   mNativeAddress = nativeAddress;
   mbIsPartOfsipXLocalPrivateNetwork = isPartOfsipXLocalPrivateNetwork;
   mXmlRpcPort = xmlRpcPort;
   mMaxMediaRelaySessions = maxMediaRelaySessions;
   mRelaySessionHandle = 0;
   if( bXmlRpcSecured )
   {
      mSymmitronUrl.setUrlType("https");
   }
   else
   {
      mSymmitronUrl.setUrlType("http");
   }
   mSymmitronUrl.setHostAddress( nativeAddress.data() );
   mSymmitronUrl.setHostPort( xmlRpcPort );
   mAvailableMediaBridgePairsList.clear();
   mBusyMediaBridgePairsList.clear();

   return preAllocateSymmitronResources();
}

bool MediaRelay::preAllocateSymmitronResources( void )
{
   OsLock lock( mMutex );
   bool bInitializationSucceeded = false;
   UtlString tempSymmitronInstanceHandle;
   UtlString errorDescription;
   int errorCode;

   // create our handle which is our 'common name':'random number'.  This handle will be used to identify
   // this media relay instance to the Symmitron.  We generate a new one on every signIn as a defensive
   // coding measure in case our signOut() gets lost.
   char ourHandle[200];
   sprintf( ourHandle, "%s:%u", COMMON_HANDLE_NAME, rand() );
   mOurInstanceHandle = ourHandle;


   // Create the SignIn request for the symmitron
   XmlRpcRequest signInRequest( mSymmitronUrl, SIGN_IN_METHOD );
   signInRequest.addParam( &mOurInstanceHandle );

   XmlRpcResponse signInResponse;
   if( executeAndValudateSymmitronRequest( signInRequest,
                                           tempSymmitronInstanceHandle,
                                           errorCode,
                                           errorDescription,
                                           signInResponse ) )
   {
      mSymmitronInstanceHandle = tempSymmitronInstanceHandle;
      mbSignedInWithSymmitron = true;
      OsSysLog::add( FAC_NAT, PRI_DEBUG, "MediaRelay::initialize() signed in successful.  Symmitron instance handle = '%s'",
                     mSymmitronInstanceHandle.data() );

      // Pre-allocate syms for our media relay sessions up to mMaxMediaRelaySessions.
      // Each media relay session requires 4 syms; 2 for RTP and 2 for RTCP.
      XmlRpcRequest createSymsRequest( mSymmitronUrl, CREATE_SYMS_METHOD );
      UtlInt requestedSymCount = 4 * mMaxMediaRelaySessions;
      UtlInt parityCode        = EVEN;
      createSymsRequest.addParam( &mOurInstanceHandle );
      createSymsRequest.addParam( &requestedSymCount );
      createSymsRequest.addParam( &parityCode );
      UtlHashMap* pValuesMap;

      // Send the request and block waiting for the response
      XmlRpcResponse createSymsResponse;
      if( ( pValuesMap = executeAndValudateSymmitronRequest( createSymsRequest,
                                                             tempSymmitronInstanceHandle,
                                                             errorCode,
                                                             errorDescription,
                                                             createSymsResponse ) ) != 0 )
      {
         // we successfully allocated all the syms we required.  The next step is then to
         // process the array of syms we got back from the symmitron and track them by
         // allocating a Sym object of each one and placing it in the mMySyms list.
         //
         // Once this is done, syms of same parity are paired into bridges.  Bridges
         // containing even syms will eventually be used to realy RTP traffic and odd
         // ones will be used to relay RTCP traffic.  As an example, if mMaxMediaRelaySessions
         // was set to 2, we would have allocated 8 syms starting at an even port.  For example,
         // lets say that the following syms got create (number in () shows UDP port allocated):
         // Sym0(5000), Sym1(5001), Sym2(5002), Sym3(5003), Sym4(5004), Sym5(5005), Sym6(5006), Sym7(5007)
         // The logic implemented in this method will pair the syms into 4 bridges as follows:
         // Bridge0(Sym0<=>Sym4), Bridge1(Sym1<=>Sym5), Bridge2(Sym2<=>Sym6), Bridge3(Sym3<=>Sym7)
         //
         // Given that our end goal is to create entities that can relay media session and that
         // media sessions are composed of both an RTP and RTCP flow, we we need to pair
         // an even bridge with the adjacent odd bridge into a MediaBridgePair which
         // contains the necessary elements to relay a complete media session:
         // MediaBridgePair0(Bridge0+Bridge1), MediaBridgePair1(Bridge2+Bridge3)
         UtlString key1Name;
         UtlString key2Name;

         key1Name = SYM_SESSION;
         UtlSList* pSymArray = dynamic_cast<UtlSList*>( pValuesMap->findValue( &key1Name ) );

         if( pSymArray )
         {
            if( pSymArray->entries() == (size_t)requestedSymCount.getValue() )
            {
               // for each sym created, extract its Id and its local IP and port info
               // and store this into our list.
               unsigned int index;
               UtlContainable* pEntry;
               bool bSymCreationSucceeded = true;

               UtlSListIterator symListIterator( *pSymArray );
               while( ( pEntry = symListIterator() ) != NULL )
               {
                  UtlHashMap* pSymDataMap = dynamic_cast<UtlHashMap*>( pEntry );
                  if( pSymDataMap )
                  {
                     // extract id and receiver information
                     UtlString key1Name( SYM_ID );
                     UtlString key2Name( SYM_RECEIVER_DATA );
                     UtlString*  pSymId;
                     UtlHashMap* pSymReceiver;

                     if( ( pSymId       = dynamic_cast<UtlString* >( pSymDataMap->findValue( &key1Name ) ) ) &&
                         ( pSymReceiver = dynamic_cast<UtlHashMap*>( pSymDataMap->findValue( &key2Name ) ) ) )
                     {
                        // extract IP address and port from receiver structure
                        UtlString* pSymIpAddress = 0;
                        UtlInt*    pSymPort = 0;

                        key1Name = SYM_RECEIVER_IP;
                        key2Name = SYM_RECEIVER_PORT;

                        if( ( pSymIpAddress = dynamic_cast<UtlString* >( pSymReceiver->findValue( &key1Name ) ) ) &&
                            ( pSymPort      = dynamic_cast<UtlInt*>( pSymReceiver->findValue( &key2Name ) ) ) )
                        {
                           Sym* pSym = new Sym( *pSymId, *pSymIpAddress, *pSymPort );
                           OsSysLog::add(FAC_NAT, PRI_DEBUG,
                                         "MediaRelay::initialize() createSyms id:'%s' IP:Port=%s:%d",
                                         pSymId->data(), pSymIpAddress->data(), (int)(pSymPort->getValue() ) );
                           mSymList.insert( pSym );
                        }
                        else
                        {
                           OsSysLog::add(FAC_NAT, PRI_CRIT,
                                         "MediaRelay::initialize() createSyms response did not find ip address or port in receiver" );
                           bSymCreationSucceeded = false;
                           break;
                        }
                     }
                     else
                     {
                        OsSysLog::add(FAC_NAT, PRI_CRIT,
                                      "MediaRelay::initialize() createSyms response did not find id and receiver info" );
                        bSymCreationSucceeded = false;
                        break;
                     }
                  }
                  else
                  {
                     OsSysLog::add(FAC_NAT, PRI_CRIT,
                                   "MediaRelay::initialize() createSyms response result had unexpected type: %s",
                                   pEntry->getContainableType() );
                     bSymCreationSucceeded = false;
                     break;
                  }
               }

               // If we successfully created all the syms then proceed to allocate the bridges
               if( bSymCreationSucceeded )
               {
                  key1Name = BRIDGE_ID;
                  Bridge* pPreviouslyCreatedBridge = 0;
                  unsigned int numberOfBridgesToCreate = mMaxMediaRelaySessions * 2;
                  for( index = 0; index < numberOfBridgesToCreate; index++ )
                  {
                     Sym* pEndpoint1Sym = static_cast<Sym*>( mSymList.at( index ) );
                     Sym* pEndpoint2Sym = static_cast<Sym*>( mSymList.at( mMaxMediaRelaySessions * 2 + index ) );
                     UtlString bridgeId;

                     if( createPausedBridgeOnSymmitron( pEndpoint1Sym, pEndpoint2Sym, bridgeId, tempSymmitronInstanceHandle, errorCode, errorDescription ) )
                     {
                        Bridge* pBridge = new Bridge( bridgeId, pEndpoint1Sym, pEndpoint2Sym );
                        // We need to create MediaBridgePairs that consist of bridge pairs,
                        // one used to relay RTP and the other to relay RTCP.  Every second
                        // bridge create will result in the creatio nof one such MediaDridgePair
                        MediaBridgePair* pMBP;
                        if( index % 2 == 0 )
                        {
                           pPreviouslyCreatedBridge = pBridge;
                        }
                        else
                        {
                           pMBP = new MediaBridgePair( pPreviouslyCreatedBridge, pBridge );
                           mAvailableMediaBridgePairsList.push_back( pMBP );
                           OsSysLog::add(FAC_NAT, PRI_DEBUG,
                                          "MediaRelay::initialize() created media bridge pair @ 0x%p linking bridge '%s' with bridge '%s'",
                                          pMBP, pPreviouslyCreatedBridge->getId().data(), pBridge->getId().data() );

                        }
                     }
                     else
                     {
                        OsSysLog::add( FAC_NAT, PRI_CRIT, "MediaRelay::initialize() createPausedBridgeOnSymmitron failed: %d:%s",
                                                          errorCode, errorDescription.data() );
                        break;
                     }
                  }

                  // check if we managed to reach the end of the loop and create all the required bridges
                  if( index >= numberOfBridgesToCreate )
                  {
                     // this completes the initialization sequence - set flag to indicate success.
                     bInitializationSucceeded = true;
                  }

               }
            }
            else
            {
               OsSysLog::add( FAC_NAT, PRI_CRIT, "MediaRelay::initialize() createSyms requested %d syms"
                                                 "but only obtained %zu - init failed", (int)(requestedSymCount.getValue()), pSymArray->entries() );
            }

         }
         else
         {
            OsSysLog::add( FAC_NAT, PRI_CRIT, "MediaRelay::initialize() createSyms response does not contain any syms" );
         }
      }
      else
      {
         OsSysLog::add( FAC_NAT, PRI_CRIT, "MediaRelay::createSyms request failed: %d:'%s'",
                        errorCode, errorDescription.data() );
      }
   }
   else
   {
      OsSysLog::add( FAC_NAT, PRI_CRIT, "MediaRelay::initialize() sign in failed: %d:'%s'",
                     errorCode, errorDescription.data() );
   }
   if( bInitializationSucceeded )
   {
      // all the request we have made to the symmitron we successful but there is one
      // more check that needs to be performed.  Every request that is made to the
      // symmitron returns an instance handle.  We need to make sure that the
      // symmitron did not restart in the middle of our initialization process.
      // Thi is determined by comparing the last instance handle we got from the
      // symmitron with the first.  If they are not the same, the initialization
      // process fails.
      if( tempSymmitronInstanceHandle.compareTo( mSymmitronInstanceHandle ) != 0 )
      {
         OsSysLog::add( FAC_NAT, PRI_CRIT, "MediaRelay::initialize() failed: symmitron reset detected, "
                                           "expected handle '%s' and received '%s' in last response",
                                           mSymmitronInstanceHandle.data(), tempSymmitronInstanceHandle.data() );
         bInitializationSucceeded = false;
         deallocateAllSymmitronResourcesAndSignOut();
      }
      mAsynchMediaRelayRequestSender.setSymmitronInstanceHandle( mSymmitronInstanceHandle );
      mAsynchMediaRelayRequestSender.start();
   }
   else
   {
      deallocateAllSymmitronResourcesAndSignOut();
   }

   return bInitializationSucceeded;
}

const Url& MediaRelay::getXmlRpcServerUrl( void ) const
{
   return mSymmitronUrl;
}

bool MediaRelay::createPausedBridgeOnSymmitron( Sym* pEndpoint1Sym, Sym* pEndpoint2Sym, UtlString& returnedBridgeId, UtlString& symmitronInstanceHandle, int& errorCode, UtlString& errorDescription )
{
   OsLock lock( mMutex );
   // creates a bridge that unites the two supplies syms and puts the bridge in a paused state.
   bool result = false;

   XmlRpcRequest createBridge( mSymmitronUrl, CREATE_BRIDGE_METHOD );
   createBridge.addParam( &mOurInstanceHandle );

   UtlHashMap* pValuesMap;
   XmlRpcResponse createBridgeResponse;
   if( ( pValuesMap = executeAndValudateSymmitronRequest( createBridge,
                                                          symmitronInstanceHandle,
                                                          errorCode,
                                                          errorDescription,
                                                          createBridgeResponse ) ) != 0 )
   {
      UtlString* pTmpStringPtr;
      UtlString keyName;
      keyName = BRIDGE_ID;

      if( ( pTmpStringPtr = dynamic_cast<UtlString*>( pValuesMap->findValue( &keyName ) ) ) )
      {
         returnedBridgeId = *pTmpStringPtr;
         UtlString symId = pEndpoint1Sym->getId();
         if( addSymToBridge( symId, returnedBridgeId, symmitronInstanceHandle, errorCode, errorDescription ) )
         {
            UtlString symId = pEndpoint2Sym->getId();
            if( addSymToBridge( symId, returnedBridgeId, symmitronInstanceHandle, errorCode, errorDescription ) )
            {
               // When a bridge is created by Symmitron, it is put in INITIAL state.  We want them
               // to be in a 'PAUSED' so that they can be quickly resumed when media-relay-requiring
               // sessions get established.  In order to move a bridge from the 'INITIAL' state to
               // the 'PAUSED' it needs to be started and then paused.  These operations are done here.
               XmlRpcRequest startBridge( mSymmitronUrl, START_BRIDGE_METHOD );
               startBridge.addParam( &mOurInstanceHandle );
               startBridge.addParam( &returnedBridgeId );
               XmlRpcResponse startBridgeResponse;
               if( executeAndValudateSymmitronRequest( startBridge,
                                                       symmitronInstanceHandle,
                                                       errorCode,
                                                       errorDescription,
                                                       startBridgeResponse ) != 0 )
               {
                  XmlRpcRequest pauseBridge( mSymmitronUrl, PAUSE_BRIDGE_METHOD );
                  pauseBridge.addParam( &mOurInstanceHandle );
                  pauseBridge.addParam( &returnedBridgeId );
                  XmlRpcResponse pauseBridgeResponse;
                  if( executeAndValudateSymmitronRequest( pauseBridge,
                                                          symmitronInstanceHandle,
                                                          errorCode,
                                                          errorDescription,
                                                          pauseBridgeResponse ) != 0 )
                  {
                     result = true;
                     OsSysLog::add(FAC_NAT, PRI_DEBUG,
                                   "MediaRelay::createPausedBridgeOnSymmitron() createBridge id:'%s' linking Sym '%s' with Sym '%s'",
                                   returnedBridgeId.data(), pEndpoint1Sym->getId().data(), pEndpoint2Sym->getId().data() );
                  }
                  else
                  {
                     OsSysLog::add( FAC_NAT, PRI_CRIT, "MediaRelay::createPausedBridgeOnSymmitron() pauseBridge request failed: %d:'%s'",
                                    errorCode, errorDescription.data() );
                  }
               }
               else
               {
                  OsSysLog::add( FAC_NAT, PRI_CRIT, "MediaRelay::createPausedBridgeOnSymmitron() startBridge request failed: %d:'%s'",
                                 errorCode, errorDescription.data() );
               }
            }
            else
            {
               OsSysLog::add(FAC_NAT, PRI_CRIT,
                             "MediaRelay::createPausedBridgeOnSymmitron() failed to add second sym %s to bridge %s",
                             symId.data(), returnedBridgeId.data() );
            }
         }
         else
         {
            OsSysLog::add(FAC_NAT, PRI_CRIT,
                          "MediaRelay::createPausedBridgeOnSymmitron() failed to add first sym %s to bridge %s",
                          symId.data(), returnedBridgeId.data() );
         }
      }
      else
      {
         OsSysLog::add(FAC_NAT, PRI_CRIT,
                       "MediaRelay::createPausedBridgeOnSymmitron() createBridge response did not find bridge id" );
      }
   }
   else
   {
      OsSysLog::add( FAC_NAT, PRI_CRIT, "MediaRelay::createPausedBridgeOnSymmitron() createBridge request failed: %d:'%s'",
                     errorCode, errorDescription.data() );
   }
   return result;
}

bool MediaRelay::addSymToBridge( UtlString& symId, UtlString& bridgeId, UtlString& symmitronInstanceHandle, int& errorCode, UtlString& errorDescription )
{
   OsLock lock( mMutex );
   bool result = false;
   // add sym to the bridge
   XmlRpcRequest addSym( mSymmitronUrl, ADD_SYM_METHOD );
   addSym.addParam( &mOurInstanceHandle );
   addSym.addParam( &bridgeId );
   addSym.addParam( &symId );

   XmlRpcResponse addSymResponse;
   if( executeAndValudateSymmitronRequest( addSym,
                                           symmitronInstanceHandle,
                                           errorCode,
                                           errorDescription,
                                           addSymResponse ) != 0 )
   {
      result = true;
   }
   else
   {
      OsSysLog::add( FAC_NAT, PRI_CRIT, "MediaRelay::addSymToBridge() addSym request failed: %d:%s",
                                         errorCode, errorDescription.data() );
   }
   return result;
}

MediaRelay::~MediaRelay()
{
   cleanUpEverything();
   mGenericTimer.stop();
}

void MediaRelay::cleanUpEverything( void )
{
   OsLock lock( mMutex );
   mActiveMediaRelaySessions.destroyAll();
   deallocateAllSymmitronResourcesAndSignOut();
}

void MediaRelay::notifySymmitronResetDetected( const UtlString& newSymmitronInstanceHandle )
{
   OsLock lock( mMutex );
   OsSysLog::add( FAC_NAT, PRI_CRIT, "MediaRelay::notifySymmitronResetDetected(): Symmitron reset detected.  New handle is '%s'", newSymmitronInstanceHandle.data() );
   // release everything we had...
   cleanUpEverything();

   if( !newSymmitronInstanceHandle.isNull() )
   {
      // A new Symmitron instance is up already, preallocate resources on that new symmitron

      if( preAllocateSymmitronResources() == false )
      {
         Alarm::raiseAlarm( "NAT_TRAVERSAL_MEDIA_RELAY_RESET_DETECTED_RECONNECTING" );
         mbPollForSymmitronRecovery = true;
      }
      else
      {
         Alarm::raiseAlarm( "NAT_TRAVERSAL_MEDIA_RELAY_RESET_DETECTED_RECONNECTED" );
      }
   }
   else
   {
      // The Symmitron is not up yet.  Set up a flag that will poll for Symmitron's recovery
      Alarm::raiseAlarm( "NAT_TRAVERSAL_LOST_CONTACT_WITH_MEDIA_RELAY" );
      mbPollForSymmitronRecovery = true;
   }
}

void MediaRelay::notifyBridgeStatistics( const UtlString& bridgeId, intptr_t numberOfPacketsProcessed, void* opaqueData )
{
   OsLock lock( mMutex );
   tMediaRelayHandle mediaRelaySessionHandle = (intptr_t)opaqueData;
   MediaRelaySession* pMediaRelaySession = getSessionByHandle( mediaRelaySessionHandle );

   OsSysLog::add( FAC_NAT, PRI_DEBUG, "MediaRelay::notifyBridgeStatistics() received stats for bridge %s belonging to MRS %u: "
                                      " packets processed = %d", bridgeId.data(), (int)mediaRelaySessionHandle, (int)numberOfPacketsProcessed );

   if( pMediaRelaySession )
   {
      PacketProcessingStatistics newStats = pMediaRelaySession->getPacketProcessingStats();
      if( numberOfPacketsProcessed != newStats.mNumberOfPacketsProcessed )
      {
         // The count of processed packets has changed.  In normal cases
         // it is because the number of packets processed has grown since the
         // last time so the stats are updated to reflect the increment.  In
         // special cases, it could also be that the number of packets processed
         // has decreased - this could happen in cases where the stats are
         // reset in error at the symmitron or have wrapped around.  We also
         // update the stats in this case to resynch with the backward jump.
         newStats.mNumberOfPacketsProcessed         = numberOfPacketsProcessed;
         newStats.mEpochTimeOfLastPacketsProcessed  = OsDateTime::getSecsSinceEpoch();
         pMediaRelaySession->setPacketProcessingStats( newStats );
      }
      // else{ processed packet count did not change so do not update anything. }
   }
}

bool MediaRelay::allocateSession( tMediaRelayHandle& relayHandle, int& endpoint1RelayRtpPort, int& endpoint2RelayRtpPort )
{
   OsLock lock( mMutex );
   bool result = false;
   // get a free bridge
   if( mAvailableMediaBridgePairsList.size() > 0 )
   {
      MediaBridgePair* pMediaBridgePairToUse = mAvailableMediaBridgePairsList.back();
      mAvailableMediaBridgePairsList.pop_back();
      mBusyMediaBridgePairsList.push_back( pMediaBridgePairToUse );

      endpoint1RelayRtpPort = pMediaBridgePairToUse->getRtpBridge()->getEndpoint1Sym()->getPort();
      endpoint2RelayRtpPort = pMediaBridgePairToUse->getRtpBridge()->getEndpoint2Sym()->getPort();
      relayHandle = mRelaySessionHandle++;

      UtlString idString;

      // set all four syms in auto-learning mode and resume them
         // Sym1 of Rtp Bridge
      idString = pMediaBridgePairToUse->getRtpBridge()->getEndpoint1Sym()->getId();
      mAsynchMediaRelayRequestSender.setDestination( mOurInstanceHandle, idString, "", 0 );
      mAsynchMediaRelayRequestSender.resumeSym( mOurInstanceHandle, idString );
      // Sym2 of Rtp Bridge
      idString = pMediaBridgePairToUse->getRtpBridge()->getEndpoint2Sym()->getId();
      mAsynchMediaRelayRequestSender.setDestination( mOurInstanceHandle, idString, "", 0 );
      mAsynchMediaRelayRequestSender.resumeSym( mOurInstanceHandle, idString );
         // Sym1 of Rtcp Bridge
      idString = pMediaBridgePairToUse->getRtcpBridge()->getEndpoint1Sym()->getId();
      mAsynchMediaRelayRequestSender.setDestination( mOurInstanceHandle, idString, "", 0 );
      mAsynchMediaRelayRequestSender.resumeSym( mOurInstanceHandle, idString );
         // Sym2 of Rtcp Bridge
      idString = pMediaBridgePairToUse->getRtcpBridge()->getEndpoint2Sym()->getId();
      mAsynchMediaRelayRequestSender.setDestination( mOurInstanceHandle, idString, "", 0 );
      mAsynchMediaRelayRequestSender.resumeSym( mOurInstanceHandle, idString );

      // unpause both bridges of the pair
      idString = pMediaBridgePairToUse->getRtpBridge()->getId();
      mAsynchMediaRelayRequestSender.resumeBridge( mOurInstanceHandle, idString );
       idString = pMediaBridgePairToUse->getRtcpBridge()->getId();
      mAsynchMediaRelayRequestSender.resumeBridge( mOurInstanceHandle, idString );

      MediaRelaySession* pMediaRelaySession = new MediaRelaySession( relayHandle, endpoint1RelayRtpPort, endpoint2RelayRtpPort, pMediaBridgePairToUse );
      tMediaRelayHandle* pHandle = new tMediaRelayHandle( relayHandle );
      mActiveMediaRelaySessions.insertKeyAndValue( pHandle, pMediaRelaySession );
      result = true;

      OsSysLog::add( FAC_NAT, PRI_DEBUG, "__NAT_DEBUG__ MRS handle=%u: Allocated session.  Caller(Port=%d, sym=%s); Callee(Port=%d, sym=%s)"
                     "MBP:%p (RtpBridge=%s)",
                     (int)relayHandle,
                     pMediaBridgePairToUse->getRtpBridge()->getEndpoint1Sym()->getPort(),
                     pMediaBridgePairToUse->getRtpBridge()->getEndpoint1Sym()->getId().data(),
                     pMediaBridgePairToUse->getRtpBridge()->getEndpoint2Sym()->getPort(),
                     pMediaBridgePairToUse->getRtpBridge()->getEndpoint2Sym()->getId().data(),
                     pMediaBridgePairToUse,
                     pMediaBridgePairToUse->getRtpBridge()->getId().data() );
   }
   else
   {
      OsSysLog::add( FAC_NAT, PRI_CRIT, "MediaRelay::allocateSession() failed to allocate a new session - "
                                        "ran out of bridges (max = %zu)", mMaxMediaRelaySessions );
      Alarm::raiseAlarm("NAT_TRAVERSAL_RAN_OUT_OF_MEDIA_RELAY_SESSIONS");
   }
   return result;
}

tMediaRelayHandle MediaRelay::cloneSession( const tMediaRelayHandle& relayHandleToClone, bool doSwapCallerAndCallee )
{
   OsLock lock( mMutex );
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

      OsSysLog::add( FAC_NAT, PRI_DEBUG, "__NAT_DEBUG__ Cloned session %u => resulting clone %u.  PortsSwapped = %d (callerPort=%d, calleePort=%d)",
                     (int)relayHandleToClone,
                     (int)relayHandleOfClone,
                     doSwapCallerAndCallee,
                     callerRtpPort,
                     calleeRtpPort );
   }
   return relayHandleOfClone;
}

bool MediaRelay::deallocateSession( const tMediaRelayHandle& handle )
{
   OsLock lock( mMutex );
   bool bDeallocSucceeded = false;

   if( handle != INVALID_MEDIA_RELAY_HANDLE )
   {
      MediaRelaySession* pMediaRelaySession;

      if( ( pMediaRelaySession = const_cast< MediaRelaySession* >( getSessionByHandle( handle ) ) ) )
      {
         bDeallocSucceeded = true;
         if( pMediaRelaySession->decrementLinkCount() == 0 )
         {
            if( !pMediaRelaySession->isaCloneOfAnotherMediaRelaySession() )
            {
               // remove the MediaBridgePair associated with the deallocated session
               // from the mBusyMediaBridgePairsList and return it to the mAvailableMediaBridgePairsList.
               // NOTE: This class adds and removes elements at the end of lists so start looking for the
               // MediaBridgePair to remove starting at the end.
               MediaBridgePair* pMediaBridgePairBeingFreed = pMediaRelaySession->getAssociatedMediaBridgePair();
               std::vector<MediaBridgePair*>::iterator pos;
               bool itemToEraseFound = false;
               for( pos = mBusyMediaBridgePairsList.begin(); pos != mBusyMediaBridgePairsList.end(); ++pos )
               {
                  if( *pos == pMediaBridgePairBeingFreed )
                  {
                     mBusyMediaBridgePairsList.erase( pos );
                     itemToEraseFound = true;
                     break;
                  }
               }

               OsSysLog::add( FAC_NAT, PRI_DEBUG, "__NAT_DEBUG__ MRS handle=%u: Deallocated session.  Caller(Port=%d, sym=%s); Callee(Port=%d, sym=%s)"
                              "MBP:%p (RtpBridge=%s)",
                              (int)handle,
                              pMediaBridgePairBeingFreed->getRtpBridge()->getEndpoint1Sym()->getPort(),
                              pMediaBridgePairBeingFreed->getRtpBridge()->getEndpoint1Sym()->getId().data(),
                              pMediaBridgePairBeingFreed->getRtpBridge()->getEndpoint2Sym()->getPort(),
                              pMediaBridgePairBeingFreed->getRtpBridge()->getEndpoint2Sym()->getId().data(),
                              pMediaBridgePairBeingFreed,
                              pMediaBridgePairBeingFreed->getRtpBridge()->getId().data() );

               if( !itemToEraseFound )
               {
                  OsSysLog::add(FAC_NAT, PRI_CRIT, "MediaRelay::deallocateSession couldn't find pMediaBridgePair being freed in mBusyMediaBridgePairsList.");
               }
               mAvailableMediaBridgePairsList.push_back( pMediaBridgePairBeingFreed );

               // return the bridges in a paused state.
               UtlString rtpBridgeId  = pMediaBridgePairBeingFreed->getRtpBridge()->getId();
               UtlString rtcpBridgeId = pMediaBridgePairBeingFreed->getRtcpBridge()->getId();
               mAsynchMediaRelayRequestSender.pauseBridge( mOurInstanceHandle, rtpBridgeId );
               mAsynchMediaRelayRequestSender.pauseBridge( mOurInstanceHandle, rtcpBridgeId );

               // remove media relay session just deallocated from our active list.
               mActiveMediaRelaySessions.destroy( &handle );
            }
            else
            {
               // we are deallocating a clone.   Remove it from our active list but
               // do not actiually de-allocate anything on the Symmitron as the
               // original copy may still be around.
               mActiveMediaRelaySessions.destroy( &handle );
            }
         }
         else
         {
            OsSysLog::add( FAC_NAT, PRI_DEBUG, "__NAT_DEBUG__ MRS handle=%u: Deallocated session but non-zero link count (%zd)",
                           (int)handle,
                           pMediaRelaySession->getLinkCount() );
         }
         assert( mAvailableMediaBridgePairsList.size() + mBusyMediaBridgePairsList.size() == mMaxMediaRelaySessions );
      }
   }
   return bDeallocSucceeded;
}

bool MediaRelay::setDirectionMode( const tMediaRelayHandle& handle, MediaDirectionality mediaRelayDirectionMode )
{
   OsLock lock( mMutex );
   bool success = false;

   if( handle != INVALID_MEDIA_RELAY_HANDLE )
   {
      const MediaRelaySession* pMediaRelaySession;
      if( ( pMediaRelaySession = getSessionByHandle( handle ) ) )
      {
         // figure out which syms to pause to achieve prescribed directionality.
         // The directionality setting is referenced from the point of view of
         // the caller.  The Bridge abstraction contains two syms (1 and 2)
         // but does not know the concept of caller or callee.  As a convention,
         // the Media Relay uses sym1 for the caller and sym2 for the callee.

         UtlString rtpSymId;
         MediaBridgePair* pMediaBridgePair = pMediaRelaySession->getAssociatedMediaBridgePair();

         success = true;
         if( pMediaRelaySession->isaCloneOfAnotherMediaRelaySession() &&
             pMediaRelaySession->areCallerAndCalleeRtpPortsSwapped() )
         {
            if( mediaRelayDirectionMode == SEND_ONLY )
            {
               mediaRelayDirectionMode = RECV_ONLY;
            }
            else if( mediaRelayDirectionMode == RECV_ONLY )
            {
               mediaRelayDirectionMode = SEND_ONLY;
            }
         }

         if( mediaRelayDirectionMode == RECV_ONLY )
         {
            // RECV_ONLY means that the caller receives but does not send therefore
            // the Sym1 from the RTP bridge need to be paused while the
            // RTCP one needs to remain open to ensure that bi-directional exchnage
            // of RTCP metrics continue to happen.
            rtpSymId = pMediaBridgePair->getRtpBridge()->getEndpoint1Sym()->getId();
            mAsynchMediaRelayRequestSender.pauseSym( mOurInstanceHandle, rtpSymId );
            // the other Sym needs to be resumed in case it was paused
            rtpSymId = pMediaBridgePair->getRtpBridge()->getEndpoint2Sym()->getId();
            mAsynchMediaRelayRequestSender.resumeSym( mOurInstanceHandle, rtpSymId );
         }
         else if( mediaRelayDirectionMode == SEND_ONLY )
         {
            // SEND_ONLY means that the caller sends but does not receive therefore
            // the Sym2 from the RTP bridge needs to be paused while the
            // RTCP one needs to remain open to ensure that bi-directional exchnage
            // of RTCP metrics continue to happen.
            rtpSymId = pMediaBridgePair->getRtpBridge()->getEndpoint2Sym()->getId();
            mAsynchMediaRelayRequestSender.pauseSym( mOurInstanceHandle, rtpSymId );
            // the other Sym needs to be resumed in case it was paused
            rtpSymId = pMediaBridgePair->getRtpBridge()->getEndpoint1Sym()->getId();
            mAsynchMediaRelayRequestSender.resumeSym( mOurInstanceHandle, rtpSymId );
         }
         else // SEND_RECV
         {
            // resume syms from both caller and callee in case they were paused.
            rtpSymId = pMediaBridgePair->getRtpBridge()->getEndpoint1Sym()->getId();
            mAsynchMediaRelayRequestSender.resumeSym( mOurInstanceHandle, rtpSymId );
            rtpSymId = pMediaBridgePair->getRtpBridge()->getEndpoint2Sym()->getId();
            mAsynchMediaRelayRequestSender.resumeSym( mOurInstanceHandle, rtpSymId );
         }
         UtlString directionalityString;
         MediaDescriptor::mediaDirectionalityValueToSdpDirectionalityAttribute( mediaRelayDirectionMode, directionalityString );
         OsSysLog::add( FAC_NAT, PRI_DEBUG, "__NAT_DEBUG__ MRS handle %u: set directionality to %s.  Caller(Port=%d, sym=%s); Callee(Port=%d, sym=%s)"
                        "MBP:%p (RtpBridge=%s)",
                        (int)handle,
                        directionalityString.data(),
                        pMediaBridgePair->getRtpBridge()->getEndpoint1Sym()->getPort(),
                        pMediaBridgePair->getRtpBridge()->getEndpoint1Sym()->getId().data(),
                        pMediaBridgePair->getRtpBridge()->getEndpoint2Sym()->getPort(),
                        pMediaBridgePair->getRtpBridge()->getEndpoint2Sym()->getId().data(),
                        pMediaBridgePair,
                        pMediaBridgePair->getRtpBridge()->getId().data() );
      }
   }
   return success;
}

bool MediaRelay::linkSymToEndpoint( const tMediaRelayHandle& relayHandle,
                                    const UtlString& endpointIpAddress,
                                    int endpointRtpPort,
                                    int endpointRtcpPort,
                                    EndpointRole ownerOfSymToLink )
{
   OsLock lock( mMutex );
   bool bLinkSucceeded = false;

   UtlString rtpSymId;
   UtlString rtcpSymId;
   const MediaRelaySession* pMediaRelaySession;

   if( ( pMediaRelaySession = getSessionByHandle( relayHandle ) ) )
   {
      if( pMediaRelaySession->isaCloneOfAnotherMediaRelaySession() &&
          pMediaRelaySession->areCallerAndCalleeRtpPortsSwapped() )
      {
         if( ownerOfSymToLink == CALLER )
         {
            ownerOfSymToLink = CALLEE;
         }
         else
         {
            ownerOfSymToLink = CALLER;
         }
      }

      MediaBridgePair* pMediaBridgePair = pMediaRelaySession->getAssociatedMediaBridgePair();

      UtlString rtpSymId;
      UtlString rtcpSymId;
      int rtpPort;
      // by convention Sym1 is associated to caller and sym2 to callee
      if( ownerOfSymToLink == CALLER )
      {
         rtpSymId  = pMediaBridgePair->getRtpBridge()->getEndpoint1Sym()->getId();
         rtpPort   = pMediaBridgePair->getRtpBridge()->getEndpoint1Sym()->getPort();
         rtcpSymId = pMediaBridgePair->getRtcpBridge()->getEndpoint1Sym()->getId();
      }
      else
      {
         rtpSymId  = pMediaBridgePair->getRtpBridge()->getEndpoint2Sym()->getId();
         rtpPort   = pMediaBridgePair->getRtpBridge()->getEndpoint2Sym()->getPort();
         rtcpSymId = pMediaBridgePair->getRtcpBridge()->getEndpoint2Sym()->getId();
      }

      mAsynchMediaRelayRequestSender.setDestination( mOurInstanceHandle, rtpSymId,  endpointIpAddress, endpointRtpPort, DEFAULT_RTP_KEEP_ALIVE_IN_MILLISECS );
      mAsynchMediaRelayRequestSender.setDestination( mOurInstanceHandle, rtcpSymId, endpointIpAddress, endpointRtcpPort, DEFAULT_RTP_KEEP_ALIVE_IN_MILLISECS );
      bLinkSucceeded = true;

      OsSysLog::add( FAC_NAT, PRI_DEBUG, "__NAT_DEBUG__ MRS handle %u: linking %u to dest %s:%u (symId=%s)",
                     (int)relayHandle,
                     rtpPort,
                     endpointIpAddress.data(),
                     endpointRtpPort,
                     rtpSymId.data() );
   }
   else
   {
      OsSysLog::add(FAC_NAT, PRI_CRIT,
                    "MediaRelay::linkSymToEndpoint failed to getSessionByHandle: %d",
                    (int)relayHandle );

   }
   return bLinkSucceeded;
}

ssize_t MediaRelay::incrementLinkCountOfMediaRelaySession( const tMediaRelayHandle& handle )
{
   OsLock lock( mMutex );
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
   OsLock lock( mMutex );
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
   OsLock lock( mMutex );
   return (MediaRelaySession*) mActiveMediaRelaySessions.findValue( &handle );
}

UtlHashMap* MediaRelay::executeAndValudateSymmitronRequest( XmlRpcRequest& requestToSend, UtlString& symmitronInstanceHandle, int& errorCode, UtlString& errorDescription, XmlRpcResponse& xmlRpcResponse, bool bRetryFailedConnection )
{
   bool responseIsValid = false;
   UtlHashMap* pStandardMap = NULL;
   errorCode = SUCCESS;
   errorDescription.remove( 0 );
   symmitronInstanceHandle.remove( 0 );

   if( requestToSend.execute( xmlRpcResponse ) == true )
   {
      UtlContainable* pValue = NULL;
      if ( !xmlRpcResponse.getResponse( pValue ) || !pValue )
      {
         OsSysLog::add(FAC_NAT, PRI_CRIT, "MediaRelay::executeAndValudateSymmitronRequest response had no result.");
      }
      else
      {
         UtlString keyName;
         pStandardMap = dynamic_cast<UtlHashMap*>( pValue );
         if ( !pStandardMap )
         {
            OsSysLog::add(FAC_NAT, PRI_CRIT,
                          "MediaRelay::executeAndValudateSymmitronRequest response result had unexpected type: %s",
                          pValue->getContainableType() );
         }
         else
         {
            // extract symmitron instance handle
            keyName = INSTANCE_HANDLE;
            UtlString* pInstanceHandle = dynamic_cast<UtlString*>( pStandardMap->findValue( &keyName ) );
            if( pInstanceHandle )
            {
               symmitronInstanceHandle = *pInstanceHandle;
            }
            else
            {
               OsSysLog::add(FAC_NAT, PRI_CRIT,
                             "MediaRelay::executeAndValudateSymmitronRequest response does not contain instance-handle" );
            }

            // extract status code and check it.
            keyName = STATUS_CODE;
            UtlString* pStatusCode = dynamic_cast<UtlString*>( pStandardMap->findValue( &keyName ) );
            if( pStatusCode->compareTo( OK ) != 0 )
            {
               // status-code is not "ok", some error happened - extract error information.
               keyName = ERROR_CODE;
               UtlInt* pErrorCode = dynamic_cast<UtlInt*>( pStandardMap->findValue( &keyName ) );
               if( pErrorCode )
               {
                  errorCode = *pErrorCode;
               }
               keyName = ERROR_INFO;
               UtlString* pErrorInfo = dynamic_cast<UtlString*>( pStandardMap->findValue( &keyName ) );
               if( pErrorInfo )
               {
                  errorDescription = *pErrorInfo;
               }
            }
            else
            {
               // status-code is "ok", mark response as valid
               responseIsValid = true;
            }
         }
      }
   }
   else
   {
      // Check if the request failed because of a failed connection.
      // That error can sometimes happen when the server closed the TCP
      // connection we were using to communicate to it and can usually
      // be recovered by sending the request again.  Try to send the
      // request once more to see if it will fly this time.
      xmlRpcResponse.getFault( &errorCode, errorDescription );
      OsSysLog::add( FAC_NAT, PRI_CRIT,
                     "MediaRelay::executeAndValudateSymmitronRequest failed to execute() request: %d : %s",
                     errorCode, errorDescription.data() );
   }
   return responseIsValid ? pStandardMap : NULL;
}

void MediaRelay::deallocateAllSymmitronResourcesAndSignOut( void )
{
   OsLock lock( mMutex );
   if( mbSignedInWithSymmitron == true )
   {
      XmlRpcRequest signOutRequest( mSymmitronUrl, SIGN_OUT_METHOD );
      signOutRequest.addParam( &mOurInstanceHandle );
      // Send the request and block waiting for the response
      XmlRpcResponse signOutResponse;
      signOutRequest.execute( signOutResponse );

      mbSignedInWithSymmitron = false;
   }

   // de-allocate memory for Syms and Bridges and clear lists that hold their pointers.
   // NOTE: the process of signing out causes the symmitron to de-allocate all the resources
   // we had created so there is no need to explicitly request the de-allocation of the syms
   // and bridges we had previously created.
   mSymList.destroyAll();
   std::vector<MediaBridgePair*>::iterator pos;
   for( pos = mAvailableMediaBridgePairsList.begin(); pos != mAvailableMediaBridgePairsList.end(); ++pos )
   {
      delete (*pos)->getRtpBridge();
      delete (*pos)->getRtcpBridge();
      delete *pos;
   }
   mAvailableMediaBridgePairsList.clear();

   for( pos = mBusyMediaBridgePairsList.begin(); pos != mBusyMediaBridgePairsList.end(); ++pos )
   {
      delete (*pos)->getRtpBridge();
      delete (*pos)->getRtcpBridge();
      delete *pos;
   }
   mBusyMediaBridgePairsList.clear();
}

bool MediaRelay::getPacketProcessingStatsForMediaRelaySession( const tMediaRelayHandle& handle,
                                                               PacketProcessingStatistics& stats )
{
   OsLock lock( mMutex );
   bool result = false;
   MediaRelaySession* pMediaRelaySession = getSessionByHandle( handle );
   if( pMediaRelaySession )
   {
      stats = pMediaRelaySession->getPacketProcessingStats();
      result = true;
   }
   return result;
}

OsStatus MediaRelay::signal( intptr_t eventData )
{
   mMutex.acquire();
   if( mGenericTimerTickCounter % GENERIC_TIMER_TICKS_BEFORE_BRIDGE_STAT_QUERY == 0 )
   {
      UtlHashMapIterator mediaRelaySessionsIterator( mActiveMediaRelaySessions );
      while( mediaRelaySessionsIterator() )
      {
         // This while-loop will post one queryBridgeStatistics message on the message
         // queue for each active media relay session.  When many media relay sessions
         // are active at once, this can generate a lot of message posts and we
         // run the risk of filling up the message queue.  The code below releases
         // the media relay mutex before posting a message.  This is done to ensure
         // that we never hit the 'queue full' condition while holding the mutex.
         // Not doing so can create a deadlock condition if the message queue
         // consumer locks that same mutex while handling messages.  This 
         // exact deadlock condition has been seen during live trials of sipXecs.
         MediaRelaySession* pMediaRelaySession = 0;
         MediaBridgePair*   pMediaBridgePair = 0;
         pMediaRelaySession = dynamic_cast<MediaRelaySession*>( mediaRelaySessionsIterator.value() );
         if( pMediaRelaySession && ( pMediaBridgePair = pMediaRelaySession->getAssociatedMediaBridgePair() ) )
         {
            UtlString instanceHandle = mOurInstanceHandle;
            UtlString bridgeId = pMediaBridgePair->getRtpBridge()->getId();
            void* opaqueData = (void *)( (intptr_t)pMediaRelaySession->getUniqueHandle() );
            mMutex.release();
            mAsynchMediaRelayRequestSender.queryBridgeStatistics( instanceHandle,
                                                                  bridgeId,
                                                                  opaqueData );
            mMutex.acquire();
         }
      }
   }

   if( mGenericTimerTickCounter % GENERIC_TIMER_TICKS_BEFORE_PING == 0 )
   {
      mAsynchMediaRelayRequestSender.ping( mOurInstanceHandle );
   }

   if( mbPollForSymmitronRecovery && ( mGenericTimerTickCounter % GENERIC_TIMER_TICKS_BEFORE_SYMMITRON_RECONNECT_ATTEMPT == 0 ) )
   {
      if( preAllocateSymmitronResources() )
      {
         // we managed to reconnect, clear the recovery flag
         mbPollForSymmitronRecovery = false;
         Alarm::raiseAlarm( "NAT_TRAVERSAL_MEDIA_RELAY_RECONNECTED" );
      }
   }

   mGenericTimerTickCounter++;
   mMutex.release();
   return OS_SUCCESS;
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
   mpOwningMediaRelay( pOwningMediaRelay )
{
}

AsynchMediaRelayRequestSender::~AsynchMediaRelayRequestSender()
{
}

void AsynchMediaRelayRequestSender::setSymmitronInstanceHandle( const UtlString& symmitronInstanceHandle )
{

   mReferenceSymmitronInstanceHandle = symmitronInstanceHandle;
}

UtlBoolean AsynchMediaRelayRequestSender::handleMessage( OsMsg& rMsg )
{
   UtlBoolean handled = FALSE;
   XmlRpcRequest* pXmlRpcRequestToSend = 0;
   Url xmlRpcServerUrl = mpOwningMediaRelay->getXmlRpcServerUrl();
   AsynchMediaRelayMsg* pSymmitronMsg = dynamic_cast <AsynchMediaRelayMsg*> ( &rMsg );

   UtlString controllerHandle    = pSymmitronMsg->getControllerHandle();
   UtlString bridgeId            = pSymmitronMsg->getBridgeId();
   UtlString symId               = pSymmitronMsg->getSymId();
   UtlString destIpAddress       = pSymmitronMsg->getIpAddress();
   UtlInt    destPort            = pSymmitronMsg->getPort();
   UtlInt    timeout             = pSymmitronMsg->getTimeout();
   UtlInt    keepAliveTime       = pSymmitronMsg->getKeepAliveTime();
   UtlString keepAliveMethod     = "NONE";
   void*     pOpaqueData         = pSymmitronMsg->getOpaqueData();

   switch ( rMsg.getMsgType() )
   {
   case OsMsg::OS_SHUTDOWN:
      OsTask::requestShutdown();
      handled = TRUE;
      break;

   case OsMsg::OS_EVENT:
      switch( rMsg.getMsgSubType() )
      {
      case AsynchMediaRelayMsg::SYMMITRON_PING:
         pXmlRpcRequestToSend = new XmlRpcRequest( xmlRpcServerUrl, PING_METHOD );
         pXmlRpcRequestToSend->addParam( &controllerHandle );
         break;

      case AsynchMediaRelayMsg::SYMMITRON_PAUSE_BRIDGE:
         pXmlRpcRequestToSend = new XmlRpcRequest( xmlRpcServerUrl, PAUSE_BRIDGE_METHOD );
         pXmlRpcRequestToSend->addParam( &controllerHandle );
         pXmlRpcRequestToSend->addParam( &bridgeId );
         break;

      case AsynchMediaRelayMsg::SYMMITRON_RESUME_BRIDGE:
         pXmlRpcRequestToSend = new XmlRpcRequest( xmlRpcServerUrl, RESUME_BRIDGE_METHOD );
         pXmlRpcRequestToSend->addParam( &controllerHandle );
         pXmlRpcRequestToSend->addParam( &bridgeId );
         break;

      case AsynchMediaRelayMsg::SYMMITRON_SET_DESTINATION:
         pXmlRpcRequestToSend = new XmlRpcRequest( xmlRpcServerUrl, SET_DESTINATION_SYM_METHOD );
         pXmlRpcRequestToSend->addParam( &controllerHandle );
         pXmlRpcRequestToSend->addParam( &symId );
         pXmlRpcRequestToSend->addParam( &destIpAddress );
         pXmlRpcRequestToSend->addParam( &destPort );
         pXmlRpcRequestToSend->addParam( &keepAliveTime );
         if( keepAliveTime != 0 )
         {
            keepAliveMethod = "USE-EMPTY-PACKET";
         }
         pXmlRpcRequestToSend->addParam( &keepAliveMethod );
         break;

      case AsynchMediaRelayMsg::SYMMITRON_PAUSE_SYM:
         pXmlRpcRequestToSend = new XmlRpcRequest( xmlRpcServerUrl, PAUSE_SYM_METHOD );
         pXmlRpcRequestToSend->addParam( &controllerHandle );
         pXmlRpcRequestToSend->addParam( &symId );
         break;

      case AsynchMediaRelayMsg::SYMMITRON_RESUME_SYM:
         pXmlRpcRequestToSend = new XmlRpcRequest( xmlRpcServerUrl, RESUME_SYM_METHOD );
         pXmlRpcRequestToSend->addParam( &controllerHandle );
         pXmlRpcRequestToSend->addParam( &symId );
         break;

      case AsynchMediaRelayMsg::SYMMITRON_SET_SYM_TIMEOUT:
         pXmlRpcRequestToSend = new XmlRpcRequest( xmlRpcServerUrl, SET_SYM_TIMEOUT_METHOD );
         pXmlRpcRequestToSend->addParam( &controllerHandle );
         pXmlRpcRequestToSend->addParam( &symId );
         pXmlRpcRequestToSend->addParam( &timeout );
         break;

      case AsynchMediaRelayMsg::SYMMITRON_GET_BRIDGE_STATS:
         pXmlRpcRequestToSend = new XmlRpcRequest( xmlRpcServerUrl, GET_BRIDGE_STATS_METHOD );
         pXmlRpcRequestToSend->addParam( &controllerHandle );
         pXmlRpcRequestToSend->addParam( &bridgeId );
         break;

      default:
         OsSysLog::add(FAC_NAT, PRI_CRIT,
                       "AsynchMediaRelayRequestSender::handleMessage: received unknown sub-type: %d",
                       rMsg.getMsgSubType() );
         break;
      }
      handled = TRUE;
      break;

   default:
      OsSysLog::add(FAC_NAT, PRI_CRIT,
                    "AsynchMediaRelayRequestSender::handleMessage: '%s' unhandled message type %d.%d",
                    mName.data(), rMsg.getMsgType(), rMsg.getMsgSubType());
      break;
   }

   if( pXmlRpcRequestToSend )
   {
      UtlString symmitronInstanceHandle;
      int errorCode;
      UtlString errorDescription;
      XmlRpcResponse xmlRpcResponse;
      UtlHashMap* pValuesMap;

      if( ( pValuesMap = MediaRelay::executeAndValudateSymmitronRequest( *pXmlRpcRequestToSend,
                                                                         symmitronInstanceHandle,
                                                                         errorCode,
                                                                         errorDescription,
                                                                         xmlRpcResponse ) ) != 0 )
      {

         // we got a valid response.  Look to see if this is a response we need to process
         if( rMsg.getMsgSubType() == AsynchMediaRelayMsg::SYMMITRON_GET_BRIDGE_STATS )
         {
            // This response contains the stats of a bridge.  Extract the relevant
            // stat information and notify the MediaRelay that the stats it
            // queried have been received.
            UtlString* pTmpString;
            UtlString keyName;

            keyName = PACKETS_SENT;
            pTmpString = dynamic_cast<UtlString*>( pValuesMap->findValue( &keyName ) );
            if( pTmpString )
            {
               UtlInt packetsSent = atoi( pTmpString->data() );
               mpOwningMediaRelay->notifyBridgeStatistics( "dummy", packetsSent, pOpaqueData );
            }
            else
            {
               OsSysLog::add(FAC_NAT, PRI_ERR,
                             "AsynchMediaRelayRequestSender::handleMessage failed to get %s member", keyName.data() );
            }
        }
      }
      else
      {
         OsSysLog::add( FAC_NAT, PRI_CRIT, "AsynchMediaRelayRequestSender::handleMessage() failed to execute request for event %d: Error=%d:'%s'",
                        rMsg.getMsgSubType(), errorCode, errorDescription.data() );
      }

      // check if the symmitron instance we received in the response matches the reference one
      // we were given at construction time.  If not ,it indicates that the Symmitron has reset
      // behind our back.  If that is the case, inform our master of the condition.
      if( !mReferenceSymmitronInstanceHandle.isNull() && mReferenceSymmitronInstanceHandle.compareTo( symmitronInstanceHandle ) != 0 )
      {
         OsSysLog::add( FAC_NAT, PRI_CRIT, "AsynchMediaRelayRequestSender::handleMessage() detected a symmitron reset. "
                        "Expected handle='%s'; received handle='%s'",
                        mReferenceSymmitronInstanceHandle.data(),
                        symmitronInstanceHandle.data() );
         mReferenceSymmitronInstanceHandle.remove( 0 );
         mpOwningMediaRelay->notifySymmitronResetDetected( symmitronInstanceHandle );
      }
   }
   delete pXmlRpcRequestToSend;
   return handled;
}

void AsynchMediaRelayRequestSender::pauseBridge( const UtlString& controllerHandle, const UtlString& bridgeId )
{
   OsSysLog::add(FAC_NAT, PRI_DEBUG,
                 "AsynchMediaRelayRequestSender::pauseBridge( %s, %s )", controllerHandle.data(), bridgeId.data() );
   AsynchMediaRelayMsg message( AsynchMediaRelayMsg::SYMMITRON_PAUSE_BRIDGE, controllerHandle, bridgeId );
   postMessageIfStarted( message );
}

void AsynchMediaRelayRequestSender::resumeBridge( const UtlString& controllerHandle, const UtlString& bridgeId )
{
   OsSysLog::add(FAC_NAT, PRI_DEBUG,
                 "AsynchMediaRelayRequestSender::resumeBridge( %s, %s )", controllerHandle.data(), bridgeId.data() );
   AsynchMediaRelayMsg message( AsynchMediaRelayMsg::SYMMITRON_RESUME_BRIDGE, controllerHandle, bridgeId );
   postMessageIfStarted( message );
}

void AsynchMediaRelayRequestSender::setDestination( const UtlString& controllerHandle, const UtlString& symId, const UtlString& ipAddress, int port, int keepAliveTime )
{
   OsSysLog::add(FAC_NAT, PRI_DEBUG,
                 "AsynchMediaRelayRequestSender::setDestination( %s, %s, %s, %u, %u )", controllerHandle.data(), symId.data(), ipAddress.data(), port, keepAliveTime );
   AsynchMediaRelayMsg message( controllerHandle, symId, ipAddress, port, keepAliveTime );
   postMessageIfStarted( message );
}

void AsynchMediaRelayRequestSender::pauseSym( const UtlString& controllerHandle, const UtlString& symId )
{
   OsSysLog::add(FAC_NAT, PRI_DEBUG,
                 "AsynchMediaRelayRequestSender::pauseSym( %s, %s )", controllerHandle.data(), symId.data() );
   AsynchMediaRelayMsg message( AsynchMediaRelayMsg::SYMMITRON_PAUSE_SYM, controllerHandle, symId );
   postMessageIfStarted( message );
}

void AsynchMediaRelayRequestSender::resumeSym( const UtlString& controllerHandle, const UtlString& symId )
{
   OsSysLog::add(FAC_NAT, PRI_DEBUG,
                 "AsynchMediaRelayRequestSender::resumeSym( %s, %s )", controllerHandle.data(), symId.data() );
   AsynchMediaRelayMsg message( AsynchMediaRelayMsg::SYMMITRON_RESUME_SYM, controllerHandle, symId );
   postMessageIfStarted( message );
}

void AsynchMediaRelayRequestSender::setSymTimeout( const UtlString& controllerHandle, const UtlString& symId, int timeout )
{
   OsSysLog::add(FAC_NAT, PRI_DEBUG,
                 "AsynchMediaRelayRequestSender::setSymTimeout( %s, %s, %d )", controllerHandle.data(), symId.data(), timeout );
   AsynchMediaRelayMsg message( controllerHandle, symId, timeout );
   postMessageIfStarted( message );
}

void AsynchMediaRelayRequestSender::ping( const UtlString& controllerHandle )
{
   OsSysLog::add(FAC_NAT, PRI_DEBUG,
                 "AsynchMediaRelayRequestSender::ping( %s )", controllerHandle.data() );
   AsynchMediaRelayMsg message( controllerHandle );
   postMessageIfStarted( message );
}

void AsynchMediaRelayRequestSender::queryBridgeStatistics( const UtlString& controllerHandle, const UtlString& bridgeId, void* opaqueData )
{
   OsSysLog::add(FAC_NAT, PRI_DEBUG,
                 "AsynchMediaRelayRequestSender::queryBridgeStatistics( %s, %s, %p )", controllerHandle.data(), bridgeId.data(), opaqueData );
   AsynchMediaRelayMsg message( controllerHandle, bridgeId, opaqueData );
   postMessageIfStarted( message );
}

OsStatus AsynchMediaRelayRequestSender::postMessageIfStarted( const OsMsg& rMsg,
                                                              const OsTime& rTimeout,
                                                              UtlBoolean sentFromISR )
{
   OsStatus result = ( isStarted() ? postMessage( rMsg, rTimeout, sentFromISR ) : OS_TASK_NOT_STARTED );
   return result;
}

AsynchMediaRelayMsg::AsynchMediaRelayMsg( EventSubType eventSubType,
                                          const UtlString& controllerHandle,
                                          const UtlString& subId ) :
   OsMsg( OS_EVENT, eventSubType ),
   mControllerHandle( controllerHandle ),
   mSubId( subId )
{

}

AsynchMediaRelayMsg::AsynchMediaRelayMsg( const UtlString& controllerHandle,
                                          const UtlString& symId,
                                          const UtlString& ipAddress,
                                          int port,
                                          int keepAliveTime ) :
   OsMsg( OS_EVENT,  AsynchMediaRelayMsg::SYMMITRON_SET_DESTINATION ),
   mControllerHandle( controllerHandle ),
   mSubId( symId ),
   mIpAddress( ipAddress ),
   mPort( port ),
   mKeepAliveTime( keepAliveTime )
{
}

AsynchMediaRelayMsg::AsynchMediaRelayMsg( const UtlString& controllerHandle,
                                          const UtlString& symId,
                                          int timeout ) :
   OsMsg( OS_EVENT,  AsynchMediaRelayMsg::SYMMITRON_SET_SYM_TIMEOUT ),
   mControllerHandle( controllerHandle ),
   mSubId( symId ),
   mTimeout( timeout )
{
}

AsynchMediaRelayMsg::AsynchMediaRelayMsg( const UtlString& controllerHandle,
                                          const UtlString& bridgeId,
                                          void* opaqueData ) :
   OsMsg( OS_EVENT,  AsynchMediaRelayMsg::SYMMITRON_GET_BRIDGE_STATS ),
   mControllerHandle( controllerHandle ),
   mSubId( bridgeId ),
   mpOpaqueData( opaqueData )
{
}

AsynchMediaRelayMsg::AsynchMediaRelayMsg( const UtlString& controllerHandle ) :
   OsMsg( OS_EVENT,  AsynchMediaRelayMsg::SYMMITRON_PING ),
   mControllerHandle( controllerHandle )
{
}

AsynchMediaRelayMsg::AsynchMediaRelayMsg( const AsynchMediaRelayMsg& rOsMsg ) :
   OsMsg( rOsMsg ),
   mControllerHandle( rOsMsg.mControllerHandle ),
   mSubId( rOsMsg.mSubId ),
   mIpAddress( rOsMsg.mIpAddress ),
   mPort( rOsMsg.mPort ),
   mTimeout( rOsMsg.mTimeout ),
   mKeepAliveTime( rOsMsg.mKeepAliveTime ),
   mpOpaqueData( rOsMsg.mpOpaqueData )
{
}

OsMsg* AsynchMediaRelayMsg::createCopy( void ) const
{
   return new AsynchMediaRelayMsg( *this );
}

const UtlString& AsynchMediaRelayMsg::getControllerHandle( void ) const
{
   return mControllerHandle;
}

const UtlString& AsynchMediaRelayMsg::getSymId( void ) const
{
   return mSubId;
}

const UtlString& AsynchMediaRelayMsg::getBridgeId( void ) const
{
   return mSubId;
}

const UtlString& AsynchMediaRelayMsg::getIpAddress( void ) const
{
   return mIpAddress;
}

int AsynchMediaRelayMsg::getPort( void ) const
{
   return mPort;
}

int AsynchMediaRelayMsg::getTimeout( void ) const
{
   return mTimeout;
}

int AsynchMediaRelayMsg::getKeepAliveTime( void ) const
{
   return mKeepAliveTime;
}

void* AsynchMediaRelayMsg::getOpaqueData( void ) const
{
   return mpOpaqueData;
}
