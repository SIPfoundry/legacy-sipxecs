//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _PtPhoneHookswitch_h_
#define _PtPhoneHookswitch_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "ptapi/PtComponent.h"
#include "os/OsTime.h"
#include "os/OsProtectEventMgr.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class PtProvider;
class PtCall;
class TaoClientTask;

//:The PtPhoneHookswitch class models the phone hook switch.

class PtPhoneHookswitch : public PtComponent
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   enum HookswitchState
   {
      ON_HOOK,
      OFF_HOOK
   };
   //!enumcode: OFF_HOOK - The phone is off hook
   //!enumcode: ON_HOOK - The phone is on hook

   PtPhoneHookswitch(PtProvider*& rpProvider);

   virtual
   ~PtPhoneHookswitch();
     //:Destructor

/* ============================ CREATORS ================================== */
   PtPhoneHookswitch();
     //:Default constructor

        PtPhoneHookswitch(TaoClientTask *pClient);

        PtPhoneHookswitch(const PtPhoneHookswitch& rPtPhoneHookswitch);
         //:Copy constructor (not implemented for this class)

        PtPhoneHookswitch& operator=(const PtPhoneHookswitch& rhs);
         //:Assignment operator (not implemented for this class)

/* ============================ MANIPULATORS ============================== */

   virtual PtStatus setHookswitchState(int state);
     //:Sets the state of the hookswitch to either ON_HOOK or OFF_HOOK.
     //!param: state - The state of the hookswitch (either ON_HOOK or OFF_HOOK)
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

/* ============================ ACCESSORS ================================= */

   virtual PtStatus getHookswitchState(int& rState);
     //:Sets <i>rState</i> to reflect the current state of the hook switch.
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus getCall(PtCall& rCall);

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
        int                             mState;
        PtProvider              *mpProvider;

        TaoClientTask   *mpClient;
        PtCall                  *mpCall;

        OsTime          mTimeOut;
/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
        OsProtectEventMgr *mpEventMgr;


};

/* ============================ INLINE METHODS ============================ */

#endif  // _PtPhoneHookswitch_h_
