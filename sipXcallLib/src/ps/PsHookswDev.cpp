//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "os/OsUtil.h"
#include "ps/PsHookswDev.h"
#ifdef _VXWORKS
#include "ps/vxw/PsHookswDevVxw.h"
#elif defined(_WIN32)
#include "ps/wnt/PsHookswDevWnt.h"
#elif defined(__pingtel_on_posix__)
#include "ps/linux/PsHookswDevLinux.h"
#else
#error Unsupported target platform.
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// STATIC VARIABLE INITIALIZATIONS
PsHookswDev* PsHookswDev::spInstance = 0;
OsBSem       PsHookswDev::sLock(OsBSem::Q_PRIORITY, OsBSem::FULL);

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Return a pointer to the Hookswitch device, creating it if necessary
PsHookswDev* PsHookswDev::getHookswDev(PsHookswTask* pHookswTask)
{
   // If the object already exists, then just return it
   if (spInstance != NULL)
      return spInstance;

   // If the object does not yet exist, then acquire the lock to ensure that
   // only one instance of the object is created
   sLock.acquire();
   if (spInstance == NULL)
   {
      // we must have a valid hookswitch task pointer before calling the
      // constructor for the hookswitch device
      assert(pHookswTask != NULL);

      switch (OsUtil::getPlatformType())
      {
#ifdef _VXWORKS
      case OsUtil::PLATFORM_BRUTUS:
         // GPIO pin 0, interrupt level 0, on hook state = 1
         spInstance = new PsHookswDevVxw(pHookswTask, 0, 0, 1);
         break;
      case OsUtil::PLATFORM_TCAS1:
         // GPIO pin 19, interrupt level 11, on hook state = 0
         spInstance = new PsHookswDevVxw(pHookswTask, 19, 11, 0);
         break;
      case OsUtil::PLATFORM_TCAS2:
      case OsUtil::PLATFORM_TCAS3:
      case OsUtil::PLATFORM_TCAS4:
      case OsUtil::PLATFORM_TCAS5:
      case OsUtil::PLATFORM_TCAS6:
      case OsUtil::PLATFORM_TCAS7:
         // GPIO pin 1, interrupt level 1, on hook state = 0
         spInstance = new PsHookswDevVxw(pHookswTask, 1, 1, 0);
         break;
#elif defined(_WIN32)
      case OsUtil::PLATFORM_WIN32:
         spInstance = new PsHookswDevWnt(pHookswTask);
         break;
#elif defined(__pingtel_on_posix__)
      case OsUtil::PLATFORM_LINUX:
      case OsUtil::PLATFORM_SOLARIS:
         spInstance = new PsHookswDevLinux(pHookswTask);
         break;
#else
#error Unsupported target platform.
#endif
      default:
         assert(FALSE);
      }
   }
   sLock.release();

   return spInstance;
}

// Destructor
PsHookswDev::~PsHookswDev()
{
   spInstance = NULL;
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

// Default constructor (called only indirectly via getHookswDev())
PsHookswDev::PsHookswDev(PsHookswTask* pHookswTask)
:  mpHookswTask(pHookswTask)
{
   // no further initialization is required
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
