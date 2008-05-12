// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <stdlib.h>

// APPLICATION INCLUDES
#include "DependencyList.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
DependencyList::DependencyList()
{
}

// Copy constructor
DependencyList::DependencyList(const DependencyList& rDependencyList)
{
}

// Destructor
DependencyList::~DependencyList()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
DependencyList& 
DependencyList::operator=(const DependencyList& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}
void DependencyList::addDependent(UtlString &rDependentName)
{
    if (mNumDependents < MAX_DEPENDENCIES)
        Dependents[mNumDependents++] = rDependentName;
    else
        osPrintf("Max dependents reached!\n");
}

void DependencyList::setName(UtlString &rProcessName)
{
    mProcessName = rProcessName;
}

void DependencyList::setDelay(int delaySecs)
{
    mDelaySecs = delaySecs;
}

void DependencyList::setCanStart(UtlBoolean bCanStart)
{
    mbCanStart = bCanStart;
}

void DependencyList::setCanRestart(UtlBoolean bCanRestart)
{
    mbCanRestart = bCanRestart;
}

void DependencyList::setCanStop(UtlBoolean bCanStop)
{
    mbCanStop = bCanStop;
}

void DependencyList::setVerifyCommand(UtlString &rVerifyComand)
{
    mVerifyCommand = rVerifyComand;
}

void DependencyList::setVerifyParameters(UtlString &rVerifyParameters)
{
    mVerifyParameters = rVerifyParameters;
}

void DependencyList::setVerifyDefaultDir(UtlString &rVerifyDefaultDir)
{
    mVerifyDefaultDir = rVerifyDefaultDir;
}

/* ============================ ACCESSORS ================================= */
UtlString DependencyList::getVerifyCommand()
{
    return mVerifyCommand;
}

UtlString DependencyList::getVerifyParameters()
{
    return mVerifyParameters;
}

UtlString DependencyList::getVerifyDefaultDir()
{
    return mVerifyDefaultDir;
}

UtlBoolean DependencyList::getCanStart()
{
    return mbCanStart;
}

UtlBoolean DependencyList::getCanRestart()
{
    return mbCanRestart;
}

UtlBoolean DependencyList::getCanStop()
{
    return mbCanStop;
}

UtlString DependencyList::getName()
{
    return mProcessName;
}

int DependencyList::getDependencyCount()
{
    return mNumDependents;
}

OsStatus DependencyList::getDependency(int index, UtlString &rDependent)
{
    OsStatus retval = OS_FAILED;

    if (index < mNumDependents)
    {
        rDependent = Dependents[index];
        retval = OS_SUCCESS;
    }

    return retval;
}

int DependencyList::getDelay()
{
    return mDelaySecs;
}

/* ============================ INQUIRY =================================== */
UtlBoolean DependencyList::isDependent(UtlString &rDependentName)
{
    UtlBoolean retval = FALSE;

    //walk the list and figure out if it's in there
    for (int i = 0; i < mNumDependents;i++)
    {
        if (Dependents[i] == rDependentName)
        {
            retval = TRUE;
            break;
        }
    }

    return retval;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */

