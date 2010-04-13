//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsFileInfo_h_
#define _OsFileInfo_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "utl/UtlDefs.h"
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "os/OsTime.h"
#include "os/OsFileInfoBase.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class OsPathWnt;
class OsFileInfoBase;

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class OsFileInfoWnt : public OsFileInfoBase
{
    friend class OsDirBase;
    friend class OsFileBase;
    friend class OsFileSystem;
    friend class OsDirWnt;
    friend class OsFileWnt;
    friend class OsFileSystemWnt;
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   OsFileInfoWnt();
     //:Default constructor

   OsFileInfoWnt(const OsFileInfoWnt& rOsFileInfo);
     //:Copy constructor

   virtual
   ~OsFileInfoWnt();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   OsFileInfoWnt& operator=(const OsFileInfoWnt& rhs);
       //:Assignment operator


/* ============================ INQUIRY =================================== */
    UtlBoolean isReadOnly() const;
      //: return TRUE if entry is readonly

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsFileInfo_h_
