//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsDir_h_
#define _OsDir_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "utl/UtlDefs.h"
#include "os/OsStatus.h"
#include "os/OsDefs.h"
#include "os/OsDirBase.h"
#include "os/wnt/OsPathWnt.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class OsFileInfoBase;
class OsDirBase;
class OsPathWnt;
class OsFileInfoWnt;

//:Abstraction class to hande directory manipulations
class OsDirWnt : public OsDirBase
{

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */


   OsDirWnt(const char* pathname);
   OsDirWnt(const OsPathWnt& rOsPath);

   OsDirWnt(const OsDirWnt& rOsDir);
     //:Copy constructor

   virtual
   ~OsDirWnt();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   OsStatus create() const;
     //: Create the path specified by this object
     //  Returns OS_SUCCESS if successful, or OS_INVALID

   OsStatus rename(const char* name);
     //: Renames the current directory to the name specified
     //  Returns:
     //         OS_SUCCESS if successful
     //         OS_INVALID if failed

/* ============================ ACCESSORS ================================= */



/* ============================ INQUIRY =================================== */

   UtlBoolean exists();
     //: Returns TRUE if the directory specified by this object exists

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   OsDirWnt();
     //:Default constructor

   OsDirWnt& operator=(const OsDirWnt& rhs);
     //:Assignment operator

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsDir_h_
