//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#include "utl/UtlRscTrace.h"

#ifdef RSC_TEST

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "utl/UtlRscStore.h"
#include "os/OsWriteLock.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

// Database of active Rscs.
// The UtlRscStore maintains a database of active Rscs (i.e., those Rscs
// that have been started by the low level OsSysRsc class).  Since the
// OsRscTask is the only task that should be accessing the Rsc database
// there is no need to serialize access (and no locking).
//
// Each entry in the database is a key/value pair where the key corresponds
// to a Rsc ID and the value is the pointer to the corresponding OsRsc
// object.  Duplicate keys are not allowed.

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
UtlRscStore::UtlRscStore(int initialStoreSize)
:  mDict(initialStoreSize), mNumInserts(0), mNumRemoves(0), mDictRWLock(OsRWMutex::Q_PRIORITY)
{
   // since we plan to store OsRsc pointers in the
   //  database, we make sure the sizes are compatible
   assert(sizeof(char*) <= sizeof(int));

   // no other work required
}

// Destructor
UtlRscStore::~UtlRscStore()
{
   cleanUp();
}

/* ============================ MANIPULATORS ============================== */

// Insert the indicated Rsc into the database of active Rscs.
// Return OS_SUCCESS if successful, OS_NAME_IN_USE if the key is
// already in the database.
OsStatus UtlRscStore::insert(int RscId, char* pRsc)
{
   UtlInt* pDictKey;
   UtlString* pDictValue;
   UtlInt* pInsertedKey;
   int removed = 0;
   {
      OsWriteLock lock(mDictRWLock);
      pDictKey   = new UtlInt(RscId);
      pDictValue = new UtlString(pRsc);
      pInsertedKey = (UtlInt*)
                 mDict.insertKeyAndValue(pDictKey, pDictValue);

      if (pInsertedKey == NULL)
      {
         osPrintf("\nUtlRscStore::insert failed: try to remove 0x%08x\n",RscId);
         UtlInt* pLookupKey;
         UtlInt* pRemKey;
         UtlString* pRemValue;

         pLookupKey = new UtlInt(RscId);
         pRemKey   = (UtlInt*)
                  mDict.removeKeyAndValue(pLookupKey,
                                   (UtlContainable*&) pRemValue);
         delete pLookupKey;

         if (pRemKey != NULL)
         {
            osPrintf("\nUtlRscStore::insert failed: 0x%08x removed\n", RscId);
            mNumRemoves++;
            delete pRemKey;          // before returning we need to destroy the
            delete pRemValue;        //  objects that were used to maintain the
            removed = 1;
         }

         delete pDictKey;            // clean up the key and value objects
         delete pDictValue;
      }
      else
      {
        mNumInserts++;
        return OS_SUCCESS;
      }
   }

   if (removed)
      return insert(RscId, pRsc);
   else
   {
      osPrintf("\nUtlRscStore::insert 0x%08x failed: mNumInserts = %d\n",
         RscId, mNumInserts);
                         // insert failed
      return OS_NAME_IN_USE;
   }

}

// Remove the indicated Rsc from the database of active Rscs.
// Return OS_SUCCESS if the indicated RscId is found, return
// OS_NOT_FOUND if there is no match for the specified key.
OsStatus UtlRscStore::remove(int RscId)
{
   UtlInt* pLookupKey;
   UtlInt* pDictKey;
   UtlString* pDictValue;

   OsWriteLock lock(mDictRWLock);
   pLookupKey = new UtlInt(RscId);
   pDictKey   = (UtlInt*)
            mDict.removeKeyAndValue(pLookupKey,
                             (UtlContainable*&) pDictValue);
   delete pLookupKey;

   if (pDictKey == NULL)
   {
      osPrintf("\nUtlRscStore::remove: 0x%08x not found: \n"
         "    mNumInserts = %d mNumRemoves = %d\n",
         RscId, mNumInserts, mNumRemoves);
      return OS_NOT_FOUND;   // did not find the specified key
   }
   else
      mNumRemoves++;

   delete pDictKey;          // before returning we need to destroy the
   delete pDictValue;        //  objects that were used to maintain the
                             //  dictionary entry
   return OS_SUCCESS;
}

void UtlRscStore::cleanUp()
{
   UtlHashMapIterator iter(mDict);
   UtlContainable*    next;
   UtlInt* key;
   UtlInt* value;

   iter.reset();
   while (next = iter())
   {
      key   = (UtlInt*) iter.key();
      value = (UtlInt*) iter.value();
      iter.remove();
      delete key;
      delete value;
      iter.reset();
   }
   mNumInserts = 0;
   mNumRemoves = 0;
}
/* ============================ ACCESSORS ================================= */

// Get an array of pointers to the Rscs that are currently active.
// The caller provides an array that can hold up to "size" OsRsc
// pointers. This method will fill in the "activeRscs" array with
// up to "size" pointers. The method returns the number of pointers
// in the array that were actually filled in.
int UtlRscStore::getActiveRscs(char* activeRscs[], int size)
{
   UtlHashMapIterator iter(mDict);
   UtlContainable*    next;
   UtlInt* value;
   int      i;

   iter.reset();
   i = 0;
   while (next = iter())
   {
      if (i >= size) break;

      value = (UtlInt*) iter.value();
      activeRscs[i] = (char*) value->value();
      i++;
   }

   return i;
}

// Get the number of insertions and removals for the Rsc database.
void UtlRscStore::getStoreStats(unsigned& nInserts, unsigned& nRemoves) const
{
   nInserts = mNumInserts;
   nRemoves = mNumRemoves;
}

// Return the number of key-value pairs in the Rsc database.
int UtlRscStore::numEntries(void) const
{
   return mDict.entries();
}

/* ============================ INQUIRY =================================== */

// Return TRUE if the Rsc database is empty.
UtlBoolean UtlRscStore::isEmpty(void) const
{
   return (numEntries() == 0);
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

#endif // RSC_TEST
