//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include "os/OsSysLog.h"

// APPLICATION INCLUDES
#include "NatTraversalRules.h"
#include "DialogTracker.h"
#include "SessionContext.h"
#include "net/SdpBody.h"
#include "utl/UtlFsm.h"
#include "utl/UtlTokenizer.h"
#include "utl/UtlHashMapIterator.h"

// DEFINES

const UtlContainableType SessionContext::TYPE = "SessionContext";

SessionContext::SessionContext( const SipMessage& sipRequest,
                                const NatTraversalRules* pNatRules,
                                const UtlString& handle,
                                MediaRelay* pMediaRelayToUse,
                                const RegistrationDB* pRegistrationDB,
                                CallTrackerInterfaceForSessionContext* pOwningCallTracker ) :
   mpReferenceDialogTracker( 0 ),
   mpCaller( 0 ),
   mpCallee( 0 ),
   mpNatTraversalRules( pNatRules ),
   mHandle( handle ),
   mpMediaRelay( pMediaRelayToUse ),
   mpOwningCallTracker( pOwningCallTracker )
{
   UtlString tmpString;

   // initialize the members of session context based on the content of the
   // SIP request passed to this c'tor

   // First, initialize the caller descriptor
   {
      mpCaller = createCallerEndpointDescriptor( sipRequest, *mpNatTraversalRules );
      mpCaller->toString( tmpString );
      OsSysLog::add(FAC_NAT, PRI_DEBUG, "SessionContext[%s]::SessionContext: Caller transport info:'%s'",
                                        mHandle.data(), tmpString.data() );
   }

   // Second, initialize the callee descriptor based on the Request URI header
   {
      mpCallee = createCalleeEndpointDescriptor( sipRequest, *mpNatTraversalRules, pRegistrationDB );
      mpCallee->toString( tmpString );
      OsSysLog::add(FAC_NAT, PRI_DEBUG, "SessionContext[%s]::SessionContext: Callee transport info:'%s'",
                                        mHandle.data(), tmpString.data() );
   }

   // Create the string that will be used to mark packets as handled by this system.  This
   // string must be unique for this system and is set to the system's private:public IP pair
   // NOTE: a leading 'X' character is added to work around the Polycom f/w problem detailed in XTRN-383.
   mSystemIdentificationString = "X";
   mSystemIdentificationString.append( mpNatTraversalRules->getProxyTransportInfo().getAddress() );
   mSystemIdentificationString.append( "-" );
   mSystemIdentificationString.append( mpNatTraversalRules->getPublicTransportInfo().getAddress() );
   OsSysLog::add(FAC_NAT, PRI_DEBUG, "+SessionContext tracker %p created; Handle=%s+",
                                       this,
                                       mHandle.data() );
}

EndpointDescriptor*
SessionContext::createCallerEndpointDescriptor( const SipMessage& sipRequest, const NatTraversalRules& natTraversalRules )
{
   // The Caller endpoint descriptor is initialized based on the information contained in the
   // contact URI.  This is where the NAT traversal feature encodes location information about
   // the caller for dialog-forming requests.
   UtlString tmpString;
   sipRequest.getContactEntry( 0, &tmpString );
   Url contactUri( tmpString );
   return new EndpointDescriptor( contactUri, natTraversalRules );
}

EndpointDescriptor*
SessionContext::createCalleeEndpointDescriptor( const SipMessage& sipRequest, const NatTraversalRules& natTraversalRules, const RegistrationDB* pRegistrationDB )
{
   // The Callee endpoint descriptor is initialized based on the information contained in the
   // Route if present or the Request URI.  The R-URI is where the NAT traversal feature encodes location
   // information about the callee for dialog-forming requests.
   UtlString tmpString;
   UtlBoolean bIsAddrSpec = FALSE;
   if( !sipRequest.getRouteUri( 0, &tmpString ) )
   {
      bIsAddrSpec = TRUE;
      sipRequest.getRequestUri( &tmpString );
   }
   Url requestUri( tmpString, bIsAddrSpec );
   return new EndpointDescriptor( requestUri, natTraversalRules, pRegistrationDB );
}

SessionContext::~SessionContext()
{
   mDialogTrackersMap.destroyAll();

   delete mpReferenceDialogTracker;
   delete mpCaller;
   delete mpCallee;
   OsSysLog::add(FAC_NAT, PRI_DEBUG, "-SessionContext tracker %p deleted; Handle=%s-",
                                       this,
                                       mHandle.data() );
}

bool SessionContext::handleRequest( SipMessage& message, const char* address, int port, bool bFromCallerToCallee )
{
   // This routine steers incoming requests to the DialogTracker instance that is
   // responsible for handling them based on the request's to- or from-tags depending
   // on the directionality of the request.
   ssize_t numberOfDialogTrackersEnteringRoutine = getNumberOfTrackedDialogs();
   bool bTrackRequestResponse = false;

   UtlString discriminatingTag = getDiscriminatingTagValue( message, bFromCallerToCallee );

   // if a discriminating tag was found, try to find a DialogTracker for it.
   if( !discriminatingTag.isNull() )
   {
      DialogTracker* pDialogTracker = 0;
      if( ( pDialogTracker = getDialogTrackerForTag( discriminatingTag ) ) != 0 )
      {
         bTrackRequestResponse = pDialogTracker->handleRequest( message, address, port, bFromCallerToCallee );
      }
      else
      {
         OsSysLog::add(FAC_NAT, PRI_CRIT, "SessionContext[%s]::handleRequest: received in-dialog request with unknown discriminating tag: %s",
                                           mHandle.data(), discriminatingTag.data() );
      }
   }
   else
   {
      // The request does not yet have a discriminating tag.  This is likely indicating a
      // dialog-forming INVITE but to be sure, check that the request is indeed an
      // INVITE in the caller->callee direction.
      UtlString method;
      message.getRequestMethod(&method);
      if( bFromCallerToCallee && method.compareTo( SIP_INVITE_METHOD ) == 0 )
      {
         // The INVITE is dialog-forming.  Check whether or not already have
         // the reference dialog tracker for it.
         if( !mpReferenceDialogTracker )
         {
            // This is the first time we see that dialog-forming request - create
            // a reference dialog tracker that will serve as a template to create
            // new DialogTracker objects for the dialogs that responses to the
            // request will establish.
            Url tempUrl;
            char tempBuffer[50];
            sprintf( tempBuffer, "%s-%s", mHandle.data(), "ref" );

            if( ( mpReferenceDialogTracker = new DialogTracker( tempBuffer, mSystemIdentificationString, this ) ) )
            {
               mpReferenceDialogTracker->handleRequest( message, address, port, bFromCallerToCallee );
               // save the From tag of the dialog-forming request. This will be used to identify
               // the discriminating tag when the directionality of a message is unknown.
               message.getFromUrl( tempUrl );
               tempUrl.getFieldParameter( "tag", mDialogOriginalFromTag );
               mDialogFormingInviteCseq.setValue( message );
               bTrackRequestResponse = true;
            }
         }
         else
         {
            // This dialog-forming request has already been seen - this is likely a
            // retransmission.  Present it to the reference dialog tracker so that
            // it can handle the retransmission properly.
            bTrackRequestResponse = mpReferenceDialogTracker->handleRequest( message, address, port, bFromCallerToCallee );
         }
      }
   }

   // Check if the processing of the request caused the last DialogTracker to be deleted.
   // If so, the SessionContext is not required anymore therefore tell the CallTracker that
   // we are ready for deletion
   if( numberOfDialogTrackersEnteringRoutine &&
       deleteDialogTrackersReadyForDeletion() == numberOfDialogTrackersEnteringRoutine )
   {
      mpOwningCallTracker->reportSessionContextReadyForDeletion( mHandle );
   }
   return bTrackRequestResponse;
}

void SessionContext::handleResponse( SipMessage& message, const char* address, int port )
{
   ssize_t numberOfDialogTrackersEnteringRoutine = getNumberOfTrackedDialogs();
   UtlString discriminatingTag = getDiscriminatingTagValue( message );

   // Retrieve DialogTracker object that handles this dialog.
   DialogTracker* pDialogTracker = 0;
   if( !discriminatingTag.isNull() &&
       ( pDialogTracker = getDialogTrackerForTag( discriminatingTag ) ) != 0 )
   {
      // present the response to the DialogTracker
      pDialogTracker->handleResponse( message, address, port );
   }
   else
   {
      if( message.getResponseStatusCode() < SIP_3XX_CLASS_CODE )
      {
         // we do not have a DialogTracker for this response.  If this is as
         // 1xx or 2xx response to the dialog-forming INVITE, this response is creating a
         // new dialog.  Create a new DialogTracker based on the reference DialogTracker
         if( !discriminatingTag.isNull() && mDialogFormingInviteCseq == CseqData( message ) )
         {
            DialogTracker* pNewDialogTracker;
            if( (pNewDialogTracker = allocateNewDialogTrackerBasedOnReference( discriminatingTag ) ) )
            {
               pNewDialogTracker->handleResponse( message, address, port );
            }
         }
      }
      else if( message.getResponseStatusCode() >= SIP_4XX_CLASS_CODE )
      {
         // This session context has received a final failure response.  The
         // INVITE has been rejected.  Present that response to all the
         // DialogTrackers so that they can terminate.
         UtlHashMapIterator dialogTrackerIterator( mDialogTrackersMap );

         while( dialogTrackerIterator() )
         {
            DialogTracker *pDialogTracker;
            pDialogTracker = dynamic_cast<DialogTracker*>( dialogTrackerIterator.value() );
            pDialogTracker->handleResponse( message, address, port );
         }
      }
   }

   // Check if the processing of the request caused the last DialogTracker to be deleted.
   // If so, the SessionContext is not required anymore therefore tell the CallTracker that
   // we are ready for deletion
   if( numberOfDialogTrackersEnteringRoutine &&
       deleteDialogTrackersReadyForDeletion() == numberOfDialogTrackersEnteringRoutine )
   {
      mpOwningCallTracker->reportSessionContextReadyForDeletion( mHandle );
   }
}

void SessionContext::handleCleanUpTimerTick( void )
{
   ssize_t numberOfDialogTrackersEnteringRoutine = getNumberOfTrackedDialogs();
   UtlHashMapIterator dialogTrackerIterator( mDialogTrackersMap );

   while( dialogTrackerIterator() )
   {
      DialogTracker *pDialogTracker;
      pDialogTracker = dynamic_cast<DialogTracker*>( dialogTrackerIterator.value() );

      pDialogTracker->handleCleanUpTimerTick();
   }

   // Check if the processing of the request caused the last DialogTracker to be deleted.
   // If so, the SessionContext is not required anymore therefore tell the CallTracker that
   // we are ready for deletion
   if( numberOfDialogTrackersEnteringRoutine &&
       deleteDialogTrackersReadyForDeletion() == numberOfDialogTrackersEnteringRoutine )
   {
      mpOwningCallTracker->reportSessionContextReadyForDeletion( mHandle );
   }
}

ssize_t SessionContext::deleteDialogTrackersReadyForDeletion( void )
{
   ssize_t numberOfDialogTrackersToDelete = mListOfDialogTrackersReadyForDeletion.size();
   if( numberOfDialogTrackersToDelete > 0 )
   {
      ssize_t index;
      UtlString handleForDialogTrackerToDelete;

      for( index = 0; index < numberOfDialogTrackersToDelete; index++ )
      {
         handleForDialogTrackerToDelete = mListOfDialogTrackersReadyForDeletion[ index ];
         mDialogTrackersMap.destroy( &handleForDialogTrackerToDelete );
      }
      mListOfDialogTrackersReadyForDeletion.clear();
   }
   return numberOfDialogTrackersToDelete;
}

DialogTracker* SessionContext::allocateNewDialogTrackerBasedOnReference( const UtlString& discriminatingTag )
{
   DialogTracker* pNewDialogTracker = 0;
   pNewDialogTracker = new DialogTracker( *mpReferenceDialogTracker, discriminatingTag );
   if( pNewDialogTracker )
   {
      addDialogTrackerToList( discriminatingTag, pNewDialogTracker );
      OsSysLog::add(FAC_NAT, PRI_DEBUG, "SessionContext[%s]::allocateNewDialogTrackerBasedOnReference: allocated DialogTracker #%zd for tag %s",
                                         mHandle.data(), getNumberOfTrackedDialogs(), discriminatingTag.data() );

      // We have a new tracker that is utilizing the same Media RelaySessions as the
      // reference.  Increment their link count to track the number of DialogTrackers using
      // them and avoid premature de-allocations.
      size_t index;
      size_t numSavedMediaDescriptors = pNewDialogTracker->getNumberOfMediaDescriptors();
      for( index = 0; index < numSavedMediaDescriptors; index++ )
      {
         const MediaDescriptor* pMediaDescriptor;
         pMediaDescriptor = pNewDialogTracker->getReadOnlyMediaDescriptor( index );
         tMediaRelayHandle tempMediaRelayHandle;

         if( ( tempMediaRelayHandle = pMediaDescriptor->getTentativeInitialMediaRelayHandle() ) != INVALID_MEDIA_RELAY_HANDLE )
         {
            mpMediaRelay->incrementLinkCountOfMediaRelaySession( tempMediaRelayHandle );
         }

         if( ( tempMediaRelayHandle = pMediaDescriptor->getTentativeNonInitialMediaRelayHandle() ) != INVALID_MEDIA_RELAY_HANDLE )
         {
            mpMediaRelay->incrementLinkCountOfMediaRelaySession( tempMediaRelayHandle );
         }

         if( ( tempMediaRelayHandle = pMediaDescriptor->getCurrentMediaRelayHandle() ) != INVALID_MEDIA_RELAY_HANDLE )
         {
            mpMediaRelay->incrementLinkCountOfMediaRelaySession( tempMediaRelayHandle );
         }
      }
   }
   return pNewDialogTracker;
}

UtlString SessionContext::getDiscriminatingTagValue( const SipMessage& message, bool bFromCallerToCallee ) const
{
   UtlString discriminatingTag;
   Url tempUrl;

   if( bFromCallerToCallee )
   {
      // caller-to-callee uses To-tag to distinguish between dialogs
      message.getToUrl( tempUrl );
   }
   else
   {
      // callee-to-caller uses From-tag to distinguish between dialogs
      message.getFromUrl( tempUrl );
   }
   tempUrl.getFieldParameter( "tag", discriminatingTag );
   return discriminatingTag;
}

UtlString SessionContext::getDiscriminatingTagValue( const SipMessage& message ) const
{
   UtlString discriminatingTag;
   Url tempUrl;

   // We do not know the directionality of the message.  In this case
   // we cannot tell if the discriminating tag will come from the From:
   // or To: header.  Return the one that does not match the dialog's
   // original From-tag.

   // Look at the To-Tag first
   message.getToUrl( tempUrl );
   tempUrl.getFieldParameter( "tag", discriminatingTag );
   if( discriminatingTag == mDialogOriginalFromTag )
   {
      message.getFromUrl( tempUrl );
      tempUrl.getFieldParameter( "tag", discriminatingTag );
   }
   return discriminatingTag;
}


DialogTracker* SessionContext::getDialogTrackerForTag( const UtlString& tag ) const
{
   DialogTracker* pMatchingDialogTracker = 0;
   pMatchingDialogTracker = dynamic_cast<DialogTracker*>(mDialogTrackersMap.findValue( &tag ) );
   return pMatchingDialogTracker;
}

DialogTracker* SessionContext::getDialogTrackerForMessage( const SipMessage& message ) const
{
   UtlString discriminatingTag;
   discriminatingTag = getDiscriminatingTagValue( message );
   return getDialogTrackerForTag( discriminatingTag );
}


void SessionContext::addDialogTrackerToList( const UtlString& tag,  DialogTracker* pNewDialogTracker )
{
   UtlString* pMapKey = new UtlString( tag );
   if( !mDialogTrackersMap.insertKeyAndValue( pMapKey, pNewDialogTracker ) )
   {
      OsSysLog::add(FAC_NAT, PRI_CRIT, "SessionContext[%s]::addDialogTrackerToList failed to insert value for key %s",
                                         mHandle.data(), tag.data() );
   }
}

ssize_t SessionContext::getNumberOfTrackedDialogs( void ) const
{
   return mDialogTrackersMap.entries();
}


bool SessionContext::removeDialogTrackerFromListAndDelete( const UtlString& tag )
{
  return ( mDialogTrackersMap.destroy( &tag ) == TRUE );
}

bool SessionContext::doesEndpointsLocationImposeMediaRelay( void ) const
{
   // The need to use a media relay for the session will be
   // determined by the location of the caller and callee.
   //  IF both the caller and callee are 'PUBLIC' then no media relay is used
   //  IF both the caller and callee are 'LOCAL_NATED' then no media relay is used
   //  IF both the caller and callee are 'REMOTE_NATED' AND they both
   //     have the same public IP address AND the relay mode is set to 'conservative'
   //     then no media relay is used.
   //  ALL other scenarios require a media relay.
   bool bMediaRelayNeeded = true;

   LocationCode callerLocation;
   LocationCode calleeLocation;

   callerLocation = mpCaller->getLocationCode();
   calleeLocation = mpCallee->getLocationCode();

   if( ( callerLocation == PUBLIC &&
         calleeLocation == PUBLIC
       ) ||
       ( callerLocation == LOCAL_NATED &&
         calleeLocation == LOCAL_NATED
       )
     )
   {
      bMediaRelayNeeded = false;
   }
   else if( callerLocation == REMOTE_NATED && calleeLocation == REMOTE_NATED )
   {
      // both the caller and the callee are remote NATed.  What we do next depends
      // on the configured media relay mode.  When the media relay mode is 'aggressive'
      // a media relay is always employed for remote NATed endpoints.  However, when
      // the mode is 'conservative', the logic will refrain from utilizing a media relay
      // if the two endpoints share the same public IP address.  In this case, it is
      // assumed that both endpoints are behind the same NAT and that we can conserve a
      // a Media relay as the endpoints can reach each other directly.
      if( mpNatTraversalRules->isConservativeModeSet() )
      {
         if( mpCaller->getPublicTransportAddress().getAddress() ==
             mpCallee->getPublicTransportAddress().getAddress() )
         {
            bMediaRelayNeeded = false;
         }
      }
   }
   return bMediaRelayNeeded;
}

bool SessionContext::allocateMediaRelaySession( const UtlString& handleOfRequestingDialogContext,
                         tMediaRelayHandle& relayHandle, int& callerRelayRtpPort, int& calleeRelayRtpPort )
{
   bool bAllocationSucceeded = false;
   relayHandle = INVALID_MEDIA_RELAY_HANDLE;

   if( mpMediaRelay->allocateSession( relayHandle, callerRelayRtpPort, calleeRelayRtpPort ) )
   {
      bAllocationSucceeded = true;
   }
   OsSysLog::add(FAC_NAT, PRI_DEBUG, "SessionContext[%s]::allocateMediaRelaySession for dialog tracker %s: handle %d: caller %d; callee %d",
                                     mHandle.data(), handleOfRequestingDialogContext.data(), (int)relayHandle, callerRelayRtpPort, calleeRelayRtpPort );
   return bAllocationSucceeded;
}

tMediaRelayHandle SessionContext::cloneMediaRelaySession( const UtlString& handleOfRequestingDialogContext,
                         tMediaRelayHandle& relayHandleToClone, bool doSwapCallerAndCallee )
{
   tMediaRelayHandle clonedRelayHandle;
   clonedRelayHandle = mpMediaRelay->cloneSession( relayHandleToClone, doSwapCallerAndCallee );
   OsSysLog::add(FAC_NAT, PRI_DEBUG, "SessionContext[%s]::SessionContextcloneMediaRelaySession for dialog tracker %s: src handle %d, dest handle %d",
                                     mHandle.data(), handleOfRequestingDialogContext.data(), (int)relayHandleToClone, (int)clonedRelayHandle );
   return clonedRelayHandle;
}

bool SessionContext::deallocateMediaRelaySession( const UtlString& handleOfRequestingDialogContext,
                         const tMediaRelayHandle& relayHandle )
{
   OsSysLog::add(FAC_NAT, PRI_DEBUG, "SessionContext[%s]::deallocateMediaRelaySession for dialog tracker %s: handle %d",
                                     mHandle.data(), handleOfRequestingDialogContext.data(), (int)relayHandle );
   return mpMediaRelay->deallocateSession( relayHandle );
}

bool SessionContext::setMediaRelayDirectionMode(  const UtlString& handleOfRequestingDialogContext,
                                                  const tMediaRelayHandle& relayHandle,
                                                  MediaDirectionality mediaRelayDirectionMode,
                                                  EndpointRole endpointRole )
{
   // Media Relay API is referenced from the caller. If the requesting enpoint is the callee, we
   // need to turn the RECV_ONLY and SEND_ONLY into SEND_ONLY and RECV_ONLY, respectively.
   if( endpointRole == CALLEE )
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
   return mpMediaRelay->setDirectionMode( relayHandle, mediaRelayDirectionMode );
}

bool SessionContext::linkFarEndMediaRelayPortToRequester(  const UtlString& handleOfRequestingDialogContext,
                                                           const tMediaRelayHandle& relayHandle,
                                                           const MediaDescriptor* pMediaDescriptor,
                                                           EndpointRole endpointRoleOfRequester )
{
   EndpointRole farEndRole;
   EndpointDescriptor *pRequestingEndpointDescriptor;

   if( endpointRoleOfRequester == CALLER )
   {
      farEndRole = CALLEE;
      pRequestingEndpointDescriptor = mpCaller;
   }
   else
   {
      farEndRole = CALLER;
      pRequestingEndpointDescriptor = mpCallee;
   }

   bool bLinkPerformed = false;

   LocationCode requestingEndpointLocation;
   UtlString    requestingEndpointIpAddress;
   int          requestingEndpointRtpPort;
   int          requestingEndpointRtcpPort;

   requestingEndpointLocation = pRequestingEndpointDescriptor->getLocationCode();

   if( requestingEndpointLocation != UNKNOWN &&
       pMediaDescriptor->getEndpoint( endpointRoleOfRequester ).getAddress() == pRequestingEndpointDescriptor->getNativeTransportAddress().getAddress() )
   {
      // SDP originates from the actual endpoint, the call is not 3PCC. If the endpoint falls into one
      // of these two categories then link the media relay to the endpoint's media IP and port
      // #1 - the location of the endpoint is PUBLIC
      // #2 - the location of the endpoint is LOCAL_NATED and the media server is location in the same subnet as sipXecs
      if( ( requestingEndpointLocation == PUBLIC ) ||
          ( requestingEndpointLocation == LOCAL_NATED &&
            mpMediaRelay->isPartOfsipXLocalPrivateNetwork() ) )
      {
         // the endpoint is at a predictable IP address and port, no need to autolearn anything
         requestingEndpointIpAddress = pMediaDescriptor->getEndpoint( endpointRoleOfRequester ).getAddress();
         requestingEndpointRtpPort =   pMediaDescriptor->getEndpoint( endpointRoleOfRequester ).getRtpPort();
         requestingEndpointRtcpPort =  pMediaDescriptor->getEndpoint( endpointRoleOfRequester ).getRtcpPort();
      }
      else
      {
         // the endpoint is behind a NAT relative to the media relay server.  This means that its
         // media IP address is going to be its public IP address but the port is non-deterministic
         requestingEndpointIpAddress = pRequestingEndpointDescriptor->getPublicTransportAddress().getAddress();
         requestingEndpointRtpPort   = 0;
         requestingEndpointRtcpPort  = 0;
      }
   }
   else
   {
      // The location of the endpoint is UNKNOWN or the IP address in the SDP does not
      // belong to the sender of the SDP in which case we may be in the presence or a 3PCC.
      // In either case, since we cannot establish the position of the endpoint relative
      // to the media relay, let's assume the worst case and autolearn both the IP address
      // and port number.
      requestingEndpointIpAddress =  "";
      requestingEndpointRtpPort   =  0;
      requestingEndpointRtcpPort  =  0;
   }
   OsSysLog::add(FAC_NAT, PRI_DEBUG, "SessionContext[%s]::linkMediaRelayPortToFarEnd for dialog tracker %s: relay handle %d - linking far-end relay port to '%s':%d,%d",
                                     mHandle.data(), handleOfRequestingDialogContext.data(), (int)(relayHandle.getValue()), requestingEndpointIpAddress.data(), requestingEndpointRtpPort, requestingEndpointRtcpPort );

   bLinkPerformed = mpMediaRelay->linkSymToEndpoint( relayHandle, requestingEndpointIpAddress, requestingEndpointRtpPort, requestingEndpointRtcpPort, farEndRole );
   return bLinkPerformed;
}

bool SessionContext::getMediaRelayAddressToUseInSdp( UtlString& mediaRelayAddressToUse, EndpointRole endpointRole ) const
{
   bool bUseNativeMediaRelayIpAddress = false;

   mediaRelayAddressToUse.remove( 0 );
   if( mpMediaRelay->isPartOfsipXLocalPrivateNetwork() )
   {
      // if both the receiver of the SDP and the media relay are located with the
      // same network, use the media relay's native IP address in the SDP, otherwise
      // use its public.
      EndpointDescriptor* pSdpRecipient = ( endpointRole == CALLER ? mpCallee : mpCaller );
      if( pSdpRecipient->getLocationCode() == LOCAL_NATED )
      {
         mediaRelayAddressToUse = mpMediaRelay->getNativeAddress();
         bUseNativeMediaRelayIpAddress = true;
      }
      else
      {
         mediaRelayAddressToUse = mpMediaRelay->getPublicAddress();
      }
   }
   else
   {
      mediaRelayAddressToUse = mpMediaRelay->getPublicAddress();
   }
   return bUseNativeMediaRelayIpAddress;
}

int SessionContext::getRtpRelayPortForMediaRelaySession( const tMediaRelayHandle& handle, EndpointRole endpointRole )
{
   return mpMediaRelay->getRtpRelayPortForMediaRelaySession( handle, endpointRole );
}

void SessionContext::reportDialogTrackerReadyForDeletion( const UtlString& handleOfRequestingDialogContext )
{
   OsSysLog::add(FAC_NAT, PRI_DEBUG, "SessionContext[%s]::reportDialogTrackerReadyForDeletion for dialog tracker %s",
                                     mHandle.data(), handleOfRequestingDialogContext.data() );
   mListOfDialogTrackersReadyForDeletion.push_back( handleOfRequestingDialogContext );
}

bool SessionContext::getPacketProcessingStatsForMediaRelaySession( const tMediaRelayHandle& handle,
                                                                   PacketProcessingStatistics& stats )
{
   return mpMediaRelay->getPacketProcessingStatsForMediaRelaySession( handle, stats );
}

const EndpointDescriptor& SessionContext::getEndpointDescriptor( EndpointRole endpointRole ) const
{
   return ( endpointRole == CALLER ? *mpCaller : *mpCallee );
}

UtlContainableType SessionContext::getContainableType( void ) const
{
   return SessionContext::TYPE;
}

unsigned SessionContext::hash() const
{
   return directHash();
}

int SessionContext::compareTo(UtlContainable const *rhsContainable ) const
{
   int result = -1;
   if ( rhsContainable->isInstanceOf( SessionContext::TYPE ) )
   {
      result = mHandle.compareTo( static_cast<const SessionContext*>(rhsContainable)->mHandle );
   }
   return result;
}
SessionContext::CseqData::CseqData() :
   mSeqNum( 0 )
{

}

SessionContext::CseqData::CseqData( const SipMessage& message )
{
   setValue( message );
}

bool SessionContext::CseqData::operator==( const CseqData& rhs )
{
   return ( mSeqNum == rhs.mSeqNum ) &&
          ( mMethod == rhs.mMethod );
}

void SessionContext::CseqData::setValue( const SipMessage& message )
{
   message.getCSeqField( &mSeqNum, &mMethod );
}
