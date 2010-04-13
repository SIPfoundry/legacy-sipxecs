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
#include "utl/UtlHashBag.h"
#include "utl/UtlHashBagIterator.h"
#include "os/OsLock.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const UtlContainableType UtlHashBag::TYPE = "UtlHashBag";

#define HASHBAG_INITIAL_BUCKET_BITS 4
#define NUM_HASHBAG_BUCKETS(bits) (1<<bits)

// STATIC VARIABLE INITIALIZATIONS

/**
 * Design Notes
 *
 * Each entry in the dynamic mBucket array is a list header for the items in that bucket.
 * The bucket number for an item is its hash code, XOR-folded to the mBucketBits
 * The entries in each bucket list are sorted into ascending order, so that the worst case
 * lookup performance (when all items hash to the same bucket) is no worse than a linear
 * search (if the hash functions are any good, it should almost always be better).
 */

/* //////////////////////////// PUBLIC /////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor

UtlHashBag::UtlHashBag() :
   mElements(0),
   mBucketBits(HASHBAG_INITIAL_BUCKET_BITS),
   mpBucket(new UtlChain[NUM_HASHBAG_BUCKETS(HASHBAG_INITIAL_BUCKET_BITS)])
{
}


// Destructor
UtlHashBag::~UtlHashBag()
{
   UtlContainer::acquireIteratorConnectionLock();
   OsLock take(mContainerLock);

   invalidateIterators();

   UtlContainer::releaseIteratorConnectionLock();

   // still holding the mContainerLock
   // walk the buckets
   for (size_t i = 0; i < numberOfBuckets(); i++)
   {
      // empty each bucket and release each UtlPair back to the pool
      while (!mpBucket[i].isUnLinked())
      {
         UtlLink* link = static_cast<UtlLink*>(mpBucket[i].listHead());
         link->detachFromList(&mpBucket[i]);
         link->release();
      }
   }
   delete [] mpBucket;   // free the bucket headers
}

/*
 * Allocate additional buckets and redistribute existing contents.
 * This should only be called through resizeIfNeededAndSafe.
 */
void UtlHashBag::resize()
{
   // already holding the mContainerLock
   UtlChain* newBucket;
   size_t    newBucketBits;

   // if an iterator had prevented resizing while many elements were added,
   // we might need to double more than once to restore the target ratio.
   for (newBucketBits = mBucketBits+1;
        mElements / NUM_HASHBAG_BUCKETS(newBucketBits) >= 3;
        newBucketBits++
        )
   {
   }

   // allocate the new buckets
   newBucket = new UtlChain[NUM_HASHBAG_BUCKETS(newBucketBits)];

   if (newBucket)
   {
      // save the old buckets until we move the entries out of them
      UtlChain* oldBucket     = mpBucket;
      size_t    numOldBuckets = numberOfBuckets();

      // put in the new buckets
      mBucketBits = newBucketBits;
      mpBucket = newBucket;

      // move all the entries to the new buckets
      size_t old;
      size_t toBeMoved;
      for (old = 0, toBeMoved = mElements;
           old < numOldBuckets && toBeMoved;
           old++
           )
      {
         while(!oldBucket[old].isUnLinked()) // old bucket is not empty yet
         {
            UtlLink* link = static_cast<UtlLink*>(oldBucket[old].head());
            link->detachFromList(&oldBucket[old]);
            insert(link, &mpBucket[bucketNumber(link->hash)]);
            toBeMoved--;
         }
      }

      delete [] oldBucket; // finished with the old empty buckets
   }
   else
   {
      assert(newBucket); // failed to allocate new buckets
   }
}

/* ============================ MANIPULATORS ============================== */


UtlContainable* UtlHashBag::insert(UtlContainable* insertedContainable)
{
   if (insertedContainable) // NULL keys are not allowed
   {
      OsLock take(mContainerLock);

      UtlLink*  link;

      link = UtlLink::get();
      link->data  = insertedContainable;
      link->hash  = insertedContainable->hash();

      mElements++;
      insert(link, &mpBucket[bucketNumber(link->hash)]);
   }

   return insertedContainable;
}

/*
 * Insert a link into a bucket (the bucket list is ordered by hashcode).
 */
void UtlHashBag::insert(UtlLink*       link,  ///< The UtlLink for the entry if it was found.
                        UtlChain*      bucket ///< The bucket list header where the entry belongs.
                        )
{
   UtlLink* inBucket;

   for (inBucket = static_cast<UtlLink*>(bucket->listHead());
        (   inBucket                     // not end of list
         && link->hash > inBucket->hash  // hash list is ordered, so if > then we're done.
         );
        inBucket = static_cast<UtlLink*>(inBucket->UtlChain::next)
        )
   {
   }
   link->UtlChain::listBefore(bucket, inBucket);

   resizeIfNeededAndSafe();
}

UtlContainable* UtlHashBag::remove(const UtlContainable* object)
{
   UtlContainable* removed = NULL;

   if (object)
   {
      OsLock take(mContainerLock);

      UtlLink*  link;
      UtlChain* bucket;

      if ( lookup(object, bucket, link) )
      {
         removed = link->data;

         notifyIteratorsOfRemove(link);

         link->detachFromList(bucket);
         removed = link->data;
         link->release();

         mElements--;
      }
   }

   return removed;
}

/**
 * Removed the designated object by reference
 * (as opposed to searching for an equality match).
 *
 * @return the object if successful, otherwise null
 */
UtlContainable* UtlHashBag::removeReference(const UtlContainable* object)
{
   UtlContainable* removed = NULL;

   if (object)
   {
      size_t   keyHash = object->hash();

      OsLock take(mContainerLock);

      UtlLink*  link;
      UtlChain* bucket;
      UtlLink* check;

      bucket = &mpBucket[bucketNumber(keyHash)];
      for (link = NULL, check = static_cast<UtlLink*>(bucket->listHead());
           (   !link                  // not found
            && check                  // not end of list
            && check->hash <= keyHash // hash list is ordered, so if > then it's not in the list
            );
           check = check->next()
           )
      {
         if (check->data == object)
         {
            link = check; // found it
         }
      }

      if (link)
      {
         notifyIteratorsOfRemove(link);

         link->detachFromList(bucket);
         removed = link->data;
         link->release();

         mElements--;
      }
   }

   return removed;
}


UtlBoolean UtlHashBag::destroy(const UtlContainable* object)
{
   UtlBoolean deletedAnObject = FALSE;

   // no need to take locks... all the changes are inside remove
   UtlContainable* wasRemoved = remove(object);

   if(wasRemoved)
   {
      delete wasRemoved;
      deletedAnObject = TRUE;
   }

   return deletedAnObject;
}


void UtlHashBag::removeAll()
{
   OsLock take(mContainerLock);

   size_t i;
   size_t toBeRemoved;
   for (i = 0, toBeRemoved = mElements;
        i < numberOfBuckets() && toBeRemoved;
        i++
        ) // for each bucket
   {
      while(!mpBucket[i].isUnLinked()) // bucket is not empty yet
      {
         UtlLink* link = static_cast<UtlLink*>(mpBucket[i].head());
         notifyIteratorsOfRemove(link);
         link->detachFromList(&mpBucket[i]);
         link->release();
         toBeRemoved--;
      }
   }
   mElements = 0;
}


void UtlHashBag::destroyAll()
{
   OsLock take(mContainerLock);

   size_t i;
   size_t toBeDestroyed;
   for (i = 0, toBeDestroyed = mElements;
        i < numberOfBuckets() && toBeDestroyed;
        i++
        ) // for each bucket
   {
      while(!mpBucket[i].isUnLinked()) // bucket is not empty yet
      {
         UtlLink* link = static_cast<UtlLink*>(mpBucket[i].head());
         notifyIteratorsOfRemove(link);
         link->detachFromList(&mpBucket[i]);
         delete link->data;
         link->release();
         toBeDestroyed--;
      }
   }
   mElements = 0;
}


/* ============================ ACCESSORS ================================= */


UtlContainable* UtlHashBag::find(const UtlContainable* object) const
{
   UtlContainable* foundObject = NULL;

   OsLock take(mContainerLock);

   UtlLink*  link;
   UtlChain* bucket;

   if (lookup(object, bucket, link))
   {
      foundObject = link->data;
   }

   return foundObject;
}

/**
 * Search for the designated object by reference.
 * @return the object if found, otherwise NULL.
 */
UtlContainable* UtlHashBag::findReference(const UtlContainable* object) const
{
   UtlContainable* found = NULL;

   if (object)
   {
      OsLock take(mContainerLock);

      UtlLink*  link = NULL;
      UtlChain* bucket;
      UtlLink* check;

      // walk the buckets
      for (size_t i = 0; link == NULL && i < numberOfBuckets(); i++)
      {
         bucket = &mpBucket[i];

         for (link = NULL, check = static_cast<UtlLink*>(bucket->listHead());
              (   !link                  // not found
               && check                  // not end of list
                 );
              check = check->next()
            )
         {
            if (check->data == object)
            {
               link = check; // found it
            }
         }
      }

      if (link)
      {
         found = link->data;
      }
   }

   return found;
}


/* ============================ INQUIRY =================================== */


size_t UtlHashBag::entries() const
{
   OsLock take(mContainerLock);

   return mElements;
}


UtlBoolean UtlHashBag::isEmpty() const
{
   OsLock take(mContainerLock);

   return mElements == 0;
}


UtlBoolean UtlHashBag::contains(const UtlContainable* object)  const
{
   UtlLink*  link;
   UtlChain* bucket;

   OsLock take(mContainerLock);

   return lookup(object, bucket, link);
}


/**
 * Get the ContainableType for the hash bag as a contained object.
 */
UtlContainableType UtlHashBag::getContainableType() const
{
   return UtlHashBag::TYPE;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

void UtlHashBag::notifyIteratorsOfRemove(const UtlLink* link)
{
   UtlLink* listNode;
   UtlHashBagIterator* foundIterator;

   for (listNode = mIteratorList.head(); listNode; listNode = listNode->next())
   {
      foundIterator = (UtlHashBagIterator*)listNode->data;
      foundIterator->removing(link);
   }
}

/*
 * Search for a given key value and return the bucket and UtlLink for it.
 * Return true if the key was found, and false if not.
 */
bool UtlHashBag::lookup(const UtlContainable* key, ///< The key to locate.
                        UtlChain*&      bucket, /**< The bucket list header in which it belongs.
                                                 *   This is set regardless of whether or not the
                                                 *   key was found in the table. */
                        UtlLink*&       link    /**< If the key was found, the UtlLink for entry.
                                                 *   If the key was not found, this is NULL. */
                        ) const
{
   UtlLink* check;
   size_t   keyHash = key->hash();

   bucket = &mpBucket[bucketNumber(keyHash)];
   for (link = NULL, check = static_cast<UtlLink*>(bucket->listHead());
        (   !link                  // not found
         && check                  // not end of list
         && check->hash <= keyHash // hash list is ordered, so if > then it's not in the list
         );
        check = static_cast<UtlLink*>(check->UtlChain::next)
        )
   {
      if (check->hash == keyHash && check->data->isEqual(key))
      {
         link = check; // found it
      }
   }
   return link != NULL;
}

size_t UtlHashBag::bucketNumber(unsigned hash) const
{
   /*
    * We only use mBucketBits of the hash to index mpBucket, but we don't want to
    * loose the information in the higher bits of the hash code.  So we 'fold' the
    * high order bits by XORing them mBucketBits at a time into the bits we'll
    * use until there are no non-zero high order bits left.
    */
   size_t foldedHash;
   size_t highBits;

   size_t lowBitsMask = numberOfBuckets() - 1;
   for ( (foldedHash = hash & lowBitsMask, // get the low bits we want into the folded value
          highBits   = hash                // don't bother masking off the low bits
          );
         (highBits = highBits >> mBucketBits); // shift out bits already used until zero
         foldedHash ^= highBits & lowBitsMask // incorporate non-zero
        )
   {
      // empty
   }
   return foldedHash;
}
