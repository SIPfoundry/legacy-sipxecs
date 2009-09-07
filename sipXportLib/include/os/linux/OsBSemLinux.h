//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#ifndef _OsBSemLinux_h_
#define _OsBSemLinux_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsBSem.h"
#include "os/linux/OsLinuxDefs.h"
#include "os/linux/pt_csem.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Binary semaphore for Linux
class OsBSemLinux : public OsBSemBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   OsBSemLinux(const int queueOptions, const int initState);
     //:Constructor

   virtual
   ~OsBSemLinux();
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
   pt_sem_t mSemImp;              // Pingtel-Linux counting semaphore

   OsBSemLinux();
     //:Default constructor (not implemented for this class)

   OsBSemLinux(const OsBSemLinux& rOsBSemLinux);
     //:Copy constructor (not implemented for this class)

   OsBSemLinux& operator=(const OsBSemLinux& rhs);
     //:Assignment operator (not implemented for this class)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsBSemLinux_h_
