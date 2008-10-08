//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//
// $$
////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

// APPLICATION INCLUDES
#include "os/OsProcess.h"
#include "os/OsTask.h"
#include "os/linux/OsProcessLinux.h"
#include "os/linux/OsUtilLinux.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

OsProcessLinux::OsProcessLinux()
{
}

// Destructor
OsProcessLinux::~OsProcessLinux()
{
}


/* ============================ MANIPULATORS ============================== */

OsStatus OsProcessLinux::setIORedirect(OsPath &rStdInputFilename,
                                       OsPath &rStdOutputFilename,
                                       OsPath &rStdErrorFilename)
{
    OsStatus retval = OS_FAILED;

    if (rStdInputFilename.length())
        mStdInputFilename = rStdInputFilename;
    else
        mStdInputFilename = "";
        
    
    if (rStdOutputFilename.length())
        mStdOutputFilename = rStdOutputFilename;
    else
        mStdInputFilename = "";
    
    if (rStdErrorFilename.length())
        mStdErrorFilename = rStdErrorFilename;
    else
        mStdInputFilename = "";
    
    return retval;
}

OsStatus OsProcessLinux::setPriority(int prio)
{
    OsStatus retval = OS_FAILED;
    
    if (setpriority(PRIO_PROCESS,mPID,prio) == 0)
    {
        retval = OS_SUCCESS;
    }

    return retval;
}

//waits for a process to complete before returning 
//or exits when WaitInSecs has completed
int OsProcessLinux::wait(int WaitInSecs)
{
    UtlBoolean bStillRunning = TRUE;
    int secs_waited = 0;
    int status = 1;
    int ExitCode = -1;

    if (WaitInSecs < 0)
        WaitInSecs = 0;

    if (mPID > 0)
    {
        while (bStillRunning && secs_waited <= WaitInSecs)
        {
            pid_t pid;
            if ((pid=waitpid(mPID,&status,WNOHANG|WUNTRACED)) == 0)
            {
                // process has not changed status so it still running.
                bStillRunning = TRUE;
                OsTask::delay(1000);

                if (WaitInSecs >  0)
                    secs_waited++;
            }
            else
            {
               if ( pid < 0 )
               {
                  // an error has occured.
                  // Mark the process as no longer running and return success (0).
                  // This usually occurs when the launch was setup to ignore the signal SIGCHLD.
                  bStillRunning = FALSE;
                  ExitCode = WEXITSTATUS(status);
               }
               else
               {
                  // pid is our mPID so the child process as exited
                  // and a signal SIGCHLD was generated.
                  // Obtain the exit code and return it.
                  bStillRunning = FALSE;
                  if (WIFEXITED(status)) 
                  {
                     ExitCode = WEXITSTATUS(status);
                  }
                  else if (WIFSIGNALED(status))
                  {
                     ExitCode = WTERMSIG(status);
                  }
                  else if (WIFSTOPPED(status))
                  {
                     ExitCode = WSTOPSIG(status);
                  }
               }
            }

        }
    }
    else
    {
        ExitCode = -1;
    }
    return ExitCode;
}

OsStatus OsProcessLinux::kill()
{
    OsStatus retval = OS_FAILED;
    
    if (::kill(mPID, SIGTERM) == 0)
    {
        int trycount = 0;

        OsSysLog::add(FAC_PROCESS, PRI_INFO,"Attempting kill on  %s",mProcessName.data());

        while (isRunning() && trycount++ < 3)
        {
            OsTask::delay(1000);    
            ::kill(mPID, SIGTERM);
        }
        
        //bust a cap in it's....
        trycount = 0;
        while (isRunning() && trycount++ < 30)
        {
            ::kill(mPID, SIGKILL);
            OsTask::delay(1000);    
        }

        if (isRunning())
        {
           OsSysLog::add(FAC_PROCESS, PRI_ERR,"Kill of '%s' FAILED",mProcessName.data());
            retval = OS_FAILED;  //not good.  the thing just won't go away
        }
        else
        {
            retval = OS_SUCCESS; //hurray! it's gone
            OsSysLog::add(FAC_PROCESS, PRI_NOTICE,"Killed '%s'",mProcessName.data());
        }
    }

    return retval;
}

OsStatus OsProcessLinux::launch(UtlString &rAppName, UtlString parameters[], OsPath &startupDir,
                    OsProcessPriorityClass prioClass, UtlBoolean bExclusive, UtlBoolean bIgnoreChildSignals)
{
    OsStatus retval = OS_FAILED;

    if (bIgnoreChildSignals)
    {
       // Ignore SIGCHLD, it will be automatically reaped (POSIX.1-2001 spec)
       OsUtilLinux::signal(SIGCHLD, SIG_IGN);
    }

    //build one string out of the array passed in
    int parameterCount = -1;
    while (!parameters[++parameterCount].isNull());

    const char ** parms = new const char * [parameterCount + 2];
    parms[0] = rAppName.data();
    for(int i = 0; i < parameterCount; i++)
        parms[i + 1] = parameters[i].data();
    parms[parameterCount + 1] = NULL;
    
    //now fork into two processes
    PID forkReturnVal = fork();
    switch (forkReturnVal) 
    {
        case -1 :   retval = OS_FAILED;
                    break;
        
        case 0 :
        {
                    // this is the child process so we need to exec the new
                    // process now it's time to redirect the output, input, and
                    // error streams

                    // Note: we must use _exit() rather than exit() here.
                    // This has to do with the interactions between C++, the OS
                    // abstraction layer, and the way threads are implemented on
                    // Linux.

                    if (mStdInputFilename.length())
                    {
                        if (!freopen(mStdInputFilename.data(),"r",stdin)) 
                        {
                            osPrintf("Could not redirect stdInput in OsProcess!");
                            _exit(1);
                        }
                    }

                    if (mStdOutputFilename.length())
                    {
                        if (!freopen(mStdOutputFilename.data(),"w",stdout)) 
                        {
                            osPrintf("Could not redirect stdOutput in OsProcess!");
                            _exit(1);
                        }
                    }               

                    if (mStdErrorFilename.length())
                    {
                        if (!freopen(mStdErrorFilename.data(),"w",stderr)) 
                        {
                            osPrintf("Could not redirect stdError in OsProcess!");
                            _exit(1);
                        }
                    }

                    // close all other file descriptors that may be open
                    int max_fd=1023;
                    struct rlimit limits ;
                    if(getrlimit(RLIMIT_NOFILE, &limits) == 0)
                    {
                       max_fd = limits.rlim_cur - 1 ;
                    }
                    for(int i=max_fd; i>2; i--)
                    {
                       close(i) ;
                    }

                    // Clear signal mask so the new process starts with
                    // a "normal" mask

                    OsTask::unBlockSignals();

                    //now apply the env variables the user may have set
                    ApplyEnv();

                    //osPrintf("About to launch: %s %s\n", rAppName.data(), cmdLine.data());
                    
                    //set the current dir for this process
                    OsFileSystem::change(startupDir);
                    //3...2...1...  Blastoff!
                    execvp(rAppName.data(), (char **) parms);
                    
                    //and it never reaches here hopefully
                    osPrintf("Failed to execute '%s'!\n", rAppName.data());
                    _exit(1);
        }
        default :   // this is the parent process
                    mPID = forkReturnVal;
                    mParentPID = getpid();
                    retval = OS_SUCCESS;
                    break;
    }

    delete[] parms;

    return retval;
}

/* ============================ ACCESSORS ================================= */
OsStatus OsProcessLinux::getByPID(PID pid, OsProcess &rProcess)
{
    OsStatus retval = OS_FAILED;
    OsProcess process;
    OsProcessIterator pi;
    
    char buf[PID_STR_LEN];
    sprintf(buf,"%ld",(long)pid);
    OsPath pidStr = buf;
    OsStatus findRetVal = pi.readProcFile(pidStr,process);
    
    if (findRetVal == OS_SUCCESS)
    {
            rProcess.mParentPID     = process.mParentPID;
            rProcess.mPID           = process.mPID;
            rProcess.mProcessName   = process.mProcessName;
            retval = OS_SUCCESS;
    }


    return retval;
}


OsStatus OsProcessLinux::getInfo(OsProcessInfo &rProcessInfo)
{
    OsStatus retval = OS_FAILED;
    
    OsProcess process;
    
    OsStatus findRetVal = getByPID(mPID,process);
    
    if(findRetVal == OS_SUCCESS)
    {
        rProcessInfo.parentProcessID = process.mParentPID;
        rProcessInfo.name = process.mProcessName;
        rProcessInfo.commandline = ""; //TODO
        rProcessInfo.prioClass = 0; //TODO
            
        retval = OS_SUCCESS;
    }

    return retval;
}


OsStatus OsProcessLinux::getUpTime(OsTime &rUpTime)
{
    OsStatus retval = OS_FAILED;


    return retval;
}

OsStatus OsProcessLinux::getPriorityClass(OsProcessPriorityClass &rPrioClass)
{
    OsStatus retval = OS_FAILED;

    return retval;
}

OsStatus OsProcessLinux::getMinPriority(int &rMinPrio)
{
    OsStatus retval = OS_FAILED;
    
    return retval;
}

OsStatus OsProcessLinux::getMaxPriority(int &rMaxPrio)
{
    OsStatus retval = OS_FAILED;

    return retval;
}

OsStatus OsProcessLinux::getPriority(int &rPrio)
{
    OsStatus retval = OS_FAILED;
    errno = 0;
    rPrio = getpriority(PRIO_PROCESS,mPID);
    if (errno == 0) 
    {
        retval = OS_SUCCESS;
    }
    return retval;
}

PID OsProcessLinux::getCurrentPID()
{
    return getpid();
}

/* ============================ INQUIRY =================================== */
UtlBoolean OsProcessLinux::isRunning() const
{
    UtlBoolean retval = FALSE;

    OsProcess rProcess;        
    if ( getByPID(mPID,rProcess) == OS_SUCCESS)
        retval = TRUE;

    return retval;   
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */



