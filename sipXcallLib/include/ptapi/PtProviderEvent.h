//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _PtProviderEvent_h_
#define _PtProviderEvent_h_

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

//:PtProviderEvent contains PtProvider-associated event data

class PtProviderEvent : public PtEvent
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   PtProviderEvent(const PtProviderEvent& rPtProviderEvent);
     //:Copy constructor

   virtual
   ~PtProviderEvent();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   PtProviderEvent& operator=(const PtProviderEvent& rhs);
     //:Assignment operator

/* ============================ ACCESSORS ================================= */

   PtStatus getProvider(PtProvider& rProvider);
     //:Returns the provider object associated with this event.
     //!param: (out) prProvider - The reference used to return the provider pointer
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   PtProviderEvent();
     //:Default constructor
};

/* ============================ INLINE METHODS ============================ */

#endif  // _PtProviderEvent_h_
