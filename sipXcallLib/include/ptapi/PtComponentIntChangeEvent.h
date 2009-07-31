//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _PtComponentIntChangeEvent_h_
#define _PtComponentIntChangeEvent_h_

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

//:PtComponentIntChangeEvent contains PtComponent-associated event data,
//:where a component's int type property has changed

class PtComponentIntChangeEvent : public PtTerminalComponentEvent
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   PtComponentIntChangeEvent(const PtComponentIntChangeEvent& rPtComponentIntChangeEvent);
     //:Copy constructor

   virtual
   ~PtComponentIntChangeEvent();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   PtComponentIntChangeEvent& operator=(const PtComponentIntChangeEvent& rhs);
     //:Assignment operator

/* ============================ ACCESSORS ================================= */
   PtStatus getOldValue(int& rValue);
     //:Returns the component property value before the change as a result
     //:of the event.
     //!param: (out) rValue - The reference used to return the component property value
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   PtStatus getNewValue(int& rValue);
     //:Returns the component property value after the change as a result of
     //:the event.
     //!param: (out) rValue - The reference used to return the component property value
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   PtComponentIntChangeEvent();
     //:Default constructor
};

/* ============================ INLINE METHODS ============================ */

#endif  // _PtComponentIntChangeEvent_h_
