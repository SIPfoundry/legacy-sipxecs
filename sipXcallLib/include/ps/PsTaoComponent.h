//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _PsTaoComponent_h_
#define _PsTaoComponent_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "utl/UtlString.h"
#include "os/OsMutex.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:The PsTaoComponent class is the base class for all individual components
//:used by the TAO (Telephony Application Objects layer) to model telephone
//:hardware.  Each distinct component type is derived from this class.
class PsTaoComponent
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum PsComponentType
   {
      BUTTON,
      DISPLAY,
      GRAPHIC_DISPLAY,
      HOOKSWITCH,
      LAMP,
      MICROPHONE,
      RINGER,
      SPEAKER,
      EXTERNAL_SPEAKER
   };

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

   void getName(UtlString& rName);
     //:Returns the name of the component

   int getType(void);
     //:Returns the type of the component, either BUTTON, DISPLAY,
     //:GRAPHIC_DISPLAY, HOOKSWITCH, LAMP, MICROPHONE, RINGER or SPEAKER.

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   OsMutex  mMutex;    // mutex used to synchronize access
   UtlString mName;     // the name of this component
   int      mType;     // the type of this component

   PsTaoComponent(const UtlString& rComponentName, int componentType);
     //:Constructor

   PsTaoComponent();
     //:Default constructor (not implemented for this class)

   virtual
   ~PsTaoComponent();
     //:Destructor

   OsMutex* getMutex(void);
     //:Return the mutex used to synchronize access to an instance of this
     //:class.

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:


   PsTaoComponent(const PsTaoComponent& rPsTaoComponent);
     //:Copy constructor (not implemented for this class)

   PsTaoComponent& operator=(const PsTaoComponent& rhs);
     //:Assignment operator (not implemented for this class)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _PsTaoComponent_h_
