//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _UtlRscTrace_h_
#define _UtlRscTrace_h_

#define RSC_TEST
#undef RSC_TEST

#ifdef RSC_TEST

// SYSTEM INCLUDES
#include "utl/UtlRscStore.h"
#include <os/OsMutex.h>

// APPLICATION INCLUDES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:A class used to keep track of the number of resource allocations and frees
// This class is used in conjunction with instrumented versions of the global
// new and delete operators.
class UtlRscTrace
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   static unsigned long sAllocCnt; // Track the number of memory allocs
   static unsigned long sFreeCnt;  // Track the number of memory frees
   static int           sTraceFlag;// If non-zero, print a line of info
                                   //  for every call to new()/delete()
   static unsigned long sStartTime;  //
   static UtlRscStore mResourceStore;

   enum RscType
   {
          NONE          = 0,
      MEMORY    = 1,
      OSSOCKET  = 2,
      OSMSGQ    = 3,
      OSBSEM    = 4,
          OSCSEM        = 5,
          OSMUTEX       = 6,
          OSRWMUTEX     = 7,
          OSTIMER       = 8,
          OSTASK        = 9
   };


/* ============================ CREATORS ================================== */

   UtlRscTrace();
     //:Default constructor (and take a checkpoint)

   UtlRscTrace(const UtlRscTrace& rRscTrace);
     //:Copy constructor

   virtual
   ~UtlRscTrace();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   UtlRscTrace& operator=(const UtlRscTrace& rhs);
     //:Assignment operator

   virtual void checkpoint();
     //:Remember the count of outstanding memory allocations

   static void setTraceFlag(int flag) { sTraceFlag = flag; };
    //: Set trace flag, if non-zero, print a line of info

   static int enableMemTracking(pthread_t taskId = 0);
    //: Set trace flag, if non-zero, print a line of info

   static int enableMsgQTracking(pthread_t taskId = 0);
    //: Set trace flag, if non-zero, print a line of info

   static int enableBSemTracking(pthread_t taskId = 0);
    //: Set trace flag, if non-zero, print a line of info

   static int enableCSemTracking(pthread_t taskId = 0);
    //: Set trace flag, if non-zero, print a line of info

   static int enableMutexTracking(pthread_t taskId = 0);
    //: Set trace flag, if non-zero, print a line of info

   static int enableRWMutexTracking(pthread_t taskId = 0);
    //: Set trace flag, if non-zero, print a line of info

   static int enableTimerTracking(pthread_t taskId = 0);
    //: Set trace flag, if non-zero, print a line of info

   static int enableTaskTracking(pthread_t taskId = 0);
    //: Set trace flag, if non-zero, print a line of info

   static int enableSocketTracking(pthread_t taskId = 0);
    //: Set trace flag, if non-zero, print a line of info

   static int disableTracking();
    //: Set trace flag, if non-zero, print a line of info

   static void addAllocCnt(intptr_t addr,
                           pthread_t taskId);
    //: Add allocCnt for the task, used for OsSocket tracking

   static void addAllocCnt(int size,
                           intptr_t addr,
                           pthread_t taskId);
    //: Add allocCnt for the task, used for memory/OsMutex/OsRWMutex tracking

        static void addAllocCnt(int size,
                                intptr_t addr,
                                const char* name,
                                int pArg,
                                int priority,
                                int options,
                                pthread_t taskId);
    //: Add allocCnt for the task, used for OsTask tracking

        static void addAllocCnt(int state,
                                intptr_t addr,
                                int timerId,
                                int type,
                                pthread_t taskId);
    //: Add allocCnt for the task, used for OsTimer/OsCSem tracking

        static void addAllocCnt(int options,
                                intptr_t addr,
                                int state,
                                pthread_t taskId);
    //: Add allocCnt for the task, used for OsBSem tracking

        static void addAllocCnt(intptr_t addr,
                               const char* name,
                                pthread_t taskId);
    //: Add allocCnt for the task, used for OsMsgQ tracking

        static void addFreeCnt(intptr_t addr, pthread_t taskId = 0);
    //: Add freeCnt for the task

/* ============================ ACCESSORS ================================= */

   static void showMem(pthread_t taskId = 0);

   static int delta();
     //:Return the change to the number of outstanding memory allocations
     //:since the last checkpoint.

   static int delta(pthread_t taskId);
     //:Return the change to the number of outstanding memory allocations
     //:since the last checkpoint.

   static int allocCnt(pthread_t taskId = 0);
     //:Return the number of memory allocations (monotonically increasing)

   static int rscStatus();
     //:Return the number of memory frees (monotonically increasing)

   static int freeCnt(pthread_t taskId = 0);
     //:Return the number of memory frees (monotonically increasing)

   static int netAllocCnt();
     //:Return the net number of allocations (allocCnt - freeCnt)

   static int netAllocCnt(pthread_t taskId);
     //:Return the net number of allocations (allocCnt - freeCnt)

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
        static long mCheckpoint; // Used to remember the net number of
                     // allocations at time of checkpoint
        static OsMutex    *mpResourceStoreLock;

        static UtlRscStore mUtlRscStore;

        static pthread_t mTaskId;
};


/* ============================ INLINE METHODS ============================ */
#endif // RSC_TEST

#endif  // _UtlRscTrace_h_
