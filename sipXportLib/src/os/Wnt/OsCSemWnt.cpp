//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

// SYSTEM INCLUDES
#include <assert.h>

#include "utl/UtlRscTrace.h"

// APPLICATION INCLUDES
#include "os/Wnt/OsCSemWnt.h"
#include "os/Wnt/OsUtilWnt.h"

#ifdef OS_CSEM_DEBUG
#include "os/OsLock.h"
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor setting the initial and max semaphore values to maxCount
OsCSemWnt::OsCSemWnt(const int queueOptions, const int maxCount) :
 OsCSemBase(queueOptions, maxCount, maxCount)
{
   init();

   // Under Windows NT, we ignore the queueOptions argument
   mSemImp = CreateSemaphore(NULL,      // no security attributes
                             maxCount,  // initial count is set to maxCount
                             maxCount,  // maximum count is set to maxCount
                             NULL);     // no name for this semaphore object
}

// Constructor allowing different initial and maximum semaphore values
OsCSemWnt::OsCSemWnt(const int queueOptions, const int maxCount,
                     const int initCount) :
  OsCSemBase(queueOptions, maxCount, initCount)
{
   init();

   // Under Windows NT, we ignore the queueOptions argument
   mSemImp = CreateSemaphore(NULL,      // no security attributes
                             initCount, // initial count is set to maxCount
                             maxCount,  // maximum count is set to maxCount
                             NULL);     // no name for this semaphore object
}

// Destructor
OsCSemWnt::~OsCSemWnt()
{
   UtlBoolean res;
   res = CloseHandle(mSemImp);

   assert(res == TRUE);   // CloseHandle should always return TRUE
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

// Block the task until the semaphore is acquired or the timeout expires
OsStatus OsCSemWnt::acquire(const OsTime& rTimeout)
{
   OsStatus res = OsUtilWnt::synchObjAcquire(mSemImp, rTimeout);

#ifdef OS_CSEM_DEBUG
   if (res == OS_SUCCESS)
      updateAcquireStats();
#endif

   return res;
}

// Conditionally acquire the semaphore (i.e., don't block)
// Return OS_BUSY if the semaphore is held by some other task
OsStatus OsCSemWnt::tryAcquire(void)
{
   OsStatus res = OsUtilWnt::synchObjTryAcquire(mSemImp);

#ifdef OS_CSEM_DEBUG
   if (res == OS_SUCCESS)
      updateAcquireStats();
#endif

   return res;
}

// Release the semaphore
OsStatus OsCSemWnt::release(void)
{
   long     prevCount;
   OsStatus res;

   if (ReleaseSemaphore(mSemImp,
                        1,            // add one to the previous value
                        &prevCount))  // previous value
      res = OS_SUCCESS;
   else
      res = OS_UNSPECIFIED;

#ifdef OS_CSEM_DEBUG
   if (res == OS_SUCCESS)
   {
      updateReleaseStats();
   }
#endif //OS_CSEM_DEBUG

   return res;
}



/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

// Common initialization shared by all (non-copy) constructors
void OsCSemWnt::init(void)
{
}

/* ============================ FUNCTIONS ================================= */
