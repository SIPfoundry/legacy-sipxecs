//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _PtPhoneLamp_h_
#define _PtPhoneLamp_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "ptapi/PtDefs.h"
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
class PtPhoneButton;
class TaoClientTask;

//:The PtPhoneLamp class models phone lamps and other simple indicators.

class PtPhoneLamp : public PtComponent
{
friend class PtPhoneButton;

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
   PtPhoneLamp(const PtPhoneLamp& rPtPhoneLamp);
     //:Copy constructor

   PtPhoneLamp(TaoClientTask *pClient);

   PtPhoneLamp& operator=(const PtPhoneLamp& rhs);
     //:Assignment operator

   PtPhoneLamp();
     //:Default constructor

   virtual
   ~PtPhoneLamp();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   virtual PtStatus setMode(int mode);
     //:Sets the indicator to one of its supported modes.
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_INVALID_ARGUMENT - The requested mode is not supported by this indicator
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

/* ============================ ACCESSORS ================================= */

   virtual PtStatus getAssociatedPhoneButton(PtPhoneButton& rButton);
     //:Returns a pointer to the PtPhoneButton object associated with this indicator.
     //!param: (out) rpButton - The pointer to the associated button object
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus getMode(int& rMode);
     //:Sets <i>rMode</i> to the current mode for this indicator,
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus getSupportedModes(int& rModeMask);
     //:Sets <i>rModeMask</i> to all of the modes that are supported for this indicator.

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

        TaoClientTask   *mpClient;

        OsTime          mTimeOut;

        int mMode;
        int mSupportedModes;
        int mType;

        PtPhoneButton *mpAssociatedButton;
        void setAssociatedButton(PtPhoneButton* pButton);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
        OsProtectEventMgr *mpEventMgr;


};

/* ============================ INLINE METHODS ============================ */

#endif  // _PtPhoneLamp_h_
