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
 *
 * 
 *
 * Thread API
 *
 * API for basic thread operations and locks. Unlike most of the other
 * VXI APIs, this is implemented in a library (on Windows a DLL with a
 * specific name, on other operating systems as a static, shared, or
 * dynamic library). Implementations of this API are operating system
 * dependant.
 *
 * To avoid cyclic dependancies, this does not perform logging. Clients
 * must do error logging themselves based on passed return codes.
 *
 ************************************************************************
 */

#ifndef _VXITRD_H
#define _VXITRD_H

#include "VXItypes.h"                  /* For VXIint, VXIbool, etc.  */

#include "VXIheaderPrefix.h"
#ifdef VXITRD_EXPORTS
#define VXITRD_API SYMBOL_EXPORT_DECL
#else
#define VXITRD_API SYMBOL_IMPORT_DECL
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
struct VXItrdMutex;
struct VXItrdThread;
struct VXItrdTimer;
#else
typedef struct VXItrdMutex   { void * dummy; } VXItrdMutex;
typedef struct VXItrdThread  { void * dummy; } VXItrdThread;
typedef struct VXItrdTimer   { void * dummy; } VXItrdTimer;
#endif

 /**
  * @name VXItrd
  * @memo Thread library
  * @version 1.0
  * @doc
  * C function library for basic mutex, thread, and timer
  * functionality to ensure portability across a wide variety of
  * operating systems and integration models (multi-threaded,
  * multi-process, etc.).
  */

/*@{*/

/* Function signature invoked on the new thread by VXItrdThreadCreate( ),
 * and the argument to that function.
 */
typedef void * VXItrdThreadArg;

#ifdef WIN32

typedef VXItrdThreadArg (__stdcall *VXItrdThreadStartFunc)(
                           VXItrdThreadArg userData);
#define VXITRD_DEFINE_THREAD_FUNC(funcName, userData) \
  VXItrdThreadArg __stdcall funcName(VXItrdThreadArg userData)

#else

typedef VXItrdThreadArg (*VXItrdThreadStartFunc)(
					  VXItrdThreadArg userData);
#define VXITRD_DEFINE_THREAD_FUNC(funcName, userData) \
  VXItrdThreadArg funcName(VXItrdThreadArg userData)

#endif

/**
 * Result codes for functions
 * 
 * Result codes less then zero are severe errors (likely to be
 * platform faults), those greater then zero are warnings (likely to
 * be application issues) 
 */
typedef enum VXItrdResult {
  /* Fatal error, terminate call    */
  VXItrd_RESULT_FATAL_ERROR       =  -100, 
  /* I/O error                      */
  VXItrd_RESULT_IO_ERROR           =   -8,
  /* Out of memory                  */
  VXItrd_RESULT_OUT_OF_MEMORY      =   -7, 
  /* System error, out of service   */
  VXItrd_RESULT_SYSTEM_ERROR       =   -6, 
  /* Errors from platform services  */
  VXItrd_RESULT_PLATFORM_ERROR     =   -5, 
  /* Return buffer too small        */
  VXItrd_RESULT_BUFFER_TOO_SMALL   =   -4, 
  /* Property name is not valid    */
  VXItrd_RESULT_INVALID_PROP_NAME  =   -3, 
  /* Property value is not valid   */
  VXItrd_RESULT_INVALID_PROP_VALUE =   -2, 
  /* Invalid function argument      */
  VXItrd_RESULT_INVALID_ARGUMENT   =   -1, 
  /* Success                        */
  VXItrd_RESULT_SUCCESS            =    0,
  /* Normal failure, nothing logged */
  VXItrd_RESULT_FAILURE            =    1,
  /* Non-fatal non-specific error   */
  VXItrd_RESULT_NON_FATAL_ERROR    =    2, 
  /* Operation is not supported     */
  VXItrd_RESULT_UNSUPPORTED        =  100
} VXItrdResult;


/**
 * Create a mutex
 *
 * @param  mutex  Handle to the created mutex
 *
 * @result        VXItrdResult 0 on success
 */
VXITRD_API VXItrdResult VXItrdMutexCreate(VXItrdMutex **mutex);

/**
 * Destroy a mutex
 *
 * @param  mutex  Handle to the mutex to destroy
 *
 * @result        VXItrdResult 0 on success
 */
VXITRD_API VXItrdResult VXItrdMutexDestroy(VXItrdMutex **mutex);

/**
 * Lock a mutex
 *
 * @param  mutex  Handle to the mutex to lock
 *
 * @result        VXItrdResult 0 on success
 */
VXITRD_API VXItrdResult VXItrdMutexLock(VXItrdMutex *mutex);

/**
 * Unlock a Mutex
 *
 * @param  mutex  Handle to the mutex to unlock
 *
 * @result        VXItrdResult 0 on success
 */
VXITRD_API VXItrdResult VXItrdMutexUnlock(VXItrdMutex *mutex);

/**
 * Create a thread
 *
 * Note thread values are not supported on some older operating
 * systems (such as IBM OS/2). Execution starts on the thread
 * immediately. To pause execution use a mutex between the thread and
 * the thread creator.
 *
 * @param  thread       Handle to the thread that is created
 * @param  startFunc    Function for the thread to start execution on
 * @param  arg          Argument to the thread function
 *  
 * @result              VXItrdResult 0 on success
 */
VXITRD_API VXItrdResult VXItrdThreadCreate(VXItrdThread          **thread,
					   VXItrdThreadStartFunc   startFunc,
					   VXItrdThreadArg         arg);

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
VXITRD_API VXItrdResult VXItrdThreadDestroyHandle(VXItrdThread **thread);

/**
 * Terminate a thread (called by the thread to exit)
 *
 * @param  thread    the thread to be canceled.
 *
 * @result           N/A, never returns
 */
VXITRD_API void VXItrdThreadCancel(VXItrdThread *thread);

/**
 * Terminate a thread (called by the thread to exit)
 *
 * @param  status    Exit value of the thread
 *
 * @result           N/A, never returns
 */
VXITRD_API void VXItrdThreadExit(VXItrdThreadArg status);

/**
 * Wait for the termination of a specified thread
 *
 * @param  thread   Handle to the thread to wait for
 * @param  status   Set to the exit value of the thread's start routine
 * @param  timeout  Timeout, in milliseconds, for waiting for the thread
 *                  to exit, pass -1 to wait forever
 *
 * @result VXItrdResult 0 on success, VXItrd_RESULT_FAILURE on timeout 
 */
VXITRD_API VXItrdResult VXItrdThreadJoin(VXItrdThread *thread,
					 VXItrdThreadArg *status,
					 long timeout);

/**
 * Get the thread ID for the specified thread
 *
 * @param  thread   Handle to the thread to get the ID for
 *
 * @result   Thread ID number
 */
VXITRD_API VXIlong VXItrdThreadGetIDFromHandle(VXItrdThread *thread);

/**
 * Get the thread ID for the current thread
 *
 * @result   Thread ID number
 */
VXITRD_API VXIlong VXItrdThreadGetID(void);

/**
 * Yield execution of the current thread to other threads/processes
 *
 * @result   N/A, no return value, always succeeds
 *
 */
VXITRD_API void VXItrdThreadYield(void);


/**
 * Create a timer
 *
 * @param  timer   Handle to the created timere
 *
 * @result         VXItrdResult 0 on success
 */
VXITRD_API VXItrdResult VXItrdTimerCreate(VXItrdTimer **timer);

/**
 * Destroy a timer
 *
 * @param  timer   Handle to the timer to destroy
 *
 * @result         VXItrdResult 0 on success
 */
VXITRD_API VXItrdResult VXItrdTimerDestroy(VXItrdTimer **timer);

/**
 * Suspend the current thread for a time period using a timer
 *
 * Note: due to other activities of the machine, the delay may be greater
 * then the configured duration.
 *
 * @param  timer        Handle to the timer to use to execute the suspend
 * @param  sleepMs      Duration to sleep, in milliseconds
 * @param  interrupted  Pointer indicating whether or not the sleep was 
 *                      interrupted by VXItrdTimerWake, TRUE if interrupted,
 *                      FALSE if not. Pass NULL if this information is not 
 *                      desired.
 * 
 * @result              VXItrdResult 0 on success
 */
VXITRD_API VXItrdResult VXItrdTimerSleep(VXItrdTimer *timer,
					 VXIint       sleepMs,
					 VXIbool     *interrupted);

/**
 * Wakes a thread that is sleeping on a timer
 *
 * Note: if no thread is currently waiting on the specified timer, the
 * next VXItrdTimerSleep( ) call on that timer will immediately wake
 * up.
 *
 * @param  timer        Handle to the timer to wake up
 * 
 * @result              VXItrdResult 0 on success
 */
VXITRD_API VXItrdResult VXItrdTimerWake(VXItrdTimer *timer);

/*@}*/

#ifdef __cplusplus
}
#endif

#include "VXIheaderSuffix.h"

#endif  /* include guard */
