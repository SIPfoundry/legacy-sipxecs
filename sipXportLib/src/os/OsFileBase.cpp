//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

//Uncomment next line to add syslog messages to debug OsFileBase
//#define DEBUG_FS

// SYSTEM INCLUDES
#include <stdio.h>

// APPLICATION INCLUDES
#include "os/OsTask.h"
#include "os/OsProcess.h"
#include "os/OsFS.h"
#include "os/OsLock.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const unsigned long CopyBufLen = 32768;
const unsigned long OsFileLockTimeout = 1000;
const PID INVALID_PID = 0;
#define READ_BUFFER_SIZE 1024

//needed this so vxworks macros will find OK for the stdio funcs
#ifdef _VXWORKS
#ifndef OK
#define OK      0
#endif /* OK */
#endif /* _VXWORKS */

// STATIC VARIABLE INITIALIZATIONS
/*
const int OsFileBase::READ_ONLY = 1;
const int OsFileBase::WRITE_ONLY = 2;
const int OsFileBase::READ_WRITE = 4;
const int OsFileBase::CREATE = 8;
const int OsFileBase::TRUNCATE = 16;
const int OsFileBase::APPEND = 32;
const int OsFileBase::FSLOCK_READ = 64;
const int OsFileBase::FSLOCK_WRITE = 128;
const int OsFileBase::FSLOCK_WAIT  = 256;
*/

//OsConfigDb stores filename+pid, and "RL" (readlock) or "WL" (writelock)
OsConfigDb *OsFileBase::mpFileLocks = NULL;

// Guard to protect Open getting call by multiple threads
OsBSem sOpenLock(OsBSem::Q_PRIORITY, OsBSem::FULL);

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsFileBase::OsFileBase(const OsPathBase& filename)
   : fileMutex(OsMutex::Q_FIFO),
     mOsFileHandle(NULL),
     mLocalLockThreadId(INVALID_PID),
     mFilename(filename)
{
   OsLock lock(fileMutex);

   if (mpFileLocks == NULL)
   {
      mpFileLocks = new OsConfigDb();
   }
}

// Copy constructor
OsFileBase::OsFileBase(const OsFileBase& rOsFileBase)
   : fileMutex(OsMutex::Q_FIFO),
     mFilename("")
{
#ifdef DEBUG_FS
   pthread_t nTaskId = 0;
   OsTask* pTask = OsTask::getCurrentTask();
   if (pTask) pTask->id(nTaskId);
   Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG, "OsFileBase::OsFileBase ENTER threadid=%ld\n", (long)nTaskId);
#endif
    OsPathBase path;
    rOsFileBase.getFileName(path);
    mFilename = path;
    mOsFileHandle = rOsFileBase.mOsFileHandle;
    mLocalLockThreadId = rOsFileBase.mLocalLockThreadId;
#ifdef DEBUG_FS
   Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG, "OsFileBase::OsFileBase EXIT threadid=%ld\n", (long)nTaskId);
#endif
}

// Destructor
OsFileBase::~OsFileBase()
{
#ifdef DEBUG_FS
   pthread_t nTaskId = 0;
   OsTask* pTask = OsTask::getCurrentTask();
   if (pTask) pTask->id(nTaskId);
   Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG, "OsFileBase::~OsFileBase ENTER threadid=%ld\n", (long)nTaskId);
#endif
    if (mOsFileHandle)
        close(); //call our close
#ifdef DEBUG_FS
   Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG, "OsFileBase::~OsFileBase EXIT threadid=%ld\n", (long)nTaskId);
#endif
}

/* ============================ MANIPULATORS ============================== */

OsStatus OsFileBase::setReadOnly(UtlBoolean isReadOnly)
{
#ifdef DEBUG_FS
   pthread_t nTaskId = 0;
   OsTask* pTask = OsTask::getCurrentTask();
   if (pTask) pTask->id(nTaskId);
   Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG, "OsFileBase::setReadOnly ENTER threadid=%ld\n", (long)nTaskId);
#endif

   OsStatus status = OsFileSystem::setReadOnly(mFilename,isReadOnly);

#ifdef DEBUG_FS
   Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG, "OsFileBase::setReadOnly EXIT threadid=%ld\n", (long)nTaskId);
#endif
    return status;
}

OsStatus OsFileBase::fileunlock()
{
#ifdef DEBUG_FS
   pthread_t nTaskId = 0;
   OsTask* pTask = OsTask::getCurrentTask();
   if (pTask) pTask->id(nTaskId);
   Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG, "OsFileBase::fileunlock ENTER threadid=%ld\n", (long)nTaskId);
#endif
    OsStatus retval = OS_SUCCESS;
    //no file locking on the base class (yet) (I'm doing linux and windows first)
#ifdef DEBUG_FS
   Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG, "OsFileBase::fileunlock EXIT threadid=%ld\n", (long)nTaskId);
#endif
    return retval;
}

OsStatus OsFileBase::filelock(const int mode)
{
#ifdef DEBUG_FS
   pthread_t nTaskId = 0;
   OsTask* pTask = OsTask::getCurrentTask();
   if (pTask) pTask->id(nTaskId);
   Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG, "OsFileBase::filelock ENTER threadid=%ld\n", (long)nTaskId);
#endif

   OsStatus retval = OS_SUCCESS;
    //no file locking on the base class (yet) (I'm doing linux and windows first)

#ifdef DEBUG_FS
   Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG, "OsFileBase::filelock EXIT threadid=%ld\n", (long)nTaskId);
#endif

   return retval;
}

OsStatus OsFileBase::open(const int mode)
{
#ifdef DEBUG_FS
   pthread_t nTaskId = 0;
   OsTask* pTask = OsTask::getCurrentTask();
   if (pTask) pTask->id(nTaskId);
   Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG, "OsFileBase::open ENTER threadid=%ld, filename=%s\n", (long)nTaskId, mFilename.data());
#endif
    //get a lock for the open call
         sOpenLock.acquire();

    OsStatus stat = OS_INVALID;
    const char* fmode = "";

    if (mode & CREATE)
    {
        fmode = "wb+";
    }

    if (mode & READ_ONLY)
        fmode = "rb";
    if (mode & WRITE_ONLY)
        fmode = "wb";
    if (mode & READ_WRITE)
        fmode = "rb+";
    if (mode & APPEND)
        fmode = "ab+";
    if (mode & TRUNCATE)
        fmode = "wb";

    mOsFileHandle = fopen(mFilename.data(),fmode);

#ifndef _VXWORKS     // 6/27/03 - Bob - Disabling locking under vxworks - crashes
    //success
    if (mOsFileHandle)
    {
#ifdef DEBUG_FS
        Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG, "OsFileBase::open fopen returned mOsFileHandle=0x%08x, fd=%d, threadid=%d, filename=%s\n", mOsFileHandle, fileno(mOsFileHandle), nTaskId, mFilename.data());
#endif
        //first test to see if we have a local file lock on that file
        //get the thread id for local locking
        mLocalLockThreadId = OsProcess::getCurrentPID();
        char* pLockName = new char[mFilename.length() + PID_STR_LEN];
        sprintf(pLockName, "%s%ld", mFilename.data(), (long)mLocalLockThreadId);

        UtlString rValue;
        if (getFileLocks()->get(pLockName,rValue) == OS_SUCCESS)
        {
            //if we want read and someone else is already reading then it's ok
            if (rValue == "RL" && (mode & READ_ONLY))
                stat = OS_SUCCESS;
            else
            if (rValue == "WL" && (mode & FSLOCK_WAIT))
            {

                //we need to wait until the lock is freed
                UtlBoolean lockFree = FALSE;
                do
                {
                    OsTask::delay(OsFileLockTimeout);
                    if (getFileLocks()->get(pLockName,rValue) != OS_SUCCESS)
                    {
                        lockFree = TRUE;
                        stat = OS_SUCCESS;
                    }
                } while (lockFree == FALSE);
            }
            else
            {
                fclose(mOsFileHandle);
                mOsFileHandle = NULL;
                mLocalLockThreadId = INVALID_PID;
                stat = OS_FILE_ACCESS_DENIED;
            }
        }
        else
        {
            rValue = "RL";
            if (mode & FSLOCK_WRITE)
                    rValue = "WL";
            getFileLocks()->set(pLockName,rValue);
            stat = OS_SUCCESS;
        }

        //if the lock is ok at this point, we need to get a cross process lock
        if (stat == OS_SUCCESS)
        {

            stat = filelock(mode); //returns success if no file sharing specified
            if (stat != OS_SUCCESS)
            {
                ::fclose(mOsFileHandle);
                mOsFileHandle = NULL;
                mLocalLockThreadId = INVALID_PID;
                stat = OS_FILE_ACCESS_DENIED;

                //remove local process locks
                getFileLocks()->remove(pLockName);

            }
        }
        delete[] pLockName;

    }
    else
    {
        switch(errno)
        {
        case EACCES:
        case EMFILE:
           stat = OS_FILE_ACCESS_DENIED;
           break;
        case ENOENT:
           stat = OS_FILE_NOT_FOUND;
           break;
        default:
           break;
        }
    }
#else
    if (mOsFileHandle)
    {
        stat = OS_SUCCESS;
    }
#endif

         sOpenLock.release();

#ifdef DEBUG_FS
   Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG, "OsFileBase::open EXIT threadid=%ld\n", nTaskId);
#endif

    return stat;
}

OsStatus OsFileBase::flush()
{
   OsLock lock(fileMutex);

#ifdef DEBUG_FS
   pthread_t nTaskId = 0;
   OsTask* pTask = OsTask::getCurrentTask();
   if (pTask) pTask->id(nTaskId);
   Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG, "OsFileBase::flush EXIT threadid=%ld\n", nTaskId);
#endif
    OsStatus stat = OS_INVALID;

    if (mOsFileHandle)
    {
      if (fflush(mOsFileHandle) == 0)
        stat = OS_SUCCESS;
    }

#ifdef DEBUG_FS
   Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG, "OsFileBase::flush EXIT threadid=%ld\n", nTaskId);
#endif
    return stat;
}


OsStatus OsFileBase::write(const void* buf, size_t buflen, size_t& bytesWritten)
{
   OsLock lock(fileMutex);

#ifdef DEBUG_FS
   pthread_t nTaskId = 0;
   OsTask* pTask = OsTask::getCurrentTask();
   if (pTask) pTask->id(nTaskId);
   Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG, "OsFileBase::write ENTER threadid=%ld\n", nTaskId);
#endif
    OsStatus stat = OS_INVALID;

    if (mOsFileHandle)
      bytesWritten = ::fwrite(buf,1,buflen,mOsFileHandle);

    if (bytesWritten == buflen)
        stat = OS_SUCCESS;

#ifdef DEBUG_FS
   Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG, "OsFileBase::write EXIT threadid=%ld\n", nTaskId);
#endif
    return stat;
}


OsStatus OsFileBase::setLength(unsigned long newLength)
{
#ifdef DEBUG_FS
   pthread_t nTaskId = 0;
   OsTask* pTask = OsTask::getCurrentTask();
   if (pTask) pTask->id(nTaskId);
   Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG, "OsFileBase::setLength ENTER threadid=%ld\n", nTaskId);
#endif
    OsStatus stat = OS_SUCCESS;

#ifdef DEBUG_FS
   Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG, "OsFileBase::setLength EXIT threadid=%ld\n", nTaskId);
#endif
    return stat;
}

OsStatus OsFileBase::setPosition(ssize_t pos, FilePositionOrigin origin)
{
    OsLock lock(fileMutex);

#ifdef DEBUG_FS
   pthread_t nTaskId = 0;
   OsTask* pTask = OsTask::getCurrentTask();
   if (pTask) pTask->id(nTaskId);
   Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG, "OsFileBase::setPosition ENTER threadid=%ld\n", nTaskId);
#endif
   OsStatus stat = OS_INVALID;

   if (mOsFileHandle)
   {
      int startloc = -1;

      if (origin == START)
         startloc = SEEK_SET;
      else
      if (origin == CURRENT)
         startloc = SEEK_CUR;
      else
      if (origin == END)
         startloc = SEEK_END;

      if (startloc != -1 && fseek(mOsFileHandle,pos,startloc) != -1)
         stat = OS_SUCCESS;
   }

#ifdef DEBUG_FS
   Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG, "OsFileBase::setPosition EXIT threadid=%ld\n", nTaskId);
#endif
   return stat;
}

OsStatus OsFileBase::getPosition(size_t &pos)
{
   OsLock lock(fileMutex);

   pos = (size_t) UTL_NOT_FOUND;

#ifdef DEBUG_FS
   pthread_t nTaskId = 0;
   OsTask* pTask = OsTask::getCurrentTask();
   if (pTask) pTask->id(nTaskId);
   Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG, "OsFileBase::getPosition ENTER threadid=%ld\n", nTaskId);
#endif

    OsStatus stat = OS_INVALID;

    if (mOsFileHandle)
    {
      pos = ftell(mOsFileHandle);
      if (pos != (size_t)UTL_NOT_FOUND)
      {
           stat = OS_SUCCESS;
      }
    }

#ifdef DEBUG_FS
   Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG, "OsFileBase::getPosition EXIT threadid=%ld\n", nTaskId);
#endif

    return stat;
}


OsStatus OsFileBase::remove(UtlBoolean bForce)
{
#ifdef DEBUG_FS
   pthread_t nTaskId = 0;
   OsTask* pTask = OsTask::getCurrentTask();
   if (pTask) pTask->id(nTaskId);
   Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG, "OsFileBase::remove ENTER threadid=%ld\n", nTaskId);
#endif
    OsStatus ret = OS_INVALID;
    //if it's open then close it
    close();

    ret = OsFileSystem::remove(mFilename.data(),FALSE,bForce);

#ifdef DEBUG_FS
   Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG, "OsFileBase::remove EXIT threadid=%ld\n", nTaskId);
#endif

    return ret;
}

OsStatus OsFileBase::rename(const OsPathBase& rNewFilename)
{
    OsLock lock(fileMutex);

#ifdef DEBUG_FS
   pthread_t nTaskId = 0;
   OsTask* pTask = OsTask::getCurrentTask();
   if (pTask) pTask->id(nTaskId);
   Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG, "OsFileBase::rename ENTER threadid=%ld\n", nTaskId);
#endif
    OsStatus ret = OS_INVALID;

    //if it's open then close it
    close();

    int err = ::rename(mFilename.data(),rNewFilename.data());
    if (err != -1)
    {
        ret = OS_SUCCESS;
    }
    else
    {
        // Rename failed, if there is a file that already exists having
        // the new name, try removing it first
        OsFile fileInPlace(rNewFilename);
        if(fileInPlace.exists())
        {
            fileInPlace.remove(TRUE);

            // Try the move again
            int err2 = ::rename(mFilename.data(),rNewFilename.data());
            if (err2 != -1)
            {
                ret = OS_SUCCESS;
            }
        }
    }
#ifdef DEBUG_FS
   Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG, "OsFileBase::rename EXIT threadid=%ld\n", nTaskId);
#endif
    return ret;
}


OsStatus OsFileBase::copy(const OsPathBase& newFilename)
{
#ifdef DEBUG_FS
   pthread_t nTaskId = 0;
   OsTask* pTask = OsTask::getCurrentTask();
   if (pTask) pTask->id(nTaskId);
   Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG, "OsFileBase::copy ENTER threadid=%ld\n", nTaskId);
#endif
    OsStatus ret = OS_FILE_WRITE_FAILED;
    size_t copySize = 0;
    size_t totalBytesRead = 0;
    size_t totalBytesWritten = 0;
    size_t BytesRead = 0;
    size_t BytesWritten = 0;
    UtlBoolean bError = FALSE;
    OsFile newFile(newFilename);

    char *buf = new char[CopyBufLen];
    if (buf)
    {
        //open existsing file
        if (open() == OS_SUCCESS)
        {
            if (getLength(copySize) == OS_SUCCESS)
            {

                //open new file
                newFile.open(CREATE);

                while ((totalBytesRead < copySize) && !bError)
                {
                    //read in one block
                    if (read(buf,CopyBufLen,BytesRead) == OS_SUCCESS)
                    {
                        totalBytesRead += BytesRead;
                        if (newFile.write(buf,BytesRead,BytesWritten) == OS_SUCCESS)
                        {
                            if (BytesWritten != BytesRead)
                            {
                                bError = TRUE;
                            }
                            else
                                totalBytesWritten += BytesWritten;
                        }
                        else
                        {
                            bError = TRUE;

                        }
                    }
                    else
                        bError = TRUE;
                }

                if (!bError)
                    ret = OS_SUCCESS;

                newFile.close();
            }

            close();

        }

        delete [] buf;
    }

#ifdef DEBUG_FS
   Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG, "OsFileBase::copy EXIT threadid=%ld\n", nTaskId);
#endif
    return ret;
}


OsStatus OsFileBase::touch()
{
#ifdef DEBUG_FS
   pthread_t nTaskId = 0;
   OsTask* pTask = OsTask::getCurrentTask();
   if (pTask) pTask->id(nTaskId);
   Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG, "OsFileBase::touch ENTER threadid=%ld\n", nTaskId);
#endif
    OsStatus stat = OS_INVALID;
    char buf[5];
    size_t BytesRead;
    size_t BytesWritten;

    //open read one byte, write it back, then close
    //hope this is what we need to do for touch
    if (exists() && open() == OS_SUCCESS)
    {
        if (read(buf,1,BytesRead) == OS_SUCCESS)
        {
            setPosition(0);
            if (write(buf,BytesRead,BytesWritten) == OS_SUCCESS)
                stat = OS_SUCCESS;
        }
        close();
    }
    else
    {
        open(CREATE);
        close();
        stat = OS_SUCCESS;
    }

#ifdef DEBUG_FS
   Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG, "OsFileBase::touch EXIT threadid=%ld\n", nTaskId);
#endif
    return stat;
}


/* ============================ ACCESSORS ================================= */

void OsFileBase::getFileName(OsPathBase& rOsPath) const
{
#ifdef DEBUG_FS
   pthread_t nTaskId = 0;
   OsTask* pTask = OsTask::getCurrentTask();
   if (pTask) pTask->id(nTaskId);
   Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG, "OsFileBase::getFileName ENTER threadid=%ld\n", nTaskId);
#endif

   rOsPath = mFilename;

#ifdef DEBUG_FS
   Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG, "OsFileBase::getFileName EXIT threadid=%ld\n", nTaskId);
#endif
}

OsStatus OsFileBase::read(void* buf, size_t buflen, size_t& bytesRead)
{
   OsLock lock(fileMutex);

#ifdef DEBUG_FS
   pthread_t nTaskId = 0;
   OsTask* pTask = OsTask::getCurrentTask();
   if (pTask) pTask->id(nTaskId);
   Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG, "OsFileBase::read ENTER threadid=%ld\n", nTaskId);
#endif
    OsStatus stat = OS_INVALID;

    if (mOsFileHandle)
    {
      bytesRead = ::fread(buf,1,buflen,mOsFileHandle);

      if (bytesRead == 0 && feof(mOsFileHandle))
         stat = OS_FILE_EOF;
      else
         stat = OS_SUCCESS;

      if (ferror(mOsFileHandle))
          stat = OS_FILE_INVALID_HANDLE;
    }
    else
        stat = OS_FILE_INVALID_HANDLE;


#ifdef DEBUG_FS
   Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG, "OsFileBase::read EXIT threadid=%ld\n", nTaskId);
#endif
    return stat;
}


OsStatus OsFileBase::readLine(UtlString &str)
{
#ifdef DEBUG_FS
   pthread_t nTaskId = 0;
   OsTask* pTask = OsTask::getCurrentTask();
   if (pTask) pTask->id(nTaskId);
   Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG, "OsFileBase::readLine ENTER threadid=%ld\n", nTaskId);
#endif
    char buf[2];
    size_t bytesRead;
    OsStatus retstat = OS_INVALID;

    buf[1] = '\0';
    str.remove(0);
    do
    {
        retstat = read(buf, 1, bytesRead);

        if (retstat == OS_SUCCESS && (*buf != '\n' && *buf != '\r'))
        {
            str.append(buf, 1);
        }

    } while (retstat == OS_SUCCESS && bytesRead == 1 && *buf != '\n');


#ifdef DEBUG_FS
   Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG, "OsFileBase::readLine EXIT threadid=%ld\n", nTaskId);
#endif

   return retstat;
}

UtlBoolean OsFileBase::close()
{
   OsLock lock(fileMutex);

#ifdef DEBUG_FS
   pthread_t nTaskId = 0;
   OsTask* pTask = OsTask::getCurrentTask();
   if (pTask) pTask->id(nTaskId);
   int fd = -1;
   if (mOsFileHandle) fd = fileno(mOsFileHandle);
   Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG, "OsFileBase::close ENTER mOsFileHandle=0x%08x, fd=%d, threadid=%ld, filename=%s\n", mOsFileHandle, fd, (long)nTaskId, mFilename.data());
#endif
    UtlBoolean retval = TRUE;

    if (mOsFileHandle)
    {
#ifndef _VXWORKS     // 6/27/03 - Bob - Disabling locking under vxworks - crashes
        // get the thread ID for local locking
        char* pLockName = new char[mFilename.length() + PID_STR_LEN];
        sprintf(pLockName, "%s%ld", mFilename.data(), (long)mLocalLockThreadId);

        // remove any local process locks
        getFileLocks()->remove(pLockName);
        mLocalLockThreadId = INVALID_PID;
        delete[] pLockName;
#endif

        if (::fclose(mOsFileHandle) != 0)
        {
            retval = FALSE;
            Os::Logger::instance().log(FAC_KERNEL, PRI_WARNING,
                          "OsFileBase::close failed, mOsFileHandle=%p, errno=%d '%s'",
                          mOsFileHandle, errno, strerror(errno));
        }
        mOsFileHandle = 0;
    }

    //remove any process locks
    fileunlock();


#ifdef DEBUG_FS
   Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG, "OsFileBase::close EXIT threadid=%ld\n", (long)nTaskId);
#endif

    return retval;
}

/* ============================ INQUIRY =================================== */
UtlBoolean OsFileBase::isEOF()
{
   OsLock lock(fileMutex);

#ifdef DEBUG_FS
   pthread_t nTaskId = 0;
   OsTask* pTask = OsTask::getCurrentTask();
   if (pTask) pTask->id(nTaskId);
   Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG, "OsFileBase::isEOF ENTER threadid=%ld\n", (long)nTaskId);
#endif
   UtlBoolean retval = FALSE;

   if (mOsFileHandle)
   {
      if (feof(mOsFileHandle))
         retval = TRUE;
   }

#ifdef DEBUG_FS
   Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG, "OsFileBase::isEOF EXIT threadid=%ld\n", (long)nTaskId);
#endif

   return retval;
}

UtlBoolean OsFileBase::isReadonly() const
{
#ifdef DEBUG_FS
   pthread_t nTaskId = 0;
   OsTask* pTask = OsTask::getCurrentTask();
   if (pTask) pTask->id(nTaskId);
   Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG, "OsFileBase::isReadonly ENTER threadid=%ld\n", (long)nTaskId);
#endif

    OsFileInfoBase info;
    getFileInfo(info);

#ifdef DEBUG_FS
   Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG, "OsFileBase::isReadonly EXIT threadid=%ld\n", (long)nTaskId);
#endif

    return info.mbIsReadOnly;
}

OsStatus OsFileBase::getLength(size_t& flength)
{
#ifdef DEBUG_FS
   pthread_t nTaskId = 0;
   OsTask* pTask = OsTask::getCurrentTask();
   if (pTask) pTask->id(nTaskId);
   Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG, "OsFileBase::getLength ENTER threadid=%ld\n", (long)nTaskId);
#endif
    OsStatus ret = OS_INVALID;
    size_t saved_pos;

    if (getPosition(saved_pos) == OS_SUCCESS)
    {
        if (setPosition(0,END) == OS_SUCCESS)
        {
            if (getPosition(flength) == OS_SUCCESS)
            {
                if (setPosition(saved_pos,START) == OS_SUCCESS)
                {
                    ret = OS_SUCCESS;
                }
            }
        }
    }

#ifdef DEBUG_FS
   Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG, "OsFileBase::getLength EXIT threadid=%ld\n", (long)nTaskId);
#endif

   return ret;
}

UtlBoolean OsFileBase::exists()
{
#ifdef DEBUG_FS
   pthread_t nTaskId = 0;
   OsTask* pTask = OsTask::getCurrentTask();
   if (pTask) pTask->id(nTaskId);
   Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG, "OsFileBase::exists ENTER threadid=%ld\n", (long)nTaskId);
#endif
    UtlBoolean stat = FALSE;

    OsFileInfo info;
    OsStatus retval = getFileInfo(info);
    if (retval == OS_SUCCESS)
        stat = TRUE;

#ifdef DEBUG_FS
   Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG, "OsFileBase::exists EXIT threadid=%ld\n", (long)nTaskId);
#endif

   return stat;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
// Assignment operator
OsFileBase&
OsFileBase::operator=(const OsFileBase& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */
