//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
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
#include "os/OsProcess.h"
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
   mDeleteGuard(OsBSemLinux::Q_PRIORITY, OsBSemBase::EMPTY),
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
}

void OsTaskLinux::ackShutdown()
{
   // the task thread is done - we can now get rid of it.
   mDataGuard.acquire();
   UtlString taskName = getName();
   Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG, "OsTaskLinux::ackShutdown '%s' %s",
                 taskName.data(), TaskStateName(mState));

   switch (mState)
   {
   case UNINITIALIZED:
   case RUNNING:
   case SHUTTING_DOWN:
      mState = TERMINATED;

      if (Os::Logger::instance().willLog(FAC_KERNEL, PRI_DEBUG))
      {
          Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG,
                    "OsTaskLinux::ackShutdown '%s' shut down",
                        taskName.data());
      }

      // DEBUGGING HACK:  Suspend requestor if target is suspended $$$
      while (isSuspended())
      {
         mDataGuard.release();
         suspend();
         mDataGuard.acquire();
      }

      // taskUnregister sets mTaskId to zero, so this serves as a flag
      // If this method is called more than once, then on the first
      // call savedTaskId will be non-zero, but on subsequent calls
      // it will be zero.
      if (mTaskId != 0) // only signal waiters on the first call
      {
         OsStatus res;
         char     idString[PID_STR_LEN];
         // Remove the key from the internal task list, before terminating it
         sprintf(idString, "%ld", mTaskId);    // convert the id to a string
         res = OsUtil::deleteKeyValue(TASKID_PREFIX, idString);

         if (res != OS_SUCCESS)
         {
            Os::Logger::instance().log(FAC_KERNEL, PRI_ERR,
                          "OsTaskLinux::ackShutdown '%s' unregister failed"
                          " mTaskId = 0x%08lx, key '%s', returns %d",
                          mName.data(), mTaskId, idString, res);
         }
         else
         {
            Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG,
                          "OsTaskLinux::ackShutdown unregistered '%s'",
                          mName.data());
         }
         mTaskId = 0;
         mDataGuard.release();

         /********************************************************************
          * No reference through 'this' is allowed after the following signal
          * because the destructor will unblock, so the memory may be freed.
          ********************************************************************/
         mDeleteGuard.release(); // signal any task in waitUntilShutDown
      }
      else
      {
         mDataGuard.release();
      }
      break;

   case TERMINATED:
      // already done - not really correct, but let it go
      mDataGuard.release();
      break;

   default:
      Os::Logger::instance().log(FAC_KERNEL, PRI_CRIT,
                    "OsTaskLinux::ackShutdown '%s' invalid mState %d",
                    taskName.data(), mState);
      assert(false);
   }
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
// Return FALSE if the task spawn fails or if the task was
// not already running.
UtlBoolean OsTaskLinux::start(void)
{
   OsLock lock(mDataGuard);

   // Allow start from either UNITIALIZED or TERMINATED states
   if (UNINITIALIZED == mState || TERMINATED == mState)
   {
      int                       linuxRes;
      char                      idString[PID_STR_LEN];
      pthread_attr_t            attributes;

      /*
        Set the mDeleteGuard initial state to Acquired.  We use try here
        because if it is already aquired (as it starts that way), that's just
        fine.  This lets us restart from the TERMINATED state, which leaves
        the mDeleteGuard released.  Thus we have to Acquire it.

        --Woof!
      */
      mDeleteGuard.tryAcquire() ;

      // construct thread attribute
      linuxRes = pthread_attr_init(&attributes);
      if (linuxRes != POSIX_OK)
      {
         Os::Logger::instance().log(FAC_KERNEL, PRI_ERR,
                       "OsTaskLinux::start pthread_attr_init failed %d %s",
                       linuxRes, strerror(linuxRes));
      }

      // Instead of using the default 2M as stack size, reduce it to 1M
      size_t stacksize = 0;
      linuxRes = pthread_attr_getstacksize(&attributes, &stacksize);
      if (linuxRes != POSIX_OK)
      {
         Os::Logger::instance().log(FAC_KERNEL, PRI_ERR,
                       "OsTaskLinux:start pthread_attr_getstacksize error %d %s",
                       linuxRes, strerror(linuxRes));
      }
      else
      {
         linuxRes = pthread_attr_setstacksize(&attributes, OSTASK_STACK_SIZE_1M);
         if (linuxRes != POSIX_OK)
         {
            Os::Logger::instance().log(FAC_KERNEL, PRI_ERR,
                          "OsTaskLinux:start pthread_attr_setstacksize error %d %s",
                          linuxRes, strerror(linuxRes));
         }
      }

      // Create threads detached
      linuxRes = pthread_attr_setdetachstate(&attributes, PTHREAD_CREATE_DETACHED);
      if (linuxRes != POSIX_OK)
      {
         Os::Logger::instance().log(FAC_KERNEL, PRI_ERR,
                       "OsTaskLinux:start pthread_attr_setdetachstate error %d %s",
                       linuxRes, strerror(linuxRes));
      }

      linuxRes = pthread_create(&mTaskId, &attributes, taskEntry, (void *)this);
      Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG,
                    "OsTaskLinux::start '%s' this = %p, mTaskId = %ld",
                    mName.data(), this, mTaskId);
      pthread_attr_destroy(&attributes);

      if (linuxRes == POSIX_OK)
      {
         // Enter the thread id into the global name database so that given the
         // thread id we will be able to find the corresponding OsTask object
         sprintf(idString, "%ld", mTaskId);   // convert the id to a string
         OsUtil::insertKeyValue(TASKID_PREFIX, idString, this);

         mState = RUNNING;
      }
      else
      {
         Os::Logger::instance().log(FAC_KERNEL, PRI_ERR,
                       "OsTaskLinux:start '%s' pthread_create failed %d %s",
                       mName.data(), linuxRes, strerror(linuxRes));
      }
   }
   else
   {
      // There are various tasks that start in the constructors and then are also
      // explicitly started; that causes this to be logged, but it's not really a
      // problem.
      Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG,
                    "OsTaskLinux:start '%s' attempting to start but not in UNINITIALIZED or TERMINATED state (%d)",
                    mName.data(), mState
                    );
   }
   return RUNNING == mState;
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

// Block all signals from the calling thread
OsStatus OsTaskLinux::blockSignals(void)
{
   sigset_t sig_set;
   int res = -1;

   res = sigfillset(&sig_set);
   if (res == 0)
   {
      res = pthread_sigmask(SIG_BLOCK, &sig_set, NULL);
   }

   return res == 0 ? OS_SUCCESS : OS_FAILED;
}

// unblock all signals from the calling thread
OsStatus OsTaskLinux::unBlockSignals(void)
{
   sigset_t sig_set;
   int res = -1;

   res = sigfillset(&sig_set);
   if (res == 0)
   {
      res = pthread_sigmask(SIG_UNBLOCK, &sig_set, NULL);
   }

   return res == 0 ? OS_SUCCESS : OS_FAILED;
}

// Wait for a signal to occur.
OsStatus OsTaskLinux::awaitSignal(int& sig_num)
{
   sigset_t sig_set;
   int res = -1;

   // Enable all signals
   sigfillset(&sig_set);
   do
   {
      res = sigwait(&sig_set, &sig_num);
   } while (res == EINTR); // bug in older glibc sometimes returns EINTR here
   errno = res;            // set errno so it can be seen outside this function

   return res == 0 ? OS_SUCCESS : OS_FAILED;
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

   ts.tv_sec = milliSecs / OsTime::MSECS_PER_SEC;
   ts.tv_nsec = (milliSecs % OsTime::MSECS_PER_SEC) * OsTime::USECS_PER_SEC; // @TODO - retest
   nanosleep(&ts, NULL);

   return OS_SUCCESS;
}

// Yield the CPU if a task of equal or higher priority is ready to run.
void OsTaskLinux::yield(void)
{
   delay(0);
}


// Wait until the task is shut down and the run method has exited.
UtlBoolean OsTaskLinux::waitUntilShutDown(int milliSecToWait)
{
   mDataGuard.acquire();
   UtlString taskName = mName; // make a stable copy for any logging below

   if (Os::Logger::instance().willLog(FAC_KERNEL, PRI_DEBUG))
   {
      OsTask* current = getCurrentTask();
      Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG, "OsTaskLinux::waitUntilShutDown '%s' for '%s' %s",
                    current ? current->mName.data() : "<UNKNOWN>", taskName.data(),
                    TaskStateName(mState));
   }

   switch (mState)
   {
   case UNINITIALIZED:
      // the task object was created but was never started,
      // so transition straight to shut down
      mState = TERMINATED;
      mDataGuard.release();

      mDeleteGuard.release(); // if anyone else is waiting, let them run too.
      break;

   case RUNNING:
      // no one has asked the task to shut down yet, so give it a chance
      mDataGuard.release();
      requestShutdown();  // ask the task to shut itself down
      yield();            // yield the CPU so the target task can terminate
      mDataGuard.acquire(); // get the lock back and fall through to waiting
      /* fall through to next case */

   case SHUTTING_DOWN:
      // wait only as much time as the caller allowed
      mDataGuard.release(); // don't hold the lock while waiting

      if (OS_WAIT_TIMEOUT == mDeleteGuard.acquire(milliSecToWait))
      {
         // the task we're waiting for is unresponsive - destroy the process.
         Os::Logger::instance().log(FAC_KERNEL, PRI_CRIT,
                       "OsTaskLinux::waitUntilShutDown "
                       "'%s' failed to terminate after %d.%03d seconds - aborting process",
                       taskName.data(), milliSecToWait / OsTime::MSECS_PER_SEC,
                       milliSecToWait % OsTime::MSECS_PER_SEC);

         assert(false);  // core dump to find deadlock
         /*
          * If you hit the above, look at each of the threads - set the frame pointer
          * to the OsTask::run in each thread, and print this->mName until you find
          * the one that matches taskName.  That is the thread that did not exit in time.
          */
      }
      else
      {
         if (Os::Logger::instance().willLog(FAC_KERNEL, PRI_DEBUG))
         {
            OsTask* currentTask = getCurrentTask();
            Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG,
                          "OsTaskLinux::waitUntilShutDown task '%s' done waiting for '%s'",
                          currentTask ? currentTask->mName.data() : "<UNKNOWN>",
                          taskName.data()
                          );
         }
         mDeleteGuard.release(); // if anyone else is waiting, let them run too.
      }
      break;

   case TERMINATED:
      // no need to wait - the task is gone.
      mDataGuard.release();

      if (Os::Logger::instance().willLog(FAC_KERNEL, PRI_DEBUG))
      {
         Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG,
                       "OsTaskLinux::waitUntilShutDown task '%s' already shut down",
                       taskName.data()
                       );
      }
      mDeleteGuard.release(); // if anyone else is waiting, let them run too.

   }

   return TRUE;
}

// Return a pointer to the OsTask object for the currently executing task
// Return NULL if none exists.
OsTaskLinux* OsTaskLinux::getCurrentTask(void)
{
   pthread_t taskId;

   taskId = pthread_self();
   return OsTaskLinux::getTaskById(taskId);
}

// Return an Id of the currently executing task
OsStatus OsTaskLinux::getCurrentTaskId(pthread_t &rid)
{
   rid = pthread_self();
   return OS_SUCCESS;
}

// Return a pointer to the OsTask object corresponding to the named task
// Return NULL if there is no task object with that name.
OsTaskLinux* OsTaskLinux::getTaskByName(const UtlString& taskName)
{
   OsStatus res;
   void*    val;

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
OsTaskLinux* OsTaskLinux::getTaskById(const pthread_t taskId)
{
   char     idString[PID_STR_LEN];
   OsStatus res;
   void*    val;

   sprintf(idString, "%ld", taskId);   // convert the id to a string
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

/* ============================ INQUIRY =================================== */

// Get the task ID for this task
OsStatus OsTaskLinux::id(pthread_t& rId)
{
   OsStatus retVal = OS_SUCCESS;

   //if started, return the taskId, otherwise return -1
   if (isStarted())
      rId = mTaskId;
   else
   {
      retVal = OS_TASK_NOT_STARTED;
      rId = (pthread_t) (-1);
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
   if (linuxRes != 0)
   {
      Os::Logger::instance().log(FAC_KERNEL, PRI_ERR,
                    "OsTaskLinux::taskEntry: pthread_attr_init failed %d %s",
                    linuxRes, strerror(linuxRes));
   }

   int linuxPriority = OsUtilLinux::cvtOsPrioToLinuxPrio(pTask->mPriority);

   if((geteuid() == 0) && (linuxPriority != RT_NO))
   {
      // Use FIFO realtime scheduling
      param.sched_priority = linuxPriority;
      linuxRes = sched_setscheduler(0, SCHED_FIFO, &param);
      if (linuxRes == POSIX_OK)
      {
         Os::Logger::instance().log(FAC_KERNEL, PRI_INFO,
                       "OsTaskLinux::taskEntry: starting '%s' at RT linux priority: %d",
                       pTask->mName.data(), linuxPriority);
      }
      else
      {
         Os::Logger::instance().log(FAC_KERNEL, PRI_ERR,
                       "OsTaskLinux::taskEntry '%s' failed to set RT linux priority %d",
                       pTask->mName.data(), linuxPriority);
      }

      // keep all memory locked into physical mem, to guarantee realtime-behaviour
      if (linuxRes == POSIX_OK)
      {
         linuxRes = mlockall(MCL_CURRENT|MCL_FUTURE);
         if (linuxRes != POSIX_OK)
         {
            Os::Logger::instance().log(FAC_KERNEL, PRI_ERR,
                          "OsTaskLinux::taskEntry '%s' failed to lock memory",
                          pTask->mName.data());
         }
      }
   }

   /*
    * Wait until the initialization in ::start() is finished.
    * Since that method is holding the mDataGuard before it calls the pthread_create
    * that created this thread and only releases it on exit, we can use it to synchronize.
    */
   pTask->mDataGuard.acquire();
   // start is done, so now we can let the OsTask::run method execute.
   pTask->mDataGuard.release();

   // Run the code the task is supposed to run, namely the run()
   // method of its class.
   pTask->run(pTask->getArg());

   // The thread has completed now, so clean up and signal the destructor
   pTask->ackShutdown();
   return NULL;
}

/* ============================ FUNCTIONS ================================= */
