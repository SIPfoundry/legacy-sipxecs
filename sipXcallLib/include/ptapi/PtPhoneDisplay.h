//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _PtPhoneDisplay_h_
#define _PtPhoneDisplay_h_

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
class TaoClientTask;


//:The PtPhoneDisplay class models the display device for a phone.
// It is the parent class for the PtPhoneTextDisplay and PtPhoneGraphicDisplay
// classes.  The interface for this class has not yet been defined.

class PtPhoneDisplay : public PtComponent
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
        PtPhoneDisplay(TaoClientTask *pClient, int type = PtComponent::DISPLAY);

        PtPhoneDisplay(int type = PtComponent::DISPLAY);
         //:Default constructor

   PtPhoneDisplay(const PtPhoneDisplay& rPtPhoneDisplay);
     //:Copy constructor (not implemented for this class)

   PtPhoneDisplay& operator=(const PtPhoneDisplay& rhs);
     //:Assignment operator (not implemented for this class)

   virtual
        ~PtPhoneDisplay();
     //:Destructor


/* ============================ MANIPULATORS ============================== */
        PtStatus setContrast(int level);

/* ============================ ACCESSORS ================================= */
        PtStatus getContrast(int& rLevel, int& rLow, int& rHigh, int& rNominal);

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

#endif  // _PtPhoneDisplay_h_
