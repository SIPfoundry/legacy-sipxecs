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
#include <utl/UtlRegex.h>

// APPLICATION INCLUDES
#include "os/OsFS.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor


OsFileIteratorBase::OsFileIteratorBase(const OsPathBase& pathname) :
   mUserSpecifiedPath(pathname),
   mFilterExp(NULL),
   mSearchHandle(INVALID_HANDLE)
{
}

OsFileIteratorBase::OsFileIteratorBase() :
   mFilterExp(NULL),
   mSearchHandle(INVALID_HANDLE)
{
}


void OsFileIteratorBase::Release()
{
    if (mFilterExp)
    {
        delete mFilterExp;
        mFilterExp = NULL;
    }
}

// Destructor
OsFileIteratorBase::~OsFileIteratorBase()
{
    Release();
}

/* ============================ MANIPULATORS ============================== */


/* ============================ ACCESSORS ================================= */

OsStatus OsFileIteratorBase::findFirst(OsPathBase& entry, const char* filterExp,
                                       OsFileType filetype)
{
    OsFileType entryType;
    UtlBoolean bDone = FALSE;
    UtlString name;
    OsStatus stat = OS_FILE_NOT_FOUND;
    UtlBoolean bFirstTime = TRUE;

    UtlBoolean bFoundOne = false;
    mMatchAttrib = filetype;
    mFileCount = 0;

    entry = "";

    Release();

    mFilterExp = new RegEx(filterExp);

    //add a sep on the end if there is not already one there
    if (   mUserSpecifiedPath.length()
        && *(mUserSpecifiedPath.data()+mUserSpecifiedPath.length()-1) != OsPathBase::separator
        )
    {
        mUserSpecifiedPath += OsPathBase::separator;
    }

    mFullSearchSpec = mUserSpecifiedPath;

    while (!bDone)
    {
        //returns OS_SUCCESS or OS_FILE_NOT_FOUND
        //because we may have found a file that is not the one we want, the next
        //search we do we should use NextEntry
        if (bFirstTime)
        {
            stat = getFirstEntryName(name, entryType);
        }
        else
        {
            stat = getNextEntryName(name, entryType);
        }
        //so we don't use FirstEntry next time
        bFirstTime = FALSE;

        if (stat == OS_SUCCESS)
        {
            if (entryType == mMatchAttrib || mMatchAttrib == ANY_FILE)
            {
                //now check if if meets our regexp criteria
                if (mFilterExp->Search(name.data()))
                {
                    entry = name;
                    stat = OS_SUCCESS;
                    bDone = TRUE;
                    bFoundOne = TRUE;
                    mFileCount++;
                }
            }
        }
        else
        {
            bDone = TRUE;
        }
    }

    if (!bFoundOne)
    {
        stat = OS_FILE_NOT_FOUND;
    }
    return stat;
}


OsStatus OsFileIteratorBase::findNext(OsPathBase& entry)
{
    OsFileType entryType;
    UtlBoolean bDone = FALSE;
    OsStatus stat = OS_FILE_NOT_FOUND;
    UtlString name;
    UtlBoolean bFoundOne = false;

    entry = "";

    while (!bDone)
    {
        //returns OS_SUCCESS or OS_FILE_NOT_FOUND
        stat = getNextEntryName(name,entryType);

        if (stat == OS_SUCCESS)
        {

            if (entryType == mMatchAttrib || mMatchAttrib == ANY_FILE)
            {
                //now check if if meets our regexp criteria
                if (mFilterExp->Search(name.data()))
                {
                    //bDone so we get out of the while (!bDone)
                    bDone = TRUE;
                    //name to return to user
                    entry = name;
                    stat = OS_SUCCESS;
                    mFileCount++;
                    bFoundOne = true;
                }
            }
        }
        else
        {
            bDone = TRUE;
        }
    }

    if (!bFoundOne)
    {
        stat = OS_FILE_NOT_FOUND;
    }
    return stat;
}


/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
OsStatus OsFileIteratorBase::getNextEntryName(UtlString &rName, OsFileType &rFileType)
{
    return OS_INVALID;
}

OsStatus OsFileIteratorBase::getFirstEntryName(UtlString &rName, OsFileType &rFileType)
{
    return OS_INVALID;
}

/* ============================ FUNCTIONS ================================= */
