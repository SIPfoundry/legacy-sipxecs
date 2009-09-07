//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>
#include <dlfcn.h>

// APPLICATION INCLUDES
#include "os/OsSysLog.h"
#include <os/linux/OsSharedLibMgrLinux.h>
#include "utl/UtlString.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// Private container class for collectable handles to shared libs
class OsSharedLibHandleLinux : public UtlString
{
public:

    OsSharedLibHandleLinux(const char* libName, void* libHandle);
    void* mLibHandle;
};

OsSharedLibHandleLinux::OsSharedLibHandleLinux(const char* libName, void* libHandle) :
UtlString(libName ? libName : "")
{
    mLibHandle = libHandle;
}

// STATIC VARIABLE INITIALIZATIONS


/* //////////////////////////// PUBLIC //////////////////////////////////// */


/* ============================ CREATORS ================================== */

// Constructor
OsSharedLibMgrLinux::OsSharedLibMgrLinux()
{
}

// Copy constructor
OsSharedLibMgrLinux::OsSharedLibMgrLinux(const OsSharedLibMgrLinux& rOsSharedLibMgrLinux)
{
}

// Destructor
OsSharedLibMgrLinux::~OsSharedLibMgrLinux()
{
}

/* ============================ MANIPULATORS ============================== */

//: Loads the given shared library
//!param: libName - name of library, may include absolute or relative path
OsStatus OsSharedLibMgrLinux::loadSharedLib(const char* libName)
{
    OsStatus status = OS_INVALID;

    // Check if we aready have a handle for this lib
    UtlString collectableName(libName ? libName : "");
    sLock.acquire();
    OsSharedLibHandleLinux* collectableLibHandle =
        (OsSharedLibHandleLinux*) mLibraryHandles.find(&collectableName);
    sLock.release();

    // We do not already have a handle for this lib
    if(!collectableLibHandle)
    {
        // Load the shared library
        void* libHandle = dlopen(libName, RTLD_NOW | RTLD_GLOBAL);

        if (!libHandle)
        {
            OsSysLog::add(FAC_KERNEL, PRI_ERR,
                "Failed to load shared library: %s error: %s",
                libName, dlerror());
            status = OS_NOT_FOUND;
        }
        else
        {
            OsSysLog::add(FAC_KERNEL, PRI_DEBUG,
                 "Loaded shared lib \"%s\" handle: %p", libName ? libName : "(null)",
                  libHandle);
            OsSharedLibHandleLinux* collectableHandle =
                new OsSharedLibHandleLinux(libName,
                                           libHandle);

            sLock.acquire();
            mLibraryHandles.insert(collectableHandle);
            sLock.release();

            status = OS_SUCCESS;
        }
    }
    else
    {
        status = OS_SUCCESS;
    }

    return(status);
}

//: Gets the address of a symbol in the shared lib
//!param: (in) libName - name of library, may include absolute or relative path
//!param: (in) symbolName - name of the variable or function exported in the shared lib
//!param: (out) symbolAddress - the address of the function or variable
OsStatus OsSharedLibMgrLinux::getSharedLibSymbol(const char* libName,
                              const char* symbolName,
                              void*& symbolAddress)
{
    OsStatus status = OS_INVALID;
    UtlString collectableName(libName ? libName : "");
    sLock.acquire();
    OsSharedLibHandleLinux* collectableLibHandle =
        (OsSharedLibHandleLinux*) mLibraryHandles.find(&collectableName);

    if(!collectableLibHandle)
    {

        OsSysLog::add(FAC_KERNEL, PRI_DEBUG,
             "OsSharedLibMgrLinux::getSharedLibSymbol library: \"%s\" not loaded yet, attempting to load",
                     collectableName.data());
        sLock.release();
        loadSharedLib(libName);
        sLock.acquire();
        collectableLibHandle =
            (OsSharedLibHandleLinux*) mLibraryHandles.find(&collectableName);
    }

    if(collectableLibHandle)
    {
        // Get a named symbol from the shared library
        symbolAddress = dlsym(collectableLibHandle->mLibHandle,
                              symbolName);

        if (!symbolAddress)
        {
            OsSysLog::add(FAC_KERNEL, PRI_ERR,
                "Failed to find symbol: %s in shared lib: %s error: %s",
                symbolName, libName ? libName : "(null)", dlerror());

            status = OS_NOT_FOUND;
        }
        else
        {
            OsSysLog::add(FAC_KERNEL, PRI_DEBUG,
                "Found symbol: %s in shared lib: %s",
                symbolName, libName ? libName : "(null)");
            status = OS_SUCCESS;
        }

    }
    else
    {
        OsSysLog::add(FAC_KERNEL, PRI_ERR,
                "Could not find or create handle for shared library: \'%s\'",
                libName ? libName : "(null)");
    }

    sLock.release();

    return(status);
}

//: Not yet implemented
OsStatus OsSharedLibMgrLinux::unloadSharedLib(const char* libName)
{
    OsStatus status = OS_NOT_YET_IMPLEMENTED;
    return(status);
}



// Assignment operator
OsSharedLibMgrLinux&
OsSharedLibMgrLinux::operator=(const OsSharedLibMgrLinux& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
