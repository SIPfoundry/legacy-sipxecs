//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#ifndef _OsCSemLinux_h_
#define _OsCSemLinux_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsCSem.h"
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

//:Counting semaphore for Linux
class OsCSemLinux : public OsCSemBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   OsCSemLinux(const int queueOptions, const int maxCount);
     //:Constructor setting the initial and max semaphore values to maxCount

   OsCSemLinux(const int queueOptions, const int maxCount, const int initCount);
     //:Constructor allowing different initial and maximum semaphore values

   virtual
   ~OsCSemLinux();
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

/* ============================ INQUIRY =================================== */

   /// Get the current count and maximum count values for this semaphore.
   virtual void getCountMax(int& count, int& max);

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */

private:
   pt_sem_t mSemImp;  // Pingtel-Linux counting semaphore


   OsCSemLinux();
     //:Default constructor (not implemented for this class)

   OsCSemLinux(const OsCSemLinux& rOsCSemLinux);
     //:Copy constructor (not implemented for this class)

   OsCSemLinux& operator=(const OsCSemLinux& rhs);
     //:Assignment operator (not implemented for this class)

   void init(void);
     //:Common initialization shared by all (non-copy) constructors


};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsCSemLinux_h_
