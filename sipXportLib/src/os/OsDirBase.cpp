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
#include <stdlib.h>
#include <stdio.h>

#if defined(__linux__) || defined(sun) || defined(_VXWORKS)
 #include <unistd.h>
 #include <dirent.h>
#endif

#ifdef WIN32
 #include <direct.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>

// APPLICATION INCLUDES
#include "os/OsFS.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// DEFINES
#if defined(_WIN32)
   #define S_DIR _S_IFDIR
   #define S_READONLY S_IWRITE
#elif defined(__pingtel_on_posix__) || defined(_VXWORKS)
   #define S_DIR S_IFDIR
   #define S_READONLY S_IWUSR
#endif

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor

OsDirBase::OsDirBase(const char* pathname)
{
    mDirName = pathname;
}

OsDirBase::OsDirBase(const OsPathBase& pathname)
{
    mDirName = pathname;
}

// Copy constructor
OsDirBase::OsDirBase(const OsDirBase& rOsDirBase)
{
    mDirName = rOsDirBase.mDirName;
}

// Destructor
OsDirBase::~OsDirBase()
{
}

/* ============================ MANIPULATORS ============================== */
// Assignment operator
OsDirBase&
OsDirBase::operator=(const OsDirBase& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

   OsStatus OsDirBase::create() const
   {
       return OS_INVALID;
   }

   OsStatus OsDirBase::remove(UtlBoolean bRecursive, UtlBoolean bForce) const
   {
       return OsFileSystem::remove(mDirName,bRecursive, bForce);
   }

   OsStatus OsDirBase::rename(const char* name)
   {
       return OS_INVALID;
   }

/* ============================ ACCESSORS ================================= */



   void OsDirBase::getPath(OsPathBase& rOsPath) const
   {
       rOsPath = mDirName;
   }

/* ============================ INQUIRY =================================== */

  UtlBoolean OsDirBase::exists()
  {
      return FALSE;
  }



  OsStatus OsDirBase::getFileInfo(OsFileInfoBase& fileinfo)
  {
     OsStatus ret = OS_INVALID;

     struct stat stats;
     if (stat((char *)mDirName.data(), &stats) == 0)
     {
        ret = OS_SUCCESS;

        fileinfo.mbIsDirectory = (stats.st_mode & S_DIR) != 0;

        fileinfo.mbIsReadOnly = (stats.st_mode & S_READONLY) != 0;

        OsTime createTime(stats.st_ctime,0);
        fileinfo.mCreateTime = createTime;

        OsTime modifiedTime(stats.st_ctime,0);
        fileinfo.mCreateTime = modifiedTime;

        fileinfo.mSize = stats.st_size;
     }

     return ret;
  }

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
