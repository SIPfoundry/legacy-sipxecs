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
#include "os/OsDefs.h"
#include <os/OsLockingList.h>
#include <os/OsLock.h>
#include "utl/UtlVoidPtr.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsLockingList::OsLockingList() :
   listMutex(OsMutex::Q_FIFO),
   iteratorLockCount(0),
   list(),
   listIterator(NULL),
   currentElement(NULL)
{
}

// Copy constructor
OsLockingList::OsLockingList(const OsLockingList& rOsLockingList) :
listMutex(OsMutex::Q_FIFO)
{
}

// Destructor
OsLockingList::~OsLockingList()
{
   // :TODO: shouldn't this be taking the lock?
        if(listIterator)
        {
                delete listIterator;
                listIterator = NULL;
        }

        list.destroyAll();
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
OsLockingList&
OsLockingList::operator=(const OsLockingList& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

void OsLockingList::push(void* element)
{
        assert(element);
        UtlVoidPtr* elementContainer = new UtlVoidPtr(element);

        // Lock before accessing the list
        OsLock localLock(listMutex);
        list.append(elementContainer);
}

void* OsLockingList::pop()
{
        void* element = NULL;

        // Lock before accessing the list
        OsLock localLock(listMutex);

        if (list.entries())
        {
            UtlVoidPtr* elementContainer = dynamic_cast<UtlVoidPtr*>(list.last());
            list.removeReference(elementContainer);
            element = (void*) elementContainer->getValue();
            delete elementContainer;
        }

        return(element);
}

int OsLockingList::getIteratorHandle()
{
        // Lock for the iterator
        listMutex.acquire();

        // Only one iterator is allowed at a time
        assert(listIterator == NULL);
        listIterator = new UtlDListIterator(list);
        iteratorLockCount++;
   currentElement=NULL;
        return(iteratorLockCount);
}

void OsLockingList::releaseIteratorHandle(int iteratorHandle)
{
        assertIterator(iteratorHandle);
        delete listIterator;
        listIterator = NULL;
   currentElement=NULL;
        // Release for the iterator
        listMutex.release();
}

void OsLockingList::resetIterator(int iteratorHandle)
{
        assertIterator(iteratorHandle);
        listIterator->reset();
   currentElement=NULL;
}

void* OsLockingList::next(int iteratorHandle)
{
        void* element = NULL;

        assertIterator(iteratorHandle);
        currentElement = (UtlVoidPtr*)((*listIterator)());
        if(currentElement)
        {
                element = (void*) currentElement->getValue();
        }
        return(element);
}


void* OsLockingList::remove(int iteratorHandle)
{
        void* element = NULL;

        assertIterator(iteratorHandle);
   if (currentElement)
   {
      UtlVoidPtr* elementContainer = (UtlVoidPtr*)list.removeReference(currentElement);
      if(elementContainer)
      {
         element = (void*) elementContainer->getValue();
         delete elementContainer;
         currentElement=NULL;
      }
        }
        return(element);
}


/* ============================ ACCESSORS ================================= */
int OsLockingList::getCount()
{
        OsLock localLock(listMutex);
        return(list.entries());
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
void OsLockingList::assertIterator(int iteratorHandle)
{
        // Only one iterator is allowed at a time
        assert(iteratorHandle == iteratorLockCount);
        assert(listIterator);
}

/* ============================ FUNCTIONS ================================= */
