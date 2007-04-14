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
#include <string.h>
#include <limits.h>

// APPLICATION INCLUDES
#include "utl/UtlInt.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
UtlContainableType UtlInt::TYPE = "UtlInt" ;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor accepting an optional default value.
UtlInt::UtlInt(int value)
{
    mValue = value ;
} 


// Copy constructor



// Destructor
UtlInt::~UtlInt()
{
}

/* ============================ OPERATORS ============================== */

// Prefix increment operator
UtlInt& UtlInt::operator++() {
    mValue++;
    return *this;
}

// Postfix increment operator
UtlInt UtlInt::operator++(int) {
    UtlInt temp = *this;
    ++*this;
    return temp;
}

// Prefix decrement operator
UtlInt& UtlInt::operator--() {
    mValue--;
    return *this;
}

// Postfix decrement operator
UtlInt UtlInt::operator--(int) {
    UtlInt temp = *this;
    --*this;
    return temp;
}

/* ============================ MANIPULATORS ============================== */

int UtlInt::setValue(int iValue)
{
    int iOldValue = mValue ;
    mValue = iValue ;

    return iOldValue ;
}

/* ============================ ACCESSORS ================================= */

int UtlInt::getValue() const 
{
    return mValue ; 
}


unsigned UtlInt::hash() const
{
   return mValue ; 
}


UtlContainableType UtlInt::getContainableType() const
{
    return UtlInt::TYPE ;
}

/* ============================ INQUIRY =================================== */

int UtlInt::compareTo(UtlContainable const * inVal) const
{
   int result ; 
   
   if (inVal->isInstanceOf(UtlInt::TYPE))
    {
        UtlInt* temp = (UtlInt*)inVal ; 
        int inInt = temp -> getValue() ; 
        result = mValue - inInt ; 
    }
    else
    {
        result = INT_MAX ; 
    }

    return result ;
}


UtlBoolean UtlInt::isEqual(UtlContainable const * inVal) const
{
    return (compareTo(inVal) == 0) ; 
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
