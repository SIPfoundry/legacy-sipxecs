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
#include "ps/PsButtonInfo.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
// Default values are provided for all of the arguments so that it is
// possible to allocate an array of PsButtonInfo objects.
PsButtonInfo::PsButtonInfo(int buttonId, const char* name, int eventMask,
                           const OsTime& repeatInterval)
:  mButtonId(buttonId),
   mButtonState(UP),
   mEventMask(eventMask),
   mRepeatInterval(repeatInterval)
{
   // all of the interesting work is done by the initializers
   assert(eventMask & BUTTON_DOWN ||
          eventMask & BUTTON_UP   ||
          eventMask & BUTTON_REPEAT);

   if (name)
   {
           mpButtonName = new char[strlen(name) + 1];
           strcpy(mpButtonName, name);
   }
   else
           mpButtonName = NULL;
}

// Copy constructor
PsButtonInfo::PsButtonInfo(const PsButtonInfo& rPsButtonInfo)
{
   if (rPsButtonInfo.mpButtonName)
   {
           mpButtonName = new char[strlen(rPsButtonInfo.mpButtonName) + 1];
           strcpy(mpButtonName, rPsButtonInfo.mpButtonName);
   }
   else
           mpButtonName = NULL;

   mButtonId       = rPsButtonInfo.mButtonId;
   mButtonState    = rPsButtonInfo.mButtonState;
   mEventMask      = rPsButtonInfo.mEventMask;
   mRepeatInterval = rPsButtonInfo.mRepeatInterval;
}

// Destructor
PsButtonInfo::~PsButtonInfo()
{
        if (mpButtonName)
        {
                delete[] mpButtonName;
                mpButtonName = 0;
        }
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
PsButtonInfo&
PsButtonInfo::operator=(const PsButtonInfo& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   if (mpButtonName)            // free the storage for the old name
      delete[] mpButtonName;

   if (rhs.mpButtonName)
   {
           mpButtonName = new char[strlen(rhs.mpButtonName) + 1];
           strcpy(mpButtonName, rhs.mpButtonName);
   }
   else
           mpButtonName = NULL;

   mButtonId       = rhs.mButtonId;
   mButtonState    = rhs.mButtonState;
   mEventMask      = rhs.mEventMask;
   mRepeatInterval = rhs.mRepeatInterval;

   return *this;
}

// Set the button state to either UP or DOWN
void PsButtonInfo::setState(int buttonState)
{
   assert(buttonState == UP || buttonState == DOWN);
   mButtonState = buttonState;
}

/* ============================ ACCESSORS ================================= */

// Return the set of event types that are being handled for this button
int PsButtonInfo::getEventMask(void) const
{
   return mEventMask;
}

// Return the button ID
int PsButtonInfo::getId(void) const
{
   return mButtonId;
}

// Return the button Name
char* PsButtonInfo::getName(void) const
{
   return mpButtonName;
}

// Get the repeat interval for this button
void PsButtonInfo::getRepInterval(OsTime& repeatIntvl) const
{
   repeatIntvl = mRepeatInterval;
}

// Return the button state (UP or DOWN)
int PsButtonInfo::getState(void) const
{
   return mButtonState;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
