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
#include <process.h>

#include "utl/UtlRscTrace.h"

// APPLICATION INCLUDES
#include "utl/UtlDefs.h"
#include "utl/UtlString.h"
#include "os/OsDefs.h"
#include "os/OsTask.h"
#include "os/Wnt/OsBSemWnt.h"
#include "os/Wnt/OsUtilWnt.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsBSemWnt::OsBSemWnt(const int queueOptions, const int initState)
{
   mOptions = queueOptions;
   // Under Windows NT, we ignore the queueOptions argument
   //  no security attributes
   //  initial count is determined by the initState parameter
   //  maximum count is 1 (since this is a binary semaphore)
   //  no name for this semaphore object
   mSemImp = CreateSemaphore(NULL, initState, 1, NULL);

#  ifdef OS_SYNC_DEBUG
   mTaskId = (initState == EMPTY) ? GetCurrentThreadId() : 0;
   mLastId = 0;
#  endif
}

// Destructor
OsBSemWnt::~OsBSemWnt()
{
    UtlBoolean res;
    res = CloseHandle(mSemImp);
    mSemImp = NULL;

    mOptions = 0;

    assert(res == TRUE);   // CloseHandle should always return TRUE
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

// Block the task until the semaphore is acquired or the timeout expires
OsStatus OsBSemWnt::acquire(const OsTime& rTimeout)
{
   OsStatus retVal;

   retVal = OsUtilWnt::synchObjAcquire(mSemImp, rTimeout);
#ifdef OS_SYNC_DEBUG
   if (retVal == OS_SUCCESS)
   {
      mTaskId = GetCurrentThreadId();
   }
#endif
   return retVal;
}

// Conditionally acquire the semaphore (i.e., don't block)
// Return OS_BUSY if the semaphore is held by some other task
OsStatus OsBSemWnt::tryAcquire(void)
{
   OsStatus retVal;

   retVal = OsUtilWnt::synchObjTryAcquire(mSemImp);
#ifdef OS_SYNC_DEBUG
   if (retVal == OS_SUCCESS)
   {
      mTaskId = GetCurrentThreadId();
   }
#endif
   return retVal;
}

// Release the semaphore
OsStatus OsBSemWnt::release(void)
{
   OsStatus ret = OS_SUCCESS;

   if (mSemImp == NULL)
   {
      ret = OS_TASK_NOT_STARTED;
   }
   else
   {
#     ifdef OS_SYNC_DEBUG
      // make this change while holding the lock
      int previousLast = mLastId;
      mLastId = GetCurrentThreadId();
      mTaskId = 0;
#     endif

      if (!ReleaseSemaphore(mSemImp,
                            1,         // add one to the previous value
                            NULL))     // don't return the old value
      {
         int lastErr;

         lastErr = GetLastError();

         if (ERROR_TOO_MANY_POSTS == lastErr) {
            ret = OS_ALREADY_SIGNALED;
         } else {
            ret = OS_UNSPECIFIED;
         }
#        ifdef OS_SYNC_DEBUG
         // still holding it, so reset the status
         mTaskId = mLastId;
         mLastId = previousLast;
#        endif
      }
   }
      return ret;
}

/* ============================ INQUIRY =================================== */

// Print semaphore information to the console
void OsBSemWnt::OsBSemShow(void)
{
   char* pOptionStr;
   char* pSemState;
   char* pTaskName;
#ifdef OS_SYNC_DEBUG
   UtlString taskName;
   OsTask* pTask;
#endif

   switch (mOptions)
   {
   case Q_FIFO:
      pOptionStr = "Q_FIFO";
      break;
   case Q_PRIORITY:
      pOptionStr = "Q_PRIORITY";
      break;
   default:
      pOptionStr = "UNKNOWN";
      break;
   }

#ifdef OS_SYNC_DEBUG
   pSemState = (mTaskId == 0) ? "AVAILABLE" : "TAKEN";
   if (mTaskId != 0)
   {
      pTask = OsTask::getTaskById(mTaskId);
      taskName = pTask->getName();
      pTaskName = (char*) taskName.data();

   }
   else
   {
      pTaskName = "N/A";
   }
#else
   pSemState = "UNKNOWN";
   pTaskName = "UNKNOWN";
#endif

   osPrintf("OsBSem object 0x%08x, semOptions=%s, state=%s, heldBy=%s\n",
            (void *) this, pOptionStr, pSemState, pTaskName);
#ifdef OS_SYNC_DEBUG
        taskName.remove(0);
#endif
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
