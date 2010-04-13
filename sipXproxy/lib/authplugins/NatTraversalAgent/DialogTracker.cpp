//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include "os/OsSysLog.h"

// APPLICATION INCLUDES
#include "DialogTracker.h"
#include "SessionContext.h"
#include "net/SdpBody.h"
#include "net/SipMessage.h"
#include "utl/UtlFsm.h"
#include "utl/UtlTokenizer.h"

// DEFINES

const UtlContainableType DialogTracker::TYPE = "DialogTracker";


// TYPE DEFINITIONS
struct DialogTrackerStateStruct
{
   WaitingForInvite                           waitingForInvite;
   WaitingForAckForInvite                     waitingForAckForInvite;
   TimeBoundState                             timeBoundState;
   Negotiating                                negotiating;
   WaitingForMediaOffer                       waitingForMediaOffer;
   WaitingFor200OkWithMediaOffer              waitingFor200OkWithMediaOffer;
   WaitingForMediaAnswer                      waitingForMediaAnswer;
   ProcessingPrack                            processingPrack;
   WaitingForPrack                            waitingForPrack;
   WaitingForAckWithAnswerForInvite           waitingForAckWithAnswerForInvite;
   WaitingForPrackWithMediaAnswer             waitingForPrackWithMediaAnswer;
   WaitingFor200OkForSlowStartPrack           waitingFor200OkForSlowStartPrack;
   WaitingFor200OkForPrack                    waitingFor200OkForPrack;
   WaitingFor200OkWithAnswerForPrack          waitingFor200OkWithAnswerForPrack;
   ProcessingPrackWaitingForAckforInvite      processingPrackWaitingForAckforInvite;
   Moribund                                   moribund; };

// STATICS INITIALIZATION
WaitingForInvite*                           DialogTracker::pWaitingForInvite = 0;
WaitingForAckForInvite*                     DialogTracker::pWaitingForAckForInvite = 0;
TimeBoundState*                             DialogTracker::pTimeBoundState = 0;
Negotiating*                                DialogTracker::pNegotiating = 0;
WaitingForMediaOffer*                       DialogTracker::pWaitingForMediaOffer = 0;
WaitingFor200OkWithMediaOffer*              DialogTracker::pWaitingFor200OkWithMediaOffer = 0;
WaitingForMediaAnswer*                      DialogTracker::pWaitingForMediaAnswer = 0;
ProcessingPrack*                            DialogTracker::pProcessingPrack = 0;
WaitingForPrack*                            DialogTracker::pWaitingForPrack = 0;
WaitingForAckWithAnswerForInvite*           DialogTracker::pWaitingForAckWithAnswerForInvite = 0;
WaitingForPrackWithMediaAnswer*             DialogTracker::pWaitingForPrackWithMediaAnswer = 0;
WaitingFor200OkForSlowStartPrack*           DialogTracker::pWaitingFor200OkForSlowStartPrack = 0;
WaitingFor200OkForPrack*                    DialogTracker::pWaitingFor200OkForPrack = 0;
WaitingFor200OkWithAnswerForPrack*          DialogTracker::pWaitingFor200OkWithAnswerForPrack = 0;
ProcessingPrackWaitingForAckforInvite*      DialogTracker::pProcessingPrackWaitingForAckforInvite = 0;
Moribund*                                   DialogTracker::pMoribund = 0;

DialogTracker::DialogTracker( const DialogTracker& referenceDialogTracker,
                              const UtlString& newHandle ) :
  mpCopyOfPatchedSdpBody( 0 )
{
   // Init state machine pointers
   initializeStatePointers();

   // Make copy of copyable members
   mHandle                                = newHandle;
   mSystemIdentificationString            = referenceDialogTracker.mSystemIdentificationString;
   mbMediaRelayRequired                   = referenceDialogTracker.mbMediaRelayRequired;
   mbDialogEstablished                    = referenceDialogTracker.mbDialogEstablished;
   mTimerTickCounter                      = referenceDialogTracker.mTimerTickCounter;
   pOwningSessionContext                  = referenceDialogTracker.pOwningSessionContext;
   mbNonIntialOfferAnswerExchangeDoneFlag = referenceDialogTracker.mbNonIntialOfferAnswerExchangeDoneFlag;
   mCurrentTransactionDirectionality      = referenceDialogTracker.mCurrentTransactionDirectionality;
   mpCurrentState                         = referenceDialogTracker.mpCurrentState;
   mRequestRetransmissionDescriptor       = referenceDialogTracker.mRequestRetransmissionDescriptor;
   mResponseRetransmissionDescriptor      = referenceDialogTracker.mResponseRetransmissionDescriptor;

   //Make deep copy of members that require it
   vector<MediaDescriptor*>::const_iterator pos;
   for( pos = referenceDialogTracker.mMediaDescriptors.begin();
        pos != referenceDialogTracker.mMediaDescriptors.end();
        ++pos )
   {
      MediaDescriptor* pMediaDescriptor = new MediaDescriptor( *(*pos) );
      appendMediaDescriptor( pMediaDescriptor );
   }
   OsSysLog::add(FAC_NAT, PRI_DEBUG, "+DialogTracker tracker %p created; Handle=%s+",
                                       this,
                                       mHandle.data() );
}

DialogTracker::DialogTracker( const UtlString& handle,
                              const UtlString& systemIdentificationString,
                              SessionContextInterfaceForDialogTracker* pOwningSessionContext ) :
   mHandle( handle ),
   mSystemIdentificationString( systemIdentificationString ),
   mbMediaRelayRequired( false ),
   mbDialogEstablished( false ),
   mbInitialDialogEstablished( false ),
   mTimerTickCounter( 0 ),
   pOwningSessionContext( pOwningSessionContext ),
   mbNonIntialOfferAnswerExchangeDoneFlag( false )
{
   // Init state pointers
   initializeStatePointers();

   // kickstart the state mahine
   const DialogTrackerState* pState = pWaitingForInvite;
   StateAlg::StartStateMachine( *this, pState );

   OsSysLog::add(FAC_NAT, PRI_DEBUG, "+DialogTracker tracker %p created; Handle=%s+",
                                       this,
                                       mHandle.data() );
}

DialogTracker::~DialogTracker()
{
   deallocateAndClearAllMediaRelaySessions();
   vector<MediaDescriptor*>::iterator pos;
   for( pos = mMediaDescriptors.begin(); pos != mMediaDescriptors.end(); ++pos )
   {
      delete *pos;
   }
   OsSysLog::add(FAC_NAT, PRI_DEBUG, "-DialogTracker tracker %p deleted; Handle=%s-",
                                       this,
                                       mHandle.data() );
}

bool DialogTracker::handleRequest( SipMessage& message, const char* address, int port, bool bFromCallerToCallee )
{
   bool bTrackRequestResponse = true;

   TransactionDirectionality directionality = ( bFromCallerToCallee ? DIR_CALLER_TO_CALLEE : DIR_CALLEE_TO_CALLER );
   UtlString method;
   message.getRequestMethod(&method);

   // remove all the elements in the message that may impair NAT traversal
   removeUnwantedElements( message );

   OsSysLog::add(FAC_NAT, PRI_DEBUG, "DialogTracker[%s]::handleRequest: received request: %s; Caller->Callee?: %d",
                                     mHandle.data(), method.data(), bFromCallerToCallee );

   if( method.compareTo(SIP_INVITE_METHOD) == 0 )
   {
      bTrackRequestResponse =
         mpCurrentState->InviteRequest( *this, message, directionality, address, port );
   }
   else if( method.compareTo(SIP_ACK_METHOD) == 0 )
   {
      bTrackRequestResponse =
         mpCurrentState->AckRequest( *this, message, directionality, address, port );
   }
   else if( method.compareTo(SIP_BYE_METHOD) == 0 )
   {
      bTrackRequestResponse =
         mpCurrentState->ByeRequest( *this, message, directionality, address, port );
   }
   else if( method.compareTo(SIP_PRACK_METHOD) == 0 )
   {
      bTrackRequestResponse =
         mpCurrentState->PrackRequest( *this, message, directionality, address, port );
   }
   else if( method.compareTo(SIP_UPDATE_METHOD) == 0 )
   {
      bTrackRequestResponse =
         mpCurrentState->UpdateRequest( *this, message, directionality, address, port );
   }
   else
   {
      OsSysLog::add(FAC_NAT, PRI_DEBUG, "DialogTracker[%s]::DialogTracker: received unhandled request: %s",
                                        mHandle.data(), method.data() );
   }
   return bTrackRequestResponse;
}

void DialogTracker::handleResponse( SipMessage& message, const char* address, int port )
{
   int responseCode = message.getResponseStatusCode();

   // remove all the elements in the message that may impair NAT traversal
   removeUnwantedElements( message );

   OsSysLog::add(FAC_NAT, PRI_DEBUG, "DialogTracker[%s]::handleResponse: received response: %d",
                                     mHandle.data(), responseCode );

   if ( responseCode < SIP_2XX_CLASS_CODE )
   {
      mpCurrentState->ProvisionalResponse( *this, message, address, port );
   }
   else if ( responseCode < SIP_3XX_CLASS_CODE )
   {
      mpCurrentState->SuccessfulResponse( *this, message, address, port );
   }
   else if ( responseCode < SIP_4XX_CLASS_CODE )
   {
      mpCurrentState->RedirectionResponse( *this, message, address, port );
   }
   else
   {
      mpCurrentState->FailureResponse( *this, message, address, port );
   }
}

void DialogTracker::handleCleanUpTimerTick( void )
{
   mpCurrentState->CleanUpTimerTick( *this );
}

const DialogTrackerState* DialogTracker::GetCurrentState() const
{
   return mpCurrentState;
}

void DialogTracker::SetCurrentState( const DialogTrackerState* pState )
{
   mpCurrentState = pState;
}

const char* DialogTracker::name( void ) const
{
   return mHandle.data();
}

bool DialogTracker::getDialogEstablishedFlag( void ) const
{
  return mbDialogEstablished;
}

void DialogTracker::setDialogEstablishedFlag( void )
{
   mbDialogEstablished = true;
}

void DialogTracker::clearDialogEstablishedFlag( void )
{
   mbDialogEstablished = false;
}

bool DialogTracker::getMediaRelayRequiredFlag( void ) const
{
   return mbMediaRelayRequired;
}

void DialogTracker::setMediaRelayRequiredFlag( void )
{
   mbMediaRelayRequired = true;
}

void DialogTracker::clearMediaRelayRequiredFlag( void )
{
   mbMediaRelayRequired = false;
}

size_t DialogTracker::getNumberOfMediaDescriptors( void ) const
{
   return mMediaDescriptors.size();
}

MediaDescriptor* DialogTracker::getModifiableMediaDescriptor( size_t descriptorIndex )
{
   MediaDescriptor* pMediaDescriptor = 0;
   if( descriptorIndex < mMediaDescriptors.size() )
   {
      pMediaDescriptor = mMediaDescriptors[ descriptorIndex ];
   }
   return pMediaDescriptor;
}

const MediaDescriptor* DialogTracker::getReadOnlyMediaDescriptor( size_t descriptorIndex )
{
   return getModifiableMediaDescriptor( descriptorIndex );
}

void DialogTracker::appendMediaDescriptor( MediaDescriptor* pMediaDescriptor )
{
   if( pMediaDescriptor )
   {
      mMediaDescriptors.push_back( pMediaDescriptor );
   }
}

void DialogTracker::resetTimerTickCounter( void )
{
   mTimerTickCounter = 0;
}
ssize_t DialogTracker::incrementTimerTickCounter( void )
{
   mTimerTickCounter++;
   return mTimerTickCounter;
}
ssize_t DialogTracker::getTimerTickCounter( void ) const
{
   return mTimerTickCounter;
}

bool DialogTracker::doesEndpointsLocationImposeMediaRelay( void ) const
{
   return pOwningSessionContext->doesEndpointsLocationImposeMediaRelay();
}

bool DialogTracker::allocateMediaRelaySession( tMediaRelayHandle& relayHandle, int& callerRelayRtpPort, int& calleeRelayRtpPort )
{
   return pOwningSessionContext->allocateMediaRelaySession( mHandle, relayHandle, callerRelayRtpPort, calleeRelayRtpPort );
}

tMediaRelayHandle DialogTracker::cloneMediaRelaySession( tMediaRelayHandle& relayHandleToClone, bool doSwapCallerAndCallee )
{
   return pOwningSessionContext->cloneMediaRelaySession( mHandle, relayHandleToClone, doSwapCallerAndCallee );
}

bool DialogTracker::deallocateMediaRelaySession( const tMediaRelayHandle& relayHandle )
{
   return pOwningSessionContext->deallocateMediaRelaySession( mHandle, relayHandle );
}

void DialogTracker::promoteTentativeMediaRelaySessionsToCurrent( void )
{
   size_t index;
   size_t numSavedMediaDescriptors = getNumberOfMediaDescriptors();
   for( index = 0; index < numSavedMediaDescriptors; index++ )
   {
      MediaDescriptor* pMediaDescriptor;
      pMediaDescriptor = getModifiableMediaDescriptor( index );
      tMediaRelayHandle tempMediaRelayHandle;

      // a new MediaRelaySession is going to become the new 'current' one.  De-allocate the MediaRelaySession that
      // is currently the 'current' one.
      if( ( tempMediaRelayHandle = pMediaDescriptor->getCurrentMediaRelayHandle() ) != INVALID_MEDIA_RELAY_HANDLE )
      {
         deallocateMediaRelaySession( tempMediaRelayHandle );
      }

      if( getNonIntialOfferAnswerExchangeDoneFlag() == true )
      {
         // a non-initial media negotiation took place.  Use non-initial tentative MediaRelaySession
         // as the new 'current' and deallocate the MediaRelaySession that got allocated during the
         // processing of the initial media negotiation.
         if( ( tempMediaRelayHandle = pMediaDescriptor->getTentativeInitialMediaRelayHandle() ) != INVALID_MEDIA_RELAY_HANDLE )
         {
            deallocateMediaRelaySession( tempMediaRelayHandle );
            pMediaDescriptor->clearTentativeInitialMediaRelayHandle();
         }
         pMediaDescriptor->setCurrentMediaRelayHandle( pMediaDescriptor->getTentativeNonInitialMediaRelayHandle() );
         pMediaDescriptor->clearTentativeNonInitialMediaRelayHandle();
      }
      else
      {
         // No non-initial media negotiation took place.  Use initial tentative MediaRelaySession
         // as the new 'current'.
         pMediaDescriptor->setCurrentMediaRelayHandle( pMediaDescriptor->getTentativeInitialMediaRelayHandle() );
         pMediaDescriptor->clearTentativeInitialMediaRelayHandle();
      }
   }
}

void DialogTracker::deallocateAndClearAllMediaRelaySessions( bool bDeallocateTentativeInitialRelays,
                                                             bool bDeallocateTentativeNonInitialRelays,
                                                             bool bDeallocateCurrentRelays )
{
   size_t index;
   size_t numSavedMediaDescriptors = getNumberOfMediaDescriptors();
   for( index = 0; index < numSavedMediaDescriptors; index++ )
   {
      MediaDescriptor* pMediaDescriptor;
      pMediaDescriptor = getModifiableMediaDescriptor( index );
      tMediaRelayHandle tempMediaRelayHandle;

      if( bDeallocateTentativeInitialRelays &&
          ( tempMediaRelayHandle = pMediaDescriptor->getTentativeInitialMediaRelayHandle() ) != INVALID_MEDIA_RELAY_HANDLE )
      {
         deallocateMediaRelaySession( tempMediaRelayHandle );
         pMediaDescriptor->clearTentativeInitialMediaRelayHandle();
      }

      if( bDeallocateTentativeNonInitialRelays && ( tempMediaRelayHandle = pMediaDescriptor->getTentativeNonInitialMediaRelayHandle() ) != INVALID_MEDIA_RELAY_HANDLE )
      {
         deallocateMediaRelaySession( tempMediaRelayHandle );
         pMediaDescriptor->clearTentativeNonInitialMediaRelayHandle();
      }

      if( bDeallocateCurrentRelays && ( tempMediaRelayHandle = pMediaDescriptor->getCurrentMediaRelayHandle() ) != INVALID_MEDIA_RELAY_HANDLE )
      {
         deallocateMediaRelaySession( tempMediaRelayHandle );
         pMediaDescriptor->clearCurrentMediaRelayHandle();
      }
   }
}

bool DialogTracker::wasMediaTrafficSeenInLastNSeconds( unsigned long numberOfSeconds )
{
   bool bTrafficSeen = false;
   unsigned long currentEpochTime = OsDateTime::getSecsSinceEpoch();
   unsigned long lastPacketsProcessedThreshold;

   lastPacketsProcessedThreshold = ( currentEpochTime > numberOfSeconds ?
                                       currentEpochTime - numberOfSeconds : 0 );

   ssize_t index;
   ssize_t numSavedMediaDescriptors = getNumberOfMediaDescriptors();
   ssize_t numOfActiveSession = 0;

   if( numSavedMediaDescriptors > 0 )
   {
      PacketProcessingStatistics stats;
      const MediaDescriptor* pMediaDescriptor;
      tMediaRelayHandle tempMediaRelayHandle;

      for( index = 0; index < numSavedMediaDescriptors; index++ )
      {
         pMediaDescriptor = getReadOnlyMediaDescriptor( index );

         if( pMediaDescriptor->getDirectionality() != INACTIVE )
         {
            if( ( tempMediaRelayHandle = pMediaDescriptor->getCurrentMediaRelayHandle() ) != INVALID_MEDIA_RELAY_HANDLE )
            {
               numOfActiveSession++;
               if( pOwningSessionContext->getPacketProcessingStatsForMediaRelaySession( tempMediaRelayHandle, stats ) )
               {
                  if( stats.mEpochTimeOfLastPacketsProcessed &&
                      stats.mEpochTimeOfLastPacketsProcessed > lastPacketsProcessedThreshold )
                  {

                     // we found one media relay session that processed packets after our thresholds.
                     // This indicates that traffic has been seen recently enough to return success
                     // and stop looking.
                     bTrafficSeen = true;
                     break;
                  }
               }
            }
         }
      }
   }

   if( numOfActiveSession == 0 )
   {
      // this is a special case where no media sessions are active so the fact that
      // no media is seen is actually normal.  Since the point of this method is
      // to detect condition where media traffic isn't flowing when it should be,
      // we return a success code in this condition.
      bTrafficSeen = true;
   }
   return bTrafficSeen;
}

bool DialogTracker::setMediaRelayDirectionMode( const tMediaRelayHandle& relayHandle, MediaDirectionality mediaRelayDirectionMode, EndpointRole endpointRole )
{
   return pOwningSessionContext->setMediaRelayDirectionMode( mHandle, relayHandle, mediaRelayDirectionMode, endpointRole );
}


bool DialogTracker::linkFarEndMediaRelayPortToRequester( const tMediaRelayHandle& relayHandle, const MediaDescriptor* pMediaDescriptor, EndpointRole endpointRoleOfRequester )
{
   return pOwningSessionContext->linkFarEndMediaRelayPortToRequester( mHandle, relayHandle, pMediaDescriptor, endpointRoleOfRequester );
}

bool DialogTracker::getMediaRelayAddressToUseInSdp( UtlString& mediaRelayAddressToUse, EndpointRole endpointRole ) const
{
   return pOwningSessionContext->getMediaRelayAddressToUseInSdp( mediaRelayAddressToUse, endpointRole );
}

bool DialogTracker::patchSdp( SdpBody* pSdpBody, int mediaIndex, int rtpPort, tMediaRelayHandle relayHandle, const UtlString& mediaRelayAddressToUse )
{
   bool bPatchedSuccessfully = false;

   if( pSdpBody && mediaIndex < pSdpBody->getMediaSetCount() )
   {
      // patch connection address
      pSdpBody->modifyMediaAddress( mediaIndex, mediaRelayAddressToUse );

      // patch RTP Port
      pSdpBody->modifyMediaPort( mediaIndex, rtpPort );

      // Remove RTCP attribute in case it was there.  Media Relay RTCP port is guaranteed
      // to be RTP+1
      pSdpBody->removeMediaAttribute( mediaIndex, "rtcp" );

      // add our custom attribute to link SDP to media relay session and remove all previous ones
      pSdpBody->removeMediaAttribute( mediaIndex, NTAP_PROPRIETARY_SDP_ATTRIB );
      char ntapAttribValue[100];
      sprintf( ntapAttribValue, "%s;%d", mSystemIdentificationString.data(), (int)relayHandle );
      pSdpBody->insertMediaAttribute( mediaIndex, NTAP_PROPRIETARY_SDP_ATTRIB,  ntapAttribValue );

      bPatchedSuccessfully = true;
   }
   return bPatchedSuccessfully;
}

void DialogTracker::savePatchedSdpPreview( SipMessage& sipMessage )
{
   if( mpCopyOfPatchedSdpBody )
   {
      delete mpCopyOfPatchedSdpBody;
      mpCopyOfPatchedSdpBody = 0;
   }
   mpCopyOfPatchedSdpBody = const_cast<SdpBody*>( sipMessage.getSdpBody() );
}

void DialogTracker::applyPatchedSdpPreview( SipMessage& sipMessage )
{
   if( mpCopyOfPatchedSdpBody )
   {
      sipMessage.setBody( mpCopyOfPatchedSdpBody->copy() );
   }
}

void DialogTracker::removeUnwantedElements( SipMessage& request )
{
   // if the Contact header contains a +sip.rendering field parameter with a
   // "no" or "unknown" value then this routine will take it out as it can
   // lead to unidirectional media streams being setup and those cause
   // speech path issues with NATs - see XECS-2090 for details.
   UtlString contact;
   for (int contactNumber = 0;
        request.getContactEntry(contactNumber, &contact);
        contactNumber++ )
   {
      Url contactUri( contact );
      UtlString sipRenderingValue;
      if( contactUri.getFieldParameter( "+sip.rendering", sipRenderingValue, 0 ) )
      {
         if( sipRenderingValue.compareTo("\"no\"", UtlString::ignoreCase )      == 0 ||
             sipRenderingValue.compareTo("no", UtlString::ignoreCase )          == 0 ||
             sipRenderingValue.compareTo("\"unknown\"", UtlString::ignoreCase ) == 0 ||
             sipRenderingValue.compareTo("unknown", UtlString::ignoreCase )     == 0 )
         {
            UtlString modifiedContact;
            contactUri.removeFieldParameter( "+sip.rendering" );
            contactUri.toString( modifiedContact );
            request.setContactField( modifiedContact, contactNumber );
         }
      }
   }
   // ... add code to remove other unwanted elements here ...
}

tMediaRelayHandle DialogTracker::getOurMediaRelayHandleEncodedInSdp( const SdpBody* pSdpBody, int mediaIndex ) const
{
   tMediaRelayHandle mediaRelayHandle = INVALID_MEDIA_RELAY_HANDLE;
   UtlString ntapProprietaryAttribValue;

   if( pSdpBody->getMediaAttribute( mediaIndex, NTAP_PROPRIETARY_SDP_ATTRIB, &ntapProprietaryAttribValue ) )
   {
      UtlString ipAddressToken;
      UtlString mediaRelayToken;

      UtlTokenizer tokenizer( ntapProprietaryAttribValue );
      if( tokenizer.next( ipAddressToken, ";" ) && tokenizer.next( mediaRelayToken, ";" ) )
      {
         // we found a proprietary attribute.  Did we put it there?
         if( ipAddressToken == mSystemIdentificationString )
         {
            mediaRelayHandle = atoi( mediaRelayToken.data() );
         }
      }
   }
   return mediaRelayHandle;
}

bool DialogTracker::hasSdpAlreadyBeenPatchedByUs( SipMessage& message, int mediaIndex ) const
{
   bool bResult = false;
   const SdpBody* pSdpBody;

   pSdpBody = message.getSdpBody();
   if( pSdpBody )
   {
      bResult = hasSdpAlreadyBeenPatchedByUs( pSdpBody, mediaIndex );
      delete pSdpBody;
   }
   return bResult;
}


bool DialogTracker::hasSdpAlreadyBeenPatchedByUs( const SdpBody* pSdpBody, int mediaIndex ) const
{
   return getOurMediaRelayHandleEncodedInSdp( pSdpBody, mediaIndex ) != INVALID_MEDIA_RELAY_HANDLE;
}

void DialogTracker::setTransactionDirectionality( TransactionDirectionality directionality )
{
   mCurrentTransactionDirectionality = directionality;
}

TransactionDirectionality DialogTracker::getTransactionDirectionality( void ) const
{
   return mCurrentTransactionDirectionality;
}

void DialogTracker::reportDialogCompleted( void )
{
   // We are notified by the state machine that the dialog has completed.  There is no longer
   // a need for this object.  Notify our owning SessionContext that we are done.
   pOwningSessionContext->reportDialogTrackerReadyForDeletion( mHandle );
}

void DialogTracker::ProcessMediaOffer( SipMessage& message, OfferAnswerPattern offerAnswerPattern )
{
   // check if the INVITE contains an SDP offer
   bool bSdpBodyHasChanged = false;
   SdpBody* pSdpBody = const_cast<SdpBody*>(message.getSdpBody());
   if( pSdpBody )
   {
      size_t numSavedMediaDescriptors     = getNumberOfMediaDescriptors();
      size_t numMediaDescriptorsInSdp     = pSdpBody->getMediaSetCount();

      // establish role of the endpoint that generated the offer we are currently processing.
      EndpointRole thisEndpointRole = EstablishEndpointRole( getTransactionDirectionality(),
                                                             message.isResponse() );

      size_t index;
      for( index = 0; index < numMediaDescriptorsInSdp; index++ )
      {
         bool sdpContainsNewMediaParameters = true;
         MediaDescriptor* pMediaDescriptor;
         if( index < numSavedMediaDescriptors )
         {
            pMediaDescriptor = getModifiableMediaDescriptor( index );
            pMediaDescriptor->setMediaTypeAndDirectionalityData( *pSdpBody, index );
            sdpContainsNewMediaParameters = pMediaDescriptor->setEndpointData( *pSdpBody, index, thisEndpointRole );
         }
         else
         {
            pMediaDescriptor = new MediaDescriptor( *pSdpBody, index, thisEndpointRole );
            appendMediaDescriptor( pMediaDescriptor );
         }

         if( pMediaDescriptor->getEndpoint( thisEndpointRole ).getRtpPort() != 0 )
         {
            // media stream is not disabled
            tMediaRelayHandle tentativeRelayHandle;
            int ourRelayRtpPort = 0;

            bool bDoPatchSdp = false;
            bool bSdpHasAlreadyBeenPatchedByUs = false;

            // Check if the SDP we are receiving has already been patched with pre-existing
            // media relay session from this system
            tMediaRelayHandle preExistingMediaRelayHandle = getOurMediaRelayHandleEncodedInSdp( pSdpBody, index );


            // Recall the parameters of the media relay session we are re-using here and create a clone
            // for it.  This is done because the ownership of the original relay session remains with
            // the dialog that started the 3PCC call.
            int originalCallerPort = PORT_NONE; // will store RTP port allocated for caller by pre-existing media relay session
            int originalCalleePort = PORT_NONE; // will store RTP port allocated for callee by pre-existing media relay session

            if( preExistingMediaRelayHandle != INVALID_MEDIA_RELAY_HANDLE &&
                preExistingMediaRelayHandle != pMediaDescriptor->getCurrentMediaRelayHandle() &&
                ( originalCallerPort = pOwningSessionContext->getRtpRelayPortForMediaRelaySession( preExistingMediaRelayHandle, CALLER ) ) != PORT_NONE &&
                ( originalCalleePort = pOwningSessionContext->getRtpRelayPortForMediaRelaySession( preExistingMediaRelayHandle, CALLEE ) ) != PORT_NONE
              )
            {
               setMediaRelayRequiredFlag(); // a media relay is going to be needed after all, not because of the
                                            // location of the endpoints but rather because we are potentially
                                            // dealing with a 3PCC call.

               bSdpHasAlreadyBeenPatchedByUs = true;
               bDoPatchSdp = true;
               // the media description we are analyzing has already been patched by us in the context of another
               // session.  This sort of scenario can happen in 3PCC call scenarios such as the one used to provide
               // music-on-hold.  In such a case, do not allocate a new media relay session and reuse the one we
               // just learned about.
               OsSysLog::add(FAC_NAT,PRI_INFO,"DialogTracker[%s]::ProcessMediaOffer:  Possible 3PCC scenario detected in offer while in state '%s'; media relay handle=%d",
                     name(), GetCurrentState()->name(), (int)preExistingMediaRelayHandle );

               // verify which port this SDP is using
               bool bDoSwapCallerAndCallee;
               pSdpBody->getMediaPort( index, &ourRelayRtpPort );
               if( ourRelayRtpPort == originalCallerPort )
               {
                  bDoSwapCallerAndCallee = ( thisEndpointRole == CALLER ? false : true );
                  tentativeRelayHandle = cloneMediaRelaySession( preExistingMediaRelayHandle, bDoSwapCallerAndCallee );
               }
               else if( ourRelayRtpPort == originalCalleePort )
               {
                  bDoSwapCallerAndCallee = ( thisEndpointRole == CALLEE ? false : true );
                  tentativeRelayHandle = cloneMediaRelaySession( preExistingMediaRelayHandle, bDoSwapCallerAndCallee );
               }
               else
               {
                  tentativeRelayHandle = pMediaDescriptor->getCurrentMediaRelayHandle();

                  OsSysLog::add(FAC_NAT,PRI_WARNING,"DialogTracker[%s]::ProcessMediaOffer:  3PCC is not using any of the media relay ports.  Handle=%d; port=%d",
                        name(), (int)preExistingMediaRelayHandle, ourRelayRtpPort );
               }
            }
            else
            {
               if( getMediaRelayRequiredFlag() )
               {
                  int callerRelayRtpPort;
                  int calleeRelayRtpPort;

                  if( allocateMediaRelaySession( tentativeRelayHandle, callerRelayRtpPort, calleeRelayRtpPort ) )
                  {
                     ourRelayRtpPort = ( thisEndpointRole == CALLER ? callerRelayRtpPort : calleeRelayRtpPort );
                     bDoPatchSdp = true;
                  }
                  else
                  {
                     OsSysLog::add(FAC_NAT,PRI_ERR,"DialogTracker[%s]::ProcessMediaOffer:  Failed to allocateMediaRelaySession while in state '%s'",
                           name(), GetCurrentState()->name() );
                     bDoPatchSdp = false;
                  }
               }
            }

            if( bDoPatchSdp )
            {
               MediaDirectionality mediaRelayDirectionMode = SEND_RECV;

               if( pMediaDescriptor->getDirectionality() == SEND_ONLY )
               {
                  // Offer is trying to establish a 'sendonly' stream which means that the media will
                  // only flow from the offerer to the answerer.  If the media packets from the media
                  // relay to the SDP answerer happen to ingress a NAT, the packets will likely be dropped
                  // by it due to lack of data egressing the NAT.  Make the following assumption: if the
                  // SDP receiver and the media relay are not in the same subnet then a NAT may be in the way.
                  // In such cases, the SDP is converted to a 'sendrecv' to cause the SDP receiver to emit packets.
                  // When the SDP answer counterpart is processed, the directionality will be returned to
                  // 'recvonly' (unless the stream is inactive) to keep the offerer happy.  Furthermore, instruct
                  // the media relay to shunt the packets from the answerer-to-offerer direction so not to subject
                  // the offerer to an unexpected incoming media packet flow.
                  pSdpBody->removeMediaAttribute( index, "sendonly" );
                  pSdpBody->insertMediaAttribute( index, "sendrecv" );
                  mediaRelayDirectionMode = SEND_ONLY;
                  pMediaDescriptor->setDirectionalityOverride( SEND_RECV );
               }
               else if( pMediaDescriptor->getDirectionality() == RECV_ONLY )
               {
                  // Offer is trying to establish a 'recvonly' stream which means that the media will
                  // only flow from the answerer to the offerer.  The mirror image of the logic used
                  // to handle pMediaDescriptor->getDirectionality() == SEND_ONLY is applied here.
                  // Refer to comment block above for more information.
                  pSdpBody->removeMediaAttribute( index, "recvonly" );
                  pSdpBody->insertMediaAttribute( index, "sendrecv" );
                  mediaRelayDirectionMode = RECV_ONLY;
                  pMediaDescriptor->setDirectionalityOverride( SEND_RECV );
               }

               if( bSdpHasAlreadyBeenPatchedByUs == false )
               {
                  // In 3PCC call scenarios notably, it is possible for an SDP offer to
                  // be seen and processed by two different DialogTracker instances.
                  // We should not attempt to link the far-end media port to the
                  // media endpoint that sent that offer if we see that the SDP has
                  // already been patched by us.  The rationale behind that logic
                  // is that in the case of 3PCC, the actual location of the media
                  // endpoint to link the far-end media port to is only known with
                  // accuracy when the SDP is seen the first time as it comes from
                  // the actual entity that produced it.  So, the net out is that
                  // we only link the far-end media endpoint to the media endpoint
                  // associated with the SDP sender when it is seen for the first
                  // time and is ommited every subsequent time that it is seen.
                  linkFarEndMediaRelayPortToRequester( tentativeRelayHandle, pMediaDescriptor, thisEndpointRole );
               }

               UtlString mediaRelayAddressToUse;
               getMediaRelayAddressToUseInSdp( mediaRelayAddressToUse, thisEndpointRole );

               setMediaRelayDirectionMode( tentativeRelayHandle, mediaRelayDirectionMode, thisEndpointRole );
               patchSdp( pSdpBody, index, ourRelayRtpPort, tentativeRelayHandle, mediaRelayAddressToUse );

               if( offerAnswerPattern == INITIAL_OFFER_ANSWER )
               {
                  pMediaDescriptor->setTentativeInitialMediaRelayHandle( tentativeRelayHandle );
               }
               else if( offerAnswerPattern == NON_INITIAL_OFFER_ANSWER )
               {
                  pMediaDescriptor->setTentativeNonInitialMediaRelayHandle( tentativeRelayHandle );
               }
               else
               {
                  OsSysLog::add(FAC_NAT,PRI_WARNING,"DialogTracker[%s]::ProcessMediaOffer:  Received unknown Offer/Answer pattern type:%d while in state '%s'",
                        name(), offerAnswerPattern, GetCurrentState()->name() );

               }
               bSdpBodyHasChanged = true;
            }
         }
      }

      if( bSdpBodyHasChanged == true )
      {
         // Save the SDP modifications and identification information of the message so that we can
         // re-apply the SDP modifications if the message gets retransmitted
         if( message.isResponse() )
         {
            mResponseRetransmissionDescriptor.setMessageToTrackRetransmissionsOf( message, *pSdpBody );
         }
         else
         {
            mRequestRetransmissionDescriptor.setMessageToTrackRetransmissionsOf( message, *pSdpBody );
         }

         // SDP body got changed - apply the new SDP body to the message.
         message.setBody( pSdpBody );
      }
      else
      {
         // we did not end up the SDP body that got allocated to us for processing - throw it away
         delete pSdpBody;
      }
   }
   else
   {
      OsSysLog::add(FAC_NAT,PRI_ERR,"DialogTracker[%s]::ProcessMediaOffer:  Failed to obtain SDP body while in state '%s'",
            name(), GetCurrentState()->name() );
   }
}

void DialogTracker::ProcessMediaAnswer( SipMessage& message, OfferAnswerPattern offerAnswerPattern )
{
   // check if the message contains an SDP answer as it should
   SdpBody* pSdpBody = const_cast<SdpBody*>(message.getSdpBody());
   if( pSdpBody )
   {
      if( getMediaRelayRequiredFlag() )
      {
         // establish role of the endpoint that generated the offer we are currently processing.
         EndpointRole endpointRole = EstablishEndpointRole( getTransactionDirectionality(),
                                                            message.isResponse() );

         size_t numSavedMediaDescriptors = getNumberOfMediaDescriptors();
         size_t numMediaDescriptorsInSdp = pSdpBody->getMediaSetCount();
         size_t index;

         // go through the saved media descriptors accumulated during the
         // offer processing and see what the SDP answer was for each one.
         for( index = 0; index < numSavedMediaDescriptors; index++ )
         {
            int mediaPort;

            MediaDescriptor* pMediaDescriptor;
            pMediaDescriptor = getModifiableMediaDescriptor( index );
            pMediaDescriptor->setEndpointData( *pSdpBody, index, endpointRole );

            tMediaRelayHandle tentativeMediaRelayHandle = INVALID_MEDIA_RELAY_HANDLE;
            if( offerAnswerPattern == INITIAL_OFFER_ANSWER )
            {
               tentativeMediaRelayHandle = pMediaDescriptor->getTentativeInitialMediaRelayHandle();
            }
            else if( offerAnswerPattern == NON_INITIAL_OFFER_ANSWER )
            {
               tentativeMediaRelayHandle = pMediaDescriptor->getTentativeNonInitialMediaRelayHandle();
            }
            else
            {
               OsSysLog::add(FAC_NAT,PRI_WARNING,"DialogTracker[%s]::ProcessMediaAnswer:  Received unknown Offer/Answer pattern type:%d while in state '%s'",
                     name(), offerAnswerPattern, GetCurrentState()->name() );
            }

            if( index < numMediaDescriptorsInSdp )
            {
               pSdpBody->getMediaPort( index, &mediaPort );
               if( mediaPort != 0 )
               {
                  // the answerer has accepted the media session.  Adjust the SDP accordingly
                  // If we do not have a valid tentative media relay handle, it means that this
                  // media session does not need a media relay, there is therefore no need
                  // to go any further in our processing of this answer media description.
                  if( tentativeMediaRelayHandle != INVALID_MEDIA_RELAY_HANDLE )
                  {
                     // adjust directionality of the call to allow for NAT traversal...
                     UtlString directionalityToUseInThisSdpString("inactive");
                     UtlString currentDirectionalityString("inactive");
                     MediaDirectionality directionalityFromOfferSdp;
                     MediaDirectionality directionalityFromAnswerSdp;
                     directionalityFromOfferSdp = pMediaDescriptor->getDirectionality();
                     directionalityFromAnswerSdp =
                              MediaDescriptor::sdpDirectionalityAttributeToMediaDirectionalityValue( *pSdpBody, index );

                     // check if we have tempered with the directionality of the call...
                     if( pMediaDescriptor->getDirectionalityOverride() != NOT_A_DIRECTION )
                     {
                        pMediaDescriptor->setDirectionalityOverride( NOT_A_DIRECTION );
                        // the offer processing changed the directionality of the call
                        // to make is suitable for NAT traversal.  Restore the original
                        // directionality before passing on the SDP answer to the offerer

                        // establish the directionality that should be present in the offer based
                        // on the answer and offer values.

                        switch( directionalityFromAnswerSdp )
                        {
                        case SEND_RECV:
                           currentDirectionalityString = "sendrecv";
                           if( directionalityFromOfferSdp ==  SEND_RECV )
                           {
                              directionalityToUseInThisSdpString = "sendrecv";
                           }
                           else if( directionalityFromOfferSdp == SEND_ONLY )
                           {
                              directionalityToUseInThisSdpString = "recvonly";
                           }
                           else if( directionalityFromOfferSdp == RECV_ONLY )
                           {
                              directionalityToUseInThisSdpString = "sendonly";
                           }
                           break;
                        case SEND_ONLY:
                           currentDirectionalityString = "sendonly";
                           if( directionalityFromOfferSdp == RECV_ONLY || directionalityFromOfferSdp == SEND_RECV )
                           {
                              directionalityToUseInThisSdpString = "sendonly";
                           }
                           break;
                        case RECV_ONLY:
                           currentDirectionalityString = "recvonly";
                           if( directionalityFromOfferSdp == SEND_ONLY || directionalityFromOfferSdp == SEND_RECV )
                           {
                              directionalityToUseInThisSdpString = "recvonly";
                           }
                           break;
                        case INACTIVE:
                        default:
                           currentDirectionalityString = "inactive";
                           break;
                        }
                     }
                     else
                     {
                        // we have not tempered with the directionality of the call yet however we may still need to
                        // do some directionality adjustments to handle cases where the SDP offer had 'sendrecv'
                        // but the SDP answer has either 'sendonly' or 'recvonly'.  Such scenarios generate
                        // unidirectional media streams which are not good for NAT traversal - instead we need
                        // both ends to send media traffic so that firewall holes get punched.  To work around
                        // such cases, change the directionality of the SDP answer to 'sendrecv' and instruct
                        // the media relay to shunt the unwanted direction.
                        if( directionalityFromOfferSdp == SEND_RECV )
                        {
                           switch( directionalityFromAnswerSdp )
                           {
                           case SEND_ONLY:
                              currentDirectionalityString = "sendonly";
                              directionalityToUseInThisSdpString = "sendrecv";
                              setMediaRelayDirectionMode( tentativeMediaRelayHandle, SEND_ONLY, endpointRole );
                              break;
                           case RECV_ONLY:
                              currentDirectionalityString = "recvonly";
                              directionalityToUseInThisSdpString = "sendrecv";
                              setMediaRelayDirectionMode( tentativeMediaRelayHandle, RECV_ONLY, endpointRole );
                              break;
                           default:
                              // do nothing
                              break;
                           }
                        }
                     }
                     if( directionalityToUseInThisSdpString != currentDirectionalityString )
                     {
                        pSdpBody->removeMediaAttribute( index, currentDirectionalityString );
                        pSdpBody->insertMediaAttribute( index, directionalityToUseInThisSdpString );
                     }

                     int rtpPort = pOwningSessionContext->getRtpRelayPortForMediaRelaySession( tentativeMediaRelayHandle, endpointRole );
                     if( rtpPort != PORT_NONE )
                     {
                        if( hasSdpAlreadyBeenPatchedByUs( pSdpBody, index ) == false )
                        {
                           // In 3PCC call scenarios notably, it is possible for an SDP answer to
                           // be seen and processed by two different DialogTracker instances.
                           // We should not attempt to link the far-end media port to the
                           // media endpoint that sent that answer if we see that the SDP has
                           // already been patched by us.  The rationale behind that logic
                           // is that in the case of 3PCC, the actual location of the media
                           // endpoint to link the far-end media port to is only known with
                           // accuracy when the SDP is seen the first time as it comes from
                           // the actual entity that produced it.  So, the net out is that
                           // we only link the far-end media endpoint to the media endpoint
                           // associated with the SDP sender when it is seen for the first
                           // time and is ommited every subsequent time that it is seen.
                           linkFarEndMediaRelayPortToRequester( tentativeMediaRelayHandle, pMediaDescriptor, endpointRole );
                        }

                        UtlString mediaRelayAddressToUse;
                        getMediaRelayAddressToUseInSdp( mediaRelayAddressToUse, endpointRole );
                        patchSdp( pSdpBody, index, rtpPort, tentativeMediaRelayHandle, mediaRelayAddressToUse );
                     }
                     else
                     {
                        OsSysLog::add(FAC_NAT,PRI_ERR, "DialogTracker[%s]::ProcessMediaAnswer:  Failed to getRtpRelayPortForMediaRelaySession for "
                                                      "Media Relay Handle %d", name(), (int)tentativeMediaRelayHandle );
                     }
                  }
               }
               else
               {
                  // the answerer has refused the media session - de-allocate the media relay session
                  // we had tentatively allocated to relay its media.
                  deallocateMediaRelaySession( tentativeMediaRelayHandle );
                  if( offerAnswerPattern == INITIAL_OFFER_ANSWER )
                  {
                     pMediaDescriptor->clearTentativeInitialMediaRelayHandle();
                  }
                  else if( offerAnswerPattern == NON_INITIAL_OFFER_ANSWER )
                  {
                     pMediaDescriptor->clearTentativeNonInitialMediaRelayHandle();
                  }
               }
            }
         }
         // Save the SDP modifications and identification information of the message so that we can
         // re-apply the SDP modifications if the message gets retransmitted
         if( message.isResponse() )
         {
            mResponseRetransmissionDescriptor.setMessageToTrackRetransmissionsOf( message, *pSdpBody );
         }
         else
         {
            mRequestRetransmissionDescriptor.setMessageToTrackRetransmissionsOf( message, *pSdpBody );
         }

         // apply the new SDP body to the message.
         message.setBody( pSdpBody );
      }
   }
   else
   {
      // response does not contain an SDP answer as we expected...  Error!
      OsSysLog::add(FAC_NAT,PRI_ERR,"DialogTracker[%s]::ProcessMediaAnswer:  Failed to obtain SDP body while in state '%s'",
                                   name(), GetCurrentState()->name() );
   }
}

EndpointRole DialogTracker::EstablishEndpointRole( TransactionDirectionality directionality, bool bMessageIsResponse ) const
{
   if( directionality == DIR_CALLER_TO_CALLEE )
   {
      return bMessageIsResponse ? CALLEE : CALLER;
   }
   else
   {
      return bMessageIsResponse ? CALLER : CALLEE;
   }
}

void DialogTracker::markRequestAsHandledByUs( SipMessage& request )
{
   if( !isRequestAlreadyHandledByUs( request ) )
   {
      request.setHeaderValue( SIP_SIPX_NAT_HANDLED, mSystemIdentificationString, 0 );
   }
}

bool DialogTracker::isRequestAlreadyHandledByUs( const SipMessage& request ) const
{
   bool rc = false;
   const char *pSystemIdentificationStringInMessage = 0;

   if( ( pSystemIdentificationStringInMessage = request.getHeaderValue( 0, SIP_SIPX_NAT_HANDLED ) ) )
   {
      rc = ( mSystemIdentificationString.compareTo( pSystemIdentificationStringInMessage ) == 0 );
   }
   return rc;
}

bool DialogTracker::isRequestAlreadyHandledByOther( const SipMessage& request ) const
{
   bool rc = false;
   const char *pSystemIdentificationStringInMessage = 0;

   if( ( pSystemIdentificationStringInMessage = request.getHeaderValue( 0, SIP_SIPX_NAT_HANDLED ) ) )
   {
      rc = ( mSystemIdentificationString.compareTo( pSystemIdentificationStringInMessage ) != 0 );
   }
   return rc;
}

bool DialogTracker::isRequestAlreadyHandledByAnyone( const SipMessage& request ) const
{
   bool rc = false;
   const char *pSystemIdentificationStringInMessage = 0;

   if( ( pSystemIdentificationStringInMessage = request.getHeaderValue( 0, SIP_SIPX_NAT_HANDLED ) ) )
   {
      rc = true;
   }
   return rc;
}

bool DialogTracker::getNonIntialOfferAnswerExchangeDoneFlag( void ) const
{
   return mbNonIntialOfferAnswerExchangeDoneFlag;
}

void DialogTracker::modifyNonIntialOfferAnswerExchangeDoneFlag( bool newValue )
{
   mbNonIntialOfferAnswerExchangeDoneFlag = newValue;
}

bool DialogTracker::isARetransmittedRequest( const SipMessage& request )
{
   return mRequestRetransmissionDescriptor == request;
}

bool DialogTracker::isARetransmittedResponse( const SipMessage& response )
{
   return mResponseRetransmissionDescriptor == response;
}

void DialogTracker::restoreSdpBodyOfRetransmittedRequest( SipMessage& request )
{
   SdpBody* pRecalledSdp;

   pRecalledSdp = mRequestRetransmissionDescriptor.getCopyOfSdpBody();
   if( pRecalledSdp )
   {
      request.setBody( pRecalledSdp );
   }
}

void DialogTracker::restoreSdpBodyOfRetransmittedResponse( SipMessage& response )
{
   SdpBody* pRecalledSdp;

   pRecalledSdp = mResponseRetransmissionDescriptor.getCopyOfSdpBody();
   if( pRecalledSdp )
   {
      response.setBody( pRecalledSdp );
   }
}

UtlContainableType DialogTracker::getContainableType( void ) const
{
   return DialogTracker::TYPE;
}

unsigned DialogTracker::hash() const
{
   return directHash();
}

int DialogTracker::compareTo(UtlContainable const *rhsContainable ) const
{
   int result = -1;
   if ( rhsContainable->isInstanceOf( DialogTracker::TYPE ) )
   {
      result = mHandle.compareTo( static_cast<const DialogTracker*>(rhsContainable)->mHandle );
   }
   return result;
}

void DialogTracker::initializeStatePointers( void )
{
   static DialogTrackerStateStruct states;

   pWaitingForInvite                           = &states.waitingForInvite;
   pWaitingForAckForInvite                     = &states.waitingForAckForInvite;
   pTimeBoundState                             = &states.timeBoundState;
   pNegotiating                                = &states.negotiating;
   pWaitingForMediaOffer                       = &states.waitingForMediaOffer;
   pWaitingFor200OkWithMediaOffer              = &states.waitingFor200OkWithMediaOffer;
   pWaitingForMediaAnswer                      = &states.waitingForMediaAnswer;
   pProcessingPrack                            = &states.processingPrack;
   pWaitingForPrack                            = &states.waitingForPrack;
   pWaitingForAckWithAnswerForInvite           = &states.waitingForAckWithAnswerForInvite;
   pWaitingForPrackWithMediaAnswer             = &states.waitingForPrackWithMediaAnswer;
   pWaitingFor200OkForSlowStartPrack           = &states.waitingFor200OkForSlowStartPrack;
   pWaitingFor200OkForPrack                    = &states.waitingFor200OkForPrack;
   pWaitingFor200OkWithAnswerForPrack          = &states.waitingFor200OkWithAnswerForPrack;
   pProcessingPrackWaitingForAckforInvite      = &states.processingPrackWaitingForAckforInvite;
   pMoribund                                   = &states.moribund;
}

DialogTracker::RetransmissionDescriptor::RetransmissionDescriptor() :
   mpSavedPatchedSdp( 0 )
{}

void DialogTracker::RetransmissionDescriptor::setMessageToTrackRetransmissionsOf( const SipMessage& messageToTrackRetransmissionsOf,
                                                                                   const SdpBody& patchedSdpBodyToCopy )
{
   delete mpSavedPatchedSdp;
   messageToTrackRetransmissionsOf.getCSeqField( &mSequenceNumber, &mMethod );
   mpSavedPatchedSdp = patchedSdpBodyToCopy.copy();
}

DialogTracker::RetransmissionDescriptor::~RetransmissionDescriptor()
{
   delete mpSavedPatchedSdp;
}

SdpBody* DialogTracker::RetransmissionDescriptor::getCopyOfSdpBody( void ) const
{
   SdpBody *pCopy = 0;
   if( mpSavedPatchedSdp )
   {
      pCopy = mpSavedPatchedSdp->copy();
   }
   return pCopy;
}

bool DialogTracker::RequestRetransmissionDescriptor::operator==( const SipMessage& request ) const
{
   bool bAreEqual = false;
   int       rhsSeqNum;
   UtlString rhsMethod;

   if( request.getCSeqField( &rhsSeqNum, &rhsMethod ) )
   {
      if( mMethod == rhsMethod && mSequenceNumber == rhsSeqNum )
      {
         bAreEqual = true;
      }
   }
   return bAreEqual;
}

bool DialogTracker::RequestRetransmissionDescriptor::operator!=( const SipMessage& request ) const
{
   return !( *this == request );
}

DialogTracker::RequestRetransmissionDescriptor&
DialogTracker::RequestRetransmissionDescriptor::operator= ( const RequestRetransmissionDescriptor& rhs )
{
   if( this != &rhs )
   {
      mpSavedPatchedSdp = ( rhs.mpSavedPatchedSdp ? rhs.mpSavedPatchedSdp->copy() : 0 );
      mMethod           = rhs.mMethod;
      mSequenceNumber   = rhs.mSequenceNumber;
   }
   return *this;
}

void DialogTracker::ResponseRetransmissionDescriptor::setMessageToTrackRetransmissionsOf( const SipMessage& messageToTrackRetransmissionsOf,
                                                                                      const SdpBody& patchedSdpBodyToCopy )
{
   mResponseCode = messageToTrackRetransmissionsOf.getResponseStatusCode();
   RetransmissionDescriptor::setMessageToTrackRetransmissionsOf( messageToTrackRetransmissionsOf, patchedSdpBodyToCopy );
}

bool DialogTracker::ResponseRetransmissionDescriptor::operator==( const SipMessage& response ) const
{
   bool bAreEqual = false;
   int       rhsSeqNum;
   UtlString rhsMethod;
   int       responseCode;

   if( response.isResponse() )
   {
      responseCode = response.getResponseStatusCode();
      if( response.getCSeqField( &rhsSeqNum, &rhsMethod ) )
      {
         if( mMethod == rhsMethod && mSequenceNumber == rhsSeqNum && mResponseCode == responseCode )
         {
            bAreEqual = true;
         }
      }
   }
   return bAreEqual;
}

bool DialogTracker::ResponseRetransmissionDescriptor::operator!=( const SipMessage& request ) const
{
   return !( *this == request );
}

DialogTracker::ResponseRetransmissionDescriptor&
DialogTracker::ResponseRetransmissionDescriptor::operator= ( const ResponseRetransmissionDescriptor& rhs )
{
   if( this != &rhs )
   {
      mpSavedPatchedSdp = ( rhs.mpSavedPatchedSdp ? rhs.mpSavedPatchedSdp->copy() : 0 );
      mMethod           = rhs.mMethod;
      mSequenceNumber   = rhs.mSequenceNumber;
      mResponseCode     = rhs.mResponseCode;
   }
   return *this;
}
