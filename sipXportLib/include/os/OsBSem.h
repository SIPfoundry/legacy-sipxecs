//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsBSem_h_
#define _OsBSem_h_

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

//:Binary semaphore
class OsBSemBase : public OsSyncBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum InitialSemaphoreState
   {
      EMPTY = 0,  // semaphore is initially unavailable
      FULL  = 1   // semaphore is initially available
   };
     //!enumcode: EMPTY - the semaphore is unavailable initially
     //!enumcode: FULL - the semaphore is available initially

   enum QueueOptions
   {
      Q_FIFO     = 0x0, // queue blocked tasks on a first-in, first-out basis
      Q_PRIORITY = 0x1  // queue blocked tasks based on their priority
   };
     //!enumcode: Q_FIFO - queues blocked tasks on a first-in, first-out basis
     //!enumcode: Q_PRIORITY - queues blocked tasks based on their priority

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

   virtual OsStatus acquire(const OsTime& rTimeout = OsTime::OS_INFINITY) = 0;
     //:Block the task until the semaphore is acquired or the timeout expires

   virtual OsStatus tryAcquire(void) = 0;
     //:Conditionally acquire the semaphore (i.e., don't block)
     // Return OS_BUSY if the semaphore is held by some other task.

   virtual OsStatus release(void) = 0;
     //:Release the semaphore

/* ============================ ACCESSORS ================================= */

   virtual void OsBSemShow(void) = 0;
     //:Print semaphore information to the console

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   int mOptions;  // options specified at time of binary semaphore creation

   OsBSemBase()  {  };
     //:Default constructor

   virtual
      ~OsBSemBase()  {  };
     //:Destructor

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   OsBSemBase(const OsBSemBase& rOsBSemBase);
     //:Copy constructor (not implemented for this class)

   OsBSemBase& operator=(const OsBSemBase& rhs);
     //:Assignment operator (not implemented for this class)

};

/* ============================ INLINE METHODS ============================ */

// Depending on the native OS that we are running on, we include the class
// declaration for the appropriate lower level implementation and use a
// "typedef" statement to associate the OS-independent class name (OsBSem)
// with the OS-dependent realization of that type (e.g., OsBSemWnt).
#if defined(_WIN32)
#  include "os/Wnt/OsBSemWnt.h"
   typedef class OsBSemWnt OsBSem;
#elif defined(_VXWORKS)
#  include "os/Vxw/OsBSemVxw.h"
   typedef class OsBSemVxw OsBSem;
#elif defined(__pingtel_on_posix__)
#  include "os/linux/OsBSemLinux.h"
   typedef class OsBSemLinux OsBSem;
#else
#  error Unsupported target platform.
#endif

#endif  // _OsBSem_h_
