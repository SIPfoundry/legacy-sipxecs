//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _PtCall_h_
#define _PtCall_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <ptapi/PtDefs.h>
#include "os/OsBSem.h"
#include "os/OsProtectEventMgr.h"
#include "cp/CpCallManager.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class PtAddress;
class PtCallListener;
class PtConnection;
class PtProvider;
class PtSessionDesc;
class PtTerminal;
class PtTerminalConnection;
class TaoClientTask;
class TaoServerTask;
class TaoReference;
class TaoObjectMap;

//:A PtCall object models a telephone call.
// A call can have zero or more connections. A two-party call has two
// connections, and a conference call has three or more connections.
// Each connection models the relationship between a call and an address,
// where an address identifies a particular party or set of parties on a
// call.
//
// <H3>Creating PtCall Objects</H3>
// Applications create instances of a PtCall object with the
// PtProvider.createCall() method, which returns a pointer to a PtCall
// object that has zero PtConnections and is in the PtCall::IDLE state.
// The PtCall maintains a reference to its provider for the life of that
// PtCall object. The PtProvider associated with a PtCall is obtained
// via the PtCall.getProvider() method.
//
// <H3>Call States</H3>
// A PtCall has a state that is obtained via the PtCall.getState() method.
// This state describes the current progress of a telephone call, where it is
// in its life cycle, and how many connections exist on the call. The PtCall
// state may be one of three values: PtCall::IDLE, PtCall::ACTIVE, or
// PtCall::INVALID. A description of each state follows.<br>
// <br>
// <dl>
// <dt><b>PtCall::IDLE</b></dt>
// <dd>This is the initial state for all calls. In this state, the call has
// zero connections; that is, PtCall.getConnections() must return an empty
// array.</dd>
// <br>
// <dt><b>PtCall::ACTIVE</b></dt>
// <dd>A call with some current ongoing activity is in this state. Calls with
// one or more associated connections must be in this state. If a call is
// in this state, the PtCall.getConnections() method must return an array
// containing at least one item.</dd>
// <br>
// <dt><b>PtCall::INVALID</b></dt>
// <dd>This is the final state for all calls. PtCall objects that lose all of
// their PtConnection objects (via a transition of the PtConnection object
// into the PtConnection::DISCONNECTED state) move into this state. Calls in
// this state have zero connections and these PtCall objects may not be used
// for any future action. In this state, the PtCall.getConnections() method
// must return an empty array. </dd>
// </dl>
// <H3>Calls and Connections</H3>
// A PtCall maintains a list of the PtConnections on that call. Applications
// obtain an array of PtConnection pointers associated with the call via the
// PtCall.getConnections() method. A PtCall retains a reference to a
// PtConnection only if it is not in the PtConnection::DISCONNECTED state.
// Therefore, if a PtCall has a reference to a PtConnection, then that
// PtConnection must not be in the PtConnection::DISCONNECTED state. When a
// connection moves into the PtConnection::DISCONNECTED state (e.g., when a
// party hangs up), the PtCall loses its reference to that PtConnection, which
// is then no longer reported via the PtCall.getConnections() method.
//
// <H3>Call Information methods</H3>
// The PtCall class provides methods that return the calling address, calling
// terminal, called address, and last redirected address information.  These
// methods only make sense for a two-party call.<br> <br>
// <ul>
// <li>The <b>calling address</b>, as returned by the PtCall.getCallingAddress()
// method, is the PtAddress that originally placed the PtCall. </li><br>
// <br>
// <li>The <b>calling terminal</b>, as returned by the PtCall.getCallingTerminal()
// method, is the PtTerminal which originally placed the PtCall. </li><br>
// <br>
// <li>The <b>called address</b>, as returned by the PtCall.getCalledAddress()
// method, is the PtAddress to which the PtCall was originally placed. </li><br>
// <br>
// <li>The <b>last redirected address</b>, as returned by the
// PtCall.getLastRedirectedAddress() method, is the PtAddress to which this
// PtCall was placed before the current destination address. For example, if
// a PtCall was forwarded from one PtAddress to another, then the first
// PtAddress is the last redirected PtAddress for this call.</li></ul>
// <p>
// Each of these methods returns NULL if their values are unknown at the
// present time. During the lifetime of a PtCall, an implementation may
// learn this additional information and, as a result, return different values
// for some or all of these methods.
//
// <H3>Conferencing Telephone Calls</H3>
// The conferencing feature supported by this interface permits two telephone
// calls to be "merged". That is, two PtCalls are merged into a single
// PtCall with the union of all of the participants of the two PtCalls being
// placed on the single PtCall.
// <p>
// Applications invoke the PtCall.conference() method to perform the
// conferencing feature. This method is given the "second" PtCall as an
// argument. All participants are moved from the second PtCall to the PtCall
// on which the method is invoked.  The second PtCall moves into the
// PtCall::INVALID state as a result.
// <p>
// In order for the conferencing feature to happen, there must be a common
// participant in both PtCalls, as represented by a single PtTerminal and
// two PtTerminalConnections, one on each of the two PtCalls. These two
// PtTerminalConnections are known as the conference controllers. In the
// real world, one of the two telephone calls must be on hold with respect
// to the controlling PtTerminal, and hence, the PtTerminalConnection on
// the second PtCall must be in the PtTerminalConnection::HELD state. The
// two conference controlling PtTerminalConnections are merged into one as
// a result of this method.
// <p>
// Applications may control which PtTerminalConnection acts as the
// conference controller via the PtCall.setConferenceController() method.
// The PtCall.getConferenceController() method returns the current
// conference controller, or NULL if there is none. If no conference
// controller is set, the implementation chooses a suitable
// PtTerminalConnection when the conferencing feature is invoked.
//
// <H3>Transferring Telephone Calls</H3>
// The transfer feature supported by this interface permits one PtCall to
// be "moved" to another PtCall. That is, all of the participants from
// one PtCall are moved to another PtCall, except for the transferring
// participant which drops off from both PtCalls.
// <p>
// Applications invoke the PtCall.transfer() method to perform the transfer
// feature. There are two overloaded versions of this method: <br>
// <ul>
// <li>The first
// method takes a second PtCall as an argument. This method acts similarly
// to PtCall.conference(), except the two PtTerminalConnections on each
// PtCall with a common PtTerminal are removed from both PtCalls. </li>
// <br>
// <li>The second
// version takes a telephone address URL as an argument. This method
// removes the transfer controller participant while placing the telephone
// call to the designated address. This version of the transfer
// feature is often known as a single-step transfer. </li>
// </ul>
// <p>
// In order for the transfer feature to happen, there must be a participant
// which acts as the transfer controller. The transfer controller is a
// PtTerminalConnection around which the transfer is performed. In the first
// version of the PtCall.transfer() method, the transfer controller must be
// present on each of the two PtCalls and share a common PtTerminal. In the
// second version, the transfer controller only applies to the PtCall object
// on which the method is invoked (since there is no second PtCall involved).
// In both cases, the transfer controller participant is no longer part of
// any PtCall once the transfer feature is complete.
// <p>
// Applications may control which PtTerminalConnection acts as the transfer
// controller via the PtCall.setTransferController() method. The
// PtCall.getTransferController() method returns the current transfer
// controller, or NULL if there is none. If no transfer controller is set,
// the implementation chooses a suitable PtTerminalConnection when the
// conferencing feature is invoked.
//
// <H3>Consultation Calls</H3>
// Consultation calls are special types of telephony calls created (often
// temporarily) for a specific purpose. Consultation calls can be created if
// a user wants to "consult" with another party briefly while currently on
// a PtCall, or for the purpose of conferencing or transferring
// with a PtCall. Consequently, consultation calls are always associated
// with another existing PtCall.
// <p>
// Applications invoke the PtCall.consult() method to perform the
// consultation feature. The instance on which the method is invoked is
// always the "idle" PtCall on which the consultation takes place.
// This method takes a
// PtTerminalConnection and a string telephone address URL as arguments.
// This consultation telephone call is associated with the PtCall of the
// PtTerminalConnection argument. This method places a telephone call from
// the same originating endpoint specified by the PtTerminalConnection
// argument to the designated telephone address.
// <h3>
// Additional Methods</h3>
// The PtCall.addParty() method adds a single party to a PtCall given some
// telephone address URL. The PtCall.drop() method then disconnects all parties
// from the PtCall, and moves it into the PtCall::INVALID state.
//
// <H3>Listeners and Events</H3>
// All events pertaining to a PtCall object are reported via
// PtCallListener objects. Applications instantiate an object that is
// derived from the PtCallListener class and use the PtCall.addCallListener()
// method to begin the delivery of events.
// <p>
// PtConnection-related and PtTerminalConnection-related events are also
// reported via the PtCallListener objects. These events include the creation
// of these objects and their state changes. Events that are reported by
// PtCallListener objects that pertain to PtConnections and
// PtTerminalConnections are instances of (or descendants of) the
// PtConnectionEvent class and the PtTerminalConnectionEvent class,
// respectively.
// <p>
// An event is delivered to the application whenever the state of the PtCall
// changes. The events corresponding to PtCall state changes are
// CALL_ACTIVE and CALL_INVALID.
//
// <H4>When Call Event Transmission Ends</H4>
// Applications receive events on a listener until the listener is removed
// or until the PtCall object is no longer observable. In these instances,
// each listener receives a CALL_EVENT_TRANSMISSION_ENDED event as its final
// event.
//
// <H4>Event Granularity</H4>
// An application may control the granularity of the events that are reported.
// Registering for the highest level (PtCallListener) will direct the
// implementation to send only PtCall-related events (including meta events).
// Registering as a listener at a lower level (PtConnectionListener or
// PtTerminalConnectionListener) directs the implementation to provide
// successively more detailed events.
//
// <H4>Registering Call Listeners via Address and Terminal</H4>
// Applications may also receive events about a call by adding a listener via
// the PtAddress or PtTerminal objects; this is done with the
// PtAddress.addCallListener()
// and PtTerminal.addCallListener() (and related) methods. These methods
// provide the ability for an application to receive call-related events when
// a PtCall contains a particular PtAddress or PtTerminal. In particular,
// methods exist to add a PtCallListener, PtConnectionListener and
// PtTerminalConnectionListener via PtAddress and PtTerminal.
// See the specifications for the PtAddress and PtTerminal classes for more
// details.

class PtCall
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum CallState
   {
      IDLE,
      ACTIVE,
      INVALID
   };

/* ============================ CREATORS ================================== */
        PtCall();
     //:Default constructor

        PtCall(TaoClientTask *pClient, const char* callId);

        PtCall(const char* callId);

        PtCall(const PtCall& rPtCall);
     //:Copy constructor (not implemented for this class)

      virtual
   ~PtCall();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   PtCall& operator=(const PtCall& rhs);
     //:Assignment operator (not implemented for this class)

   virtual PtStatus addCallListener(PtCallListener& rCallListener);
     //:Adds a listener to this call.
     //!param: (in) rCallListener - The listener to add to this call
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_EXISTS - <i>rCallListener</i> is already registered
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus addParty(const char* newPartyURL,
                     PtSessionDesc* pSessionDesc,
                     PtConnection& rConnection);
     //:Adds an additional party to an existing call.
     // This is sometimes called a "single-step conference" because a party
     // is conferenced directly into the call. The telephone address
     // provided as the <i>newPartyURL</i> argument must be valid.
     //!param: (in) newPartyURL - Address of the party to add to this call
     //!param: (in) pSessionDesc - Pointer to the requested attributes for the new connection or NULL to use defaults
     //!param: (out) rpConnection - Set to point to the PtConnection associated with the new party
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_INVALID_PARTY - Invalid <i>newPartyURL</i>
     //!retcode: PT_INVALID_STATE - The call is either not ACTIVE or has fewer than two ESTABLISHED PtConnections
     //!retcode: PT_RESOURCE_UNAVAILABLE - Insufficient resources
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus conference(PtCall& rOtherCall);
     //:Merges two PtCalls together, resulting in the union of the
     //:participants of both PtCalls being placed on a single PtCall.
     // If successful, all of the participants from <i>rOtherCall</i> will
     // be moved to the PtCall on which this method is invoked.
     //!param: (in) rOtherCall - The PtCall object whose participants will be moved to this PtCall
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_INVALID_ARGUMENT - The <i>rOtherCall</i> object is not valid for the conference
     //!retcode: PT_INVALID_STATE - The call is either not ACTIVE or the conference controllers are not in the proper state
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus connect(PtTerminal& rTerminal, PtAddress& rAddress,
                    const char* destinationURL, PtSessionDesc* pSessionDesc);
     //:Places a telephone call from an originating endpoint to a destination
     //:address URL.
     // The <i>pSessionDesc</i> argument points to an object containing the
     // attributes requested for the connection.
     //!param: (in) rTerminal - The originating terminal
     //!param: (in) rAddress - The originating address
     //!param: (in) destinationURL - The intended destination for the call
     //!param: (in) pSessionDesc - Pointer to the requested attributes for the new connection or NULL to use defaults
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_INVALID_ARGUMENT - Bad <i>rTerminal</i>, <i>rAddress</i> or <i>rSessionDesc</i> argument
     //!retcode: PT_INVALID_PARTY - Invalid <i>destinationURL</i>
     //!retcode: PT_RESOURCE_UNAVAILABLE - Insufficient resources
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus consult(PtTerminalConnection& rTerminalConnection,
                    const char* destinationURL,
                    PtSessionDesc* pSessionDesc,
                    PtConnection& rSrcConnection,
                    PtConnection& rDstConnection);
     //:Creates a new call between the PtTerminal associated with
     //:<i>rTerminalConnection</i> and the destination indicated by
     //:<i>destinationURL</i>.
     //!param: (in) rTerminalConnection - The terminal connection used to obtain the originating terminal
     //!param: (in) destinationURL - The intended destination for the new call
     //!param: (in) pSessionDesc - Pointer to the requested attributes for the new connection or NULL to use defaults
     //!param: (out) rpSrcConnection - Set to point to the PtConnection for the origination of the call
     //!param: (out) rpDstConnection - Set to point to the PtConnection for the termination of the call
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_INVALID_PARTY - Invalid <i>destinationURL</i>
     //!retcode: PT_INVALID_STATE - The call associated with <i>rTerminalConnection</i> is not ACTIVE
     //!retcode: PT_RESOURCE_UNAVAILABLE - Insufficient resources
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus drop(void);
     //:Drops the entire call.
     // This method is equivalent to using the PtConnection.disconnect()
     // method on each PtConnection which is part of the PtCall. Typically,
     // each PtConnection on the PtCall will move into the
     // PtConnection::DISCONNECTED state, each PtTerminalConnection will
     // move into the PtTerminalConnection::DROPPED state, and the PtCall
     // will move into the PtCall::INVALID state.
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus removeCallListener(PtCallListener& rCallListener);
     //:Removes the indicated listener from this call.
     //!param: (in) rCallListener - The listener to remove from this call
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_NOT_FOUND - <i>rCallListener</i> not registered
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus setConferenceController(PtTerminalConnection* pController);
     //:Sets the PtTerminalConnection that acts as the conference controller
     //:for this call.
     // The conference controller represents the participant in the call
     // around which a conference takes place.
     //!param: pController - The terminal connection to use as a conference controller for this call
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_INVALID_ARGUMENT - The indicated terminal connection is not part of this call
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus setTransferController(PtTerminalConnection* pController);
     //:Sets the PtTerminalConnection that acts as the transfer controller
     //:for this call.
     // The transfer controller represents the participant in the call
     // around which a transfer takes place.
     //!param: pController - The terminal connection to use as a transfer controller for this call
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_INVALID_ARGUMENT - The indicated terminal connection is not part of this call
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus transfer(PtCall& rOtherCall);
     //:Moves all participants from one PtCall to another, with the
     //:exception of a selected common participant.
     //!param: (in) rOtherCall - The PtCall object whose participants will be moved to this PtCall
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_INVALID_ARGUMENT - The <i>rOtherCall</i> object is not valid for the transfer
     //!retcode: PT_INVALID_STATE - The call is either not ACTIVE or the conference controllers are not in the proper state
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus transfer(const char* destinationURL,
                     PtSessionDesc* pSessionDesc,
                     PtConnection& rNewConnection,
                                         int transferType = CpCallManager::CP_BLIND_TRANSFER);
     //:Transfers all participants currently on this PtCall, with the
     //:exception of the transfer controller participant, to the address
     //:indicated by the destination URL.
     //!param: (in) destinationURL - The intended destination for the new call
     //!param: (in) pSessionDesc - Pointer to the requested attributes for the new call or NULL to use defaults
     //!param: (out) rNewConnection - Set to point to the PtConnection that the participants have been moved to
     //!param: (in) transferType - CP_BLIND_TRANSFER or CP_SINGLE_CALL_TRANSFER
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_INVALID_PARTY - Invalid <i>destinationURL</i>
     //!retcode: PT_RESOURCE_UNAVAILABLE - Insufficient resources
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available


   virtual PtStatus hold(UtlBoolean bBridgeParticipants = TRUE) ;
     //:Places the call on hold and optionally allows call participants to
     //:continue speaking among other participants (bridged) or participants
     //:are requested to not transmit date (far end hold).
     //
     //!param (in): bBridgeParticipants - Controls whether participants are
     //             placed on local hold or far end hold.  If TRUE, local
     //             hold is selected and participants are bridged and able
     //             to communicate with each other.  If FALSE, all parties
     //             are muted and not sent audio data.
     //
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_BUSY - Unable to communicate with call processing

   virtual PtStatus unhold(UtlBoolean bRemoteParticipants = TRUE) ;
     //:Places the call into focus and takes any held connections off hold.
     //:Optionally remote participants can be taken off hold or left in
     //:their present state.
     //!param (in) bRemoteParticipants - Controls whether all particpants
     //            are taken off hold or not.  If TRUE, all participants
     //            are taken off hold.  If FALSE, only the local connection
     //            is taken off hold  (placed into focus).
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_BUSY - Unable to communicate with call processing

   virtual PtStatus setCodecCPULimit(const int limit,
                                     const UtlBoolean bAutoRenegotiate = TRUE);
      //:Sets the CPU codec limit for this call.
      //!param (in) limit - The codec/CPU limit for this call.  The value can
      //       be set to LOW (0) or HIGH (1).  If set to LOW, only LOW CPU
      //       intensive codecs are allowed.  If set to HIGH, both LOW and
      //       HIGH CPU intensive codes are allowed.
      //!param (in) bAutoRenegotiate - Defines if call call processiong should
      //       automatically force a renegotation of codecs to match the
      //       specified level.
      //!retcode: PT_SUCCESS - Success
      //!retcode: PT_BUSY - Unable to communicate with call processing


   virtual PtStatus forceCodecRenegotiation() ;
      //:Forces the renegotation of all connections for this call.
      //!retcode: PT_SUCCESS - Success
      //!retcode: PT_BUSY - Unable to communicate with call processing



/* ============================ ACCESSORS ================================= */

   virtual PtStatus getCallListeners(PtCallListener* callListeners[], int size,
                             int& rNumItems);
     //:Returns an array of PtCallListener pointers for all of the call
     //:listeners presently associated with this call.
     //!param: (out) callListeners - The array of pointers to known call listeners
     //!param: (in) size - The number of elements in the <i>callListeners</i> array
     //!param: (out) rNumItems - The number of items assigned
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_MORE_DATA - There are more than <i>size</i> listeners
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus getCalledAddress(PtAddress& rAddress);
     //:Sets <i>rpAddress</i> to point to the called PtAddress associated
     //:with this call.  This method should only be used for two-party calls.
     // The called PtAddress is the PtAddress to which the call was
     // originally placed.  If the called address is unknown or not yet known,
     // <i>rpAddress</i> will be set to NULL.
     //!param: (out) rAddress - Set to point to the called PtAddress for this call
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus getCallingAddress(PtAddress& rAddress);
     //:Sets <i>rpAddress</i> to point to the calling PtAddress associated
     //:with this call.  This method should only be used for two-party calls.
     // The calling PtAddress is the PtAddress which originally placed the
     // telephone call.  If the calling address is unknown or not yet known,
     // <i>rpAddress</i> will be set to NULL.
     //!param: (out) rAddress - Set to point to the calling PtAddress for this call
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus getCallingTerminal(PtTerminal& rTerminal);
     //:Sets <i>rpTerminal</i> to point to the calling PtTerminal associated
     //:with this call.  This method should only be used for two-party calls.
     // The calling PtTerminal is the PtTerminal which originally placed the
     // telephone call.  If the calling terminal is unknown or not yet known,
     // <i>rpTerminal</i> will be set to NULL.
     //!param: (out) rpTerminal - Set to point to the calling PtTerminal for this call
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus getConferenceController(PtTerminalConnection& rController);
     //:Sets <i>rpController</i> to point to the PtTerminalConnection which
     //:currently acts as the conference controller.
     // The conference controller represents the participant in the telephone
     // system around which a conference takes place.  When a PtCall is
     // initially created, it has no conference controller.  This method sets
     // <i>rpController</i> to non-NULL only if the application has
     // previously set the conference controller.  If the current conference
     // controller leaves the call, the call reverts to not having a
     // conference controller.
     //!param: (out) rpController - Set to point to the conference controller for this call
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus getConnections(PtConnection connections[], int size,
                           int& rNumItems);
     //:Returns an array of pointers to PtConnection objects currently
     //:associated with this call.
     // The caller provides an array that can hold up to <i>size</i>
     // PtConnection pointers.  This method will fill in the
     // <i>connections</i> array with up to <i>size</i> pointers.  The
     // actual number of pointers filled in is passed back via the
     // <i>nItems</i> argument.
     //!param: (out) connections - The array of PtConnection pointers
     //!param: (in) size - The number of elements in the <i>connections</i> array
     //!param: (out) rNumItems - The number of items assigned
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_MORE_DATA - There are more than <i>size</i> connections
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus getLastRedirectedAddress(PtAddress& rAddress);
     //:Sets <i>rpAddress</i> to point to the last redirected PtAddress
     //:associated with this call.  This method should only be used for
     //:two-party calls.
     // The last redirected PtAddress is the PtAddress at which this call
     // was placed immediately before the current PtAddress.  This address
     // is of interest when a PtCall is forwarded to a different PtAddress
     // before being answered.  If the last redirected address is unknown
     // or not yet known, <i>rpAddress</i> will be set to NULL.
     //!param: (out) rpAddress - Set to point to the last redirected PtAddress for this call
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus getProvider(PtProvider& rProvider);
     //:Returns a pointer to the PtProvider associated with this call.
     //!param: (out) rpProvider - a pointer to the PtProvider object associated with this call
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus getState(int& rState);
     //:Sets <i>rState</i> to the current state of the telephone call, either
     //:IDLE, ACTIVE or INVALID.
     //!retcode: IDLE - Initial state for a call
     //!retcode: ACTIVITY - Call with one or more connections
     //!retcode: INVALID - Call that is no longer valid (and has zero connections)

   PtStatus getCallId(char* callId, int len);
     //:Sets <i>callId</i> to the current call id.
     //!retcode: IDLE - Initial state for a call
     //!retcode: ACTIVITY - Call with one or more connections
     //!retcode: INVALID - Call that is no longer valid (and has zero connections)

   virtual PtStatus getTransferController(PtTerminalConnection& rController);
     //:Sets <i>rpController</i> to point to the PtTerminalConnection which
     //:currently acts as the transfer controller.
     // The conference controller represents the participant in the telephone
     // system around which a transfer takes place.  When a PtCall is
     // initially created, it has no transfer controller.  This method sets
     // <i>rpController</i> to non-NULL only if the application has
     // previously set the transfer controller.  If the current transfer
     // controller leaves the call, the call reverts to not having a
     // transfer controller.
     //!param: (out) rpController - Set to point to the transfer controller for this call
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus numConnections(int& rCount);
     //:Returns the number of connections associated with this call.
     //!param: (out) rCount - The number of connections associated with this call
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus numCallListeners(int& rCount);
     //:Returns the number of call listeners associated with this call.
     //!param: (out) rCount - The number of call listeners associated with this call
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus getCodecCPUCost(int& cost) ;
      //:Gets the current CPU cost given the negotiated codecs.
      //!param (out) cost - The cpu cost of the currently negotiated and
      //       advertised codes.   A value of zero indicates LOW and 1
      //       indicates HIGH.
      //!retcode: PT_SUCCESS - Success
      //!retcode: PT_BUSY - Unable to communicate with call processing

   virtual  PtStatus getCodecCPULimit(int& cost) ;
      //:Gets the max  CPU cost given the negotiated codecs.
      //!param (out) cost - The cpu cost of the currently negotiated and
      //       advertised codes.   A value of zero indicates LOW and 1
      //       indicates HIGH.
      //!retcode: PT_SUCCESS - Success
      //!retcode: PT_BUSY - Unable to communicate with call processing


/* ============================ INQUIRY =================================== */

friend class PtCallEvent;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
        UtlString       mCallId;
        char    mState;

    static OsBSem           semInit ;
      //: Binary Semaphore used to guard initialiation and tear down
        static TaoReference             *mpTransactionCnt;
        static int                              mRef;

        PtTerminalConnection* mpConfController;
        TaoClientTask   *mpClient;

        OsTime          mTimeOut;

        void    initialize();

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

        OsProtectEventMgr *mpEventMgr;

};

/* ============================ INLINE METHODS ============================ */

#endif  // _PtCall_h_
