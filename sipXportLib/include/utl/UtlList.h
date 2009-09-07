//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _UtlList_h_
#define _UtlList_h_

//#define GLIST_SANITY_TEST
#ifdef LIST_SANITY_TEST
# include "assert.h"
#endif

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlDefs.h"
#include "utl/UtlLink.h"
#include "utl/UtlContainer.h"

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
 * UtlList is an abstract base class for list classes designed to contain any number
 * of objects derived from the UtlContainable class.  Like any UtlContainer, a
 * UtlList may contain objects of different UtlContainableType
 * (e.g. UtlInts and UtlVoidPtrs), however, sorting and comparison behavior
 * may be non-obvious or undefined, so this is not recommended.
 *
 * Most list accessors and inquiry methods are performed by equality as
 * opposed to by reference.  That is, the comparisons between UtlContainable
 * objects are made using the UtlContainable::isEqual or UtlContainable::compareTo
 * methods, so for example, two different UtlInt* values (having different pointer
 * values) would compare as equal if they both contained the same integer value.
 *
 * Some methods are concerned with references; these compare the actual UtlContainable*
 * pointer values, so for example list.containsReference(obj) call will search
 * for a pointer match with each UtlContainable* on the list, matching only when
 * the value of the 'obj' pointer is found.
 *
 * @see UtlContainable for the methods that must be implemented by a class for its
 * objects to be stored in any UtlContainer.
 *
 * Like other UtlContainer classes, UtlList is itself a UtlContainable, so one can have
 * lists of lists and other complex structures.
 */
class UtlList : public UtlContainer, public UtlChain
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

// this base class cannot be instantiated directly - the constructor is protected

// Destructor
    virtual ~UtlList();

/* ============================ MANIPULATORS ============================== */

    /**
     * Removes and returns the first item in the list (pop).
     *
     * @return the first object if successful, otherwise null
     */
    UtlContainable* get() ;

    /**
     * Remove the designated object by reference
     * (as opposed to searching for an equality match).
     *
     * @return the object if successful, otherwise null
     */
    UtlContainable* removeReference(const UtlContainable* obj);

    /**
     * Remove the designated object by equality (as opposed to by reference).
     */
    virtual UtlContainable* remove(const UtlContainable* object) = 0;

    /**
     * Remove the object at (zero-based) location N.
     *
     * @return the object removed from the list, or NULL if there was no object at index N
     */
    UtlContainable* removeAt(const size_t N);

    /**
     * Removes the designated objects from the list and frees the object
     * by calling delete.
     */
    virtual UtlBoolean destroy(const UtlContainable*);

    /**
     * Removes all elements from the list and deletes each one.
     */
    void destroyAll();

    /**
     * Removes all elements from the list without freeing the objects.
     */
    void removeAll();

/* ============================ ACCESSORS ================================= */

    /**
     * Find the first occurence of the designated object by equality (as
     * opposed to by reference).
     */
    virtual UtlContainable* find(const UtlContainable*) const = 0;

    /**
     * Return the element at position N or null if N is out of bounds.
     */
    virtual UtlContainable* at(size_t N) const;

    /**
     * Return the first element (head) of the list
     */
    virtual UtlContainable* first() const ;

    /**
     * Return the last element (tail) of the list
     */
    virtual UtlContainable* last() const ;

/* ============================ INQUIRY =================================== */

    /**
     * Return the total number of elements within the container
     */
    virtual size_t entries() const;

    /**
     * Return true of the container is empty (entries() == 0), otherwise false.
     */
    virtual UtlBoolean isEmpty() const;

    /**
     * Return true if the container includes the designated object.  Each
     * element within the list is tested for equality against the designated
     * object using the equals() method.
     */
    virtual UtlBoolean contains(const UtlContainable* object) const;

    /**
     * Return true if the list contains the designated object reference.
     */
    virtual UtlBoolean containsReference(const UtlContainable *) const ;

    /**
     * Return the number of occurrences of the designated object
     */
    virtual size_t occurrencesOf(const UtlContainable* obj) const = 0;

    /**
     * Return the list position of the designated object or UTL_NOT_FOUND  if
     * not found.
     */
    virtual ssize_t index(const UtlContainable* obj) const = 0;


   /**
    * Get the ContainableType for the list as a contained object.
    */
   virtual UtlContainableType getContainableType() const;

   static UtlContainableType TYPE ;    /**< Class type used for runtime checking */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:


    /**
     * The UtlList constructor is protected - only subclasses may be instantiated
     */
    UtlList();

    friend class UtlListIterator;

    /**
     * notifyIteratorsOfRemove - called before removing any element in the collection
     */
    void notifyIteratorsOfRemove(UtlLink* element);

    /**
     * removeLink is used internally to manipulate the links.
     *
     * :NOTE: the caller must hold the mContainerLock
     *
     * This does not return a new value for the current list position;
     * this is because it will call the <some-list-iterator>::removing method on the
     * removed element, passing the new value.  This means the that current position
     * update is always done the same way no matter what routine did the removing.
     */
    virtual void removeLink(UtlLink* toBeRemoved);

#ifdef LIST_SANITY_TEST
#  define LIST_SANITY_CHECK { if (!sanityCheck()){ assert(FALSE); } }
    bool sanityCheck() const;
#else
#  define LIST_SANITY_CHECK /* sanityCheck() */
#endif

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

} ;

/* ============================ INLINE METHODS ============================ */

#endif    // _UtlList_h_
