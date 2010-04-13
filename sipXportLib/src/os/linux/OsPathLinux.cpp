//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "os/OsFS.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsPathLinux::OsPathLinux()
{
}

// Make one from a char string
OsPathLinux::OsPathLinux(const char *pathname) :
   OsPathBase(pathname)
{
}
// Make one from a UtlStringchar string
OsPathLinux::OsPathLinux(const UtlString &pathname) :
   OsPathBase(pathname)
{
}

// Copy constructor
OsPathLinux::OsPathLinux(const OsPathLinux& rOsPath)
{
    *this = rOsPath.data();
}

// Destructor
OsPathLinux::~OsPathLinux()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
OsPathLinux&
OsPathLinux::operator=(const OsPathLinux& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   UtlString::operator=(rhs.data());

   return *this;
}

/* ============================ ACCESSORS ================================= */



/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
