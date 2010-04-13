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
#include "utl/UtlContainable.h"
#include "utl/UtlLink.h"
#include "utl/UtlInt.h"
#include "utl/UtlHashMapIterator.h"
#include "utl/UtlHashMap.h"
#include "os/OsLock.h"


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const UtlContainableType UtlHashMap::TYPE = "UtlHashMap";

#define HASHMAP_INITIAL_BUCKET_BITS 4

// STATIC VARIABLE INITIALIZATIONS

const UtlInt INTERNAL_NULL_OBJECT(666);
const UtlContainable* UtlHashMap::INTERNAL_NULL = &INTERNAL_NULL_OBJECT;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Default Constructor
UtlHashMap::UtlHashMap() :
   mElements(0),
   mBucketBits(HASHMAP_INITIAL_BUCKET_BITS),
   mpBucket(new UtlChain[NUM_HASHMAP_BUCKETS(HASHMAP_INITIAL_BUCKET_BITS)])
{
}


// Destructor
UtlHashMap::~UtlHashMap()
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
         UtlPair* pair = static_cast<UtlPair*>(mpBucket[i].listHead());
         pair->detachFromList(&mpBucket[i]);
         pair->release();
      }
   }
   delete [] mpBucket;   // free the bucket headers
}

/*
 * Allocate additional buckets and redistribute existing contents.
 * This should only be called through resizeIfNeededAndSafe.
 */
void UtlHashMap::resize()
{
   // already holding the mContainerLock
   UtlChain* newBucket;
   size_t    newBucketBits;

   // if an iterator had prevented resizing while many elements were added,
   // we might need to double more than once to restore the target ratio.
   for (newBucketBits = mBucketBits+1;
        mElements / NUM_HASHMAP_BUCKETS(newBucketBits) >= 3;
        newBucketBits++
        )
   {
   }

   // allocate the new buckets
   newBucket = new UtlChain[NUM_HASHMAP_BUCKETS(newBucketBits)];

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
            UtlPair* pair = static_cast<UtlPair*>(oldBucket[old].head());
            pair->detachFromList(&oldBucket[old]);
            insert(pair, &mpBucket[bucketNumber(pair->hash)]);
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

UtlContainable* UtlHashMap::remove(const UtlContainable* key)
{
   UtlContainable* unusedValue;

   return removeKeyAndValue(key, unusedValue);
}


UtlContainable* UtlHashMap::removeReference(const UtlContainable* key)
{
   UtlContainable* unusedValue;

   // Locking is done by removeKeyAndValue().

   return removeKeyAndValue(key, unusedValue);
}



UtlBoolean UtlHashMap::destroy(const UtlContainable* key)
{
   UtlBoolean wasRemoved = FALSE;
   UtlContainable* value;

   // Locking is done by removeKeyAndValue().

   UtlContainable* removedKey = removeKeyAndValue(key, value);

   if(removedKey)
   {
      wasRemoved = TRUE;
      delete removedKey;
      if (value != INTERNAL_NULL)
      {
         delete value;
      }
   }

   return wasRemoved;
}


void UtlHashMap::removeAll()
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
         UtlPair* pair = static_cast<UtlPair*>(mpBucket[i].head());
         notifyIteratorsOfRemove(pair);
         pair->detachFromList(&mpBucket[i]);
         pair->release();
         toBeRemoved--;
      }
   }
   mElements = 0;
}


void UtlHashMap::destroyAll()
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
         UtlPair* pair = static_cast<UtlPair*>(mpBucket[i].head());
         notifyIteratorsOfRemove(pair);
         pair->detachFromList(&mpBucket[i]);
         delete pair->data;
         if (pair->value != INTERNAL_NULL)
         {
            delete pair->value;
         }
         pair->release();
         toBeDestroyed--;
      }
   }
   mElements = 0;
}


// insert a key with a NULL value
UtlContainable* UtlHashMap::insert(UtlContainable* obj)
{
   // Locking will be done by insertKeyAndValue().

   return insertKeyAndValue(obj, NULL);
}

UtlContainable* UtlHashMap::insertKeyAndValue(UtlContainable* key, UtlContainable* value)
{
   UtlContainable* insertedKey = NULL;

   if (!value)
   {
      // This const_cast is a little dangerous, as the application can obtain
      // a non-const pointer to INTERNAL_NULL via UtlHashMap::find.
      value = const_cast<UtlContainable*>(INTERNAL_NULL);
   }

   if (key && value) // NULL keys and values are not allowed
   {
      OsLock take(mContainerLock);

      UtlPair*  pair;
      UtlChain* bucket;

      if(!lookup(key, bucket, pair))
      {
         pair = UtlPair::get();
         pair->data  = key;
         pair->hash  = key->hash();
         pair->value = value;
         mElements++;
         insert(pair, bucket);
         insertedKey = key;
      }
      else
      {
         // this key is already in the map, so this is an error - leave insertedKey == NULL
      }
   }
   return insertedKey;
}

UtlContainable* UtlHashMap::removeKeyAndValue(const UtlContainable* key, UtlContainable*& value)
{
   UtlContainable* removed = NULL;
   value = NULL;

   if (key)
   {
      OsLock take(mContainerLock);

      UtlPair*  pair;
      UtlChain* bucket;

      if ( lookup(key, bucket, pair) )
      {
         removed = pair->data;
         value = (pair->value != INTERNAL_NULL) ? pair->value : NULL;

         notifyIteratorsOfRemove(pair);

         pair->detachFromList(bucket);
         removed = pair->data;
         value   = pair->value;
         pair->release();

         mElements--;
      }
   }

   return removed;
}


void UtlHashMap::copyInto(UtlHashMap& into) const
{
    UtlHashMapIterator i(*this);
    while (i() != NULL)
    {
       into.insertKeyAndValue(i.key(), i.value());
    }
}


/* ============================ ACCESSORS ================================= */

UtlContainable* UtlHashMap::findValue(const UtlContainable* key) const
{
   UtlContainable* foundValue = NULL;

   UtlPair* foundPair;
   UtlChain* unusedBucket;

   OsLock take(mContainerLock);

   if (lookup(key, unusedBucket, foundPair))
   {
      foundValue = foundPair->value != INTERNAL_NULL ? foundPair->value : NULL;
   }

   return foundValue;
}


UtlContainable* UtlHashMap::find(const UtlContainable* key) const
{
   UtlContainable* foundKey = NULL;

   UtlPair* foundPair;
   UtlChain* unusedBucket;

   OsLock take(mContainerLock);

   if (lookup(key, unusedBucket, foundPair))
   {
      foundKey = foundPair->data;
   }

   return foundKey;
}


/* ============================ INQUIRY =================================== */

size_t UtlHashMap::entries() const
{
   OsLock take(mContainerLock);

   return mElements;
}


UtlBoolean UtlHashMap::isEmpty() const
{
   return entries() == 0;
}


UtlBoolean UtlHashMap::contains(const UtlContainable* key)  const
{
   return find(key) != NULL;
}


/**
 * Get the ContainableType for the hash map as a contained object.
 */
UtlContainableType UtlHashMap::getContainableType() const
{
   return UtlHashMap::TYPE;
}


void UtlHashMap::notifyIteratorsOfRemove(const UtlPair* pair)
{
   UtlLink* listNode;
   UtlHashMapIterator* foundIterator;

   for (listNode = mIteratorList.head(); listNode; listNode = listNode->next())
   {
      foundIterator = (UtlHashMapIterator*)listNode->data;
      foundIterator->removing(pair);
   }
}


/*
 * Search for a given key value and return the bucket and UtlPair for it.
 * Return true if the key was found, and false if not.
 */
bool UtlHashMap::lookup(const UtlContainable* key, ///< The key to locate.
                        UtlChain*&      bucket, /**< The bucket list header in which it belongs.
                                                 *   This is set regardless of whether or not the
                                                 *   key was found in the table. */
                        UtlPair*&       pair    /**< If the key was found, the UtlPair for entry.
                                                 *   If the key was not found, this is NULL. */
                        ) const
{
   UtlPair* check;
   size_t   keyHash = key->hash();

   bucket = &mpBucket[bucketNumber(keyHash)];
   for (pair = NULL, check = static_cast<UtlPair*>(bucket->listHead());
        (   !pair                  // not found
         && check                  // not end of list
         && keyHash <= check->hash // hash list is ordered, so if > then it's not in the list
         );
        check = static_cast<UtlPair*>(check->UtlChain::next)
        )
   {
      if (check->hash == keyHash && check->data->isEqual(key))
      {
         pair = check; // found it
      }
   }
   return pair != NULL;
}

/*
 * Insert a pair into a bucket (the bucket list is ordered by hashcode).
 */
void UtlHashMap::insert(UtlPair*       pair,  ///< The UtlPair for the entry if it was found.
                        UtlChain*      bucket ///< The bucket list header where the entry belongs.
                        )
{
   UtlPair* position;

   for (position = static_cast<UtlPair*>(bucket->listHead());
        (   position                     // not end of list
         && pair->hash <= position->hash // hash list is ordered, so if > then we're done.
         );
        position = static_cast<UtlPair*>(position->UtlChain::next)
        )
   {
   }
   /*
    * At this point, position is either:
    *  - NULL, in which case pair goes on the tail of the bucket
    *  - non-NULL in which case pair goes before position
    *
    * NOTE - it is an unchecked error if pair->data->isEqual(position->data)
    *        the caller must ensure this is not the case.
    */
   pair->UtlChain::listBefore(bucket, position);

   resizeIfNeededAndSafe();
}


size_t UtlHashMap::bucketNumber(unsigned hash) const
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
         (highBits = highBits >> mBucketBits);  // shift out bits already used until zero
         foldedHash ^= highBits & lowBitsMask // incorporate non-zero
        )
   {
   }
   return foldedHash;
}


/* ============================ FUNCTIONS ================================= */
