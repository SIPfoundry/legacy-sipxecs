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
#include "os/OsFS.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsFileSystemWnt::OsFileSystemWnt()
{
}

// Copy constructor
OsFileSystemWnt::OsFileSystemWnt(const OsFileSystemWnt& rOsFileSystem)
{
}

// Destructor
OsFileSystemWnt::~OsFileSystemWnt()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
OsFileSystemWnt&
OsFileSystemWnt::operator=(const OsFileSystemWnt& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

//: Returns OS_SUCCESS if file copied ok.
//  see OsFile for more return codes
OsStatus OsFileSystemWnt::copy(const OsPathWnt& source, const OsPathWnt& dest)
{
    OsFileWnt sourcefile(source);
    return sourcefile.copy(dest);
}


//: Renames the directory or file specified by path
//  (no path may be specifed on 2nd parameter)
OsStatus OsFileSystemWnt::rename(const OsPathWnt& source, const OsPathWnt& dest)
{
    OsStatus stat = OS_SUCCESS;

    OsDirWnt dir(source);
    return dir.rename(dest);
}


//: Change the current working directory to the specified location
OsStatus OsFileSystemWnt::change(const OsPathWnt& path)
{
    OsStatus stat = OS_INVALID;

    if (chdir(path) != -1)
        stat = OS_SUCCESS;

    return stat;
}

//: Creates the specified directory
//  Fails if a file by the same name exist in the parent directory
OsStatus OsFileSystemWnt::createDir(const OsPathWnt& path)
{
    OsDirWnt dir(path);
    return dir.create();
}

//: returns the current working directory for the process
//
OsStatus OsFileSystemWnt::getWorkingDirectory(OsPathWnt& path)
{
    char buf[256];
    OsStatus stat = OS_INVALID;

    if (getcwd(buf,256))
    {
        stat = OS_SUCCESS;
        path=buf;
    }

    return stat;
}


/* ============================ ACCESSORS ================================= */

OsStatus OsFileSystemWnt::getFileInfo(OsPathWnt& filespec, OsFileInfoWnt& fileInfo)
{
    OsDirWnt dir(filespec);
    return dir.getFileInfo(fileInfo);

}


/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
