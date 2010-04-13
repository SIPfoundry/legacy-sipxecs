//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsRWMutex_h_
#define _OsRWMutex_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsBSem.h"
#include "os/OsCSem.h"
#include "utl/UtlDefs.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Mutual exclusion semaphore handling multiple readers and writers
// Two kinds of concurrent tasks, called "readers" and "writers", share a
// single resource. The readers can use the resource simultaneously, but each
// writer must have exclusive access to it. When a writer is ready to use the
// resource, it should be enabled to do so as soon as possible.
class OsRWMutex
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum QueueOptions
   {
      Q_FIFO     = 0x0, // queue blocked tasks on a first-in, first-out basis
      Q_PRIORITY = 0x1  // queue blocked tasks based on their priority
   };
     //!enumcode: Q_FIFO - queues blocked tasks on a first-in, first-out basis
     //!enumcode: Q_PRIORITY - queues blocked tasks based on their priority

/* ============================ CREATORS ================================== */

   OsRWMutex(const int queueOptions);
     //:Constructor

   virtual
   ~OsRWMutex();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   virtual OsStatus acquireRead(void);
     //:Block (if necessary) until the task acquires the resource for reading
     // Multiple simultaneous readers are allowed.

   virtual OsStatus acquireWrite(void);
     //:Block (if necessary) until the task acquires the resource for writing
     // Only one writer at a time is allowed (and no readers).

   virtual OsStatus tryAcquireRead(void);
     //:Conditionally acquire the resource for reading (i.e., don't block)
     // Multiple simultaneous readers are allowed.
     // Return OS_BUSY if the resource is held for writing by some other task

   virtual OsStatus tryAcquireWrite(void);
     //:Conditionally acquire the resource for writing (i.e., don't block).
     // Only one writer at a time is allowed (and no readers).
     // Return OS_BUSY if the resource is held for writing by some other task
     // or if there are running readers.

   virtual OsStatus releaseRead(void);
     //:Release the resource for reading

   virtual OsStatus releaseWrite(void);
     //:Release the resource for writing

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   OsBSem mGuard;             // binary semaphore used to implement the
                              //  critical section for protecting the RWMutex
                                   //  structure from concurrent access
        OsCSem mReadSem;           // semaphore used to signal readers
        OsCSem mWriteSem;          // semaphore used to signal writers
   OsBSem mWriteExclSem;      // binary semaphore used to ensure mutual
                              //  exclusion among multiple concurrent writers
        int    mActiveReadersCnt;  // number of active reader tasks
        int    mActiveWritersCnt;  // number of active writer tasks (always <= 1)
        int    mRunningReadersCnt; // number of running reader tasks
        int    mRunningWritersCnt; // number of running writer tasks (always <= 1)

   OsStatus doAcquireRead(UtlBoolean dontBlock);
     //:Helper function used to acquire the resource for reading

   OsStatus doAcquireWrite(UtlBoolean dontBlock);
     //:Helper function used to acquire the resource for writing

   OsStatus doAcquireExclWrite(UtlBoolean dontBlock);
     //:Helper function used to acquire a semaphore for exclusive writing.
     // A binary semaphore is used to ensure that there is only a single
     // writer operating on the resource at any one time.

   OsStatus doReleaseRead(void);
     //:Helper function allowing a reader to give up access to the resource.
     // The mGuard object must be acquired (and ultimately be released) by
     // callers of this method.

   OsStatus doReleaseNonExclWrite(UtlBoolean guardIsHeld);
     //:Helper function to release non-exclusive write access to the resource
     // The mGuard object must be acquired (and ultimately be released) by
     // callers of this method.

   OsStatus doReleaseExclWrite(void);
     //:Helper function to release exclusive write access to the resource

   void grantReadTickets(void);
     // Determine whether the resource can be granted immediately to readers
     // and, if so, grant sufficient read "tickets" for all active but not
     // yet running readers.

   void grantWriteTickets(void);
     // Determine whether the resource can be granted immediately to writers
     // and, if so, grant sufficient write "tickets" for all active but not
     // yet running writers.

   OsRWMutex();
     //:Default constructor (not implemented for this class)

   OsRWMutex(const OsRWMutex& rOsRWMutex);
     //:Copy constructor (not implemented for this class)

           OsRWMutex& operator=(const OsRWMutex& rhs);
     //:Assignment operator (not implemented for this class)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsRWMutex_h_
