//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsProcessIteratorWnt_h_
#define _OsProcessIteratorWnt_h_

// SYSTEM INCLUDES
#include <windows.h>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "os/OsProcess.h"
#include "os/OsProcessIterator.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class OsProcessBase;
class OsProcessIteratorBase;

//: Used to enumerate running processes

class OsProcessIteratorWnt : OsProcessIteratorBase
{

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
   OsProcessIteratorWnt();
     //:Default constructor

   virtual ~OsProcessIteratorWnt();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

    OsStatus findFirst(OsProcess &rProcess);
    //: Start enumeration of running processes
    //: Returns OS_SUCCESS if found
    //: Returns OS_FAILED if none found.

    OsStatus findNext(OsProcess &rProcess);
    //: Continues enumeration of running processes
    //: Returns OS_SUCCESS if found
    //: Returns OS_FAILED if none found.


/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    OsProcess mProcess;
    HANDLE hProcessSnapshot;
    //:Last process found by this class

};

/* ============================ INLINE METHODS ============================ */



#endif  // _OsProcessIteratorWnt_h_
