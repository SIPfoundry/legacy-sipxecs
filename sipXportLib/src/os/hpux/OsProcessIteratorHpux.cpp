//
// Copyright (C) 2007 Hewlett-Packard Development Company, L.P.
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>
#include <stdlib.h>

// APPLICATION INCLUDES
#include "os/OsFS.h"
#include "os/hpux/OsProcessIteratorHpux.h"

// PSTAT INCLUDES
#include <sys/param.h>
#include <sys/pstat.h>


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsProcessIteratorHpux::OsProcessIteratorHpux() :
hProcessSnapshot(0)
{
   idx = 0;
}

// Destructor
OsProcessIteratorHpux::~OsProcessIteratorHpux()
{
}

/* ============================ MANIPULATORS ============================== */


/* ============================ ACCESSORS ================================= */
OsStatus OsProcessIteratorHpux::findFirst(OsProcess &rProcess)
{
    OsStatus retval = OS_FAILED;
    struct pst_status pststatus;

    idx = 0;

    if( pstat_getproc(&pststatus, sizeof(struct pst_status), 1, idx) == 1 )
    {
        rProcess.mProcessName = pststatus.pst_ucomm;
    	rProcess.mProcessName.strip(UtlString::both, ' ');
    	rProcess.mPID = pststatus.pst_pid;
    	rProcess.mParentPID = pststatus.pst_ppid;

	idx = pststatus.pst_idx + 1;
    	retval = OS_SUCCESS;
    }

    return retval;
}

OsStatus OsProcessIteratorHpux::findNext(OsProcess &rProcess)
{
    OsStatus retval = OS_FAILED;

    struct pst_status pststatus;

    if( pstat_getproc(&pststatus, sizeof(struct pst_status), 1, idx) == 1 )
    {
    	rProcess.mProcessName = pststatus.pst_ucomm;
    	rProcess.mProcessName.strip(UtlString::both, ' ');
    	rProcess.mPID = pststatus.pst_pid;
    	rProcess.mParentPID = pststatus.pst_ppid;

	idx = pststatus.pst_idx + 1;
    	retval = OS_SUCCESS;
    }

    return retval;
}

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

//reads the status file in the /proc/nnnn directory
OsStatus OsProcessIteratorHpux::readProcFile(OsPath &procDirname, OsProcess & rProcess)
{
    OsStatus retval = OS_FAILED;
    struct pst_status pststatus;
    PID pid = atol(procDirname.data());

    if( pstat_getproc(&pststatus, sizeof(struct pst_status), 0, pid) == 1 )
    {
    	rProcess.mProcessName = pststatus.pst_ucomm;
    	rProcess.mProcessName.strip(UtlString::both, ' ');
    	rProcess.mPID = pststatus.pst_pid;
    	rProcess.mParentPID = pststatus.pst_ppid;
    	retval = OS_SUCCESS;
    }

    return retval;
}

/* ============================ FUNCTIONS ================================= */
