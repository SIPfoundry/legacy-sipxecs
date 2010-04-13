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

#ifdef TEST
#include "utl/UtlMemCheck.h"
#endif //TEST

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsTask.h"
#include "os/OsCSem.h"

#ifdef OS_CSEM_DEBUG
#include "os/OsLock.h"
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */



// Constructor allowing different initial and maximum semaphore values
OsCSemBase::OsCSemBase(const int queueOptions, const int maxCount,
                     const int initCount)
#ifdef OS_CSEM_DEBUG
 : mGuard(OsBSem::Q_PRIORITY, OsBSem::FULL),
   mQueueOptions(queueOptions),
   mInitialCount(initCount),
   mMaxCount(maxCount),
   mCurCount(initCount),
   mHighCount(initCount),
   mLowCount(initCount),
   mNumAcquires(0),
   mNumReleases(0)
#endif
{
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

#ifdef OS_CSEM_DEBUG
int OsCSemBase::getValue(void)
{
   return mCurCount;
}
#endif



// Print counting semaphore information to the console
// Output enabled via a compile-time #ifdef
#ifdef OS_CSEM_DEBUG
void OsCSemBase::show(void)
{
   OsLock lock(mGuard);

   osPrintf("OK: OsCSem: OsCSem=0x%08x, options=%d, initCnt=%d, maxCnt=%d\n",
            (void *) this, mQueueOptions, mInitialCount, mMaxCount);
   osPrintf("OK: OsCSem: curCnt=%d, acquires=%d, releases=%d, highCnt=%d, lowCnt=%d\n",
            mCurCount, mNumAcquires, mNumReleases, mHighCount, mLowCount);
}
#endif


#ifdef OS_CSEM_DEBUG
// Update the statistics associated with acquiring a counting semaphore.
void OsCSemBase::updateAcquireStats(void)
{
   OsLock lock(mGuard);

   mCurCount--;
   mNumAcquires++;
   if (mCurCount < mLowCount)
      mLowCount = mCurCount;


   if ((mCurCount < -1) || (mCurCount > mMaxCount))
   {

       osPrintf("OsCSem::updateAcquireStats: OsCSem=0x%08x, mCurCount=%d, mMaxCount=%d\n",
                    (void *) this, mCurCount, mMaxCount);
       osPrintf("OsCSem: OsCSem=0x%08x, options=%d, initCnt=%d, maxCnt=%d\n",
                    (void *) this, mQueueOptions, mInitialCount, mMaxCount);
       osPrintf("OsCSem: curCnt=%d, acquires=%d, releases=%d, highCnt=%d, lowCnt=%d\n",
                    mCurCount, mNumAcquires, mNumReleases, mHighCount, mLowCount);
   }
   else
   {
#ifdef OS_CSEM_DEBUG_SHOWALL
       osPrintf("OK: OsCSem: OsCSem=0x%08x, mCurCnt=%d, maxCnt=%d\n",
                    (void *) this, mCurCount, mMaxCount);
#endif
   }

}

// Update the statistics associated with releasing a counting semaphore
void OsCSemBase::updateReleaseStats(void)
{
   OsLock lock(mGuard);

   mCurCount++;
   mNumReleases++;
   if (mCurCount > mHighCount)
      mHighCount = mCurCount;


   if ((mCurCount < 0) || (mCurCount > mMaxCount))
   {

       osPrintf("OsCSem::updateReleaseStats: OsCSem=0x%08x, mCurCount=%d, mMaxCount=%d\n",
                    (void *) this, mCurCount, mMaxCount);
       osPrintf("OsCSem: OsCSem=0x%08x, options=%d, initCnt=%d, maxCnt=%d\n",
                    (void *) this, mQueueOptions, mInitialCount, mMaxCount);
       osPrintf("OsCSem: curCnt=%d, acquires=%d, releases=%d, highCnt=%d, lowCnt=%d\n",
                     mCurCount, mNumAcquires, mNumReleases, mHighCount, mLowCount);
   }
   else
   {
#ifdef OS_CSEM_DEBUG_SHOWALL
       osPrintf("OK: OsCSem: OsCSem=0x%08x, mCurCnt=%d, maxCnt=%d\n",
                    (void *) this, mCurCount, mMaxCount);
#endif
   }
}
#endif
