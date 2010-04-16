//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include "os/OsDefs.h"
#include "os/OsBSem.h"
#include "os/OsLock.h"
#include "assert.h"

// APPLICATION INCLUDES
#include "utl/UtlLink.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#ifndef UTLLINK_BLOCK_SIZE
#define UTLLINK_BLOCK_SIZE 1000
#endif

// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Insert a new item into a list before an existing entry (before NULL == at the tail).
void UtlChain::listBefore(UtlChain* list,
                          UtlChain* existing
                          )
{
   if (!existing) // before NULL means at the tail
   {
      // insert on the tail of the list
      if (list->prev)
      {
         // this list has at least one UtlLink on it, so just link this on the existing chain
         chainAfter(list->prev);
         list->prev = this;
      }
      else
      {
         list->next = this;
         list->prev = this;
      }
   }
   else
   {
      chainBefore(existing);
      if (list->next == existing) // existing was the head of the list
      {
         list->next = this;
      }
   }
}


/// Insert a new item into a list before an existing entry (after NULL == at the head).
void UtlChain::listAfter(UtlChain* list,
                         UtlChain* existing
                         )
{

   if (!existing) // after NULL means at the head
   {
      // insert on the head of the list
      if (list->next)
      {
         // this list has at least one UtlLink on it, so just link this on the existing chain
         chainBefore(list->next);
         list->next = this;
      }
      else
      {
         list->next = this;
         list->prev = this;
      }
   }
   else
   {
      chainAfter(existing);
      if (list->prev == existing) // existing was the tail of the list
      {
         list->prev = this;
      }
   }
}

UtlChain* UtlChain::detachFromList(UtlChain* list)
{
   if (this == list->next)
   {
      list->next = next;
   }
   if (this == list->prev)
   {
      list->prev = prev;
   }
   unchain();

   return this;
}


/// Pool of available objects derived from UtlChain.
/**
 * This avoids excessive heap operations; rather than delete unused UtlChains, they are
 * stored on the mPool here.  To limit the heap overhead associated with allocating
 * UtlChain, they are allocated in mBlockSize blocks, which are chained on
 * mBlocks.
 *
 * The actual allocation of the blocks and initial chaining is done by the allocator
 * function supplied by the UtlChain subclass.
 */
class UtlChainPool
{
public:

protected:
   friend class UtlLink;
   friend class UtlPair;

   /// Allocate blocksize instances of the subclass and chain them into the pool.
   typedef void allocator(size_t    blocksize, ///< number of instances to allocate
                          UtlChain* blockList, ///< list header for first instance
                          UtlChain* pool       ///< list header for others
                          );
   /**<
    * This function is supplied by the subclass to the UtlChainPool constructor.
    * It is responsible for allocating a block of blocksize instances of its subclass.
    * The first instance in each block is added to the blockList, so that the UtlChainPool
    * destructor can delete the block.  The remaining (blocksize-1) instances are
    * chained onto the pool list header.
    */

   /// Create a UtlChainPool that uses blockAllocator to create UtlChain derived objects.
   UtlChainPool(allocator* blockAllocator, size_t blockSize) :
      mLock(OsBSem::Q_PRIORITY, OsBSem::FULL),
      mBlockSize(blockSize),
      mAllocations(0),
      mAllocator(blockAllocator)
      {
      }

   /// Get a UtlLink with chain pointers NULL
   UtlChain* get()
      {
         UtlChain* newChain;
         {  // critical section for member variables
            OsLock poolLock(mLock);

            if (mPool.isUnLinked()) // are there available objects in the pool?
            {
               // no - get the subclass to allocate some more
               mAllocator(mBlockSize, &mBlocks, &mPool);
               mAllocations++;
            }

            // pull the first UtlChain off the mPool
            newChain = mPool.listHead();
            if (newChain)
            {
               newChain->detachFromList(&mPool);
            }
         }  // end of critical section
         return newChain;
      }

   /// Return freeLink to the pool of available UtlLinks.
   void release(UtlChain* freeChain)
      {
         OsLock poolLock(mLock);

         // put this freed object on the tail of the pool list
         freeChain->listBefore(&mPool, NULL);
      }

   /// Returns the total number of subclasses instances allocated by this pool.
   /**
    * The returned count does not include the 1 instance in each allocation that is
    * consumed to manage the pool.
    */
   size_t totalAllocated()
      {
         return mAllocations * (mBlockSize-1); // one per block is overhead
      }

private:

   /// Release all dynamic memory used by the UtlLinkPool.
   ~UtlChainPool()
      {
         OsLock poolLock(mLock);

         UtlChain* block;
         while (!mBlocks.isUnLinked()) // blocks still on block list
         {
            block = mBlocks.listHead()->detachFromList(&mBlocks);
            delete[] block;
         }
      }

   OsBSem        mLock; ///< lock for all the other member variables
   size_t        mBlockSize;
   size_t        mAllocations;
   allocator*    mAllocator;
   UtlChain      mPool;     ///< list of available UtlLinks.
   UtlChain      mBlocks;   /**< list of memory blocks allocated by the mAllocator.
                             *   Each block is an mBlockSize array of objects derived from
                             *   UtlChain. The 0th element is used to form the linked list
                             *   of blocks.  The rest are made a part of the mPool.*/
};

// The pool of available UtlLinks
UtlChainPool* UtlLink::spLinkPool = new UtlChainPool(UtlLink::allocate, UTLLINK_BLOCK_SIZE);

UtlContainable* UtlLink::unlink()
{
   // Take the link block out of its list, and return the data pointer
   unchain();
   UtlContainable* theData = data;
   spLinkPool->release(this);
   // :NOTE: cannot reference any member after call to release...

   return theData;
}


UtlLink* UtlLink::before(UtlChain* existing, UtlContainable* newData)
{
   UtlLink* newLink;

   newLink       = get();
   newLink->data = newData;
   newLink->hash = newData->hash();
   newLink->chainBefore(existing);
   return newLink;
}


UtlLink* UtlLink::after(UtlChain* existing, UtlContainable* newData)
{
   UtlLink* newLink;

   newLink       = get();
   newLink->data = newData;
   newLink->hash = newData->hash();
   newLink->chainAfter(existing);
   return newLink;
}

/// Insert a new item into a list before an existing entry (before NULL == at the tail).
UtlLink* UtlLink::listBefore(UtlChain* list,
                             UtlChain* existing,
                             UtlContainable* newData
                             )
{
   UtlLink* newLink;

   newLink       = get();
   newLink->data = newData;
   newLink->hash = newData->hash();
   newLink->UtlChain::listBefore(list, existing);
   return newLink;
}


/// Insert a new item into a list before an existing entry (after NULL == at the head).
UtlLink* UtlLink::listAfter(UtlChain* list,
                            UtlChain* existing,
                            UtlContainable* newData
                            )
{
   UtlLink* newLink;
   newLink       = get();
   newLink->data = newData;
   newLink->hash = newData->hash();
   newLink->UtlChain::listAfter(list, existing);

   return newLink;
}

UtlContainable* UtlLink::detachFrom(UtlChain* list)
{
   UtlContainable* theData;

   theData = data;
   data = NULL;
   hash = 0;
   detachFromList(list);
   release();

   return theData;
}

/*
 * This is for use in UtlContainer only.
 * get a UtlLink for internal iterator list use.
 */
UtlLink* UtlLink::get()
{
   return static_cast<UtlLink*>(spLinkPool->get());
}

/// Return a UtlLink to the pool.
void UtlLink::release()
{
   // Clear the pointer to the subordinate object, to ensure that it doesn't
   // appear to be pointed-to.
   data = NULL;
   // Clear the hash, in case it has the same value (bit-wise) as "data".
   hash = 0;

   spLinkPool->release(this);
}


size_t UtlLink::totalAllocated()
{
   return spLinkPool->totalAllocated();
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

void UtlLink::allocate(size_t    blocksize, ///< number of instances to allocate
                       UtlChain* blockList, ///< list header for first instance
                       UtlChain* pool       ///< list header for others
                       )
{
   UtlLink* newBlock = new UtlLink[blocksize];
   assert(newBlock);

   // The first UtlChain is consumed to chain the list of blocks
   //     so that the destructor can free them.
   newBlock->UtlChain::listBefore(blockList, NULL);

   // chain the rest of the new UtlLinks onto the mLinkPool
   for (size_t i = 1; i < blocksize; i++)
   {
      newBlock[i].UtlChain::listBefore(pool, NULL);
   }
}


// The pool of available UtlPairs
UtlChainPool* UtlPair::spPairPool = new UtlChainPool(UtlPair::allocate, UTLLINK_BLOCK_SIZE);

void UtlPair::allocate(size_t    blocksize, ///< number of instances to allocate
                       UtlChain* blockList, ///< list header for first instance
                       UtlChain* pool       ///< list header for others
                       )
{
   UtlPair* newBlock = new UtlPair[blocksize];
   assert(newBlock);

   // The first UtlChain is consumed to chain the list of blocks
   //     so that the destructor can free them.
   newBlock->UtlChain::listBefore(blockList, NULL);

   // chain the rest of the new UtlLinks onto the mLinkPool
   for (size_t i = 1; i < blocksize; i++)
   {
      newBlock[i].UtlChain::listBefore(pool, NULL);
   }
}

UtlPair* UtlPair::get()
{
   return static_cast<UtlPair*>(spPairPool->get());
}

void UtlPair::release()
{
   // Clear the pointer to the subordinate object, to ensure that it doesn't
   // appear to be pointed-to.
   value = NULL;

   spPairPool->release(this);
}
