//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "ptapi/PtComponent.h"
#include "ptapi/PtPhoneGraphicDisplay.h"
#include "ptapi/PtPhoneLamp.h"
#include "ps/PsButtonTask.h"
#include "tao/TaoClientTask.h"
#include "tao/TaoEvent.h"
#include "tao/TaoEventDispatcher.h"
#include "tao/TaoString.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
PtPhoneGraphicDisplay::PtPhoneGraphicDisplay(int type)
: PtPhoneDisplay(type)
{
}

PtPhoneGraphicDisplay::PtPhoneGraphicDisplay(TaoClientTask *pClient, int type)
: PtPhoneDisplay(pClient, type)
{
}

// Copy constructor
PtPhoneGraphicDisplay::PtPhoneGraphicDisplay(const PtPhoneGraphicDisplay& rPtPhoneGraphicDisplay)
: PtPhoneDisplay(rPtPhoneGraphicDisplay)
{
}

// Destructor
PtPhoneGraphicDisplay::~PtPhoneGraphicDisplay()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
PtPhoneGraphicDisplay&
PtPhoneGraphicDisplay::operator=(const PtPhoneGraphicDisplay& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

        return *this;
}


/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
