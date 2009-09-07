//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "os/OsNameDb.h"
#include "os/OsReadLock.h"
#include "os/OsWriteLock.h"
#include "utl/UtlVoidPtr.h"
#include "utl/UtlString.h"

// EXTERNAL FUNCTIONS

// EXTERNAL VARIABLES

// CONSTANTS
static const int DEFAULT_NAMEDB_SIZE = 100;

// STATIC VARIABLE INITIALIZATIONS
OsNameDb* OsNameDb::spInstance = NULL;
OsBSem*   OsNameDb::spLock = new OsBSem(OsBSem::Q_PRIORITY, OsBSem::FULL);

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Return a pointer to the singleton object, creating it if necessary
OsNameDb* OsNameDb::getNameDb(void)
{
   spLock->acquire();
   if (spInstance == NULL) // not created while getting lock?
   {
      spInstance = new OsNameDb();
   }
   spLock->release();

   return spInstance;
}

// Destructor
// Since the name database is a singleton object, this destructor should
// not get called unless we are shutting down the system.
OsNameDb::~OsNameDb()
{
   spInstance = NULL;
}

/* ============================ MANIPULATORS ============================== */

// Add the key-value pair to the name database
// Return OS_SUCCESS if successful, OS_NAME_IN_USE if the key is already in
// the database.
OsStatus OsNameDb::insert(const UtlString& rKey,
                          void* value)
{
   OsWriteLock          lock(mRWLock);
   UtlString*  pDictKey;
   UtlVoidPtr* pDictValue;
   UtlString*  pInsertedKey;

   pDictKey   = new UtlString(rKey);
   pDictValue = new UtlVoidPtr(value);
   pInsertedKey = (UtlString*)
                  mDict.insertKeyAndValue(pDictKey, pDictValue);

   if (pInsertedKey == NULL)
   {                             // insert failed
      delete pDictKey;           // clean up the key and value objects
      delete pDictValue;

      return OS_NAME_IN_USE;
   }
   else
   {
      return OS_SUCCESS;
   }
}

// Remove the indicated key-value pair from the name database.
// If pValue is non-NULL, the value for the key-value pair is returned
// via pValue.
// Return OS_SUCCESS if the lookup is successful, return OS_NOT_FOUND
// if there is no match for the specified key.
OsStatus OsNameDb::remove(const UtlString& rKey,
                          void** pValue)
{
   OsWriteLock          lock(mRWLock);
   OsStatus   result = OS_NOT_FOUND;
   UtlString* pDictKey;
   UtlContainable* pDictValue;

   pDictKey = dynamic_cast<UtlString*>
      (mDict.removeKeyAndValue(&rKey, pDictValue));

   // If a value was found and removed ...
   if (pDictKey != NULL)
   {
      result =  OS_SUCCESS;

      // If the caller provided a pointer through which to return the
      // integer value, do so.
      if (pValue != NULL)
      {
         UtlVoidPtr* voidPtrValue = dynamic_cast<UtlVoidPtr*>(pDictValue);
         if (voidPtrValue)
         {
            *pValue = voidPtrValue->getValue();
         }
         else
         {
            result = OS_NOT_FOUND;
         }
      }

      // Delete the key and value objects.
      delete pDictKey;
      delete pDictValue;
   }

   //  Return success or failure as appropriate.
   return result;
}

/* ============================ ACCESSORS ================================= */

// Retrieve the value associated with the specified key.
// If pValue is non-NULL, the value is returned via pValue.
// Return OS_SUCCESS if the lookup is successful, return OS_NOT_FOUND if
// there is no match for the specified key.
OsStatus OsNameDb::lookup(const UtlString& rKey,
                          void** pValue)
{
   OsReadLock  lock(mRWLock);
   OsStatus    result = OS_NOT_FOUND;
   UtlVoidPtr* pDictValue;

   pDictValue = (UtlVoidPtr*)
                mDict.findValue(&rKey); // perform the lookup

   if (pDictValue != NULL)
   {
      if (pValue != NULL)       // if we have a valid pointer,
      {                         //  return the corresponding value
         *pValue = pDictValue->getValue();
      }
      result = OS_SUCCESS;
   }

   return result;
}

// Return the number of key-value pairs in the name database
int OsNameDb::numEntries(void)
{
   OsReadLock lock(mRWLock);

   return mDict.entries();
}

/* ============================ INQUIRY =================================== */

// Return TRUE if the name database is empty
UtlBoolean OsNameDb::isEmpty(void)
{
   return (numEntries() == 0);
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

// Constructor (only available internal to the class)
OsNameDb::OsNameDb() :
   mDict(),
   mRWLock(OsRWMutex::Q_PRIORITY)
{
   // since we plan to store object pointers as well as integer values in the
   //  database, we make sure the sizes are compatible
   //TODO: is this test still required now that UtlVoidPtr is used instead of UtlInt
   //for this data?
   assert(sizeof(void*) <= sizeof(intptr_t));

   // no other work required
}

/* ============================ FUNCTIONS ================================= */
