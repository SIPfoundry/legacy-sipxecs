//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _PsTaoLamp_h_
#define _PsTaoLamp_h_

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
class PsTaoButton;

//:The PsTaoLamp class models the keypad and feature buttons.
class PsTaoLamp : public PsTaoComponent
{
   friend class PsPhoneTask;
     // The PsPhoneTask is responsible for creating and destroying
     // all objects derived from the PsTaoComponent class.  No other entity
     // should invoke the constructors or destructors for these classes.

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum IndicatorMode
   {
      MODE_OFF           = 0x00,
      MODE_STEADY        = 0x01,
      MODE_FLASH         = 0x02,
      MODE_FLUTTER       = 0x04,
      MODE_BROKENFLUTTER = 0x08,
      MODE_WINK          = 0x10
   };
   //!enumcode: MODE_OFF - The indicator mode is off
   //!enumcode: MODE_STEADY - The indicator mode is continuously on
   //!enumcode: MODE_FLASH - The indicator mode is slow on and off
   //!enumcode: MODE_FLUTTER - The indicator mode is fast on and off
   //!enumcode: MODE_BROKENFLUTTER - The indicator is the superposition of flash and flutter
   //!enumcode: MODE_WINK - The indicator mode is wink

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

   OsStatus setMode(int mode);
     //:Sets the indicator to one of its supported modes.
     //!retcode: OS_SUCCESS - Success
     //!retcode: OS_INVALID_ARGUMENT - The requested mode is not supported by this indicator
     //!retcode: OS_PROVIDER_UNAVAILABLE - The provider is not available

/* ============================ ACCESSORS ================================= */

   OsStatus getAssociatedPhoneButton(PsTaoButton*& rpButton);
     //:Returns a pointer to the PsTaoButton object associated with this indicator.
     //!param: (out) rpButton - The pointer to the associated button object
     //!retcode: OS_SUCCESS - Success
     //!retcode: OS_PROVIDER_UNAVAILABLE - The provider is not available

   OsStatus getMode(int& rMode);
     //:Sets <i>rMode</i> to the current mode for this indicator,
     //!retcode: OS_SUCCESS - Success
     //!retcode: OS_PROVIDER_UNAVAILABLE - The provider is not available

   OsStatus getSupportedModes(int& rModeMask);
     //:Sets <i>rModeMask</i> to all of the modes that are supported for this indicator.

 /* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
        PsTaoLamp(const UtlString& rComponentName, int componentType);

   PsTaoLamp();
     //:Default constructor

   virtual
   ~PsTaoLamp();
     //:Destructor

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   PsTaoLamp(const PsTaoLamp& rPsTaoLamp);
     //:Copy constructor (not implemented for this class)

   PsTaoLamp& operator=(const PsTaoLamp& rhs);
     //:Assignment operator (not implemented for this class)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _PsTaoLamp_h_
