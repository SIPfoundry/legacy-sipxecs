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
#include <ctype.h>

// APPLICATION INCLUDES
#include "os/OsReadLock.h"
#include "os/OsWriteLock.h"
#include "ps/PsLampInfo.h"
#include "ps/PsLampTask.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
PsLampTask* PsLampTask::spInstance = 0;
OsBSem      PsLampTask::sLock(OsBSem::Q_PRIORITY, OsBSem::FULL);

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Return a pointer to the Lamp task, creating it if necessary
PsLampTask* PsLampTask::getLampTask(void)
{
   UtlBoolean isStarted;

   // If the task object already exists, and the corresponding low-level task
   // has been started, then use it
   if (spInstance != NULL && spInstance->isStarted())
      return spInstance;

   // If the task does not yet exist or hasn't been started, then acquire
   // the lock to ensure that only one instance of the task is started
   sLock.acquire();
   if (spInstance == NULL)
       spInstance = new PsLampTask();

   isStarted = spInstance->isStarted();
   if (!isStarted)
   {
      isStarted = spInstance->start();
      assert(isStarted);
   }
   sLock.release();

   return spInstance;
}

// Destructor
PsLampTask::~PsLampTask()
{
   OsWriteLock lock(mMutex);       // acquire a write lock

   doCleanup();
   delete mpLampDev;
   spInstance = NULL;
}

/* ============================ MANIPULATORS ============================== */

// Cause the Lamp task to (re)initialize itself.
// The task will allocate an array [0..maxLampIndex] of PsLampInfo
// objects to hold lamp state.
OsStatus PsLampTask::init(const int maxLampIndex)
{
   OsWriteLock lock(mMutex);      // acquire a write lock

   doCleanup();                   // release old dynamic storage (if any)

   mMaxLampIdx = maxLampIndex;
   mpLampInfo  = new PsLampInfo[maxLampIndex+1];

   // calculate the common multiple for all of the lamp modes (in ticks)
   //  this value will be used to determine when it is safe to reset the
   //  mTickCnt value to zero.
   mModeTickMultiple = (FLASH_ON_TICKS   + FLASH_OFF_TICKS) *
                       (FLUTTER_ON_TICKS + FLUTTER_OFF_TICKS) *
                       (WINK_ON_TICKS    + WINK_OFF_TICKS);

   // initialize all lamps to OFF
   mOnLamps                = 0x0;
   mModeBrokenFlutterLamps = 0x0;
   mModeFlashLamps         = 0x0;
   mModeFlutterLamps       = 0x0;
   mModeSteadyLamps        = 0x0;
   mModeWinkLamps          = 0x0;

   return OS_SUCCESS;
}

// Set the lamp information for the lamp designated by "index".
// Returns OS_NOT_FOUND if the index is out of range.
OsStatus PsLampTask::setLampInfo(int index,
                                 int lampId,
                                 const char* lampName,
                                 PsLampInfo::LampMode lampMode)
{
   OsWriteLock lock(mMutex);      // acquire a write lock
   PsLampInfo  lampInfo(lampId, lampName, lampMode);

   if ((index < 0) || (index > mMaxLampIdx))
   {
      assert(FALSE);
      return OS_NOT_FOUND;
   }

   mpLampInfo[index] = lampInfo;
   calculateLampModeAggregates();
   return OS_SUCCESS;
}

// Set the mode for the lamp indicated by lampId.
// Returns OS_NOT_FOUND if there is no lamp with that lampId.
OsStatus PsLampTask::setMode(int lampId, PsLampInfo::LampMode lampMode)
{
   int i;
   PsLampInfo* pLampInfo;
   OsWriteLock lock(mMutex);      // acquire a write lock

   // assert(mpLampInfo != NULL);
   if (mpLampInfo != NULL) {
      for (i=0; i <= mMaxLampIdx; i++)
          {
         pLampInfo = &(mpLampInfo[i]);
                 if (pLampInfo && (pLampInfo->getId() == lampId))
                 {
            pLampInfo->setMode(lampMode);
                        calculateLampModeAggregates();
                        return OS_SUCCESS;
                 }
          }
   }

   return OS_NOT_FOUND;
}

// Set the mode for the lamp indicated by pLampName
// Returns OS_NOT_FOUND if there is no lamp with that name.
OsStatus PsLampTask::setMode(const char* pLampName,
                             PsLampInfo::LampMode lampMode)
{
   int i;
   PsLampInfo* pLampInfo;
   OsWriteLock lock(mMutex);      // acquire a write lock

   // assert(mpLampInfo != NULL);
   if (mpLampInfo != NULL) {
      for (i=0; i <= mMaxLampIdx; i++)
          {
         pLampInfo = &(mpLampInfo[i]);
         if (pLampInfo && (strcmp(pLampInfo->getName(), pLampName) == 0))
                 {
            pLampInfo->setMode(lampMode);
            calculateLampModeAggregates();
            return OS_SUCCESS;
                 }
          }
   }

   return OS_NOT_FOUND;
}

/* ============================ ACCESSORS ================================= */

// Return the lamp information for the lamp designated by "index".
const PsLampInfo& PsLampTask::getLampInfo(const int index)
{
   OsReadLock lock(mMutex);       // acquire a read lock

   assert((mpLampInfo != NULL) && (index <= mMaxLampIdx));
   return mpLampInfo[index];
}

// Returns the max index for the array of PsLampInfo objects.
int PsLampTask::getMaxLampIndex(void) const
{
   return mMaxLampIdx;
}

// Get the current mode for the lamp designated by lampId.
// The mode is returned in the "rMode" variable.
// Returns OS_NOT_FOUND if there is no lamp with that lampId.
OsStatus PsLampTask::getMode(int lampId, PsLampInfo::LampMode& rMode)
{
   int i;
   PsLampInfo* pLampInfo;
   OsReadLock lock(mMutex);      // acquire a read lock

   assert(mpLampInfo != NULL);
   for (i=0; i <= mMaxLampIdx; i++)
   {
      pLampInfo = &(mpLampInfo[i]);
      if (pLampInfo && (pLampInfo->getId() == lampId))
      {
         rMode = pLampInfo->getMode();
         return OS_SUCCESS;
      }
   }

   rMode = PsLampInfo::OFF;
   return OS_NOT_FOUND;
}

// Get the current mode for the lamp designated by pLampName.
// The mode is returned in the "rMode" variable.
// Returns OS_NOT_FOUND if there is no lamp with that name.
OsStatus PsLampTask::getMode(const char* pLampName,
                             PsLampInfo::LampMode& rMode)
{
   int i;
   PsLampInfo* pLampInfo;
   OsReadLock lock(mMutex);      // acquire a read lock

   assert(mpLampInfo != NULL);
   for (i=0; i <= mMaxLampIdx; i++)
   {
      pLampInfo = &(mpLampInfo[i]);
      if (pLampInfo && (strcmp(pLampInfo->getName(), pLampName) == 0))
      {
         rMode = pLampInfo->getMode();
         return OS_SUCCESS;
      }
   }

   rMode = PsLampInfo::OFF;
   return OS_NOT_FOUND;
}

// Returns the name for the lamp designated by lampId.
// The name is returned in the "rpName" variable.
// Returns OS_NOT_FOUND if there is no lamp with that lampId.
OsStatus PsLampTask::getName(int lampId, const char*& rpName)
{
   int i;
   PsLampInfo* pLampInfo;
   OsReadLock lock(mMutex);      // acquire a read lock

   assert(mpLampInfo != NULL);
   for (i=0; i <= mMaxLampIdx; i++)
   {
      pLampInfo = &(mpLampInfo[i]);
      if (pLampInfo && (pLampInfo->getId() == lampId))
      {
         rpName = pLampInfo->getName();
         return OS_SUCCESS;
      }
   }

   rpName = "";
   return OS_NOT_FOUND;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

// Default constructor (called only indirectly via getLampTask())
PsLampTask::PsLampTask()
:  OsTask("PsLamp"),
   mMaxLampIdx(-1),                // not yet initialized
   mMutex(OsRWMutex::Q_PRIORITY),  // create mutex for protecting data
   mTickCnt(0),
   mpLampInfo(NULL)                // not yet initialized
{
   mpLampDev = PsLampDev::getLampDev(this);
}

// The body of the task -- responsible for updating the lamps as needed.
int PsLampTask::run(void* pArg)
{
   UtlBoolean     doShutdown;
   UtlBoolean     isFlashOn;
   UtlBoolean     isFlutterOn;
   UtlBoolean     isWinkOn;
   unsigned long onLamps;

   do
   {
      doShutdown = isShuttingDown();
      if (!doShutdown && mpLampInfo)
      {
         OsReadLock lock(mMutex);   // acquire a read lock

         mTickCnt++;                        // to avoid problems with
         if (mTickCnt == mModeTickMultiple) //  wrapping reset the tick
            mTickCnt = 0;                   //  count to zero when it
                                            //  reaches mModeTickMultiple

         // determine which lamp modes are in effect for this tick
         isWinkOn    = ((mTickCnt % (WINK_ON_TICKS + WINK_OFF_TICKS))
                       < WINK_ON_TICKS);
         isFlashOn   = ((mTickCnt % (FLASH_ON_TICKS + FLASH_OFF_TICKS))
                       < FLASH_ON_TICKS);
         isFlutterOn = ((mTickCnt % (FLUTTER_ON_TICKS + FLUTTER_OFF_TICKS))
                       < FLUTTER_ON_TICKS);

         // determine which lamps should be on for this tick
         onLamps = mModeSteadyLamps;
         if (isFlashOn)                onLamps |= mModeFlashLamps;
         if (isFlutterOn)              onLamps |= mModeFlutterLamps;
         if (isFlashOn && isFlutterOn) onLamps |= mModeBrokenFlutterLamps;
         if (isWinkOn)                 onLamps |= mModeWinkLamps;

         // if the lamp settings have changed, then tell the lamp device
         //  manager which lamps to turn on
         if (onLamps != mOnLamps)
         {
            mpLampDev->lightLamps(onLamps);
            // osPrintf("PsLampDev::lightLamps(0x%x), tickCnt=%d\n",
            //          onLamps, mTickCnt);
            mOnLamps = onLamps;
         }
      }

      delay(TICK_PERIOD_MSECS);
   }
   while (!doShutdown);

   return 0;                // and then exit
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

// Calculate the lamp mode aggregates (the lamp IDs that are turned on for
// each mode)
void PsLampTask::calculateLampModeAggregates(void)
{
   int         i;
   PsLampInfo* pLampInfo;

   mModeBrokenFlutterLamps = 0x0;
   mModeFlashLamps         = 0x0;
   mModeFlutterLamps       = 0x0;
   mModeSteadyLamps        = 0x0;
   mModeWinkLamps          = 0x0;

   assert(mpLampInfo != NULL);
   for (i=0; i <= mMaxLampIdx; i++)
   {
      pLampInfo = &(mpLampInfo[i]);
      switch (pLampInfo->getMode())
      {
      case PsLampInfo::BROKEN_FLUTTER:
         mModeBrokenFlutterLamps |= (unsigned long) pLampInfo->getId();;
         break;
      case PsLampInfo::FLASH:
         mModeFlashLamps |= (unsigned long) pLampInfo->getId();;
         break;
      case PsLampInfo::FLUTTER:
         mModeFlutterLamps |= (unsigned long) pLampInfo->getId();;
         break;
      case PsLampInfo::OFF:
         // do nothing
         break;
      case PsLampInfo::STEADY:
         mModeSteadyLamps |= (unsigned long) pLampInfo->getId();;
         break;
      case PsLampInfo::WINK:
         mModeWinkLamps |= (unsigned long) pLampInfo->getId();;
         break;
      default:
         assert(FALSE);
         break;
      }
   }
}

// Release dynamically allocated storage.
// A write lock should be acquired before calling this method.
void PsLampTask::doCleanup(void)
{
   // destroy the array of lamp info objects
   if (mpLampInfo != NULL)
   {
      delete[] mpLampInfo;
      mpLampInfo = NULL;
   }
}

/* ============================ FUNCTIONS ================================= */
