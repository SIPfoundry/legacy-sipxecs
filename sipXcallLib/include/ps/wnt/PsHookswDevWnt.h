//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _PsHookswDevWnt_h_
#define _PsHookswDevWnt_h_

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

//:Dummy phone set hook switch device support on the Wnt platform
class PsHookswDevWnt : public PsHookswDev
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   PsHookswDevWnt(PsHookswTask* pHookswTask);
     //:Constructor
     // Initialize the interrupt controller for hookswitch interrupt handling

   virtual
   ~PsHookswDevWnt();
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

   PsHookswDevWnt(const PsHookswDevWnt& rPsHookswDevWnt);
     //:Copy constructor (not implemented for this class)

   PsHookswDevWnt& operator=(const PsHookswDevWnt& rhs);
     //:Assignment operator (not implemented for this class)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _PsHookswDevWnt_h_
