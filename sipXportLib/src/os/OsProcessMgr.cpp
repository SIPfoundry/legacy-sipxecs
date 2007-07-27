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

#ifdef __pingtel_on_posix__
   #include <unistd.h>
#endif

// APPLICATION INCLUDES
#include "os/OsConfigDb.h"
#include "os/OsFS.h"
#include "os/OsStatus.h"
#include "os/OsProcessMgr.h"
#include "os/OsTask.h"
#include "os/OsSysLog.h"
#include "os/OsLock.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// STATIC VARIABLE INITIALIZATIONS
OsProcessMgr * OsProcessMgr::spManager = NULL;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsProcessMgr::OsProcessMgr() : 
mStdInputFilename(""),
mStdOutputFilename(""),
mStdErrorFilename("")
{
}

// Destructor
OsProcessMgr::~OsProcessMgr()
{
    mCurrentStateMap.destroyAll();
    mUserRequestedStateMap.destroyAll();
}

/* ============================ MANIPULATORS ============================== */

//by setting user requested state, you really dont change the current state.
//This was added so an external program, through the watchdog, would know what state the user 
//wanted the process to be in
OsStatus OsProcessMgr::setUserRequestState(const UtlString &rAlias, const int userRequestedState)
{
    OsStatus retval = OS_FAILED;

    OsSysLog::add(FAC_PROCESSMGR, PRI_DEBUG,
                  "ENTERING setUserRequestState  %s state = %d\n",
                  rAlias.data(),userRequestedState);

    switch(userRequestedState)
    {
    case USER_PROCESS_START:
    case USER_PROCESS_STOP:
    case USER_PROCESS_RESTART:
        // Only update the User Requested State for processes that are being monitored.
        if (mCurrentStateMap.contains(&rAlias) )
        {
            // Add/Update the entry in the User Requested State map.
            if (mUserRequestedStateMap.contains(&rAlias) )
            {
                UtlInt* pInt = dynamic_cast<UtlInt*>(mUserRequestedStateMap.findValue(&rAlias));
                *pInt = userRequestedState;
            }
            else
            {
                mUserRequestedStateMap.insertKeyAndValue(new UtlString(rAlias), new UtlInt(userRequestedState));
            }
            retval = OS_SUCCESS;
            OsSysLog::add(FAC_PROCESSMGR, PRI_DEBUG,
                          "SUCCESS setUserRequestState  %s state = %d\n",
                          rAlias.data(),userRequestedState);
        }
        else
        {
            OsSysLog::add(FAC_PROCESSMGR, PRI_ERR,"setUserRequestState: Invalid alias: %s.\n", 
                          rAlias.data());
        }
        break;

    default:
        OsSysLog::add(FAC_PROCESSMGR, PRI_ERR,"setUserRequestState: Invalid state: %d.\n", 
                      userRequestedState);
        break;
    }

    OsSysLog::add(FAC_PROCESSMGR, PRI_DEBUG,
                  "EXITING setUserRequestState  %s state = %d\n",
                  rAlias.data(),userRequestedState);

    return  retval;
}

OsStatus OsProcessMgr::setIORedirect(OsPath &rStdInputFilename,
                                     OsPath &rStdOutputFilename,
                                     OsPath &rStdErrorFilename)
{
    mStdInputFilename = rStdInputFilename;
    mStdOutputFilename = rStdOutputFilename;
    mStdErrorFilename = rStdErrorFilename;

    return OS_SUCCESS;
}

OsStatus OsProcessMgr::startProcess(UtlString &rAlias,
                                    UtlString &rExeName,
                                    UtlString rParameters[],
                                    UtlString &startupDir,
                                    OsProcessBase::OsProcessPriorityClass prio,
                                    UtlBoolean bExeclusive)
{
    OsStatus retval = OS_FAILED;
    OsProcess process;

    OsSysLog::add(FAC_PROCESSMGR, PRI_DEBUG,"Attempting start on  %s\n",rAlias.data());
    setAliasState(rAlias,PROCESS_STARTING);
    process.setIORedirect(mStdInputFilename,mStdOutputFilename,mStdErrorFilename);
    OsPath startDir = startupDir;
    if (process.launch(rExeName,rParameters,startDir) == OS_SUCCESS)
    {
        OsSysLog::add(FAC_PROCESSMGR, PRI_DEBUG,"Before addEntry for alias  %s\n",rAlias.data());
        addEntry(rAlias,process.getPID());
        retval = OS_SUCCESS;
        OsSysLog::add(FAC_PROCESSMGR, PRI_DEBUG,"Started OK for alias %s\n",rAlias.data());
    }
    else
    {
       OsSysLog::add(FAC_PROCESSMGR, PRI_ERR,"Start FAILED for %s\n",rAlias.data());
    }
    
    return retval;

}

OsStatus OsProcessMgr::stopProcess(UtlString &rAlias)
{
    OsStatus retval = OS_FAILED;
    OsProcess process;

    if (getProcessByAlias(rAlias,process) == OS_SUCCESS)
    {
        //must use internal so locking doesn't occur.  we already locked
        int state = getAliasState(rAlias);

        if (state == PROCESS_STARTED)
        {
            OsSysLog::add(FAC_PROCESSMGR, PRI_DEBUG,"Attempting stop on  %s\n",rAlias.data());
            setAliasState(rAlias,PROCESS_STOPPING);
            retval = process.kill();

            if (retval == OS_SUCCESS)
            {
               setAliasState(rAlias,PROCESS_STOPPED);
            }
            else
            {
                OsSysLog::add(FAC_PROCESSMGR, PRI_ERR,"Error stopping %s\n",rAlias.data());
                fprintf(stderr,"process.kill() failed in stopProcess(Alias) \n");
            }

        }

    }

    return retval;
}


OsStatus OsProcessMgr::stopProcess(PID pid)
{
    OsStatus retval = OS_FAILED;
    OsProcess process;
    UtlString aliasName;

    if (OsProcess::getByPID(pid,process) == OS_SUCCESS)
    {
        OsStatus findStatus = getAliasByPID(process.getPID(),aliasName);

        if (findStatus == OS_SUCCESS)
        {
           int state = getAliasState(aliasName);

           if (state == PROCESS_STARTED)
           {
              OsSysLog::add(FAC_PROCESSMGR, PRI_DEBUG,"Attempting stop on  %s\n",aliasName.data());
              setAliasState(aliasName,PROCESS_STOPPING);
              retval = process.kill();
              if (retval == OS_SUCCESS)
              {
                 //now find the alias which has that pid
                 OsStatus findStatus = getAliasByPID(process.getPID(),aliasName);
                 if (findStatus == OS_SUCCESS)
                 {
                    setAliasState(aliasName,PROCESS_STOPPED);
                 }
              }
              else
              {
                 OsSysLog::add(FAC_PROCESSMGR, PRI_ERR,"Error stopping %s\n",aliasName.data());
              }
           }
        }
    }
    else
    {
        OsSysLog::add(FAC_PROCESSMGR, PRI_ERR,
                      "Error getting process in stopProcess(pid) pid=%d\n",pid);
    }

    return retval;
}

OsStatus OsProcessMgr::getAliasByPID(PID pid ,UtlString &rAlias)
{
    OsStatus retval = OS_FAILED;
    rAlias = "";

    UtlString* pAlias;
    UtlHashMapIterator iMap(mCurrentStateMap);
    while ((pAlias = dynamic_cast<UtlString*>(iMap())) && OS_FAILED == retval)
    {
        UtlString* pState = dynamic_cast<UtlString*>(mCurrentStateMap.findValue(pAlias));
        int x = atoi(pState->data());
        if( pid == x )
        {
            rAlias = *pAlias;
            retval = OS_SUCCESS;    
        }
    }

    return retval;
}

OsStatus OsProcessMgr::getProcessByAlias(const UtlString &rAlias, OsProcess &rProcess)
{
    OsStatus retval = OS_FAILED;

    OsSysLog::add(FAC_PROCESSMGR, PRI_DEBUG,"ENTERING getProcessByAlias(%s)", rAlias.data());
 
    UtlString* pAlias;
    UtlHashMapIterator iMap(mCurrentStateMap);
    bool bFound = false;
    while ((pAlias = dynamic_cast<UtlString*>(iMap())) && !bFound)
    {
        if (rAlias == *pAlias)
        {
            bFound = true;
            UtlString* pState = dynamic_cast<UtlString*>(mCurrentStateMap.findValue(pAlias));

            int pid = atoi(pState->data());
            OsSysLog::add(FAC_PROCESSMGR, PRI_DEBUG,
                          "getProcessByAlias  checking if pid %d for alias %s is valid",
                          pid, rAlias.data());

            if (pid > 0)
            {
                retval = OsProcess::getByPID(pid,rProcess);
                if (retval == OS_SUCCESS)
                {
                   OsSysLog::add(FAC_PROCESSMGR, PRI_DEBUG,
                                 "getProcessByAlias  alias %s pid %d IS VALID",
                                 rAlias.data(),pid);
                }
                else
                {
                   OsSysLog::add(FAC_PROCESSMGR, PRI_DEBUG,
                                 "getProcessByAlias  alias %s pid %d is NOT valid",
                                 rAlias.data(),pid);
                }
            }
        }
    }


    OsSysLog::add(FAC_PROCESSMGR, PRI_DEBUG,"EXITING getProcessByAlias(%s)", rAlias.data());

    return retval;
}


int OsProcessMgr::getAliasState(const UtlString &rAlias)
{
    int state = PROCESS_FAILED;

    OsSysLog::add(FAC_PROCESSMGR, PRI_DEBUG,"ENTERING getAliasState  %s ",rAlias.data());

    // Lookup the alias is the map of monitored processes.
    if (!mCurrentStateMap.contains(&rAlias) )
    {
        // Not found.
        state = PROCESS_NEVERRUN;
        OsSysLog::add(FAC_PROCESSMGR, PRI_ERR,"getAliasState: Invalid alias: %s.\n", 
                      rAlias.data());
    }
    else
    {
        // Found.
        UtlString* pState = dynamic_cast<UtlString*>(mCurrentStateMap.findValue(&rAlias));
        if (!getCurrentStateFromString(*pState, state))
        {
            // Not a recognized state string, but is it a PID?
            if (atoi(pState->data()) > 0)
            {
                // The process is started.
                state = PROCESS_STARTED;

                // Use this opportunity to check if a valid PID is recorded.
                OsProcess process;
                OsSysLog::add(FAC_PROCESSMGR, PRI_DEBUG,
                              "getAliasState  checking if alias %s has valid PID",
                              rAlias.data());
                if (getProcessByAlias(rAlias, process) == OS_FAILED)
                {
                    OsSysLog::add(FAC_PROCESSMGR, PRI_ERR,
                                  "getAliasState - alias %s DOES NOT HAVE a valid PID",
                                  rAlias.data());
                    state = PROCESS_STARTING;
                    setAliasState(rAlias,PROCESS_STARTING);
                }
                else
                {
                   OsSysLog::add(FAC_PROCESSMGR, PRI_DEBUG,
                                 "getAliasState - alias %s HAS a valid PID",rAlias.data());
                }
            }
            else
            {
                // Not a recognized state string or a PID.  Flag this.
                OsSysLog::add(FAC_PROCESSMGR, PRI_ERR, "getAliasState(%s) - Unknown state: %s",
                              rAlias.data(), pState->data());
                setAliasState(rAlias,PROCESS_FAILED);
            }
        }
    }

    OsSysLog::add(FAC_PROCESSMGR, PRI_DEBUG,"EXITING getAliasState %s",rAlias.data());
    
    return state;
}

void OsProcessMgr::setAliasStopped(const UtlString &rAlias)
{
    setAliasState(rAlias,PROCESS_STOPPED);
}

/* ============================ ACCESSORS ================================= */
OsProcessMgr *OsProcessMgr::getInstance()
{
   if (!spManager)
   {
      spManager = new OsProcessMgr();
   }
   
   return spManager;
}

int OsProcessMgr::getUserRequestState(UtlString &rAlias)
{
    int state = USER_PROCESS_NONE;

    OsSysLog::add(FAC_PROCESSMGR, PRI_DEBUG,"ENTERING getUserRequestState(%s)",rAlias.data());

    // Lookup the alias is the map of monitored processes.
    UtlInt* pState = dynamic_cast<UtlInt*>(mUserRequestedStateMap.findValue(&rAlias));
    if (! pState)
    {
        // Not found.
        OsSysLog::add(FAC_PROCESSMGR, PRI_ERR,"getUserRequestState: Invalid alias: %s.\n", 
                      rAlias.data());
    }
    else
    {
       state = *pState;
    }

    OsSysLog::add(FAC_PROCESSMGR, PRI_DEBUG,"EXITING getUserRequestState(%s), state: %d",
                  rAlias.data(), state);

    return state;
}

/* ============================ INQUIRY =================================== */

UtlBoolean OsProcessMgr::isStarted(UtlString &rAlias)
{
    UtlBoolean retval = FALSE;

    if (getAliasState(rAlias) == PROCESS_STARTED)
    {
       retval = TRUE;
    }
    return retval;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
/* //////////////////////////// PRIVATE /////////////////////////////////// */

OsStatus OsProcessMgr::setAliasState(const UtlString &rAlias, int newstate)
{
    OsStatus retval = OS_FAILED;
    
    OsSysLog::add(FAC_PROCESSMGR, PRI_DEBUG, "ENTERING setAliasState(%s, %d)", 
        rAlias.data(), newstate);

    UtlString sState;
    getCurrentStateString(newstate, sState);
    switch(newstate)
    {
    case PROCESS_STARTING:
    case PROCESS_STOPPING:
    case PROCESS_STOPPED:
    case PROCESS_FAILED:
    {
        // Update/Add the entry in the Current State map.
        UtlString* pStr = dynamic_cast<UtlString*>(mCurrentStateMap.findValue(&rAlias));
        if (pStr)
        {
            *pStr = sState;
        }
        else
        {
            mCurrentStateMap.insertKeyAndValue(new UtlString(rAlias), new UtlString(sState));

        }
        retval = OS_SUCCESS;
    }
    break;

    case PROCESS_NEVERRUN:
        removeEntry(rAlias);
        retval = OS_SUCCESS;
        break;

    default:
        OsSysLog::add(FAC_PROCESSMGR, PRI_ERR, "setAliasState: Invalid state %d", newstate);
        break;
    }

    OsSysLog::add(FAC_PROCESSMGR, PRI_DEBUG,
                  "EXITING setAliasState  %s state = %d\n",rAlias.data(),newstate);

    return  retval;
}
                    
void OsProcessMgr::addEntry(const UtlString &rAlias, int pid)
{        
    char buf[20];
    sprintf(buf,"%d",pid);

    OsSysLog::add(FAC_PROCESSMGR, PRI_DEBUG,"Attempting addEntry(%s, %d)", 
        rAlias.data(), pid);

    // Insert the new entry, clearing the old entry if one exists.
    removeEntry(rAlias);
    mCurrentStateMap.insertKeyAndValue(new UtlString(rAlias), new UtlString(buf));
}

void OsProcessMgr::removeEntry(const UtlString &rAlias)
{
    mCurrentStateMap.destroy(&rAlias);
    mUserRequestedStateMap.destroy(&rAlias);
}

bool OsProcessMgr::getCurrentStateString(const int state, UtlString& str)
{
    bool result = true;

    switch (state)
    {
    case PROCESS_STARTED:
       str = "STARTED";
       break;

    case PROCESS_STARTING:
       str = "STARTING";
       break;

    case PROCESS_STOPPING:
       str = "STOPPING";
       break;

    case PROCESS_STOPPED:
       str = "STOPPED";
       break;

    case PROCESS_FAILED:
       str = "FAILED";
       break;

    default:
       result = false;
       break;
    }

    return result;
}

bool OsProcessMgr::getCurrentStateFromString(const UtlString& str, int& state)
{
    bool result = true;

    if (str == "STARTED")
    {
        state = PROCESS_STARTED;
    }
    else if (str == "STARTING")
    {
        state = PROCESS_STARTING;
    }
    else if (str == "STOPPING")
    {
        state = PROCESS_STOPPING;
    }
    else if (str == "STOPPED")
    {
        state = PROCESS_STOPPED;
    }
    else if (str == "FAILED")
    {
        state = PROCESS_FAILED;
    }
    else
    {
        result = false;
    }

    return result;
}

bool OsProcessMgr::getUserRequestedStateString(const int state, UtlString& str)
{
    bool result = true;

    switch (state)
    {
    case USER_PROCESS_START:
       str = "USER_START";
       break;

    case USER_PROCESS_STOP:
       str = "USER_STOP";
       break;

    case USER_PROCESS_RESTART:
       str = "USER_RESTART";
       break;

    default:
       result = false;
       break;
    }

    return result;
}

bool OsProcessMgr::getUserRequestedStateFromString(const UtlString& str, int& state)
{
    bool result = true;

    if (str == "USER_START")
    {
        state = USER_PROCESS_START;
    }
    else if (str == "USER_STOP")
    {
        state = USER_PROCESS_STOP;
    }
    else if (str == "USER_RESTART")
    {
        state = USER_PROCESS_RESTART;
    }
    else
    {
        result = false;
    }

    return result;
}

/* ============================ FUNCTIONS ================================= */




