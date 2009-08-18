//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "mp/MpResource.h"
#include "mp/MpResourceSortAlg.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MpResourceSortAlg::MpResourceSortAlg()
{
}

// Destructor
MpResourceSortAlg::~MpResourceSortAlg()
{
   // no work required
}

/* ============================ MANIPULATORS ============================== */

// Use a topological sort to order the resource pointers passed in via
// the "unsorted" array.  The sorted pointers are returned via the
// "sorted" array.
// Returns OS_SUCCESS if the sort operation was successful, returns
// OS_LOOP_DETECTED if a loop in the flow graph was detected.
OsStatus MpResourceSortAlg::doSort(MpResource* unsorted[],
                                   MpResource* sorted[], int numResources)
{
   int      i;
   OsStatus res;

   mUnsorted        = unsorted;
   mSorted          = sorted;
   mNextSortedIndex = numResources - 1;

   // mark all the resources as "NOT_VISITED"
   for (i=0; i < numResources; i++)
   {
      mUnsorted[i]->setVisitState(MpResource::NOT_VISITED);
   }

   // visit (start a depth-first search from) all of the resources
   // that have no inputs
   for (i=0; i < numResources; i++)
   {
      if (mUnsorted[i]->numInputs() == 0)
      {
         res = visitResource(mUnsorted[i]);
         if (res != OS_SUCCESS)
            return res;
      }
   }

   // sanity check on the results of the sort
   assert(mNextSortedIndex == -1);
   for (i=0; i < numResources; i++)
   {
      assert(mUnsorted[i]->getVisitState() == MpResource::FINISHED);
   }

   return OS_SUCCESS;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

// Visits the indicated flow graph resource.
// Returns OS_SUCCESS if the visit was successful, OS_LOOP_DETECTED if
// we looped back to a resource that we are already visiting.
OsStatus MpResourceSortAlg::visitResource(MpResource* pResource)
{
   int         dummy;
   int         i;
   int         loopCnt;
   MpResource* pNext;
   OsStatus    res;
   int         visitState;

   visitState = pResource->getVisitState();

   if (visitState == MpResource::FINISHED)           // already visited
      return OS_SUCCESS;
   else if (visitState == MpResource::IN_PROGRESS)   // loop in the flow graph
      return OS_LOOP_DETECTED;

   loopCnt = pResource->maxOutputs();
   pResource->setVisitState(MpResource::IN_PROGRESS);
   for (i = 0; i < loopCnt; i++)            // check all possible outputs
   {
      if (pResource->isOutputConnected(i))  // is anything connected?
      {
         pResource->getOutputInfo(i, pNext, dummy);
         if (pNext != NULL)
         {
            res = visitResource(pNext);     // extend the search
            if (res != OS_SUCCESS)
            {                               // something went wrong, terminate
               return res;                  //  the visit operation and return
            }
         }
      }
   }

   // all outputs connected to this resource have been visited, mark this
   // resource's visit state as "FINISHED" and insert the resource into the
   // sorted array.
   pResource->setVisitState(MpResource::FINISHED);
   mSorted[mNextSortedIndex] = pResource;
   mNextSortedIndex--;

   return OS_SUCCESS;
}

/* ============================ FUNCTIONS ================================= */
