//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#ifndef _OsTaskLinux_h_
#define _OsTaskLinux_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsMutex.h"
#include "os/OsBSem.h"
#include "os/OsStatus.h"
#include "os/OsTask.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class OsTime;

/// Task abstraction for Linux.
/**
 * A task represents a thread of execution. All tasks run within the same
 * address space but have their own stack and program counter. Tasks may be
 * created and deleted dynamically.
 *
 * Users create tasks by:
 * # Deriving a new class based on OsTask or one of its descendants,
 *   and overriding the run() method in the derived class.
 * # Calling the constructor for the derived class.
 * # Invoking the start() method for the derived class.  This creates the
 *   corresponding low-level OS task and associates it with the class.
 *
 * Note: Many of the methods in this class are only applicable once the
 * start() method for the object has been called and the corresponding
 * low-level task has been created.  Accordingly, before a successful call
 * to start(), most of the methods in this class return the
 * OS_TASK_NOT_STARTED status.
 */
class OsTaskLinux : public OsTaskBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum LinuxPriorities
   {
      RT_NO = 0,     // Non-realtime priority, does not use realtime scheduling
      RT_LOW = 1,    // Lowest realtime priority
      RT_NORMAL = 2,
      RT_HIGH = 3,
      RT_HIGHEST = 4 // Highest realtime priority
   };

   /// Constructor
   OsTaskLinux(const UtlString& name ="" /* TODO - how much will this break? */,
             void* pArg=NULL,
             const int priority=DEF_PRIO,
             const int options=DEF_OPTIONS,
             const int stackSize=DEF_STACKSIZE);

   /// Destructor -- delete the task
   virtual
   ~OsTaskLinux();

   /// The entry point for the task.
   virtual int run(void* pArg) = 0;
   /**<
    * Derive new tasks as subclasses of OsTask, overriding this method.
    */

   /// Spawn a new task and invoke its run() method.
   virtual UtlBoolean start(void);
   /**<
   * @returns TRUE if the spawning of the new task is successful,
   * FALSE if the task spawn fails or if the task has already
   * been started.
   */

   /// Suspend the task.
   virtual OsStatus suspend(void);
   /**<
    * This routine suspends the task. Suspension is additive: thus, tasks
    * can be delayed and suspended, or pended and suspended. Suspended,
    * delayed tasks whose delays expire remain suspended. Likewise,
    * suspended, pended tasks that unblock remain suspended only.
    */

   /// Resume the task.
   virtual OsStatus resume(void);
   /**<
    * This routine resumes the task. The task suspension is cleared, and
    * the task operates in the remaining state.
    */

   /// Set the errno status for the task.
   virtual OsStatus setErrno(int errno);

   /// Set the priority of the task.
   virtual OsStatus setPriority(int priority);
   /**<
    * Priorities range from 0, the highest priority, to 255, the lowest
    * priority.
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
   virtual OsStatus awaitSignal(int& sig_num);
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

   /// Return a pointer to the OsTask object for the currently executing task.
   static OsTaskLinux* getCurrentTask(void);
   ///< @returns NULL if none exists.

   /// Return an Id of the currently executing task.
   static OsStatus getCurrentTaskId(pthread_t &rid);

   /// Return a pointer to the OsTask object corresponding to the named task.
   static OsTaskLinux* getTaskByName(const UtlString& taskName);
   ///< @returns NULL if there is no task object with that name.

   /// Return a pointer to the OsTask object corresponding to taskId.
   static OsTaskLinux* getTaskById(const pthread_t taskId);
   ///< @returns NULL if there is no task object with that id.

   /// Get the errno status for the task.
   virtual OsStatus getErrno(int& rErrno);

   /// Return the execution options for the task.
   virtual int getOptions(void);

   /// Return the priority of the task.
   virtual OsStatus getPriority(int& rPriority);

   /// Get the task ID for this task.
   virtual OsStatus id(pthread_t& rId);

   // Check if the task is ready to run.
   virtual UtlBoolean isReady(void);
   ///< @returns TRUE if the task is ready, otherwise FALSE.

   /// Check if the task is suspended.
   virtual UtlBoolean isSuspended(void);
   ///< @returns TRUE is the task is suspended, otherwise FALSE.


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /// Wait until the task is shut down and the run method has exited.
   virtual UtlBoolean waitUntilShutDown(int milliSecToWait = 20 * OsTime::MSECS_PER_SEC);
   /**<
    * Any subclass of OsTask should call this method in
    * the destructor to protect access to any members which are
    * used by the run method.
    *
    * @NOTE If milliSecToWait expires, the thread is assumed to be hung,
    * and the entire process is aborted with a critical error.
    */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   pthread_t mTaskId;      ///< Linux unique ID for the thread
   OsBSem    mDeleteGuard; ///< Mutex guard to prevent unwanted task deletion
   int       mSuspendCnt;  ///< Counts the nesting level of suspend() calls

   // saved initialization information (used for task restarts)
   int       mOptions;
   int       mPriority;
   int       mStackSize;

   /// Function that serves as the starting address for a Linux task.
   static void * taskEntry(void* arg);

   /// Acknowledge a shutdown request.
   virtual void ackShutdown(void);
   /**<
    * This method is called at the end of taskEntry, after the
    * run() method has returned, so the thread is no longer running.
    */

   /// Copy constructor (not implemented for this class).
   OsTaskLinux(const OsTaskLinux& rOsTaskLinux);

   /// Assignment operator (not implemented for this class).
   OsTaskLinux& operator=(const OsTaskLinux& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsTaskLinux_h_
