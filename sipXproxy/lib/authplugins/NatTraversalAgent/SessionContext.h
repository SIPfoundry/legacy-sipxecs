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
#include "sipdb/RegDB.h"

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
   virtual bool doesEndpointsLocationImposeMediaRelay( const SipMessage& request ) const = 0;

   /**
    * Used to notify the owning SessionContext that the dialog a DialogTracker
    * non longer needs to be tracked and that the DialogTracker object is redy for
    * deletion.
    * \param [in] handleOfRequestingDialogContext - what it says
    */
   virtual void reportDialogTrackerReadyForDeletion( const UtlString& handleOfRequestingDialogContext ) = 0;

   /**
    * Computes the media relay IP address to use when patching SDP originating from endpoint
    * designated by endpointRole param
    * \param [out] mediaRelayAddressToUse - will contain the Symmitron address to use when patching SDP
    * \param [in] endpointRole - role of endpoint making the request.
    * \return true if media relay's native IP address is to be used. false if media relay's public IP address is to be used
    */
   virtual bool getMediaRelayAddressToUseInSdp( UtlString& mediaRelayAddressToUse,
                                                EndpointRole endpointRole ) const = 0;

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
                   RegDB* pRegDB,
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
   bool handleRequest ( SipMessage& message,
                        const char* address,
                        int port,
                        bool bFromCallerToCallee,
                        bool* reevaluateDestination );
   void handleResponse( SipMessage& message, const char* address, int port );
   void handleCleanUpTimerTick( void );

   // /////////////////////////////////////////// //
   // SessionContextInterfaceForDialogTracker     //
   // /////////////////////////////////////////// //
   //                 --Note--                    //
   // See SessionContextInterfaceForDialogTracker //
   // for method comments.                        //
   // /////////////////////////////////////////// //
   virtual bool doesEndpointsLocationImposeMediaRelay( const SipMessage& request ) const;
   virtual bool getMediaRelayAddressToUseInSdp( UtlString& mediaRelayAddressToUse, EndpointRole endpointRole ) const;
   virtual void reportDialogTrackerReadyForDeletion( const UtlString& handleOfRequestingDialogContext );

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
                                                              RegDB* regDb );

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
