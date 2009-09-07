//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _UtlRscStore_h_
#define _UtlRscStore_h_

#include "utl/UtlRscTrace.h"

#ifdef RSC_TEST


// SYSTEM INCLUDES
#include "os/OsDefs.h"
#include "os/OsBSem.h"
#include "os/OsRWMutex.h"
#include "os/OsStatus.h"

// APPLICATION INCLUDES
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class UtlRscTrace;

//:Database of active Rscs.
// The UtlRscStore maintains a database of active Rscs (i.e., those Rscs
// that have been started by the low level OsSysRsc class).  Since the
// OsRscTask is the only task that should be accessing the Rsc database
// there is no need to serialize access (and no locking).<br>
// <br>
// Each entry in the database is a key/value pair where the key corresponds
// to a Rsc ID and the value is the pointer to the corresponding OsRsc
// object.  Duplicate keys are not allowed.
class UtlRscStore
{
friend UtlRscTrace;

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum StoreInitSize { RSC_STORE_INIT_SIZE = 1000000 };

/* ============================ CREATORS ================================== */

   UtlRscStore(int initialStoreSize = RSC_STORE_INIT_SIZE);
     //:Default constructor

   virtual
   ~UtlRscStore();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   OsStatus insert(int RscId, char* pRsc);
     //:Insert the indicated Rsc into the database of active Rscs.
     // Return OS_SUCCESS if successful, OS_NAME_IN_USE if the key is
     // already in the database.

   OsStatus remove(int RscId);
     //:Remove the indicated Rsc from the database of active Rscs.
     // Return OS_SUCCESS if the indicated RscId is found, return
     // OS_NOT_FOUND if there is no match for the specified key.

   void cleanUp();

/* ============================ ACCESSORS ================================= */

   int getActiveRscs(char* activeRscs[], int size);
     //:Get an array of pointers to the Rscs that are currently active.
     // The caller provides an array that can hold up to <i>size</i> OsRsc
     // pointers. This method will fill in the <i>activeRscs</i> array with
     // up to <i>size</i> pointers. The method returns the number of pointers
     // in the array that were actually filled in.

   void getStoreStats(unsigned& nInserts, unsigned& nRemoves) const;
     //:Get the number of insertions and removals for the Rsc database.

   int numEntries(void) const;
     //:Return the number of key-value pairs in the name database.

/* ============================ INQUIRY =================================== */

   UtlBoolean isEmpty(void) const;
     //:Return TRUE if the Rsc database is empty.

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   UtlHashMap mDict;          // hash table used to store the key/value
                                    //  pairs
   unsigned long mNumInserts;            // number of insertions into the database
   unsigned long mNumRemoves;            // number of removals from the database

   OsRWMutex mDictRWLock;
   UtlRscStore(const UtlRscStore& rUtlRscStore);
     //:Copy constructor (not implemented for this class)

   UtlRscStore& operator=(const UtlRscStore& rhs);
     //:Assignment operator (not implemented for this class)

#ifdef TEST
   static bool sIsTested;
     //:Set to true after the tests for this class have been executed once

   void test();
     //:Verify assertions for this class

   // Test helper functions
   void testCreators();
   void testManipulators();
   void testAccessors();
   void testInquiry();

#endif TEST
};

/* ============================ INLINE METHODS ============================ */
#endif // RSC_TEST

#endif  // _UtlRscStore_h_
