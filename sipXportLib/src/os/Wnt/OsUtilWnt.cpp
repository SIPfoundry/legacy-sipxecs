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

// APPLICATION INCLUDES
#include "os/Wnt/OsUtilWnt.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES

// CONSTANTS
const int MILLISECS_PER_SEC      = 1000;
const int MICROSECS_PER_MILLISEC = 1000;

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

// (rschaaf) A note on manipulating synchronization objects:
// For Windows NT, the code used to acquire synchronization objects is common
// for binary semaphores, counting semaphores and mutexes. We capture this
// common code here to avoid having to duplicate it for each of the
// specialized synchronization objects.
//
// Ideally, I'd like to use inheritance to avoid code duplication in the
// derived classes. However, in our object model the Windows NT-specific
// classes for binary semaphore, counting semaphore and mutex do not have a
// common Windows NT-specific base class. It therefore does not appear to be
// possible to use single inheritance to achieve this goal and I am
// deliberately avoiding the use (and attendant complications) of multiple
// inheritance.

// Block the task until the synch obj is acquired or the timeout expires
OsStatus OsUtilWnt::synchObjAcquire(const HANDLE synchObj,
                                    const OsTime& rTimeout)
{
   DWORD    ntRes;
   DWORD    msecsTimer;
   OsStatus res;

   if (rTimeout.isInfinite())
      msecsTimer = INFINITE;
   else
   {
      assert(OsUtilWnt::isOsTimeValid(rTimeout));
      msecsTimer = OsUtilWnt::cvtOsTimeToWntTime(rTimeout);
   }

   ntRes = WaitForSingleObject(synchObj, msecsTimer);
   switch (ntRes)
   {
   case 0:
      res = OS_SUCCESS;
      break;
   case WAIT_TIMEOUT:
      res = OS_WAIT_TIMEOUT;
      break;
   case WAIT_ABANDONED:
      res = OS_WAIT_ABANDONED;
      break;
   default:
/*
      osPrintf(
         "OsUtilWnt::synchObjAcquire: WaitForSingleObject() returned %d,\n"
         "   GetLastError() = %d"
         "\n\n", ntRes, GetLastError());
*/
      res = OS_UNSPECIFIED;
      break;
   }

   return res;
}

// Conditionally acquire the synch obj (i.e., don't block)
// Return OS_BUSY if the synch object is held by some other task
OsStatus OsUtilWnt::synchObjTryAcquire(const HANDLE synchObj)
{
   OsStatus res;

   res = OsUtilWnt::synchObjAcquire(synchObj, OsTime::NO_WAIT);

   if (res == OS_WAIT_TIMEOUT)
      return OS_BUSY;
   else
      return res;
}


/* ============================ ACCESSORS ================================= */

// Convert an OsTime to the corresponding number of millisecs for WinNT
DWORD OsUtilWnt::cvtOsTimeToWntTime(const OsTime& rTimer)
{
   return (rTimer.seconds() * MILLISECS_PER_SEC) +
          (rTimer.usecs()   / MICROSECS_PER_MILLISEC);
}

// Convert an abstraction layer task priority to a WinNT thread priority
int OsUtilWnt::cvtOsPrioToWntPrio(const int osPrio)
{
   // Map task priority values from the abstraction layer to Win NT as
   // follows:
   //  0         THREAD_PRIORITY_HIGHEST
   //  1   - 85  THREAD_PRIORITY_ABOVE_NORMAL
   //  86  - 170 THREAD_PRIORITY_NORMAL
   //  171 - 254 THREAD_PRIORITY_BELOW_NORMAL
   //  255       THREAD_PRIORITY_LOWEST

   assert(osPrio >= 0 && osPrio <= 255);

   if (osPrio == 0)
      return THREAD_PRIORITY_HIGHEST;
   else if (osPrio <= 85)
      return THREAD_PRIORITY_ABOVE_NORMAL;
   else if (osPrio <= 170)
      return THREAD_PRIORITY_NORMAL;
   else if (osPrio <= 254)
      return THREAD_PRIORITY_BELOW_NORMAL;
   else
      return THREAD_PRIORITY_LOWEST;
}

// Convert a WinNT thread priority to an abstraction layer task priority
int OsUtilWnt::cvtWntPrioToOsPrio(const int wntPrio)
{
   int osPrio;

   switch (wntPrio)
   {
   case THREAD_PRIORITY_HIGHEST:
      osPrio = 0;
      break;
   case THREAD_PRIORITY_ABOVE_NORMAL:
      osPrio = 43;                 // middle of range from 1 - 85
      break;
   case THREAD_PRIORITY_NORMAL:
      osPrio = 128;                // middle of range from 86 - 170
      break;
   case THREAD_PRIORITY_BELOW_NORMAL:
      osPrio = 213;                // middle of range from 171 - 254
      break;
   case THREAD_PRIORITY_LOWEST:
      osPrio = 255;
      break;
   default:
      assert(FALSE);                 // should not happen
      osPrio = -1;
      break;
   }

   return osPrio;
}

/* ============================ INQUIRY =================================== */

// Verify that the OsTime is >= 0 and representable in msecs
UtlBoolean OsUtilWnt::isOsTimeValid(const OsTime& rTimer)
{
   int secs = rTimer.seconds();

   // Assume that the size of an int is 4 bytes
   assert(sizeof(int) == 4);

   // Timeout values should be in the future
   if (secs < 0)
      return FALSE;

   // The following test is "<" rather than "<=" since the contribution of
   //  the microseconds portion of rTimer could add another second.
   return (secs < (0x7FFFFFFF / MILLISECS_PER_SEC));
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
