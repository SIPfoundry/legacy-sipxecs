// $Id$
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>
#include <string.h>
#include <ctype.h>

// APPLICATION INCLUDES
#include "CategorizedString.h"
#include "utl/UtlString.h"
#include "os/OsDefs.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
const UtlContainableType CategorizedString::TYPE = "CategorizedString";

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor accepting an integer and a null terminated source string.
CategorizedString::CategorizedString(int priority, const char* szSource)
{
   mPriority = priority;
   mString = new char[strlen(szSource) + 1];
   strcpy(mString, szSource);
}

// Destructor
CategorizedString::~CategorizedString()
{
   delete[] mString;
}

/* ============================ MANIPULATORS ============================== */


/* ============================ ACCESSORS ================================= */

// Return a read-only pointer to the underlying data. 
const char* CategorizedString::data() const  
{
    return mString; 
}

// Returns a hash value for the object. The algorithm is copied from UtlString,
// which uses the algorithm in g_string_hash() in glib, but also adds the
// priority value.
unsigned CategorizedString::hash() const  
{
   const char* pHashData = mString;
   unsigned hashValue = 0;

   while (*pHashData != '\0')
   {
      hashValue = (hashValue << 5) - hashValue + *pHashData;
      pHashData++;
   }
   hashValue += mPriority;

   return hashValue;
}

// Get the ContainableType for a UtlContainable derived class.
UtlContainableType CategorizedString::getContainableType() const
{
    return CategorizedString::TYPE;
}

/* ============================ INQUIRY =================================== */

// Compare this object to another like object.
int CategorizedString::compareTo(UtlContainable const * compareContainable)
   const
{
   int compareFlag;

    if (compareContainable != NULL &&
        compareContainable->isInstanceOf(CategorizedString::TYPE) == TRUE)
    {
       CategorizedString* compareCategorizedString =
          (CategorizedString*) compareContainable;
       if (mPriority < compareCategorizedString->mPriority)
       {
          compareFlag = -1;
       }
       else if (mPriority > compareCategorizedString->mPriority)
       {
          compareFlag = 1;
       } else {
          compareFlag = strcmp(mString, compareCategorizedString->mString);
       }
    }
    else
    {
       compareFlag = -1;
    }

    return compareFlag;
}


// Test this object to another like object for equality. 
UtlBoolean CategorizedString::isEqual(UtlContainable const* compareContainable)
   const 
{
    return (compareTo(compareContainable) == 0);
}


/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ INLINE METHODS ============================ */
