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
#include <io.h>

// APPLICATION INCLUDES
#include "os/OsFS.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor


OsFileIteratorWnt::OsFileIteratorWnt(const OsPathWnt& pathname) :
OsFileIteratorBase(pathname)
{
}

// Override this function for releasing mSearchHandle.
void OsFileIteratorWnt::Release()
{
    OsFileIteratorBase::Release();

    if (mSearchHandle != INVALID_HANDLE)
    {
        _findclose(mSearchHandle);
        mSearchHandle = INVALID_HANDLE;
    }

}

// Destructor
OsFileIteratorWnt::~OsFileIteratorWnt()
{
    Release();
}

/* ============================ MANIPULATORS ============================== */


/* ============================ ACCESSORS ================================= */
/*

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
OsFileIteratorWnt::OsFileIteratorWnt()
{
}



OsStatus OsFileIteratorWnt::getFirstEntryName(UtlString &name, OsFileType &type)
{
    OsStatus stat = OS_FILE_NOT_FOUND;

    name = "";
    UtlString fullPath = mFullSearchSpec;
    fullPath += "*";

    _finddata_t FileInfo;

    mSearchHandle = _findfirst(fullPath.data(),&FileInfo);
    if (mSearchHandle != -1)
    {
        name = FileInfo.name;
        stat = OS_SUCCESS;
    }
    else
        stat = OS_FILE_NOT_FOUND;

    if (FileInfo.attrib & _A_SUBDIR)
        type = DIRECTORIES;
    else
        type = FILES;

    return stat;
}



OsStatus OsFileIteratorWnt::getNextEntryName(UtlString &name, OsFileType &type)
{
    OsStatus stat = OS_FILE_NOT_FOUND;

    name = "";

    _finddata_t FileInfo;
    int retcode = _findnext(mSearchHandle,&FileInfo);
    if (retcode != -1)
    {
        stat = OS_SUCCESS;
        name = FileInfo.name;
    }

    if (FileInfo.attrib & _A_SUBDIR)
        type = DIRECTORIES;
    else
        type = FILES;


    return stat;
}

/* ============================ FUNCTIONS ================================= */
