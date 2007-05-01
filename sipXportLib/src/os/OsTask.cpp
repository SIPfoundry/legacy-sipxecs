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

   if (!isStarted() && !isShuttingDown())
      mState = SHUT_DOWN;
   else
      mState = SHUTTING_DOWN;
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
   return (mState == SHUT_DOWN);
}

// Return TRUE if a task shutdown has been requested but not acknowledged
UtlBoolean OsTaskBase::isShuttingDown(void)
{
   return (mState == SHUTTING_DOWN);
}

// Return TRUE if the task has been started (and has not been shut down)
UtlBoolean OsTaskBase::isStarted(void)
{
   return (mState == STARTED);
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

   if (mName != "")
      OsUtil::insertKeyValue(TASK_PREFIX, mName, (int) this);
}

// Destructor
OsTaskBase::~OsTaskBase()
{
   OsStatus res;

   if (mName != "")
   {
      res = OsUtil::deleteKeyValue(TASK_PREFIX, mName);
      //assert(res == OS_SUCCESS);
   }

   mName = OsUtil::NULL_OS_STRING;
}

// Wait until the task is shut down and the run method has exited.
// Most sub classes of OsTask should call this method in
// the destructor before deleting any members which are
// accessed by the run method.
UtlBoolean OsTaskBase::waitUntilShutDown(int milliSecToWait)
{
   // If task is already shut down, just return.
   if (isShutDown())
      return TRUE;

   UtlString taskName = getName();
   if (taskName.isNull())
   {
      char b[40];
      sprintf(b, "[%p]", this);
      taskName.append(b);
   }

   if (isStarted())
   {
      requestShutdown();  // ask the task to shut itself down
      yield();            // yield the CPU so the target task can terminate
   }

   // wait up to another nineteen seconds (20 total) for the task to terminate
   // printing out a console complaint every second
   if (isShuttingDown())
   {
      int i;

      // wait up to a second for the task to terminate.
      for (i = 0; (i < 10) && isShuttingDown(); i++)
         delay(milliSecToWait/200);         // wait 1/10 second

      for (i = 1; (i < 20) && isShuttingDown(); i++)
      {
         OsSysLog::add(FAC_KERNEL, PRI_WARNING, "Task '%s' failed to terminate after %f seconds",
                  taskName.data(), (milliSecToWait * i) / 20000.0);
         delay(milliSecToWait/20);
      }

      // if still no response from the task, assume it is unresponsive and
      // destroy the object
      if (isShuttingDown())
      {
         OsSysLog::add(FAC_KERNEL, PRI_ERR, "Task '%s' failed to terminate after %f seconds",
                  taskName.data(), milliSecToWait / 1000.0);
      }
   }

   // Do not exit if not shut down
   while (isShuttingDown())
   {
         OsSysLog::add(FAC_KERNEL, PRI_ERR, "Task '%s' failed to terminate, waiting...",
                  taskName.data());
         delay(300000);
   }

   return(isShutDown());
}

// Acknowledge a shutdown request
// The platform specific entry point which calls the run
// method should call this method immediately after run exits.
// to indicate that it is now shut down.
void OsTaskBase::ackShutdown(void)
{
   OsLock lock(mDataGuard);

   assert(isStarted() || isShuttingDown() || isShutDown());

   mState = SHUT_DOWN;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
