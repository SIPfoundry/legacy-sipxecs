//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// APPLICATION INCLUDES
#include "NatMaintainer.h"
#include "os/OsLock.h"
#include "os/OsDateTime.h"
#include "utl/UtlSListIterator.h"
#include "utl/UtlSList.h"
#include "net/CallId.h"
#include "net/Url.h"
#include "net/SipMessage.h"
#include "SipRouter.h"
#include "NatTraversalAgentDataTypes.h"

// DEFINES
#define NAT_REFRESH_INTERVAL_IN_MILLISECS (20000)
#define NAT_MAINTAINER_MARKER_STRING ("sipXecs-reniatniamtan")
#define NUMBER_OF_UDP_PORTS (65536)
// NAMESPACES
using namespace std;
// CONSTANTS
// TYPEDEFS
// STATIC INITIALIZERS

NatMaintainer::NatMaintainer( SipRouter* sipRouter ) :
   mRefreshRoundNumber( 0 ),
   mpRegistrationDB( RegistrationDB::getInstance() ),
   mpSubscriptionDB( SubscriptionDB::getInstance() ),
   mNextSeqValue( 1 ),
   mTimerMutex( OsMutex::Q_FIFO ),
   mpSipRouter( sipRouter ),
   mpEndpointsKeptAliveList(
      new KeepAliveEndpointDescriptor[ NUMBER_OF_UDP_PORTS ] ),
   mpKeepAliveMessage( 0 ),
   mExternalKeepAliveListMutex( OsMutex::Q_FIFO )
{
   mTimerMutex.acquire();

   // Build SIP Options message that will be used to keep remote NATed NAT & firewall pinholes open.
   UtlString optionsMessageString =
       "OPTIONS sip:anonymous@anonymous.invalid SIP/2.0\r\n"
       "To: sip:anonymous@anonymous.invalid\r\n"
       "From: \"SipXecs Keepalive\"<sip:anonymous@anonymous.invalid>;tag=30543f3483e1cb11ecb40866edd3295b-reniatniamtan\r\n"
       "Call-ID: f88dfabce84b6a2787ef024a7dbe8749-reniatniamtan\r\n"
       "Cseq: 1 OPTIONS\r\n"
       "Via: SIP/2.0/UDP willgetpatched:5060;branch=z9hG4bK-234fc22f2nnthda-reniatniamtan\r\n"
       "Max-Forwards: 20\r\n"
       "Accept: application/sdp\r\n"
       "Contact: <sip:anonymous@anonymous.invalid>\r\n"
       "Content-Length: 0\r\n"
       "Server: sipXecs/NatMaintainer\r\n"
       "\r\n";

   mpKeepAliveMessage = new SipMessage( optionsMessageString, optionsMessageString.length() );
   
   // base call Id, base branch and From-tag
   CallId::getNewCallId( mBaseCallId );
   UtlString fromTag;
   CallId::getNewTag( fromTag );
   mpKeepAliveMessage->setFromFieldTag( fromTag );   
}

NatMaintainer::~NatMaintainer()
{
   waitUntilShutDown();
   if( mpRegistrationDB )
   {
      mpRegistrationDB->releaseInstance();
      mpRegistrationDB = 0;
   }

   if( mpSubscriptionDB )
   {
      mpSubscriptionDB->releaseInstance();
      mpSubscriptionDB = 0;
   }
   mExternalKeepAliveList.destroyAll();
   delete [] mpEndpointsKeptAliveList;
   delete mpKeepAliveMessage;
}

int NatMaintainer::run( void* runArg )
{
   OsStatus rc;
   while( !isShuttingDown() )
   {
//TODO: Optimization.  Do not maintain db entries for which we are not the primary since
// no pinhole is open for us at the remote NAT if we are the secondary.  Note:  once this
// optimization gets implemented, we will need to start refreshing callers of every active
// CallTracker if not registered with the system handling the call to ensure that its
// pinhole remains open throughout the call.
      rc = mTimerMutex.acquire( NAT_REFRESH_INTERVAL_IN_MILLISECS );
      if( rc == OS_WAIT_TIMEOUT )
      {
         if( mpSipRouter )
         {
            mRefreshRoundNumber++;
            int timeNow = OsDateTime::getSecsSinceEpoch();
            
            // Increment CSeq so that the OPTIONS sent in this 
            // wave have incrementing Cseq as per spec
            mpKeepAliveMessage->setCSeqField( mNextSeqValue, "OPTIONS" );
            mNextSeqValue++;

            // timer has expired - refresh timeout
            UtlSList resultList;
            UtlString stringToMatch( SIPX_PRIVATE_CONTACT_URI_PARAM );

            // start by sending keep-alives to non-expired contacts for far-end NATed phones
            // found in the subscription database
            mpSubscriptionDB->getUnexpiredContactsFieldsContaining( stringToMatch, timeNow, resultList );
            sendKeepAliveToContactList( resultList );
            resultList.destroyAll();

            // next, send keep-alives to non-expired contacts for far-end NATed phones
            // found in the registration database
            mpRegistrationDB->getUnexpiredContactsFieldsContaining( stringToMatch, timeNow, resultList );
            sendKeepAliveToContactList( resultList );
            resultList.destroyAll();

            // finally, send keep-alives to the endpoints that were inserted into our
            // external keep alive list by other components of the NAT traversal feature.
            sendKeepAliveToExternalKeepAliveList();
         }
      }
   }
   return 0;
}

void NatMaintainer::addEndpointToKeepAlive( const TransportData& endpointToKeepAlive )
{
   OsLock lock( mExternalKeepAliveListMutex );
   TransportData* pTransportDataToAppend = new TransportData( endpointToKeepAlive );
   mExternalKeepAliveList.append( pTransportDataToAppend );
}

void NatMaintainer::removeEndpointToKeepAlive( const TransportData& endpointKeptAlive )
{
   OsLock lock( mExternalKeepAliveListMutex );
   mExternalKeepAliveList.destroy( &endpointKeptAlive );
}

void NatMaintainer::sendKeepAliveToContactList( UtlSList& contactList )
{
   UtlSListIterator iter( contactList );
   UtlString* pString;

   while( ( pString = (UtlString*)iter() ) )
   {
      Url url( *pString );
      PublicTransportData publicTransport( url );

      if( publicTransport.isInitialized() && publicTransport.getTransportProtocol().compareTo( "udp", UtlString::ignoreCase ) == 0 )
      {
         sendKeepAliveToEndpoint( publicTransport.getAddress(), publicTransport.getPort() );
      }
   }
}
void NatMaintainer::sendKeepAliveToExternalKeepAliveList( void )
{
   OsLock lock( mExternalKeepAliveListMutex );

   UtlSListIterator iter( mExternalKeepAliveList );
   TransportData* pTransportToKeepAlive;

   while( ( pTransportToKeepAlive = (TransportData*)iter() ) )
   {
      sendKeepAliveToEndpoint( pTransportToKeepAlive->getAddress(), pTransportToKeepAlive->getPort() );
   }
}

void NatMaintainer::sendKeepAliveToEndpoint( const char* pIpAddress, uint16_t portNumber )
{
   bool bDoSendKeepAlive = true;
   KeepAliveEndpointDescriptor* pKeepAliveEndpointDescriptor;

   pKeepAliveEndpointDescriptor = &( mpEndpointsKeptAliveList[ portNumber ] );

   if( pKeepAliveEndpointDescriptor->mLastRefreshRoundNumber == mRefreshRoundNumber )
   {
      // We have already sent a keep-alive to an endpoint utilizing this port in this refresh
      // round.  Check to see if we also have an IP address match.  If we do, that endpoint
      // has already been refreshed in this round so do not send it yet another refresh.
      if( strcmp( pIpAddress, pKeepAliveEndpointDescriptor->mIpAddress ) == 0 )
      {
         bDoSendKeepAlive = false;
      }
   }
   else
   {
      // We have yet to send a keep-alive to an endpoint utilizing this port in this refresh round
      // Save the endpoint's IP address against the port in mpEndpointsKeptAliveList[portNumber]
      // to prevent the generation of any aditional keep alives to that endpoint during that round.
      pKeepAliveEndpointDescriptor->mLastRefreshRoundNumber = mRefreshRoundNumber;
      strncpy( pKeepAliveEndpointDescriptor->mIpAddress, pIpAddress, MAX_IP_ADDRESS_STRING_SIZE );
      pKeepAliveEndpointDescriptor->mIpAddress[ MAX_IP_ADDRESS_STRING_SIZE ] = 0;
   }
   if( bDoSendKeepAlive )
   {
      // Generate unique call-id and branches for each remote endpoint
      UtlString callId = pIpAddress;
      callId.append('-').appendNumber( portNumber, "%d" )
            .append('-').append( NAT_MAINTAINER_MARKER_STRING ).append('-').append( mBaseCallId );
      mpKeepAliveMessage->setCallIdField( callId );
      
      UtlString branchId = callId;
      branchId.appendNumber( mNextSeqValue, "%d" );
      mpKeepAliveMessage->setTopViaTag( branchId.data(), "branch" );   
      mpSipRouter->sendUdpKeepAlive( *mpKeepAliveMessage, pIpAddress, portNumber );
   }
}


void NatMaintainer::requestShutdown( void )
{
   mTimerMutex.release();
   OsTask::requestShutdown();
}
