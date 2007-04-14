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

//used for processAlias.dat locking
int gFile_descr = -1;

// when ProcessMgr calls come funcs internally, we dont want locking to occur
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS


// STATIC VARIABLE INITIALIZATIONS
OsProcessMgr * OsProcessMgr::spManager = NULL;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsProcessMgr::OsProcessMgr(const char* workingDirectory) :
mProcessFilename(PROCESS_ALIAS_FILE),
mProcessLockFilename(PROCESS_ALIAS_LOCK_FILE),
mWorkPath(workingDirectory),
mStdInputFilename(""),
mStdOutputFilename(""),
mStdErrorFilename(""),
pProcessList(NULL),
mAliasLockFileCount(0),
mMutex(OsMutex::Q_PRIORITY)
{
    if (!pProcessList)
    {
        lockAliasFile();

        pProcessList = new OsConfigDb();
        loadProcessFile();

        unlockAliasFile();

    }
}

// Destructor
OsProcessMgr::~OsProcessMgr()
{
   if (pProcessList)
   {
      delete pProcessList;
   }
}

/* ============================ MANIPULATORS ============================== */

//by setting user requested state, you really dont change the current state.
//This was added so an external program, through the watchdog, would know what state the user 
//wanted the process to be in
OsStatus OsProcessMgr::setUserRequestState(UtlString &rAlias, int userRequestedState)
{
    OsStatus retval = OS_FAILED;
    UtlString origVal;
    UtlString buf;

    lockAliasFile();

    OsSysLog::add(FAC_PROCESSMGR, PRI_DEBUG,
                  "ENTERING setUserRequestState  %s state = %d\n",
                  rAlias.data(),userRequestedState);

    switch(userRequestedState)
    {
    case USER_PROCESS_START   :
       buf = "USER_START";
       break;
    case USER_PROCESS_STOP    :
       buf = "USER_STOP";
       break;
    case USER_PROCESS_RESTART :
       buf = "USER_RESTART";
       break;
    default:
       OsSysLog::add(FAC_PROCESSMGR, PRI_ERR,"setUserRequestState: Invalid state!\n");
       break;
    }

    //only do it if there is an entry already
    if (pProcessList->get(rAlias,origVal) == OS_SUCCESS)
    {
        //now only make it the first part of the two part value.
        //we do this because the user may have already set a requested state
        //and they are changing it to a new value
        getAliasFirstValue(origVal);

        //now append the user requested value
        origVal += " : ";
        origVal += buf;

        //no return code for next func...
        pProcessList->set(rAlias,origVal);

        //now try to save it
        if (storeProcessFile() == OS_SUCCESS)
        {
            retval = OS_SUCCESS;
            OsSysLog::add(FAC_PROCESSMGR, PRI_DEBUG,
                          "SUCCESS setUserRequestState  %s state = %d\n",
                          rAlias.data(),userRequestedState);
        }
        else
        {
            OsSysLog::add(FAC_PROCESSMGR, PRI_ERR,
                          "setUserRequestState  %s state = %d\n",
                          rAlias.data(),userRequestedState);
        }
    }

    unlockAliasFile();

    OsSysLog::add(FAC_PROCESSMGR, PRI_DEBUG,
                  "EXITING setUserRequestState  %s state = %d\n",
                  rAlias.data(),userRequestedState);

    return  retval;

}

void OsProcessMgr::setProcessListFilename(UtlString &rFilename)
{
    mProcessFilename = rFilename;
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

    lockAliasFile();
                                         
    OsSysLog::add(FAC_PROCESSMGR, PRI_DEBUG,"Attempting start on  %s\n",rAlias.data());
    setAliasState(rAlias,PROCESS_STARTING);
    process.setIORedirect(mStdInputFilename,mStdOutputFilename,mStdErrorFilename);
    OsPath startDir = startupDir;
    if (process.launch(rExeName,rParameters,startDir) == OS_SUCCESS)
    {
        OsSysLog::add(FAC_PROCESSMGR, PRI_DEBUG,"Before addEntry for alias  %s\n",rAlias.data());
        retval = addEntry(rAlias,process.getPID());
        OsSysLog::add(FAC_PROCESSMGR, PRI_DEBUG,"Started OK for alias %s\n",rAlias.data());
    }
    else
    {
       OsSysLog::add(FAC_PROCESSMGR, PRI_ERR,"Start FAILED for %s\n",rAlias.data());
    }
    
    unlockAliasFile();

    return retval;

}

OsStatus OsProcessMgr::stopProcess(UtlString &rAlias)
{
    OsStatus retval = OS_FAILED;
    OsProcess process;

    lockAliasFile();

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

    unlockAliasFile();

    return retval;
}


OsStatus OsProcessMgr::stopProcess(PID pid)
{
    OsStatus retval = OS_FAILED;
    OsProcess process;
    UtlString aliasName;

    lockAliasFile();

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

    unlockAliasFile();

    return retval;
}

OsStatus OsProcessMgr::getAliasByPID(PID pid ,UtlString &rAlias)
{
    OsStatus retval = OS_FAILED;
    UtlString searchKey = "";
    UtlString nextKey;
    UtlString nextValue;
    UtlString fullEnv = "";
    UtlBoolean bFailed = TRUE;
    rAlias = "";

    lockAliasFile();

    if (loadProcessFile() == OS_SUCCESS)
    {
        pProcessList->getNext(searchKey,nextKey, nextValue);

        getAliasFirstValue(nextValue);

        while (nextKey != "")
        {
            searchKey = nextKey;

            if (pid == atoi(nextValue.data()))
            {
                rAlias = searchKey;
                bFailed = FALSE;
                break;
            }

            pProcessList->getNext(searchKey,nextKey, nextValue);
            getAliasFirstValue(nextValue);
        }

        if (!bFailed)
        {
            retval = OS_SUCCESS;
        }
    }

    unlockAliasFile();

    return retval;

}

OsStatus OsProcessMgr::getProcessByAlias(UtlString &rAlias, OsProcess &rProcess)
{
    OsStatus retval = OS_FAILED;

    OsSysLog::add(FAC_PROCESSMGR, PRI_DEBUG,"ENTERING getProcessByAlias  %s ",rAlias.data());

    lockAliasFile();

    if (loadProcessFile() == OS_SUCCESS)
    {
        OsSysLog::add(FAC_PROCESSMGR, PRI_DEBUG,
                      "SUCCESS getProcessByAlias  %s loading process file",rAlias.data());
        UtlString value;
        if (pProcessList->get(rAlias,value) == OS_SUCCESS)
        {
            getAliasFirstValue(value);

            int pid = atoi(value.data());
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
    else
    {
       UtlString f = mWorkPath + "/" + mProcessFilename;
       OsSysLog::add(FAC_PROCESSMGR, PRI_ERR,
                     "OsProcessMgr::getProcessByAlias Error loading process file '%s' for alias '%s'",
                     f.data(),
                     rAlias.data());
    }
    
    unlockAliasFile();

    OsSysLog::add(FAC_PROCESSMGR, PRI_DEBUG,"EXITING getProcessByAlias  %s ",rAlias.data());

    return retval;
}


int OsProcessMgr::getAliasState(UtlString &rAlias)
{
    int state = PROCESS_FAILED;

    OsSysLog::add(FAC_PROCESSMGR, PRI_DEBUG,"ENTERING getAliasState  %s ",rAlias.data());

    lockAliasFile();

    pProcessList->remove(rAlias);
    loadProcessFile();
    UtlString value;

    if (pProcessList->get(rAlias,value) != OS_SUCCESS)
    {
       state = PROCESS_NEVERRUN;
    }
    else
    {
        //since the value returned contains current_state : request_state
        //we need to grab only the part we need
        getAliasFirstValue(value);

        value.toUpper();

        if (value == "STARTING")
        {
           state = PROCESS_STARTING;
        }
        else if (value == "STOPPING")
        {
           state = PROCESS_STOPPING;
        }
        else if (value == "STOPPED")
        {
           state = PROCESS_STOPPED;
        }
        else if (value == "FAILED")
        {
           state = PROCESS_FAILED;
        }
        else if (atoi(value) > 0)
        {
            state = PROCESS_STARTED;
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
           OsSysLog::add(FAC_PROCESSMGR, PRI_ERR,
                         "getAliasState - alias %s HAS a valid PID",rAlias.data());
           setAliasState(rAlias,PROCESS_FAILED);
        }
    }

    unlockAliasFile();

    OsSysLog::add(FAC_PROCESSMGR, PRI_DEBUG,"EXITING getAliasState %s",rAlias.data());
    
    return state;
}

void OsProcessMgr::setAliasStopped(UtlString &rAlias)
{
    lockAliasFile();

    setAliasState(rAlias,PROCESS_STOPPED);

    unlockAliasFile();
}

/* ============================ ACCESSORS ================================= */
OsProcessMgr *OsProcessMgr::getInstance(const char* workingDirectory)
{
   if (!spManager)
   {
      spManager = new OsProcessMgr(workingDirectory);
   }
   
   return spManager;
}

int OsProcessMgr::getUserRequestState(UtlString &rAlias)
{
    int state = USER_PROCESS_NONE;

    OsSysLog::add(FAC_PROCESSMGR, PRI_DEBUG,"ENTERING getUserRequestState  %s ",rAlias.data());

    lockAliasFile();

    pProcessList->remove(rAlias);
    loadProcessFile();
    UtlString value;

    if (pProcessList->get(rAlias,value) != OS_SUCCESS)
    {
       state = USER_PROCESS_NONE;
    }
    else
    {
        //since the value returned contains current_state : request_state
        //we need to grab only the part we need
        getAliasSecondValue(value);

        value.toUpper();

        if (value == "USER_START")
        {
           state = USER_PROCESS_START;
        }
        else if (value == "USER_STOP")
        {
           state = USER_PROCESS_STOP;
        }
        else if (value == "USER_RESTART")
        {
           state = USER_PROCESS_RESTART;
        }
        else
        {
            state = USER_PROCESS_NONE;
        }
    }

    unlockAliasFile();

    OsSysLog::add(FAC_PROCESSMGR, PRI_DEBUG,"EXITING getUserRequestState %s",rAlias.data());

    return state;

}

/* ============================ INQUIRY =================================== */

UtlBoolean OsProcessMgr::isStarted(UtlString &rAlias)
{
    UtlBoolean retval = FALSE;

    lockAliasFile();

    if (getAliasState(rAlias) == PROCESS_STARTED)
    {
       retval = TRUE;
    }
    unlockAliasFile();
    return retval;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
/* //////////////////////////// PRIVATE /////////////////////////////////// */

void OsProcessMgr::getAliasFirstValue(UtlString &rinValue)
{
    UtlString retString = "";

    //search for a " : "
    //if found, copy only up to that area
    size_t separatorPos = rinValue.index(" : ");
    if (separatorPos != UTL_NOT_FOUND)
    {
        rinValue.remove(separatorPos);
    }

}

void OsProcessMgr::getAliasSecondValue(UtlString &rinValue)
{
    UtlString retString = "";

    //search for a " : "
    //if found, copy only up to that area
    size_t separatorPos = rinValue.index(" : ");
    if (separatorPos != UTL_NOT_FOUND)
    {
        rinValue = rinValue.data()+separatorPos+3;
    }

}

OsStatus OsProcessMgr::setAliasState(UtlString &rAlias,int newstate)
{
    OsStatus retval = OS_FAILED;
    UtlString buf;

    lockAliasFile();

    OsSysLog::add(FAC_PROCESSMGR, PRI_DEBUG,"ENTERING setAliasState  %s state = %d\n",
                  rAlias.data(),newstate);

    switch(newstate)
    {
    case PROCESS_STARTING   :
       buf = "STARTING";
       break;
    case PROCESS_STOPPING   :
       buf = "STOPPING";
       break;
    case PROCESS_STOPPED    :
       buf = "STOPPED";
       break;
    case PROCESS_FAILED     :
       buf = "FAILED";
       break;
    case PROCESS_NEVERRUN   :
       removeEntry(rAlias);
       retval = OS_SUCCESS;
       break;
    default:
       OsSysLog::add(FAC_PROCESSMGR, PRI_ERR, "setAliasState: Invalid state %d", newstate);
       break;
    }

    if (newstate != PROCESS_NEVERRUN)
    {
        //no return code for next func...
        pProcessList->set(rAlias,buf);

        //now try to save it
        if (storeProcessFile() == OS_SUCCESS)
        {
            retval = OS_SUCCESS;
            OsSysLog::add(FAC_PROCESSMGR, PRI_DEBUG,"SUCCESS setAliasState  %s state = %d\n",
                          rAlias.data(),newstate);
        }
        else
        {
            OsSysLog::add(FAC_PROCESSMGR, PRI_ERR, "setAliasState: %s state = %d\n",
                          rAlias.data(),newstate);
        }
    }

    unlockAliasFile();

    OsSysLog::add(FAC_PROCESSMGR, PRI_DEBUG,
                  "EXITING setAliasState  %s state = %d\n",rAlias.data(),newstate);

    return  retval;
}
                    
OsStatus OsProcessMgr::addEntry(UtlString &rAlias, int pid)
{               
    OsStatus retval = OS_FAILED;
    char buf[20];
    sprintf(buf,"%d",pid);

    lockAliasFile();

    loadProcessFile();

    pProcessList->remove(rAlias);
    
    //no return code for next func...
    pProcessList->set(rAlias,buf);

    //now try to save it
    OsSysLog::add(FAC_PROCESSMGR, PRI_DEBUG,"Attempting addEntry");
    if (storeProcessFile() == OS_SUCCESS)
    {
        OsSysLog::add(FAC_PROCESSMGR, PRI_DEBUG,
                      "addEntry to %s. PID=%d  Flushing processAlias.dat to disk. ",
                      rAlias.data(),pid);
        retval = OS_SUCCESS;
    }

    unlockAliasFile();

    return retval;
}


OsStatus OsProcessMgr::removeEntry(UtlString &rAlias)
{
    OsStatus retval = OS_FAILED;

    lockAliasFile();

    if (pProcessList->remove(rAlias) == OS_SUCCESS)
    {
        //now try to save it
       if (storeProcessFile() == OS_SUCCESS)
       {
            retval = OS_SUCCESS;
       }
    }

    unlockAliasFile();

    return retval;
}

void OsProcessMgr::lockAliasFile()
{
    OsLock lock(mMutex);

    UtlString tmpStr(mWorkPath);
    tmpStr = mWorkPath;
    tmpStr += "/";
    tmpStr += mProcessLockFilename;

    OsPath lockFullPath = tmpStr;

    //if we already have a lock then just return
    if (mAliasLockFileCount > 0)
    {
        mAliasLockFileCount++;
        return;
    }

    UtlBoolean bIsHellFrozenOver = FALSE;
    int i = 0;
    while (!bIsHellFrozenOver)
    {
         OsSysLog::add(FAC_PROCESSMGR, PRI_DEBUG,
                       "Trying to get lock on file: %s...", tmpStr.data());

#ifdef __pingtel_on_posix__
        gFile_descr = open(lockFullPath.data(),O_RDWR | O_CREAT | O_EXCL,0644);
#elif _WIN32
        gFile_descr = _open(lockFullPath.data(),O_RDWR | O_CREAT | O_EXCL);
#endif

        if (gFile_descr == -1)
        {
            OsSysLog::add(FAC_PROCESSMGR, PRI_DEBUG, "FAILED getting lock!\n");
          
            if (i++ > 60)
            {
                i = 0;
                OsSysLog::add(FAC_PROCESSMGR, PRI_ERR, "FAILED 15 TIMES.  Trying remove...\n");
                if (OsFileSystem::remove(lockFullPath.data()) == OS_SUCCESS)
                {
                    OsSysLog::add(FAC_PROCESSMGR, PRI_ERR,
                                  "Remove successful.  Execution continuing...\n");
                }
                else
                {                 
                  OsSysLog::add(FAC_PROCESSMGR, PRI_ERR,
                                "Error removing %s lock file.\n",lockFullPath.data());
                }
            }
        }
        else
        {
            OsSysLog::add(FAC_SIP, PRI_DEBUG, "Got it!\n");
            break;
        }

        OsTask::delay(1000);
    }

    mAliasLockFileCount++;
}

void OsProcessMgr::unlockAliasFile()
{
    OsLock lock(mMutex);

    OsPath lockFullPath = mWorkPath + "/" + mProcessLockFilename;

    OsSysLog::add(FAC_PROCESSMGR, PRI_DEBUG,"Unlocking AliasLockCount=%d",mAliasLockFileCount);

    if (mAliasLockFileCount > 0)
    {
       mAliasLockFileCount--;
    }
    
    if (mAliasLockFileCount == 0)
    {

        if (gFile_descr)
#ifdef __pingtel_on_posix__
        close(gFile_descr);
#elif _WIN32
        _close(gFile_descr);
#endif
        if (OsFileSystem::remove(lockFullPath,FALSE,TRUE) != OS_SUCCESS)
        {
            OsSysLog::add(FAC_PROCESSMGR, PRI_ERR,"Could not remove lock file!");
        }

    }
}

OsStatus OsProcessMgr::loadProcessFile()
{
    OsStatus retval = OS_FAILED;
    OsPath processFullPath = mWorkPath + "/" + mProcessFilename;
    OsDir processDir(mWorkPath);

    OsSysLog::add(FAC_PROCESSMGR, PRI_DEBUG,
                  "OsProcessMgr::loadProcessFile Loading file '%s'",
                  processFullPath.data());
    if (OsFileSystem::exists(processFullPath) == OS_SUCCESS)
    {
        OsStatus loadCode = pProcessList->loadFromFile(processFullPath.data());

        if (loadCode == OS_SUCCESS)
        {
           retval = OS_SUCCESS;
        }
        else
        {
            OsSysLog::add(FAC_PROCESSMGR, PRI_ERR,
                          "OsProcessMgr::loadProcessFile Error loading file '%s'",
                          processFullPath.data());
        }
    }

    return retval;
}


OsStatus OsProcessMgr::storeProcessFile()
{
    OsStatus retval = OS_FAILED;

    //create the work directory if needed

    OsDir processDir(mWorkPath);

    OsPath processFullPath = mWorkPath + "/" + mProcessFilename;


    OsSysLog::add(FAC_PROCESSMGR, PRI_DEBUG,"ATTEMPTING save on storeProcessFile %s",
                  processFullPath.data());
    OsStatus savedCode = pProcessList->storeToFile(processFullPath.data());

    if (savedCode == OS_SUCCESS)
    {
         OsSysLog::add(FAC_PROCESSMGR, PRI_DEBUG,"SUCCESS saving in storeProcessFile %s",
                       processFullPath.data());
         retval = OS_SUCCESS;
    }
    else
    {
       OsSysLog::add(FAC_PROCESSMGR, PRI_ERR,"storeProcessFile code %d saving in %s ",
                     savedCode, processFullPath.data());
    }
    

    return retval;
}
/* ============================ FUNCTIONS ================================= */




