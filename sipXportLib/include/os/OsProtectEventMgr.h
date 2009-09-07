//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsProtectEventMgr_h_
#define _OsProtectEventMgr_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsBSem.h"
#include "os/OsProtectEvent.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS


class OsProtectEventMgr
{

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   static OsProtectEventMgr* getEventMgr(void* userData = 0);
     //:Return a pointer to the OsProtectEventMgr, creating it if necessary

   virtual
   ~OsProtectEventMgr();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   OsProtectedEvent* alloc(void* userData = 0);
     //:Allocate a new OsProtectEvent
     // Return OS_SUCCESS if allocation is successful

   OsStatus release(OsProtectedEvent* pEvent);
     //:Release the OsProtectEvent
     // Return OS_SUCCESS if the OsProtectEvent is released.

/* ============================ ACCESSORS ================================= */

   int allocCnt();
     //:Return the number of events that are allocated.

   int availCnt();
     //:Return the number of events that are available.

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

#if defined(_pingtel_on_posix_)
        OsProtectEventMgr(void* userData = 0, int initialCount = 1000,
                                        int softLimit = 2500,
                                        int hardLimit = 25000,
                                        int increment = 500);
#else
        OsProtectEventMgr(void* userData = 0, int initialCount = 200,
                                        int softLimit = 500,
                                        int hardLimit = 5000,
                                        int increment = 50);
#endif
     //:Constructor

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   // Static data members used to enforce Singleton behavior
   static OsProtectEventMgr*    spInstance;             // pointer to the single instance of
                                                                                                                                //  the OsProtectEventMgr class
   static OsBSem     sLock;         // semaphore used to ensure that there
                                    //  is only one instance of this class

   OsBSem    mListSem;   // semaphore used to protect the list table
   int       mAllocs;    // number of allocations
   int       mFrees;    // number of freed events

   int      mInitialCount;   // initial number of items
   int      mCurrentCount;   // current number of items
   int      mSoftLimit;      // soft limit, warn when expanding above this
   int      mHardLimit;      // hard limit AND length of mpElts
   int      mIncrement;      // number to create when expanding
   int      mNext;           // index to next element to examine

   OsProtectedEvent**  mpEvents;          // array of pointers to contained objects

   OsProtectEventMgr(const OsProtectEventMgr& rOsProtectEventMgr);
     //:Copy constructor (not implemented for this class)

   OsProtectEventMgr& operator=(const OsProtectEventMgr& rhs);
     //:Assignment operator (not implemented for this class)

        OsStatus addtoList(unsigned int key, unsigned int pEvent);

        OsStatus remove(unsigned int key);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsProtectEventMgr_h_
