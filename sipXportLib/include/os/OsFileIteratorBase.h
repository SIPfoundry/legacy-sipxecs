//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsFileIteratorBase_h_
#define _OsFileIteratorBase_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "os/OsPathBase.h"
#include <utl/UtlRegex.h>

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

//:Abstraction class to iterate through files and/or directories
class OsFileIteratorBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
    enum OsFileType
    {
        ANY_FILE,
        DIRECTORIES,
        FILES
    };

#ifdef _WIN32
    enum { INVALID_HANDLE = -1 };
#else
    enum { INVALID_HANDLE = 0  };
#endif



    //: type specified for FindFirst
    //!enumcode: ANY_FILE - Directories and Files
    //!enumcode: DIRECTORY - Search for directories only
    //!enumcode: FILE - Search for files only

/* ============================ CREATORS ================================== */

   OsFileIteratorBase();

   OsFileIteratorBase(const OsPathBase& rPathName);

    virtual ~OsFileIteratorBase();
     //:Destructor


/* ============================ MANIPULATORS ============================== */


/* ============================ ACCESSORS ================================= */

    virtual OsStatus findFirst(OsPathBase& rEntry,  const char* filterExp = ".*",
                               OsFileType fileType = ANY_FILE);
      //: Searches a directory specified by rEntry for all entries matching
      //: the (unanchored) regexp filterExp and also of type fileType.
      //: Returns the full path name of the found entries.
      //: filterExp is unanchored; it need only match a substring of the
      //: file name.  To force it to match the entire file name, use "^...$".
      //: On Unix-like systems the "." and ".." entries may be returned.

    virtual OsStatus findNext(OsPathBase& rEntry);
      //: Finds the next entry matching the search criteria.
      //: Use FindFirst before calling this function.


/* ============================ INQUIRY =================================== */
    int getFileCount() {return mFileCount;}
    //: Returns total files enumerated thus far.

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    OsPathBase  mUserSpecifiedPath;
      //: What the user passed in as path to search
    OsPathBase  mFullSearchSpec;
      //: What is searched against the filesystem
    RegEx*      mFilterExp;
      //: The regular expression that the user searched for

    long mSearchHandle;

    // release memory that allocated for mFilterExp
    //
    // Morerover, OsFileIteratorBase's Subclass needs override this
    // function for releasing mSearchHandle.
    virtual void Release();

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    virtual OsStatus getNextEntryName(UtlString &rName, OsFileType &rFileType);
      //: Platform dependant call for getting entry
    virtual OsStatus getFirstEntryName(UtlString &rName, OsFileType &rFileType);
      //: Platform dependant call for getting entry

    OsFileType mMatchAttrib;
      //: Attributes for file matching

    long mFileCount;
      //: How many file did this class find

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsFileIteratorBase_h_
