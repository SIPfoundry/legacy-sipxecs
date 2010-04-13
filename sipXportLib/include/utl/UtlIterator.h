//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _UtlIterator_h_
#define _UtlIterator_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlDefs.h"
#include "os/OsBSem.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class UtlContainable;
class UtlContainer;

/**
 * UltIterator defines an abstract Iterator for walking through the elements
 * of UtlContainer derived class.
 *
 * <p>
 * Example Code:
 * <pre>
 *    // Create an iterator that walks through the elements of myContentSource.
 *    FooIterator itor(myContentSource);
 *    MyObject* pObj;
 *    // Fetch a pointer to each element of myContentSource into pObj.
 *    while ((pObj = itor()))
 *    {
 *       // Do something to *pObj.
 *    }
 *    // Reset itor to its initial state, so itor() starts walking through the
 *    // elements of myContentSource all over again.
 *    itor.reset();
 *    while ((pObj = itor()))
 *    {
 *       // Do something else to *pObj.
 *    }
 * </pre>
 * (The extra parentheses in the while clauses are to mark that that
 * operation is an assignment, not a comparison.)
 */
class UtlIterator
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   UtlIterator(const UtlContainer& container);

    /**
     * Destructor
     */
    virtual ~UtlIterator() = 0;

/* ============================ MANIPULATORS ============================== */


    /// Return the next element.
    virtual UtlContainable* operator()() = 0 ;

    /// Reset the iterator cursor so that it will again return all elements in the container.
    virtual void reset() = 0 ;

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    friend class UtlContainer;

    /******************************************************************
     * @par Removing Method - Variable Signature.
     *
     * All iterators must have a 'removing' method so that they can be notified
     * by the container when an element is removed.
     *
     * There is no prototype for it here because the signature differs depending
     * on the container type.
     *
     * removing is called by the UtlContainer when an element is about to be
     * removed from the container.  The iterator must ensure that the removed
     * element is not returned by any subsequent call.
     * if element != NULL, it points to the element to be removed.
     * if element == NULL, means all elements to be removed.
     */
    // virtual void removing( ... type depends on the class of the iterator... ) = 0;


    void addToContainer(const UtlContainer* container);

    /**
     * invalidate is called by the UtlContainer from its destructor.
     * It disconnects the iterator from its container object (sets
     * mpContainerRef to NULL).
     * Any subsequent invocation of this iterator (other than its
     * destructor) must return an error.
     *
     * :NOTE: Both the sIiteratorListLock and the container lock must be held by the caller.
     */
    virtual void invalidate();

    /**
     * The mContainerRefLock must be held whenever the mpMyContainer value
     * is being used or modified.  If the mpIteratorListLock in the container
     * is also held, then the mpIteratorListLock must be taken first.
     */
    mutable OsBSem mContainerRefLock;

    /**
     *  The container which this iterator indexes into.
     */
    UtlContainer* mpMyContainer;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    /**
     * There is no copy constructor
     */
    UtlIterator(const UtlIterator& noCopyAllowed);

    /**
     * There is no assignment operator
     */
    UtlIterator& operator=(const UtlIterator& noCopyAllowed);

} ;

/* ============================ INLINE METHODS ============================ */

#endif  // _UtlIterator_h_
