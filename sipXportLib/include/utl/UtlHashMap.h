//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _UtlHashMap_h_
#define _UtlHashMap_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlDefs.h"
#include "utl/UtlContainer.h"

// DEFINES
// MACROS
#define NUM_HASHMAP_BUCKETS(bits) (1<<bits)

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class UtlContainable;
class UtlPair;

/**
 * UtlHashMap is a container object that allows you to store keys and
 * values.  Key must be unique (testing for equality using the
 * UtlContainer::isEquals(...) method).
 */
class UtlHashMap : public UtlContainer
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

    /**
     * Default Constructor
     */
    UtlHashMap();

    /**
     * Destructor
     */
    virtual ~UtlHashMap();

/* ============================ MANIPULATORS ============================== */

    /**
     * Inserts a key and value pair into the hash map.
     *
     * If the inserted key is already in the table, this method
     * fails (returns NULL - note that this means if value is NULL,
     * then you can't tell whether there was an error or not).
     * To replace the value for a given key, the old value must
     * be Removed before the new value is inserted.
     *
     * @return the key on success, otherwise NULL
     */
    UtlContainable* insertKeyAndValue(UtlContainable* key, UtlContainable* value);


    /**
     * Inserts the designated containable object into the list
     * with a NULL value (see note regarding use of NULL value
     * in insertKeyAndValue).
     * If there is an equal key in the UtlHashMap already,
     * the insert will fail.
     *
     * @return the object if successful, otherwise NULL
     */
    UtlContainable* insert(UtlContainable* obj);


    /**
     * Remove the designated key and its associated value.
     *
     * @return the key or NULL if not found
     */
    UtlContainable* remove(const UtlContainable* key);


    /**
     * Remove the designated object by reference
     * (as opposed to searching for an equality match).
     * Note that *object must be an allocated UtlContainable, as removeReference
     * evaluates its hash.
     *
     * @return the key or NULL if not found
     */
    UtlContainable* removeReference(const UtlContainable* key);


    /**
     * Remove the designated key and its associated value.
     * The UtlHashMap is searched for an entry whose key is equal to 'key'.
     * If found, the entry's key pointer is returned, the entry's value
     * pointer is put in *value, and the entry is deleted from the UtlHashMap.
     *
     * @return the key or NULL if not found
     */
    UtlContainable* removeKeyAndValue(const UtlContainable* key, UtlContainable*& value);


    /**
     * Removes the designated key and its associated value from the map
     * and frees the key and the value (if not NULL) by calling delete.
     */
    virtual UtlBoolean destroy(const UtlContainable* key);


    /**
     * Removes all elements from the hash map and deletes each element.
     *
     * [XPL-195] This method holds the global lock on the UtlHashMap while
     * calling the destructors for the keys and values, so those destructors
     * cannot reference the hash map.
     */
    virtual void destroyAll();


    /**
     * Removes all elements from the hash map without deleting the elements
     */
    virtual void removeAll();

/* ============================ ACCESSORS ================================= */

    /**
     * Return the value for a given key or NULL if not found.
     */
    UtlContainable* findValue(const UtlContainable* key) const;


    /**
     * Return the designated key if found otherwise NULL.
     */
    virtual UtlContainable* find(const UtlContainable* key) const;

/* ============================ INQUIRY =================================== */

    /**
     * Return the total number of keys in the hash map
     */
    size_t entries() const;


    /**
     * Return true if the hash map is empty (entries() == 0), otherwise false.
     */
    UtlBoolean isEmpty() const;


    /**
     * Return true if the hash map includes an entry with the specified key.
     */
    UtlBoolean contains(const UtlContainable* key) const;


    /**
     * Get the ContainableType for the hash bag as a contained object.
     */
    virtual UtlContainableType getContainableType() const;

    static const UtlContainableType TYPE;///< the type constant for this class

    /**
     * Make a copy of all of the items BY POINTER in (*this) instance
     * into the given map. It does not clear the given map. IF USING
     * destroyAll call, be sure to call this on only ONE map instance.
     */
    void copyInto(UtlHashMap& map) const;

    /// The current number of buckets in the hash.
    size_t numberOfBuckets() const
       {
          return NUM_HASHMAP_BUCKETS(mBucketBits);
       }


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    friend class UtlHashMapIterator;

    static const UtlContainable* INTERNAL_NULL;

    /// If the Hash is too full, add additional buckets.
    /**
     * Assumes that the caller is holding the mContainerLock.
     *
     * This calls resize to actually do the resize if it is safe.
     */
    void resizeIfNeededAndSafe()
       {
          if (   ( mElements / NUM_HASHMAP_BUCKETS(mBucketBits) >= 3 ) // mean bucket 3 or more
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

    static    UtlChainPool* spPairPool; ///< pool of available UtlPair objects.

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

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
                UtlPair*&       pair       /**< If the key was found, the UtlPair for the entry.
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

    // no copy constructor is provided
    UtlHashMap(UtlHashMap&);

    /** Use copyInto instead */
    UtlHashMap& operator=(const UtlHashMap&);

    /**
     * notifyIteratorsOfRemove - called before removing any entry from the UtlHashMap
     */
    void notifyIteratorsOfRemove(const UtlPair* pair);
};

#endif    // _UtlHashMap_h_
