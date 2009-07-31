//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _PtEventListener_h_
#define _PtEventListener_h_

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
class PtEventMask;

//:Base class for listener objects.
// The listener class is used to register interest and obtain notification
// of events of interest on Pingtel objects.  The PtEventListener is
// specialized, in the provided derived classes, with methods for each event
// type that a Pingtel object may generate.  An application that is
// interested in being notified of specific events must implement
// the derived class for the listener capable of receiving those events
// (the default implementation provides only empty stubs).  The application
// then registers the listener (via an appropriate add<type>Listener method)
// on the object signaling the events of interest.  As events occur, they are
// reported to all listeners that have registered interest in events of that
// type.  Events are reported by invoking a method on the listener that
// corresponds to the event type and passing it the event object as an
// argument.
// <p>
//
// To provide finer granularity on the control of events that get passed to
// the listener, the PtEventMask may be used to define the specific events
// that are to be passed.  The PtEventMask may be used only to reduce the
// event types that a listener will receive.  The full set of events
// a listener may receive is defined by the derived listener class.  Reducing
// the events that a listener is interested in is a useful means of optimizing
// the performance of the Pingtel system.  Due to the distributed nature
// of the Pingtel system, reducing the number of unnecessary events that
// get sent results in less network latency and traffic.

class PtEventListener
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   PT_CLASS_INFO_MEMBERS

/* ============================ CREATORS ================================== */

   PtEventListener(PtEventMask* pMask = NULL);
     //:Default constructor
     //!param: (in) pMask - Event mask defining events the listener is interested in.  This must be a subset of the events that the listener supports.  The mask may be NULL where it is assumed that all events applicable to the derived listener are of interest.

   virtual
   ~PtEventListener();
     //:Destructor

/* ============================ MANIPULATORS ============================== */


/* ============================ ACCESSORS ================================= */

   PtStatus getEventMask(const PtEventMask*& rpMask) const;
     //:Returns the PtEventMask that defines the events of interest
     // The event mask is read only.  Unpredictable results will occur if
     // the event mask is modified after a listener is constructed.
     // The mask may be NULL where it is assumed that all events applicable
     // to the derived listener are of interest.
     //!param: (out) rpMask - Reference to a pointer to the PtEventMask.
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   static const char* className();
     //:Returns the name of this class
     // Returns the string representation of the name of this class.

/* ============================ INQUIRY =================================== */

   virtual PtBoolean isClass(const char* pClassName);
     //:Determines if this object if of the specified type.
     //!param: (in) pClassName - The string to compare with the name of this class.
     //!retcode: TRUE - If the given string contains the class name of this class
     //!retcode: FALSE - If the given string does not match that of this class

   virtual PtBoolean isInstanceOf(const char* pClassName);
     //:Determines if this object is either an instance of or is derived from
     //:the specified type.
     //!param: (in) pClassName - The string to compare with the name of this class
     //!retcode: TRUE - If this object is either an instance of or is derived from the specified class
     //!retcode: FALSE - If this object is not an instance of the specified class

   PtBoolean isEventEnabled(PtEvent::PtEventId& eventId);
     //:Determines if the given event type is of interest to this listener.
     //!param: (in) eventId - The event id corresponding to the event type
     //!retcode: TRUE - If the given event type is of interest to this listener
     //!retcode: FALSE - If the given event type is NOT of interest to this listener

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   PtEventMask* mpMask;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   PtEventListener(const PtEventListener& rPtEventListener);
     //:Copy constructor

   PtEventListener& operator=(const PtEventListener& rhs);
     //:Assignment operator
};

/* ============================ INLINE METHODS ============================ */

#endif  // _PtEventListener_h_
