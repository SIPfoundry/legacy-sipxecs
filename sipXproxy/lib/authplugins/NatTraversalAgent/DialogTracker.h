//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _DIALOGTRACKER_H_
#define _DIALOGTRACKER_H_

// SYSTEM INCLUDES
#include <vector>

// APPLICATION INCLUDES
#include "NatTraversalAgentDataTypes.h"
#include "DialogTrackerStates.h"
#include "utl/UtlContainable.h"
#include "utl/UtlString.h"

// DEFINES
#define MAX_TIMER_TICK_COUNTS_BEFORE_DIALOG_TRACKER_CLEAN_UP (6)

// NAMESPACES
using namespace std;
// CONSTANTS
// FORWARD DECLARATIONS
class SessionContextInterfaceForDialogTracker;

// TYPEDEFS

// classifies the various offer/answer patterns into either INITIAL_OFFER_ANSWER or
 // NON_INITIAL_OFFER_ANSWER.
 //--------------------------------------------------------------------------------------
 //         Offer                Answer         Pattern Family
 //--------------------------------------------------------------------------------------
 //   INVITE Req.          2xx INVITE Resp.       INITIAL_OFFER_ANSWER
 //   INVITE Req.          1xx-rel INVITE Resp.   INITIAL_OFFER_ANSWER
 //   2xx INVITE Resp.     ACK Req.               INITIAL_OFFER_ANSWER
 //   INVITE Req.          1xx INVITE Resp.       INITIAL_OFFER_ANSWER
 //   1xx-rel INVITE Resp. PRACK Req.             INITIAL_OFFER_ANSWER
 //   PRACK Req.           200 PRACK Resp.        NON_INITIAL_OFFER_ANSWER
 //--------------------------------------------------------------------------------------
 typedef enum
 {
    INITIAL_OFFER_ANSWER,
    NON_INITIAL_OFFER_ANSWER
 } OfferAnswerPattern;

/**
 * The DialogTracker class implements the state machine that
 * knows how to track a dialog's answer/offer and intervene
 * when necessary to facilitatre NAT traversal.  This includes
 * adjusting SDP and configuring the Media Relay when needed.
 */
class DialogTracker : public UtlContainable
{

public:
   // constructor
   DialogTracker( const UtlString& handle,
                  const UtlString& systemIdentificationString,
                  SessionContextInterfaceForDialogTracker* pOwningSessionContext );
   DialogTracker( const DialogTracker& referenceDialogTracker, const UtlString& newHandle );

   // destructor
   virtual ~DialogTracker();

   // ////////////////////// //
   // UtlContainable methods //
   // ////////////////////// //
   virtual UtlContainableType getContainableType( void ) const;
   virtual unsigned hash() const;
   virtual int compareTo(UtlContainable const *rhsContainable ) const;

   // ////////////////////////////////////////////// //
   // Incoming events relevant to this DialogTracker //
   // ////////////////////////////////////////////// //
   bool handleRequest ( SipMessage& message, const char* address, int port, bool bFromCallerToCallee );
   void handleResponse( SipMessage& message, const char* address, int port );
   void handleCleanUpTimerTick( void );

   // ////////////////////////////////// //
   // Methods required by SessionContext //
   // ////////////////////////////////// //
   const UtlString& getHandle( void ) const { return mHandle; }
   void setHandle( const UtlString& newHandle ){ mHandle = newHandle; }

   // /////////////////////////////////////// //
   // state machine framework related methods //
   // /////////////////////////////////////// //
   const DialogTrackerState* GetCurrentState() const;        ///< Returns the current state, as required by UtlFsm's StateAlg
   void SetCurrentState( const DialogTrackerState* pState ); ///< Sets the current state, as required by UtlFsm's StateAlg
   const char* name( void ) const;

   // ////////////////////////////////// //
   // State machine state helper methods //
   // ////////////////////////////////// //

   // Clean-up timer related methods
   void resetTimerTickCounter( void );
   ssize_t incrementTimerTickCounter( void );
   ssize_t getTimerTickCounter( void ) const;

   // Manipulators for 'Dialog Established' flag
   bool getDialogEstablishedFlag( void ) const;
   void setDialogEstablishedFlag( void );
   void clearDialogEstablishedFlag( void );

   // Manipulators for 'Media Relay Required' flag
   bool getMediaRelayRequiredFlag( void ) const;
   void setMediaRelayRequiredFlag( void );
   void clearMediaRelayRequiredFlag( void );

   // Manipulators for 'Non Initial Offer/Answer done' flag
   bool getNonIntialOfferAnswerExchangeDoneFlag( void ) const;
   void modifyNonIntialOfferAnswerExchangeDoneFlag( bool newValue );

   // Media Descriptor manipulators
   size_t getNumberOfMediaDescriptors( void ) const;
   MediaDescriptor* getModifiableMediaDescriptor( size_t descriptorIndex );
   const MediaDescriptor* getReadOnlyMediaDescriptor( size_t descriptorIndex );
   void appendMediaDescriptor( MediaDescriptor* pMediaDescriptor );

   // Media Relay Session-related methods
   bool allocateMediaRelaySession( tMediaRelayHandle& relayHandle, int& callerRelayRtpPort, int& calleeRelayRtpPort );
   tMediaRelayHandle cloneMediaRelaySession( tMediaRelayHandle& relayHandleToClone, bool doSwapCallerAndCallee );
   bool deallocateMediaRelaySession( const tMediaRelayHandle& relayHandle );
   void deallocateAndClearAllMediaRelaySessions( bool bDeallocateTentativeInitialRelays = true,
                                                 bool bDeallocateTentativeNonInitialRelays = true,
                                                 bool bDeallocateCurrentRelays = true );
   bool wasMediaTrafficSeenInLastNSeconds( unsigned long numberOfSeconds );

   // Methods for Media Relay handle encoding/decoding in SDP
   tMediaRelayHandle getOurMediaRelayHandleEncodedInSdp( const SdpBody* pSdpBody, int mediaIndex ) const;
   bool hasSdpAlreadyBeenPatchedByUs( SipMessage& message, int mediaIndex ) const;
   bool hasSdpAlreadyBeenPatchedByUs( const SdpBody* pSdpBody, int mediaIndex ) const;

   // Transaction directionality-related methods
   void setTransactionDirectionality( TransactionDirectionality directionality );
   TransactionDirectionality getTransactionDirectionality( void ) const;

   // Methods used to help determine if message is already NAT-compensated
   void markRequestAsHandledByUs( SipMessage& request );
   bool isRequestAlreadyHandledByUs( const SipMessage& request ) const;
   bool isRequestAlreadyHandledByOther( const SipMessage& request ) const;
   bool isRequestAlreadyHandledByAnyone( const SipMessage& request ) const;

   // Retransmission handling methods
   bool isARetransmittedRequest( const SipMessage& request );
   bool isARetransmittedResponse( const SipMessage& response );
   void restoreSdpBodyOfRetransmittedRequest( SipMessage& request );
   void restoreSdpBodyOfRetransmittedResponse( SipMessage& response );

   // Media Relay use determination method
   bool doesEndpointsLocationImposeMediaRelay( void ) const;

   /**
    * Performs the necessary actions and modifications to ensure that SDP Offer
    * produces a media session that can traverse NATs
    */
   void ProcessMediaOffer ( SipMessage& message, OfferAnswerPattern offerAnswerPattern );

   /**
    * Performs the necessary actions and modifications to ensure that SDP Answer
    * produces a media session that can traverse NATs
    */
   void ProcessMediaAnswer( SipMessage& message, OfferAnswerPattern offerAnswerPattern );

   /**
    * The processing of Offers and Answers haas caused the allocation of tentative MediaRelaySessions
    * to be used if the INVITE proves to be successful (accepted with 200 OK).  This method is called
    * after an INVITE succeeds to promote the tentatively allcoated MediaRelaySessions into the obes
    * that actually get used to relay the newly negotiated media session.
    */
   void promoteTentativeMediaRelaySessionsToCurrent( void );

   /**
    * Returns the role (CALLER or CALLEE of the originator the message currently being processed.
    */
   EndpointRole EstablishEndpointRole( TransactionDirectionality directionality, bool bMessageIsResponse ) const;

   /**
    *   Notification from the state machine that the dialog is completed and that it
    *   can delete itself.  The caller of this method MUST NOT invoke and further
    *   methods on this DialogTracker after having called this method.
    */
   void reportDialogCompleted( void );

   /**
    *   Configures the directionality of the media relay.  Note that the directionality
    *   specified in the 'mediaRelayDirectionMode' parameter is referenced from the caller.
    */
   bool setMediaRelayDirectionMode( const tMediaRelayHandle& relayHandle, MediaDirectionality mediaRelayDirectionMode, EndpointRole endpointRole );

   /**
    *   Establishes the requester as the media destination for the far-end's media relay port
    */
   bool linkFarEndMediaRelayPortToRequester( const tMediaRelayHandle& relayHandle, const MediaDescriptor* pMediaDescriptor, EndpointRole endpointRoleOfRequester );

   /**
    *   Computes the media relay IP address to use when patching SDP originating from endpoint designated by endpointRole param
    *   @returns true if media relay's native IP address is to be used. false if media relay's public IP address is to be used
    */
   bool getMediaRelayAddressToUseInSdp( UtlString& mediaRelayAddressToUse, EndpointRole endpointRole ) const;

   /**
    *   Modifies the supplied SDP body's media description section as specified by the 'mediaIndex'
    *   parameter with the media connection information provided.
    */
   bool patchSdp( SdpBody* pSdpBody, int mediaIndex, int rtpPort, tMediaRelayHandle relayHandle, const UtlString& mediaRelayAddressToUse );

   /**
    *   Saves a copy of the SDP body contained in the supplied message.  This method
    *   is intended to be used to save the content of an SDP body patched by the NAT
    *   traversal feature so that it can be re-applied to any subsequent message that
    *   carries same SDP as the supplied message.  This helps meet the standards requirement
    *   that states that all SDP previews be identical.
    */
   void savePatchedSdpPreview( SipMessage& sipMessage );

   /**
    *   Applies the patched SDP preview previously saved by the savePatchedSdpPreview()
    *   method to the supplied message
    */
   void applyPatchedSdpPreview( SipMessage& sipMessage );

   /**
    *   Removes all the elements that may impair NAT traversal from the message.
    */
   static void removeUnwantedElements( SipMessage& request );

   static const UtlContainableType TYPE;    /** < Class type used for runtime checking */

   // ///////////////////////////////////////////////////////// //
   // DialogTracker states.  Note: the state objects are shared //
   //                        by all instances of this class.    //
   // See NAT_Traversal_Design_Doc.doc section 4.3.5 for        //
   // description of these states.                              //
   // ///////////////////////////////////////////////////////// //
    static WaitingForInvite*       pWaitingForInvite;
    static WaitingForAckForInvite* pWaitingForAckForInvite;
    static TimeBoundState*         pTimeBoundState;
    static Moribund*               pMoribund;
    static Negotiating*            pNegotiating;
       static WaitingForMediaOffer*                       pWaitingForMediaOffer;
       static WaitingFor200OkWithMediaOffer*              pWaitingFor200OkWithMediaOffer;
       static WaitingForMediaAnswer*                      pWaitingForMediaAnswer;
       static WaitingForAckWithAnswerForInvite*           pWaitingForAckWithAnswerForInvite;
       static WaitingForPrackWithMediaAnswer*             pWaitingForPrackWithMediaAnswer;
       static WaitingFor200OkForSlowStartPrack*           pWaitingFor200OkForSlowStartPrack;
       static ProcessingPrack*                            pProcessingPrack;
          static WaitingForPrack*                            pWaitingForPrack;
          static WaitingFor200OkForPrack*                    pWaitingFor200OkForPrack;
          static WaitingFor200OkWithAnswerForPrack*          pWaitingFor200OkWithAnswerForPrack;
          static ProcessingPrackWaitingForAckforInvite*      pProcessingPrackWaitingForAckforInvite;

protected:

private:
   void initializeStatePointers( void );

   class RetransmissionDescriptor
   {
      public:
         RetransmissionDescriptor();
         virtual ~RetransmissionDescriptor();

         virtual void setMessageToTrackRetransmissionsOf( const SipMessage& messageToTrackRetransmissionsOf,
                                                          const SdpBody& patchedSdpBodyToCopy );
         SdpBody* getCopyOfSdpBody( void ) const;

      protected:
         SdpBody*  mpSavedPatchedSdp;
         UtlString mMethod;
         int       mSequenceNumber;
   };

   class RequestRetransmissionDescriptor : public RetransmissionDescriptor
   {
      public:
         virtual ~RequestRetransmissionDescriptor(){};
         bool operator==( const SipMessage& request ) const;
         bool operator!=( const SipMessage& request ) const;
         RequestRetransmissionDescriptor& operator= ( const RequestRetransmissionDescriptor& rhs );
   };

   class ResponseRetransmissionDescriptor : public RetransmissionDescriptor
   {
      public:
         virtual ~ResponseRetransmissionDescriptor(){};
         bool operator==( const SipMessage& response ) const;
         bool operator!=( const SipMessage& request ) const;
         virtual void setMessageToTrackRetransmissionsOf( const SipMessage& messageToTrackRetransmissionsOf,
                                                          const SdpBody& patchedSdpBodyToCopy );
         ResponseRetransmissionDescriptor& operator= ( const ResponseRetransmissionDescriptor& rhs );

      private:
         int mResponseCode;
   };

   UtlString mHandle;
   UtlString mSystemIdentificationString;
   bool mbMediaRelayRequired;
   TransactionDirectionality mCurrentTransactionDirectionality;
   bool mbDialogEstablished;
   bool mbInitialDialogEstablished;
   const DialogTrackerState* mpCurrentState;
   vector<MediaDescriptor*> mMediaDescriptors;  // each entry in vector maps to an 'm=' line in an SDP body
   ssize_t     mTimerTickCounter;
   SessionContextInterfaceForDialogTracker* pOwningSessionContext;
   RequestRetransmissionDescriptor mRequestRetransmissionDescriptor;
   ResponseRetransmissionDescriptor mResponseRetransmissionDescriptor;
   bool mbNonIntialOfferAnswerExchangeDoneFlag;
   SdpBody* mpCopyOfPatchedSdpBody;

   friend class DialogTrackerTest;
};

#endif // _DIALOGTRACKER_H_
