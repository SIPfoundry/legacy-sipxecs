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
#include "os/OsMsgPool.h"
#include "utl/UtlString.h"
#include "os/OsSysLog.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

   // Default constructor.  model is a message of the single type that
   // will be contained in the pool, and its createCopy virtual method
   // will be used to populate the pool.  The caller disposes of model
OsMsgPool::OsMsgPool(const char* name,
   const OsMsg& model,
   int initialCount,
   int softLimit,
   int hardLimit,
   int increment,
   OsMsgPoolSharing sharing)
: mIncrement(increment), mNext(0)
{
   int i;
   OsMsg* pMsg;
   mpMutex = NULL;
   mCurrentCount = 0;
   mpModel = model.createCopy();
   mpModel->setReusable(TRUE);
   mpModel->setInUse(FALSE);

   mpName = new UtlString((NULL == name) ? "Unknown" : name);

   mInitialCount = (initialCount > 1) ? initialCount : 10;
   mSoftLimit = (mInitialCount > softLimit) ? mInitialCount : softLimit;
   mHardLimit = (mSoftLimit > hardLimit) ? mSoftLimit : hardLimit;

   if (mHardLimit > mInitialCount) {
      assert(mIncrement>0);
      mIncrement = (mIncrement>0) ? mIncrement : 1;
   }

   mpElts = new OsMsg*[mHardLimit];

   for (i=0; i<mHardLimit; i++) mpElts[i] = NULL;

   for (i=0; i<mInitialCount; i++) {
      pMsg = mpModel->createCopy();
      if (NULL != pMsg) {
         pMsg->setReusable(TRUE);
         pMsg->setInUse(FALSE);
         mpElts[i] = pMsg;
         mCurrentCount++;
      }
   }

   if (MULTIPLE_CLIENTS == sharing) {
      mpMutex = new OsMutex(OsMutex::Q_PRIORITY |
                            OsMutex::DELETE_SAFE |
                            OsMutex::INVERSION_SAFE);
      assert(NULL != mpMutex);
   }
}

// Destructor
OsMsgPool::~OsMsgPool()
{
   // Hmmm...
   int i;
   OsMsg* pMsg;

   if (NULL != mpMutex) mpMutex->acquire();
   for (i=0; i<mCurrentCount; i++) {
      pMsg = mpElts[i];
      if (NULL != pMsg) {
         pMsg->setReusable(FALSE);
         if (!pMsg->isMsgInUse()) {
            mpElts[i] = NULL;
            delete pMsg;
         }
      }
   }
   delete[] mpElts;
   mpModel->setReusable(FALSE);
   delete mpModel;
   delete mpMutex;
   delete mpName;
}

/* ============================ MANIPULATORS ============================== */

   // Find and return an available element of the pool, creating more if
   // necessary and permitted.  Return NULL if failure.

OsMsg* OsMsgPool::findFreeMsg()
{
   int i;
   OsMsg* pMsg;
   OsMsg* ret = NULL;

   // If there is a mutex for this pool, acquire it before doing any work.
   if (NULL != mpMutex)
   {
      mpMutex->acquire();
   }

   // Scan mNext through the table looking for a message that is
   // allocated and not in use.
   for (i=0; ((i<mCurrentCount)&&(NULL==ret)); i++)
   {
      // Examine the element.
      pMsg = mpElts[mNext];
      if ((NULL != pMsg) && !pMsg->isMsgInUse())
      {
         pMsg->setInUse(TRUE);
         ret = pMsg;
      }
      // Advance mNext, wrapping around if it reaches mCurrentCount.
      mNext++;
      if (mNext >= mCurrentCount)
      {
         mNext = 0;
      }
   }

   // If no free message was found.
   if (NULL == ret)
   {
      if (mCurrentCount > mSoftLimit)
      {
         if (mSoftLimit <= mHardLimit)
         {
            OsSysLog::add(FAC_KERNEL, PRI_WARNING,
                          "OsMsgPool::FindFreeMsg '%s' queue size (%d) exceeds soft limit (%d)\n",
                          mpName->data(), mCurrentCount, mSoftLimit);
         }
      }

      if (mCurrentCount < mHardLimit)
      {
         int limit;

         mNext = mCurrentCount;
         limit = mCurrentCount + mIncrement;
         if (limit > mHardLimit) limit = mHardLimit;
         // Create the new elements.
         for (i=mCurrentCount; i<limit; i++)
         {
            pMsg = mpModel->createCopy();
            if (NULL != pMsg)
            {
               pMsg->setReusable(TRUE);
               pMsg->setInUse(FALSE);
               mpElts[i] = pMsg;
               mCurrentCount++;
            }
         }

         ret = mpElts[mNext];
         assert(NULL!=ret);

         if ((NULL != ret) && !ret->isMsgInUse())
         {
            ret->setInUse(TRUE);
         }
         mNext++;

         if (mNext >= mCurrentCount)
         {
            mNext = 0;
         }
      }
      else
      {
         if (mSoftLimit <= mHardLimit)
         {
            OsSysLog::add(FAC_KERNEL, PRI_CRIT,
                          "OsMsgPool::FindFreeMsg '%s' queue size (%d) exceeds hard limit (%d)\n",
                          mpName->data(), mCurrentCount, mHardLimit);
         }

         mSoftLimit = mHardLimit + 1;
      }
   }

   // If there is a mutex for this pool, release it.
   if (NULL != mpMutex)
   {
      mpMutex->release();
   }
   return ret;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

// Return the number of items in use.
int OsMsgPool::getNoInUse(void)
{
   int i, count;

   if (NULL != mpMutex)
   {
      mpMutex->acquire();
   }

   count = 0;
   for (i=0; i < mCurrentCount; i++)
   {
      if (mpElts[i] != NULL && mpElts[i]->isMsgInUse())
      {
         count++;
      }
   }

   if (NULL != mpMutex)
   {
      mpMutex->release();
   }

   return count;
}

// Return the current soft limit.
int OsMsgPool::getSoftLimit(void)
{
   return mSoftLimit;
}

// Return the current hard limit.
int OsMsgPool::getHardLimit(void)
{
   return mHardLimit;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
