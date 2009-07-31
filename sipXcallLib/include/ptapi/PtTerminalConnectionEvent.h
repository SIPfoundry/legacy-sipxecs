//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _PtTerminalConnectionEvent_h_
#define _PtTerminalConnectionEvent_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "ptapi/PtDefs.h"
#include "ptapi/PtTerminalConnection.h"
#include "ptapi/PtCallEvent.h"
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class PtCallEvent;
class PtTerminalConnection;
class TaoClientTask;

//:PtTerminalConnectionEvent contains PtTerminalConnection-associated
//:event data

class PtTerminalConnectionEvent : public PtCallEvent
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

    PtTerminalConnectionEvent(PtEvent::PtEventId eventId,
                                                        int metaCode,
                                                        const char* terminalName,
                                                        const char* callId,
                                                        int sipResponseCode,
                                                        const char* sipResponseText,
                                                        const char* newCallId = 0,
                                                        const char** oldCallIds = 0,
                                                        int numOldCalls = 0,
                                                        const char* address = 0,
                                                        PtEvent::PtEventCause cause = PtEvent::CAUSE_NORMAL,
                                                        int isLocal = -1,
                                                        TaoClientTask *pClient = NULL);


        PtTerminalConnectionEvent(PtEvent::PtEventId eventId = PtEvent::EVENT_INVALID);
     //:Default constructor

        PtTerminalConnectionEvent(TaoClientTask *pClient);
        //: Constructor.

   PtTerminalConnectionEvent(const PtTerminalConnectionEvent& rPtTerminalConnectionEvent);
     //:Copy constructor

   virtual
   ~PtTerminalConnectionEvent();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   PtTerminalConnectionEvent& operator=(const PtTerminalConnectionEvent& rhs);
     //:Assignment operator

   virtual void setEventAddress(const char* address);           // address
   virtual void setEventTerminal(const char* terminal);         // terminal
   virtual void setEventTcLocal(int islocal);                           // isLocal?
/* ============================ ACCESSORS ================================= */

   PtStatus getTerminalConnection(PtTerminalConnection& rTerminalConnection);
     //:Returns the terminal connection object associated with this event.
     //!param: (out) rTerminalConnection - The reference to the terminal connection
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
        char mAddress[128];
        char mTerminalName[128];
        int       mIsTermConnLocal;
        UtlBoolean      mbGotTerminalConnection;
        PtTerminalConnection mTerminalConnection;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:


};

/* ============================ INLINE METHODS ============================ */

#endif  // _PtTerminalConnectionEvent_h_
