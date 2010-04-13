//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _UtlHashBag_h_
#define _UtlHashBag_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlDefs.h"
#include "utl/UtlContainer.h"
#include "utl/UtlLink.h"

// DEFINES
// MACROS
#define NUM_HASHBAG_BUCKETS(bits) (1<<bits)

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class UtlContainable;

/**
 * A UtlHashBag is an orderless container that efficiently allows for both
 * random access and iteration.
 */
class UtlHashBag : public UtlContainer
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

/* ============================ CREATORS ================================== */

   /**
    * Constructor
    */
   UtlHashBag();

   /**
    * Destructor
    */
   virtual ~UtlHashBag();
/* ============================ MANIPULATORS ============================== */

   /**
    * Insert the designated object into this container.
    *
    * @return the given object on success otherwise null.
    */
   virtual UtlContainable* insert(UtlContainable* object);

   /**
    * Remove one matching object from this container.
    *
    * @return the removed object if a match was found, otherwise NULL.
    */
   virtual UtlContainable* remove(const UtlContainable* object);

   /**
    * Remove the designated object by reference
    * (as opposed to searching for an equality match).
    * Note that *object must be an allocated UtlContainable, as removeReference
    * evaluates its hash.
    *
    * @return the object if successful, otherwise null
    */
   virtual UtlContainable* removeReference(const UtlContainable* object);

   /**
    * Removes one matching object from the bag and deletes the object
    *
    * @return true if a match was found, false if not
    */
   virtual UtlBoolean destroy(const UtlContainable* object);

   /**
    * Removes all elements from the container and deletes each one.
     *
     * [XPL-195] This method holds the global lock on the UtlHashBag while
     * calling the destructor for the values, so the destructor cannot
     * reference the hash bag.
    */
   virtual void destroyAll();

   /**
    * Removes all elements from the container without freeing the objects.
    */
   virtual void removeAll();

/* ============================ ACCESSORS ================================= */

   /**
    * Return the designated object if found, otherwise null.
    */
   virtual UtlContainable* find(const UtlContainable* object) const;

   /**
    * Search for the designated object by reference.
    * @return the object if found, otherwise NULL.
    * 'object' need not be a valid object pointer.
    */
   virtual UtlContainable* findReference(const UtlContainable* object) const;


/* ============================ INQUIRY =================================== */


   /**
    * Return the total number of elements within the container.
    */
   size_t entries() const;

   /**
    * Return true of the container is empty (entries() == 0), otherwise false.
    */
   UtlBoolean isEmpty() const;

   /**
    * Return true if the container includes the designated object.  Each
    * element within the list is tested for equality against the designated
    * object using the equals() method.
    */
   UtlBoolean contains(const UtlContainable* object) const;


   /**
    * Get the ContainableType for the hash bag as a contained object.
    */
   virtual UtlContainableType getContainableType() const;

   static UtlContainableType TYPE; ///< the type constant for this class

   /// The current number of buckets in the hash.
   size_t numberOfBuckets() const
      {
         return NUM_HASHBAG_BUCKETS(mBucketBits);
      }

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:
   friend class UtlHashBagIterator;

   void notifyIteratorsOfRemove(const UtlLink* pair);

   /// If the Hash is too full, add additional buckets.
   /**
    * Assumes that the caller is holding the mContainerLock.
    *
    * This calls resize to actually do the resize if it is safe.
    */
   void resizeIfNeededAndSafe()
      {
         if (   ( mElements / NUM_HASHBAG_BUCKETS(mBucketBits) >= 3 ) // mean bucket 3 or more
             && ( mIteratorList.isUnLinked() )   /* there are no iterators -
                                                  * resizing moves elements to new buckets,
                                                  * which could cause an iterator to miss some
                                                  * and to return others more than once.
                                                  */
             )
         {
            resize();
         }
      }

   size_t    mElements;   ///< number of UtlContainable objects in this UtlHashMap
   size_t    mBucketBits; ///< number of bits used to index the buckets
   UtlChain* mpBucket;    ///< an array of 2**n UtlChain elements, each used as a list header.

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:


   /// Insert a link into a bucket (the bucket list is ordered by hashcode).
   void insert(UtlLink*       link,  ///< The UtlLink for the entry if it was found.
               UtlChain*      bucket ///< The bucket list header where the entry belongs.
               );

   /// Allocate additional buckets and redistribute existing contents.
   void resize();
   /**
    * This should only be called through resizeIfNeededAndSafe.
    */

   /// Search for a given key value and return the the UtlPair and bucket for it.
   bool lookup(const UtlContainable* key, ///< The key to locate.
               UtlChain*&      bucket,    /**< The bucket list header in which it belongs.
                                           *   This is set regardless of whether or not the
                                           *   key was found in the table. */
               UtlLink*&       pair       /**< If the key was found, the UtlPair for the entry.
                                           *   If the key was not found, this is NULL. */
               ) const;
   /**<
    * @return true if the key was found, and false if not.
    */

   /// Insert a pair into a bucket.
   void insert(UtlPair*        pair,   /**< The UtlPair for the entry - data, value, and hash
                                        *   are already set. */
               UtlChain*       bucket  ///< The bucket list header where the entry belongs.
               );

   /// Calculate the bucket number for a given hash.
   size_t bucketNumber(unsigned hash) const;


   // Don't allow the implicit copy constructor.
   UtlHashBag(UtlHashBag&);

   UtlHashBag& operator=(UtlHashBag&);

};

/* ============================ INLINE METHODS ============================ */


#endif    // _UtlHashBag_h_
