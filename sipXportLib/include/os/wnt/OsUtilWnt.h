//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsUtilWnt_h_
#define _OsUtilWnt_h_

// SYSTEM INCLUDES
#include <windows.h>

// APPLICATION INCLUDES
#include "utl/UtlDefs.h"
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "os/OsTime.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Static methods that are useful when running on top of Window NT
class OsUtilWnt
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

   static OsStatus synchObjAcquire(const HANDLE synchObj,
                                             const OsTime& rTimeout = OsTime::OS_INFINITY);
     //:Block the task until the synch obj is acquired or the timeout expires

   static OsStatus synchObjTryAcquire(const HANDLE synchObj);
     //:Conditionally acquire the synch obj (i.e., don't block)
     // Return OS_BUSY if the synch object is held by some other task

/* ============================ ACCESSORS ================================= */

   static DWORD cvtOsTimeToWntTime(const OsTime& rTimer);
     //:Convert an OsTime to the corresponding number of millisecs for WinNT

   static int cvtOsPrioToWntPrio(const int osPrio);
     //:Convert an abstraction layer task priority to a WinNT thread priority

   static int cvtWntPrioToOsPrio(const int wntPrio);
     //:Convert a WinNT thread priority to an abstraction layer task priority
/* ============================ INQUIRY =================================== */

   static UtlBoolean isOsTimeValid(const OsTime& rTimer);
     //:Verify that the OsTime is >= 0 and representable in msecs

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   OsUtilWnt();
     //:Default constructor (not implemented for this class)

   OsUtilWnt(const OsUtilWnt& rOsUtilWnt);
     //:Copy constructor (not implemented for this class)

   virtual
   ~OsUtilWnt();
     //:Destructor (not implemented for this class)

   OsUtilWnt& operator=(const OsUtilWnt& rhs);
     //:Assignment operator (not implemented for this class)


};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsUtilWnt_h_
