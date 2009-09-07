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
#include <stdio.h>
#include <direct.h>
#include <sys/types.h>
#include <sys/stat.h>

// APPLICATION INCLUDES
#include "os/OsFS.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor

OsDirWnt::OsDirWnt(const char* pathname) :
OsDirBase(pathname)
{
}

OsDirWnt::OsDirWnt(const OsPathWnt& pathname) :
OsDirBase(pathname.data())
{
}

// Copy constructor
OsDirWnt::OsDirWnt(const OsDirWnt& rOsDirWnt) :
OsDirBase(rOsDirWnt.mDirName.data())
{
}

// Destructor
OsDirWnt::~OsDirWnt()
{
}

/* ============================ MANIPULATORS ============================== */
// Assignment operator
OsDirWnt&
OsDirWnt::operator=(const OsDirWnt& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}


OsStatus OsDirWnt::create() const
{
    OsStatus ret = OS_INVALID;
    OsPathBase path;

    if (mDirName.getNativePath(path) == OS_SUCCESS)
    {
        int err = _mkdir((const char *)path.data());
        if (err != -1)
        {
            ret = OS_SUCCESS;
        }
    }

    return ret;
}


OsStatus OsDirWnt::rename(const char* name)
{
    OsStatus ret = OS_INVALID;
    OsPathBase path;
    if (mDirName.getNativePath(path) == OS_SUCCESS)
    {
        int err = ::rename(path.data(),name);
        if (err != -1)
        {
            ret = OS_SUCCESS;

            //make this object point to new path
            mDirName = name;

        }
    }

    return ret;
}


/* ============================ ACCESSORS ================================= */




/* ============================ INQUIRY =================================== */

UtlBoolean OsDirWnt::exists()
{
    UtlBoolean stat = FALSE;

    OsFileInfoWnt info;
    OsStatus retval = getFileInfo(info);
    if (retval == OS_SUCCESS)
        stat = TRUE;

    return stat;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
