//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _PsLampDev_h_
#define _PsLampDev_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsBSem.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
class PsLampTask;

//:Base class for the phone set lamp device
// Platform-specific lamp classes are all derived from this class.
class PsLampDev
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   static PsLampDev* getLampDev(PsLampTask* pLampTask);
     //:Return a pointer to the lamp device, creating it if necessary.

   virtual
   ~PsLampDev();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   virtual void lightLamps(unsigned long onLamps) = 0;
     //:Turn on the indicated lamps
     // Each lamp is represented by a bit in the onLamps parameter.

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   static PsLampDev*   spInstance;  // pointer to the single instance of
                                    //  the PsLampDev class
   static OsBSem       sLock;       // semaphore used to ensure that there
                                    //  is only one instance of this class
   PsLampDev(PsLampTask* pLampTask=NULL);
     //:Constructor

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   PsLampTask* mpLampTask;

   PsLampDev(const PsLampDev& rPsLampDev);
     //:Copy constructor (not implemented for this class)

   PsLampDev& operator=(const PsLampDev& rhs);
     //:Assignment operator (not implemented for this class)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _PsLampDev_h_
