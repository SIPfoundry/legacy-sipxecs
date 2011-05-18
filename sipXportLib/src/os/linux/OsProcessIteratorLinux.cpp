//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>
#include <stdlib.h>

// APPLICATION INCLUDES
#include "os/OsFS.h"
#include "os/linux/OsProcessIteratorLinux.h"
#include "os/OsLogger.h"


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsProcessIteratorLinux::OsProcessIteratorLinux() :
hProcessSnapshot(0)
{
   OsPath procDir = "/proc";

   mpFileIterator = new OsFileIterator(procDir);

}

// Destructor
OsProcessIteratorLinux::~OsProcessIteratorLinux()
{
    if (mpFileIterator)
        delete mpFileIterator;

}

/* ============================ MANIPULATORS ============================== */


/* ============================ ACCESSORS ================================= */
OsStatus OsProcessIteratorLinux::findFirst(OsProcess &rProcess)
{
    OsStatus retval = OS_FAILED;
    OsPath procName;
    OsStatus status = mpFileIterator->findFirst(procName,"^[0-9]+$",OsFileIterator::DIRECTORIES);
    if (status == OS_SUCCESS)
    {
       //this next line will fill in the needed members of the rProcess object
        if (readProcFile(procName,rProcess) == OS_SUCCESS)
        {
            retval = OS_SUCCESS;
        }
        else
        {
            osPrintf("ERROR: Couldn't read %s file!\n",procName.data());
        }
    }
    else
    {
        osPrintf("No Files found in ProcIterator::findfirst\n");
    }

    return retval;
}

OsStatus OsProcessIteratorLinux::findNext(OsProcess &rProcess)
{
    OsStatus retval = OS_FAILED;

    OsPath procName;

    if (mpFileIterator)
    {

        OsStatus status = mpFileIterator->findNext(procName);
        if (status != OS_FILE_NOT_FOUND)
        {
            //this next line will fill in the needed members of the rProcess object
            if (readProcFile(procName,rProcess) == OS_SUCCESS)
                retval = OS_SUCCESS;
            else
                osPrintf("ERROR: Couldn't read %s file!\n",procName.data());
        }
    }

    return retval;
}

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

//reads the status file in the /proc/nnnn directory
OsStatus OsProcessIteratorLinux::readProcFile(OsPath &procDirname, OsProcess & rProcess)
{
    OsStatus retval = OS_FAILED;
    OsPath fullProcName = "/proc/";
    fullProcName += procDirname;
    OsPath procCmdLine = fullProcName;
    procCmdLine += "/cmdline";
    fullProcName += "/status";

    // Get the process name.
    OsFileLinux procFile(fullProcName);
    if (procFile.open(OsFile::READ_ONLY) == OS_SUCCESS)
    {
        size_t len = 5000; //since the length is always 0 for these files, lets try to read 5k
        char *buffer = new char[len+1];
        if (buffer)
        {
            size_t bytesRead;
            procFile.read((void *)buffer,len,bytesRead);

            if (bytesRead)
            {
                procFile.close();
                //null-terminate the string
                buffer[bytesRead] = 0;
                //now parse the info we need
                char *ptr = strtok(buffer,"\n");
                while(ptr)
                {
                    if (memcmp(ptr,"Name:",5) == 0)
                    {
                        ptr +=5;

                        while (*ptr == ' ' || *ptr == 0x09)
                            ptr++;

                        rProcess.mProcessName = ptr;
                        rProcess.mProcessName.strip(UtlString::both, ' ');

                    }
                    else
                    if (memcmp(ptr,"Pid:",4) == 0)
                    {
                        rProcess.mPID = atol(ptr+4);
                    }
                    else
                    if (memcmp(ptr,"PPid:",5) == 0)
                    {
                        rProcess.mParentPID = atol(ptr+5);
                    }

                    ptr = strtok(NULL,"\n");
                }

                //say we are successful
                retval = OS_SUCCESS;
            }
            else
                osPrintf("Couldn't read bytes in readProcFile\n");

            delete [] buffer;
        }

        procFile.close();
    }

    OsFileLinux procCmdFile(procCmdLine);

    if (procCmdFile.open(OsFile::READ_ONLY) == OS_SUCCESS)
    {
        size_t len = 5000; //since the length is always 0 for these files, lets try to read 5k
        char *buffer = new char[len+1];
        if (buffer)
        {
            size_t bytesRead;
            procCmdFile.read((void *)buffer,len,bytesRead);

            if (bytesRead)
            {
                procCmdFile.close();
                //null-terminate the string
                buffer[bytesRead] = 0;

                // Full command may contain null between parms.  Substitute
                // the null will a space character.
                size_t substrlen;
                substrlen = strlen(buffer);
                while (substrlen < bytesRead)
                {
                   buffer[substrlen] = ' ';
                   substrlen = strlen(buffer);
                }
                rProcess.mProcessCmdLine = buffer;
                rProcess.mProcessCmdLine.strip(UtlString::both, ' ');

                retval = OS_SUCCESS;
            }
            else
                osPrintf("Couldn't read bytes in readProcFile\n");


            delete [] buffer;
        }
        procCmdFile.close();
    }

    return retval;
}

/* ============================ FUNCTIONS ================================= */
