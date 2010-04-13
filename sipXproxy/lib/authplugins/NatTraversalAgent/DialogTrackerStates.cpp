//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "DialogTrackerStates.h"
#include "DialogTracker.h"
#include "net/SipMessage.h"

// DEFINES
#define IDLE_MEDIA_MAXIMUM_IN_SECONDS ( 30 * 60 )

// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

const DialogTrackerState* DialogTrackerState::GetParent( DialogTracker& impl ) const
{
   return 0;
}

const DialogTrackerState* DialogTrackerState::GetInitialState( DialogTracker& impl ) const
{
   return 0;
}

void DialogTrackerState::DoEntryAction( DialogTracker& impl ) const
{
}

void DialogTrackerState::DoExitAction( DialogTracker& impl ) const
{
}

const char* DialogTrackerState::name( void ) const
{
   return "DialogTrackerState";
}

bool DialogTrackerState::InviteRequest( DialogTracker& impl, SipMessage& request, TransactionDirectionality direction, const char* address, int port  ) const
{
   if( !impl.isRequestAlreadyHandledByOther( request ) )
   {
      impl.markRequestAsHandledByUs( request );
   }

   if( impl.isARetransmittedRequest( request ) )
   {
      impl.restoreSdpBodyOfRetransmittedRequest( request );
   }
   else
   {
      OsSysLog::add(FAC_NAT,PRI_WARNING,"'%s': Received unexpected event InviteRequest while in state '%s'",
            impl.name(), impl.GetCurrentState()->name() );
   }
   return true;
}

bool DialogTrackerState::AckRequest( DialogTracker& impl, SipMessage& request, TransactionDirectionality direction, const char* address, int port ) const
{
   if( impl.isARetransmittedRequest( request ) )
   {
      impl.restoreSdpBodyOfRetransmittedRequest( request );
   }
   else
   {
      OsSysLog::add(FAC_NAT,PRI_WARNING,"'%s': Received unexpected event AckRequest while in state '%s'",
            impl.name(), impl.GetCurrentState()->name() );
   }
   return false;
}

bool DialogTrackerState::ByeRequest( DialogTracker& impl, SipMessage& request, TransactionDirectionality direction, const char* address, int port ) const
{
   impl.deallocateAndClearAllMediaRelaySessions();
   ChangeState( impl, impl.pMoribund );
   return false;
}

bool DialogTrackerState::PrackRequest( DialogTracker& impl, SipMessage& request, TransactionDirectionality direction, const char* address, int port ) const
{
   if( impl.isARetransmittedRequest( request ) )
   {
      impl.restoreSdpBodyOfRetransmittedRequest( request );
   }
   else
   {
      OsSysLog::add(FAC_NAT,PRI_WARNING,"'%s': Received unexpected event PrackRequest while in state '%s'",
            impl.name(), impl.GetCurrentState()->name() );
   }
   return true;
}

bool DialogTrackerState::UpdateRequest( DialogTracker& impl, SipMessage& request, TransactionDirectionality direction, const char* address, int port ) const
{
   if( impl.isARetransmittedRequest( request ) )
   {
      impl.restoreSdpBodyOfRetransmittedRequest( request );
   }
   else
   {
      OsSysLog::add(FAC_NAT,PRI_WARNING,"'%s': Received unexpected event UpdateRequest while in state '%s'",
            impl.name(), impl.GetCurrentState()->name() );
   }
   return true;
}

void DialogTrackerState::ProvisionalResponse( DialogTracker& impl, SipMessage& response, const char* address, int port ) const
{
   if( impl.isARetransmittedResponse( response ) )
   {
      impl.restoreSdpBodyOfRetransmittedResponse( response );
   }
   else
   {
      OsSysLog::add(FAC_NAT,PRI_WARNING,"'%s': Received unexpected event ProvisionalResponse while in state '%s'",
            impl.name(), impl.GetCurrentState()->name() );
   }
}

void DialogTrackerState::SuccessfulResponse( DialogTracker& impl, SipMessage& response, const char* address, int port ) const
{
   if( impl.isARetransmittedResponse( response ) )
   {
      impl.restoreSdpBodyOfRetransmittedResponse( response );
   }
   else
   {
      OsSysLog::add(FAC_NAT,PRI_WARNING,"'%s': Received unexpected event SuccessfulResponse while in state '%s'",
            impl.name(), impl.GetCurrentState()->name() );
   }
}

void DialogTrackerState::RedirectionResponse( DialogTracker& impl, SipMessage& response, const char* address, int port ) const
{
   OsSysLog::add(FAC_NAT,PRI_WARNING,"'%s': Received unexpected event RedirectionResponse while in state '%s'",
         impl.name(), impl.GetCurrentState()->name() );
}

void DialogTrackerState::FailureResponse( DialogTracker& impl, SipMessage& response, const char* address, int port ) const
{
   OsSysLog::add(FAC_NAT,PRI_WARNING,"'%s': Received unexpected event FailureResponse while in state '%s'",
         impl.name(), impl.GetCurrentState()->name() );
}

void DialogTrackerState::CleanUpTimerTick( DialogTracker& impl ) const
{
   OsSysLog::add(FAC_NAT,PRI_WARNING,"'%s': Received unexpected event CleanUpTimerTick while in state '%s'",
         impl.name(), impl.GetCurrentState()->name() );
}

void DialogTrackerState::ChangeState( DialogTracker& impl,
                                       const DialogTrackerState* targetState ) const
{
    StateAlg::ChangeState( impl, this, targetState );
}


const char* WaitingForInvite::name( void ) const
{
   return "WaitingForInvite";
}

void WaitingForInvite::DoEntryAction( DialogTracker& impl ) const
{
   impl.modifyNonIntialOfferAnswerExchangeDoneFlag( false );
}

bool WaitingForInvite::InviteRequest( DialogTracker& impl, SipMessage& request, TransactionDirectionality direction, const char* address, int port ) const
{
   bool bTrackRequestResponse = false;
   impl.setTransactionDirectionality( direction );

   // check is another sipX is already taking care of NAT traversing this session.
   if( !impl.isRequestAlreadyHandledByOther( request ) )
   {
      bTrackRequestResponse = true;
      impl.markRequestAsHandledByUs( request );
      // check if the INVITE contains an SDP offer
      if( request.hasSdpBody() )
      {
         // request contains an SDP offer.  Check if the SDP or the or the actual
         // location of the two endpoints impose the use of a media relay.
         if( impl.doesEndpointsLocationImposeMediaRelay() )
         {
            OsSysLog::add(FAC_NAT,PRI_DEBUG,"'%s:%s' - Media relay required",
                  impl.name(), impl.GetCurrentState()->name() );
            impl.setMediaRelayRequiredFlag();
         }
         else
         {
            OsSysLog::add(FAC_NAT,PRI_DEBUG,"'%s:%s' - Media relay not required",
                  impl.name(), impl.GetCurrentState()->name() );
            impl.clearMediaRelayRequiredFlag();
         }
         impl.ProcessMediaOffer( request, INITIAL_OFFER_ANSWER );
         ChangeState( impl, impl.pWaitingForMediaAnswer );
      }
      else
      {
         // request does contains an SDP offer.  This INVITE may be used to set up a 3PCC
         // call.  Play it safe and impose the use of a media relay to guarantee speechpath
         impl.setMediaRelayRequiredFlag();
         ChangeState( impl, impl.pWaitingForMediaOffer );
      }
   }
   else
   {
      // this particular session is already being handled by another sipX i nthe network.
      // That sipX is taking care of overcoming the NATs that separate the endpoints involved
      // in that session.  There isn't much value we can add here so just bail on tracking that
      // session.
      ChangeState( impl, impl.pMoribund );
   }
   return bTrackRequestResponse;
}

void WaitingForInvite::CleanUpTimerTick( DialogTracker& impl ) const
{
   if( impl.getDialogEstablishedFlag() )
   {
      // we have an established dialog - check it see if the media is still flowing
      if( !impl.wasMediaTrafficSeenInLastNSeconds( IDLE_MEDIA_MAXIMUM_IN_SECONDS ) )
      {
         OsSysLog::add(FAC_NAT,PRI_WARNING,"'%s': Terminating dialog tracker due to excessive media inactivity period",
               impl.name() );
         ChangeState( impl, impl.pMoribund );
      }
   }
}

const char* TimeBoundState::name( void ) const
{
   return "TimeBoundState";
}

void TimeBoundState::DoEntryAction( DialogTracker& impl ) const
{
   impl.resetTimerTickCounter();
}

void TimeBoundState::CleanUpTimerTick( DialogTracker& impl ) const
{
   if( impl.incrementTimerTickCounter() >= MAX_TIMER_TICK_COUNTS_BEFORE_DIALOG_TRACKER_CLEAN_UP )
   {
      OsSysLog::add(FAC_NAT,PRI_DEBUG,"'%s:%s' - cleaning up stale dialog tracker",
             impl.name(), impl.GetCurrentState()->name() );
      ChangeState( impl, impl.pMoribund );
   }
}

const char* Negotiating::name( void ) const
{
   return "Negotiating";
}

const DialogTrackerState* Negotiating::GetParent( DialogTracker& impl ) const
{
   return impl.pTimeBoundState;
}

void Negotiating::FailureResponse( DialogTracker& impl, SipMessage& response, const char* address, int port ) const
{
   int seqNum;
   UtlString seqMethod;

   if( response.getCSeqField( &seqNum, &seqMethod ) )
   {
      if( seqMethod.compareTo( SIP_INVITE_METHOD ) == 0 )
      {
         // session negotiation failed.  Deallocate all the tentative
         // media relays tentatively allocated to handle the media
         // sessions that just failed.
         impl.deallocateAndClearAllMediaRelaySessions( true, true, false );

         if( !impl.getDialogEstablishedFlag() )
         {
            // this is a final failure response to a dialog-forming INVITE.  That
            // event marks the end of the dialog hence, we do not need to continue
            // to track it.
            ChangeState( impl, impl.pMoribund );
         }
         else
         {
            // the renegotiation failed but the dialog is still active. Go back to state where
            // we wait for an incoming INVITE.
            ChangeState( impl, impl.pWaitingForInvite );
         }
      }
      else
      {
         OsSysLog::add(FAC_NAT,PRI_DEBUG,"'%s:%s' - Received unexpected successful response for %s request",
               impl.name(), impl.GetCurrentState()->name(), seqMethod.data() );
      }
   }
}

const char* ProcessingPrack::name( void ) const
{
   return "ProcessingPrack";
}

const DialogTrackerState* ProcessingPrack::GetParent( DialogTracker& impl ) const
{
   return impl.pNegotiating;
}

void ProcessingPrack::ProvisionalResponse( DialogTracker& impl, SipMessage& response, const char* address, int port ) const
{
   int seqNum;
   UtlString seqMethod;

   if( response.getCSeqField( &seqNum, &seqMethod ) )
   {
      if( seqMethod.compareTo( SIP_INVITE_METHOD ) == 0 )
      {
         // normally we would not be expecting this provisional response to carry
         // an SDP in this state however we have encountered in the field some endpoints
         // that repeat the SDP answer they already sent in a previous
         // reliable provisional response (see XECS-2079 for the details).
         // Given that, if an SDP answer is found, we will reprocess it
         // to make sure it gets the same transformations that the initial
         // one got as per XECS-2089.
         if( response.hasSdpBody() )
         {
            impl.ProcessMediaAnswer( response, INITIAL_OFFER_ANSWER );
         }
      }
      else
      {
         OsSysLog::add(FAC_NAT,PRI_DEBUG,"'%s:%s' - Received unexpected Provisional Response for %s request",
               impl.name(), impl.GetCurrentState()->name(), seqMethod.data() );
      }
   }
   // We have received a response - although that does
   // not cause a state machine state change, we need to reset the tick counter
   // to show that there is still activity in this dialog.
   impl.resetTimerTickCounter();
}

void ProcessingPrack::SuccessfulResponse( DialogTracker& impl, SipMessage& response, const char* address, int port ) const
{
   int seqNum;
   UtlString seqMethod;

   if( response.getCSeqField( &seqNum, &seqMethod ) )
   {
      if( seqMethod.compareTo( SIP_INVITE_METHOD ) == 0 )
      {
         // normally we would not be expecting this 200 OK response to carry
         // an SDP in this state however we have encountered in the field some endpoints
         // that repeat the SDP answer they already sent in a previous
         // Successful response (see XECS-2079 for the details).
         // Given that, if an SDP answer is found, we will reprocess it
         // to make sure it gets the same transformations that the initial
         // one got as per XECS-2089.
         if( response.hasSdpBody() )
         {
            impl.ProcessMediaAnswer( response, INITIAL_OFFER_ANSWER );
         }
      }
      else
      {
         OsSysLog::add(FAC_NAT,PRI_DEBUG,"'%s:%s' - Received unexpected Successful Response for %s request",
               impl.name(), impl.GetCurrentState()->name(), seqMethod.data() );
      }
      // We have received a response - although that does
      // not cause a state machine state change, we need to reset the tick counter
      // to show that there is still activity in this dialog.
      impl.resetTimerTickCounter();
   }
}

const char* WaitingForAckForInvite::name( void ) const
{
   return "WaitingForAckForInvite";
}

const DialogTrackerState* WaitingForAckForInvite::GetParent( DialogTracker& impl ) const
{
   return impl.pTimeBoundState;
}


bool WaitingForAckForInvite::AckRequest( DialogTracker& impl, SipMessage& request, TransactionDirectionality direction, const char* address, int port ) const
{
   impl.setDialogEstablishedFlag();
   impl.promoteTentativeMediaRelaySessionsToCurrent();
   ChangeState( impl, impl.pWaitingForInvite );
   return false;
}

void Moribund::DoEntryAction( DialogTracker& impl ) const
{
   impl.reportDialogCompleted();
}

const char* Moribund::name( void ) const
{
   return "Moribund";
}

const DialogTrackerState* WaitingForMediaOffer::GetParent( DialogTracker& impl ) const
{
   return impl.pNegotiating;
}

const char* WaitingForMediaOffer::name( void ) const
{
   return "WaitingForMediaOffer";
}

void WaitingForMediaOffer::ProvisionalResponse( DialogTracker& impl, SipMessage& response, const char* address, int port ) const
{
   if( response.getResponseStatusCode() != SIP_TRYING_CODE )
   {
      // Both reliable and unreliable provisional responses can carry SDP bodies.  According to
      // draft-ietf-sipping-sip-offeranswer-04.txt section 3.1, unreliable provisional responses
      // carrying an offer is a mere preview of what the 'real' SDP offfer will be and that it
      // must be identical to it.  Since we may be changing the SDP of the 'real' offer to compensate
      // for NATs we need to also manipulate the 'preview' offer to make it meet the requirement that
      // the preview and 'real' offers be identical.
      impl.ProcessMediaOffer( response, INITIAL_OFFER_ANSWER );

      // we are receiving a provisional response - check if it is sent reliably...
      if( response.getHeaderValue( 0, SIP_RSEQ_FIELD ) )
      {
         // Presence of RSeq: header in the message indicates that it is sent reliably
         ChangeState( impl, impl.pWaitingForPrackWithMediaAnswer );
      }
      else
      {
         // We have received an unreliable provisional response - take a copy of the
         // patched SDP so that it can be re-applied to subsequent responses carrying
         // the same SDP body.
         impl.savePatchedSdpPreview( response );
         ChangeState( impl, impl.pWaitingFor200OkWithMediaOffer );
      }
   }
}

void WaitingForMediaOffer::SuccessfulResponse( DialogTracker& impl, SipMessage& response, const char* address, int port ) const
{
   impl.ProcessMediaOffer( response, INITIAL_OFFER_ANSWER );
   ChangeState( impl, impl.pWaitingForAckWithAnswerForInvite );
}

const DialogTrackerState* WaitingFor200OkWithMediaOffer::GetParent( DialogTracker& impl ) const
{
   return impl.pNegotiating;
}

const char* WaitingFor200OkWithMediaOffer::name( void ) const
{
   return "WaitingFor200OkWithMediaOffer";
}

void WaitingFor200OkWithMediaOffer::ProvisionalResponse( DialogTracker& impl, SipMessage& response, const char* address, int port ) const
{
   if( response.getResponseStatusCode() != SIP_TRYING_CODE )
   {
      // RFC requires that all SDP previews be identical.  In ensure that this
      // requirement is met, we apply the saved copy of the patched SDP preview
      // to the response.
      if( response.hasSdpBody() )
      {
         impl.applyPatchedSdpPreview( response );
      }

      // we are receiving a provisional response - check if it is sent reliably...
      if( response.getHeaderValue( 0, SIP_RSEQ_FIELD ) )
      {
         // Presence of RSeq: header in the message indicates that it is sent reliably
         ChangeState( impl, impl.pWaitingForPrackWithMediaAnswer );
      }
      else
      {
         // We have received an unreliable provisional response - although that does
         // not cause a state machine state change, we need to reset the tick counter
         // to show that there is still activity in this dialog.
         impl.resetTimerTickCounter();
      }
   }
}

void WaitingFor200OkWithMediaOffer::SuccessfulResponse( DialogTracker& impl, SipMessage& response, const char* address, int port ) const
{
   // RFC requires that all SDP previews be identical.  In ensure that this
   // requirement is met, we apply the saved copy of the patched SDP preview
   // to the response.
   if( response.hasSdpBody() )
   {
      impl.applyPatchedSdpPreview( response );
   }
   ChangeState( impl, impl.pWaitingForAckWithAnswerForInvite );
}

const DialogTrackerState* WaitingForMediaAnswer::GetParent( DialogTracker& impl ) const
{
   return impl.pNegotiating;
}

const char* WaitingForMediaAnswer::name( void ) const
{
   return "WaitingForMediaAnswer";
}

void WaitingForMediaAnswer::ProvisionalResponse( DialogTracker& impl, SipMessage& response, const char* address, int port ) const
{
   if( response.getResponseStatusCode() != SIP_TRYING_CODE )
   {
      if( response.hasSdpBody() )
      {
         // Both reliable and unreliable provisional responses can carry SDP bodies.  According to
         // draft-ietf-sipping-sip-offeranswer-04.txt section 3.1, unreliable provisional responses
         // carrying an answer is a mere previes of what the 'real' SDP answer will be and that it
         // must be identical to it.  Since we may be changing the SDP of the 'real' answer to compensate
         // for NATs we need to also manipulate the 'preview' answer to make match the requirement that
         // the preview and 'real' answers be identical.

         // we are receiving a provisional response - check if it is sent reliably...
         impl.ProcessMediaAnswer( response, INITIAL_OFFER_ANSWER );
         if( response.getHeaderValue( 0, SIP_RSEQ_FIELD ) )
         {
            // Presence of RSeq: header in the message indicates that it is sent reliably
            ChangeState( impl, impl.pWaitingForPrack );
         }
         else
         {
            // We have received an unreliable provisional response - although that does
            // not cause a state machine state change, we need to reset the tick counter
            // to show that there is still activity in this dialog.
            impl.resetTimerTickCounter();
         }
      }
   }
}

void WaitingForMediaAnswer::SuccessfulResponse( DialogTracker& impl, SipMessage& response, const char* address, int port ) const
{
   int seqNum;
   UtlString seqMethod;

   if( response.getCSeqField( &seqNum, &seqMethod ) )
   {
      if( seqMethod.compareTo( SIP_INVITE_METHOD ) == 0 )
      {
         impl.ProcessMediaAnswer( response, INITIAL_OFFER_ANSWER );
         ChangeState( impl, impl.pWaitingForAckForInvite );
      }
   }
}

const DialogTrackerState* WaitingForPrack::GetParent( DialogTracker& impl ) const
{
   return impl.pProcessingPrack;
}

const char* WaitingForPrack::name( void ) const
{
   return "WaitingForPrack";
}

bool WaitingForPrack::PrackRequest( DialogTracker& impl, SipMessage& request, TransactionDirectionality direction, const char* address, int port ) const
{
   if( request.hasSdpBody() )
   {
      impl.ProcessMediaOffer( request, NON_INITIAL_OFFER_ANSWER );
      ChangeState( impl, impl.pWaitingFor200OkWithAnswerForPrack );
   }
   else
   {
      ChangeState( impl, impl.pWaitingFor200OkForPrack );
   }
   return true;
}

const DialogTrackerState* WaitingForAckWithAnswerForInvite::GetParent( DialogTracker& impl ) const
{
   return impl.pNegotiating;
}

const char* WaitingForAckWithAnswerForInvite::name( void ) const
{
   return "WaitingForAckWithAnswerForInvite";
}

bool WaitingForAckWithAnswerForInvite::AckRequest( DialogTracker& impl, SipMessage& request, TransactionDirectionality direction, const char* address, int port ) const
{
   impl.ProcessMediaAnswer( request, INITIAL_OFFER_ANSWER );
   impl.setDialogEstablishedFlag();
   impl.promoteTentativeMediaRelaySessionsToCurrent();
   ChangeState( impl, impl.pWaitingForInvite );
   return false;
}

const DialogTrackerState* WaitingForPrackWithMediaAnswer::GetParent( DialogTracker& impl ) const
{
   return impl.pNegotiating;
}

const char* WaitingForPrackWithMediaAnswer::name( void ) const
{
   return "WaitingForPrackWithMediaAnswer";
}

bool WaitingForPrackWithMediaAnswer::PrackRequest( DialogTracker& impl, SipMessage& request, TransactionDirectionality direction, const char* address, int port ) const
{
   impl.ProcessMediaAnswer( request, INITIAL_OFFER_ANSWER );
   ChangeState( impl, impl.pWaitingFor200OkForSlowStartPrack );
   return true;
}

const DialogTrackerState* WaitingFor200OkForSlowStartPrack::GetParent( DialogTracker& impl ) const
{
   return impl.pNegotiating;
}

const char* WaitingFor200OkForSlowStartPrack::name( void ) const
{
   return "WaitingFor200OkForSlowStartPrack";
}

void WaitingFor200OkForSlowStartPrack::SuccessfulResponse( DialogTracker& impl, SipMessage& response, const char* address, int port ) const
{
   int seqNum;
   UtlString seqMethod;

   if( response.getCSeqField( &seqNum, &seqMethod ) )
   {
      if( seqMethod.compareTo( SIP_PRACK_METHOD ) == 0 )
      {
         ChangeState( impl, impl.pWaitingForAckForInvite );
      }
      else
      {
         OsSysLog::add(FAC_NAT,PRI_DEBUG,"'%s:%s' - Received unexpected successful response for %s request",
               impl.name(), impl.GetCurrentState()->name(), seqMethod.data() );
      }
   }
}

void WaitingFor200OkForSlowStartPrack::FailureResponse( DialogTracker& impl, SipMessage& response, const char* address, int port ) const
{
   int seqNum;
   UtlString seqMethod;

   if( response.getCSeqField( &seqNum, &seqMethod ) )
   {
      if( seqMethod.compareTo( SIP_PRACK_METHOD ) == 0 )
      {
         // PRACK was rejected.  Keep tentative INVITE media relays in case an
         // acceptable PRACK gets generated by the UAC.
         ChangeState( impl, impl.pWaitingForPrackWithMediaAnswer );
      }
      else
      {
         OsSysLog::add(FAC_NAT,PRI_DEBUG,"'%s:%s' - Received unexpected failure response for %s request",
               impl.name(), impl.GetCurrentState()->name(), seqMethod.data() );
      }
   }
}

const DialogTrackerState* WaitingFor200OkForPrack::GetParent( DialogTracker& impl ) const
{
   return impl.pProcessingPrack;
}

const char* WaitingFor200OkForPrack::name( void ) const
{
   return "WaitingFor200OkForPrack";
}

void WaitingFor200OkForPrack::SuccessfulResponse( DialogTracker& impl, SipMessage& response, const char* address, int port ) const
{
   int seqNum;
   UtlString seqMethod;

   if( response.getCSeqField( &seqNum, &seqMethod ) )
   {
      if( seqMethod.compareTo( SIP_PRACK_METHOD ) == 0 )
      {
         ChangeState( impl, impl.pProcessingPrackWaitingForAckforInvite );
      }
      else
      {
         // Not interesting for us but our parent class provides some handling for
         // other successful responses.
         ProcessingPrack::SuccessfulResponse( impl, response, address, port );
      }
   }
}

void WaitingFor200OkForPrack::FailureResponse( DialogTracker& impl, SipMessage& response, const char* address, int port ) const
{
   int seqNum;
   UtlString seqMethod;

   if( response.getCSeqField( &seqNum, &seqMethod ) )
   {
      if( seqMethod.compareTo( SIP_PRACK_METHOD ) == 0 )
      {
         ChangeState( impl, impl.pWaitingForPrack );
      }
      else
      {
         OsSysLog::add(FAC_NAT,PRI_DEBUG,"'%s:%s' - Received unexpected successful response for %s request",
               impl.name(), impl.GetCurrentState()->name(), seqMethod.data() );
      }
   }
}

const DialogTrackerState* WaitingFor200OkWithAnswerForPrack::GetParent( DialogTracker& impl ) const
{
   return impl.pProcessingPrack;
}

const char* WaitingFor200OkWithAnswerForPrack::name( void ) const
{
   return "WaitingFor200OkWithAnswerForPrack";
}

void WaitingFor200OkWithAnswerForPrack::SuccessfulResponse( DialogTracker& impl, SipMessage& response, const char* address, int port ) const
{
   int seqNum;
   UtlString seqMethod;

   if( response.getCSeqField( &seqNum, &seqMethod ) )
   {
      if( seqMethod.compareTo( SIP_PRACK_METHOD ) == 0 )
      {
         impl.ProcessMediaAnswer( response, NON_INITIAL_OFFER_ANSWER );
         impl.modifyNonIntialOfferAnswerExchangeDoneFlag( true );
         ChangeState( impl, impl.pProcessingPrackWaitingForAckforInvite );
      }
      else
      {
         // Not interesting for us but our parent class provides some handling for
         // other successful responses.
         ProcessingPrack::SuccessfulResponse( impl, response, address, port );
      }
   }
}

void WaitingFor200OkWithAnswerForPrack::FailureResponse( DialogTracker& impl, SipMessage& response, const char* address, int port ) const
{
   int seqNum;
   UtlString seqMethod;

   if( response.getCSeqField( &seqNum, &seqMethod ) )
   {
      if( seqMethod.compareTo( SIP_PRACK_METHOD ) == 0 )
      {
         // PRACK offer/answer failed.  Deallocate any tentative media relays allocation
         // in preparation for that failed PRACK offer/answer negotiation.
         impl.deallocateAndClearAllMediaRelaySessions( false, true, false );
         ChangeState( impl, impl.pWaitingForPrack );
      }
      else
      {
         OsSysLog::add(FAC_NAT,PRI_DEBUG,"'%s:%s' - Received unexpected failure response for %s request",
               impl.name(), impl.GetCurrentState()->name(), seqMethod.data() );
      }
   }
}

const DialogTrackerState* ProcessingPrackWaitingForAckforInvite::GetParent( DialogTracker& impl ) const
{
   return impl.pProcessingPrack;
}

const char* ProcessingPrackWaitingForAckforInvite::name( void ) const
{
   return "ProcessingPrackWaitingForAckforInvite";
}

bool ProcessingPrackWaitingForAckforInvite::AckRequest( DialogTracker& impl, SipMessage& request, TransactionDirectionality direction, const char* address, int port ) const
{
      impl.setDialogEstablishedFlag();
      impl.promoteTentativeMediaRelaySessionsToCurrent();
      ChangeState( impl, impl.pWaitingForInvite );
      return false;
}
