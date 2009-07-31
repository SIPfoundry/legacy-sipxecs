//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _PsButtonInfo_h_
#define _PsButtonInfo_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsTime.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Phone set button information
class PsButtonInfo
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum EventTypes
   {
      UNSPECIFIED   = 0,
      BUTTON_DOWN   = 0x1,
      BUTTON_UP     = 0x2,
      KEY_DOWN      = 0x4,  //a key hit on NT
      KEY_UP        = 0x8,
      BUTTON_REPEAT = 0x16
   };

   enum ButtonState
   {
      UP,
      DOWN
   };

/* ============================ CREATORS ================================== */

   PsButtonInfo(int buttonId=-1,
                                const char* name="",
                                int eventMask=BUTTON_DOWN|BUTTON_UP,
                const OsTime& repeatInterval=OsTime::OS_INFINITY);
     //:Constructor

   PsButtonInfo(const PsButtonInfo& rPsButtonInfo);
     //:Copy constructor

   virtual
   ~PsButtonInfo();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   PsButtonInfo& operator=(const PsButtonInfo& rhs);
     //:Assignment operator

   virtual void setState(int buttonState);
     //:Set the button state to either UP or DOWN

/* ============================ ACCESSORS ================================= */

   virtual int getEventMask(void) const;
     //:Return the set of event types that are being handled for this button

   virtual int getId(void) const;
     //:Return the button ID

   virtual char* getName(void) const;
     //:Return the button name

   virtual void getRepInterval(OsTime& repeatIntvl) const;
     //:Get the repeat interval for this button

   virtual int getState(void) const;
     //:Return the button state (UP or DOWN)

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   char*  mpButtonName;
   int    mButtonId;
   int    mButtonState;
   int    mEventMask;
   OsTime mRepeatInterval;


};

/* ============================ INLINE METHODS ============================ */

#endif  // _PsButtonInfo_h_
