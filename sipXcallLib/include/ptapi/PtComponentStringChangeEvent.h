//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _PtComponentStringChangeEvent_h_
#define _PtComponentStringChangeEvent_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "ptapi/PtTerminalComponentEvent.h"
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:PtComponentStringChangeEvent contains PtComponent-associated event data,
//:where a component's string type property has changed

class PtComponentStringChangeEvent : public PtTerminalComponentEvent
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   PtComponentStringChangeEvent(const PtComponentStringChangeEvent& rPtComponentStringChangeEvent);
     //:Copy constructor

   virtual
   ~PtComponentStringChangeEvent();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   PtComponentStringChangeEvent& operator=(const PtComponentStringChangeEvent& rhs);
     //:Assignment operator

/* ============================ ACCESSORS ================================= */
   PtStatus getOldValue(const char*& rpValue);
     //:Returns the component property value before the change as a result
     //:of the event.
     //!param: (out) rpValue - The reference used to return the component property value.  Note the returned string is valid only for the life of this event object.
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   PtStatus getNewValue(const char*& rpValue);
     //:Returns the component property value after the change as a result of the event.
     //!param: (out) rpValue - The reference used to return the component property value.  Note the returned string is valid only for the life of this event object.
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   PtComponentStringChangeEvent();
     //:Default constructor
};

/* ============================ INLINE METHODS ============================ */

#endif  // _PtComponentStringChangeEvent_h_
