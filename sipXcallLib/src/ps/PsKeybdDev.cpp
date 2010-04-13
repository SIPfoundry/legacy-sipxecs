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
#include "ps/PsKeybdDev.h"
#ifdef _VXWORKS
#include "ps/vxw/PsKeybdDevBrutus.h"
#include "ps/vxw/PsKeybdDevTcas.h"
#elif defined(_WIN32)
#include "ps/wnt/PsKeybdDevWnt.h"
#elif defined(__pingtel_on_posix__)
#include "ps/linux/PsKeybdDevLinux.h"
#else
#error Unsupported target platform.
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// STATIC VARIABLE INITIALIZATIONS
PsKeybdDev* PsKeybdDev::spInstance = 0;
OsBSem      PsKeybdDev::sLock(OsBSem::Q_PRIORITY, OsBSem::FULL);

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Return a pointer to the keyboard device, creating it if necessary
PsKeybdDev* PsKeybdDev::getKeybdDev(PsButtonTask* pButtonTask)
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
      assert(pButtonTask != NULL);

      switch (OsUtil::getPlatformType())
      {
#ifdef _VXWORKS
      case OsUtil::PLATFORM_BRUTUS:
         spInstance = new PsKeybdDevBrutus(pButtonTask);
         break;
      case OsUtil::PLATFORM_TCAS1:             // fall through
      case OsUtil::PLATFORM_TCAS2:
      case OsUtil::PLATFORM_TCAS3:
      case OsUtil::PLATFORM_TCAS4:
      case OsUtil::PLATFORM_TCAS5:
      case OsUtil::PLATFORM_TCAS6:
      case OsUtil::PLATFORM_TCAS7:
         spInstance = new PsKeybdDevTcas(pButtonTask);
         break;
#elif defined(_WIN32)
      case OsUtil::PLATFORM_WIN32:
         spInstance = new PsKeybdDevWnt(pButtonTask);
         break;
#elif defined(__pingtel_on_posix__)
      case OsUtil::PLATFORM_LINUX:
      case OsUtil::PLATFORM_SOLARIS:
         spInstance = new PsKeybdDevLinux(pButtonTask);
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
PsKeybdDev::~PsKeybdDev()
{
   spInstance = NULL;
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

// Default constructor (called only indirectly via getKeybdDev())
PsKeybdDev::PsKeybdDev(PsButtonTask* pButtonTask)
:  mpButtonTask(pButtonTask)
{
   // no further initialization is required
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
