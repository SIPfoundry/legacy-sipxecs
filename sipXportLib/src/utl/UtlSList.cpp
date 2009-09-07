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
#include "utl/UtlSListIterator.h"
#include "utl/UtlSList.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const UtlContainableType UtlSList::TYPE = "UtlSList";

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
UtlSList::UtlSList()
   : UtlList()
{
}


/* ============================ MANIPULATORS ============================== */

// Assignment operator


// Append containable object to the end.
UtlContainable* UtlSList::append(UtlContainable* obj)
{
   OsLock take(mContainerLock);

    if(obj != NULL)
    {
       LIST_SANITY_CHECK;
       UtlLink::listBefore(this, NULL, obj);
       LIST_SANITY_CHECK;
    }

    return(obj);
}


// Insert at the designated position.
UtlContainable* UtlSList::insertAt(size_t N, UtlContainable* obj)
{
   // :NOTE: this method is deliberately not the same as g_list_insert in that
   //        the glib routine will accept a value of N > the length of the list
   //        but this routine treats that as an error.
   UtlContainable* inserted = NULL;

   OsLock take(mContainerLock);

   LIST_SANITY_CHECK;
   size_t n;
   UtlLink* link;
   for (n = 0, link = head(); link && n < N; link = link->next(), n++)
   {
   }
   if (n == N)
   {
      UtlLink::listBefore(this, link, obj);
      inserted = obj;
   }
   LIST_SANITY_CHECK;

   return inserted;
}


UtlContainable* UtlSList::insertAfter(UtlLink* afterNode, UtlContainable* object)
{
   OsLock take(mContainerLock);

   UtlLink::listAfter(this, afterNode, object);
   return object;
}


// Inserts at the end postion (tailer).
UtlContainable* UtlSList::insert(UtlContainable* obj)
{
    return append(obj);
}


// Remove the designated object by equality.
UtlContainable* UtlSList::remove(const UtlContainable* object)
{
   UtlLink* listNode;
   UtlLink* found;
   UtlContainable* foundObject = NULL;

   OsLock take(mContainerLock);

   LIST_SANITY_CHECK;
   for (listNode = head(), found = NULL; listNode && !found; listNode = listNode->next())
   {
      UtlContainable* visitNode = (UtlContainable*) listNode->data;
      if(visitNode && visitNode->compareTo(object) == 0)
      {
         found = listNode;
      }
   }

   if (found)
   {
      foundObject = (UtlContainable*)found->data;
      removeLink(found);
   }
   LIST_SANITY_CHECK;

   return foundObject;
}


// Removes and frees the designated object.
UtlBoolean UtlSList::destroy(const UtlContainable* obj)
{
    UtlBoolean result = FALSE;

    // this does not take the mContainerLock, because all the container state changes
    // are made inside the remove method, which already takes it.

    UtlContainable* removed = remove(obj);
    if (removed)
    {
        result = TRUE;
        delete removed;
    }
    return result;
}


/* ============================ ACCESSORS ================================= */

// Find the first occurrence of the designated object by equality.
UtlContainable* UtlSList::find(const UtlContainable* containableToMatch) const
{
   UtlLink* listNode;
   UtlContainable* matchElement = NULL;
   UtlContainable* visitNode;

   unsigned targetHash = containableToMatch->hash();

   OsLock take(mContainerLock);

   LIST_SANITY_CHECK;
   for(listNode = head()->findNextHash(targetHash);
       listNode &&  matchElement == NULL;
       listNode = listNode->next()->findNextHash(targetHash)
       )
   {
      visitNode = (UtlContainable*) listNode->data;
      if(visitNode && visitNode->compareTo(containableToMatch) == 0)
      {
         matchElement = visitNode;
      }
   }
   LIST_SANITY_CHECK;

   return(matchElement);
}


/* ============================ INQUIRY =================================== */


// Return the number of occurrences of the designated object.
size_t UtlSList::occurrencesOf(const UtlContainable* containableToMatch) const
{
   int count = 0;
   UtlLink* listNode;
   UtlContainable* visitNode = NULL;

   OsLock take(mContainerLock);

   LIST_SANITY_CHECK;
   for(listNode = head(); listNode; listNode = listNode->next())
   {
      visitNode = (UtlContainable*)listNode->data;
      if(visitNode && visitNode->compareTo(containableToMatch) == 0)
      {
         count++;
      }
   }
   LIST_SANITY_CHECK;

   return(count);
}


// Return the list position of the designated object.
ssize_t UtlSList::index(const UtlContainable* containableToMatch) const
{
    ssize_t matchedIndex = UTL_NOT_FOUND;
    ssize_t currentIndex;
    UtlLink* listNode;
    UtlContainable* visitNode = NULL;

    OsLock take(mContainerLock);

    LIST_SANITY_CHECK;
    for(listNode = head(), currentIndex = 0;
        matchedIndex == UTL_NOT_FOUND && listNode;
        listNode = listNode->next()
        )
    {
        visitNode = (UtlContainable*) listNode->data;
        if(visitNode && visitNode->compareTo(containableToMatch) == 0)
        {
           matchedIndex = currentIndex;
        }
        else
        {
           currentIndex++;
        }
    }
    LIST_SANITY_CHECK;

    return matchedIndex;
}


/**
 * Get the ContainableType for the list as a contained object.
 */
UtlContainableType UtlSList::getContainableType() const
{
   return UtlSList::TYPE;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
