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
#include "utl/UtlDateTime.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
UtlContainableType UtlDateTime::TYPE = "UtlDateTime" ;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor accepting an optional default value.
UtlDateTime::UtlDateTime(OsDateTime value)
{
    mTime = value ;
}


// Copy constructor



// Destructor
UtlDateTime::~UtlDateTime()
{
}

/* ============================ MANIPULATORS ============================== */

void UtlDateTime::setTime(const OsDateTime& value)
{
    mTime = value ;
}

/* ============================ ACCESSORS ================================= */

void UtlDateTime::getTime(OsDateTime& value) const
{
    value = mTime ;
}


unsigned int UtlDateTime::hash() const
{
   return (unsigned int)mTime.getSecsSinceEpoch() ;
}


UtlContainableType UtlDateTime::getContainableType() const
{
    return UtlDateTime::TYPE ;
}

/* ============================ INQUIRY =================================== */

int UtlDateTime::compareTo(UtlContainable const * inVal) const
{
   int result ;

   if (inVal->isInstanceOf(UtlDateTime::TYPE))
    {
        UtlDateTime* temp = (UtlDateTime*)inVal ;
        OsDateTime inTime;
        temp->getTime(inTime);
        time_t tempValue = inTime.getSecsSinceEpoch() ;
        result = tempValue - mTime.getSecsSinceEpoch() ;
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
