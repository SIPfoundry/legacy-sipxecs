#ifndef _OsProcessIteratorFreeBSD_h_
#define _OsProcessIteratorFreeBSD_h_

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

class OsProcessIteratorFreeBSD : OsProcessIteratorBase
{

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

    friend class OsProcessLinux;

/* ============================ CREATORS ================================== */
   OsProcessIteratorFreeBSD();
     //:Default constructor

   OsProcessIteratorFreeBSD(const char* filterExp);

     //:Return processes filtered by name

   virtual ~OsProcessIteratorFreeBSD();
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



#endif /* _OsProcessIteratorFreeBSD_h_ */
