//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _PtTerminalEvent_h_
#define _PtTerminalEvent_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "ptapi/PtEvent.h"
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS

// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
class PtTerminal;
class TaoClientTask;

//:PtTerminalEvent contains PtTerminal-associated event data
class PtTerminalEvent : public PtEvent
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   PtTerminalEvent(PtEvent::PtEventId eventId = PtEvent::EVENT_INVALID);
     //:Default constructor

   PtTerminalEvent(const PtTerminalEvent& rPtTerminalEvent);
     //:Copy constructor

   virtual
   ~PtTerminalEvent();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   PtTerminalEvent& operator=(const PtTerminalEvent& rhs);
     //:Assignment operator

/* ============================ ACCESSORS ================================= */
   PtStatus getTerminal(PtTerminal& rTerminal);
     //:Returns the terminal object associated with this event.
     //!param: (out) rTerminal - The reference to the terminal
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

        PtTerminalEvent(PtEvent::PtEventId eventId, const char* terminalName, TaoClientTask *pClient);

        TaoClientTask *mpClient;
    char* mpTerminalName;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:


};

/* ============================ INLINE METHODS ============================ */

#endif  // _PtTerminalEvent_h_
