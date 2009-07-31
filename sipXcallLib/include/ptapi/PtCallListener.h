//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _PtCallListener_h_
#define _PtCallListener_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <ptapi/PtEventListener.h>
#include <ptapi/PtMultiCallMetaEvent.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class PtCallEvent;

//:The PtCallListener is used to register with PtAddress, PtTerminal, and
//:PtCall objects to receive events from associated PtCall objects.

class PtCallListener : public PtEventListener
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   PT_CLASS_INFO_MEMBERS

/* ============================ CREATORS ================================== */

   PtCallListener(PtEventMask* pMask = NULL);
     //:Default constructor
     //!param: (in) pMask - Event mask defining events the listener is interested in.  This must be a subset of the events that the listener supports.  The mask may be NULL where it is assumed that all events applicable to the derived listener are of interest.

   virtual
   ~PtCallListener();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

  virtual void callEventTransmissionEnded(const PtCallEvent& rEvent);
     //:Method invoked on listener for event id =
     //:CALL_EVENT_TRANSMISSION_ENDED
     //:indicating that the application will no longer receive call events on
     //:this instance of the PtCallListener.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void callActive(const PtCallEvent& rEvent);
     //:Method invoked on listener for event id =
     //:CALL_ACTIVE
     //:indicating that the state of the call object has changed to
     //:PtCall::ACTIVE.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void callInvalid(const PtCallEvent& rEvent);
     //:Method invoked on listener for event id =
     //:CALL_INVALID
     //:indicating that the state of the call object has changed to
     //:PtCall::INVALID.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void callMetaCallStartStarted(const PtCallEvent& rEvent);
     //:Method invoked on listener for event id =
     //:CALL_META_CALL_STARTING_STARTED
     //:indicating that the starting of a new call.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void callMetaCallStartEnded(const PtCallEvent& rEvent);
     //:Method invoked on listener for event id =
     //:CALL_META_CALL_STARTING_ENDED
     //:indicating that the starting of a new call is completed.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void callMetaCallEndStarted(const PtCallEvent& rEvent);
     //:Method invoked on listener for event id =
     //:CALL_META_CALL_ENDING_STARTED
     //:indicating that the starting of dropping a call.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void callMetaCallEndEnded(const PtCallEvent& rEvent);
     //:Method invoked on listener for event id =
     //:CALL_META_CALL_ENDING_ENDED
     //:indicating that the droping of a call is completed.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void callMetaProgressStarted(const PtCallEvent& rEvent);
     //:Method invoked on listener for event id =
     //:CALL_META_PROGRESS_STARTED
     //:indicating that the current call in the telephony platform has changed
     //:state, and events will follow which indicate the changes to this call.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void callMetaProgressEnded(const PtCallEvent& rEvent);
     //:Method invoked on listener for event id =
     //:CALL_META_PROGRESS_ENDED
     //:indicating that the current call in the telephony platform has changed
     //:state, and all the events that were associated with that change have
     //:now been reported.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void callMetaSnapshotStarted(const PtCallEvent& rEvent);
     //:Method invoked on listener for event id =
     //:CALL_META_SNAPSHOT_STARTED
     //:indicating that the Pingtel implementation is reporting to the
     //:application the current state of the call on the associated telephony
     //:platform, by reporting a set of simulated state changes that, in
     //:effect, construct the current state of the call.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void callMetaSnapshotEnded(const PtCallEvent& rEvent);
     //:Method invoked on listener for event id =
     //:CALL_META_SNAPSHOT_ENDED
     //:indicating that the Pingtel implementation has finished reporting a
     //:set of simulated state changes that, in effect, construct the current
     //:state of the call.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void callMetaAddPartyStarted(const PtCallEvent& rEvent);
     //:Method invoked on listener for event id = CALL_META_ADD_PARTY_STARTED
     //:indicating that a party has been added to the call. A "party"
     //:corresponds to a PtConnection being added. Note that if a
     //:PtTerminalConnection is added, it carries a meta event of
     //:CALL_META_PROGRESS_STARTED.
     // The event parameter is valid only within this method.
     // The implementation must copy the event if it is needed
     // beyond the scope of an invocation.  The implementation of
     // this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void callMetaAddPartyEnded(const PtCallEvent& rEvent);
     //:Method invoked on listener for event id =
     //:CALL_META_ADD_PARTY_ENDED
     //:indicates the end of the group of events related to the add party meta
     //:event.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void callMetaRemovePartyStarted(const PtCallEvent& rEvent);
     //:Method invoked on listener for event id =
     //:CALL_META_REMOVE_PARTY_STARTED
     //:indicating that a party (i.e. connection) has been removed from the
     //:call by moving into the PtConnection::DISCONNECTED state.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void callMetaRemovePartyEnded(const PtCallEvent& rEvent);
     //:Method invoked on listener for event id =
     //:CALL_META_REMOVE_PARTY_ENDED
     //:indicating the end of the group of events related to the remove party
     //:meta event.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void multicallMetaMergeStarted(const PtMultiCallMetaEvent& rEvent);
     //:Method invoked on listener for event id =
     //:MULTICALL_META_MERGE_STARTED
     //:indicating that calls are merging, and events will follow which
     //:indicate the changes to those calls.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void multicallMetaMergeEnded(const PtMultiCallMetaEvent& rEvent);
     //:Method invoked on listener for event id =
     //:MULTICALL_META_MERGE_ENDED
     //:indicating that calls have merged, and that all state change events
     //:resulting from this merge have been reported.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void multicallMetaTransferStarted(const PtMultiCallMetaEvent& rEvent);
     //:Method invoked on listener for event id =
     //:MULTICALL_META_TRANSFER_STARTED
     //:indicating that a transfer is occurring, and events will follow which
     //:indicate the changes to the affected calls.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void multicallMetaTransferEnded(const PtMultiCallMetaEvent& rEvent);
     //:Method invoked on listener for event id =
     //:MULTICALL_META_TRANSFER_ENDED
     //:indicating that a transfer has completed, and that all state change
     //:events resulting from this transfer have been reported.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

/* ============================ ACCESSORS ================================= */

   static const char* className();
     //:Returns the name of this class
     //!returns: Returns the string representation of the name of this class


   PtStatus getLocation(UtlString* rpLocation);

/* ============================ INQUIRY =================================== */

   virtual PtBoolean isClass(const char* pClassName);
     //:Determines if this object if of the specified type.
     //!param: (in) pClassName - the string to compare with the name of this class.
     //!retcode: TRUE - if the given string contains the class name of this class.
     //!retcode: FALSE - if the given string does not match that of this class

   virtual PtBoolean isInstanceOf(const char* pClassName);
     //:Determines if this object is either an instance of or is derived from
     //:the specified type.
     //!param: (in) pClassName - the string to compare with the name of this class.
     //!retcode: TRUE - if this object is either an instance of or is derived from the specified class.
     //!retcode: FALSE - if this object is not an instance of the specified class.

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

        UtlString* mpOriginatingIP;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   PtCallListener(const PtCallListener& rPtCallListener);
     //:Copy constructor

   PtCallListener& operator=(const PtCallListener& rhs);
     //:Assignment operator

};

/* ============================ INLINE METHODS ============================ */

#endif  // _PtCallListener_h_
