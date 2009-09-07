//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsFileSystem_h_
#define _OsFileSystem_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDateTime.h"
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "os/OsFS.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Helper class that constructs OsDir and OsFile objects
//:for you.  This may be expanded to include enumerating versions
//:of these functions.

class OsFileSystem
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */


/* ============================ MANIPULATORS ============================== */

   OsFileSystem& operator=(const OsFileSystem& rhs);
     //:Assignment operator

   static OsStatus copy(const OsPath& rSource, const OsPath& rOsPath);
     //: Returns TRUE if file moved ok

   static OsStatus remove(const OsPath& rOsPath, UtlBoolean bRecursive = FALSE, UtlBoolean bForce = FALSE);
     //: Removes the directory or file specified by path
     //: Specify bForce = TRUE to remove if read-only


   static OsStatus rename(const OsPath& rSourceFile, const OsPath& rDestFile);
     //: Renames the directory or file specified by path

   static OsStatus change(const OsPath& rOsPath);
     //: Change the current working directory to the specified location

   static OsStatus createDir(const OsPath& rOsPath, const UtlBoolean createParent = FALSE);
     //: Creates the specified directory
     //: Fails if a file by the same name already exists in the directory

   static OsStatus setReadOnly(const OsPath& rFile, UtlBoolean isReadOnly);
     //: Sets the read-only flag onthe specified file.
     //: Set to TRUE to make the file READONLY


/* ============================ ACCESSORS ================================= */

   static OsStatus getFileInfo(OsPath& filespec, OsFileInfo& rfileInfo);
     //: Retrieve system info for specified directory of file

/* ============================ INQUIRY =================================== */
   static UtlBoolean exists(const OsPath& rFilename);
     //: Returns true if file exists

   static OsStatus getWorkingDirectory(OsPath& rOsPath);
     //: Returns the current working directory

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   OsFileSystem(const OsFileSystem& rOsFileSystem);
     //:Copy constructor

   OsFileSystem();
     //:Default constructor

   virtual ~OsFileSystem();
     //:Destructor

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   static OsStatus removeTree(const OsPath& rOsPath, UtlBoolean bForce = FALSE);
     //: Removes a directory, files and all sub-dirs
     //: Specify bForce = TRUE to remove files and directories
     //: even if read-only

   static OsStatus createDirRecursive(const OsPath& rOsPath);
     //: Recursively creates a directory and its parents if non-existant

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsFileSystem_h_
