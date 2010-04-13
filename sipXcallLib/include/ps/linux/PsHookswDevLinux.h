//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#ifndef _PsHookswDevLinux_h_
#define _PsHookswDevLinux_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "ps/PsHookswDev.h"
//#include "ps/PsHookswTask.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
class PsHookswTask;

//:Dummy phone set hook switch device support on the Linux platform
class PsHookswDevLinux : public PsHookswDev
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   PsHookswDevLinux(PsHookswTask* pHookswTask);
     //:Constructor
     // Initialize the interrupt controller for hookswitch interrupt handling

   virtual
   ~PsHookswDevLinux();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   virtual void disableIntr(void);
     //:Disable hook switch interrupts

   virtual void enableIntr(UtlBoolean lookForOffHook);
     //:Enable hook switch interrupts

/* ============================ ACCESSORS ================================= */

   static void setHookState(int hookState);
     //:Set the hardware state
     // On NT there is no hardware so it is emulated

/* ============================ INQUIRY =================================== */

   virtual UtlBoolean isOffHook(void);
     //:Return TRUE if the hookswitch is "off hook", otherwise FALSE.

   virtual UtlBoolean isOnHook(void);
     //:Return TRUE if the hookswitch is "on hook", otherwise FALSE.

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   static int sHookSwitchState;

   PsHookswDevLinux(const PsHookswDevLinux& rPsHookswDevLinux);
     //:Copy constructor (not implemented for this class)

   PsHookswDevLinux& operator=(const PsHookswDevLinux& rhs);
     //:Assignment operator (not implemented for this class)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _PsHookswDevLinux_h_
