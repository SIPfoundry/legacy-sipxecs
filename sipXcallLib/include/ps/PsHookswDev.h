//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _PsHookswDev_h_
#define _PsHookswDev_h_

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
class PsHookswTask;

//:Base class for the phone set hook switch device
// Platform-specific hook switch classes are all derived from this class.
class PsHookswDev
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   static PsHookswDev* getHookswDev(PsHookswTask* pHookswTask=NULL);
     //:Return a pointer to the hookswitch device, creating it if necessary.

   virtual
   ~PsHookswDev();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   virtual void disableIntr(void) = 0;
     //:Disable hook switch interrupts

   virtual void enableIntr(UtlBoolean lookForOffHook) = 0;
     //:Enable hook switch interrupts

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

   virtual UtlBoolean isOffHook(void) = 0;
     //:Return TRUE if the hookswitch is "off hook", otherwise FALSE.

   virtual UtlBoolean isOnHook(void) = 0;
     //:Return TRUE if the hookswitch is "on hook", otherwise FALSE.

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   static PsHookswDev* spInstance;  // pointer to the single instance of
                                    //  the PsHookswDev class
   static OsBSem       sLock;       // semaphore used to ensure that there
                                    //  is only one instance of this class
   PsHookswTask*       mpHookswTask;

   PsHookswDev(PsHookswTask* pHookswTask);
     //:Constructor

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   PsHookswDev(const PsHookswDev& rPsHookswDev);
     //:Copy constructor (not implemented for this class)

   PsHookswDev& operator=(const PsHookswDev& rhs);
     //:Assignment operator (not implemented for this class)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _PsHookswDev_h_
