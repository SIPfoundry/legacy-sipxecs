//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _PtProviderListener_h_
#define _PtProviderListener_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "ptapi/PtEventListener.h"
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class PtEventListener;
class PtProviderEvent;
class PtAddressEvent;
class PtTerminalEvent;

//:The PtProviderListener is used to register with and receive events from
//:PtProvider objects.

class PtProviderListener : public PtEventListener
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   PtProviderListener(PtEventMask* pMask = NULL);
    //:Default constructor
    //!param: (in) pMask - Event mask defining events the listener is interested
        // in.  This must be a subset of the events that the listener supports.  The
    // mask may be NULL where it is assumed that all events applicable to the
    // derived listener are of interest.

   PtProviderListener(const PtProviderListener& rPtProviderListener);
     //:Copy constructor

   virtual
   ~PtProviderListener();
     //:Destructor

   PtProviderListener& operator=(const PtProviderListener& rhs);
     //:Assignment operator
/* ============================ MANIPULATORS ============================== */

   void providerEventTransmissionEnded(const PtProviderEvent& rEvent);
     //:Method invoked on listener for event id =
     //:PROVIDER_EVENT_TRANSMISSION_ENDED
     //:indicating that the application will no longer receive provider
     //:events on this instance of the PtProviderListener.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

   void providerInService(const PtProviderEvent& rEvent);
     //:Method invoked on listener for event id =
     //:PROVIDER_IN_SERVICE
     //:indicating that the state of the PtProvider object has changed to
     //:PtProvider::IN_SERVICE.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

   void providerOutOfService(const PtProviderEvent& rEvent);
     //:Method invoked on listener for event id =
     //:PROVIDER_OUT_OF_SERVICE
     //:indicating that the state of the Provider object has changed to
     //:PtProvider::OUT_OF_SERVICE.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

   void providerShutdown(const PtProviderEvent& rEvent);
     //:Method invoked on listener for event id =
     //:PROVIDER_SHUTDOWN
     //:indicating that the state of the PtProvider object has changed to
     //:PtProvider::SHUTDOWN.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

   void providerAddressAdded(const PtAddressEvent& rEvent);
     //:Method invoked on listener for event id =
     //:PROVIDER_ADDRESS_ADDED
     //:indicating that a new PtAddress has been added to the provider.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

   void providerAddressRemoved(const PtAddressEvent& rEvent);
     //:Method invoked on listener for event id =
     //:PROVIDER_ADDRESS_REMOVED
     //:indicating that a PtAddress has been removed from the provider.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

   void providerTerminalAdded(const PtTerminalEvent& rEvent);
     //:Method invoked on listener for event id =
     //:PROVIDER_TERMINAL_ADDED
     //:indicating that a new PtTerminal has been added to the provider.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

   void providerTerminalRemoved(const PtTerminalEvent& rEvent);
     //:Method invoked on listener for event id =
     //:PROVIDER_TERMINAL_REMOVED
     //:indicating that a PtTerminal has been removed from the provider.
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


};

/* ============================ INLINE METHODS ============================ */

#endif  // _PtProviderListener_h_
