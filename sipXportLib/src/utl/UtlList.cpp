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
#include "utl/UtlListIterator.h"
#include "utl/UtlList.h"
#include "os/OsLock.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const UtlContainableType UtlList::TYPE = "UtlList";

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
UtlList::UtlList()
{
}


// Destructor
UtlList::~UtlList()
{
   UtlContainer::acquireIteratorConnectionLock();

   OsLock take(mContainerLock);

   LIST_SANITY_CHECK;

   invalidateIterators();

   UtlContainer::releaseIteratorConnectionLock();

   UtlLink* node;
   while((node = head()))
   {
      removeLink(node);
   }
}

/* ============================ MANIPULATORS ============================== */


// Removes and returns the first item in the list (pop).
UtlContainable* UtlList::get()
{
   OsLock take(mContainerLock);

   LIST_SANITY_CHECK;
   UtlContainable* firstElement = NULL;

   UtlLink* firstNode = head();

   if(firstNode)
   {
      firstElement = (UtlContainable*) firstNode->data;
      removeLink(firstNode);
   }

   return(firstElement);
}



// Removed the designated object by reference.
UtlContainable* UtlList::removeReference(const UtlContainable* containableToMatch)
{
   UtlContainable* foundElement = NULL;

   UtlLink* foundNode = NULL;
   UtlLink* listNode;

   OsLock take(mContainerLock);

   LIST_SANITY_CHECK;

   for(listNode = head(); listNode && !foundElement; listNode = listNode->next())
   {
      if((UtlContainable*) listNode->data == containableToMatch)
      {
         foundNode = listNode;
         foundElement = (UtlContainable*) listNode->data;
      }
   }

   if (foundNode)
   {
      removeLink(foundNode);
   }

   return(foundElement);
}


void UtlList::removeLink(UtlLink* toBeRemoved)
{
   // The caller already holds the mContainerLock.

   UtlLink*         listNode = NULL;
   UtlListIterator* eachIterator;

   for (listNode = mIteratorList.head(); listNode; listNode = listNode->next())
   {
      eachIterator = (UtlListIterator*)listNode->data;
      eachIterator->removing(toBeRemoved);
   }

   toBeRemoved->detachFrom(this);
}


// Removes and frees the designated objects.
UtlBoolean UtlList::destroy(const UtlContainable* obj)
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


// Removes and delete all elements
void UtlList::destroyAll()
{
   UtlLink* node;

   OsLock take(mContainerLock);

   LIST_SANITY_CHECK;
   while((node = head()))
   {
      UtlContainable* theObject = (UtlContainable*)node->data;
      removeLink(node);
      if(theObject)
      {
         delete theObject;
      }
   }
}


// Remove the object at position index
UtlContainable* UtlList::removeAt(const size_t N)
{
   UtlContainable* removed = NULL;

   OsLock take(mContainerLock);

   UtlLink* link;
   size_t n;
   for (n = 0, link = head(); link && n < N; link = link->next(), n++)
   {
   }
   if (link)
   {
      removed = (UtlContainable*)link->data;
      removeLink(link);
   }

   return removed;
}


// Remove all elements, but do not free the objects
void UtlList::removeAll()
{
   UtlLink* node;

   OsLock take(mContainerLock);

   LIST_SANITY_CHECK;
   while((node = head()))
   {
      removeLink(node);
   }
}

/* ============================ ACCESSORS ================================= */

// Return the first element (head) of the list.
UtlContainable* UtlList::first() const
{
   OsLock take(mContainerLock);

   UtlLink* firstNode = head();

   return firstNode ? (UtlContainable*) firstNode->data : NULL;
}


// Return the last element (tail) of the list.
UtlContainable* UtlList::last() const
{
   OsLock take(mContainerLock);

   UtlLink* lastNode = tail();

   return lastNode ? (UtlContainable*) lastNode->data : NULL;
}


// Return the element at position N.
UtlContainable* UtlList::at(size_t N) const
{
   OsLock take(mContainerLock);

   size_t n;
   UtlLink* link;
   for (n = 0, link = head(); link && n < N; link = link->next(), n++)
   {
   }
   return link ? (UtlContainable*)link->data : NULL;
}

/* ============================ INQUIRY =================================== */

// Return the total number.
size_t UtlList::entries() const
{
   OsLock take(mContainerLock);

   size_t count;
   UtlLink* node;
   for (count = 0, node = head(); node; count++, node=node->next())
   {
   }
   return count;
}


// Return true of the container is empty.
UtlBoolean UtlList::isEmpty() const
{
   return !head();
}


// Return true if the container includes the designated object.
UtlBoolean UtlList::contains(const UtlContainable* object) const
{
   return(find(object) != NULL);
}


// Return true if the list contains the designated object reference .
UtlBoolean UtlList::containsReference(const UtlContainable* containableToMatch) const
{
   UtlLink* listNode;
   UtlBoolean isMatch = FALSE;

   OsLock take(mContainerLock);

   for(listNode = head(); listNode && !isMatch; listNode = listNode->next())
   {
      if((UtlContainable*)listNode->data == containableToMatch)
      {
         isMatch = TRUE;
      }
   }

   return isMatch;
}

/**
 * Get the ContainableType for the list as a contained object.
 */
UtlContainableType UtlList::getContainableType() const
{
   return UtlList::TYPE;
}

#ifdef GLIST_SANITY_TEST
bool UtlList::sanityCheck() const
{
   UtlLink* thisNode;
   UtlLink* prevNode;

   // The caller already holds the mContainerLock.

   for ( ( prevNode=NULL, thisNode=head() );
         thisNode;
         ( prevNode=thisNode, thisNode=thisNode->next() )
        )
   {
      if (thisNode->prev() != prevNode)
      {
         return FALSE;
      }
   }
   return TRUE;
}
#endif /* GLIST_SANITY_TEST */


/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
