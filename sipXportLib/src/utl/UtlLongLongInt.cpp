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
#include <stdlib.h>

// APPLICATION INCLUDES
#include "utl/UtlLongLongInt.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
UtlContainableType UtlLongLongInt::TYPE = "UtlLongLongInt" ;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor accepting an optional default value.
UtlLongLongInt::UtlLongLongInt(Int64 value)
{
    mValue = value ;
}


// Copy constructor



// Destructor
UtlLongLongInt::~UtlLongLongInt()
{
}

/* ============================ OPERATORS ============================== */

// Prefix increment operator
UtlLongLongInt& UtlLongLongInt::operator++() {
    mValue++;
    return *this;
}

// Postfix increment operator
UtlLongLongInt UtlLongLongInt::operator++(int) {
    UtlLongLongInt temp = *this;
    ++*this;
    return temp;
}

// Prefix decrement operator
UtlLongLongInt& UtlLongLongInt::operator--() {
    mValue--;
    return *this;
}

// Postfix decrement operator
UtlLongLongInt UtlLongLongInt::operator--(int) {
    UtlLongLongInt temp = *this;
    --*this;
    return temp;
}

/* ============================ MANIPULATORS ============================== */

Int64 UtlLongLongInt::setValue(Int64 iValue)
{
    Int64 iOldValue = mValue ;
    mValue = iValue ;

    return iOldValue ;
}

/* ============================ ACCESSORS ================================= */

Int64 UtlLongLongInt::getValue() const
{
    return mValue ;
}


unsigned UtlLongLongInt::hash() const
{
   return mValue ;
}


UtlContainableType UtlLongLongInt::getContainableType() const
{
    return UtlLongLongInt::TYPE ;
}

/* ============================ INQUIRY =================================== */

int UtlLongLongInt::compareTo(UtlContainable const * inVal) const
{
   int result ;

   if (inVal->isInstanceOf(UtlLongLongInt::TYPE))
    {
        UtlLongLongInt* temp = (UtlLongLongInt*)inVal ;
        Int64 inIntll = temp -> getValue() ;
        if (mValue > inIntll) {
        	result = 1 ;
        }
        else if (mValue == inIntll) {
        	result = 0 ;
        }
        else {
        	// mValue < inIntll
        	result = -1 ;
        }
    }
    else
    {
    	// The result for a non-like object is undefined except that we must
    	// declare that the two objects are not equal
    	result = INT_MAX ;
    }

    return result ;
}


UtlBoolean UtlLongLongInt::isEqual(UtlContainable const * inVal) const
{
    return (compareTo(inVal) == 0) ;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
