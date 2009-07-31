//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _PtMultiCallEvent_h_
#define _PtMultiCallEvent_h_

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

//:PtMultiCallEvent contains PtCall-associated event data,
//:where multiple calls are involved.
// MultiCall events are typically merges of multiple calls into a single call.
// The original set of calls may be retrieved via the getOldCalls method.  The
// new call that is generated, or the resulting merged call, may be retrieved
// via the inherited getCall method.

class PtMultiCallEvent : public PtCallEvent
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   PtMultiCallEvent(const PtMultiCallEvent& rPtMultiCallEvent);
     //:Copy constructor

   virtual
   ~PtMultiCallEvent();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   PtMultiCallEvent& operator=(const PtMultiCallEvent& rhs);
     //:Assignment operator

/* ============================ ACCESSORS ================================= */

   PtStatus getOldCalls(PtCall*[] calls, int size, int& nItems);
     //:Returns an array of pointers to PtCall objects that are involved
     //:with this multicall event.
     // The caller provides an array that can hold up to <i>size</i>
     // PtCall pointers.  This method will fill in the <i>calls</i> array with
     // up to <i>size</i> pointers.  The actual number of pointers filled in
     // is passed back via the <i>nItems</i> argument.
     //!param: (out) calls - The array of PtCall pointers
     //!param: (in) size - The number of elements in the <i>calls</i> array
     //!param: (out) nItems - The number of items assigned
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_MORE_DATA - There are more than <i>size</i> calls
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   PtStatus numOldCalls(int& count);
     //:Returns the number of calls involved with this multicall event.
     //!param: (out) count - The number of calls
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   PtMultiCallEvent();
     //:Default constructor
};

/* ============================ INLINE METHODS ============================ */

#endif  // _PtMultiCallEvent_h_
