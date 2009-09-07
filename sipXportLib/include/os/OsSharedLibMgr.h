//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsSharedLibMgrBase_h_
#define _OsSharedLibMgrBase_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <os/OsStatus.h>
#include <os/OsBSem.h>
#include "utl/UtlHashBag.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//: Singleton manager class to load shared libraries and access symbols in the libs
// Class detailed description which may extend to multiple lines
class OsSharedLibMgrBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

    static OsSharedLibMgrBase* getOsSharedLibMgr();

/* ============================ CREATORS ================================== */

   virtual
   ~OsSharedLibMgrBase();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   virtual OsStatus loadSharedLib(const char* libName) = 0;
   //: Loads the given shared library
   //!param: libName - name of library, may include absolute or relative path

   virtual OsStatus getSharedLibSymbol(const char* libName,
                              const char* symbolName,
                              void*& symbolAddress) = 0;
   //: Gets the address of a symbol in the shared lib
   //!param: (in) libName - name of library, may include absolute or relative path
   //!param: (in) symbolName - name of the variable or function exported in the shared lib
   //!param: (out) symbolAddress - the address of the function or variable

   virtual OsStatus unloadSharedLib(const char* libName) = 0;
   //: Not yet implemented

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    OsSharedLibMgrBase();
     //:Default constructor disallowed, use getOsSharedLibMgr

    // Static data members used to enforce Singleton behavior
    static OsSharedLibMgrBase* spInstance; // pointer to the single instance of
                                          //  the OsProtectEventMgr class
    static OsBSem sLock; // semaphore used to ensure that there
                         //  is only one instance of this class

    UtlHashBag mLibraryHandles;
    // List of all the shared libraries and the os specific handle for each

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   OsSharedLibMgrBase(const OsSharedLibMgrBase& rOsSharedLibMgrBase);
     //:Copy constructor

   OsSharedLibMgrBase& operator=(const OsSharedLibMgrBase& rhs);
     //:Assignment operator

};

/* ============================ INLINE METHODS ============================ */
#if defined(_WIN32)
#  include "os/wnt/OsSharedLibMgrWnt.h"
   typedef class OsSharedLibMgrWnt OsSharedLibMgr;
#elif defined(_VXWORKS)
#  include "os/vxw/OsSharedLibMgrVxw.h"
   typedef class OsSharedLibMgrVxw OsSharedLibMgr;
#elif defined(__pingtel_on_posix__)
#  include "os/linux/OsSharedLibMgrLinux.h"
   typedef class OsSharedLibMgrLinux OsSharedLibMgr;
#else
#  error Unsupported target platform.
#endif

#endif  // _OsSharedLibMgrBase_h_
