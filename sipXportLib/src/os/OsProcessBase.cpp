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
#include <stdlib.h>

#ifdef HAVE_CONFIG_H
#  include "config.h"
#else
#  ifdef _VXWORKS
    #include <envLib.h>  //needed for putenv
#  endif
#endif

// APPLICATION INCLUDES
#include "os/OsFS.h"
#include "os/OsProcess.h"


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsProcessBase::OsProcessBase() :
mPID(-1), mParentPID(-1), mProcessName("")
{
}

// Destructor
OsProcessBase::~OsProcessBase()
{
}

/* ============================ MANIPULATORS ============================== */

OsStatus OsProcessBase::setEnv(UtlString &rKey, UtlString &rValue)
{
    OsStatus retval = OS_FAILED;

    //store the env variable
    mEnvList.set(rKey.data(),rValue.data());
    retval = OS_SUCCESS;

    return retval;
}

OsStatus OsProcessBase::unsetEnv(UtlString &rKey)
{
    OsStatus retval = OS_FAILED;

    //store the env variable
    if (mEnvList.remove(rKey) == OS_SUCCESS)
    {
        retval = OS_SUCCESS;
    }

    return retval;
}

/* ============================ ACCESSORS ================================= */
PID OsProcessBase::getCurrentPID()
{
    return 0; //doesn't mean anything for base class, we need to know platform first
}

PID OsProcessBase::getPID()
{
    return mPID;
}

PID OsProcessBase::getParentPID()
{
    return mParentPID;
}


OsStatus OsProcessBase::getEnv(UtlString &rKey, UtlString &rValue)
{
    OsStatus retval = OS_FAILED;

    if (mEnvList.get(rKey,rValue) == OS_SUCCESS)
    {
        retval = OS_SUCCESS;
    }

    return retval;
}

OsStatus OsProcessBase::getProcessName(UtlString &rProcessName)
{
    OsStatus retval = OS_FAILED;
    
    if (!mProcessName.isNull())
    {
        rProcessName = mProcessName;
        retval = OS_SUCCESS;
    }

    return retval;
}

/* ============================ INQUIRY =================================== */

//dummy wait routine
int OsProcessBase::wait(int WaitInSecs)
{
    return -1;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
OsStatus OsProcessBase::ApplyEnv()
{
    OsStatus retval = OS_FAILED;
    UtlString searchKey = "";
    UtlString nextKey;
    UtlString nextValue;
    UtlBoolean bFailed = FALSE;
#ifndef HAVE_SETENV
    UtlString fullEnv;
#endif

    mEnvList.getNext(searchKey,nextKey, nextValue);
    while (nextKey != "")
    {
        searchKey = nextKey;
        int ret;
        // Use setenv if available, or else putenv.
#ifdef HAVE_SETENV
        ret = setenv(nextKey.data(), nextValue.data(), 1);
#else // HAVE_SETENV
        fullEnv = nextKey;
        fullEnv += "=";
        fullEnv += nextValue;
        ret = putenv((char *) fullEnv.data()); // putenv's argument is (char *), not (const char *)
#endif // HAVE_SETENV
        if (ret != 0)
        {
            bFailed = TRUE;
            break;
        }

        mEnvList.getNext(searchKey,nextKey, nextValue);
    }
    
    if (!bFailed)
        retval = OS_SUCCESS;

    return retval;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */




