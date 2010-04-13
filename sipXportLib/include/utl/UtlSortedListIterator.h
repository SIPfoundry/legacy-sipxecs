//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _UtlSortedListIterator_h_
#define _UtlSortedListIterator_h_

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
class UtlContainable;
class UtlSortedList;

/**
 * UtlSortedListIterator allows developers to iterator (walks through) an
 * UtlSortedList.
 *
 * @see UtlIterator
 * @see UtlSortedList
 */
class UtlSortedListIterator : public UtlListIterator
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

    /**
     * Constructor accepting a source UtlSortedList
     */
    UtlSortedListIterator(const UtlSortedList& list);



/* ============================ MANIPULATORS ============================== */


    /**
     * Find the designated object, and reset the iterator so that it is the current position.
     *
     * @return The  element or NULL if no more elements are available.
     */
    virtual UtlContainable* findNext(const UtlContainable* objectToFind);


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    friend class UtlSortedList;


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

/* ============================ INLINE METHODS ============================ */

#endif    // _UtlSortedListIterator_h_
