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
#include "os/OsExcept.h"
#include "os/OsLock.h"
#include "os/OsUtil.h"
#include "os/OsDateTime.h"
#include "os/Wnt/OsTaskWnt.h"
#include "os/Wnt/OsUtilWnt.h"
#include <process.h>
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsTaskWnt::OsTaskWnt(const UtlString& name,
                     void* pArg,
                     const int priority,
                     const int options,
                     const int stackSize)
:  OsTaskBase(name, pArg, priority, options, stackSize),
   mDeleteGuard(OsRWMutex::Q_PRIORITY),
   mSuspendCnt(0),
   mThreadH(NULL),
   mThreadId(NULL),
   mOptions(options),
   mPriority(priority),
   mStackSize(stackSize)
{
   // other than initialization, no work required
}

// Destructor
OsTaskWnt::~OsTaskWnt()
{
   OsLock lock(mDataGuard);

   doWntTerminateTask(FALSE);
}

// Delete the task even if the task is protected from deletion.
// After calling this method, the user will still need to delete the
// corresponding OsTask object to reclaim its storage.
OsStatus OsTaskWnt::deleteForce(void)
{
   OsLock lock(mDataGuard);

   if (!isStarted())
      return OS_TASK_NOT_STARTED;

   doWntTerminateTask(TRUE);

   return OS_SUCCESS;
}
/* ============================ MANIPULATORS ============================== */

// Restart the task.
// The task is first terminated, and then reinitialized with the same
// name, priority, options, stack size, original entry point, and
// parameters it had when it was terminated.
// Return TRUE if the restart of the task is successful.
UtlBoolean OsTaskWnt::restart(void)
{
   OsLock lock(mDataGuard);

   doWntTerminateTask(FALSE);
   return doWntCreateTask();
}

// Resume the task.
// This routine resumes the task. The task suspension is cleared, and
// the task operates in the remaining state.
OsStatus OsTaskWnt::resume(void)
{
   int    ntResult;
   OsLock lock(mDataGuard);

   if (!isStarted())
      return OS_TASK_NOT_STARTED;

   assert(mSuspendCnt > 0);
   ntResult = ResumeThread(mThreadH);
   if (ntResult >= 0)
   {
      mSuspendCnt--;
      return OS_SUCCESS;
   }
   else
      return OS_UNSPECIFIED;
}

// Spawn a new task and invoke its run() method.
// Return TRUE if the spawning of the new task is successful.
// Return FALSE if the task spawn fails or if the task has already
// been started.
UtlBoolean OsTaskWnt::start(void)
{
   OsLock lock(mDataGuard);

   if (isStarted())
      return FALSE;

   return doWntCreateTask();
}

// Suspend the task.
// This routine suspends the task. Suspension is additive: thus, tasks
// can be delayed and suspended, or pended and suspended. Suspended,
// delayed tasks whose delays expire remain suspended. Likewise,
// suspended, pended tasks that unblock remain suspended only.
OsStatus OsTaskWnt::suspend(void)
{
   int    ntResult;
   OsLock lock(mDataGuard);

   if (!isStarted())
      return OS_TASK_NOT_STARTED;

   assert(mSuspendCnt >= 0);
   ntResult = SuspendThread(mThreadH);
   if (ntResult >= 0)
   {
      mSuspendCnt++;
      return OS_SUCCESS;
   }
   else
      return OS_UNSPECIFIED;
}

// Set the errno status for the task.
// This call has no effect under Windows NT and, if the task has been
// started, will always returns OS_SUCCESS
OsStatus OsTaskWnt::setErrno(int errno)
{
   if (!isStarted())
      return OS_TASK_NOT_STARTED;

   return OS_SUCCESS;
}

// Set the execution options for the task
// The only option that can be changed after a task has been created
// is whether to allow breakpoint debugging.
// This call has no effect under Windows NT and always returns OS_SUCCESS
OsStatus OsTaskWnt::setOptions(int options)
{
   return OS_SUCCESS;
}

// Set the priority of the task.
// Priorities range from 0, the highest priority, to 255, the lowest priority.
OsStatus OsTaskWnt::setPriority(int priority)
{
   int    wntPrio;
   OsLock lock(mDataGuard);

   if (!isStarted())  {
      mPriority = priority; // save mPriority for later use
      return OS_TASK_NOT_STARTED;
   }

   wntPrio = OsUtilWnt::cvtOsPrioToWntPrio(priority);

   if (SetThreadPriority(mThreadH, wntPrio)) {
      mPriority = priority;
      return OS_SUCCESS;
   } else
      return OS_UNSPECIFIED;
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
OsStatus OsTaskWnt::varAdd(int* pVar)
{
   if (!isStarted())
      return OS_TASK_NOT_STARTED;

   return OS_NOT_YET_IMPLEMENTED;
}

// Remove a task variable from the task.
// This routine removes a specified task variable, pVar, from its
// task's context. The private value of that variable is lost.
OsStatus OsTaskWnt::varDelete(int* pVar)
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
OsStatus OsTaskWnt::varSet(int* pVar, int value)
{
   if (!isStarted())
      return OS_TASK_NOT_STARTED;

   return OS_NOT_YET_IMPLEMENTED;
}

// Block all signals from the calling thread
OsStatus OsTaskLinux::blockSignals(void)
{
   // I've no idea what to do for Wnt.
   return OS_NOT_YET_IMPLEMENTED;
}

// Unblock all signals from the calling thread
OsStatus OsTaskLinux::unBlockSignals(void)
{
   // I've no idea what to do for Wnt.
   return OS_NOT_YET_IMPLEMENTED;
}

// Wait for a signal to occur.
OsStatus OsTaskLinux::awaitSignal(int& sig_num)
{
   // I've no idea what to do for Wnt.
   return OS_NOT_YET_IMPLEMENTED;
}

// Delay a task from executing fpr the specified number of milliseconds.
// This routine causes the calling task to relinquish the CPU for the
// duration specified. This is commonly referred to as manual
// rescheduling, but it is also useful when waiting for some external
// condition that does not have an interrupt associated with it.
OsStatus OsTaskWnt::delay(const int milliSecs)
{
   Sleep(milliSecs);

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
OsStatus OsTaskWnt::lock(void)
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
OsStatus OsTaskWnt::unlock(void)
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
OsStatus OsTaskWnt::safe(void)
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
OsStatus OsTaskWnt::unsafe(void)
{
   OsTask*  pTask;
   OsStatus res;

   pTask = getCurrentTask();
   res = pTask->mDeleteGuard.releaseRead();
   assert(res == OS_SUCCESS);

   return res;
}

// Yield the CPU if a task of equal or higher priority is ready to run.
void OsTaskWnt::yield(void)
{
   delay(0);
}

/* ============================ ACCESSORS ================================= */

// Return a pointer to the OsTask object for the currently executing task
// Return NULL if none exists.
OsTaskWnt* OsTaskWnt::getCurrentTask(void)
{
   pthread_t threadId;

   threadId = GetCurrentThreadId();
   return OsTaskWnt::getTaskById(threadId);
}

// Return the Id of the currently executing task
OsStatus OsTaskWnt::getCurrentTaskId(pthread_t &rid)
{
   rid = GetCurrentThreadId();
   return OS_SUCCESS;
}

// Return a pointer to the OsTask object corresponding to the named task
// Return NULL if there is no task object with that name.
OsTaskWnt* OsTaskWnt::getTaskByName(const UtlString& taskName)
{
   OsStatus res;
   int      val;

   res = OsUtil::lookupKeyValue(TASK_PREFIX, taskName, &val);
   assert(res == OS_SUCCESS || res == OS_NOT_FOUND);

   if (res == OS_SUCCESS)
   {
      assert(val != 0);
      return ((OsTaskWnt*) val);
   }
   else
      return NULL;
}

// Return a pointer to the OsTask object corresponding to taskId
// Return NULL is there is no task object with that id.
OsTaskWnt* OsTaskWnt::getTaskById(const pthread_t taskId)
{
   char     idString[PID_STR_LEN];
   OsStatus res;
   int      val;

   itoa((intptr_t)taskId, idString, PID_STR_LEN);   // convert the id to a string
   res = OsUtil::lookupKeyValue(TASKID_PREFIX, idString, &val);
   assert(res == OS_SUCCESS || res == OS_NOT_FOUND);

   if (res == OS_SUCCESS)
   {
      assert(val != 0);
      return ((OsTaskWnt*) val);
   }
   else
      return NULL;

}


// Get the errno status for the task
// Under Windows NT, the rErrno value will always be 0.
OsStatus OsTaskWnt::getErrno(int& rErrno)
{
   rErrno = 0;

   if (!isStarted())
      return OS_TASK_NOT_STARTED;

   return OS_SUCCESS;
}

// Return the execution options for the task
int OsTaskWnt::getOptions(void)
{
   return mOptions;
}

// Return the priority of the task
OsStatus OsTaskWnt::getPriority(int& rPriority)
{
   int wntPrio;

   if (!isStarted())
      return OS_TASK_NOT_STARTED;

   wntPrio = GetThreadPriority(mThreadH);
   rPriority = OsUtilWnt::cvtWntPrioToOsPrio(wntPrio);

   return OS_SUCCESS;
}

// Get the value of a task variable.
// This routine returns the private value of a task variable for its
// task. The task is usually not the calling task, which can get its
// private value by directly accessing the variable. This routine is
// provided primarily for debugging purposes.
OsStatus OsTaskWnt::varGet(void)
{
   if (!isStarted())
      return OS_TASK_NOT_STARTED;

   return OS_NOT_YET_IMPLEMENTED;
}

/* ============================ INQUIRY =================================== */

// Get the task ID for this task
OsStatus OsTaskWnt::id(pthread_t& rId)
{
   OsStatus retVal = OS_SUCCESS;

   //if started, return the taskId, otherwise return -1
   if (isStarted())
      rId = mThreadId;
   else
   {
      retVal = OS_TASK_NOT_STARTED;
      rId = -1;
   }


   return retVal;
}

// Check if the task is ready to run
// Return TRUE is the task is ready, otherwise FALSE.
// Under Windows NT, this method returns the opposite of isSuspended()
UtlBoolean OsTaskWnt::isReady(void)
{
   if (!isStarted())
      return FALSE;

   return(!isSuspended());
}

// Check if the task is suspended.
// Return TRUE is the task is suspended, otherwise FALSE.
UtlBoolean OsTaskWnt::isSuspended(void)
{
   OsLock lock(mDataGuard);

   if (!isStarted())
      return FALSE;

   return mSuspendCnt > 0;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

// Do the real work associated with creating a new WinNT task
// The mDataGuard lock should be held upon entry into this method.
UtlBoolean OsTaskWnt::doWntCreateTask(void)
{
   char  idString[PID_STR_LEN];
   pthread_t threadId;

   mThreadH = (HANDLE)_beginthreadex(
                0,             // don't specify any thread attributes
                mStackSize,    // stack size (in bytes)
                threadEntry,   // starting address of the new thread
                (void*) this,  // parameter value that will be passed
                               //  to the new thread
                CREATE_SUSPENDED, // suspend thread until priority is set
                &threadId);    // thread identifier return value

   mSuspendCnt = 0;
   mThreadId   = threadId;

   if (mThreadH != NULL)
   {
      // Enter the thread id into the global name database so that given the
      // thread id we will be able to find the corresponding OsTask object
      itoa((intptr_t)mThreadId, idString, PID_STR_LEN);  // convert the id to a string
      OsUtil::insertKeyValue(TASKID_PREFIX, idString, (intptr_t)this);

      mState = STARTED;

      // Unlike VxWorks, the priority is not an argument in the creation call,
      // so we have to set it explicitly ourselves.  To that end, we start
      // the thread "suspended" and release it after setting its priority.
      setPriority(mPriority);
      ResumeThread(mThreadH);
      return TRUE;
   }
   else
      return FALSE;
}

// Do the real work associated with terminating a WinNT task
// The mDataGuard lock should be held upon entry into this method.
void OsTaskWnt::doWntTerminateTask(UtlBoolean force)
{
   char      idString[PID_STR_LEN];
   UtlBoolean ntResult;
   OsStatus  res;

   if (mState == UNINITIALIZED)
      return;           // there is no low-level task, just return

   if (!force)
   {
      // We are being well behaved and will wait until the task is no longer
      // safe from deletes.  A task is made safe from deletes by acquiring
      // a read lock on its mDeleteGuard. In order to delete a task, the
      // application must acquire a write lock. This will only happen after
      // all of the read lock holders have released their locks.
      res = mDeleteGuard.acquireWrite();
      assert(res == OS_SUCCESS);
   }

   if (mThreadH != NULL)
   {
     // Remove the key from the internal task list, before terminating it
     itoa((intptr_t)mThreadId, idString, PID_STR_LEN);   // convert the id to a string
     res = OsUtil::deleteKeyValue(TASKID_PREFIX, idString);

    //before we go ahead and kill the thread, lets make sure it's still running
    DWORD ExitCode;

    // KLUDGE ALERT- we need to avoid calling
    // TerminateThread at all costs.
    // TerminateThread is VERY VERY dangerous.
    // Wait for 5 seconds, or until the thread exited
    // before calling TerminateThread.
    // attempt to wait for the thread to exit on its own
    for (int i = 0; i < 50; i++)
    {
        GetExitCodeThread( mThreadH,&ExitCode);
        if (ExitCode != STILL_ACTIVE)
        {
            break;
        }
        Sleep(100);
    }

    if (ExitCode == STILL_ACTIVE)
          TerminateThread(mThreadH, 0);          // first get rid of the thread
                                             //  ignore the return code since
                                             //  it's possible the thread has
                                             //  already terminated

      ntResult = CloseHandle(mThreadH);      // next we close the handle
      assert(ntResult == TRUE);
   }

   mThreadH   = 0;
   mThreadId  = 0;
   mState     = UNINITIALIZED;

   if (!force)
   {
      res = mDeleteGuard.releaseWrite();     // release the write lock
      assert(res == OS_SUCCESS);
   }
}

// Function that serves as the starting address for a Windows thread
unsigned int __stdcall OsTaskWnt::threadEntry(LPVOID arg)
{
   OsTaskWnt* pTask;

   pTask = (OsTaskWnt*) arg;

   // Windows uses a different random number generator  each
   // thread.  This means that you need to initialized the
   // random number generator seed in each thread as well
   // The following is an attempt to create a good seed
   // for this task.
   int eTimeInt = OsDateTime::getSecsSinceEpoch();
   pthread_t taskId;
   pTask->id(taskId);
   // Get the lower 16 bits (most varying) of the time and task pointer
   int lower16Time = eTimeInt % (256 * 256);
   int lower16Thread = taskId % (256 * 256);
   // move the lower 16 bits (most varying) of the current time
   // to the upper 16 bits
   int lower16TaskPtr = ((int)pTask) << 16;

   int randSeed = lower16Time + lower16Thread + lower16TaskPtr;
   //osPrintf("OsTaskWnt::threadEntry time: %d id: %d task: %d randSeed: %u\n",
   //    lower16Time, lower16Thread, lower16TaskPtr, (unsigned int)randSeed);

   srand(randSeed);

   //osPrintf("OsTaskWnt::threadEntry rand: %d threadId: %d epoch: %d ptr: %d\n",
   //    rand(), taskId, eTimeInt, pTask);

   unsigned int returnCode = pTask->run(pTask->getArg());

   // After run returns be sure to mark the thread as shut down
   pTask->ackShutdown();

   return(returnCode);
}

/* ============================ FUNCTIONS ================================= */
