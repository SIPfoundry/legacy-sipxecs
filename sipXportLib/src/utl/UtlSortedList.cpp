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

// APPLICATION INCLUDES
#include "utl/UtlContainable.h"
#include "utl/UtlSortedList.h"
#include "os/OsLock.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const UtlContainableType UtlSortedList::TYPE = "UtlSortedList";

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
UtlSortedList::UtlSortedList()
   : UtlList()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator


// Sorted Insert according to the object comparison function
UtlContainable* UtlSortedList::insert(UtlContainable* obj)
{
   OsLock take(mContainerLock);

   UtlLink::listBefore(this, findNode(head(), POSITION, obj), obj);
   return obj;
}

// Remove the designated object by equality
UtlContainable* UtlSortedList::remove(const UtlContainable* obj)
{
   UtlLink*      listNode;
   UtlContainable*  removed = NULL;

   OsLock take(mContainerLock);

   listNode = findNode(head(), EXACTLY, obj);
   if (listNode)
   {
      removed = (UtlContainable*)listNode->data;
      removeLink(listNode);
   }

   return removed;
}

/* ============================ ACCESSORS ================================= */

// Find the first occurence of the designated object by equality.
UtlContainable* UtlSortedList::find(const UtlContainable* obj) const
{
   UtlLink*          listNode;
   UtlContainable* matchNode = NULL;

   OsLock take(mContainerLock);

   listNode = findNode(head(), EXACTLY, obj);
   if (listNode != NULL)
   {
      matchNode = (UtlContainable*)listNode->data;
   }

   return matchNode;
}


/* ============================ INQUIRY =================================== */

// Return the list position of the designated object.
ssize_t UtlSortedList::index(const UtlContainable* obj) const
{
   ssize_t         index = UTL_NOT_FOUND;
   ssize_t         thisIndex;
   UtlLink*        listNode;
   unsigned        keyHash = obj->hash();

   OsLock take(mContainerLock);

   for (listNode = head(), thisIndex = 0;
        listNode && index == UTL_NOT_FOUND;
        listNode = listNode->next(), thisIndex++)
   {
      if (   listNode->data                      // there is an object (for safety sake)
          && listNode->hash == keyHash           // quick test for possible equality
          && listNode->data->compareTo(obj) == 0 // real (but slower) test for equality
          )
      {
         index = thisIndex;
      }
   }

   return index;
}

// Return the number of occurrences of the designated object.
size_t UtlSortedList::occurrencesOf(const UtlContainable* containableToMatch) const
{
   int count = 0;
   UtlLink* listNode;
   UtlContainable* visitNode = NULL;
   int             comparison;

   OsLock take(mContainerLock);

   for (listNode = head(), comparison = 0;
        comparison <= 0 && listNode;
        listNode = listNode->next()
        )
   {
      visitNode = (UtlContainable*)listNode->data;
      if(visitNode && visitNode->compareTo(containableToMatch) == 0)
      {
         count++;
      }
   }

   return(count);
}

/**
 * Get the ContainableType for the list as a contained object.
 */
UtlContainableType UtlSortedList::getContainableType() const
{
   return UtlSortedList::TYPE;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

// Return the first UtlLink node to find.
UtlLink* UtlSortedList::findNode(UtlLink* start, MatchType match, const UtlContainable* obj) const
{
   UtlLink*        listNode;
   UtlLink*        foundNode;
   UtlContainable* listElement;
   int             comparison = 0;

   // the caller already holds the mContainerLock

   for (listNode = start, foundNode = NULL;
        !foundNode && listNode;
        listNode = listNode->next()
        )
   {
      listElement = (UtlContainable*)listNode->data;
      if (listElement)
      {
         // we can't use the hash as a shortcut here because we need the order too
         comparison = listElement->compareTo(obj);
         if ( comparison >= 0 )
         {
            foundNode = listNode;
         }
      }
   }

   if (foundNode && match == EXACTLY && comparison != 0) // match not exact
   {
      foundNode = NULL;
   }
   return foundNode;
}
