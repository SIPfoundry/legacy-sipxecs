//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#ifndef _OsMutexLinux_h_
#define _OsMutexLinux_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsMutex.h"
#include "os/linux/OsLinuxDefs.h"
#include "os/linux/pt_mutex.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Mutual exclusion semaphore (mutex) for Linux
// The mutual-exclusion semaphore is a specialized version of the binary
// semaphore, designed to address issues inherent in mutual exclusion, such
// as recursive access to resources, priority inversion, and deletion safety
// The fundamental behavior of the mutual-exclusion semaphore is identical to
// except for the following restrictions: it can only be used for mutual
// exclusion and it can only be released by the task that acquired it.
class OsMutexLinux : public OsMutexBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   OsMutexLinux(const unsigned options);
     //:Constructor

   virtual
   ~OsMutexLinux();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   virtual OsStatus acquire(const OsTime& rTimeout = OsTime::OS_INFINITY);
     //:Block the task until the semaphore is acquired or the timeout expires

   virtual OsStatus tryAcquire(void);
     //:Conditionally acquire the mutex (i.e., don't block)
     // Return OS_BUSY if the mutex is held by some other task

   virtual OsStatus release(void);
     //:Release the semaphore

/* ============================ ACCESSORS ================================= */

   virtual void OsMutexShow(void);
     //:Print mutex information to the console.

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   pt_mutex_t mMutexImp;  // Pingtel-Linux mutex

   OsMutexLinux();
     //:Default constructor (not implemented for this class)

   OsMutexLinux(const OsMutexLinux& rOsMutexLinux);
     //:Copy constructor (not implemented for this class)

   OsMutexLinux& operator=(const OsMutexLinux& rhs);
     //:Assignment operator (not implemented for this class)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsMutexLinux_h_
