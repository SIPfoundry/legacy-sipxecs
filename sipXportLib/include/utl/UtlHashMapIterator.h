//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _UtlHashMapIterator_h_
#define _UtlHashMapIterator_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlDefs.h"
#include "utl/UtlIterator.h"
#include "utl/UtlHashMap.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class UtlContainable;
class UtlPair;

/**
 * UtlHashMapIterator allows developers to iterate (walks through) the
 * objects in a UtlHashMap.
 *
 * @see UtlIterator
 * @see UtlSList
 */
class UtlHashMapIterator : public UtlIterator
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

/* ============================ CREATORS ================================== */

   /**
    * Constructor accepting a source UtlHashMap
    */
   UtlHashMapIterator(const UtlHashMap& hashMap);


   /**
    * Destructor
    */
   virtual ~UtlHashMapIterator();

/* ============================ MANIPULATORS ============================== */

   /**
    * Return the key for the next element.
    *
    * @return The next element key or NULL if no more elements are available.
    */
   virtual UtlContainable* operator()();


   /**
    * Reset the list by moving the iterator cursor to the location before the
    * first element.
    */
   virtual void reset();


/* ============================ ACCESSORS ================================= */

   /**
    * Gets the key of the current element
    *
    * This method is undefined if the next element has not been called
    * (e.g. immediately after construction or after calling reset()).
    * If the current element has been removed from the hash (either through
    * the remove method on the iterator or directly on the UtlHashMap),
    * this method returns NULL.  The remaining values in the iteration sequence
    * are not affected (the next call to () will return the next key).
    */
   UtlContainable* key() const;


   /**
    * Gets the value of the current element
    *
    * This method is undefined if the next element has not been called
    * (e.g. immediately after construction or after calling reset()).
    * If the current element has been removed from the hash (either through
    * the remove method on the iterator or directly on the UtlHashMap),
    * this method returns NULL.  The remaining values in the iteration sequence
    * are not affected (the next call to () will return the next key).
    */
   UtlContainable* value() const;

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:
   friend class UtlHashMap;

   /**
    * removing is called by the UtlHashMap when an element is about to be
    * removed from the container.  The iterator must ensure that the element
    * for the removed node is not returned by any subsequent call.
    */
   virtual void removing(const UtlPair* key);


/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

   void init();

   size_t   mPosition;      ///< current bucket number [0..numberOfBuckets-1]
   UtlPair* mpCurrentPair;  ///< current UtlPair within the bucket, or BEFORE_FIRST
   bool     mPairIsValid;   /**< true if mpCurrentPair is the valid current position
                             * The only time this is false is when the UtlContainable
                             * at the current position was removed while it was current. */

   // no copy constructor
   UtlHashMapIterator(UtlHashMapIterator&);

};

/* ============================ INLINE METHODS ============================ */

#endif    // _UtlHashMapIterator_h_
