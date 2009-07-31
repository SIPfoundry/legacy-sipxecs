//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _PsTaoHookswitch_h_
#define _PsTaoHookswitch_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "ps/PsTaoComponent.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:The PsTaoHookswitch class models the phone hook switch.
class PsTaoHookswitch : public PsTaoComponent
{
   friend class PsPhoneTask;
     // The PsPhoneTask is responsible for creating and destroying
     // all objects derived from the PsTaoComponent class.  No other entity
     // should invoke the constructors or destructors for these classes.

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum HookswitchState
   {
      ON_HOOK,
      OFF_HOOK
   };

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

   void setHookSwitch(int hookswitchState);
     //:Sets the state of the hookswitch to either ON_HOOK or OFF_HOOK.

/* ============================ ACCESSORS ================================= */

   int getHookSwitchState(void);
     //:Returns the current state of the hookswitch.

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
        PsTaoHookswitch(const UtlString& rComponentName, int componentType);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   int mHookState;     // hookswitch state


   virtual
   ~PsTaoHookswitch();
     //:Destructor

   PsTaoHookswitch();
     //:Default constructor (not implemented for this class)

   PsTaoHookswitch(const PsTaoHookswitch& rPsTaoHookswitch);
     //:Copy constructor (not implemented for this class)

   PsTaoHookswitch& operator=(const PsTaoHookswitch& rhs);
     //:Assignment operator (not implemented for this class)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _PsTaoHookswitch_h_
