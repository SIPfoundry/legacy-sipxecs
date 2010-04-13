//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsMutexWnt_h_
#define _OsMutexWnt_h_

// SYSTEM INCLUDES
#include <windows.h>

// APPLICATION INCLUDES
#include "os/OsMutex.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS

// TYPEDEFS
typedef HANDLE WinMutex;

// FORWARD DECLARATIONS

//:Mutual exclusion semaphore (mutex) for Windows NT
// The mutual-exclusion semaphore is a specialized version of the binary
// semaphore, designed to address issues inherent in mutual exclusion, such
// as recursive access to resources, priority inversion, and deletion safety
// The fundamental behavior of the mutual-exclusion semaphore is identical to
// except for the following restrictions: it can only be used for mutual
// exclusion and it can only be released by the task that acquired it.
class OsMutexWnt : public OsMutexBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   OsMutexWnt(const unsigned options);
     //:Constructor

   virtual
   ~OsMutexWnt();
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
   WinMutex mMutexImp;  // Windows NT mutex

   OsMutexWnt();
     //:Default constructor (not implemented for this class)

   OsMutexWnt(const OsMutexWnt& rOsMutexWnt);
     //:Copy constructor (not implemented for this class)

   OsMutexWnt& operator=(const OsMutexWnt& rhs);
     //:Assignment operator (not implemented for this class)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsMutexWnt_h_
