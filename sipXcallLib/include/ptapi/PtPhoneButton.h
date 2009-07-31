//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _PtPhoneButton_h_
#define _PtPhoneButton_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "ptapi/PtComponent.h"
#include "os/OsBSem.h"
#include "os/OsProtectEventMgr.h"
// DEFINES
#define MAX_NAME_LENGTH         128

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class PtPhoneLamp;
class PtComponent;
class TaoClientTask;

//:The PtPhoneButton class models keypad and feature buttons. Applications
// may "press" this button via the buttonPress() on the PtPhoneButton object. Each
// button has an identifying piece of information associated with it (e.g.
// the text label on the telephone keypad button). Applications may obtain and set
// this information via the getInfo() and setInfo() methods, repsectively. Additionally,
// there may be a lamp-component associated with a button, obtained via the
// getAssociatedPhoneLamp() method.

class PtPhoneButton : public PtComponent
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
        PtPhoneButton();
         //:Default constructor

        PtPhoneButton(TaoClientTask *pClient, const char* name = 0);

        PtPhoneButton(const PtPhoneButton& rPtPhoneButton);
         //:Copy constructor (not implemented for this class)

        PtPhoneButton& operator=(const PtPhoneButton& rhs);
         //:Assignment operator (not implemented for this class)

        virtual
        ~PtPhoneButton();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

        virtual PtStatus buttonPress(void);
         //:Press this button.
         //!retcode: PT_SUCCESS - Success
         //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

        virtual PtStatus setInfo(char* buttonInfo);
         //:Set the information associated with this button.
         //!param: (in) buttonInfo - The string to associate with this button
         //!retcode: PT_SUCCESS - Success
         //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

        PtStatus setInfo2(char* buttonInfo);
     //:Set the information associated with this button locally.

/* ============================ ACCESSORS ================================= */

   virtual PtStatus getAssociatedPhoneLamp(PtPhoneLamp& rLamp);
     //:Returns a pointer to the PtPhoneLamp object associated with this button.
     //!param: (out) rpLamp - The pointer to the associated lamp object
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   PtStatus getInfo(char* pInfo, int maxLen);
     //:Returns the information associated with this button.
     //!param: (out) pInfo - A pointer to the string associated with this button

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
        char    mpInfo[MAX_NAME_LENGTH + 1];

        TaoClientTask           *mpClient;
        PtPhoneLamp*            mpLamp;

        OsTime          mTimeOut;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
        OsProtectEventMgr *mpEventMgr;



};

/* ============================ INLINE METHODS ============================ */

#endif  // _PtPhoneButton_h_
