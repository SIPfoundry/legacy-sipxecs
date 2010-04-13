//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _PtPhoneGraphicDisplay_h_
#define _PtPhoneGraphicDisplay_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "ptapi/PtPhoneDisplay.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class TaoClientTask;

//:A PtPhoneGraphicDisplay object represents a display device that is
//:pixel-addressable.
// The interface for this class has not yet been defined.

class PtPhoneGraphicDisplay : public PtPhoneDisplay
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
   PtPhoneGraphicDisplay(int type = PtComponent::GRAPHIC_DISPLAY);
     //:Default constructor

    PtPhoneGraphicDisplay(TaoClientTask *pClient, int type = PtComponent::GRAPHIC_DISPLAY);

   PtPhoneGraphicDisplay(const PtPhoneGraphicDisplay& rPtPhoneGraphicDisplay);
     //:Copy constructor (not implemented for this class)

   PtPhoneGraphicDisplay& operator=(const PtPhoneGraphicDisplay& rhs);
     //:Assignment operator (not implemented for this class)

   virtual
   ~PtPhoneGraphicDisplay();
     //:Destructor


/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */
//   virtual PtStatus getName(char*& rpName);
     //:Returns the name associated with this component.
     //!param: (out) rpName - The reference used to return the name
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

//   virtual PtStatus getType(int& rType);
     //:Returns the type associated with this component.
     //!param: (out) rType - The reference used to return the component type
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:


};

/* ============================ INLINE METHODS ============================ */

#endif  // _PtPhoneGraphicDisplay_h_
