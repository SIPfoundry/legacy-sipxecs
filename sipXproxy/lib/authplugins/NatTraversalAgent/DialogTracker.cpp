//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include "os/OsLogger.h"

// APPLICATION INCLUDES
#include "DialogTracker.h"
#include "SessionContext.h"
#include "net/SdpBody.h"
#include "net/SipMessage.h"
#include "utl/UtlFsm.h"
#include "utl/UtlTokenizer.h"
#include "NatTraversalRules.h"

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
                              const UtlString& newHandle,
                              const NatTraversalRules* pNatTraversalRules ) :
  mbInitialDialogEstablished(false),
  mpCopyOfPatchedSdpBody( 0 ),
  mpNatTraversalRules( pNatTraversalRules )
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
   Os::Logger::instance().log(FAC_NAT, PRI_DEBUG, "+DialogTracker tracker %p created; Handle=%s+",
                                       this,
                                       mHandle.data() );
}

DialogTracker::DialogTracker( const UtlString& handle,
                              const UtlString& systemIdentificationString,
                              SessionContextInterfaceForDialogTracker* pOwningSessionContext,
                              const NatTraversalRules* pNatTraversalRules ) :
   mHandle( handle ),
   mSystemIdentificationString( systemIdentificationString ),
   mbMediaRelayRequired( false ),
   mbDialogEstablished( false ),
   mbInitialDialogEstablished( false ),
   mTimerTickCounter( 0 ),
   pOwningSessionContext( pOwningSessionContext ),
   mbNonIntialOfferAnswerExchangeDoneFlag( false ),
   mpNatTraversalRules(pNatTraversalRules)
{
   // Init state pointers
   initializeStatePointers();

   // kickstart the state mahine
   const DialogTrackerState* pState = pWaitingForInvite;
   StateAlg::StartStateMachine( *this, pState );

   Os::Logger::instance().log(FAC_NAT, PRI_DEBUG, "+DialogTracker tracker %p created; Handle=%s+",
                                       this,
                                       mHandle.data() );
}

DialogTracker::~DialogTracker()
{
   vector<MediaDescriptor*>::iterator pos;
   for( pos = mMediaDescriptors.begin(); pos != mMediaDescriptors.end(); ++pos )
   {
      delete *pos;
   }
   Os::Logger::instance().log(FAC_NAT, PRI_DEBUG, "-DialogTracker tracker %p deleted; Handle=%s-",
                                       this,
                                       mHandle.data() );
}

bool DialogTracker::handleRequest( SipMessage& message,
                                    const char* address,
                                    int port,
                                    bool bFromCallerToCallee,
                                    bool* reevaluateDestination )
{
   bool bTrackRequestResponse = true;

   TransactionDirectionality directionality = ( bFromCallerToCallee ? DIR_CALLER_TO_CALLEE : DIR_CALLEE_TO_CALLER );
   UtlString method;
   message.getRequestMethod(&method);

   // remove all the elements in the message that may impair NAT traversal
   removeUnwantedElements( message );

   Os::Logger::instance().log(FAC_NAT, PRI_DEBUG, "DialogTracker[%s]::handleRequest: received request: %s; Caller->Callee?: %d",
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
      Os::Logger::instance().log(FAC_NAT, PRI_DEBUG, "DialogTracker[%s]::DialogTracker: received unhandled request: %s",
                                        mHandle.data(), method.data() );
   }

   if (reevaluateDestination &&
       mbMediaRelayRequired &&
       method.compareTo(SIP_INVITE_METHOD) == 0 &&
       !message.getHeaderValue( 0, SIP_SIPX_NTAP_CONTACT))
   {
     *reevaluateDestination = true;
     UtlString routeUri = "sip:" + mpNatTraversalRules->getMediaRelayPublicAddress() + ":";
     routeUri.appendNumber(mpNatTraversalRules->getMediaRelayPort());
     routeUri += ";lr";

     message.addRouteUri( routeUri.data() );

     UtlString tmpString;
     message.getContactEntry( 0, &tmpString );
     message.setHeaderValue(SIP_SIPX_NTAP_CONTACT, tmpString );

     UtlString msgBytes;
     ssize_t msgLen;
     message.getBytes(&msgBytes, &msgLen);
     Os::Logger::instance().log(FAC_NAT, PRI_DEBUG, "DialogTracker::handleRequest: NTAP route to sems \n%s", msgBytes.data() );
   }

   return bTrackRequestResponse;
}

void DialogTracker::handleResponse( SipMessage& message, const char* address, int port )
{
   int responseCode = message.getResponseStatusCode();

   // remove all the elements in the message that may impair NAT traversal
   removeUnwantedElements( message );

   Os::Logger::instance().log(FAC_NAT, PRI_DEBUG, "DialogTracker[%s]::handleResponse: received response: %d",
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

bool DialogTracker::doesEndpointsLocationImposeMediaRelay( const SipMessage& request ) const
{
  return pOwningSessionContext->doesEndpointsLocationImposeMediaRelay(request);
}

bool DialogTracker::wasMediaTrafficSeenInLastNSeconds( unsigned long numberOfSeconds )
{
   bool bTrafficSeen = false;
   ssize_t numOfActiveSession = 0;

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

bool DialogTracker::getMediaRelayAddressToUseInSdp( UtlString& mediaRelayAddressToUse, EndpointRole endpointRole ) const
{
   return pOwningSessionContext->getMediaRelayAddressToUseInSdp( mediaRelayAddressToUse, endpointRole );
}

bool DialogTracker::patchSdp( SdpBody* pSdpBody,
                              int mediaIndex,
                              const UtlString& mediaRelayAddressToUse )
{
   bool bPatchedSuccessfully = false;

   //
   // patch the session level address if one is present
   //
   pSdpBody->modifySessionAddress( mediaRelayAddressToUse );

   if( pSdpBody && mediaIndex < pSdpBody->getMediaSetCount() )
   {
      // patch connection address
      pSdpBody->modifyMediaAddress( mediaIndex, mediaRelayAddressToUse );


      // Remove RTCP attribute in case it was there.  Media Relay RTCP port is guaranteed
      // to be RTP+1
      pSdpBody->removeMediaAttribute( mediaIndex, "rtcp" );

      // add our custom attribute to link SDP to media relay session and remove all previous ones
      pSdpBody->removeMediaAttribute( mediaIndex, NTAP_PROPRIETARY_SDP_ATTRIB );
      char ntapAttribValue[100];

      sprintf( ntapAttribValue, "%s;%d", mSystemIdentificationString.data(), (int)NTAP_SDP_ATTRIB_MAGIC_NUMBER );
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

int DialogTracker::getOurSdpAttribMagicNumberEncodedInSdp( const SdpBody* pSdpBody, int mediaIndex ) const
{
   int sdpAttribMagicNumber = 0;
   UtlString ntapProprietaryAttribValue;

   if( pSdpBody->getMediaAttribute( mediaIndex, NTAP_PROPRIETARY_SDP_ATTRIB, &ntapProprietaryAttribValue ) )
   {
      UtlString ipAddressToken;
      UtlString sdpAttribMagicNumberToken;

      UtlTokenizer tokenizer( ntapProprietaryAttribValue );
      if( tokenizer.next( ipAddressToken, ";" ) && tokenizer.next( sdpAttribMagicNumberToken, ";" ) )
      {
         // we found a proprietary attribute.  Did we put it there?
         if( ipAddressToken == mSystemIdentificationString )
         {
           sdpAttribMagicNumber = atoi( sdpAttribMagicNumberToken.data() );
         }
      }
   }
   return sdpAttribMagicNumber;
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
   return getOurSdpAttribMagicNumberEncodedInSdp( pSdpBody, mediaIndex ) == NTAP_SDP_ATTRIB_MAGIC_NUMBER;
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
         MediaDescriptor* pMediaDescriptor;
         if( index < numSavedMediaDescriptors )
         {
            pMediaDescriptor = getModifiableMediaDescriptor( index );
            pMediaDescriptor->setMediaTypeAndDirectionalityData( *pSdpBody, index );
            pMediaDescriptor->setEndpointData( *pSdpBody, index, thisEndpointRole );
         }
         else
         {
            pMediaDescriptor = new MediaDescriptor( *pSdpBody, index, thisEndpointRole );
            appendMediaDescriptor( pMediaDescriptor );
         }

         if( pMediaDescriptor->getEndpoint( thisEndpointRole ).getRtpPort() != 0 )
         {
            // media stream is not disabled

            bool bDoPatchSdp = false;

            if( getMediaRelayRequiredFlag() )
            {
               bDoPatchSdp = true;
            }

            if( bDoPatchSdp )
            {
               if (!message.getHeaderValue( 0, SIP_SIPX_NTAP_ADDRESS ))
               {
                  UtlString mediaRelayAddressToUse;
                  getMediaRelayAddressToUseInSdp( mediaRelayAddressToUse, thisEndpointRole );
                  message.setHeaderValue( SIP_SIPX_NTAP_ADDRESS, mediaRelayAddressToUse );
               }
               else
               {
                  UtlString mediaRelayAddressToUse = message.getHeaderValue( 0, SIP_SIPX_NTAP_ADDRESS );
                  patchSdp( pSdpBody, index, mediaRelayAddressToUse );

                  bSdpBodyHasChanged = true;
               }
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
      Os::Logger::instance().log(FAC_NAT,PRI_ERR,"DialogTracker[%s]::ProcessMediaOffer:  Failed to obtain SDP body while in state '%s'",
            name(), GetCurrentState()->name() );
   }
}

void DialogTracker::ProcessMediaAnswer( SipMessage& message, OfferAnswerPattern offerAnswerPattern )
{
   // check if the message contains an SDP answer as it should
   bool bSdpBodyHasChanged = false;
   bool bShouldDeleteSdpBody = true;
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


            if( index < numMediaDescriptorsInSdp )
            {
               pSdpBody->getMediaPort( index, &mediaPort );
               if( mediaPort != 0 )
               {
                  if (!message.getHeaderValue( 0, SIP_SIPX_NTAP_ADDRESS ))
                  {
                     UtlString mediaRelayAddressToUse;
                     getMediaRelayAddressToUseInSdp( mediaRelayAddressToUse, endpointRole );
                     message.setHeaderValue( SIP_SIPX_NTAP_ADDRESS, mediaRelayAddressToUse );
                  }
                  else
                  {
                     UtlString mediaRelayAddressToUse = message.getHeaderValue( 0, SIP_SIPX_NTAP_ADDRESS );
                     patchSdp( pSdpBody, index, mediaRelayAddressToUse );

                     bSdpBodyHasChanged = true;
                  }
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

           // apply the new SDP body to the message.
           message.setBody( pSdpBody );
           bShouldDeleteSdpBody = false;
         }
      }

      if (bShouldDeleteSdpBody)
      {
    	  //not needed above so it can be safely deleted
    	  delete pSdpBody;
      }
   }
   else
   {
      // response does not contain an SDP answer as we expected...  Error!
      Os::Logger::instance().log(FAC_NAT,PRI_ERR,"DialogTracker[%s]::ProcessMediaAnswer:  Failed to obtain SDP body while in state '%s'",
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
   mpSavedPatchedSdp( 0 ),
   mSequenceNumber(0)
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
