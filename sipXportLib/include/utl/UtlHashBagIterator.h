//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _UtlHashBagIterator_h_
#define _UtlHashBagIterator_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlDefs.h"
#include "utl/UtlIterator.h"
#include "utl/UtlHashBag.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class UtlContainable ;

/**
 * UtlHashBagIterator allows developers to iterator (walks through) an
 * UtlHashBag.
 *
 * @see UtlIterator
 * @see UtlSList
 */
class UtlHashBagIterator : public UtlIterator
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   /**
    * Construct an iterator over all objects in a given UtlHashBag
    * If key is specified, iterate only over objects that match that key
    * (UtlHashBags may have any number of copies of a given object)
    */
   UtlHashBagIterator(UtlHashBag& hashBag, UtlContainable* key = NULL);

   /**
     * Destructor
     */
    virtual ~UtlHashBagIterator();

/* ============================ MANIPULATORS ============================== */

    /**
     * Return the next element.
     *
     * @return The next element or NULL if no more elements are available.
     */
    virtual UtlContainable*    operator()() ;

    /**
     * Reset the list by moving the iterator cursor to the location before the
     * first element.
     */
    virtual void reset() ;

/* ============================ ACCESSORS ================================= */

    /**
     * Gets the key of the current element
     */
    UtlContainable* key() const ;

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    friend class UtlHashBag;

    /**
     * removing is called by the UtlHashMap when an element is about to be
     * removed from the container.  The iterator must ensure that the element
     * for the removed node is not returned by any subsequent call.
     */
    virtual void removing(const UtlLink* node);


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    void init(UtlHashBag& hashBag);

    UtlContainable* mpSubsetMatch; ///< if non-NULL, points to the key that defines the subset
    unsigned        mSubsetHash;   ///< if mpSubsetMatch != NULL, this is its hash code

    size_t   mPosition;      ///< current bucket number [0..numberOfBuckets-1]
    UtlLink* mpCurrentLink;  ///< current UtlLink within the bucket, or BEFORE_FIRST
    bool     mLinkIsValid;   /**< true if mpCurrentLink is the valid current position
                              * The only time this is false is when the UtlContainable
                              * at the current position was removed while it was current. */

    // no copy constructor
    UtlHashBagIterator(UtlHashBagIterator&);
} ;

/* ============================ INLINE METHODS ============================ */

#endif    // _UtlHashBagIterator_h_
