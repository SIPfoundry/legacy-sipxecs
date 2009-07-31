//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _PtPhoneMicrophone_h_
#define _PtPhoneMicrophone_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
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
class TaoClientTask;

//:The PtPhoneMicrophone class models a phone microphone.

class PtPhoneMicrophone : public PtComponent
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum GainLevel
   {
      OFF    = 0,
      MIDDLE = 5,
      FULL   = 10
   };
   //!enumcode: OFF - The microphone is turned off
   //!enumcode: MIDDLE  - The microphone gain is set to the middle of its range
   //!enumcode: FULL - The microphone gain is set to its maximum level

/* ============================ CREATORS ================================== */
   PtPhoneMicrophone();
     //:Default constructor

   PtPhoneMicrophone(TaoClientTask *pClient);

   PtPhoneMicrophone(const PtPhoneMicrophone& rPtPhoneMicrophone);
     //:Copy constructor (not implemented for this class)

   PtPhoneMicrophone& operator=(const PtPhoneMicrophone& rhs);
     //:Assignment operator (not implemented for this class)

   virtual
   ~PtPhoneMicrophone();
     //:Destructor


/* ============================ MANIPULATORS ============================== */

   virtual PtStatus setGain(int gain);
     //:Sets the microphone gain (volume) to a value between OFF and
     //:FULL (inclusive).
     //!param: gain - The microphone gain level
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_INVALID_ARGUMENT - invalid gain level
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

/* ============================ ACCESSORS ================================= */

   virtual PtStatus getGain(int& rGain);
     //:Sets <i>rGain</i> to the current microphone gain level.
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

        TaoClientTask   *mpClient;

        OsTime          mTimeOut;
/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
        OsProtectEventMgr *mpEventMgr;


};

/* ============================ INLINE METHODS ============================ */

#endif  // _PtPhoneMicrophone_h_
