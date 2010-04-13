//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsSharedLibMgrWnt_h_
#define _OsSharedLibMgrWnt_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <os/OsSharedLibMgr.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class OsSharedLibMgrWnt : public OsSharedLibMgrBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
    friend class OsSharedLibMgrBase;

public:

/* ============================ CREATORS ================================== */

   virtual
   ~OsSharedLibMgrWnt();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   virtual OsStatus loadSharedLib(const char* libName);
   //: Loads the given shared library
   //!param: libName - name of library, may include absolute or relative path

   virtual OsStatus getSharedLibSymbol(const char* libName,
                              const char* symbolName,
                              void*& symbolAddress);
   //: Gets the address of a symbol in the shared lib
   //!param: (in) libName - name of library, may include absolute or relative path
   //!param: (in) symbolName - name of the variable or function exported in the shared lib
   //!param: (out) symbolAddress - the address of the function or variable

   virtual OsStatus unloadSharedLib(const char* libName);
   //: Not yet implemented

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   OsSharedLibMgrWnt();
     //:Default constructor disallowed, use getOsSharedLibMgr

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   OsSharedLibMgrWnt(const OsSharedLibMgrWnt& rOsSharedLibMgrWnt);
     //:Copy constructor

   OsSharedLibMgrWnt& operator=(const OsSharedLibMgrWnt& rhs);
     //:Assignment operator

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsSharedLibMgrWnt_h_
