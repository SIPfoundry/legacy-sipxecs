//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _TaoProviderListener_h_
#define _TaoProviderListener_h_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "tao/TaoDefs.h"
//#include "ptapi/PtTerminalConnectionListener.h"
#include "ptapi/PtProviderListener.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class TaoTransportTask;
class OsConnectionSocket;

class TaoProviderListener : public PtProviderListener
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

        TaoProviderListener(TaoObjHandle objId,
                                                                 TaoObjHandle clientSocket,
                                                                 TaoTransportTask* pSvrTransport,
                                                                 const char * terminalName);

        TaoProviderListener(PtEventMask* pMask = NULL);
    //:Default constructor
    //!param: (in) pMask - Event mask defining events the listener is interested
        // in.  This must be a subset of the events that the listener supports.  The
    // mask may be NULL where it is assumed that all events applicable to the
    // derived listener are of interest.

   TaoProviderListener(const TaoProviderListener& rTaoProviderListener);
     //:Copy constructor

   virtual
   ~TaoProviderListener();
     //:Destructor

   TaoProviderListener& operator=(const TaoProviderListener& rhs);
     //:Assignment operator
/* ============================ MANIPULATORS ============================== */

   void providerEventTransmissionEnded(const PtProviderEvent& rEvent);
     //:Method invoked on listener for event id =
     //:PROVIDER_EVENT_TRANSMISSION_ENDED
     //:indicating that the application will no longer receive provider
     //:events on this instance of the TaoProviderListener.
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
        int                                             mObjId;
        char*                                   mTerminalName;

        TaoObjHandle                    mhClientSocket;
        TaoTransportTask*               mpSvrTransport;
        OsConnectionSocket*             mpConnectionSocket;



};

#endif // _TaoProviderListener_h_
