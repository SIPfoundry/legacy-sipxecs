//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _OsCSem_h_
#define _OsCSem_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsSyncBase.h"
#include "os/OsBSem.h"

// DEFINES

//Uncomment next line to see semaphore errors when they occur
//#define OS_CSEM_DEBUG

//Uncomment next line (as well as above line) to see all acquires and releases.
//#define OS_CSEM_DEBUG_SHOWALL

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Counting semaphore
class OsCSemBase : public OsSyncBase
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

/* ============================ MANIPULATORS ============================== */

   virtual OsStatus acquire(const OsTime& rTimeout = OsTime::OS_INFINITY) = 0;
     //:Block the task until the semaphore is acquired or the timeout expires

   virtual OsStatus tryAcquire(void) = 0;
     //:Conditionally acquire the semaphore (i.e., don't block)
     // Return OS_BUSY if the semaphore is held by some other task.

   virtual OsStatus release(void) = 0;
     //:Release the semaphore


/* ============================ ACCESSORS ================================= */

#ifdef OS_CSEM_DEBUG
   virtual void show(void) ;
     //:Print statistics gathered

   virtual int getValue(void) ;
     //:Returns the current value of the semaphone
#endif

/* ============================ INQUIRY =================================== */

   /// Get the current count and maximum count values for this semaphore.
   virtual void getCountMax(int& count, int& max) = 0;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

#ifdef OS_CSEM_DEBUG
   OsBSem       mGuard;         // Guard to protect the statistics
                                // used for debugging
   int          mQueueOptions;
   int          mInitialCount;
   int          mMaxCount;
   int          mCurCount;
   int          mHighCount;
   int          mLowCount;
   int          mNumAcquires;
   int          mNumReleases;
#endif

   OsCSemBase(const int queueOptions, const int maxCount,
                     const int initCount) ;
     //:Default constructor

   virtual ~OsCSemBase()  {  };
     //:Destructor

#ifdef OS_CSEM_DEBUG
   void updateAcquireStats(void);
     //:Update the statistics associated with acquiring a counting semaphore

   void updateReleaseStats(void);
     //:Update the statistics associated with releasing a counting semaphore
#endif

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:


   OsCSemBase(const OsCSemBase& rOsCSemBase);
     //:Copy constructor (not implemented for this class)

   OsCSemBase& operator=(const OsCSemBase& rhs);
     //:Assignment operator (not implemented for this class)

};

/* ============================ INLINE METHODS ============================ */

// Depending on the native OS that we are running on, we include the class
// declaration for the appropriate lower level implementation and use a
// "typedef" statement to associate the OS-independent class name (OsCSem)
// with the OS-dependent realization of that type (e.g., OsCSemWnt).
#if defined(_WIN32)
#  include "os/Wnt/OsCSemWnt.h"
   typedef class OsCSemWnt OsCSem;
#elif defined(_VXWORKS)
#  include "os/Vxw/OsCSemVxw.h"
   typedef class OsCSemVxw OsCSem;
#elif defined(__pingtel_on_posix__)
#  include "os/linux/OsCSemLinux.h"
   typedef class OsCSemLinux OsCSem;
#else
#  error Unsupported target platform.
#endif

#endif  // _OsCSem_h_
