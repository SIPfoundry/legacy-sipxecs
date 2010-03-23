//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _UTLLINK_H_
#define _UTLLINK_H_

#include "assert.h"

// SYSTEM INCLUDES
#include "os/OsDefs.h"

// APPLICATION INCLUDES
#include "utl/UtlContainable.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class UtlLink;
class UtlChainPool;

/**
 * UtlChain is the internal class that implements the linked list blocks
 * for other Utl classes.  It may not be used directly because by itself
 * it is not thread safe.  Use one of the lists types derived from UtlList.
 *
 * Each UtlChain links forward (next) and backward (prev) in the chain of links;
 * ends of a chain are indicated by NULL values.  A UtlChain not in a chain,
 * including a newly constructed instance, has NULL pointers in both directions.
 *
 * A UtlChain can also be used as a list header whose links point to the ends
 * of a NULL-terminated list of UtlLink's.
 */
class UtlChain
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:
   /// Constructor initializes to unlinked.
   UtlChain() :
      prev(NULL),
      next(NULL)
      {
      }

   /// Destructor
   ~UtlChain()
      {
      }

/* //////////////////////////// PROTECTED //////////////////////////////////// */
  protected:
   friend class UtlChainPool;
   friend class UtlLink;
   friend class UtlPair;
   friend class UtlContainer;
   friend class UtlList;
   friend class UtlHashMap;
   friend class UtlHashMapIterator;
   friend class UtlHashBag;
   friend class UtlHashBagIterator;
   friend class UtlChainTest;
   friend class UtlLinkTest;

   UtlChain* prev; ///< backward chain pointer
   UtlChain* next; ///< forward chain pointer

   // ================================================================
   /** @name                  Chain Operations
    *
    * These methods manipulate the forward and backward links within a
    * chain.  They do no do anything with respect to any header, so they
    * can be used to implement chains that are linear (NULL terminated)
    * or circular.
    */
   ///@{

   /// Is this block not linked to anything?
   bool isUnLinked() const
      {
         return (!(prev||next));
      }

   /// Take the link out of its chain.
   void unchain()
      {
         if (prev)
         {
            prev->next = next;
         }
         if (next)
         {
            next->prev = prev;
         }
         prev=NULL;
         next=NULL;
      }

   /// Insert a new UtlChain before existing.
   /**
    * This may be called only on an unlinked UtlChain
    */
   void chainBefore(UtlChain* existing)
      {
         assert(isUnLinked()); // not valid on a link that's in a chain

         next = existing;
         if (existing->prev)
         {
            prev = existing->prev;
         }
         if (prev)
         {
            prev->next = this;
         }
         existing->prev = this;
      }


   /// Insert a new UtlChain after existing.
   /**
    * This may be called only on an unlinked UtlChain
    */
   void chainAfter(UtlChain* existing)
      {
         assert(isUnLinked()); // not valid on a link that's in a chain

         prev = existing;
         next = existing->next;
         if (next)
         {
            next->prev = this;
         }
         existing->next = this;
      }

   ///@}

   // ================================================================
   /** @name                  List Operations
    *
    * These methods do the special handling for using a UtlChain as a list
    * header.  The UtlChain.next points to the head (first) UtlLink on the
    * list.  The UtlChain.prev points to the tail (last) UtlLink.
    *
    * The UtlLink objects on the list form a NULL-terminated chain -
    * they do not point to the UtlChain that serves as the header.
    */
   ///@{

   /// Returns the head (first) UtlLink on the list (or NULL if the list is empty).
   UtlChain* listHead() const
      {
         return next;
      }

   /// Returns the tail (last) UtlLink on the list (or NULL if the list is empty).
   UtlChain* listTail() const
      {
         return prev;
      }

   /// Returns the head (first) UtlLink on the list (or NULL if the list is empty).
   UtlLink* head() const
      {
         return (UtlLink*)next;
      }

   /// Returns the tail (last) UtlLink on the list (or NULL if the list is empty).
   UtlLink* tail() const
      {
         return (UtlLink*)prev;
      }

   /// Insert this link into a list before an existing entry (before NULL == at the tail).
   void listBefore(UtlChain* list,    ///< the list to insert into
                   UtlChain* existing /**< the UtlLink for the position in the
                                       *   list to insert before.  NULL means
                                       *   at the end of the list. */
                   );
   /**<
    * @note
    * This method does not verify that the existing element is actually on the list; doing
    * so is the responsibility of the caller.  If the list is empty, existing must be NULL.
    */


   /// Insert this link into a list before an existing entry (after NULL == at the head).
   void listAfter(UtlChain* list,     ///< the list to insert into
                  UtlChain* existing  /**< the UtlLink for the position in the
                                       *   list to insert after.  NULL means
                                       *   at the beginning of the list. */
                  );
   /**<
    * @note
    * This method does not verify that the existing element is actually on the list; doing
    * so is the responsibility of the caller.  If the list is empty, existing must be NULL.
    */

   /// Remove a link from a list.
   UtlChain* detachFromList(UtlChain* listHead);
   /**<
    * @note
    * This method does not verify that the UtlLink
    * object being detached is actually on the specified list; doing
    * so is the responsibility of the caller.
    */

///@}

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:
};

/**
 * UtlLink implements linked lists of data blocks.
 * It may not be used directly because it is not thread safe; use one of the
 * classes derived from UtlList.
 *
 * In addition to the links forward (next) and backward (prev) provided by the
 * parent UtlChain, a UtlLink also points to an item whose place it implements
 * in the list (data).
 *
 * @nosubgrouping
 */
class UtlLink : public UtlChain
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

   // ================================================================
   /** @name                  Traversal Operations
    *
    * These methods move forward and back in a chain of UtlLinks.
    */
   ///@{

   /// Returns the next UtlLink forward in a chain (or NULL for the end).
   UtlLink* next() const
      {
         return static_cast<UtlLink*>(UtlChain::next);
      }

   /// Returns the next UtlLink backward in a chain (or NULL for the end).
   UtlLink* prev() const
      {
         return static_cast<UtlLink*>(UtlChain::prev);
      }

   /// Linear search starting at this link for a matching data value.
   UtlLink* findDataRef(UtlContainable* target) const
      {
         UtlLink* theLink;
         for (theLink=const_cast<UtlLink*>(this);
              theLink && theLink->data != target;
              theLink=theLink->next())
         {
         }
         return theLink;
      }

   /// Linear search starting at this link for a matching data value.
   UtlLink* findNextHash(unsigned targetHash) const
      {
         UtlLink* theLink;
         for (theLink=const_cast<UtlLink*>(this);
              theLink && theLink->hash != targetHash;
              theLink=theLink->next())
         {
         }
         return theLink;
      }

   /// The containable object whose place in the list this UtlLink is tracking.
   UtlContainable*    data;
   /// The hash code for the containable object whose place in the list this UtlLink is tracking.
   unsigned           hash;

   ///@}

   // ================================================================
   ///@name                  Memory Management
   ///@{

   /// Get the total number of UtlLink blocks allocated.
   /**
    * Because the underlying UtlLinkPool implementation allocates UtlLinks in blocks,
    * this number will usually be slightly higher than the maximum number ever in use
    * (rounded up to the nearest UTLLINK_BLOCK_SIZE)
    */
   static size_t totalAllocated();

   ///@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:
   friend class UtlChainPool;
   friend class UtlContainer;
   friend class UtlList;
   friend class UtlListIterator;
   friend class UtlSList;
   friend class UtlSListIterator;
   friend class UtlSortedList;
   friend class UtlHashBag;
   friend class UtlHashBagIterator;
   friend class UtlLinkTest;

   // ================================================================
   /** @name                  Link Manipulation in a Chain
    *
    * These methods insert and remove this UtlLink in a chain.  They do not
    * do any special handling for the ends of a chain, so they can be used
    * in either NULL-terminated or circular chains.
    */
   ///@{

   /// Take the link block out of its list, and return the data pointer
   UtlContainable* unlink();
   /**<
    * @note
    * After this call, the UtlLink has been released, and the pointer to it
    * may not be used.
    */

   /// Insert a new UtlLink to newData before existing, returning the new UtlLink.
   static UtlLink* before(UtlChain* existing, UtlContainable* newData);

   /// Insert a new UtlLink to newData after existing, returning the new UtlLink.
   static UtlLink* after(UtlChain* existing, UtlContainable* newData);

   ///@}

   // ================================================================
   /** @name                  List Operations
    *
    * These methods do the special handling for using a UtlChain as a list
    * header.  The UtlChain.next points to the head (first) UtlLink on the
    * list.  The UtlChain.prev points to the tail (last) UtlLink.
    *
    * The UtlLink objects on the list form a NULL-terminated chain -
    * they do not point to the UtlChain that serves as the header.
    */
   ///@{


   /// Insert a new item into a list before an existing entry (before NULL == at the tail).
   static UtlLink* listBefore(UtlChain* list,    ///< the list to insert into
                              UtlChain* existing,/**< the UtlLink for the position in the
                                                  *   list to insert before.  NULL means
                                                  *   at the end of the list. */
                              UtlContainable* newData ///< the new data item to be inserted.
                              );
   /**<
    * @note
    * This method does not verify that the existing element is actually on the list; doing
    * so is the responsibility of the caller.  If the list is empty, existing must be NULL.
    */


   /// Insert a new item into a list before an existing entry (after NULL == at the head).
   static UtlLink* listAfter(UtlChain* list,     ///< the list to insert into
                             UtlChain* existing, /**< the UtlLink for the position in the
                                                  *   list to insert after.  NULL means
                                                  *   at the beginning of the list. */
                             UtlContainable* newData ///< the new data item to be inserted.
                             );
   /**<
    * @note
    * This method does not verify that the existing element is actually on the list; doing
    * so is the responsibility of the caller.  If the list is empty, existing must be NULL.
    */

   /// Remove a link from a list.
   UtlContainable* detachFrom(UtlChain* listHead);
   /**<
    * @note
    * This method does not verify that the UtlLink
    * object being detached is actually on the specified list; doing
    * so is the responsibility of the caller.
    */

   /// Find the first matching target in the list by reference.
   static UtlLink* findData(UtlChain* list, UtlContainable* target)
      {
         return list->next ? static_cast<UtlLink*>(list->next)->findDataRef(target) : NULL;
      }

   ///@}

   // ================================================================
   /** @name                  Constructor and Destructor
    *
    * @see UtlLinkPool for how a UtlLink is allocated and freed.
    */
   ///@{

   /// The UtlLink constructor is protected.
   /**
    * A UtlLink should only be instantiated by a call to UtlLinkPool::get
    * because the UtlLinkPool recycles them rather than allocating and
    * deallocating from the system heap.
    */
   UtlLink() :
      data(NULL),
      hash(0)
      {
      };

   /// Destructor
   /**
    * A UtlLink is only destructed when the UtlLinkPool destructor is invoked.
    */
   ~UtlLink()
      {
      };

   /// Get a UtlLink from the pool.
   static UtlLink* get();

   /// Return a UtlLink to the pool.
   void release();

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

   /// The allocator function to be passed to the UtlChainPool
   static void allocate(size_t    blocksize, ///< number of instances to allocate
                        UtlChain* blockList, ///< list header for first instance
                        UtlChain* pool       ///< list header for others
                        );


   /// The pool of available UltLink instances.
   static UtlChainPool* spLinkPool;
};

/// Associate a key object (the parent UtlLink data) with its value object.
class UtlPair : public UtlLink
{
  protected:
   friend class UtlHashMap;
   friend class UtlHashMapIterator;
   friend class UtlHashBagIterator;

   UtlContainable* value;

   UtlPair() :
      value(NULL)
      {
      };

   ~UtlPair()
      {
      }

   static UtlChainPool* spPairPool;

   /// Get a UtlPair from the pool.
   static UtlPair* get();

   /// Return a UtlPair to the pool.
   void release();

  private:
   /// The allocator function to be passed to the UtlChainPool
   static void allocate(size_t    blocksize, ///< number of instances to allocate
                        UtlChain* blockList, ///< list header for first instance
                        UtlChain* pool       ///< list header for others
                        );

};

#endif    // _UTLLINK_H_
