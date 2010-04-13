//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#ifndef _OsPathLinux_h_
#define _OsPathLinux_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "os/OsPathBase.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class OsPathBase;

//:OS generic path class.  Will massage any input string so separators are correct.
//:Also provided functions to
class OsPathLinux : public OsPathBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:


/* ============================ CREATORS ================================== */

   OsPathLinux();
     //:Default constructor

   OsPathLinux(const OsPathLinux& rOsPathLinux);
     //:Copy constructor

   virtual
   ~OsPathLinux();
     //:Destructor

   OsPathLinux(const UtlString& rPath);
     //: Copy contructor

   OsPathLinux(const char* pPath);
     //: Construct OsPathLinux from char*

   OsPathLinux(const UtlString& rVolume, const UtlString& rDirName, const UtlString& rFileName,
           const UtlString& rExtension);
     //: Forms a OsPathLinux from discrete parts

/* ============================ MANIPULATORS ============================== */

    OsPathLinux& operator=(const OsPathLinux& rhs);
      //:Assignment operator

/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:


};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsPathLinux_h_
