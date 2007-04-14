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
#include "os/OsRWMutex.h"
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

//:Task abstraction for Linux
// A task represents a thread of execution. All tasks run within the same
// address space but have their own stack and program counter. Tasks may be
// created and deleted dynamically.
//
// Users create tasks by:
// 1) Deriving a new class based on OsTask or one of its descendants,
//    and overriding the run() method in the derived class.
// 2) Calling the constructor for the derived class.
// 3) Invoking the start() method for the derived class.  This creates the
//    corresponding low-level OS task and associates it with the class.
//
// Note: Many of the methods in this class are only applicable once the
// start() method for the object has been called and the corresponding
// low-level task has been created.  Accordingly, before a successful call
// to start(), most of the methods in this class return the
// OS_TASK_NOT_STARTED status.
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

/* ============================ CREATORS ================================== */

   OsTaskLinux(const UtlString& name="",
             void* pArg=NULL,
             const int priority=DEF_PRIO,
             const int options=DEF_OPTIONS,
             const int stackSize=DEF_STACKSIZE);
     //:Constructor

   virtual
   ~OsTaskLinux();
     //:Destructor -- delete the task

   virtual OsStatus deleteForce(void);
     //:Delete the task even if the task is protected from deletion
     // After calling this method, the user will still need to delete the
     // corresponding OsTask object to reclaim its storage.

/* ============================ MANIPULATORS ============================== */

   virtual UtlBoolean restart(void);
     //:Restart the task
     // The task is first terminated, and then reinitialized with the same
     // name, priority, options, stack size, original entry point, and
     // parameters it had when it was terminated.
     // Return TRUE if the restart of the task is successful.

   virtual OsStatus resume(void);
     //:Resume the task
     // This routine resumes the task. The task suspension is cleared, and
     // the task operates in the remaining state.

   virtual int run(void* pArg) = 0;
     //:The entry point for the task.
     // Derive new tasks as subclasses of OsTask, overriding this method.

   virtual UtlBoolean start(void);
     //:Spawn a new task and invoke its run() method.
     // Return TRUE if the spawning of the new task is successful.
     // Return FALSE if the task spawn fails or if the task has already
     // been started.
     
   virtual OsStatus suspend(void);
     //:Suspend the task
     // This routine suspends the task. Suspension is additive: thus, tasks
     // can be delayed and suspended, or pended and suspended. Suspended,
     // delayed tasks whose delays expire remain suspended. Likewise,
     // suspended, pended tasks that unblock remain suspended only.

   virtual OsStatus setErrno(int errno);
     //:Set the errno status for the task

   virtual OsStatus setOptions(int options);
     //:Set the execution options for the task
     // The only option that can be changed after a task has been created
     // is whether to allow breakpoint debugging.

   virtual OsStatus setPriority(int priority);
     //:Set the priority of the task
     // Priorities range from 0, the highest priority, to 255, the lowest
     // priority.

   virtual OsStatus varAdd(int* pVar);
     //:Add a task variable to the task
     // This routine adds a specified variable pVar (4-byte memory
     // location) to its task's context. After calling this routine, the
     // variable is private to the task. The task can access and modify
     // the variable, but the modifications are not visible to other tasks,
     // and other tasks' modifications to that variable do not affect the
     // value seen by the task. This is accomplished by saving and restoring
     // the variable's value each time a task switch occurs to or from the
     // calling task.

   virtual OsStatus varDelete(int* pVar);
     //:Remove a task variable from the task
     // This routine removes a specified task variable, pVar, from its
     // task's context. The private value of that variable is lost.

   virtual OsStatus varSet(int* pVar, int value);
     //:Set the value of a private task variable
     // This routine sets the private value of the task variable for a
     // specified task. The specified task is usually not the calling task,
     // which can set its private value by directly modifying the variable.
     // This routine is provided primarily for debugging purposes.

   static OsStatus delay(const int milliSecs);
     //:Delay a task from executing for the specified number of milliseconds
     // This routine causes the calling task to relinquish the CPU for the
     // duration specified. This is commonly referred to as manual
     // rescheduling, but it is also useful when waiting for some external
     // condition that does not have an interrupt associated with it.

   static OsStatus lock(void);
     //:Disable rescheduling for the currently executing task
     // This routine disables task context switching. The task that calls
     // this routine will be the only task that is allowed to execute,
     // unless the task explicitly gives up the CPU by making itself no
     // longer ready. Typically this call is paired with unlock();
     // together they surround a critical section of code. These
     // preemption locks are implemented with a counting variable that
     // allows nested preemption locks. Preemption will not be unlocked
     // until unlock() has been called as many times as lock().

   static OsStatus unlock(void);
     //:Enable rescheduling for the currently executing task
     // This routine decrements the preemption lock count. Typically
     // this call is paired with lock() and concludes a critical
     // section of code. Preemption will not be unlocked until
     // unlock() has been called as many times as lock(). When
     // the lock count is decremented to zero, any tasks that were
     // eligible to preempt the current task will execute.

   static OsStatus safe(void);
     //:Make the calling task safe from deletion
     // This routine protects the calling task from deletion. Tasks that
     // attempt to delete a protected task will block until the task is
     // made unsafe, using unsafe(). When a task becomes unsafe, the
     // deleter will be unblocked and allowed to delete the task.
     // The safe() primitive utilizes a count to keep track of
     // nested calls for task protection. When nesting occurs,
     // the task becomes unsafe only after the outermost unsafe()
     // is executed.

   static OsStatus unsafe(void);
     //:Make the calling task unsafe from deletion
     // This routine removes the calling task's protection from deletion.
     // Tasks that attempt to delete a protected task will block until the
     // task is unsafe. When a task becomes unsafe, the deleter will be
     // unblocked and allowed to delete the task.
     // The unsafe() primitive utilizes a count to keep track of nested
     // calls for task protection. When nesting occurs, the task becomes
     // unsafe only after the outermost unsafe() is executed.

   static void yield(void);
     //:Yield the CPU if a task of equal or higher priority is ready to run.

/* ============================ ACCESSORS ================================= */

   static OsTaskLinux* getCurrentTask(void);
     //:Return a pointer to the OsTask object for the currently executing task
     // Return NULL if none exists.

   static OsStatus getCurrentTaskId(int &rid);
     //:Return an Id of the currently executing task

   static OsTaskLinux* getTaskByName(const UtlString& taskName);
     //:Return a pointer to the OsTask object corresponding to the named task
     // Return NULL if there is no task object with that name.

   static OsTaskLinux* getTaskById(const int taskId);
     //:Return a pointer to the OsTask object corresponding to taskId
     // Return NULL is there is no task object with that id.

   virtual OsStatus getErrno(int& rErrno);
     //:Get the errno status for the task

   virtual int getOptions(void);
     //:Return the execution options for the task

   virtual OsStatus getPriority(int& rPriority);
     //:Return the priority of the task

   virtual OsStatus varGet(void);
     //:Get the value of a task variable
     // This routine returns the private value of a task variable for its
     // task. The task is usually not the calling task, which can get its
     // private value by directly accessing the variable. This routine is
     // provided primarily for debugging purposes.

/* ============================ INQUIRY =================================== */

   virtual OsStatus id(int& rId);
     //:Get the task ID for this task

   virtual UtlBoolean isReady(void);
     //:Check if the task is ready to run
     // Return TRUE is the task is ready, otherwise FALSE.
     // Under Windows NT, this method returns the opposite of isSuspended()

   virtual UtlBoolean isSuspended(void);
     //:Check if the task is suspended
     // Return TRUE is the task is suspended, otherwise FALSE.

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   pthread_t mTaskId;      // Linux unique ID for task
   OsRWMutex mDeleteGuard; // RWMutex guard to prevent unwanted task deletion
   int       mSuspendCnt;  // Counts the nesting level of suspend() calls

   // saved initialization information (used for task restarts)
   int       mOptions;
   int       mPriority;
   int       mStackSize;

   UtlBoolean doLinuxCreateTask(const char* pTaskName);
     //:Do the real work associated with creating a new VxWorks task
     // The mDataGuard lock should be held upon entry into this method.

   void doLinuxTerminateTask(UtlBoolean doForce);
     //:Do the real work associated with terminating a VxWorks task
     // The mDataGuard lock should be held upon entry into this method.

   /**
    * taskUnregister
    * remove mapping from the OsNameDb for this thread id
    */
   void taskUnregister(void);

   static void * taskEntry(void* arg);
     //:Function that serves as the starting address for a Linux task

   OsTaskLinux(const OsTaskLinux& rOsTaskLinux);
     //:Copy constructor (not implemented for this class)

   OsTaskLinux& operator=(const OsTaskLinux& rhs);
     //:Assignment operator (not implemented for this class)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsTaskLinux_h_

