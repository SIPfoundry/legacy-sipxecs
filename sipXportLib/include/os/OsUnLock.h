//
// Copyright (C) 2009 Nortel Networks, certain elements licensed under a Contributor Agreement.  
//
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
// &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;      OsLock lock(myBSemaphore);    <br>
//                                                                   <br>
// &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;      < critical section >          <br>
//                                                                   <br>
// &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;      {                               <br>
// &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;         OsUnLock unLock(myBSemaphore);    <br>
//                                                                      <br>
// &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;         < not in critical section >          <br>
// &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;      }                               <br>
//                                                                   <br>
// &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;      < critical section >          <br>
// &nbsp;&nbsp;                      }                               <br>
// &nbsp;&nbsp;                      ...                             </font>
// <p>
// Necessarily, ~OsUnLock() may block.
// Note that as regards locking, there are two critical sections, and 
// the entries into each of them (the construction of 'lock' and the
// destruction of 'unLock') must both be considered in regard to deadlocks,
// etc.  
// In particular, if there are several OsLock's to be released, the
// OsUnLock's should be in the *reverse* order, so that the re-seizures
// that happen when the OsUnLock's are destroyed happen in the same
// order as the original seizures.

class OsUnLock
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   OsUnLock(OsSyncBase& rSemaphore)
   : mrSemaphore(rSemaphore) { rSemaphore.release(); };
     //:Constructor

   virtual
   ~OsUnLock()  { mrSemaphore.acquire(); };
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
