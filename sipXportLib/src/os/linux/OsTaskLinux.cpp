//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//
// $$
////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <sys/mman.h>

// APPLICATION INCLUDES
#include "os/OsExcept.h"
#include "os/OsLock.h"
#include "os/OsUtil.h"
#include "os/linux/OsLinuxDefs.h"
#include "os/linux/OsTaskLinux.h"
#include "os/linux/OsUtilLinux.h"

// EXTERNAL FUNCTIONS

// EXTERNAL VARIABLES

// CONSTANTS

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsTaskLinux::OsTaskLinux(const UtlString& name,
                     void* pArg,
                     const int priority,
                     const int options,
                     const int stackSize)
:  OsTaskBase(name, pArg, priority, options, stackSize),
   mTaskId(0),
   mDeleteGuard(OsRWMutex::Q_PRIORITY),
   mSuspendCnt(0),
   mOptions(options),
   mPriority(priority),
   mStackSize(stackSize)
{
   // other than initialization, no work required
}

// Destructor
OsTaskLinux::~OsTaskLinux()
{
   waitUntilShutDown();
   doLinuxTerminateTask(FALSE);
}

// Delete the task even if the task is protected from deletion.
// After calling this method, the user will still need to delete the
// corresponding OsTask object to reclaim its storage.
OsStatus OsTaskLinux::deleteForce(void)
{
   OsLock lock(mDataGuard);

   doLinuxTerminateTask(TRUE);

   return OS_SUCCESS;
}
/* ============================ MANIPULATORS ============================== */

// Restart the task.
// The task is first terminated, and then reinitialized with the same
// name, priority, options, stack size, original entry point, and
// parameters it had when it was terminated.
// Return TRUE if the restart of the task is successful.
UtlBoolean OsTaskLinux::restart(void)
{
   OsLock lock(mDataGuard);

   doLinuxTerminateTask(FALSE);
   return doLinuxCreateTask(getName());
}

// Resume the task.
// This routine resumes the task. The task suspension is cleared, and
// the task operates in the remaining state.
OsStatus OsTaskLinux::resume(void)
{
   OsLock lock(mDataGuard);

   if (!isStarted())
      return OS_TASK_NOT_STARTED;

   if (mSuspendCnt < 1) // we're already running
      return OS_SUCCESS;

   if (--mSuspendCnt == 0) // predecrement to perform the test afterward
      pthread_kill(mTaskId, SIGCONT);
   return OS_SUCCESS;
}

// Spawn a new task and invoke its run() method.
// Return TRUE if the spawning of the new task is successful.
// Return FALSE if the task spawn fails or if the task has already
// been started.
UtlBoolean OsTaskLinux::start(void)
{
   OsLock lock(mDataGuard);

   if (isStarted())
      return FALSE;

   return doLinuxCreateTask(getName());
}
     
// Suspend the task.
// This routine suspends the task. Suspension is additive: thus, tasks
// can be delayed and suspended, or pended and suspended. Suspended,
// delayed tasks whose delays expire remain suspended. Likewise,
// suspended, pended tasks that unblock remain suspended only.
OsStatus OsTaskLinux::suspend(void)
{
   OsLock lock(mDataGuard);

   if (!isStarted())
      return OS_TASK_NOT_STARTED;

   if (mSuspendCnt++ == 0) // postincrement to perform the test beforehand
      pthread_kill(mTaskId, SIGSTOP);
   return OS_SUCCESS;
}

// Set the errno status for the task.
OsStatus OsTaskLinux::setErrno(int errno)
{
   if (!isStarted())
      return OS_TASK_NOT_STARTED;

   return OS_SUCCESS;
}

// Set the execution options for the task
// The only option that can be changed after a task has been created
// is whether to allow breakpoint debugging.
OsStatus OsTaskLinux::setOptions(int options)
{
   return OS_NOT_YET_IMPLEMENTED;
}

// Set the priority of the task.
// Priorities range from 0, the highest priority, to 255, the lowest priority.
OsStatus OsTaskLinux::setPriority(int priority)
{
   int    linuxRes;
   int    policy;
   struct sched_param param;

   OsLock lock(mDataGuard);

   if (!isStarted()) {
      mPriority = priority; // save mPriority for later use
      return OS_TASK_NOT_STARTED;
   }

   pthread_getschedparam(mTaskId, &policy, &param);
   param.sched_priority = OsUtilLinux::cvtOsPrioToLinuxPrio(priority);
   linuxRes = pthread_setschedparam(mTaskId, policy, &param);

   if (linuxRes == POSIX_OK)
   {
         mPriority = priority;
         return OS_SUCCESS;
   }

   return OS_INVALID_PRIORITY;
}

// Add a task variable to the task.
// This routine adds a specified variable pVar (4-byte memory
// location) to its task's context. After calling this routine, the
// variable is private to the task. The task can access and modify
// the variable, but the modifications are not visible to other tasks,
// and other tasks' modifications to that variable do not affect the
// value seen by the task. This is accomplished by saving and restoring
// the variable's value each time a task switch occurs to or from the
// calling task.
OsStatus OsTaskLinux::varAdd(int* pVar)
{
   if (!isStarted())
      return OS_TASK_NOT_STARTED;

   return OS_NOT_YET_IMPLEMENTED;
}

// Remove a task variable from the task.
// This routine removes a specified task variable, pVar, from its
// task's context. The private value of that variable is lost.
OsStatus OsTaskLinux::varDelete(int* pVar)
{
   if (!isStarted())
      return OS_TASK_NOT_STARTED;

   return OS_NOT_YET_IMPLEMENTED;
}

// Set the value of a private task variable.
// This routine sets the private value of the task variable for a
// specified task. The specified task is usually not the calling task,
// which can set its private value by directly modifying the variable.
// This routine is provided primarily for debugging purposes.
OsStatus OsTaskLinux::varSet(int* pVar, int value)
{
   if (!isStarted())
      return OS_TASK_NOT_STARTED;

   return OS_NOT_YET_IMPLEMENTED;
}

// Delay a task from executing for the specified number of milliseconds.
// This routine causes the calling task to relinquish the CPU for the
// duration specified. This is commonly referred to as manual
// rescheduling, but it is also useful when waiting for some external
// condition that does not have an interrupt associated with it.
OsStatus OsTaskLinux::delay(const int milliSecs)
{
   struct timespec ts;
   assert(milliSecs >= 0);       // negative delays don't make sense

   ts.tv_sec = milliSecs / 1000;
   ts.tv_nsec = (milliSecs % 1000) * 1000000;
   nanosleep(&ts, NULL);

   return OS_SUCCESS;
}

// Disable rescheduling for the currently executing task.
// This routine disables task context switching. The task that calls
// this routine will be the only task that is allowed to execute,
// unless the task explicitly gives up the CPU by making itself no
// longer ready. Typically this call is paired with unlock();
// together they surround a critical section of code. These
// preemption locks are implemented with a counting variable that
// allows nested preemption locks. Preemption will not be unlocked
// until unlock() has been called as many times as lock().
OsStatus OsTaskLinux::lock(void)
{
   return OS_NOT_YET_IMPLEMENTED;
}

// Enable rescheduling for the currently executing task.
// This routine decrements the preemption lock count. Typically
// this call is paired with lock() and concludes a critical
// section of code. Preemption will not be unlocked until
// unlock() has been called as many times as lock(). When
// the lock count is decremented to zero, any tasks that were
// eligible to preempt the current task will execute.
OsStatus OsTaskLinux::unlock(void)
{
   return OS_NOT_YET_IMPLEMENTED;
}

// Make the calling task safe from deletion.
// This routine protects the calling task from deletion. Tasks that
// attempt to delete a protected task will block until the task is
// made unsafe, using unsafe(). When a task becomes unsafe, the
// deleter will be unblocked and allowed to delete the task.
// The safe() primitive utilizes a count to keep track of
// nested calls for task protection. When nesting occurs,
// the task becomes unsafe only after the outermost unsafe()
// is executed.
OsStatus OsTaskLinux::safe(void)
{
   OsTask*  pTask;
   OsStatus res;

   pTask = getCurrentTask();
   res = pTask->mDeleteGuard.acquireRead();
   assert(res == OS_SUCCESS);

   return res;
}

// Make the calling task unsafe from deletion.
// This routine removes the calling task's protection from deletion.
// Tasks that attempt to delete a protected task will block until the
// task is unsafe. When a task becomes unsafe, the deleter will be
// unblocked and allowed to delete the task.
// The unsafe() primitive utilizes a count to keep track of nested
// calls for task protection. When nesting occurs, the task becomes
// unsafe only after the outermost unsafe() is executed.
OsStatus OsTaskLinux::unsafe(void)
{
   OsTask*  pTask;
   OsStatus res;

   pTask = getCurrentTask();
   res = pTask->mDeleteGuard.releaseRead();
   assert(res == OS_SUCCESS);

   return res;
}

// Yield the CPU if a task of equal or higher priority is ready to run.
void OsTaskLinux::yield(void)
{
   delay(0);
}

/* ============================ ACCESSORS ================================= */

// Return a pointer to the OsTask object for the currently executing task
// Return NULL if none exists.
OsTaskLinux* OsTaskLinux::getCurrentTask(void)
{
   int taskId;

   taskId = (int)pthread_self();
   return OsTaskLinux::getTaskById(taskId);
}

// Return an Id of the currently executing task
OsStatus OsTaskLinux::getCurrentTaskId(int &rid)
{
   rid = (int)pthread_self();
   return OS_SUCCESS;
}

// Return a pointer to the OsTask object corresponding to the named task
// Return NULL if there is no task object with that name.
OsTaskLinux* OsTaskLinux::getTaskByName(const UtlString& taskName)
{
   OsStatus res;
   int      val;

   res = OsUtil::lookupKeyValue(TASK_PREFIX, taskName, &val);
   assert(res == OS_SUCCESS || res == OS_NOT_FOUND);

   if (res == OS_SUCCESS)
   {
      assert(val != 0);
      return ((OsTaskLinux*) val);
   }
   else
      return NULL;
}

// Return a pointer to the OsTask object corresponding to taskId
// Return NULL is there is no task object with that id.
OsTaskLinux* OsTaskLinux::getTaskById(const int taskId)
{
   char     idString[15];
   OsStatus res;
   int      val;

   sprintf(idString, "%d", taskId);   // convert the id to a string
   res = OsUtil::lookupKeyValue(TASKID_PREFIX, idString, &val);
   assert(res == OS_SUCCESS || res == OS_NOT_FOUND);

   if (res == OS_SUCCESS)
   {
      assert(val != 0);
      return ((OsTaskLinux*) val);
   }
   else
      return NULL;

}

// Get the errno status for the task
// We do have per-thread errno's under Linux, but there's no way to
// get them except from the thread itself. We could use a message to
// the thread to report back its errno, but then we'd have to wait
// for it to run and get the message. Solution: don't bother.
OsStatus OsTaskLinux::getErrno(int& rErrno)
{
   if (!isStarted())
      return OS_TASK_NOT_STARTED;

   rErrno = 0; // no error

   return OS_SUCCESS;
}

// Return the execution options for the task
int OsTaskLinux::getOptions(void)
{
   return mOptions;
}
     
// Return the priority of the task
OsStatus OsTaskLinux::getPriority(int& rPriority)
{
//   int    linuxRes;
//   int    policy;
//   struct sched_param param;

   if (!isStarted())
      return OS_TASK_NOT_STARTED;

//   linuxRes = pthread_getschedparam(mTaskId, &policy, &param);
//   assert(linuxRes == POSIX_OK);

//   rPriority = OsUtilLinux::cvtLinuxPrioToOsPrio(param.sched_priority);
   rPriority = mPriority;

   return OS_SUCCESS;
}
     
// Get the value of a task variable.
// This routine returns the private value of a task variable for its
// task. The task is usually not the calling task, which can get its
// private value by directly accessing the variable. This routine is
// provided primarily for debugging purposes.
OsStatus OsTaskLinux::varGet(void)
{
   if (!isStarted())
      return OS_TASK_NOT_STARTED;

   return OS_NOT_YET_IMPLEMENTED;
}

/* ============================ INQUIRY =================================== */

// Get the task ID for this task
OsStatus OsTaskLinux::id(int& rId)
{
   OsStatus retVal = OS_SUCCESS;
   
   //if started, return the taskId, otherwise return -1
   if (isStarted())
      rId = (int)mTaskId;
   else
   {
      retVal = OS_TASK_NOT_STARTED;
      rId = -1;  
   }


   return retVal;
}

// Check if the task is ready to run
// Return TRUE is the task is ready, otherwise FALSE.
// Under Linux, this method returns the opposite of isSuspended()
UtlBoolean OsTaskLinux::isReady(void)
{
   if (!isStarted())
      return FALSE;
   
   return (!isSuspended());
}

// Check if the task is suspended.
// Return TRUE is the task is suspended, otherwise FALSE.
UtlBoolean OsTaskLinux::isSuspended(void)
{
   OsLock lock(mDataGuard);

   if (!isStarted())
      return FALSE;

   return mSuspendCnt > 0;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

// Do the real work associated with creating a new Linux task
UtlBoolean OsTaskLinux::doLinuxCreateTask(const char* pTaskName)
{
   int                       linuxRes;
   char                      idString[15];
   pthread_attr_t            attributes;

  // construct thread attribute
   linuxRes = pthread_attr_init(&attributes);
   if (linuxRes != POSIX_OK) {
      OsSysLog::add(FAC_KERNEL, PRI_ERR, "doLinuxCreateTask: pthread_attr_init failed (%d) ", linuxRes);
   }

   // Instead of using the default 2M as stack size, reduce it to 1M
   size_t stacksize = 0;
   linuxRes = pthread_attr_getstacksize(&attributes, &stacksize);
   if (linuxRes != POSIX_OK) {
      OsSysLog::add(FAC_KERNEL, PRI_ERR, "OsTaskLinux:doLinuxCreateTask pthread_attr_getstacksize error, returned %d", linuxRes);
   } else {
      linuxRes = pthread_attr_setstacksize(&attributes, OSTASK_STACK_SIZE_1M);
      if (linuxRes != POSIX_OK)
         OsSysLog::add(FAC_KERNEL, PRI_ERR, "OsTaskLinux:doLinuxCreateTask pthread_attr_setstacksize error, returned %d", linuxRes);
   }

   // Create threads detached
   linuxRes = pthread_attr_setdetachstate(&attributes, PTHREAD_CREATE_DETACHED);
   if (linuxRes != POSIX_OK) {
      OsSysLog::add(FAC_KERNEL, PRI_ERR, "OsTaskLinux:doLinuxCreateTask pthread_attr_setdetachstate error, returned %d", linuxRes);
   }
   
   linuxRes = pthread_create(&mTaskId, &attributes, taskEntry, (void *)this);
   pthread_attr_destroy(&attributes);

   if (linuxRes == POSIX_OK)
   {
      // Enter the thread id into the global name database so that given the
      // thread id we will be able to find the corresponding OsTask object
      sprintf(idString, "%d", (int)mTaskId);   // convert the id to a string
      OsUtil::insertKeyValue(TASKID_PREFIX, idString, (int) this);

      mState = STARTED;
      return TRUE;
   }
   else
   {
      OsSysLog::add(FAC_KERNEL, PRI_ERR, "OsTaskLinux:doLinuxCreateTask pthread_create failed, returned %d in %s (%p)", linuxRes, mName.data(), this);
      return FALSE;
   }
}

// Do the real work associated with terminating a Linux task
void OsTaskLinux::doLinuxTerminateTask(UtlBoolean doForce)
{
   OsStatus res;
   pthread_t savedTaskId;

   OsSysLog::add(FAC_KERNEL, PRI_DEBUG,
                 "OsTaskLinux::doLinuxTerminateTask, deleting task thread: %x,"
                 " force = %d", (int)mTaskId, doForce);

   // if there is no low-level task, or entry in the name database, just return
   if ((mState != UNINITIALIZED) && ((int)mTaskId != 0))
   {
      // DEBUGGING HACK:  Suspend requestor if target is suspended $$$
      while (isSuspended())
      {
         suspend();
      }
      
      if (!doForce)
      {
         // We are being well behaved and will wait until the task is no longer
         // safe from deletes.  A task is made safe from deletes by acquiring
         // a read lock on its mDeleteGuard. In order to delete a task, the
         // application must acquire a write lock. This will only happen after
         // all of the read lock holders have released their locks.
         res = mDeleteGuard.acquireWrite();
         assert(res == OS_SUCCESS);
      }

      savedTaskId = mTaskId; // taskUnregister sets mTaskId to zero;
      taskUnregister();
      
      // Send the thread the actual cancellation request.
      if (mState == STARTED)
      {
         requestShutdown();
         /* maybe replace this with a call to waitUntilShutDown() ? */
         for(int i = 0; i < 10 && isShuttingDown(); i++)
         {
            delay(100);
         }
      }
      if (mState == SHUTTING_DOWN)
      {
         if (savedTaskId != 0)
         {
            pthread_cancel(savedTaskId);
         }
      }
      
      if (!doForce)
      {
         res = mDeleteGuard.releaseWrite();     // release the write lock
         assert(res == OS_SUCCESS);
      }
   }

   mState = UNINITIALIZED;
}

extern "C" {int setLinuxTaskStartSuspended(int susp);}
extern "C" {int setVTSusp(int susp);}

int linuxTaskStartSuspended = 0;
int setLinuxTaskStartSuspended(int susp) {
   int save = linuxTaskStartSuspended;
   linuxTaskStartSuspended = susp;
   return save;
}

int setVTSusp(int susp)
{
   return setLinuxTaskStartSuspended(susp);
}

// Function that serves as the starting address for a Linux thread
void * OsTaskLinux::taskEntry(void* arg)
{
   OsStatus                  res;
   int                       linuxRes;
   OsTaskLinux*              pTask;
   pthread_attr_t            attributes;
   struct sched_param        param;

   pTask = (OsTaskLinux*) arg;

   // If we ever receive a thread cancel request, it means that the OsTask
   // object is in the process of being destroyed.  To avoid the situation
   // where a thread attempts to run after its containing OsTask object has
   // been freed, we set the thread up so that the cancel takes effect
   // immediately (as opposed to waiting until the next thread cancellation
   // point).
   pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

   if (linuxTaskStartSuspended)
   {
      pthread_kill(pthread_self(), SIGSTOP);
   }

  // construct thread attribute
   linuxRes = pthread_attr_init(&attributes);
   if (linuxRes != 0) {
      OsSysLog::add(FAC_KERNEL, PRI_ERR, "OsTaskLinux::taskEntry: pthread_attr_init failed (%d) ", linuxRes);
   }

   int linuxPriority = OsUtilLinux::cvtOsPrioToLinuxPrio(pTask->mPriority);

   if((geteuid() == 0) && (linuxPriority != RT_NO))
   {
      // Use FIFO realtime scheduling
      param.sched_priority = linuxPriority;
      linuxRes = sched_setscheduler(0, SCHED_FIFO, &param); 
      if (linuxRes == POSIX_OK)
      {
         OsSysLog::add(FAC_KERNEL, PRI_INFO, 
                       "OsTaskLinux::taskEntry: starting %s at RT linux priority: %d", 
                       pTask->mName.data(), linuxPriority);
      }
      else
      {
         OsSysLog::add(FAC_KERNEL, PRI_ERR, 
                       "OsTaskLinux::taskEntry: failed to set RT linux priority: %d for task: %s", 
                       linuxPriority, pTask->mName.data());
      }

      // keep all memory locked into physical mem, to guarantee realtime-behaviour
      if (linuxRes == POSIX_OK)
      {
         linuxRes = mlockall(MCL_CURRENT|MCL_FUTURE);
         if (linuxRes != POSIX_OK)
         {
            OsSysLog::add(FAC_KERNEL, PRI_ERR, 
                          "OsTaskLinux::taskEntry: failed to lock memory for task: %s", 
                          pTask->mName.data());
         }
      }
   }

   // Wait until our init in doLinuxCreateTask() is finished
   int waitTime = 0;
   // Beware that the task may get from Starting to Started to ShuttingDown
   // before we have a chance to test its status!
   while (!(pTask->isStarted() || pTask->isShuttingDown() ||
            pTask->isShutDown()))
   {
       delay(waitTime);
       waitTime += 10;
   }

   // Run the code the task is supposed to run, namely the run()
   // method of its class.

   // Mark the task as not safe to delete.
   res = pTask->mDeleteGuard.acquireRead();
   assert(res == OS_SUCCESS);

   unsigned int returnCode = pTask->run(pTask->getArg());

   // After run returns be sure to mark the thread as shut down.
   pTask->ackShutdown();

   // Then remove it from the OsNameDb.
   pTask->taskUnregister();

   // Mark the task as now safe to delete.
   res = pTask->mDeleteGuard.releaseRead();
   assert(res == OS_SUCCESS);

   return ((void *)returnCode);
}

void OsTaskLinux::taskUnregister(void)
{
   OsStatus res;
   char     idString[15];
   
   if ( 0 != (int)mTaskId )
   {
      // Remove the key from the internal task list, before terminating it
      sprintf(idString, "%d", (int)mTaskId);    // convert the id to a string
      res = OsUtil::deleteKeyValue(TASKID_PREFIX, idString);
   }
   else
   {
      res = OS_SUCCESS;
   }

   if (res != OS_SUCCESS)
   {
      OsSysLog::add(FAC_KERNEL, PRI_ERR, "OsTaskLinux::doLinuxTerminateTask, failed to delete"
                    " mTaskId = 0x%08x, key '%s', returns %d",
                    (int) mTaskId, idString, res);
   }
   mTaskId = 0;

   assert(res == OS_SUCCESS || res == OS_NOT_FOUND);

}

/* ============================ FUNCTIONS ================================= */
