//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlDList.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const UtlContainableType UtlDList::TYPE = "UtlDList";

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
UtlDList::UtlDList()
    : UtlSList()
{
}


// Copy constructor


// Destructor
UtlDList::~UtlDList()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator


/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/**
 * Get the ContainableType for the list as a contained object.
 */
UtlContainableType UtlDList::getContainableType() const
{
   return UtlDList::TYPE;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
