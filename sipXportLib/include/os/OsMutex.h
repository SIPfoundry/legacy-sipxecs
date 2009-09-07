//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsMutex_h_
#define _OsMutex_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsSyncBase.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Mutual exclusion semaphore (mutex)
// The mutual-exclusion semaphore is a specialized version of the binary
// semaphore, designed to address issues inherent in mutual exclusion, such
// as recursive access to resources, priority inversion, and deletion safety.


// The fundamental behavior of the mutual-exclusion semaphore is identical to
// that of a binary semaphore except for:

// - It can only be used for mutual exclusion and it can only be released by
// the thread that acquired it.

// - If a thread already holds the mutex, it may acquire it again without
// first releasing it.  The thread must release the mutex as many times as it
// has acquired it before the mutex is free to be acquired by another thread.

class OsMutexBase : public OsSyncBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum MutexOptions
   {
      Q_FIFO         = 0x0, // queue blocked tasks on a first-in, first-out
                            //  basis
      Q_PRIORITY     = 0x1, // queue blocked tasks based on their priority
      DELETE_SAFE    = 0x4, // protect a task that owns the mutex from
                            //  unexpected deletion
      INVERSION_SAFE = 0x8  // protect the system from priority inversion: NOTE
                            //  VxWorks requires Q_PRIORITY with this.
   };
     //!enumcode: Q_FIFO - queues blocked tasks on a first-in, first-out basis
     //!enumcode: Q_PRIORITY - queues blocked tasks based on their priority
     //!enumcode: DELETE_SAFE - protects a task that owns the mutex from unexpected deletion
     //!enumcode: INVERSION_SAFE - protects the system from priority inversion

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

   virtual OsStatus acquire(const OsTime& rTimeout = OsTime::OS_INFINITY) = 0;
     //:Block the task until the mutex is acquired or the timeout expires

   virtual OsStatus tryAcquire(void) = 0;
     //:Conditionally acquire the mutex (i.e., don't block)
     // Return OS_BUSY if the mutex is held by some other task

   virtual OsStatus release(void) = 0;
     //:Release the mutex

/* ============================ ACCESSORS ================================= */

   virtual void OsMutexShow(void) = 0;
     //:Print mutex information to the console.

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   OsMutexBase() { };
     //:Default constructor

   virtual
   ~OsMutexBase() { };
     //:Destructor

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   OsMutexBase(const OsMutexBase& rOsMutexBase);
     //:Copy constructor (not implemented for this class)

   OsMutexBase& operator=(const OsMutexBase& rhs);
     //:Assignment operator (not implemented for this class)

};

/* ============================ INLINE METHODS ============================ */

// Depending on the native OS that we are running on, we include the class
// declaration for the appropriate lower level implementation and use a
// "typedef" statement to associate the OS-independent class name (OsMutex)
// with the OS-dependent realization of that type (e.g., OsMutexWnt).
#if defined(_WIN32)
#  include "os/Wnt/OsMutexWnt.h"
   typedef class OsMutexWnt OsMutex;
#elif defined(_VXWORKS)
#  include "os/Vxw/OsMutexVxw.h"
   typedef class OsMutexVxw OsMutex;
#elif defined(__pingtel_on_posix__)
#  include "os/linux/OsMutexLinux.h"
   typedef class OsMutexLinux OsMutex;
#else
#  error Unsupported target platform.
#endif

#endif  // _OsMutex_h_
