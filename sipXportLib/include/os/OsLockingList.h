//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsLockingList_h_
#define _OsLockingList_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsMutex.h>
#include "utl/UtlDList.h"
#include "utl/UtlDListIterator.h"
#include "utl/UtlVoidPtr.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//: A list with an attached lock that can be taken and held.
//
class OsLockingList
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   OsLockingList();
     //:Default constructor

   virtual
   ~OsLockingList();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   int getIteratorHandle();
   //: Get a handle to lock the list and allow safe iteration

   void resetIterator(int iteratorHandle);
   //: Reset the iterator back to the beginning

   void* next(int iteratorHandle);
   //: Get without removing the next element

   void* remove(int iteratorHandle);
   //: Get and remove the pointer at the current element

   void releaseIteratorHandle(int iteratorHandle);
   //: Release the iterator lock so that other methods may be used

   void push(void* element);
   //: Add an element to the end
   // This method blocks while the iterator is outstanding or other
   // methods are in use.  A NULL element is not allowed.

   void* pop();
   //: Get and remove the last element
   // This method blocks while the iterator is outstanding or other
   // methods are in use.

   int getCount();
   //: Get the number of elements in the list
   // Temporarily seizes the list's lock.

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   OsMutex listMutex;
   int iteratorLockCount;
   UtlDList list;
   UtlDListIterator* listIterator;
   UtlVoidPtr* currentElement;

   void assertIterator(int iteratorHandle);

   OsLockingList& operator=(const OsLockingList& rhs);
     //:Assignment operator

   OsLockingList(const OsLockingList& rOsLockingList);
     //:Copy constructor

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsLockingList_h_
