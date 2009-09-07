//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#ifndef _OsUtilLinux_h_
#define _OsUtilLinux_h_

// SYSTEM INCLUDES
#include <time.h>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "os/OsTime.h"
#include "os/linux/OsLinuxDefs.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
typedef void (*sighandler_t)(int);

// FORWARD DECLARATIONS

//:Static methods that are useful when running on top of Linux
class OsUtilLinux
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

   static int cvtOsPrioToLinuxPrio(const int osPrio);
     //:Convert an abstraction layer task priority to a Linux task priority

   static int cvtLinuxPrioToOsPrio(const int linuxPrio);
     //:Convert a Linux task priority to an abstraction layer task priority

   static void cvtOsTimeToTimespec(OsTime time1, struct timespec * time2);
     //:Convert an OsTime class relative to the current time to a struct
     // timespec relative to epoch

   static sighandler_t signal(int signum, sighandler_t handler);
     //:Replacement for ::signal() that works better

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   OsUtilLinux();
     //:Default constructor (not implemented for this class)
     // We identify this as a protected method so that gcc doesn't complain
     // that the class only defines a private constructor and has no friends.

   virtual
   ~OsUtilLinux();
     //:Destructor (not implemented for this class)
     // We identify this as a protected method so that gcc doesn't complain
     // that the class only defines a private destructor and has no friends.

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   OsUtilLinux(const OsUtilLinux& rOsUtilLinux);
     //:Copy constructor (not implemented for this class)

   OsUtilLinux& operator=(const OsUtilLinux& rhs);
     //:Assignment operator (not implemented for this class)


};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsUtilLinux_h_
