//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _PtEventMask_h_
#define _PtEventMask_h_

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

//:The PtEventMask is used to filter events which get passed to a
//:PtEventListener.
// To provide finer granularity on the control of events that get passed to
// a listener, the PtEventMask may be used to define the specific events
// that are to be passed.  The PtEventMask may be used only to reduce the
// event types that a listener will receive.  The full set of events
// a listener may receive is defined by the derived listener class.  Reducing
// the events that a listener is interested in is a useful means of optimizing
// the performance of the Pingtel system.  Due to the distributed nature
// of the Pingtel system, reducing the number of unnecessary events that
// get sent results in less network latency and traffic.
class PtEventMask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   PtEventMask();
     //:Default constructor

   PtEventMask(const PtEventMask& rPtEventMask);
     //:Copy constructor

   virtual
   ~PtEventMask();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   PtEventMask& operator=(const PtEventMask& rhs);
     //:Assignment operator

/* ============================ ACCESSORS ================================= */

   PtStatus setEvents(PtEvent::PtEventId events[], int numEvents);
     //:Sets the events of interest for this mask
     //!param: (in) events - Array of event ids of interest
     //!param: (in) numEvents - The number of event ids in the <i>events</i> array
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_INVALID_ARGUMENT - An invalid event id was contained in <i>events</i>

   PtStatus addEvents(PtEvent::PtEventId events[], int numEvents);
     //:Adds events of interest to this mask (Union operation)
     //!param: (in) events - Array of event ids of interest
     //!param: (in) numEvents - The number of event ids in the <i>events</i> array
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_INVALID_ARGUMENT - An invalid event id was contained in <i>events</i>

   PtStatus addEvents(PtEventMask& rMask);
     //:Adds events of interest to this mask (Union operation)
     //!param: (in) rMask - A PtEventMask containing the set of event ids to be added to this mask's event ids.
     //!retcode: PT_SUCCESS - Success

/* ============================ INQUIRY =================================== */
   PtBoolean isEventEnabled(PtEvent::PtEventId eventId);
     //:Inquires if the given event type is of interest to this listener
     //!param: (in) eventId - The event id corresponding to the event type
     //!retcode: TRUE - If the given event type is of interest to this listener
     //!retcode: FALSE - If the given event type is NOT of interest to this listener

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

/* ============================ INLINE METHODS ============================ */

#endif  // _PtEventMask_h_
