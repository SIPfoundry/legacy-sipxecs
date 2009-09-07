//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsSyncBase_h_
#define _OsSyncBase_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "os/OsTime.h"

// DEFINES
// If OS_SYNC_DEBUG, enable debugging information for binary semaphores and
// mutexes.
//#define OS_SYNC_DEBUG

#ifdef OS_SYNC_DEBUG
#  include "os/OsDateTime.h"
#endif

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS

//:Base class for the synchronization mechanisms in the OS abstraction layer

class OsSyncBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   virtual
      ~OsSyncBase() { };
     //:Destructor

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

   OsSyncBase& operator=(const OsSyncBase& rhs);
     //:Assignment operator

   virtual OsStatus acquire(const OsTime& rTimeout = OsTime::OS_INFINITY) = 0;
     //:Block until the sync object is acquired or the timeout expires

   virtual OsStatus tryAcquire(void) = 0;
     //:Conditionally acquire the semaphore (i.e., don't block)
     // Return OS_BUSY if the sync object is held by some other task.

   virtual OsStatus release(void) = 0;
     //:Release the sync object

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

#ifdef OS_SYNC_DEBUG
/// Operations on an OsSyncBase object
   typedef enum
   {
      crumbUnused,   ///< array slot not yet used
      crumbCreated,  ///< OsSyncBase constructed
      crumbAcquired, ///< OsSyncBase acquired
      crumbReleased, ///< OsSyncBase released
      crumbDeleted   ///< OsSyncBase deleted
   } OsSyncOperation;
#endif

#  ifdef OS_SYNC_DEBUG

   /// Track usage of any OsSyncBase object
   class OsSyncCrumbs
   {
#    define NUMBER_OF_CRUMBS 6

     public:

      /// Constructor to initialize crumb trail
      OsSyncCrumbs() :
         mCrumb(0)
      {
         mTrail[mCrumb].operation = crumbCreated;
         mTrail[mCrumb].taskId = 0; // default; parent constructor should call dropCrumb
         for ( unsigned int crumb=1; crumb < NUMBER_OF_CRUMBS; crumb++ )
         {
            mTrail[crumb].operation = crumbUnused;
            mTrail[crumb].taskId    = 0;
         }
      }

      /// record the task id and operation in the mTrail circular buffer
      void dropCrumb(pthread_t id, OsSyncOperation op)
      {
         mCrumb = (mCrumb + 1) % NUMBER_OF_CRUMBS;
         mTrail[mCrumb].operation = op;
         mTrail[mCrumb].taskId    = id;
         OsDateTime::getCurTime(mTrail[mCrumb].time);
      }

      ~OsSyncCrumbs()
      {
         // better if the destructor in the object calls, but make sure there is something.
         dropCrumb(0, crumbDeleted);
      }

     private:

      unsigned int mCrumb;  ///< circular index into mTrail: most recently used entry
      struct
      {
         OsSyncOperation operation; ///< operation on the syncronizer
         pthread_t       taskId;    ///< the ID of the task that touched the syncronizer
         OsTime          time;      ///< when the operation happened
      } mTrail[NUMBER_OF_CRUMBS];
   } mSyncCrumbs;

#  endif

   OsSyncBase() { };
     //:Default constructor

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   OsSyncBase(const OsSyncBase& rOsSyncBase);
     //:Copy constructor

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsSyncBase_h_
