// $Id$
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES


// APPLICATION INCLUDES
#include "utl/UtlBool.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
UtlContainableType UtlBool::TYPE = "UtlBool" ;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor accepting an optional default value.
UtlBool::UtlBool(bool value)
{
    mValue = value ;
}


// Copy constructor



// Destructor
UtlBool::~UtlBool()
{
}

/* ============================ MANIPULATORS ============================== */

void UtlBool::setValue(bool value)
{
    mValue = value ;
}

/* ============================ ACCESSORS ================================= */

bool UtlBool::getValue() const
{
    return mValue ;
}


unsigned UtlBool::hash() const
{
   return mValue ;
}


UtlContainableType UtlBool::getContainableType() const
{
    return UtlBool::TYPE ;
}

/* ============================ INQUIRY =================================== */

int UtlBool::compareTo(UtlContainable const * inVal) const
{
   int result ;

   if (inVal->isInstanceOf(UtlBool::TYPE))
    {
        UtlBool* temp = (UtlBool*)inVal ;
        bool inBool = temp -> getValue() ;
        if (inBool == mValue)
        {
            result = 0 ;
        }
        else
        {
            result = 1 ;
        }
    }
    else
    {
        result = 1 ;
    }

    return result ;
}



/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
