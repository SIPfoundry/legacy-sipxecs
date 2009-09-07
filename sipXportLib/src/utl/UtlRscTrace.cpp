//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#include "utl/UtlRscTrace.h"

#ifdef RSC_TEST

// SYSTEM INCLUDES
#include <stdio.h>
#include <time.h>
#if defined(_WIN32)
#include <malloc.h>
#elif defined(_VXWORKS)
#include <stdLib.h>
#endif

// APPLICATION INCLUDES
#include "os/OsLock.h"

// EXTERNAL FUNCTIONS
#ifdef _VXWORKS
#include "mp/sa1100.h"
volatile int* osTimerCtr = (int*) SA1100_OSTIMER_COUNTER;
#endif

// EXTERNAL VARIABLES
// CONSTANTS
#define TIME_LENGTH             64;
#define HISTORY_LENGTH  1028;

// STATIC VARIABLE INITIALIZATIONS
unsigned long UtlRscTrace::sAllocCnt  = 0; // Number of memory allocs
unsigned long UtlRscTrace::sFreeCnt   = 0; // Number of memory frees
int           UtlRscTrace::sTraceFlag = 0; // If non-zero, trace calls to the
                                           //  new and delete operators
unsigned long UtlRscTrace::sStartTime  = 0; //
pthread_t     UtlRscTrace::mTaskId         = 0;

long              UtlRscTrace::mCheckpoint = 0L; // Used to remember the net number of
OsMutex   *UtlRscTrace::mpResourceStoreLock = new OsMutex(OsMutex::Q_PRIORITY);
UtlRscStore UtlRscTrace::mUtlRscStore = UtlRscStore();

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
UtlRscTrace::UtlRscTrace()
{
   mCheckpoint = UtlRscTrace::sAllocCnt - UtlRscTrace::sFreeCnt;
}

// Copy constructor
UtlRscTrace::UtlRscTrace(const UtlRscTrace& rUtlRscTrace)
{
   mCheckpoint = rUtlRscTrace.mCheckpoint;
}

// Destructor
UtlRscTrace::~UtlRscTrace()
{
   // no work required
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
UtlRscTrace&
UtlRscTrace::operator=(const UtlRscTrace& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   mCheckpoint = rhs.mCheckpoint;
   return *this;
}

// Remember the count of outstanding memory allocations
void UtlRscTrace::checkpoint()
{
   mCheckpoint = netAllocCnt();
}

void UtlRscTrace::addAllocCnt(int size,
                                                          intptr_t addr,
                                                          const char* name,
                                                          int pArg,
                                                          int priority,
                                                          int options,
                                                          pthread_t taskId)
{
        if (mTaskId && (mTaskId != taskId))
                return;

    mpResourceStoreLock->acquire();
        int traceFlag = sTraceFlag;
        sTraceFlag = 0;

        char buf[1024];
    long  ts;
#ifdef _VXWORKS
    ts = *osTimerCtr - sStartTime;
        sprintf(buf, "0x%08x 0x%08x %6d %12d %s %s %6d %6d %6d", taskId, addr, size, ts, taskName(taskId),
                                                                                        name, pArg, priority, options);

#else
        time( (time_t*)&ts );
        sprintf(buf, "0x%08x 0x%08x %6d %12d %s %6d %6d %6d", taskId, addr, size, ts,
                                                                                        name, pArg, priority, options);
#endif

        if (OS_SUCCESS != mUtlRscStore.insert(addr, buf))
        {
//              printf("\nUtlRscStore.insert returned error.\n");
        }
        sTraceFlag = traceFlag;
        mpResourceStoreLock->release();
}

void UtlRscTrace::addAllocCnt(intptr_t addr,
                                                          const char* name,
                                                          pthread_t taskId)
{
        if (mTaskId && (mTaskId != taskId))
                return;

    mpResourceStoreLock->acquire();
        int traceFlag = sTraceFlag;
        sTraceFlag = 0;

        char buf[1024];
    long  ts;
#ifdef _VXWORKS
    ts = *osTimerCtr - sStartTime;
        sprintf(buf, "0x%08x 0x%08x %12d %s %s", taskId, addr, ts, taskName(taskId), name);

#else
        time( (time_t*)&ts );
        sprintf(buf, "0x%08x 0x%08x %12d %s", taskId, addr, ts, name);
#endif

        if (OS_SUCCESS != mUtlRscStore.insert(addr, buf))
        {
//              printf("\nUtlRscStore.insert returned error.\n");
        }
        sTraceFlag = traceFlag;
        mpResourceStoreLock->release();
}

void UtlRscTrace::addAllocCnt(int options,
                                                          intptr_t addr,
                                                          int state,
                                                          pthread_t taskId)
{
        if (mTaskId && (mTaskId != taskId))
                return;

    mpResourceStoreLock->acquire();
        int traceFlag = sTraceFlag;
        sTraceFlag = 0;

        char buf[512];
    long  ts;
#ifdef _VXWORKS
    ts = *osTimerCtr - sStartTime;
        sprintf(buf, "0x%08x 0x%08x %6d %12d %s %6d", taskId, addr, options, ts, taskName(taskId), state);

#else
        time( (time_t*)&ts );
        sprintf(buf, "0x%08x 0x%08x %6d %12d %6d %6d", taskId, addr, options, ts, state);
#endif

        if (OS_SUCCESS != mUtlRscStore.insert(addr, buf))
        {
//              printf("\nUtlRscStore.insert returned error.\n");
        }
        sTraceFlag = traceFlag;
        mpResourceStoreLock->release();
}

void UtlRscTrace::addAllocCnt(int state,
                                                          intptr_t addr,
                                                          int timerId,
                                                          int type,
                                                          pthread_t taskId)
{
        if (mTaskId && (mTaskId != taskId))
                return;

    mpResourceStoreLock->acquire();
        int traceFlag = sTraceFlag;
        sTraceFlag = 0;

        char buf[512];
    long  ts;
#ifdef _VXWORKS
    ts = *osTimerCtr - sStartTime;
        sprintf(buf, "0x%08x 0x%08x %6d %12d %s %6d %6d", taskId, addr, state, ts, taskName(taskId), timerId, type);

#else
        time( (time_t*)&ts );
        sprintf(buf, "0x%08x 0x%08x %6d %12d %6d %6d", taskId, addr, state, ts, timerId, type);
#endif

        if (OS_SUCCESS != mUtlRscStore.insert(addr, buf))
        {
//              printf("\nUtlRscStore.insert returned error.\n");
        }
        sTraceFlag = traceFlag;
        mpResourceStoreLock->release();
}

void UtlRscTrace::addAllocCnt(int size,
                                                          intptr_t addr,
                                                          pthread_t taskId)
{
        if (mTaskId && (mTaskId != taskId))
                return;

    mpResourceStoreLock->acquire();
        int traceFlag = sTraceFlag;
        sTraceFlag = 0;

        char buf[512];
    long  ts;
#ifdef _VXWORKS
    ts = *osTimerCtr - sStartTime;
        sprintf(buf, "0x%08x 0x%08x %6d %12d %s", taskId, addr, size, ts, taskName(taskId));

#else
        time( (time_t*)&ts );
        sprintf(buf, "0x%08x 0x%08x %6d %12d", taskId, addr, size, ts);
#endif

        if (OS_SUCCESS != mUtlRscStore.insert(addr, buf))
        {
//              printf("\nUtlRscStore.insert returned error.\n");
        }
        sTraceFlag = traceFlag;
        mpResourceStoreLock->release();
}

void UtlRscTrace::addAllocCnt(intptr_t addr,
                                                          pthread_t taskId)
{
        if (mTaskId && (mTaskId != taskId))
                return;

    mpResourceStoreLock->acquire();
        int traceFlag = sTraceFlag;
        sTraceFlag = 0;

        char buf[512];
    long  ts;
#ifdef _VXWORKS
    ts = *osTimerCtr - sStartTime;
        sprintf(buf, "0x%08x 0x%08x %12d %s", taskId, addr, ts, taskName(taskId));

#else
        time( (time_t*)&ts );
        sprintf(buf, "0x%08x 0x%08x %12d", taskId, addr, ts);
#endif

        if (OS_SUCCESS != mUtlRscStore.insert(addr, buf))
        {
//              printf("\nUtlRscStore.insert returned error.\n");
        }
        sTraceFlag = traceFlag;
        mpResourceStoreLock->release();
}

void UtlRscTrace::addFreeCnt(intptr_t addr, pthread_t taskId)
{
        if (mTaskId && (mTaskId != taskId))
                return;

    mpResourceStoreLock->acquire();
        int traceFlag = sTraceFlag;
        sTraceFlag = 0;
        if (OS_SUCCESS != mUtlRscStore.remove(addr))
        {
//              printf("\nUtlRscStore.remove returned error: address - 0x%08x taskId - 0x%08x.\n", addr, taskId);
        }
        sTraceFlag = traceFlag;
        mpResourceStoreLock->release();
}


/* ============================ ACCESSORS ================================= */

// Return the change to the number of outstanding memory allocations
// since the last checkpoint.
int UtlRscTrace::delta()
{
   return netAllocCnt() - mCheckpoint;
}

int UtlRscTrace::delta(pthread_t taskId)
{
   return netAllocCnt(taskId) - mCheckpoint;
}

// Return the number of memory allocations (monotonically increasing)
int UtlRscTrace::allocCnt(pthread_t taskId)
{
        unsigned int size;
        char** activeRscs;
        int i;

    mpResourceStoreLock->acquire();
        int traceFlag = sTraceFlag;
        sTraceFlag = 0;
        size = mUtlRscStore.numEntries();

        activeRscs = (char**) new long[size];
        for (i = 0; i < size; i++)
        {
                activeRscs[i] = new char[256];
        }
        mUtlRscStore.getActiveRscs(activeRscs, size);
        sTraceFlag = traceFlag;
        mpResourceStoreLock->release();

        int id;
        unsigned long cnt = 0;
        printf("        total current alloc's: %d\n", size);
        switch (traceFlag)
        {
        case MEMORY:
                printf("  taskId     address     size    time   taskName\n");
                break;

        case OSSOCKET:
                printf("  taskId     address     time     taskName\n");
                break;

        case OSMSGQ:
                printf("  taskId     address     time     taskName           qName\n");
                break;

        case OSBSEM:
                printf("  taskId     address  options  time     taskName     state\n");
                break;

        case OSCSEM:
                printf("  taskId     address    queueOptions  time   taskName    maxCount   initCount  \n");
                break;

        case OSMUTEX:
        case OSRWMUTEX:
                printf("  taskId     address     options   time   taskName\n");
                break;

        case OSTIMER:
                printf("  taskId     address     state   time   taskName    timerId   type  \n");
                break;

        case OSTASK:
                printf("  taskId     address     size    time  taskName   taskName  pArg priority options\n");
                break;

        case NONE:
        default:
                break;
        }
        printf("============================================================================\n");
        if (taskId == 0)
        {
                for (i = 0; i < size; i++)
                {
                        cnt++;
                        printf("%s\n", activeRscs[i]);
                }
                printf("============================================================================\n");
                printf("        total current alloc's: %d\n", size);
        }
        else
        {
                for (i = 0; i < size; i++)
                {
                        sscanf(activeRscs[i], "0x%08x", &id);
                        if (id == taskId)
                        {
                                cnt++;
                                printf("%s\n", activeRscs[i]);
                        }
                }
                printf("============================================================================\n");
#ifdef _VXWORKS
                printf("current alloc's for %s 0x%08x: %d\n", taskName(taskId), taskId, cnt);
#else
                printf("current alloc's for 0x%08x: %d\n", taskId, cnt);
#endif
        }

        return cnt;
}

// Return the number of memory frees (monotonically increasing)
int UtlRscTrace::rscStatus()
{
        unsigned int nInserts;
        unsigned int nRemoves;

        mUtlRscStore.getStoreStats(nInserts, nRemoves);
        printf("inserts: %d removes: %d\n", nInserts, nRemoves);

        return (nInserts - nRemoves);
}

int UtlRscTrace::freeCnt(pthread_t taskId)
{
        unsigned int nInserts;
        unsigned int nRemoves;

        mUtlRscStore.getStoreStats(nInserts, nRemoves);
        printf("%d\n", nRemoves);

        return nRemoves;
}

// Return the net number of allocations (allocCnt - freeCnt)
int UtlRscTrace::netAllocCnt()
{
        int cnt;

        cnt = mUtlRscStore.numEntries();
        printf("%d\n", cnt);

        return cnt;
}

int UtlRscTrace::netAllocCnt(pthread_t taskId)
{
        int cnt;

        cnt = mUtlRscStore.numEntries();
        printf("%d\n", cnt);

        return cnt;
}

int UtlRscTrace::enableMemTracking(pthread_t taskId)
{
#ifdef _VXWORKS
    sStartTime = *osTimerCtr;
#endif
        mTaskId = taskId;
        sTraceFlag = MEMORY;
        return mTaskId;
}

int UtlRscTrace::enableMsgQTracking(pthread_t taskId)
{
#ifdef _VXWORKS
    sStartTime = *osTimerCtr;
#endif
        mTaskId = taskId;
        sTraceFlag = OSMSGQ;
        return mTaskId;
}

int UtlRscTrace::enableBSemTracking(pthread_t taskId)
{
#ifdef _VXWORKS
    sStartTime = *osTimerCtr;
#endif
        mTaskId = taskId;
        sTraceFlag = OSBSEM;
        return mTaskId;
}

int UtlRscTrace::enableCSemTracking(pthread_t taskId)
{
#ifdef _VXWORKS
    sStartTime = *osTimerCtr;
#endif
        mTaskId = taskId;
        sTraceFlag = OSCSEM;
        return mTaskId;
}

int UtlRscTrace::enableMutexTracking(pthread_t taskId)
{
#ifdef _VXWORKS
    sStartTime = *osTimerCtr;
#endif
        mTaskId = taskId;
        sTraceFlag = OSMUTEX;
        return mTaskId;
}

int UtlRscTrace::enableRWMutexTracking(pthread_t taskId)
{
#ifdef _VXWORKS
    sStartTime = *osTimerCtr;
#endif
        mTaskId = taskId;
        sTraceFlag = OSRWMUTEX;
        return mTaskId;
}

int UtlRscTrace::enableTimerTracking(pthread_t taskId)
{
#ifdef _VXWORKS
    sStartTime = *osTimerCtr;
#endif
        mTaskId = taskId;
        sTraceFlag = OSTIMER;
        return mTaskId;
}

int UtlRscTrace::enableTaskTracking(pthread_t taskId)
{
#ifdef _VXWORKS
    sStartTime = *osTimerCtr;
#endif
        mTaskId = taskId;
        sTraceFlag = OSTASK;
        return mTaskId;
}

int UtlRscTrace::enableSocketTracking(pthread_t taskId)
{
#ifdef _VXWORKS
    sStartTime = *osTimerCtr;
#endif
        mTaskId = taskId;
        sTraceFlag = OSSOCKET;
        return mTaskId;
}

int UtlRscTrace::disableTracking()
{
        allocCnt(0);
        sTraceFlag = NONE;
        mTaskId = 0;
        mUtlRscStore.cleanUp();
        return sTraceFlag;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

#endif // RSC_TEST
