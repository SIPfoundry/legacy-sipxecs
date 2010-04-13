//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _PtAddressListener_h_
#define _PtAddressListener_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:The PtAddressListener is used to register with and receive events from
//:PtAddress objects

class PtAddressListener : public PtEventListener
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   PtAddressListener(PtEventMask* pMask = NULL);
     //:Default constructor
     //!param: (in) pMask - Event mask defining events the listener is interested in.  This must be a subset of the events that the listener supports.  The mask may be NULL where it is assumed that all events applicable to the derived listener are of interest.

   virtual
   ~PtAddressListener();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

  void addressEventTransmissionEnded(const PtAddressEvent& rEvent);
     //:Method invoked on listener for event id =
     //:ADDRESS_EVENT_TRANSMISSION_ENDED
     //:indicating that the application will no longer receive address
     //:events on this instance of the PtAddressListener.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  void addressDoNotDisturbEnabled(const PtAddressEvent& rEvent);
     //:Method invoked on listener for event id =
     //:ADDRESS_DO_NOT_DISTURB_ENABLED
     //:indicating the state of the <i>do-not-disturb</i> feature has changed
     //:to enabled for the PtAddress.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.


  void addressDoNotDisturbDisabled(const PtAddressEvent& rEvent);
     //:Method invoked on listener for event id =
     //:ADDRESS_DO_NOT_DISTURB_DISABLED
     //:indicating the state of the <i>do-not-disturb</i> feature has changed
     //:to disabled for the PtAddress.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  void addressForwardingChanged(const PtAddressEvent& rEvent);
     //:Method invoked on listener for event id =
     //:ADDRESS_FORWARDING_CHANGED
     //:indicating the state of the forward feature has changed for the
     //:PtAddress.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  void addressMessagesWaiting(const PtAddressEvent& rEvent);
     //:Method invoked on listener for event id =
     //:ADDRESS_MESSAGES_WAITING
     //:indicating the state of the message waiting feature has changed to
     //:messages waiting for the PtAddress.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  void addressNoMessagesWaiting(const PtAddressEvent& rEvent);
     //:Method invoked on listener for event id =
     //:ADDRESS_NO_MESSAGES_WAITING
     //:indicating the state of the message waiting feature has changed to
     //:no messages waiting for the PtAddress.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   PtAddressListener(const PtAddressListener& rPtAddressListener);
     //:Copy constructor

   PtAddressListener& operator=(const PtAddressListener& rhs);
     //:Assignment operator
};

/* ============================ INLINE METHODS ============================ */

#endif  // _PtAddressListener_h_
