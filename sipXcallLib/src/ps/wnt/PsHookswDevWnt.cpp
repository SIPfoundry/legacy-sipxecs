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
#include "ps/wnt/PsHookswDevWnt.h"
#include "ps/PsHookswTask.h"
#include "ps/PsMsg.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// STATIC VARIABLE INITIALIZATIONS
int PsHookswDevWnt::sHookSwitchState = PsHookswTask::ON_HOOK;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor.
// Initialize the interrupt controller for hookswitch interrupt handling
PsHookswDevWnt::PsHookswDevWnt(PsHookswTask* pHookswTask)
:  PsHookswDev(pHookswTask)
{
   // disable interrupt
   disableIntr();

}

// Destructor
PsHookswDevWnt::~PsHookswDevWnt()
{
   // disable the interrupt
   disableIntr();
}

/* ============================ MANIPULATORS ============================== */

// Disable hook switch interrupts
void PsHookswDevWnt::disableIntr(void)
{
}

// Enable hook switch interrupts
void PsHookswDevWnt::enableIntr(UtlBoolean lookForOffHook)
{
}

/* ============================ ACCESSORS ================================= */
void PsHookswDevWnt::setHookState(int hookState)
{
        sHookSwitchState = hookState;
}
/* ============================ INQUIRY =================================== */

// Return TRUE if the hookswitch is "off hook", otherwise FALSE.
UtlBoolean PsHookswDevWnt::isOffHook(void)
{
        return (sHookSwitchState == PsHookswTask::OFF_HOOK );
}

// Return TRUE if the hookswitch is "on hook", otherwise FALSE.
UtlBoolean PsHookswDevWnt::isOnHook(void)
{
        return (sHookSwitchState == PsHookswTask::ON_HOOK );
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
