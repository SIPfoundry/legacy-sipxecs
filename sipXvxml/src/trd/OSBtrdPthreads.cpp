/*****************************************************************************
 *****************************************************************************
 *
 * 
 *
 * SBtrd API implementation
 *
 * This provides the Linux implementation of the VXItrd API for basic
 * thread operations and locks. Unlike most of the other VXI APIs,
 * this is implemented in a library (on Windows a DLL with a specific
 * name, on other operating systems as a static, shared, or dynamic
 * library). Implementations of this API are operating system
 * dependant.
 *
 * To avoid cyclic dependancies, this does not perform logging. Clients
 * must do error logging themselves based on passed return codes.
 *
 *****************************************************************************
 *****************************************************************************/


/****************License************************************************
 *
 * Copyright 2000-2001.  SpeechWorks International, Inc.  
 *
 * Use of this software is subject to notices and obligations set forth
 * in the SpeechWorks Public License - Software Version 1.1 which is
 * included with this software.
 *
 * SpeechWorks is a registered trademark, and SpeechWorks Here, 
 * DialogModules and the SpeechWorks logo are trademarks of SpeechWorks 
 * International, Inc. in the United States and other countries. 
 * 
 ************************************************************************
 */

static const char *rcsid = 0 ? (char *) &rcsid :
"";

// -----1=0-------2=0-------3=0-------4=0-------5=0-------6=0-------7=0-------8


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define VXITRD_EXPORTS
#define SIPXCHANGE_STACK_SIZE_1M 1024*1024
#define SIPXCHANGE_STACK_SIZE_128K 128*1024
#include "VXItrd.h"                   // Header for this API

extern "C" {
struct VXItrdMutex {
  pthread_mutex_t mutex;
};
}

typedef enum VXItrdThreadState {
  STATE_STARTING = 0,
  STATE_RUNNING = 1,
  STATE_EXITED = 2
} VXItrdThreadState;

extern "C" {
struct VXItrdThread {
  pthread_t thread;                       // Thread handle
  VXItrdThreadState state;                // Thread state
  VXItrdMutex *refCountMutex;             // For reference count protection
  VXIulong refCount;                      // References to this structure
  VXItrdThreadStartFunc thread_function;  // user's thread function
  VXItrdThreadArg thread_arg;             // user's thread argument
};
}

extern "C" {
struct VXItrdTimer {
  VXIbool isSleeping;   /* If 1, thread is currently sleeping */
  VXIbool wakeUp;	/* If 1, thread will ignore the next Sleep() */
};
}

// -----1=0-------2=0-------3=0-------4=0-------5=0-------6=0-------7=0-------8


/**
 * Internal utility function for sleeping
 */
static VXItrdResult VXItrdSleep(VXIint millisecondDelay)
{
  // Do not want to use usleep( ), non-portable and it is usually implemented
  // via signals which may mess up other software in the system which is
  // also trying to use signals. Note that it is very important to set up
  // the timer each time as on some OSes (like Linux) the timeout var is
  // modified to indicate the actual time slept.
  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = millisecondDelay * 1000;
  if (select (0, NULL, NULL, NULL, &timeout) < 0)
    return VXItrd_RESULT_SYSTEM_ERROR;

  return VXItrd_RESULT_SUCCESS;
}


/**
 * Creates a new mutex initialized to the unlocked state.  If the calling
 * thread terminates, the created mutex is not automatically deallocated as
 * it may be shared among multiple threads.
 *
 * @return -1 Fatal error (ex: system lacks necessary resources for creation)
 *           0 Success; valid mutex has been created
 */
VXITRD_API VXItrdResult VXItrdMutexCreate(VXItrdMutex **mutex)
{
  if (mutex == NULL) 
	return VXItrd_RESULT_INVALID_ARGUMENT;

  *mutex = NULL;
  
  // Create the wrapper object */
  VXItrdMutex *result = new VXItrdMutex;
  if (result == NULL)
    return VXItrd_RESULT_OUT_OF_MEMORY;

  // Create the critical section */
  int rc;
  pthread_mutexattr_t mutexattr;
  
  // Create and configure mutex attributes for recursive mutex
  rc = pthread_mutexattr_init(&mutexattr);
  if (rc != 0) {
	delete result;
	result = NULL;
    return VXItrd_RESULT_NON_FATAL_ERROR;
  }
         
#ifndef _unixware7_
  rc = pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
  if (rc != 0) {
    pthread_mutexattr_destroy(&mutexattr);
	delete result;
	result = NULL;
    return VXItrd_RESULT_NON_FATAL_ERROR;
  }
#endif

  // Initialize mutex with RECURSIVE attributes
  rc = pthread_mutex_init(&result->mutex, &mutexattr);
  if (rc != 0) {
    pthread_mutexattr_destroy(&mutexattr);
	delete result;
	result = NULL;
    return VXItrd_RESULT_NON_FATAL_ERROR;
  }

  rc = pthread_mutexattr_destroy(&mutexattr);
  if (rc != 0) {
    pthread_mutex_destroy(&result->mutex);
	delete result;
	result = NULL;
    return VXItrd_RESULT_NON_FATAL_ERROR;
  }

  *mutex = result;
  return VXItrd_RESULT_SUCCESS;
}


/**
 * Deletes an existing mutex.  It is illegal to delete a locked mutex.
 *
 * @return -1 Fatal error (ex: invalid mutex)
 *          0 Success; mutex has been destroyed
 *          1 Mutex is locked
 */
VXITRD_API VXItrdResult VXItrdMutexDestroy(VXItrdMutex **mutex)
{
  if ((mutex == NULL) || (*mutex == NULL))
	return VXItrd_RESULT_INVALID_ARGUMENT;

  int rc;

  rc = pthread_mutex_destroy(&(*mutex)->mutex);
  if (rc != 0) {
    return VXItrd_RESULT_NON_FATAL_ERROR;
  }
  
  delete *mutex;
  *mutex = NULL;

  return VXItrd_RESULT_SUCCESS;
}


/**
 * Locks an existing mutex.  If the mutex is already locked, the thread waits
 * for the mutex to become available.
 *
 * @return: -1 Fatal error (ex: invalid mutex or deadlock detected)
 *           0 Success; mutex is now locked
 *           1 Mutex already locked by current thread.
 */
VXITRD_API VXItrdResult VXItrdMutexLock(VXItrdMutex *mutex)
{
  if (mutex == NULL)
	return VXItrd_RESULT_INVALID_ARGUMENT;
  
  int rc;
  
  rc = pthread_mutex_lock(&mutex->mutex);
  if (rc != 0) {
    return VXItrd_RESULT_NON_FATAL_ERROR;
  }
  
  return VXItrd_RESULT_SUCCESS;
}


/**
 * Unlocks a mutex owned by the thread.
 *
 * @return: -1 Fatal error (ex: invalid mutex)
 *           0 Success; mutex no longer owned by calling thread
 *           1 Mutex not owned by thread.
 */
VXITRD_API VXItrdResult VXItrdMutexUnlock(VXItrdMutex *mutex)
{
  if (mutex == NULL)
	return VXItrd_RESULT_INVALID_ARGUMENT;

  int rc;

  rc = pthread_mutex_unlock(&mutex->mutex);
  if (rc != 0) {
    return VXItrd_RESULT_NON_FATAL_ERROR;
  }

  return VXItrd_RESULT_SUCCESS;
}


/**
 * Internal: wrapper functions for starting/cleaning up threads
 */
static void VXItrdThreadCleanup(VXItrdThreadArg userData)
{
  VXItrdThread *thread = (VXItrdThread *) userData;

  // No longer active
  thread->state = STATE_EXITED;

  // Free our copy of the thread handle
  VXItrdThreadDestroyHandle (&thread);
}


static VXITRD_DEFINE_THREAD_FUNC(VXItrdThreadStart, userData)
{
  VXItrdThread *thread = (VXItrdThread *) userData;

  // Set the state and register a cleanup function to finish it,
  // required for ThreadJoin( ) support
  thread->state = STATE_RUNNING;
  pthread_cleanup_push(VXItrdThreadCleanup, userData);

  // Call the user function
  (*thread->thread_function)(thread->thread_arg);

  // Cleanup, the 1 to pop causes the cleanup function to be invoked
  pthread_cleanup_pop(1);

  return NULL;  // never gets called, just to eliminate compile warning
}


/**
 * Create a thread.  Note: thread values are not supported on OS/2.
 *    execution starts on the thread immediately. To pause execution 
 *    use a Mutex between the thread and the thread creator.
 *
 * @param   thread the thread object to be created
 * @param   thread_function the function for the thread to start execution on
 * @param   thread_arg the arguments to the thread function
 * @return  VXItrdResult of operation.  Return SUCCESS if thread has been 
 *          created and started.
 *
 */
VXITRD_API 
VXItrdResult VXItrdThreadCreate(VXItrdThread **thread,
				VXItrdThreadStartFunc thread_function,
				VXItrdThreadArg thread_arg)
{
  if ((thread == NULL) || (thread_function == NULL))
    return VXItrd_RESULT_INVALID_ARGUMENT;

  *thread = NULL;

  // Create the wrapper object
  VXItrdThread *result = new VXItrdThread;
  if (result == NULL)
    return VXItrd_RESULT_OUT_OF_MEMORY;

  memset(result, 0, sizeof (VXItrdThread));
  result->state = STATE_STARTING;
  result->refCount = 1; // 1 for parent
  result->thread_function = thread_function;
  result->thread_arg = thread_arg;

  if (VXItrdMutexCreate(&result->refCountMutex) != VXItrd_RESULT_SUCCESS) {
    VXItrdThreadDestroyHandle(&result);
    return VXItrd_RESULT_SYSTEM_ERROR;
  }

  // construct thread attribute
  pthread_attr_t thread_attr;
  int prc = pthread_attr_init(&thread_attr);
  if (prc != 0) {
    VXItrdThreadDestroyHandle(&result);
    return VXItrd_RESULT_NON_FATAL_ERROR;
  }
  
  // configure thread attribute
#ifdef USE_DETACHED_THREADS
  prc = pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
  if (prc != 0) {
    pthread_attr_destroy(&thread_attr);
    VXItrdThreadDestroyHandle(&result);
    return VXItrd_RESULT_NON_FATAL_ERROR;
  }
#else
  // use joinable threads
  // this is default - no work required
#endif

  // Instead of using the default 2M as stack size, reduce it to 1M
  size_t stacksize = 0;

  prc = pthread_attr_getstacksize(&thread_attr, &stacksize);

  if (prc == 0) { 
    prc = pthread_attr_setstacksize(&thread_attr, SIPXCHANGE_STACK_SIZE_128K);
  }

  // Start the thread using our wrapper function
  result->refCount++;  // for child
  prc = pthread_create(&result->thread, &thread_attr, VXItrdThreadStart,
                       (VXItrdThreadArg) result);

  pthread_attr_destroy(&thread_attr);

  if (prc != 0) {
    result->refCount--;
    VXItrdThreadDestroyHandle(&result);
    return VXItrd_RESULT_NON_FATAL_ERROR;
  }

  *thread = result;

  return VXItrd_RESULT_SUCCESS;
}


/**
 * Destroy a thread handle
 *
 * Note: this does NOT stop or destroy the thread, it just releases
 * the handle for accessing it. If this is not done, a memory leak
 * occurs, so if the creator of the thread never needs to communicate
 * with the thread again it should call this immediately after the
 * create if the create was successful.
 *
 * @param  thread  Handle to the thread to destroy
 *
 * @result VXItrdResult 0 on success 
 */
VXITRD_API VXItrdResult VXItrdThreadDestroyHandle(VXItrdThread **thread)
{
  if ((thread == NULL) || (*thread == NULL))
    return VXItrd_RESULT_INVALID_ARGUMENT;

  // Decrement ref count
  VXIulong refs = 0;
  if ((*thread)->refCountMutex) {
    VXItrdMutexLock ((*thread)->refCountMutex);
    (*thread)->refCount--;
    refs = (*thread)->refCount;
    VXItrdMutexUnlock ((*thread)->refCountMutex);
  }

  if (refs == 0) {
    // No longer needed
    if ((*thread)->refCountMutex)
      VXItrdMutexDestroy (&(*thread)->refCountMutex);
    delete *thread;
  }

  *thread = NULL;

  return VXItrd_RESULT_SUCCESS;
}


/**
 * Terminate a thread.  Called by the thread on exit.
 *
 * @param  VXItrdThread *thread the thread to be canceled.
 * @result           N/A, never returns
 */
VXITRD_API void VXItrdThreadCancel(VXItrdThread *thread)
{
      // Send the thread the actual cancellation request.
      // This happens as a signal being sent to the thread,
      // so we could in the future catch the signal and handle
      // thread cancellation from a signal handler.
      pthread_cancel(thread->thread);
      
}



/**
 * Terminate a thread.  Called by the thread on exit.
 *
 * @param  status  Exit code for the thread
 * @result           N/A, never returns
 */
VXITRD_API void VXItrdThreadExit(VXItrdThreadArg status)
{
  pthread_exit(status);
}


/**
 * Causes the calling thread to wait for the termination of a specified
 *   'thread' with a specified timeout, in milliseconds. 
 *
 * @param   thread the 'thread' that is waited for its termination.
 * @param   status contains the exit value of the thread's start routine.
 * @return  VXItrdResult of operation.  Return SUCCESS if specified 'thread' 
 *          terminating.
 */
VXITRD_API VXItrdResult VXItrdThreadJoin(VXItrdThread *thread,
					 VXItrdThreadArg *status,
					 long timeout)
{
  if ((thread == NULL ) || (status == NULL) || (timeout < -1))
    return VXItrd_RESULT_INVALID_ARGUMENT;

  VXItrdResult rc = VXItrd_RESULT_SUCCESS;
  if (timeout == -1) {
    // Wait forever
    if (pthread_join(thread->thread, status) != 0)
      rc = VXItrd_RESULT_SYSTEM_ERROR;
  } else {
    // Wait with a timeout, we just do a polling implementation
    long remainingMsec = timeout;
    while ((remainingMsec > 0) && (thread->state != STATE_EXITED) &&
           (rc == VXItrd_RESULT_SUCCESS)) {
      if (remainingMsec > 50) {
        rc = VXItrdSleep (50);
        remainingMsec -= 50;
      } else {
        rc = VXItrdSleep (remainingMsec);
        remainingMsec = 0;
      }
    }

    if (rc == VXItrd_RESULT_SUCCESS) {
      if (thread->state == STATE_EXITED) {
        // Collect the thread
        if (pthread_join(thread->thread, status) != 0)
          rc = VXItrd_RESULT_SYSTEM_ERROR;
      } else {
        rc = VXItrd_RESULT_FAILURE;
      }
    }
  }

  return rc;
}


/**
 * Get the thread ID for the specified thread
 *
 * @param  thread   Handle to the thread to get the ID for
 *
 * @result   Thread ID number
 */
VXITRD_API VXIlong VXItrdThreadGetIDFromHandle(VXItrdThread *thread)
{
  if (thread == NULL) 
	return -1;

  return thread->thread;
}


/**
 * Get the ID of the current handle.
 *
 * @return  The current thread handle identifier.
 *
 */
VXITRD_API VXIlong VXItrdThreadGetID(void)
{
  return (VXIlong)pthread_self(); 
}


/**
 * Purpose Yield the process schedule in the current thread.
 *
 * @return  void
 *
 */
VXITRD_API void VXItrdThreadYield(void)
{
  sched_yield();
}


/**
 * Create a timer
 *
 * @param   timer  a pointer to a timer
 * @return  VXItrdResult of operation.  Return SUCCESS if timer has been 
 *          created
 *
 */
VXITRD_API VXItrdResult VXItrdTimerCreate(VXItrdTimer **timer)
{
  if (timer == NULL) 
	return VXItrd_RESULT_INVALID_ARGUMENT;
  
  VXItrdTimer *result = new VXItrdTimer;
  if (result == NULL)
    return VXItrd_RESULT_OUT_OF_MEMORY;

  // Initialize all data
  result->isSleeping = FALSE;
  result->wakeUp = FALSE;

  *timer = result;

  return VXItrd_RESULT_SUCCESS;
}


/**
 * Destroy a timer
 *
 * @param   timer  a pointer to a timer
 * @return  VXItrdResult of operation.  Return SUCCESS if timer has been 
 *          created
 *
 */
VXITRD_API VXItrdResult VXItrdTimerDestroy(VXItrdTimer **timer)
{
  if ((timer == NULL) || (*timer == NULL))
	return VXItrd_RESULT_INVALID_ARGUMENT;

  // Don't leave the corresponding thread in a suspended state.
  VXItrdResult rc = VXItrd_RESULT_SUCCESS;
  if ((*timer)->isSleeping) {
    VXItrdTimerWake(*timer);
    
    // Wait for the sleeping thread to wake up, note that on some OSes
    // like Linux the timeout is modified to give the actual amount of
    // time slept so need to re-initialize each time
    while (((*timer)->isSleeping) && 
           ((rc = VXItrdSleep (50)) == VXItrd_RESULT_SUCCESS))
      ; // keep going
  }

  delete *timer;
  *timer = NULL;

  return rc;
}


/**
 * Puts the current thread to sleep for a configurable duration.
 *    Due to other activities of the machine, the delay is the minimum
 *    length that the timer will wait.
 *
 * @param   timer  a pointer to a timer
 * @param   millisecondDelay  the minimum number of milliseconds to wait
 * @param   interrupted  a pointer (may optionally be NULL) indicating whether
 *            or not the sleep was interrupted by VXItrdTimerWake.
 * @return  VXItrdResult of operation.  Return SUCCESS if timer could sleep.
 *
 */
VXITRD_API VXItrdResult VXItrdTimerSleep(VXItrdTimer *timer,
					 VXIint millisecondDelay,
					 VXIbool *interrupted)
{
  if ((timer == NULL) || (millisecondDelay < 0)) 
    return VXItrd_RESULT_INVALID_ARGUMENT;

 
  if (timer->isSleeping == TRUE)
    return VXItrd_RESULT_FATAL_ERROR;

  // Sleep, we just do a polling implementation
  VXItrdResult rc = VXItrd_RESULT_SUCCESS;
  VXIint remainingMsec = millisecondDelay;
  timer->isSleeping = TRUE;
  while ((remainingMsec > 0) && (timer->wakeUp == FALSE) &&
         (rc == VXItrd_RESULT_SUCCESS)) {
    if (remainingMsec > 50) {
      rc = VXItrdSleep (50);
      remainingMsec -= 50;
    } else {
      rc = VXItrdSleep (remainingMsec);
      remainingMsec = 0;
    }
  }

  if (interrupted)
    *interrupted = (timer->wakeUp ? TRUE : FALSE);

  timer->wakeUp = FALSE;
  timer->isSleeping = FALSE;

  return VXItrd_RESULT_SUCCESS;
}


/**
 * Wakes a sleeping thread, if the target is not already sleeping
 *    it immediately wakes when it tries to sleep the next time.
 *
 * @param   timer  a pointer to a timer
 * @return  VXItrdResult of operation.  Return SUCCESS if timer could wake.
 *
 */
VXITRD_API VXItrdResult VXItrdTimerWake(VXItrdTimer *timer)
{
  if (timer == NULL) 
    return VXItrd_RESULT_INVALID_ARGUMENT;

  timer->wakeUp = TRUE;

  return VXItrd_RESULT_SUCCESS;
}
