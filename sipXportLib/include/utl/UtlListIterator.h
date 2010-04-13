//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _UtlListIterator_h_
#define _UtlListIterator_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlDefs.h"
#include "utl/UtlLink.h"
#include "utl/UtlIterator.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class UtlContainable ;
class UtlList ;

/**
 * UtlListIterator allows developers to iterator (walks through) an UtlList.
 *
 * @see UtlIterator
 * @see UtlList
 */
class UtlListIterator : public UtlIterator
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

    /**
     * Constructor accepting a source UtlList
     */
    UtlListIterator(const UtlList& list) ;

    /**
     * Destructor
     */
    virtual ~UtlListIterator();

/* ============================ MANIPULATORS ============================== */

    /**
     * Return the next element.
     *
     * @return The next element or NULL if no more elements are available.
     */
    virtual UtlContainable* operator()() ;

    /**
     * Reset the list by moving the iterator cursor to the location before the
     * first element.
     */
    virtual void reset() ;

    /**
     * Find the designated object, and reset the iterator so that it is the current position.
     *
     * @return The  element or NULL if no more elements are available.
     */
    virtual UtlContainable* findNext(const UtlContainable* objectToFind) = 0;

    /**
     * Move the iterator to the last element within the iterator.
     */
    virtual UtlContainable* toLast() ;

/* ============================ ACCESSORS ================================= */

    /**
     * return the current value .
     */
    UtlContainable* item() const;

/* ============================ INQUIRY =================================== */

    /**
     * Is the iterator positioned at the last element?
     */
    UtlBoolean atLast() const ;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    UtlLink* mpCurrentNode;

    friend class UtlList;

    /**
     * removing is called by the UtlList when an element is about to be
     * removed from the container.  The iterator must ensure that the element
     * for the removed node is not returned by any subsequent call.
     */
    virtual void removing(const UtlLink* node);


    static const UtlLink*  NOWHERE;
    static UtlLink const* OFF_LIST_END;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    static OsBSem sIteratorListLock;
} ;

/* ============================ INLINE METHODS ============================ */

#endif    // _UtlListIterator_h_
