//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _PtAddressEvent_h_
#define _PtAddressEvent_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:PtAddressEvent contains PtAddress-associated event data.

class PtAddressEvent : public PtEvent
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   PtAddressEvent(const PtAddressEvent& rPtAddressEvent);
     //:Copy constructor

   virtual
   ~PtAddressEvent();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   PtAddressEvent& operator=(const PtAddressEvent& rhs);
     //:Assignment operator

/* ============================ ACCESSORS ================================= */
   PtStatus getAddress(PtAddress*& prAddress);
     //:Returns the address object associated with this event.
     //!param: (out) prAddress - The reference used to return the address pointer
     //!retcode: OP_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   PtAddressEvent();
     //:Default constructor
};

/* ============================ INLINE METHODS ============================ */

#endif  // _PtAddressEvent_h_
