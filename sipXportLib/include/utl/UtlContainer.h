//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _UtlContainer_h_
#define _UtlContainer_h_

// SYSTEM INCLUDES
#include <stdlib.h>

// APPLICATION INCLUDES
#include "utl/UtlDefs.h"
#include "utl/UtlLink.h"
#include "utl/UtlContainable.h"
#include "utl/UtlIterator.h"
#include "os/OsBSem.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * UtlContainer defines and abstract container designed to hold UtlContainable
 * derived objects.
 */
class UtlContainer : public UtlContainable
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */

    /**
     * Default Constructor
     */
    UtlContainer();

    /**
     * Destructor
     */
    virtual ~UtlContainer();

/* ============================ MANIPULATORS ============================== */

    /**
     * Inserts the designated containable object into the list
     *
     * @return the object if successful, otherwise null
     */
    virtual UtlContainable* insert(UtlContainable* obj) = 0;

    /**
     * Removes the designated objects from the list and frees the object
     * by calling delete.
     */
    virtual UtlBoolean destroy(const UtlContainable*) = 0;

    /**
     * Removes all elements from the container and deletes each one.
     */
    virtual void destroyAll() = 0;

    /**
     * Removes the designated object by reference
     * (as opposed to searching for an equality match).
     *
     * @return the object if successful, otherwise null
     */
    virtual UtlContainable* removeReference(const UtlContainable* object) = 0;

    /**
     * Removes all elements from the container without freeing the objects.
     */
    virtual void removeAll() = 0;

/* ============================ ACCESSORS ================================= */

    /**
     * Find the designated value within the container
     */
    virtual UtlContainable* find(const UtlContainable*) const = 0;


    /**
     * Calculate a unique hash code for this object.  If the equals
     * operator returns true for another object, then both of those
     * objects must return the same hashcode.
     */
    virtual unsigned hash() const;

    /**
     * Get the ContainableType for a UtlContainable derived class.
     */
    virtual UtlContainableType getContainableType() const;

    static const UtlContainableType TYPE;    /** < Class type used for runtime checking */

/* ============================ INQUIRY =================================== */

    /**
     * Determine if the container is empty.
     */
    virtual UtlBoolean isEmpty() const = 0;

    /**
     * Determine if the container includes the designated object.
     */
    virtual UtlBoolean contains(const UtlContainable *)  const = 0;

    /**
     * Determine the number of elements within the container.
     */
    virtual size_t entries() const = 0;

    /**
     * Compare the this object to another like-objects.  Results for
     * designating a non-like object are undefined.
     *
     * @returns 0 if equal, < 0 if less then and >0 if greater.
     */
    virtual int compareTo(const UtlContainable* otherObject) const;

    /// Lock the linkage between containers and iterators
    static void acquireIteratorConnectionLock();
    /**<
     * This must be called by any code that will take both the
     * mContainerRefLock in an iterator and the mContainerLock in a
     * container.  It can be released as soon as both those locks are
     * acquired, and should be, since most operations on any iterator
     * will need to take it briefly.
     */

    /// Unlock the linkage between containers and iterators
    static void releaseIteratorConnectionLock();

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    friend class UtlIterator;

    /// Add an iterator to the list to be notified of changes to this container.
    void addIterator(UtlIterator* newIterator ///< to be notified of container changes
                     ) const; // this const is a lie, but not a detectable one...
    /**<
     * <strong>
     * The caller must be holding this->mContainerLock and newIterator->mContainerRefLock
     * </strong>
     *
     * This also sets the mpMyContainer pointer in newIterator.
     */

    /// Called from iterator destructor to prevent further notices.
    void removeIterator(UtlIterator* existingIterator ///< iterator to remove from notice list
                        ) const;
    /**<
     * <strong>
     *   The caller must be holding both this->mContainerLock and
     *   existingIterator->mContainerRefLock; see also acquireIteratorConnectionLock.
     * </strong>
     *
     * Remove the existingIterator from the list to be called for
     * changes to this UtlContainer.
     */

    /// Call the invalidate method on all iterators
    void invalidateIterators();
    /**<
     * This is for use in subclasses that have other state that must
     * be cleaned up.
     *
     * :NOTE: the caller must be holding the iterator list lock;
     *  see iteratorListLock
     */

    /// Must be taken when making any change to container state
    mutable OsBSem mContainerLock;


    /**
     * mpIterator list is the list of existing UtlIterator objects
     *   constructed using this UtlContainer
     *
     * This is used to invoke methods on each UtlIterator when changes are made to the UtlContainer
     *   ->remove when an element is about to be removed from the UtlContainer,
     *   ->invalidate when this UtlContainer is being deleted
     * see sIteratorConnectionLock
     *
     * mIteratorList must be mutable because we can create an iterator
     * on a const UtlContainer, and that iterator must be added to
     * mIteratorList as other references to the UtlContainer might change it.
     */
    mutable UtlChain mIteratorList;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    /// This lock prevent container/iterator deadlocks
    static OsBSem* spIteratorConnectionLock;
    /**<
     * UtlContainer/UtlIterator locking strategy
     *
     * The mpMyContainer pointer in a UtlIterator is protected by
     * the UtlIterator::mContainerRefLock.  That lock must be held to
     * write mpMyContainer, and while an iterator method is accessing
     * its container, *mpMyContainer.
     *
     * All other member variables of any UtlContainer, and all other
     * member variables of any UtlIterator are protected by the
     * UtlContainer::mContainerLock.
     *
     * A UtlIterator must always take both, because it has to lock the
     * mpMyContainer value to find its UtlContainer, and then lock the
     * mContainerLock before looking at the  UtlContainer state.
     *
     * A UtlContainer can usually take just the mContainerLock, but
     * when it is adding or removing a UtlIterator, it must lock the iterator's
     * mContainerRefLock as well so that it can change the mpMyContainer value.
     *
     * To prevent deadlocks, the sIteratorConnectionLock must be taken
     * before either lock when both are going to be needed.  It can
     * (and should) be released as soon as both locks are taken.  It does
     * not need to be held through the entire operation - once both
     * individual locks are taken, everything is safe and there can be no
     * deadlock.  Since holding it will block any operation on any UtlIterator,
     * it should be released as early as possible.
     *
     * Thus, the common sequences of operations are:
     *
     * Iterator operations:
     *
     *   UtlContainer::acquireIteratorConnectionLock();
     *   OsLock take(mContainerRefLock);
     *
     *   UtlList* myList = dynamic_cast<UtlList*>(mpMyContainer);
     *   if (myList != NULL)
     *   {
     *      OsLock take(myList->mContainerLock);
     *      UtlContainer::releaseIteratorConnectionLock(); // as soon as both locks are taken
     *
     *      // ... whatever the method does ...
     *   }
     *   else
     *   {
     *      UtlContainer::releaseIteratorConnectionLock();
     *   }
     *
     * Container operations (that does not affect mpMyContainer in any iterator):
     *
     *   {
     *      OsLock take (mContainerLock);
     *
     *      ... whatever the method does ...
     *   }
     *
     * Container destructor (must take sIteratorConnectionLock to disconnect iterators):
     *
     *   UtlContainer::acquireIteratorConnectionLock();
     *   OsLock take(mContainerRefLock);
     *
     *   UtlList* myList = dynamic_cast<UtlList*>(mpMyContainer);
     *   if (myList != NULL)
     *   {
     *      OsLock take(myList->mContainerLock);
     *      UtlContainer::releaseIteratorConnectionLock(); // as soon as both locks are taken
     *
     *      myList->removeIterator(this);
     *      mpMyContainer = NULL;
     *   }
     *   else
     *   {
     *      UtlContainer::releaseIteratorConnectionLock();
     *   }
     */

    /**
     * There is no copy constructor
     */
    UtlContainer(const UtlContainer& copy );

    /**
     * There is no assignment operator
     */
    UtlContainer& operator=(const UtlContainer& copy );

};

/* ============================ INLINE METHODS ============================ */

#endif    // _UtlContainer_h_
