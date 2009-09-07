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
//#include <pthread.h>

#include "utl/UtlRscTrace.h"

// APPLICATION INCLUDES
#include "os/linux/OsMutexLinux.h"
#include "os/linux/OsUtilLinux.h"
#include "os/linux/pt_mutex.h"
#include "os/OsTask.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor allowing the user to specify options
OsMutexLinux::OsMutexLinux(const unsigned options)
{
   int res;

   res = pt_mutex_init(&mMutexImp);
   assert(res == POSIX_OK);

#  ifdef OS_SYNC_DEBUG
   mSyncCrumbs.dropCrumb(pthread_self(), crumbCreated);
#  endif

}

// Destructor
OsMutexLinux::~OsMutexLinux()
{
   int res;
   res = pt_mutex_destroy(&mMutexImp);

   if(res != POSIX_OK)
   {
       osPrintf("**** ERROR: OsMutex at %p could not be destroyed in thread %ld! ****\n", this, pthread_self());
   }
#  ifdef OS_SYNC_DEBUG
   mSyncCrumbs.dropCrumb(pthread_self(), crumbDeleted);
#  endif
}

/* ============================ MANIPULATORS ============================== */

// Block the task until the mutex is acquired or the timeout expires
OsStatus OsMutexLinux::acquire(const OsTime& rTimeout)
{
   struct timespec timeout;
   OsStatus status;

   if(rTimeout.isInfinite())
   {
      status = (pt_mutex_lock(&mMutexImp) == POSIX_OK) ? OS_SUCCESS : OS_BUSY;
   }
   else if(rTimeout.isNoWait())
   {
      status = (pt_mutex_trylock(&mMutexImp) == POSIX_OK) ? OS_SUCCESS : OS_BUSY;
   }
   else
   {
      OsUtilLinux::cvtOsTimeToTimespec(rTimeout, &timeout);
      status = (pt_mutex_timedlock(&mMutexImp, &timeout) == POSIX_OK)
         ? OS_SUCCESS : OS_WAIT_TIMEOUT;
   }

#ifdef OS_SYNC_DEBUG
   if (status == OS_SUCCESS)
   {
      mSyncCrumbs.dropCrumb(pthread_self(), crumbAcquired);
   }
#endif
   return status;
}

// Conditionally acquire the mutex (i.e., don't block)
// Return OS_BUSY if the lock is held by some other task
OsStatus OsMutexLinux::tryAcquire(void)
{
   OsStatus status = (pt_mutex_trylock(&mMutexImp) == POSIX_OK) ? OS_SUCCESS : OS_BUSY;

#ifdef OS_SYNC_DEBUG
   if (status == OS_SUCCESS)
   {
      mSyncCrumbs.dropCrumb(pthread_self(), crumbAcquired);
   }
#endif

   return status;
}

// Release the mutex
OsStatus OsMutexLinux::release(void)
{
#ifdef OS_SYNC_DEBUG
   // change these while still holding the lock...
   mSyncCrumbs.dropCrumb(pthread_self(), crumbReleased);
#endif

   return (pt_mutex_unlock(&mMutexImp) == POSIX_OK) ? OS_SUCCESS : OS_BUSY;
}

/* ============================ ACCESSORS ================================= */

// Print mutex information to the console
void OsMutexLinux::OsMutexShow(void)
{
   osPrintf("OsMutex object %p\n", (void*) this);
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
