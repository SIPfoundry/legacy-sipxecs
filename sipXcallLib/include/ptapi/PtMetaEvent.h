//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _PtMetaEvent_h_
#define _PtMetaEvent_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "ptapi/PtEvent.h"
#include "tao/TaoClientTask.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:The <CODE>MetaEvent</CODE> interface is the base interface for
// all PTAPI Meta events.
// All Meta events must extend this interface.
// <p>
// An individual <CODE>MetaEvent</CODE>
// conveys, directly and with necessary details,
// what an application needs to know to respond to a higher-level PTAPI event.
// <p>
// Currently, meta events are defined in pairs, and they convey three
// types of information:
// first, the type of higher-level operation which has occurred;
// second, the beginning and end of the sequence of "normal" PTAPI events
// which were generated to convey the consequential PTAPI model state changes
// that occurred because of this higher-level operation;
// third, which PTAPI entities were involved in the higher-level operation.
// <p>
// A PTAPI implementation is alerted to changes in the state of the associated telephony platform,
// and reflects that state by sending a stream of PTAPI objects, delimited by MetaEvents.
// An application learns of the details of that state change by processing all the events
// between the starting and ending MetaEvents.
// <p>
// Generally the application may draw incorrect conclusions about the current state of
// the associated telephony platform if it decides to act before processing
// all the events delimited by the MetaEvents.
// Specifically, an application which wishes to submit queries to the PTAPI implementation
// about the current state of PTAPI entities should not submit the query until it has
// processed the matching "ending" MetaEvent.
// <p>
// The specific Meta event is indicated by
// the <CODE>Event.getID()</CODE>
// value returned by the event.
// The specific Meta event provides context information about the higher-level event.
// <p>
// The <CODE>Event.getMetaEvent</CODE> method returns the Meta event
// associated with a "normal" event (or returns null).

class PtMetaEvent : public PtEvent
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

    PT_CLASS_INFO_MEMBERS

/* ============================ CREATORS ================================== */
   PtMetaEvent(PtEventId eventId = EVENT_INVALID,
                                int metaCode = META_EVENT_NONE,
                                int numOldCalls = 0,
                                const char* callId = 0,
                                TaoClientTask *pClient = 0,
                                int sipResponseCode = 0,
                                const char* sipResponseText = 0,
                                const char** oldCallIds = 0,
                                const char* newCallId = NULL);
     //:Default constructor

   PtMetaEvent(const PtMetaEvent& rPtMetaEvent);
     //:Copy constructor

   virtual
   ~PtMetaEvent();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   PtMetaEvent& operator=(const PtMetaEvent& rhs);
     //:Assignment operator

/* ============================ ACCESSORS ================================= */
   static const char* className();
     //:Returns the name of this class.
     //!returns: Returns the string representation of the name of this class

/* ============================ INQUIRY =================================== */
   virtual PtBoolean isClass(const char* pClassName);
     //:Determines if this object if of the specified type.
     //!param: (in) pClassName - The string to compare with the name of this class.
     //!retcode: TRUE - If the given string contains the class name of this class.
     //!retcode: FALSE - If the given string does not match that of this class

   virtual PtBoolean isInstanceOf(const char* pClassName);
     //:Determines if this object is either an instance of or is derived from
     //:the specified type.
     //!param: (in) pClassName - The string to compare with the name of this class.
     //!retcode: TRUE - If this object is either an instance of or is derived from the specified class.
     //!retcode: FALSE - If this object is not an instance of the specified class.

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

/* ============================ INLINE METHODS ============================ */

#endif  // _PtMetaEvent_h_
