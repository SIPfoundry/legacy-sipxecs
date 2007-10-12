//
// Copyright (C) 2004, 2005 Pingtel Corp.
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

#include <stdio.h>
#include <stdarg.h>

// APPLICATION INCLUDES
#include "os/OsExcept.h"
#include "os/OsLock.h"
#include "os/OsTask.h"
#include "os/OsUtil.h"

// EXTERNAL FUNCTIONS

// EXTERNAL VARIABLES

// CONSTANTS

// STATIC VARIABLE INITIALIZATIONS
const int OsTaskBase::DEF_OPTIONS   = 0;             // default task options
const int OsTaskBase::DEF_PRIO      = 128;           // default task priority
const int OsTaskBase::DEF_STACKSIZE = 16384;         // default task stacksize
const UtlString OsTaskBase::TASK_PREFIX("Task.");     // Task name db prefix
const UtlString OsTaskBase::TASKID_PREFIX("TaskID."); // TaskId name db prefix
int OsTaskBase::taskCount = 0; 

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

// Request a task shutdown.
// The run() method of the derived class is expected to call the
// isShuttingDown() method to detect when a task shutdown has been
// requested.
void OsTaskBase::requestShutdown(void)
{
   OsLock lock(mDataGuard);
   OsSysLog::add(FAC_KERNEL, PRI_DEBUG, "OsTaskBase::requestShutdown '%s' %s", mName.data(),
                 TaskStateName(mState));

   switch (mState)
   {
   case UNINITIALIZED:
      mState = TERMINATED;      
      break;

   case RUNNING:
      mState = SHUTTING_DOWN;      
      break;

   case SHUTTING_DOWN:
   case TERMINATED:
      // already done - not really correct, but let it go
      break;
   }
}

// Set the userData for the task.
// The class does not use this information itself, but merely stores
// it on behalf of the caller.
void OsTaskBase::setUserData(int data)
{
   mUserData = data;
}

// Adds a syslog entry of the given facility and priority
OsStatus OsTaskBase::syslog(const OsSysLogFacility facility,
                            const OsSysLogPriority priority,
                            const char*            format,
                                                    ...)
{
   int taskId;
   int processId;

   if (OsSysLog::willLog(facility, priority))
   {
      id(taskId);
      processId = 0;

      va_list ap;
      va_start(ap, format);

      OsSysLog::vadd(mName.data(), taskId, facility, priority,
            format, ap) ;
      va_end(ap);
   }

   return OS_SUCCESS ;
}


/* ============================ ACCESSORS ================================= */

// Get the void* value passed as an argument to the task
void* OsTaskBase::getArg(void)
{
   return mpArg;
}

// Get the name associated with the task
const UtlString& OsTaskBase::getName(void)
{
   return mName;
}

// Return the userData for the task.
int OsTaskBase::getUserData(void)
{
   return mUserData;
}

void OsTaskBase::yield(void)
{
    OsTask::yield();
}

OsStatus OsTaskBase::delay(const int milliSecs)
{
    return(OsTask::delay(milliSecs));
}

/* ============================ INQUIRY =================================== */

// Return TRUE if a task shutdown has been requested and acknowledged
UtlBoolean OsTaskBase::isShutDown(void)
{
   return (TERMINATED == mState);
}

// Return TRUE if a task shutdown has been requested but not acknowledged
UtlBoolean OsTaskBase::isShuttingDown(void)
{
   return (SHUTTING_DOWN == mState);
}

// Return TRUE if the task has been started (and has not been shut down)
UtlBoolean OsTaskBase::isStarted(void)
{
   return (RUNNING == mState);
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

// Constructor
OsTaskBase::OsTaskBase(const UtlString& name,
                       void* pArg,
                       const int priority,
                       const int options,
                       const int stackSize)
:  mDataGuard(OsMutex::Q_PRIORITY + OsMutex::INVERSION_SAFE),
   mState(UNINITIALIZED),
   mpArg(pArg),
   mUserData(0)
{
        // If name contains %d insert the task count/index
    assert(name.length() < 240);
    char nameBuffer[256];
    sprintf(nameBuffer, name.data(), taskCount++);
    mName.append(nameBuffer);

    OsSysLog::add(FAC_KERNEL, PRI_DEBUG, "OsTask::_ '%s' created %p", mName.data(), this);

    if (!mName.isNull())
    {
      OsUtil::insertKeyValue(TASK_PREFIX, mName, (int) this);
    }
}

// Destructor
OsTaskBase::~OsTaskBase()
{
   if (!mName.isNull())
   {
      OsUtil::deleteKeyValue(TASK_PREFIX, mName);
   }

   mName.remove(0);
}

// Acknowledge a shutdown request.
void OsTaskBase::ackShutdown(void)
{
   OsLock lock(mDataGuard);

   switch (mState)
   {
   case UNINITIALIZED:
   case RUNNING:
   case SHUTTING_DOWN:
      mState = TERMINATED;
      break;

   case TERMINATED:
      // already done - not really correct, but let it go
      break;
   }
}

const char* OsTaskBase::TaskStateName(enum TaskState state)
{
   const char* StateName[] =
   {
      "UNINITIALIZED",
      "RUNNING",
      "SHUTTING_DOWN",
      "TERMINATED",
      "<INVALID>"
   };
   const char* name;

   switch (state)
   {
   case UNINITIALIZED:
   case RUNNING:
   case SHUTTING_DOWN:
   case TERMINATED:
      name = StateName[state];
      break;
   default:
      name = StateName[4];
   }

   return name;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
