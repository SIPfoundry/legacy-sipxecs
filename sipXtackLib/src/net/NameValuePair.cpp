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
#include <ctype.h>
#include <stdio.h>

// APPLICATION INCLUDES
#include "net/NameValuePair.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
int NameValuePair::count = 0;
OsMutex   NameValuePair::mCountLock(OsMutex::Q_PRIORITY);

int getNVCount()
{
        return NameValuePair::count;
}
/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
NameValuePair::NameValuePair(const char* name, const char* value) :
        UtlString(name)
{
   valueString = NULL;
   setValue(value);

#ifdef TEST_ACCOUNT
        mCountLock.acquire();
   count++;
        mCountLock.release();
#endif
}

// Copy constructor
NameValuePair::NameValuePair(const NameValuePair& rNameValuePair) :
UtlString(rNameValuePair),
valueString( NULL )
{
    setValue(rNameValuePair.valueString);
}

// Destructor
NameValuePair::~NameValuePair()
{
   if(valueString)
   {
                delete[] valueString;
                valueString = 0;
   }

#ifdef TEST_ACCOUNT
        mCountLock.acquire();
   count--;
        mCountLock.release();
#endif
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
NameValuePair&
NameValuePair::operator=(const NameValuePair& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   ((UtlString&) *this) = rhs.data();
   setValue(rhs.valueString);

   return *this;
}

/* ============================ ACCESSORS ================================= */
const char* NameValuePair::getValue()
{
        return(valueString);
}

void NameValuePair::setValue(const char* newValue)
{
        if(newValue)
        {
                size_t len = strlen(newValue);

                if(valueString && len > strlen(valueString))
                {
                        delete[] valueString;
                        valueString = new char[len + 1];
                }
                else if (!valueString)
                {
                         valueString = new char[len + 1];
                }

                memcpy(valueString, newValue, len + 1 );
        }
        else if(valueString)
        {
                delete[] valueString;
                valueString = 0;
        }
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
