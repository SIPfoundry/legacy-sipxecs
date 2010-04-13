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
#include "ps/PsLampDev.h"
#include "ps/PsLampTask.h"
#ifdef _VXWORKS
#include "ps/vxw/PsLampDevTcas.h"
#elif defined(_WIN32)
#include "ps/wnt/PsLampDevWnt.h"
#elif defined(__pingtel_on_posix__)
#include "ps/linux/PsLampDevLinux.h"
#else
#error Unsupported target platform.
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// STATIC VARIABLE INITIALIZATIONS
PsLampDev* PsLampDev::spInstance = 0;
OsBSem     PsLampDev::sLock(OsBSem::Q_PRIORITY, OsBSem::FULL);

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Return a pointer to the lamp device, creating it if necessary.
PsLampDev* PsLampDev::getLampDev(PsLampTask* pLampTask)
{
   // If the object already exists, then just return it
   if (spInstance != NULL)
      return spInstance;

   // If the object does not yet exist, then acquire the lock to ensure that
   // only one instance of the object is created
   sLock.acquire();
   if (spInstance == NULL)
   {
      switch (OsUtil::getPlatformType())
      {
#ifdef _VXWORKS
      case OsUtil::PLATFORM_TCAS1:             // fall through
      case OsUtil::PLATFORM_TCAS2:
      case OsUtil::PLATFORM_TCAS3:
      case OsUtil::PLATFORM_TCAS4:
      case OsUtil::PLATFORM_TCAS5:
      case OsUtil::PLATFORM_TCAS6:
      case OsUtil::PLATFORM_TCAS7:
         spInstance = new PsLampDevTcas(pLampTask);
         break;
#elif defined(_WIN32)
      case OsUtil::PLATFORM_WIN32:
         spInstance = new PsLampDevWnt(pLampTask);
         break;
#elif defined(__pingtel_on_posix__)
      case OsUtil::PLATFORM_LINUX:
      case OsUtil::PLATFORM_SOLARIS:
         spInstance = new PsLampDevLinux(pLampTask);
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
PsLampDev::~PsLampDev()
{
   spInstance = NULL;
   mpLampTask = NULL;
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

// Default constructor (called only indirectly via getLampDev())
PsLampDev::PsLampDev(PsLampTask* pLampTask)
{
   assert(pLampTask != NULL);
   mpLampTask = pLampTask;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
