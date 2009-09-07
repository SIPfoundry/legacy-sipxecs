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
#include "utl/UtlHashBagIterator.h"
#include "os/OsLock.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/*
 * ===========================  Design Notes  =====================================
 *
 *   The following tables show the state of the position tracking member variables
 *   for different iterator states.  The states of a keyed iterator and an unkeyed
 *   iterator are shown separately.  (The state following a reset() is the same as
 *   the Initial state)
 *
 * ================ Unkeyed iterator (created without passing a key object)
 *                            mPosition           mpCurrentLink     mLinkIsValid
 * Initial()                      0                  NULL             true
 * on returned item      0..(numberOfBuckets-1)    ->UtlLink->item    true
 * on removed item       0..(numberOfBuckets-1)    ->UtlLink->item    false
 * after last item          numberOfBuckets          NULL             true
 *
 * ================ Keyed iterator (created providing a key object)
 *                            mPosition           mpCurrentLink     mLinkIsValid
 * Initial(key)       bucketNumber(mpSubsetHash)      NULL            true
 * on returned item   bucketNumber(mpSubsetHash)  ->UtlLink->item     true
 * on removed item    bucketNumber(mpSubsetHash)  ->UtlLink->item     false
 * after last item          numberOfBuckets          NULL             true
 *
 */

/* ============================ CREATORS ================================== */

// Constructor
UtlHashBagIterator::UtlHashBagIterator(UtlHashBag& hashBag, UtlContainable* key) :
   UtlIterator(hashBag),
   mpSubsetMatch(key)
{
   OsLock container(hashBag.mContainerLock);
   addToContainer(mpMyContainer);

   init(hashBag);
}


// Destructor
UtlHashBagIterator::~UtlHashBagIterator()
{
   UtlContainer::acquireIteratorConnectionLock();
   OsLock take(mContainerRefLock);
   UtlHashBag* myHashBag = dynamic_cast<UtlHashBag*>(mpMyContainer);
   if (myHashBag)
   {
      OsLock container(myHashBag->mContainerLock);
      UtlContainer::releaseIteratorConnectionLock();

      myHashBag->removeIterator(this);

      // in case the existence of this iterator has been preventing a resize
      myHashBag->resizeIfNeededAndSafe();

      mpMyContainer = NULL;
   }
   else
   {
      UtlContainer::releaseIteratorConnectionLock();
   }
}

/* ============================ MANIPULATORS ============================== */


UtlContainable* UtlHashBagIterator::operator()()
{
   UtlContainable* foundObject = NULL;

   UtlContainer::acquireIteratorConnectionLock();
   OsLock take(mContainerRefLock);
   UtlHashBag* myHashBag = dynamic_cast<UtlHashBag*>(mpMyContainer);
   if (myHashBag)
   {
      OsLock container(myHashBag->mContainerLock);
      UtlContainer::releaseIteratorConnectionLock();

      if (mPosition < myHashBag->numberOfBuckets())
      {
         if (mpSubsetMatch)
         {
            /*
             * This iterator only returns elements matching mpSubsetMatch
             * so mPosition (the bucket index) never changes - all matches
             * are by definition in the same bucket.
             */
            UtlLink* link;
            for ( link = (  mpCurrentLink
                          ? static_cast<UtlLink*>(mpCurrentLink->UtlChain::next)
                          : static_cast<UtlLink*>(myHashBag->mpBucket[mPosition].listHead())
                          );
                  (   !foundObject              // no match found yet
                   && link                      // but we have a link
                   && link->hash <= mSubsetHash /* bucket list is sorted by hash code,
                                                 * so when link->hash > mSubsetHash
                                                 * there will be no more matches */
                   );
                  link = link->next()
                 )
            {
               if (   link->hash == mSubsetHash          // save the call to isEqual
                   && link->data->isEqual(mpSubsetMatch) // the real test of equality
                   )
               {
                  mpCurrentLink = link;
                  foundObject   = link->data;
               }
            }
            if (!foundObject)
            {
               mPosition = myHashBag->numberOfBuckets(); // that's it - no more matches
            }
         }
         else
         {
            // this iterator has no subset to match - it walks all elements in the hash

            for ( mpCurrentLink = (  mpCurrentLink
                                   ? static_cast<UtlLink*>(mpCurrentLink->UtlChain::next)
                                   : static_cast<UtlLink*>(
                                      myHashBag->mpBucket[mPosition].listHead())
                                   );
                  (   !mpCurrentLink // there was a next - this bucket has another item in it
                   && (  ++mPosition // if not, bump the bucket
                       < myHashBag->numberOfBuckets() // have we looked at the last bucket?
                       )
                   );
                  mpCurrentLink = static_cast<UtlLink*>(myHashBag->mpBucket[mPosition].listHead())
                 )
            {
            }

            if(mpCurrentLink)
            {
               foundObject = mpCurrentLink->data;
            }
            else
            {
               // this iterator is done - mPosition walked off the end.
            }
         }
      }
      else
      {
         // mPosition >= myHashMap->numberOfBuckets(), so we've run off the end of the entries.
         mpCurrentLink = NULL;
      }
      // Set mLinkIsValid (since mLinkIsValid is FALSE only when the current
      // item is one we backed up to after removing the previous current item).
      // See the chart at the top of this file.
      mLinkIsValid = TRUE;
   }
   else
   {
      UtlContainer::releaseIteratorConnectionLock();
   }

   return foundObject;
}

void UtlHashBagIterator::reset()
{
   UtlContainer::acquireIteratorConnectionLock();
   OsLock take(mContainerRefLock);
   UtlHashBag* myHashBag = dynamic_cast<UtlHashBag*>(mpMyContainer);
   if (myHashBag)
   {
      OsLock container(myHashBag->mContainerLock);
      UtlContainer::releaseIteratorConnectionLock();

      init(*myHashBag);
   }
   else
   {
      UtlContainer::releaseIteratorConnectionLock();
   }
}

/* ============================ ACCESSORS ================================= */

// Gets the key of the current element
UtlContainable* UtlHashBagIterator::key() const
{
   UtlContainable* current = NULL;

   UtlContainer::acquireIteratorConnectionLock();
   OsLock take(mContainerRefLock);
   UtlHashBag* myHashBag = dynamic_cast<UtlHashBag*>(mpMyContainer);
   if (myHashBag)
   {
      OsLock container(myHashBag->mContainerLock);
      UtlContainer::releaseIteratorConnectionLock();

      if (mLinkIsValid && mpCurrentLink) // current position is defined
      {
         current = mpCurrentLink->data;
      }
   }
   else
   {
      UtlContainer::releaseIteratorConnectionLock();
   }

   return current;
}


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

void UtlHashBagIterator::removing(const UtlLink* link)
{
   // caller is holding the mContainerLock

   if ((link = mpCurrentLink))
   {
      mLinkIsValid  = false;
      mpCurrentLink = mpCurrentLink->prev();
   }
}


/* //////////////////////////// PRIVATE /////////////////////////////////// */

void UtlHashBagIterator::init(UtlHashBag& hashBag)
{
   // caller is holding the mContainerLock

   mpCurrentLink = NULL;
   mLinkIsValid  = true;

   if (mpSubsetMatch)
   {
      mSubsetHash = mpSubsetMatch->hash();
      mPosition = hashBag.bucketNumber(mSubsetHash);
   }
   else
   {
      mPosition = 0;
   }
}

/* ============================ FUNCTIONS ================================= */
