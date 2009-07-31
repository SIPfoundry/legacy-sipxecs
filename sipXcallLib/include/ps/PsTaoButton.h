//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _PsTaoButton_h_
#define _PsTaoButton_h_

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
class PsTaoLamp;

//:The PsTaoButton class models the keypad and feature buttons.
class PsTaoButton : public PsTaoComponent
{
   friend class PsPhoneTask;
     // The PsPhoneTask is responsible for creating and destroying
     // all objects derived from the PsTaoComponent class.  No other entity
     // should invoke the constructors or destructors for these classes.

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum ButtonState
   {
      UP,
      DOWN
   };

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

   void buttonDown(void);
     //:Press (and hold) the button down.

   void buttonUp(void);
     //:Release the button.

   void buttonPress(void);
     //:Press the button, effectively buttonDown() followed by buttonUp().

   UtlBoolean setInfo(const UtlString& rInfo);
     //:Sets the information associated with this button.

/* ============================ ACCESSORS ================================= */

   PsTaoLamp* getAssociatedPhoneLamp(void);
     //:Returns a pointer to the lamp associated with this button or NULL if
     //:there is no associated lamp.

   void getInfo(UtlString& rInfo);
     //:Returns the information associated with this button.

/* ============================ INQUIRY =================================== */

   UtlBoolean isButtonDown(void);
     //:Returns TRUE if the button is down, otherwise FALSE.

   UtlBoolean isButtonRepeating(void);
     //:Returns TRUE if the button is repeating (and down), otherwise FALSE.

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
        PsTaoButton(const UtlString& rComponentName, int componentType);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   int        mButtonState;     // button state (up or down)
   UtlString   mButtonInfo;      // button info
   UtlBoolean  mIsRepeating;     // TRUE if button is repeating
   UtlBoolean  mbNotSetBefore;     // TRUE if button info is not yet set
   PsTaoLamp* mpAssocLamp;      // pointer to the associated lamp

   PsTaoButton(const UtlString& rName, const UtlString& rInfo);
     //:Constructor

   virtual
   ~PsTaoButton();
     //:Destructor

   void setAssociatedPhoneLamp(PsTaoLamp& rLamp);

   PsTaoButton();
     //:Default constructor (not implemented for this class)

   PsTaoButton(const PsTaoButton& rPsTaoButton);
     //:Copy constructor (not implemented for this class)

   PsTaoButton& operator=(const PsTaoButton& rhs);
     //:Assignment operator (not implemented for this class)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _PsTaoButton_h_
