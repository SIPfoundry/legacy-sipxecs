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
#ifdef __pingtel_on_posix__
#  include <pthread.h>
#endif

// APPLICATION INCLUDES
#include "os/OsRWMutex.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES

// CONSTANTS
static int SEMAPHORE_CNT_MAX = 100;  // upper bound on the number of
                                     //  simultaneous readers or writers

// STATIC VARIABLE INITIALIZATIONS

// Mutual exclusion semaphore handling multiple readers and writers.
//
// Two kinds of concurrent tasks, called "readers" and "writers", share a
// single resource. The readers can use the resource simultaneously, but each
// writer must have exclusive access to it. When a writer is ready to use the
// resource, it should be enabled to do so as soon as possible.
//
// This implementation is based on the description from the book "Operating
// Systems Principles" by Per Brinch Hansen, 1973.  This solution uses:
//   - a binary semaphore (mGuard) to ensure against concurrent access to
//     internal object data
//   - counting semaphores (mReadSem, mWriteSem) to coordinate the actions of
//     readers and writers of the resource
//   - a binary semaphore (mWriteExclSem) to ensure that only one writer at a
//     time is given access to the resource.

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsRWMutex::OsRWMutex(const int queueOptions) :
   mGuard(queueOptions, OsBSem::FULL),
   mReadSem(queueOptions, SEMAPHORE_CNT_MAX, 0),
        mWriteSem(queueOptions, SEMAPHORE_CNT_MAX, 0),
   mWriteExclSem(queueOptions, OsBSem::FULL),
        mActiveReadersCnt(0),
        mActiveWritersCnt(0),
        mRunningReadersCnt(0),
        mRunningWritersCnt(0)
{
        // all of the work is done by the initializers
}

// Destructor
OsRWMutex::~OsRWMutex()
{
        // no work required
}

/* ============================ MANIPULATORS ============================== */

// Block (if necessary) until the task acquires the resource for reading.
// Multiple simultaneous readers are allowed.
OsStatus OsRWMutex::acquireRead(void)
{
   return doAcquireRead(FALSE);
}

// Block until the task acquires the resource for writing.
// Only one writer at a time is allowed (and no readers).
OsStatus OsRWMutex::acquireWrite(void)
{
   return doAcquireWrite(FALSE);
}

// Conditionally acquire the resource for reading (i.e., don't block)
// Multiple simultaneous readers are allowed.
// Return OS_BUSY if the resource is held for writing by some other task
OsStatus OsRWMutex::tryAcquireRead(void)
{
   return doAcquireRead(TRUE);
}

// Conditionally acquire the resource for writing (i.e., don't block)
// Return OS_BUSY if the resource is held for writing by some other task
// or if there are running readers.
OsStatus OsRWMutex::tryAcquireWrite(void)
{
   return doAcquireWrite(TRUE);
}

// Release the resource for reading
OsStatus OsRWMutex::releaseRead(void)
{
   OsStatus res;

   assert(mRunningReadersCnt > 0);

   res = mGuard.acquire();         // start critical section
   assert(res == OS_SUCCESS);

   res = doReleaseRead();
   assert(res == OS_SUCCESS);

   res = mGuard.release();         // exit critical section
   assert(res == OS_SUCCESS);

   return OS_SUCCESS;
}

// Release the resource for writing
OsStatus OsRWMutex::releaseWrite(void)
{
   OsStatus res;

   assert(mRunningWritersCnt > 0);

   res = doReleaseExclWrite();     // release the semaphore used to ensure
   assert(res == OS_SUCCESS);      //  exclusive access among multiple writers

   res = doReleaseNonExclWrite(FALSE);  // give up the resource for writing
   assert(res == OS_SUCCESS);

   return OS_SUCCESS;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

// Helper function used to acquire the resource for reading
OsStatus OsRWMutex::doAcquireRead(UtlBoolean dontBlock)
{
   // This works as follows:
   // 1) Determine whether we can acquire a read ticket without blocking.
   //    If not and "dontBlock" is TRUE, return an OS_BUSY indication.
   // 2) Indicate that we want to acquire a "ticket" for reading by
   //    incrementing the mActiveReadersCnt
   // 3) If the resource is held for writing, block on the mReadSem semaphore
   //    until reading is allowed and read tickets are available
   // 4) If the resource is not held for writing, generate enough read tickets
   //    to satisfy all active but not yet running readers
   //
   // The state information in the OsRWMutex object itself (this) is
   // protected using the mGuard critical section.

   OsStatus res;

   res = mGuard.acquire();         // start of critical section
   assert(res == OS_SUCCESS);

   if (dontBlock && mActiveWritersCnt > 0)
   {                               // writers are active
      res = mGuard.release();      // exit the critical section
      assert(res == OS_SUCCESS);

      return OS_BUSY;              // indicate that the resource is busy
   }
   else
   {
      mActiveReadersCnt++;
      grantReadTickets();

      res = mGuard.release();      // exit the critical section
      assert(res == OS_SUCCESS);

      res = mReadSem.acquire();    // wait for a read ticket
      assert(res == OS_SUCCESS);

      return OS_SUCCESS;
   }
}

// Helper function used to acquire the resource for writing
OsStatus OsRWMutex::doAcquireWrite(UtlBoolean dontBlock)
{
   // This works as follows:
   // 1) Determine whether we can acquire a write ticket without blocking.
   //    If not and "dontBlock" is TRUE, return an OS_BUSY indication.
   // 2) Indicate that we want to acquire a "ticket" for writing by
   //    incrementing the mActiveWritersCnt
   // 3) If the resource is held for writing, block on the mWriteSem semaphore
   //    until writing is allowed and a write ticket is available
   //
   // The state information in the OsRWMutex object itself (this) is
   // protected using the mGuard critical section.

   OsStatus res;
   OsStatus savedRes;

   res = mGuard.acquire();         // start critical section
   assert(res == OS_SUCCESS);

   if (dontBlock && (mRunningReadersCnt > 0 || mRunningWritersCnt > 0))
   {                               // there are readers running or there
                                   //  is a writer running
      res = mGuard.release();      // exit critical section
      assert(res == OS_SUCCESS);

      return OS_BUSY;              // indicate that the resource is busy
   }

   // If we make it to this point, the resource is either not obviously busy
   // or we are willing to wait for it.

   mActiveWritersCnt++;
   grantWriteTickets();

   if (dontBlock)
   {                                     // try to acquire the resource but
      savedRes = mWriteSem.tryAcquire(); //  don't block if it's busy
      assert(savedRes == OS_SUCCESS || savedRes == OS_BUSY);

      if (savedRes != OS_SUCCESS)
      {
         // We failed to obtain the resource for non-exclusive writing.
         // Reset the internal state (give back the write ticket) to account
         // for the failure
         res = doReleaseNonExclWrite(TRUE);
         assert(res == OS_SUCCESS);

         res = mGuard.release();         // exit critical section
         assert(res == OS_SUCCESS);

         return savedRes;
      }

      res = mGuard.release();            // exit critical section
      assert(res == OS_SUCCESS);
   }
   else  // dontBlock == FALSE
   {
      res = mGuard.release();            // exit critical section before
      assert(res == OS_SUCCESS);         //  attempting to acquire the
                                         //  semaphore (which may block)
      res = mWriteSem.acquire();         // acquire non-exclusive write
      assert(res == OS_SUCCESS);         //  access, blocking if necessary
   }

   // At this point, we have successfully obtained a write ticket
   // (non-exclusive write access) for the resource. Now obtain the exclusive
   // write semaphore to ensure that there is only one writer operating on
   // the resource at a time.
   savedRes = doAcquireExclWrite(dontBlock);
   assert(savedRes == OS_SUCCESS || savedRes == OS_BUSY);

   if (savedRes != OS_SUCCESS)
   {
      // We failed to obtain the resource for exclusive writing.
      // Reset the internal state (give back the write ticket) to account for
      // the failure
      res = doReleaseNonExclWrite(FALSE);
      assert(res == OS_SUCCESS);
   }

   return savedRes;
}

// Helper function used to acquire a semaphore for exclusive writing. A
// binary semaphore is used to ensure that there is only a single writer
// operating on the resource at any one time.
OsStatus OsRWMutex::doAcquireExclWrite(UtlBoolean dontBlock)
{
   OsStatus res;

   if (dontBlock)
   {
      res = mWriteExclSem.tryAcquire();
      assert(res == OS_SUCCESS || res == OS_BUSY);
   }
   else
   {
      res = mWriteExclSem.acquire();
      assert(res == OS_SUCCESS);
   }

   return OS_SUCCESS;
}

// Helper function allowing a reader to give up access to the resource.
//
// The mGuard object must be acquired (and ultimately be released) by callers
// of this method.
OsStatus OsRWMutex::doReleaseRead(void)
{
   mRunningReadersCnt--;           // give up the resource for reading
   mActiveReadersCnt--;
   assert(mRunningReadersCnt >= 0 &&
          mActiveReadersCnt  >= 0 &&
          mActiveReadersCnt  >= mRunningReadersCnt);
   grantWriteTickets();            // handle any writers that may be waiting

   return OS_SUCCESS;
}

// Helper function to release the semaphore allowing (non-exclusive)
// write access.
//
// The mGuard object must be acquired (and ultimately be released) by callers
// of this method.
OsStatus OsRWMutex::doReleaseNonExclWrite(UtlBoolean guardIsHeld)
{
   OsStatus res;

   if (!guardIsHeld)
   {
      res = mGuard.acquire();
      assert(res == OS_SUCCESS);
   }

   mRunningWritersCnt--;
   mActiveWritersCnt--;
   assert(mRunningWritersCnt >= 0 &&
          mActiveWritersCnt  >= 0 &&
          mActiveWritersCnt  >= mRunningWritersCnt);
   grantReadTickets();             // handle any readers that may be waiting

   if (!guardIsHeld)
   {
      res = mGuard.release();
      assert(res == OS_SUCCESS);
   }

   return OS_SUCCESS;
}

// Helper function to release the semaphore used to ensure exclusive access
// when there are multiple writers.
OsStatus OsRWMutex::doReleaseExclWrite(void)
{
   OsStatus res;

   res = mWriteExclSem.release();
   assert(res == OS_SUCCESS);

   return OS_SUCCESS;
}

// Determine whether the resource can be granted immediately to readers
// and, if so, grant sufficient read "tickets" for all active but not
// yet running readers.
//
// The mGuard object must be acquired (and ultimately be released) by callers
// of this method.
void OsRWMutex::grantReadTickets(void)
{
   if (mActiveWritersCnt == 0)
   {
      while (mRunningReadersCnt < mActiveReadersCnt)
      {
         mRunningReadersCnt++;
         mReadSem.release();
      }
   }
}

// Determine whether the resource can be granted immediately to writers
// and, if so, grant sufficient write "tickets" for all active but not
// yet running writers.
//
// The mGuard object must be acquired (and ultimately be released) by callers
// of this method.
void OsRWMutex::grantWriteTickets(void)
{
   if (mRunningReadersCnt == 0)
   {
      while (mRunningWritersCnt < mActiveWritersCnt)
      {
         mRunningWritersCnt++;
         mWriteSem.release();
      }
   }
}

/* ============================ FUNCTIONS ================================= */
