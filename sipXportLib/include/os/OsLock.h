//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsLock_h_
#define _OsLock_h_

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "OsSyncBase.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Lock class for mutual exclusion in a critical section
// This class uses OsBSem (binary semaphore) objects for synchronization.
// The constructor for the class automatically blocks until the designated
// semaphore is acquired. Similarly, the destructor automatically releases
// the lock. The easiest way to use this object as a guard for a critical
// section is to create the object as a variable on the stack just before
// the critical section. When the variable goes out of scope, the lock will
// be automatically released. An example of this form of use is shown below.
// <p>
// <font face="courier">
// &nbsp;&nbsp;                      ...                             <br>
// &nbsp;&nbsp;                      {                               <br>
// &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;      OsLock lock(myBSemaphore);    <br>
//                                                                   <br>
// &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;      < critical section >          <br>
// &nbsp;&nbsp;                      }                               <br>
// &nbsp;&nbsp;                      ...                             </font>
// <p>
// The semaphore can be released before the OsLock is destroyed by calling
// OsLock::release().

class OsLock
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   OsLock(OsSyncBase& rSemaphore)
   : mrSemaphore(rSemaphore)
   , mAcquired(true)
   {
      rSemaphore.acquire();
   }
     //:Constructor

   virtual
   ~OsLock()
   {
      if (mAcquired)
      {
         mrSemaphore.release();
      }
   }
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   /// Release the semaphore before the OsLock is destgroyed.
   void release()
   {
      assert(mAcquired);
      mAcquired = false;
      mrSemaphore.release();
   }

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   OsSyncBase& mrSemaphore;

   /// true if the semaphore has been acquired but not released.
   //  Normally true during the lifetime of the OsLock, but can
   //  be set false by ::release().
   bool mAcquired;

   OsLock();
     //:Default constructor (not implemented for this class)

   OsLock(const OsLock& rOsLock);
     //:Copy constructor (not implemented for this class)

   OsLock& operator=(const OsLock& rhs);
     //:Assignment operator (not implemented for this class)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsLock_h_
