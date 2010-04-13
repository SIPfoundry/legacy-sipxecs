//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _OsTask_h_
#define _OsTask_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsMsgQ.h"
#include "os/OsMutex.h"
#include "os/OsStatus.h"
#include "os/OsSysLog.h"

// DEFINES
#define OSTASK_STACK_SIZE_1M 1024*1024
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class OsTime;

/// Task abstraction
/**
 * A task object encapsulates an execution thread.  All tasks run within the same
 * address space but each has its own stack and program counter. Tasks may be
 * created and deleted dynamically.
 *
 * A task is created by instantiating an object of some subclass of this one.
 * Instantiating the object allows certain properties of the task to be manipulated,
 * but does not actually instantiate the thread or start the task execution.
 *
 * To start the task, invoke the start() method for the derived class.  This creates
 * the corresponding low-level OS thread and associates it with the OsTask object.
 *
 * The normal way to stop the task is to delete the object: the destructor will block,
 * and invoke the requestShutdown method on the object.  The requestShutdown method sends
 * an indication to the task that it should exit, which the task should acknowledge by
 * calling OsTask::requestShutdown to set the state to SHUTTING_DOWN.  The task is run()
 * method is responsible for checking the state, or the derived class may override the
 * requestShutdown method to ensure that the indication is delivered.   When the task
 * exits, the ackShutdown method of this class sets the object state to TERMINATED, and
 * the destructor is allowed to complete.
 *
 * @Note if the destructor for the derived class is going to manipulate any members
 * that are also used by the executing thread, then it must call waitUntilShutDown
 * before using the member variables; this blocks until the thread has completed.
 *
 * @see OsServerTask for an abstraction suitable for any task that uses OsMsg events
 * as its primary method of communication with other tasks.
 *
 * Many of the methods in this class are only applicable once the
 * start() method for the object has been called and the corresponding
 * low-level task has been created.  Accordingly, before a successful call
 * to start(), most of the methods in this class return the
 * OS_TASK_NOT_STARTED status.
 *
 */
class OsTaskBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   static const int DEF_OPTIONS;         // default task execution options
   static const int DEF_PRIO;            // default task priority
   static const int DEF_STACKSIZE;       // default task stack size
   static const UtlString TASK_PREFIX;   // prefix for OsTask names stored in
                                         //  the name database
   static const UtlString TASKID_PREFIX; // prefix for taskID stored in the
                                         //  name database.

   static OsMutex sTaskCountMutex;       // Mutex to protect sTaskCount
   static int sTaskCount;                // Sequence number of created tasks

   /// The state of the thread associated with this OsTask object.
   enum TaskState
   {
      UNINITIALIZED,   ///< thread not created, no name DB entries
      RUNNING,         ///< thread running, name DB entries exist
      SHUTTING_DOWN,   ///< requested thread shutdown
      TERMINATED       ///< thread has exited, name DB entries still exist
   };

   static const char* TaskStateName(enum TaskState state);

   /// Spawn a new task and invoke its run() method..
   virtual UtlBoolean start(void) = 0;
   /**<
    * @Return TRUE if the spawning of the new task is successful,
    *         FALSE if the task spawn fails or if the task has already been started.
    */

   /// Request a task shutdown.
   virtual void requestShutdown(void);
   /**<
    * Normally, it is not necessary to explicitly call this method, since it is
    * implicitly called from waitUntilShutDown if the task is in RUNNING state.
    *
    * If the derived task class ever waits on anything, then it MUST either:
    * - Check isShuttingDown frequently enough to detect a shutdown request
    * or
    * - Override this method with something that wakes the task up and
    *   initiates shutdown processing.  The derived method should call
    *   OsTaskBase::requestShutdown to set mState to SHUTTING_DOWN.
    *
    * @see OsServerTask::requestShutdown for an example.
    */

   /// Suspend the task.
   virtual OsStatus suspend(void) = 0;
   /**<
    * This routine suspends the task. Suspension is additive: thus, tasks
    * can be delayed and suspended, or pended and suspended. Suspended,
    * delayed tasks whose delays expire remain suspended. Likewise,
    * suspended, pended tasks that unblock remain suspended only.
    */

   /// Resume the task.
   virtual OsStatus resume(void) = 0;
   /**<
    * This routine resumes the task. The task suspension is cleared, and
    * the task operates in the remaining state.
    */

   /// Set the errno status for the task.
   virtual OsStatus setErrno(int errno) = 0;
   /**<
    * The only option that can be changed after a task has been created
    * is whether to allow breakpoint debugging.
    */

   /// Set the priority of the task.
   virtual OsStatus setPriority(int priority) = 0;
   /**<
    * Priorities range from 0, the highest priority, to 255, the lowest
    * priority.
    */

   /// Set the userData for the task..
   virtual void setUserData(int data);
   /**<
    * The class does not use this information itself, but merely stores
    * it on behalf of the caller.
    */

   /// Adds a syslog entry to the system logger.
   virtual OsStatus syslog(const OsSysLogFacility facility, ///< See OsSysLogFacility.
                           const OsSysLogPriority priority, ///< See OsSysLogPriority.
                           const char*            format,   ///< sprintf format string
                           ... )
#                          ifdef __GNUC__
                            // with the -Wformat switch, this enables format string checking
                           __attribute__ ((format (printf, 4, 5)))
#                          endif
                           ;
   /**<
    * @deprecated please use OsSysLog::add
    */

   /// Block all signals from the calling thread
   static OsStatus blockSignals(void);
   /**<
    * Block all signals from this thread.  Static so "main()"
    * can call it to isolate it and all subsequent threads from signals.
    */

   /// Unblock all signals from the calling thread
   static OsStatus unBlockSignals(void);
   /**<
    * Unblock all signals from this thread.  Static so "main()"
    * can call it to unisolate it and all subsequent threads from signals.
    */

   /// wait for a signal
   virtual OsStatus awaitSignal(int& sig_num) = 0;
   /**<
    * Waits for an external signal to occur.  Returns the
    * signal number that happened in sig_num.
    */

   /// Delay a task from executing for the specified number of milliseconds.
   static OsStatus delay(const int milliSecs);
   /**<
    * This routine causes the calling task to relinquish the CPU for the
    * duration specified. This is commonly referred to as manual
    * rescheduling, but it is also useful when waiting for some external
    * condition that does not have an interrupt associated with it.
    */

   /// Yield the CPU if a task of equal or higher priority is ready to run.
   static void yield(void);

/* ============================ ACCESSORS ================================= */

   /// Return a pointer to the OsTask object for the currently executing task.
   static OsTaskBase* getCurrentTask(void);
   /**<
    * Return NULL if none exists.
    */

   /// Return an Id of the currently executing task.
   static OsStatus getCurrentTaskId(pthread_t &rid);
   /**<
    * This Id is unique within the current process, but not necessarily
    * over the entire host.
    * Any two simultaneous executions that share their memory space
    * will have different values from getCurrentTaskId().
    */

   /// Return a pointer to the OsTask object corresponding to the named task.
   static OsTaskBase* getTaskByName(const UtlString& taskName);
   /**<
    * Return NULL if there is no task object with that name.
    */

   /// Return a pointer to the OsTask object corresponding to taskId.
   static OsTaskBase* getTaskById(const pthread_t taskId);
   /**<
    * Return NULL is there is no task object with that id.
    */

   /// Get the void* value passed as an argument to the task.
   virtual void* getArg(void);

   /// Get the errno status for the task.
   virtual OsStatus getErrno(int& rErrno) = 0;

   /// Get the name associated with the task.
   virtual const UtlString& getName(void);

   /// Return the execution options for the task.
   virtual int getOptions(void) = 0;

   /// Return the priority of the task.
   virtual OsStatus getPriority(int& rPriority) = 0;

   /// Return the userData for the task..
   virtual int getUserData(void);

/* ============================ INQUIRY =================================== */

   /// Get the task ID for this task.
   virtual OsStatus id(pthread_t& rId) = 0;

   /// Check if the task is ready to run.
   virtual UtlBoolean isReady(void) = 0;
   /**<
    * Return TRUE is the task is ready, otherwise FALSE.
    */

   /// Return TRUE if a task shutdown has been requested and acknowledged.
   virtual UtlBoolean isShutDown(void);

   /// Return TRUE if a task shutdown has been requested but not acknowledged.
   virtual UtlBoolean isShuttingDown(void);

   /// Return TRUE if the task has been started (and has not been shut down).
   virtual UtlBoolean isStarted(void);

   /** Return TRUE if the task is uninitialized or running (that is,
    *  not shutting down or shut down).
    */
   virtual UtlBoolean isNotShut(void);

   /// Check if the task is suspended.
   virtual UtlBoolean isSuspended(void) = 0;
   /**<
    * Return TRUE is the task is suspended, otherwise FALSE.
    */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   OsMutex    mDataGuard; ///< protects mName and mState
   UtlString  mName;      ///< global name of the task (used in logging)

   TaskState mState;      ///< the relationship between this object and the actual thread

   OsTaskBase(const UtlString& name,
              void* pArg,
              const int priority,
              const int options,
              const int stackSize);

   virtual
   ~OsTaskBase();

   /// The entry point for the task.
   virtual int run(void* pArg) = 0;
   /**<
    * This method is called in the underlying thread context when
    * the thread begins execution (triggered by a call to the start method).
    * When this routine exits, the underlying thread is destroyed.
    *
    * This method and anything it calls must provide a means for shutting
    * down the task, which may be a simple as periodically checking the
    * isShuttingDown method, or may mean overriding the requestShutdown
    * method to provide some other synchronization primitive.
    */

   /** Wait until the task is shut down and the run method has exited.
    *  If the task is in RUNNING state, it first calls requestShutdown().
    */
   virtual UtlBoolean waitUntilShutDown(int milliSecToWait = 20 * OsTime::MSECS_PER_SEC) = 0;
   /**<
    * Any subclass of OsTask should call this method in
    * the destructor to protect access to any members which are
    * used by the run method.
    *
    * @NOTE If milliSecToWait expires, the thread is assumed to be hung,
    * and the entire process is aborted with a critical error.
    *
    * @NOTE This method is protected instead of public because
    * subclasses likely will require additional processing to shut
    * down the task correctly.  If this method needs to be exposed,
    * create a public method for the subclass that calls
    * waitUntilShutDown (and does any other needed processing).
    */

   /// Acknowledge a shutdown request.
   virtual void ackShutdown(void);
   /**<
    * The OsTask object calls this method just after the run() method exits to indidate that
    * the thread has stopped execution.
    */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   void*     mpArg;       ///< argument passed to the task
   int       mUserData;   /**< data stored on behalf of the user.  This data
                           *  is read/written via getUserData()/setUserData()
                           */

   OsTaskBase(const OsTaskBase& rOsTask);
     //:Copy constructor (not implemented for this class)

   OsTaskBase& operator=(const OsTaskBase& rhs);
     //:Assignment operator (not implemented for this class)
};

/* ============================ INLINE METHODS ============================ */

// Depending on the native OS that we are running on, we include the class
// declaration for the appropriate lower level implementation and use a
// "typedef" statement to associate the OS-independent class name (OsTask)
// with the OS-dependent realization of that type (e.g., OsTaskWnt).
#if defined(_WIN32)
#  include "os/Wnt/OsTaskWnt.h"
   typedef class OsTaskWnt OsTask;
#elif defined(__pingtel_on_posix__)
#  include "os/linux/OsTaskLinux.h"
   typedef class OsTaskLinux OsTask;
#else
#  error Unsupported target platform.
#endif

#endif  // _OsTask_h_
