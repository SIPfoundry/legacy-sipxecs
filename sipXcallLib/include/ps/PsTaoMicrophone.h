//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _PsTaoMicrophone_h_
#define _PsTaoMicrophone_h_

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

//:The PsTaoMicrophone class models the keypad and feature buttons.
class PsTaoMicrophone : public PsTaoComponent
{
   friend class PsPhoneTask;
     // The PsPhoneTask is responsible for creating and destroying
     // all objects derived from the PsTaoComponent class.  No other entity
     // should invoke the constructors or destructors for these classes.

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

/* ============================ MANIPULATORS ============================== */

   OsStatus setGain(int gain);
     //:Sets the microphone gain (volume) to a value between OFF and
     //:FULL (inclusive).
     //!param: gain - The microphone gain level
     //!retcode: OS_SUCCESS - Success
     //!retcode: OS_INVALID_ARGUMENT - invalid gain level
     //!retcode: OS_PROVIDER_UNAVAILABLE - The provider is not available

/* ============================ ACCESSORS ================================= */

   OsStatus getGain(int& rGain);
     //:Sets <i>rGain</i> to the current microphone gain level.
     //!retcode: OS_SUCCESS - Success
     //!retcode: OS_PROVIDER_UNAVAILABLE - The provider is not available

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
        PsTaoMicrophone(const UtlString& rComponentName, int componentType);

   PsTaoMicrophone();
     //:Default constructor

   virtual
   ~PsTaoMicrophone();
     //:Destructor

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   int      mGain;     // the gain

   PsTaoMicrophone(const PsTaoMicrophone& rPsTaoMicrophone);
     //:Copy constructor (not implemented for this class)

   PsTaoMicrophone& operator=(const PsTaoMicrophone& rhs);
     //:Assignment operator (not implemented for this class)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _PsTaoMicrophone_h_
