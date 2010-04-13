//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsProcessIteratorBase_h_
#define _OsProcessIteratorBase_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "os/OsProcess.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//: Used to enumerate running processes

class OsProcessIteratorBase
{

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
   OsProcessIteratorBase();
     //:Default constructor

   OsProcessIteratorBase(const char* filterExp);
     //:Return processes filtered by name

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

    virtual OsStatus findFirst(OsProcess &rProcess) = 0;
    //: Start enumeration of running processes
    //: Returns OS_SUCCESS if found
    //: Returns OS_FAILED if none found.

    virtual OsStatus findNext(OsProcess &rProcess) = 0;
    //: Continues enumeration of running processes
    //: Returns OS_SUCCESS if found
    //: Returns OS_FAILED if none found.


/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   virtual ~OsProcessIteratorBase();
     //:Destructor

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    OsProcess mProcess;
    //:Last process found by this class

    UtlString mProcessNameFilter;
    //: Used to match enumerated files for filtering

    OsProcess::OsProcessPriorityClass prioFilterClass;
    //: Used to match enumerated files for filtering

};

/* ============================ INLINE METHODS ============================ */

// Depending on the native OS that we are running on, we include the class
// declaration for the appropriate lower level implementation and use a
// "typedef" statement to associate the OS-independent class name (OsProcess)
// with the OS-dependent realization of that type (e.g., OsMutexWnt).
#if defined(_WIN32)
#  include "os/Wnt/OsProcessIteratorWnt.h"
   typedef class OsProcessIteratorWnt OsProcessIterator;
#elif defined(_VXWORKS)
#  include "os/Vxw/OsProcessIteratorVxw.h"
   typedef class OsProcessIteratorVxw OsProcessIterator;
#elif defined(__hpux)
#  include "os/hpux/OsProcessIteratorHpux.h"
   typedef class OsProcessIteratorHpux OsProcessIterator;
#elif defined(__FreeBSD__)
#  include "os/freebsd/OsProcessIteratorFreeBSD.h"
   typedef class OsProcessIteratorFreeBSD OsProcessIterator;
#elif defined(__pingtel_on_posix__)
   // XXX This implementation is very Linux specific, __pingtel_on_posix__
   // is too generic a check.  It should check for some linux define instead.
#  include "os/linux/OsProcessIteratorLinux.h"
   typedef class OsProcessIteratorLinux OsProcessIterator;
#else
#  error Unsupported target platform.
#endif


#endif  // _OsProcessIteratorBase_h_
