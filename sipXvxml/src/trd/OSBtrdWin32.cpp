/*****************************************************************************
 *****************************************************************************
 *
 * 
 *
 * SBtrd API implementation
 *
 * This provides the Win32 implementation of the VXItrd API for basic
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

#ifndef WIN32
#error This file is intended only for use with the Microsoft Windows API.
#endif

#include <stdlib.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <process.h>

#define VXITRD_EXPORTS
#include "VXItrd.h"                   // Header for this API

#if ( ! defined(NDEBUG) ) && ( ! defined(VXITRD_RECURSIVE_MUTEX) )
#include <assert.h>

/* Suppress/disable warnings triggered by asserts below */
#pragma warning(4 : 4130) /* logical operation on address of string constant */
#endif

extern "C" struct VXItrdMutex {
#ifdef VXITRD_KERNEL_MUTEX
  HANDLE mutex;
#else
  CRITICAL_SECTION critSection;
#endif
#if ( ! defined(NDEBUG) ) && ( ! defined(VXITRD_RECURSIVE_MUTEX) )
  bool locked;
#endif
};

extern "C" struct VXItrdThread {
  HANDLE threadHandle;
  pthread_t threadID;
};

extern "C" struct VXItrdTimer {
  HANDLE timerEvent;
};

// -----1=0-------2=0-------3=0-------4=0-------5=0-------6=0-------7=0-------8


/**
 * Creates a new mutex initialized to the unlocked state.  If the calling
 * thread terminates, the created mutex is not automatically deallocated as
 * it may be shared among multiple threads.
 *
 * @return Fatal error (ex: system lacks necessary resources for creation)
 *         Success; valid mutex has been created
 */
VXITRD_API VXItrdResult VXItrdMutexCreate(VXItrdMutex **mutex)
{
  VXItrdResult rc = VXItrd_RESULT_SUCCESS;
  if (mutex == NULL) return VXItrd_RESULT_INVALID_ARGUMENT;

  *mutex = NULL;
  
  // Create the wrapper object
  VXItrdMutex *result = new VXItrdMutex;
  if (result == NULL)
    return VXItrd_RESULT_OUT_OF_MEMORY;
#if ( ! defined(NDEBUG) ) && ( ! defined(VXITRD_RECURSIVE_MUTEX) )
  result->locked = false;
#endif

  // Create the critical section or mutex, use Win32 exception
  // handling to catch predicatable problems
  __try {
#ifdef VXITRD_KERNEL_MUTEX
    result->mutex = CreateMutex(NULL, FALSE, NULL);
    if (result->mutex == NULL)
      rc = VXItrd_RESULT_SYSTEM_ERROR;
#else
    InitializeCriticalSection(&result->critSection);
#endif
  }
  __except (GetExceptionCode() == STATUS_NO_MEMORY ? 
	    EXCEPTION_EXECUTE_HANDLER : 
	    EXCEPTION_CONTINUE_SEARCH ) {
    rc = VXItrd_RESULT_OUT_OF_MEMORY;
  }

  if (rc == VXItrd_RESULT_SUCCESS)
    *mutex = result;
  else if (result)
    delete result;
  result = NULL;

  return rc;
}


/**
 * Deletes an existing mutex.  It is illegal to delete a locked mutex.
 *
 * @return Fatal error (ex: invalid mutex)
 *         Success; mutex has been destroyed
 *         Mutex is locked
 */
VXITRD_API VXItrdResult VXItrdMutexDestroy(VXItrdMutex **mutex)
{
  VXItrdResult rc = VXItrd_RESULT_SUCCESS;
  if ((! mutex) || (! *mutex)) return VXItrd_RESULT_INVALID_ARGUMENT;

#if ( ! defined(NDEBUG) ) && ( ! defined(VXITRD_RECURSIVE_MUTEX) )
  // Check the lock state
  if ( (*mutex)->locked ) {
    assert ("VXItrdMutexDestroy( ) on locked mutex" == NULL);
    return VXItrd_RESULT_FATAL_ERROR;
  }
#endif

  // Are no predictable problems, so no Win32 exception handling
#ifdef VXITRD_KERNEL_MUTEX
  if (CloseHandle((*mutex)->mutex) == FALSE)
    rc = VXItrd_RESULT_SYSTEM_ERROR;
#else
  DeleteCriticalSection(&(*mutex)->critSection);
#endif

  delete *mutex;
  *mutex = NULL;
  return rc;
}


/**
 * Locks an existing mutex.  If the mutex is already locked, the thread waits
 * for the mutex to become available.
 *
 * @return: Fatal error (ex: invalid mutex or deadlock detected)
 *          Success; mutex is now locked
 *          Mutex already locked by current thread.
 */
VXITRD_API VXItrdResult VXItrdMutexLock(VXItrdMutex *mutex)
{
  if (mutex == NULL) return VXItrd_RESULT_INVALID_ARGUMENT;
  
  __try {
#ifdef VXITRD_KERNEL_MUTEX
    int result = WaitForSingleObject(mutex->mutex, INFINITE);
    if (result == WAIT_FAILED) 
      return VXItrd_RESULT_SYSTEM_ERROR;
#else
    EnterCriticalSection(&mutex->critSection);
#endif
  }
  __except (GetExceptionCode() == STATUS_INVALID_HANDLE ? 
	    EXCEPTION_EXECUTE_HANDLER : 
	    EXCEPTION_CONTINUE_SEARCH ) {
    // Yes, Microsoft says STATUS_INVALID_HANDLE is returned on low memory
    return VXItrd_RESULT_OUT_OF_MEMORY;
  }
  
#if ( ! defined(NDEBUG) ) && ( ! defined(VXITRD_RECURSIVE_MUTEX) )
  // Check the lock state
  if ( mutex->locked ) {
    // Should not be locking the same mutex twice, very OS dependant
    // (Win32 says the behavior is undefined) and not gauranteed by 
    // VXItrdMutex
    assert ("VXItrdMutexLock( ) on already locked mutex" == NULL);
    return VXItrd_RESULT_FATAL_ERROR;
  } else {
    mutex->locked = true;
  }
#endif

  return VXItrd_RESULT_SUCCESS;
}


/**
 * Unlocks a mutex owned by the thread.
 *
 * @return: Fatal error (ex: invalid mutex)
 *          Success; mutex no longer owned by calling thread
 *          Mutex not owned by thread.
 */
VXITRD_API VXItrdResult VXItrdMutexUnlock(VXItrdMutex *mutex)
{
  if (mutex == NULL) return VXItrd_RESULT_INVALID_ARGUMENT;

#if ( ! defined(NDEBUG) ) && ( ! defined(VXITRD_RECURSIVE_MUTEX) )
  // Check the lock state
  if ( ! mutex->locked ) {
    // Unlocking a mutex that wasn't locked
    assert ("VXItrdMutexUnlock( ) on unlocked mutex" == NULL);
    return VXItrd_RESULT_FATAL_ERROR;
  } else {
    mutex->locked = false;
  }
#endif

#ifdef VXITRD_KERNEL_MUTEX
  int result = ReleaseMutex(mutex->mutex);
  if (result == FALSE)
    return VXItrd_RESULT_FATAL_ERROR;
#else
  LeaveCriticalSection(&mutex->critSection);
#endif

  return VXItrd_RESULT_SUCCESS;
}


/**
 * Purpose  Create a thread.  Note thread values are not supported on OS/2.
 *          execution starts on the thread immediately. To pause execution 
 *          use a Mutex between the thread and the thread creator.
 *
 * @param   thread the thread object to be created
 * @param   start_func the function for the thread to start execution on
 * @param   arg the arguments to the thread function
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

  // Use _beginthreadex( ) so that the CRT (common runtime
  // environment) is fully initialized so all CRT functions work,
  // CreateThread( ) doesn't do this. However, this is Microsoft Visual C++
  // specific, not part of the official Win32 API.
  typedef unsigned int (__stdcall *beginthreadfunc)(void *);
  beginthreadfunc func = reinterpret_cast<beginthreadfunc>(thread_function);
  void *arg = static_cast<void *>(thread_arg);

  result->threadHandle = 
    reinterpret_cast<HANDLE>(_beginthreadex(NULL, 0, func, arg, 0, 
					    &result->threadID));
  if (result->threadHandle == 0) {
    delete result;
    result = NULL;
    return VXItrd_RESULT_FATAL_ERROR;
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
  if ((! thread) || (! *thread)) return VXItrd_RESULT_INVALID_ARGUMENT;

  CloseHandle ((*thread)->threadHandle);
  delete *thread;
  *thread = NULL;
  return VXItrd_RESULT_SUCCESS;
}


/**
 * Terminate a thread (called by the thread to exit)
 *
 * @param  thread    the thread to be canceled.
 *
 * @result           N/A, never returns
 */
VXITRD_API void VXItrdThreadCancel(VXItrdThread *thread)
{
	  //before we go ahead and kill the thread, lets make sure it's still running
	  DWORD ExitCode;
	  GetExitCodeThread( thread,&ExitCode); 

	  if (ExitCode == STILL_ACTIVE)	
          TerminateThread(thread, 0);          // first get rid of the thread
                                             //  ignore the return code since
                                             //  it's possible the thread has
                                             //  already terminated
}


/**
 * Purpose Terminate a thread.  Called by the thread on exit.
 *
 * @param  status  Exit code for the thread
 * @result           N/A, never returns
 */
VXITRD_API void VXItrdThreadExit(VXItrdThreadArg status)
{
  _endthreadex(reinterpret_cast<unsigned int>(status));
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

  if (timeout == -1)
    timeout = INFINITE;

  VXItrdResult rc;
  switch (WaitForSingleObject(thread->threadHandle, timeout)) {
  case WAIT_OBJECT_0:
  case WAIT_ABANDONED:
    rc = VXItrd_RESULT_SUCCESS;
    break;
  case WAIT_TIMEOUT:
    rc = VXItrd_RESULT_FAILURE;
    break;
  default:
    rc = VXItrd_RESULT_FATAL_ERROR;
  }

  DWORD stat;
  if (rc == VXItrd_RESULT_SUCCESS) {
    if (GetExitCodeThread(thread->threadHandle, &stat) == 0)
      rc = VXItrd_RESULT_FATAL_ERROR;
    else
      *status = reinterpret_cast<VXItrdThreadArg>(stat);
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
  if (thread == NULL) return -1;
  return thread->threadID;
}


/**
 * Purpose Get the ID of the current handle.
 *
 * @return  The current thread handle identifier.
 *
 */
VXITRD_API VXIlong VXItrdThreadGetID(void)
{
  return GetCurrentThreadId();
}


/**
 * Purpose Yield the process schedule in the current thread.
 *
 * @return  void
 *
 */
VXITRD_API void VXItrdThreadYield(void)
{
  Sleep(0);
}


/**
 * Purpose  Create a timer
 *
 * @param   mutex  a pointer to a mutex
 * @return  VXItrdResult of operation.  Return SUCCESS if timer has been 
 *          created
 *
 */
VXITRD_API VXItrdResult VXItrdTimerCreate(VXItrdTimer **timer)
{
  if (timer == NULL) return VXItrd_RESULT_INVALID_ARGUMENT;
  
  *timer = NULL;
  
  // Create the wrapper object
  VXItrdTimer *result = new VXItrdTimer;
  if (result == NULL)
    return VXItrd_RESULT_OUT_OF_MEMORY;

  // Create the timer, a Win32 event
  result->timerEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
  if (result->timerEvent == NULL) {
    delete result;
    result = NULL;
    return VXItrd_RESULT_FATAL_ERROR;
  }

  *timer = result;
  return VXItrd_RESULT_SUCCESS;
}


/**
 * Purpose  Destroy a timer
 *
 * @param   timer  a pointer to a timer
 * @return  VXItrdResult of operation.  Return SUCCESS if timer has been 
 *          created
 *
 */
VXITRD_API VXItrdResult VXItrdTimerDestroy(VXItrdTimer **timer)
{
  if ((! timer) || (! *timer)) return VXItrd_RESULT_INVALID_ARGUMENT;

  if (CloseHandle((*timer)->timerEvent) == FALSE) 
    return VXItrd_RESULT_FATAL_ERROR;

  delete *timer;
  *timer = NULL;
  return VXItrd_RESULT_SUCCESS;
}


/**
 * Purpose  Puts the current thread to sleep for a configurable duration.
 *          Due to other activities of the machine, the delay is the minimum
 *          length that the timer will wait.
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

  switch (WaitForSingleObject(timer->timerEvent, millisecondDelay)) {
  case WAIT_OBJECT_0:
    if (interrupted != NULL) *interrupted = 1;
    break;
  case WAIT_TIMEOUT:
    if (interrupted != NULL) *interrupted = 0;
    break;
  default:
    return VXItrd_RESULT_FATAL_ERROR;
  }

  return VXItrd_RESULT_SUCCESS;
}


/**
 * Purpose  Wakes a sleeping thread, if the target is not already sleeping
 *          it immediately wakes when it tries to sleep the next time.
 *
 * @param   timer  a pointer to a timer
 * @return  VXItrdResult of operation.  Return SUCCESS if timer could wake.
 *
 */
VXITRD_API VXItrdResult VXItrdTimerWake(VXItrdTimer *timer)
{
  if (timer == NULL) return VXItrd_RESULT_INVALID_ARGUMENT;

  if (SetEvent(timer->timerEvent) == 0) return VXItrd_RESULT_FATAL_ERROR;

  return VXItrd_RESULT_SUCCESS;
}
