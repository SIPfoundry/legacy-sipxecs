//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include "os/OsLogger.h"

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
                                RegDB* pRegDb,
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
      Os::Logger::instance().log(FAC_NAT, PRI_DEBUG, "SessionContext[%s]::SessionContext: Caller transport info:'%s'",
                                        mHandle.data(), tmpString.data() );
   }

   // Second, initialize the callee descriptor based on the Request URI header
   {
      mpCallee = createCalleeEndpointDescriptor( sipRequest, *mpNatTraversalRules, pRegDb );
      mpCallee->toString( tmpString );
      Os::Logger::instance().log(FAC_NAT, PRI_DEBUG, "SessionContext[%s]::SessionContext: Callee transport info:'%s'",
                                        mHandle.data(), tmpString.data() );
   }

   // Create the string that will be used to mark packets as handled by this system.  This
   // string must be unique for this system and is set to the system's private:public IP pair
   // NOTE: a leading 'X' character is added to work around the Polycom f/w problem detailed in XTRN-383.
   mSystemIdentificationString = "X";
   mSystemIdentificationString.append( mpNatTraversalRules->getProxyTransportInfo().getAddress() );
   mSystemIdentificationString.append( "-" );
   mSystemIdentificationString.append( mpNatTraversalRules->getPublicTransportInfo().getAddress() );
   Os::Logger::instance().log(FAC_NAT, PRI_DEBUG, "+SessionContext tracker %p created; Handle=%s+",
                                       this,
                                       mHandle.data() );
}

EndpointDescriptor*
SessionContext::createCallerEndpointDescriptor( const SipMessage& sipRequest, const NatTraversalRules& natTraversalRules )
{
   // The Caller endpoint descriptor is initialized based on the information contained in the
   // contact URI.  This is where the NAT traversal feature encodes location information about
   // the caller for dialog-forming requests.
   UtlString tmpString = sipRequest.getHeaderValue( 0, SIP_SIPX_NTAP_CONTACT );
   if ( tmpString.isNull() )
   {
     sipRequest.getContactEntry( 0, &tmpString );
   }

   Url contactUri( tmpString );

   return new EndpointDescriptor( contactUri, natTraversalRules );
}

EndpointDescriptor*
SessionContext::createCalleeEndpointDescriptor( const SipMessage& sipRequest, const NatTraversalRules& natTraversalRules, RegDB* pRegDB )
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
   return new EndpointDescriptor( requestUri, natTraversalRules, pRegDB );
}

SessionContext::~SessionContext()
{
   mDialogTrackersMap.destroyAll();

   delete mpReferenceDialogTracker;
   delete mpCaller;
   delete mpCallee;
   Os::Logger::instance().log(FAC_NAT, PRI_DEBUG, "-SessionContext tracker %p deleted; Handle=%s-",
                                       this,
                                       mHandle.data() );
}

bool SessionContext::handleRequest( SipMessage& message,
                                    const char* address,
                                    int port,
                                    bool bFromCallerToCallee,
                                    bool* reevaluateDestination )
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
         bTrackRequestResponse = pDialogTracker->handleRequest( message, address, port, bFromCallerToCallee, reevaluateDestination );
      }
      else
      {
         Os::Logger::instance().log(FAC_NAT, PRI_CRIT, "SessionContext[%s]::handleRequest: received in-dialog request with unknown discriminating tag: %s",
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

            if( ( mpReferenceDialogTracker = new DialogTracker( tempBuffer, mSystemIdentificationString, this, mpNatTraversalRules ) ) )
            {
               mpReferenceDialogTracker->handleRequest( message, address, port, bFromCallerToCallee, reevaluateDestination );
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
            bTrackRequestResponse = mpReferenceDialogTracker->handleRequest( message, address, port, bFromCallerToCallee, reevaluateDestination );
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
   pNewDialogTracker = new DialogTracker( *mpReferenceDialogTracker, discriminatingTag, mpNatTraversalRules );
   if( pNewDialogTracker )
   {
      addDialogTrackerToList( discriminatingTag, pNewDialogTracker );
      Os::Logger::instance().log(FAC_NAT, PRI_DEBUG, "SessionContext[%s]::allocateNewDialogTrackerBasedOnReference: allocated DialogTracker #%zd for tag %s",
                                         mHandle.data(), getNumberOfTrackedDialogs(), discriminatingTag.data() );

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
      Os::Logger::instance().log(FAC_NAT, PRI_CRIT, "SessionContext[%s]::addDialogTrackerToList failed to insert value for key %s",
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

bool SessionContext::doesEndpointsLocationImposeMediaRelay( const SipMessage& request ) const
{
  
  //
  // If the request is a result of a hairpin from sipXbridge, media relay is not required.
  // sipXbridge will handle all media relay requirement
  //
  if (mpNatTraversalRules->isBridgeHairPin(request))
  {
    OS_LOG_INFO(FAC_SIP, "SessionContext::doesEndpointsLocationImposeMediaRelay - detected bridge hairpin.  Backing off ...");
    return false;
  }
  
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

void SessionContext::reportDialogTrackerReadyForDeletion( const UtlString& handleOfRequestingDialogContext )
{
   Os::Logger::instance().log(FAC_NAT, PRI_DEBUG, "SessionContext[%s]::reportDialogTrackerReadyForDeletion for dialog tracker %s",
                                     mHandle.data(), handleOfRequestingDialogContext.data() );
   mListOfDialogTrackersReadyForDeletion.push_back( handleOfRequestingDialogContext );
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
