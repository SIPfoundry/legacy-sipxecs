//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "os/OsLogger.h"
#include "os/linux/OsCSemLinux.h"
#include "os/linux/OsUtilLinux.h"
#include "os/linux/pt_csem.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor setting the initial and max semaphore values to maxCount
OsCSemLinux::OsCSemLinux(const int queueOptions, const int maxCount) :
   OsCSemBase(queueOptions, maxCount, maxCount)
{
   int res;
   init();

   res = pt_sem_init(&mSemImp, maxCount, maxCount);
   assert(res == POSIX_OK);

#  ifdef OS_SYNC_DEBUG
   pthread_t me = pthread_self();
   mSyncCrumbs.dropCrumb(me, crumbCreated);
#  endif
}

// Constructor allowing different initial and maximum semaphore values
OsCSemLinux::OsCSemLinux(const int queueOptions, const int maxCount,
                     const int initCount) :
  OsCSemBase(queueOptions, maxCount,initCount)
{
   int res;
   init();

   res = pt_sem_init(&mSemImp, maxCount, initCount);
   assert(res == POSIX_OK);

#  ifdef OS_SYNC_DEBUG
   pthread_t me = pthread_self();
   mSyncCrumbs.dropCrumb(me, crumbCreated);
#  endif
}

// Destructor
OsCSemLinux::~OsCSemLinux()
{
   pt_sem_destroy(&mSemImp);

#  ifdef OS_SYNC_DEBUG
   mSyncCrumbs.dropCrumb(pthread_self(), crumbDeleted);
#  endif
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

// Block the task until the semaphore is acquired or the timeout expires
OsStatus OsCSemLinux::acquire(const OsTime& rTimeout)
{
   struct timespec timeout;
   OsStatus res;

   if (rTimeout.isInfinite())
      res = (pt_sem_wait(&mSemImp) == POSIX_OK) ? OS_SUCCESS : OS_BUSY;
   else if (rTimeout.isNoWait())
      res = (pt_sem_trywait(&mSemImp) == POSIX_OK) ? OS_SUCCESS : OS_BUSY;
   else
   {
      OsUtilLinux::cvtOsTimeToTimespec(rTimeout, &timeout);
      res = (pt_sem_timedwait(&mSemImp, &timeout) == POSIX_OK) ? OS_SUCCESS : OS_WAIT_TIMEOUT;
   }

#ifdef OS_CSEM_DEBUG
   if (res == OS_SUCCESS)
   {
      updateAcquireStats();
   }
#endif

#ifdef OS_SYNC_DEBUG
   if (res == OS_SUCCESS)
   {
      mSyncCrumbs.dropCrumb(pthread_self(), crumbAcquired);
   }
#endif

   return res;
}

// Conditionally acquire the semaphore (i.e., don't block)
// Return OS_BUSY if the semaphore is held by some other task
OsStatus OsCSemLinux::tryAcquire(void)
{
   OsStatus res;

   res = (pt_sem_trywait(&mSemImp) == POSIX_OK) ? OS_SUCCESS : OS_BUSY;

#ifdef OS_CSEM_DEBUG
   if (res == OS_SUCCESS)
   {
      updateAcquireStats();
   }
#endif

#ifdef OS_SYNC_DEBUG
   if (res == OS_SUCCESS)
   {
      mSyncCrumbs.dropCrumb(pthread_self(), crumbAcquired);
   }
#endif

   return res;
}
/*
// Returns the current value of the semaphone
int OsCSemLinux::getValue(void)
{
   return pt_sem_getvalue(&mSemImp);
}
*/

// Release the semaphore
OsStatus OsCSemLinux::release(void)
{
   OsStatus res;

#ifdef OS_SYNC_DEBUG
   mSyncCrumbs.dropCrumb(pthread_self(), crumbReleased);
#endif

   res = (pt_sem_post(&mSemImp) == POSIX_OK) ? OS_SUCCESS : OS_BUSY;

#ifdef OS_CSEM_DEBUG
   if (res == OS_SUCCESS)
   {
      updateReleaseStats();
   }
#endif

    return res;
}


/* ============================ INQUIRY =================================== */

// Get the current count and maximum count values for this semaphore.
void OsCSemLinux::getCountMax(int& count, int& max)
{
   count = mSemImp.count;
   max = mSemImp.max;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

// Common initialization shared by all (non-copy) constructors
void OsCSemLinux::init(void)
{
}
