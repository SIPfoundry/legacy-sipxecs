//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsFileSystemWnt_h_
#define _OsFileSystemWnt_h_

// SYSTEM INCLUDES
#include <stdio.h>
#if defined(_WIN32)
#   include <io.h>
#   include <direct.h>
#   include <share.h>
#elif defined(_VXWORKS)
#   include <unistd.h>
#   include <dirent.h>
#elif defined(__pingtel_on_posix__)
#   include <unistd.h>
#   include <stdlib.h>
#   define O_BINARY 0 // There is no notion of a "not binary" file under POSIX,
                      // so we just set O_BINARY used below to no bits in the mask.
#else
#   error Unsupported target platform.
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

// APPLICATION INCLUDES
#include "os/OsFS.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class OsPathWnt;
class OsFileInfoWnt;

//:Helper class that constructs OsDir and OsFile objects
//:for you.  This may be expanded to include enumerating versions
//:of these functions.

class OsFileSystemWnt
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */


/* ============================ MANIPULATORS ============================== */

   OsFileSystemWnt& operator=(const OsFileSystemWnt& rhs);
     //:Assignment operator

   static OsStatus copy(const OsPathWnt& rSource, const OsPathWnt& rOsPath);
     //: Returns TRUE if file moved ok

   static OsStatus rename(const OsPathWnt& rSourceFile, const OsPathWnt& rDestFile);
     //: Renames the directory or file specified by path

   static OsStatus change(const OsPathWnt& rOsPath);
     //: Change the current working directory to the specified location

   static OsStatus createDir(const OsPathWnt& rOsPath);
     //: Creates the specified directory
     //: Fails if a file by the same name already exists in the directory

   static OsStatus setReadOnly(const OsPathWnt& rOsPath, UtlBoolean bState);
     //: Sets the specifed file or path to readonly

/* ============================ ACCESSORS ================================= */

   static OsStatus getFileInfo(OsPathBase& filespec, OsFileInfoBase& rfileInfo);
     //: Retrieve system info for specified directory of file

   static OsStatus OsFileSystemWnt::getWorkingDirectory(OsPathWnt& rPath);
     //: returns the current working directory for the process

/* ============================ ACCESSORS ================================= */
   static OsStatus OsFileSystemWnt::getFileInfo(OsPathWnt& rFilespec, OsFileInfoWnt& rFileInfo);
     //: Retrieve system info for specified directory of file
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   OsFileSystemWnt(const OsFileSystemWnt& rOsFileSystemWnt);
     //:Copy constructor

   OsFileSystemWnt();
     //:Default constructor

   virtual ~OsFileSystemWnt();
     //:Destructor

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:


};

/* ============================ INLINE METHODS ============================ */



#endif  // _OsFileSystemWnt_h_
