//
// Copyright (C) 2007 Hewlett-Packard Development Company, L.P.
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
////////////////////////////////////////////////////////////////////////

#ifndef _OsProcessIteratorHpux_h_
#define _OsProcessIteratorHpux_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "os/OsProcess.h"
#include "os/OsProcessIterator.h"
#include "os/OsFS.h"

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

class OsProcessIteratorHpux : OsProcessIteratorBase
{

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

    friend class OsProcessLinux;

/* ============================ CREATORS ================================== */
   OsProcessIteratorHpux();
     //:Default constructor

   OsProcessIteratorHpux(const char* filterExp);

     //:Return processes filtered by name

   virtual ~OsProcessIteratorHpux();
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
    OsStatus readProcFile(OsPath &procDirname, OsProcess & rProcess);


    OsProcessLinux mProcess;
    int hProcessSnapshot;
    int idx;

    //:Last process found by this class

};

/* ============================ INLINE METHODS ============================ */



#endif  // _OsProcessIteratorHpux_h_
