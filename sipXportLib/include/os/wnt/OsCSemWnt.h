//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _OsCSemWnt_h_
#define _OsCSemWnt_h_

// SYSTEM INCLUDES
#include <windows.h>

// APPLICATION INCLUDES
#include "os/OsCSem.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS

// TYPEDEFS
typedef HANDLE WinSemaphore;

// FORWARD DECLARATIONS

//:Counting semaphore for Windows NT
class OsCSemWnt : public OsCSemBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   OsCSemWnt(const int queueOptions, const int maxCount);
     //:Constructor setting the initial and max semaphore values to maxCount

   OsCSemWnt(const int queueOptions, const int maxCount, const int initCount);
     //:Constructor allowing different initial and maximum semaphore values

   virtual
   ~OsCSemWnt();
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
   WinSemaphore mSemImp;  // Windows NT semaphore


   OsCSemWnt();
     //:Default constructor (not implemented for this class)

   OsCSemWnt(const OsCSemWnt& rOsCSemWnt);
     //:Copy constructor (not implemented for this class)

   OsCSemWnt& operator=(const OsCSemWnt& rhs);
     //:Assignment operator (not implemented for this class)

   void init(void);
     //:Common initialization shared by all (non-copy) constructors

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsCSemWnt_h_
