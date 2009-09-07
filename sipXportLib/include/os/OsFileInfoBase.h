//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsFileInfoBase_h_
#define _OsFileInfoBase_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "os/OsTime.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class OsPathBase;

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class OsFileInfoBase
{
    friend class OsDirBase;
    friend class OsFileBase;
    friend class OsFileLinux;
    friend class OsFileVxw;
    friend class OsFileSystem;
    friend class OsDirWnt;
    friend class OsFileWnt;
    friend class OsFileSystemWnt;

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   OsFileInfoBase();
     //:Default constructor

   OsFileInfoBase(const OsFileInfoBase& rOsFileInfoBase);
     //:Copy constructor

   virtual
   ~OsFileInfoBase();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   OsFileInfoBase& operator=(const OsFileInfoBase& rhs);
       //:Assignment operator

/* ============================ ACCESSORS ================================= */
    OsStatus getCreateTime(OsTime& rTime) const;
      //: Returns the creation time in seconds since epoch
    OsStatus getModifiedTime(OsTime& rTime) const;
      //: Returns the modified time in seconds since epoch

    OsStatus getSize(unsigned long& rSize) const;
      //: Returns the entry size

/* ============================ INQUIRY =================================== */
    UtlBoolean isReadOnly() const;
      //: return TRUE if entry is readonly

    UtlBoolean isDir() const;
      //: return TRUE if entry is a directory

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    OsTime mCreateTime;
    OsTime mModifiedTime;
    UtlBoolean mbIsReadOnly;
    UtlBoolean mbIsDirectory;
    unsigned long mSize;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsFileInfoBase_h_
