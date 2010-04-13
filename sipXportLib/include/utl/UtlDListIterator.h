//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _UtlDListIterator_h_
#define _UtlDListIterator_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlDefs.h"
#include "utl/UtlSListIterator.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * UtlDListIterator allows developers to iterate (walks through) a UtlDList.
 *
 * @see UtlIterator
 * @see UtlDList
 */
class UtlDList;

class UtlDListIterator : public UtlSListIterator
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   /**
    * Constructor accepting a source UtlDList
    */
   UtlDListIterator(const UtlDList& list) ;


   /**
     * Destructor
     */
    virtual ~UtlDListIterator();

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    friend class UtlDList;



/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    UtlDListIterator();
};

/* ============================ INLINE METHODS ============================ */

#endif    // _UtlDListIterator_h_
