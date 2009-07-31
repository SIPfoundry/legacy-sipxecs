//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _PtConnectionEvent_h_
#define _PtConnectionEvent_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "ptapi/PtCallEvent.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class PtConnection;
class TaoClientTask;

//:PtConnectionEvent contains PtConnection-associated event data

class PtConnectionEvent : public PtCallEvent
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
        PtConnectionEvent(PtEvent::PtEventId eventId,
                                                        int metaCode,
                                                        const char* addressName,
                                                        const char* callId,
                                                        int sipResponseCode,
                                                        const char* sipResponseText,
                                                        const char* newCallId = 0,
                                                        const char** oldCallIds = 0,
                                                        int numOldCalls = 0,
                                                        PtEvent::PtEventCause cause = PtEvent::CAUSE_NORMAL,
                                                        TaoClientTask *pClient = NULL);

        PtConnectionEvent(TaoClientTask *pClient);


   PtConnectionEvent(const PtConnectionEvent& rPtConnectionEvent);
     //:Copy constructor

        PtConnectionEvent(PtEvent::PtEventId eventId = PtEvent::EVENT_INVALID);
     //:Default constructor

   virtual
   ~PtConnectionEvent();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   PtConnectionEvent& operator=(const PtConnectionEvent& rhs);
     //:Assignment operator

   virtual void setEventAddress(const char* address);

/* ============================ ACCESSORS ================================= */
   PtStatus getConnection(PtConnection& rConnection) const ;
     //:Returns the connection object associated with this event.
     //!param: (out) prConnection - The reference used to return the connection pointer
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
        char mAddressName[128];

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

/* ============================ INLINE METHODS ============================ */

#endif  // _PtConnectionEvent_h_
