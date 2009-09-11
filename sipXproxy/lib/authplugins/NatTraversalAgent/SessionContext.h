//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _SESSIONCONTEXT_H_
#define _SESSIONCONTEXT_H_

// SYSTEM INCLUDES
#include <vector>

// APPLICATION INCLUDES
#include "NatTraversalAgentDataTypes.h"
#include "CallTracker.h"
#include "MediaRelay.h"
#include "utl/UtlContainable.h"
#include "utl/UtlString.h"

// DEFINES
#define MAX_TIMER_TICK_COUNTS_BEFORE_SESSION_CONTEXT_CLEAN_UP (6)

// NAMESPACES
using namespace std;
// CONSTANTS
// FORWARD DECLARATIONS
class NatTraversalRules;
class DialogTracker;

// TYPEDEFS

class SessionContextInterfaceForDialogTracker
{
public:

   /**
    * Determines whether or not a media relay needs to be employed in the media session
    * between the caller and the callee baed o ntheir relative location in the network.
    */
   virtual bool doesEndpointsLocationImposeMediaRelay( void ) const = 0;

   /**
    * Requests the allocation of a media relay session on the Smmitron.
    * \param [in] handleOfRequestingDialogContext - what it says
    * \param [out] relayHandle - handle to allocated media relay session
    * \param [out] callerRelayRtpPort - RTP port representing caller on the Symmitron.
    *                                   This is the RTP port the callee will be asked
    *                                   to send its media to.
    * \param [out] calleeRelayRtpPort - RTP port representing callee on the Symmitron.
    *                                   This is the RTP port the caller will be asked
    *                                   to send its media to.
    * \return - true media relay session was properly allocated.
    */
   virtual bool allocateMediaRelaySession( const UtlString& handleOfRequestingDialogContext,
                                           tMediaRelayHandle& relayHandle,
                                           int& callerRelayRtpPort,
                                           int& calleeRelayRtpPort ) = 0;

   /**
    * Requests the cloning of an existing media relay session.  This cloning technique is
    * utilized in scenarios where multiple unrelated dialogs use the same media relay session.
    * The best example of such a scenario is a 3PCC call such as MOH.
    * \param [in] handleOfRequestingDialogContext - what it says
    * \param [in] relayHandleToClone - handle of media relay session to clone
    * \param [in] doSwapCallerAndCallee - indicates whether or not the roles of the caller
    *                                     and callee need to be inverted in the cloned session.
    * \return - handle of the clone media relay session
    */
   virtual tMediaRelayHandle cloneMediaRelaySession( const UtlString& handleOfRequestingDialogContext,
                                                     tMediaRelayHandle& relayHandleToClone,
                                                     bool doSwapCallerAndCallee ) = 0;

   /**
    * Requests the de-allocation of a previously allocated media relay session.
    * \param [in] handleOfRequestingDialogContext - what it says
    * \param [in] relayHandle - handle of media relay session to de-allocate
    * \return - true media relay session was properly de-allocated.
    */
   virtual bool deallocateMediaRelaySession( const UtlString& handleOfRequestingDialogContext,
                                             const tMediaRelayHandle& relayHandle ) = 0;

   /**
    * Used to query the RTP port that a given MediaRelaySession has allocated
    * to the endpoint whose role is passed as a parameter.
    * \param [in] handle - handle of media relay session to query
    * ]param [in] endpointRole - role of endpoint for which to retrieve RTP port
    * \return - PORT_NONE if failed, otherwise RTP port number
    */
   virtual int getRtpRelayPortForMediaRelaySession( const tMediaRelayHandle& handle,
                                                    EndpointRole endpointRole ) = 0;

   /**
    * Used to notify the owning SessionContext that the dialog a DialogTracker
    * non longer needs to be tracked and that the DialogTracker object is redy for
    * deletion.
    * \param [in] handleOfRequestingDialogContext - what it says
    */
   virtual void reportDialogTrackerReadyForDeletion( const UtlString& handleOfRequestingDialogContext ) = 0;

   /**
    * Configures the directionality of the media relay.  Note that the directionality
    * specified in the 'mediaRelayDirectionMode' parameter is referenced from the caller.
    * \param [in] handleOfRequestingDialogContext - what it says
    * \param [in] relayHandle - handle of media relay session on which to apply the direction mode
    * \param [in] mediaRelayDirectionMode - direction mode to apply to media relay session referenced
    *                                       to the originator of the request.
    * \param [in] endpointRole - role of the originator of the request.
    * \return - indicates whether or not the direction was set successfully.
    */
   virtual bool setMediaRelayDirectionMode( const UtlString& handleOfRequestingDialogContext,
                                            const tMediaRelayHandle& relayHandle,
                                            MediaDirectionality mediaRelayDirectionMode,
                                            EndpointRole endpointRole ) = 0;

   /**
    * Establishes the requester as the media destination for the far-end's media relay port.
    * \param [in] handleOfRequestingDialogContext - what it says
    * \param [in] relayHandle - handle of media relay session to apply the change
    * \param [in] pMediaDescriptor - pointer descriptor containing information about the requestor
    * \param [in] endpointRoleOfRequester - role of requestor.
    * \return - indicates whether or not the destination was set successfully.
    */
   virtual  bool linkFarEndMediaRelayPortToRequester( const UtlString& handleOfRequestingDialogContext,
                                                      const tMediaRelayHandle& relayHandle,
                                                      const MediaDescriptor* pMediaDescriptor,
                                                      EndpointRole endpointRoleOfRequester ) = 0;

   /**
    * Computes the media relay IP address to use when patching SDP originating from endpoint
    * designated by endpointRole param
    * \param [out] mediaRelayAddressToUse - will contain the Symmitron address to use when patching SDP
    * \param [in] endpointRole - role of endpoint making the request.
    * \return true if media relay's native IP address is to be used. false if media relay's public IP address is to be used
    */
   virtual bool getMediaRelayAddressToUseInSdp( UtlString& mediaRelayAddressToUse,
                                                EndpointRole endpointRole ) const = 0;

   /**
    * Retrieves the packet processing statistics for a given media relay session designated by its handle
    * \param [in] handle - handle of media relay session for which to obtain packet processing stats
    * \param [out] stats - structure that will receive the requested stats information
    * \return true operation succeeds, false otherwise
    */
   virtual bool getPacketProcessingStatsForMediaRelaySession( const tMediaRelayHandle& handle,
                                                              PacketProcessingStatistics& stats ) = 0;

   virtual ~SessionContextInterfaceForDialogTracker(){};
};

/**
 * The sessionContext class implements the state machine that
 * knows jow to track a call fork answer/offer and intervene
 * when necessary to facilitatre NAT traversal.  This includes
 * adjusting SDP and configuring the Media Relay when needed.
 */
class SessionContext : public UtlContainable, public SessionContextInterfaceForDialogTracker
{

public:
   // constructor
   SessionContext( const SipMessage& sipRequest,
                   const NatTraversalRules* pNatRules,
                   const UtlString& handle,
                   MediaRelay* pMediaRelayToUse,
                   const RegistrationDB* pRegistrationDB,
                   CallTrackerInterfaceForSessionContext* pOwningCallTracker );

   // destructor
   virtual ~SessionContext();

   // ////////////////////// //
   // UtlContainable methods //
   // ////////////////////// //
   virtual UtlContainableType getContainableType( void ) const;
   virtual unsigned hash() const;
   virtual int compareTo(UtlContainable const *rhsContainable ) const;

   // /////////////////////////////////////////////// //
   // Incoming events relevant to this SessionContext //
   // /////////////////////////////////////////////// //
   bool handleRequest ( SipMessage& message, const char* address, int port, bool bFromCallerToCallee );
   void handleResponse( SipMessage& message, const char* address, int port );
   void handleCleanUpTimerTick( void );

   // /////////////////////////////////////////// //
   // SessionContextInterfaceForDialogTracker     //
   // /////////////////////////////////////////// //
   //                 --Note--                    //
   // See SessionContextInterfaceForDialogTracker //
   // for method comments.                        //
   // /////////////////////////////////////////// //
   virtual bool doesEndpointsLocationImposeMediaRelay( void ) const;
   virtual bool allocateMediaRelaySession( const UtlString& handleOfRequestingDialogContext,
                                           tMediaRelayHandle& relayHandle,
                                           int& callerRelayRtpPort,
                                           int& calleeRelayRtpPort );
   virtual tMediaRelayHandle cloneMediaRelaySession( const UtlString& handleOfRequestingDialogContext,
                                                     tMediaRelayHandle& relayHandleToClone,
                                                     bool doSwapCallerAndCallee );
   virtual bool deallocateMediaRelaySession( const UtlString& handleOfRequestingDialogContext,
                                             const tMediaRelayHandle& relayHandle );
   virtual bool setMediaRelayDirectionMode( const UtlString& handleOfRequestingDialogContext,
                                            const tMediaRelayHandle& relayHandle,
                                            MediaDirectionality mediaRelayDirectionMode,
                                            EndpointRole endpointRole );
   virtual bool linkFarEndMediaRelayPortToRequester( const UtlString& handleOfRequestingDialogContext,
                                                     const tMediaRelayHandle& relayHandle,
                                                     const MediaDescriptor* pMediaDescriptor,
                                                     EndpointRole endpointRoleOfRequester );
   virtual bool getMediaRelayAddressToUseInSdp( UtlString& mediaRelayAddressToUse, EndpointRole endpointRole ) const;
   virtual int getRtpRelayPortForMediaRelaySession( const tMediaRelayHandle& handle, EndpointRole endpointRole );
   virtual void reportDialogTrackerReadyForDeletion( const UtlString& handleOfRequestingDialogContext );
   virtual bool getPacketProcessingStatsForMediaRelaySession( const tMediaRelayHandle& handle,
                                                              PacketProcessingStatistics& stats );

   // //// //
   // Misc //
   // //// //
   const EndpointDescriptor& getEndpointDescriptor( EndpointRole endpointRole ) const;
   ssize_t getNumberOfTrackedDialogs( void ) const;

   // /////////////////////////// //
   // Methods used by CallTracker //
   // /////////////////////////// //
   DialogTracker* getDialogTrackerForMessage( const SipMessage& message ) const;

   // //////////////////////////////////////////////// //
   // Utility methods for creating EndpointDescriptors //
   // //////////////////////////////////////////////// //
   /**
    * Creates an endpoint descriptor representing the caller
    * \param [in] sipRequest - request from which caller endpoint descritptor will be initialized
    * \param [in] natTraversalRules - rules containing valuable topology info to initialize descriptor
    * \return new EndpointDescriptor instance representing the caller.  The invoker of this method
    *         is responsible for deleting the instance once it is done with it.
    */
   static EndpointDescriptor* createCallerEndpointDescriptor( const SipMessage& sipRequest,
                                                              const NatTraversalRules& natTraversalRules );

   /**
    * Creates an endpoint descriptor representing the callee
    * \param [in] sipRequest - request from which callee endpoint descritptor will be initialized
    * \param [in] natTraversalRules - rules containing valuable topology info to initialize descriptor
    * \param [in] pRegistrationDB - pointer to registration DB sometimes used to deduce the callee's location info
    * \return new EndpointDescriptor instance representing the callee.  The invoker of this method
    *         is responsible for deleting the instance once it is done with it.
    */
   static EndpointDescriptor* createCalleeEndpointDescriptor( const SipMessage& sipRequest,
                                                              const NatTraversalRules& natTraversalRules,
                                                              const RegistrationDB* pRegistrationDB );

   static const UtlContainableType TYPE;    /** < Class type used for runtime checking */

protected:

private:
   UtlString getDiscriminatingTagValue( const SipMessage& message, bool bFromCallerToCallee ) const;
   UtlString getDiscriminatingTagValue( const SipMessage& message ) const;
   DialogTracker* getDialogTrackerForTag( const UtlString& tag ) const;
   void addDialogTrackerToList( const UtlString& tag, DialogTracker* pNewDialogTracker );
   bool removeDialogTrackerFromListAndDelete( const UtlString& tag );
   DialogTracker* allocateNewDialogTrackerBasedOnReference( const UtlString& discriminatingTag );
   ssize_t deleteDialogTrackersReadyForDeletion( void ); // returns number deleted.

   typedef enum
   {
      SDP_TOWARDS_CALLER,
      SDP_TOWARDS_CALLEE
   } SdpRecipient;

   struct CseqData
   {
      CseqData();
      CseqData( const SipMessage& message );
      bool operator==( const CseqData& rhs );
      void setValue( const SipMessage& message );
      UtlString mMethod;
      int       mSeqNum;
   };

   UtlString mSystemIdentificationString;
   CseqData mDialogFormingInviteCseq;
   UtlString mDialogOriginalFromTag;
   DialogTracker*  mpReferenceDialogTracker;
   EndpointDescriptor* mpCaller;
   EndpointDescriptor* mpCallee;
   const NatTraversalRules* mpNatTraversalRules;
   UtlString mHandle;
   MediaRelay* mpMediaRelay;
   UtlHashMap mDialogTrackersMap;
   CallTrackerInterfaceForSessionContext* mpOwningCallTracker;
   vector<UtlString> mListOfDialogTrackersReadyForDeletion;

   friend class SessionContextTest;
};

#endif // _SESSIONCONTEXT_H_
