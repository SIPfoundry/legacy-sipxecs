//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _MpResourceSortAlg_h_
#define _MpResourceSortAlg_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsStatus.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
class MpResource;

//:Algorithm for performing a topological sort on the MpResource objects in
//:a flow graph.
class MpResourceSortAlg
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   MpResourceSortAlg();
     //:Constructor

   virtual
   ~MpResourceSortAlg();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   OsStatus doSort(MpResource* unsorted[],
                   MpResource* sorted[], int numResources);
     //:Use a topological sort to order the resource pointers passed in via
     //:the "unsorted" array.  The sorted pointers are returned via the
     //:"sorted" array.
     //!param: (in) unsorted - unsorted array of MpResource pointers
     //!param: (out) sorted - sorted array of MpResource pointers
     //!param: (in) numResources - size of the resource pointer arrays
     //!retcode: OS_SUCCESS - the sort operation was successful
     //!retcode: OS_LOOP_DETECTED - if a loop in the flow graph was detected

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   MpResource** mUnsorted;
   MpResource** mSorted;
   int          mNextSortedIndex;

   OsStatus visitResource(MpResource* pResource);
     //:Visits the indicated flow graph resource.
     // Returns OS_SUCCESS if the visit was successful, OS_LOOP_DETECTED if
     // we looped back to a resource that we are already visiting.

   MpResourceSortAlg(const MpResourceSortAlg& rMpResourceSortAlg);
     //:Copy constructor (not implemented for this class)

   MpResourceSortAlg& operator=(const MpResourceSortAlg& rhs);
     //:Assignment operator (not implemented for this class)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpResourceSortAlg_h_
