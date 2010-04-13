//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsBSemWnt_h_
#define _OsBSemWnt_h_

// SYSTEM INCLUDES
#include <windows.h>

// APPLICATION INCLUDES
#include "os/OsBSem.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS

// TYPEDEFS
typedef HANDLE WinSemaphore;

// FORWARD DECLARATIONS

//:Binary semaphore for Windows NT
class OsBSemWnt : public OsBSemBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   OsBSemWnt(const int queueOptions, const int initState);
     //:Constructor

   virtual
   ~OsBSemWnt();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   virtual OsStatus acquire(const OsTime& rTimeout = OsTime::OS_INFINITY);
     //:Block the task until the semaphore is acquired or the timeout expires

   virtual OsStatus tryAcquire(void);
     //:Conditionally acquire the semaphore (i.e., don't block)
     // Return OS_BUSY if the semaphore is held by some other task

   virtual OsStatus release(void);
     //:Release the semaphore

/* ============================ ACCESSORS ================================= */

   virtual void OsBSemShow(void);
     //:Print semaphore information to the console

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   WinSemaphore mSemImp;  // Windows NT semaphore

   OsBSemWnt();
     //:Default constructor (not implemented for this class)

   OsBSemWnt(const OsBSemWnt& rOsBSemWnt);
     //:Copy constructor (not implemented for this class)

   OsBSemWnt& operator=(const OsBSemWnt& rhs);
     //:Assignment operator (not implemented for this class)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsBSemWnt_h_
