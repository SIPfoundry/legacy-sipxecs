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
#include <assert.h>

// APPLICATION INCLUDES
#include "os/OsExcept.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsExcept::OsExcept(const int majorCode, const int minorCode,
                                                 const UtlString& rText, const UtlString& rContext) :
   mMajorCode(majorCode),
        mMinorCode(minorCode),
        mpText(new UtlString(rText)),
        mpContext(new UtlString(rContext))
{
        // All of the work is done by the initializers
}

// Copy constructor
OsExcept::OsExcept(const OsExcept& rOsExcept) :
   mMajorCode(rOsExcept.mMajorCode),
        mMinorCode(rOsExcept.mMinorCode),
        mpText(new UtlString(*rOsExcept.mpText)),
        mpContext(new UtlString(*rOsExcept.mpContext))
{
        // All of the work is done by the initializers
}

// Destructor
OsExcept::~OsExcept()
{
   delete mpText;
   mpText = 0;

   delete mpContext;
   mpContext = 0;
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
OsExcept&
OsExcept::operator=(const OsExcept& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   mMajorCode = rhs.mMajorCode;
   mMinorCode = rhs.mMinorCode;

        delete mpText;
        mpText     = new UtlString(*rhs.mpText);

        delete mpContext;
        mpContext  = new UtlString(*rhs.mpContext);

   return *this;
}

// Set major exception code
void OsExcept::setMajorCode(const int majorCode)
{
   mMajorCode = majorCode;
}

// Set minor exception code
void OsExcept::setMinorCode(const int minorCode)
{
   mMinorCode = minorCode;
}

// Set exception context
void OsExcept::setContext(const UtlString& rContext)
{
   *mpContext = rContext;
}

// Set exception text
void OsExcept::setText(const UtlString& rText)
{
   *mpText = rText;
}

/* ============================ ACCESSORS ================================= */

// Get major exception code
int OsExcept::getMajorCode(void) const
{
   return mMajorCode;
}

// Get minor exception code
int OsExcept::getMinorCode(void) const
{
   return mMinorCode;
}

// Get exception context
const UtlString& OsExcept::getContext(void) const
{
   return *mpContext;
}

// Get exception text
const UtlString& OsExcept::getText(void) const
{
   return *mpText;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
