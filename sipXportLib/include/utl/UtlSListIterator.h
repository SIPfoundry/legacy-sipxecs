//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _UtlSListIterator_h_
#define _UtlSListIterator_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlDefs.h"
#include "utl/UtlListIterator.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class UtlContainable ;
class UtlSList ;

/**
 * UtlSListIterator allows developers to iterator (walks through) an UtlSList.
 *
 * @see UtlIterator
 * @see UtlSList
 */
class UtlSListIterator : public UtlListIterator
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

    /**
     * Constructor accepting a source UtlSList
     */
    UtlSListIterator(const UtlSList& list);


    /**
     * Find the designated object, and reset the iterator so that it is the current position.
     *
     * @return The  element or NULL if no more elements are available.
     */
    virtual UtlContainable* findNext(const UtlContainable* objectToFind);

    /**
     * Take a look at the next element in the list without moving the position
     * of the iterator.
     *
     * @return The next element or NULL if already at the end.
     */
    virtual UtlContainable* peekAtNext(void);

    /**
     * Insert the designated element after the current iterator
     * position.
     */
    UtlContainable* insertAfterPoint(UtlContainable*) ;

/* ============================ ACCESSORS ================================= */


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    friend class UtlSList;


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

} ;

/* ============================ INLINE METHODS ============================ */

#endif    // _UtlSListIterator_h_
