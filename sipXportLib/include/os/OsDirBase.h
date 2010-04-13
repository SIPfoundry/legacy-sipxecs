//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _OsDirBase_h_
#define _OsDirBase_h_

// SYSTEM INCLUDES
#include "os/OsStatus.h"
#include "os/OsDefs.h"
#include "os/OsFS.h"

// APPLICATION INCLUDES

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class OsFileInfoBase;
class OsPathBase;
//:Abstraction class to hande directory manipulations
class OsDirBase
{

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */


   OsDirBase(const char* pathname);
   OsDirBase(const OsPathBase& rOsPath);

   OsDirBase(const OsDirBase& rOsDir);
     //:Copy constructor

   virtual
   ~OsDirBase();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   virtual OsStatus create() const;
     //: Create the path specified by this object
     //  Returns OS_SUCCESS if successful, or OS_INVALID

   virtual OsStatus remove(UtlBoolean bRecursive, UtlBoolean bForce) const;
     //: Removes the directory name specified by this object
     //: Set bForce to TRUE to remove read-only directory
     //: Set bRecursive to TRUE remove sub-directories
     //  Returns:
     //         OS_SUCCESS if successful
     //         OS_FILE_ACCESS_DENIED if directory is in use or contains files
     //         OS_FILE_PATH_NOT_FOUND if specifed directory is not found

   virtual OsStatus rename(const char* name);
     //: Renames the current directory to the name specified
     //  Returns:
     //         OS_SUCCESS if successful
     //         OS_INVALID if failed

/* ============================ ACCESSORS ================================= */

   virtual OsStatus getFileInfo(OsFileInfoBase& rFileInfo);
     //: Returns the file information for this objects path (see
     //:        OsFileInfo for more detail)
     //  Returns:
     //         OS_SUCCESS if successful
     //         OS_INVALID if failed

   virtual void getPath(OsPathBase& rOsPath) const;
     //: Returns a reference to the full path stored in this object

/* ============================ INQUIRY =================================== */

   virtual UtlBoolean exists();
     //: Returns TRUE if the directory specified by this object exists

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   OsDirBase& operator=(const OsDirBase& rhs);
     //:Assignment operator

   OsPathBase mDirName;
      //:Directory name

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsDirBase_h_
