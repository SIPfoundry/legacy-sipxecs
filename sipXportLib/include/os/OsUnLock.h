//
// Copyright (C) 2010 Avaya Inc., certain elements licensed under a Contributor Agreement.
// Copyright (C) 2009 Nortel Networks, certain elements licensed under a Contributor Agreement.
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _OsUnLock_h_
#define _OsUnLock_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "OsLock.h"
#include "OsSyncBase.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Scoped device for temporarily releasing a lock in a critical section
// Typical use is:
// <p>
// <font face="courier">
// &nbsp;&nbsp;                      ...                             <br>
// &nbsp;&nbsp;                      {                               <br>
// &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;      OsLockUnlockable lock(myBSemaphore); <br>
//                                                                   <br>
// &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;      < critical section >          <br>
//                                                                   <br>
// &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;      {                               <br>
// &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;         OsUnLock unLock(lock);    <br>
//                                                                      <br>
// &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;         < not in critical section >          <br>
// &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;      }                               <br>
//                                                                   <br>
// &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;      < critical section >          <br>
// &nbsp;&nbsp;                      }                               <br>
// &nbsp;&nbsp;                      ...                             </font>
// <p>
// OsLockUnlockable behaves like OsLock, except that it can be the
// subject of an OsUnlock.  Thus, its critical section is not
// necessarily coextensive with its scope.
// <p>
// Note that the argument to OsUnLock::OsUnLock is the
// OsLockUnlockable, not the semaphore.  This is to avoid problems
// when the argument to OsLock is a complex expression, which might
// not yield the same value when reexecuted.
// The OsLockUnlockable should have a scope which statically encloses the scope
// of the OsUnLock.
// After the OsUnLock is created, it is good practice to clear the
// value of any variable whose value is invalidated by the fact that
// the semaphore has been released.
// <p>
// Necessarily, ~OsUnLock() may block.
// Note that as regards locking, there are two critical sections, and
// the entries into each of them (the construction of 'lock' and the
// destruction of 'unLock') must both be considered in regard to deadlocks,
// etc.
// In particular, if there are several OsLockUnlockable's to be released, the
// OsUnLock's should be in the *reverse* order, so that the re-seizures
// that happen when the OsUnLock's are destroyed happen in the same
// order as the original seizures.

class OsLockUnlockable : public OsLock
{
   friend class OsUnLock;

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   OsLockUnlockable(OsSyncBase& rSemaphore)
   : OsLock(rSemaphore)
   {
   }
     //:Constructor

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   OsLockUnlockable();
     //:Default constructor (not implemented for this class)

   OsLockUnlockable(const OsLockUnlockable& rOsLock);
     //:Copy constructor (not implemented for this class)

   OsLockUnlockable& operator=(const OsLockUnlockable& rhs);
     //:Assignment operator (not implemented for this class)

   OsSyncBase& getSemaphore()
   {
      return mrSemaphore;
   }
     //:Get the semaphore.
};


class OsUnLock
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   OsUnLock(OsLockUnlockable& lock)
      : mrSemaphore(lock.getSemaphore())
   {
      mrSemaphore.release();
   }
     //:Constructor
   // lock's scope should statically enclose the scope of this OsUnLock.

   virtual
   ~OsUnLock()
   {
      mrSemaphore.acquire();
   }
     //:Destructor

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   OsSyncBase& mrSemaphore;

   OsUnLock();
     //:Default constructor (not implemented for this class)

   OsUnLock(const OsUnLock& rOsUnLock);
     //:Copy constructor (not implemented for this class)

   OsUnLock& operator=(const OsUnLock& rhs);
     //:Assignment operator (not implemented for this class)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsUnLock_h_
