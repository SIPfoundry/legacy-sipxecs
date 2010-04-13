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
#include "os/OsDefs.h"
#include "os/linux/OsBSemLinux.h"
#include "os/linux/OsUtilLinux.h"
#include "os/linux/pt_csem.h"
#include "os/OsSysLog.h"
#include "os/OsTask.h"

#ifdef OS_SYNC_DEBUG
#  include "os/OsDateTime.h"
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsBSemLinux::OsBSemLinux(const int queueOptions, const int initState)
{
   int res;

   mOptions = queueOptions;
   res = pt_sem_init(&mSemImp, 1, initState);
   assert(res == POSIX_OK);

#  ifdef OS_SYNC_DEBUG
   pthread_t me = pthread_self();
   mSyncCrumbs.dropCrumb(me, crumbCreated);
   if (EMPTY == initState)
   {
      mSyncCrumbs.dropCrumb(me, crumbAcquired);
   }
#  endif
}

// Destructor
OsBSemLinux::~OsBSemLinux()
{
   int res;
   res = pt_sem_destroy(&mSemImp);

   if (res != POSIX_OK)
   {
      OsSysLog::add(FAC_KERNEL, PRI_ERR,
                    "OsBSemLinux::~OsBSemLinux pt_sem_destroy returned %d in task %lu",
                    res, (unsigned long)pthread_self()
                    );
   }
#  ifdef OS_SYNC_DEBUG
   mSyncCrumbs.dropCrumb(pthread_self(), crumbDeleted);
#  endif

}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

// Block the task until the semaphore is acquired or the timeout expires
OsStatus OsBSemLinux::acquire(const OsTime& rTimeout)
{
   OsStatus retVal;
   struct timespec timeout;

#  ifdef OS_SYNC_DEBUG
   OsTime waitingSince;
   OsDateTime::getCurTime(waitingSince);
#  endif

   if(rTimeout.isInfinite())
   {
      retVal = (pt_sem_wait(&mSemImp) == POSIX_OK) ? OS_SUCCESS : OS_BUSY;
   }
   else if(rTimeout.isNoWait())
   {
      retVal = (pt_sem_trywait(&mSemImp) == POSIX_OK) ? OS_SUCCESS : OS_BUSY;
   }
   else
   {
      OsUtilLinux::cvtOsTimeToTimespec(rTimeout, &timeout);
      retVal = (pt_sem_timedwait(&mSemImp, &timeout) == POSIX_OK) ? OS_SUCCESS : OS_WAIT_TIMEOUT;
   }
#ifdef OS_SYNC_DEBUG
   if (OS_SUCCESS == retVal)
   {
      mSyncCrumbs.dropCrumb(pthread_self(), crumbAcquired);
      /* use the waitingSince to keep the compiler from optimizing it away */
      if ( 0 == waitingSince.cvtToMsecs() )// should never be true
      {
         retVal = OS_SUCCESS;
      }
   }
#endif
   return retVal;
}

// Conditionally acquire the semaphore (i.e., don't block)
// Return OS_BUSY if the semaphore is held by some other task
OsStatus OsBSemLinux::tryAcquire(void)
{
   OsStatus retVal;

   retVal = (pt_sem_trywait(&mSemImp) == POSIX_OK) ? OS_SUCCESS : OS_BUSY;
#ifdef OS_SYNC_DEBUG
   if (retVal == OS_SUCCESS)
   {
      mSyncCrumbs.dropCrumb(pthread_self(), crumbAcquired);
   }
#endif
   return retVal;
}

// Release the semaphore
OsStatus OsBSemLinux::release(void)
{
   OsStatus retVal;

#ifdef OS_SYNC_DEBUG
   // change these while still holding the lock...
   mSyncCrumbs.dropCrumb(pthread_self(), crumbReleased);
#endif

   retVal = (pt_sem_post(&mSemImp) == POSIX_OK) ? OS_SUCCESS : OS_ALREADY_SIGNALED;

   return retVal;
}

/* ============================ INQUIRY =================================== */

// Print semaphore information to the console
void OsBSemLinux::OsBSemShow(void)
{
   // don't need this
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */
