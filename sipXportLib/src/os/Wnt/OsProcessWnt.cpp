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
#include <windows.h>
#include <process.h>
#include <tlhelp32.h>

// APPLICATION INCLUDES
#include "os/OsProcess.h"
#include "os/wnt/OsProcessWnt.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

OsProcessWnt::OsProcessWnt() :
mStdErrorHandle(0),mStdOutputHandle(0),
mStdInputHandle(0),
mhProcess(0),
mhThread(0)
{
}

// Destructor
OsProcessWnt::~OsProcessWnt()
{
    if (mStdErrorHandle != NULL && mStdErrorHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(mStdErrorHandle);
    }
    if (mStdOutputHandle != NULL && mStdOutputHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(mStdOutputHandle);
    }
    if (mStdInputHandle != NULL && mStdInputHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(mStdInputHandle);
    }
    if (mhThread)
    {
        CloseHandle(mhThread);
    }
   if (mhProcess)
    {
        CloseHandle(mhProcess);
    }
}


/* ============================ MANIPULATORS ============================== */

OsStatus OsProcessWnt::setIORedirect(OsPath &rStdInputFilename, OsPath &rStdOutputFilename, OsPath &rStdErrorFilename)
{
    OsStatus retval = OS_FAILED;
    UtlBoolean bOneFailed = FALSE;

    if (!rStdInputFilename.isNull())
    {
        mStdInputHandle = CreateFile(rStdInputFilename.data(),
                            GENERIC_READ,                       // access mode
                            FILE_SHARE_READ,                     // share mode
                            0,                                  // SD
                            OPEN_EXISTING,                      // how to create
                            FILE_ATTRIBUTE_NORMAL,              // file attributes
                            NULL);                             // handle to template file
        if (mStdInputHandle == INVALID_HANDLE_VALUE)
        {
            osPrintf("Could not open input file for Std Input for new process\n");
            bOneFailed = TRUE;
        }
        else
        {
            mStdInputFilename = rStdInputFilename;
        }
    }

    if (!rStdErrorFilename.isNull())
    {
        mStdErrorHandle = CreateFile(rStdErrorFilename.data(),
                            GENERIC_WRITE,                       // access mode
                            FILE_SHARE_READ,                     // share mode
                            0,                                  // SD
                            OPEN_ALWAYS,                        // how to create
                            FILE_ATTRIBUTE_NORMAL,              // file attributes
                            NULL);                              // handle to template file
        if (mStdErrorHandle == INVALID_HANDLE_VALUE)
        {
            osPrintf("Could not open %s for Std Error on new process\n");
            bOneFailed = TRUE;
        }
        else
        {
            mStdErrorFilename = rStdErrorFilename;
        }
    }

    if (!rStdOutputFilename.isNull())
    {
        mStdOutputHandle = CreateFile(rStdOutputFilename.data(),
                            GENERIC_WRITE,                       // access mode
                            FILE_SHARE_READ,                     // share mode
                            0,                                  // SD
                            OPEN_ALWAYS,                        // how to create
                            FILE_ATTRIBUTE_NORMAL,              // file attributes
                            NULL);                              // handle to template file
        if (mStdOutputHandle == INVALID_HANDLE_VALUE)
        {
            osPrintf("Could not open %s for Std Error on new process\n");
            bOneFailed = TRUE;
        }
        else
        {
            mStdOutputFilename = rStdOutputFilename;
        }
    }

    if (!bOneFailed)
        retval = OS_SUCCESS;
    else
    {
        if (mStdErrorHandle != INVALID_HANDLE_VALUE)
            CloseHandle(mStdErrorHandle);
        if (mStdOutputHandle != INVALID_HANDLE_VALUE)
            CloseHandle(mStdOutputHandle);
        if (mStdInputHandle != INVALID_HANDLE_VALUE)
            CloseHandle(mStdInputHandle);
    }

    return retval;
}

OsStatus OsProcessWnt::setPriority(int prio)
{
    OsStatus retval = OS_FAILED;

    return retval;
}

OsStatus OsProcessWnt::kill()
{
    OsStatus retval = OS_FAILED;
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE,FALSE,mPID);

    if (hProcess)
    {
        if (TerminateProcess(hProcess,0))
            retval = OS_SUCCESS;

        int trycount = 0;

        while (isRunning() && trycount++ < 30)
        {
            if (TerminateProcess(hProcess,0))
                retval = OS_SUCCESS;
        }
    }

    CloseHandle(hProcess);

    return retval;
}

//waits for a process to complete before returning
//or exits when WaitInSecs has completed
int OsProcessWnt::wait(int WaitInSecs)
{
    DWORD ExitCode = STILL_ACTIVE;
    int secs_waited = 0;

    if (WaitInSecs < 0)
        WaitInSecs = 0;

    if (mhProcess)
    {
        while (ExitCode == STILL_ACTIVE && secs_waited <= WaitInSecs)
        {
            if (GetExitCodeProcess(mhProcess,
                        &ExitCode))
            {
            }
            else
                osPrintf("Error getting exitcode for process %d\n",mPID);

            if (ExitCode == STILL_ACTIVE)
            {
                Sleep(1000);

                if (WaitInSecs >  0)
                    secs_waited++;
            }

        }
    }
    else
        ExitCode = -1;

    return ExitCode;
}

OsStatus OsProcessWnt::launch(UtlString &rAppName, UtlString parameters[],OsPath &startupDir,
                    OsProcessPriorityClass prioClass, UtlBoolean bExeclusive, UtlBoolean bIgnoreChildSignals)
{
    OsStatus retval = OS_FAILED;
        STARTUPINFO StartupInfo;
        PROCESS_INFORMATION ProcessInformation;
    UtlString cmdLine = startupDir + OsPath::separator;
        cmdLine += rAppName;
/*
    int saved_stderr = dup(2);
    int saved_stdout = dup(1);
    int saved_stdin  = dup(0);
*/
    //build one string out of the array passed in
    int parameterCount = 0;
    while (!parameters[parameterCount].isNull())
    {
        if(parameters[parameterCount].index(" ") != UTL_NOT_FOUND)
            parameters[parameterCount] = "\"" + parameters[parameterCount] + "\"";
        cmdLine.append(" ");
        cmdLine.append(parameters[parameterCount]);
        parameterCount++;
    }
        //clear out structure
        memset(&StartupInfo,'\0',sizeof(STARTUPINFO));

        StartupInfo.cb = sizeof(STARTUPINFO);
        StartupInfo.lpReserved = NULL;
        StartupInfo.wShowWindow = SW_MINIMIZE|SW_HIDE;
        StartupInfo.lpDesktop = NULL;
        StartupInfo.lpTitle = NULL;

    StartupInfo.dwFlags |= STARTF_USESTDHANDLES;

    //now it's time to redirect the output,input and error streams
    if (mStdErrorFilename.length())
        StartupInfo.hStdError  = mStdErrorHandle;
    else
        StartupInfo.hStdError =GetStdHandle(STD_ERROR_HANDLE);

    if (mStdInputFilename.length())
        StartupInfo.hStdInput  = mStdInputHandle;
    else
        StartupInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);

    if (mStdOutputFilename.length())
        StartupInfo.hStdOutput = mStdOutputHandle;
    else
        StartupInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);

    //now apply the env variables the user may have set
    ApplyEnv();


    //3...2...1...  LAUNCH!!!!
        int retcode = CreateProcess(
                                NULL,
                // name of executable module (null because we want to execute all commands)
                // even things such as dir
                                (char *)cmdLine.data(),       // command line string
                                NULL,
                                NULL,
                //this originally was TRUE but the web browser was never coming back.
                                FALSE,       // handle inheritance flag
//                CREATE_NEW_CONSOLE,
                                CREATE_NO_WINDOW | DETACHED_PROCESS,      // creation flags
                                NULL,       // new environment block
                                startupDir.data(), // startupdir
                                &StartupInfo,
                                &ProcessInformation
                                );

    if (retcode != 0)
    {
        //translate the incoming priority to Wnt values
        DWORD wntPrio = NORMAL_PRIORITY_CLASS;

        switch(prioClass)
        {
            case IdlePriorityClass:     wntPrio = IDLE_PRIORITY_CLASS;
                                        break;
            case NormalPriorityClass:   wntPrio = NORMAL_PRIORITY_CLASS;
                                        break;
            case HighPriorityClass:     wntPrio = HIGH_PRIORITY_CLASS;
                                        break;
            case RealtimePriorityClass: wntPrio = REALTIME_PRIORITY_CLASS;
                                        break;
            default:                    osPrintf("**** Invalid process priority class specified!\n");
                                        osPrintf("***  Defaulting to NormalPriorityClass *** \n");
                                        break;
        }

        if (!SetPriorityClass(ProcessInformation.hProcess, wntPrio))
        {
            osPrintf("*** Could not change the process priority on launch ***\n");
            osPrintf("*** Priority will be the parents priority ! ***\n");
        }

        if (bExeclusive)
        {
            //here is where we check if a process by the same name is already running
        }

        mPID = ProcessInformation.dwProcessId;
        mParentPID = getpid();
        mhProcess = ProcessInformation.hProcess;
        mhThread = ProcessInformation.hThread;
        retval = OS_SUCCESS;
    }
    else
    {

        LPVOID lpMsgBuf;
        FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            GetLastError(),
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
            (LPTSTR) &lpMsgBuf,
            0,
            NULL
            );
        osPrintf("***** ERROR FROM LAUNCH: %s",(LPCTSTR)lpMsgBuf);
        // Free the buffer.
        LocalFree( lpMsgBuf );

    }
/*
        dup2(saved_stderr,2);
        dup2(saved_stdout,1);
        dup2(saved_stdin, 0);
*/
    return retval;
}

/* ============================ ACCESSORS ================================= */
OsStatus OsProcessWnt::getByPID(PID pid, OsProcess &rProcess)
{
    OsStatus retval = OS_FAILED;
    OsProcess process;
    OsProcessIterator pi;

    OsStatus findRetVal = pi.findFirst(process);

    while (findRetVal == OS_SUCCESS)
    {
        if (process.getPID() == pid)
        {
            rProcess.mParentPID     = process.mParentPID;
            rProcess.mPID           = process.mPID;
            rProcess.mProcessName   = process.mProcessName;
            retval = OS_SUCCESS;
            break;
        }
        findRetVal = pi.findNext(process);
    }


    return retval;
}


OsStatus OsProcessWnt::getInfo(OsProcessInfo &rProcessInfo)
{
    OsStatus retval = OS_FAILED;

    HANDLE hProcessSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,mPID);
    if (hProcessSnapshot)
    {
        PROCESSENTRY32 pe;
        if (Process32First(hProcessSnapshot,&pe))
        {
            rProcessInfo.parentProcessID = mParentPID;
            rProcessInfo.name = pe.szExeFile;
            rProcessInfo.commandline = mParameters;
            rProcessInfo.prioClass = pe.pcPriClassBase;

            retval = OS_SUCCESS;
        }

        CloseHandle(hProcessSnapshot);
    }

    return retval;
}


OsStatus OsProcessWnt::getUpTime(OsTime &rUpTime)
{
    OsStatus retval = OS_FAILED;

    rUpTime = OsTime::OS_INFINITY;

    FILETIME creationTime;
    FILETIME exitTime;     // process exit time
    FILETIME kernelTime;   // process kernel-mode time
    FILETIME userTime;      // process user-mode time

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION,FALSE,mPID);
    if (GetProcessTimes(hProcess, &creationTime, &exitTime, &kernelTime, &userTime))
    {

        retval = OS_SUCCESS;
        CloseHandle(hProcess);
    }

    return retval;
}

OsStatus OsProcessWnt::getPriorityClass(OsProcessPriorityClass &rPrioClass)
{
    OsStatus retval = OS_FAILED;
    OsProcessInfo processInfo;

    if (getInfo(processInfo))
    {
        switch(processInfo.prioClass)
        {
            case IDLE_PRIORITY_CLASS:
                    break;

            case NORMAL_PRIORITY_CLASS:
                    break;

            case HIGH_PRIORITY_CLASS:
                    break;

            case REALTIME_PRIORITY_CLASS:
                    break;
        }
    }

    return retval;
}

OsStatus OsProcessWnt::getMinPriority(int &rMinPrio)
{
    OsStatus retval = OS_FAILED;
    OsProcessInfo processInfo;

    if (getInfo(processInfo))
    {
        switch(processInfo.prioClass)
        {
            case IDLE_PRIORITY_CLASS:
                    break;

            case NORMAL_PRIORITY_CLASS:
                    break;

            case HIGH_PRIORITY_CLASS:
                    break;

            case REALTIME_PRIORITY_CLASS:
                    break;
        }
    }

    return retval;
}

OsStatus OsProcessWnt::getMaxPriority(int &rMaxPrio)
{
    OsStatus retval = OS_FAILED;



    return retval;
}

OsStatus OsProcessWnt::getPriority(int &rPrio)
{
    OsStatus retval = OS_FAILED;

    return retval;
}

int OsProcessWnt::getCurrentPID()
{
    return _getpid();
}

/* ============================ INQUIRY =================================== */
UtlBoolean OsProcessWnt::isRunning() const
{
    UtlBoolean retval = FALSE;

    DWORD ExitCode = 0;
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION,TRUE,mPID);

    if (hProcess)
    {
        if (GetExitCodeProcess(hProcess,&ExitCode))
        {
            if (ExitCode == STILL_ACTIVE)
                retval = TRUE;
        }
        CloseHandle(hProcess);
    }


    return retval;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
