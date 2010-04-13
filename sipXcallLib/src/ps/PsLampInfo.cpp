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
#include "os/OsDefs.h"
#include "ps/PsLampInfo.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
// Default values are provided for all of the arguments so that it is
// possible to allocate an array of PsLampInfo objects.
PsLampInfo::PsLampInfo(int lampId, const char* pName, LampMode mode)
:  mLampId(lampId),
   mLampMode(mode)
{
   if (pName)
   {
      mpLampName = new char[strlen(pName) + 1];
           strcpy(mpLampName, pName);
   }
   else
           mpLampName = NULL;
}

// Copy constructor
PsLampInfo::PsLampInfo(const PsLampInfo& rPsLampInfo)
{
   if (rPsLampInfo.mpLampName)
   {
           mpLampName = new char[strlen(rPsLampInfo.mpLampName) + 1];
           strcpy(mpLampName, rPsLampInfo.mpLampName);
   }
   else
   {
           mpLampName = NULL;
   }

   mLampId   = rPsLampInfo.mLampId;
   mLampMode = rPsLampInfo.mLampMode;
}

// Destructor
PsLampInfo::~PsLampInfo()
{
        if (mpLampName)
   {
                delete[] mpLampName;
   }

   mpLampName = NULL;
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
PsLampInfo&
PsLampInfo::operator=(const PsLampInfo& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   if (mpLampName != NULL)
      delete[] mpLampName;

   if (rhs.mpLampName)
   {
           mpLampName = new char[strlen(rhs.mpLampName) + 1];
           strcpy(mpLampName, rhs.mpLampName);
   }
   else
   {
           mpLampName = NULL;
   }

   mLampId   = rhs.mLampId;
   mLampMode = rhs.mLampMode;

   return *this;
}

/* ============================ ACCESSORS ================================= */

// Returns the lamp ID
int PsLampInfo::getId(void) const
{
   return mLampId;
}

// Returns the lamp name
const char* PsLampInfo::getName(void) const
{
   return mpLampName;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

// Set all of the properties for the PsLampInfo object
void PsLampInfo::setInfo(int lampId, LampMode mode, char* pLampName)
{
   mLampId   = lampId;
   mLampMode = mode;

   if (mpLampName != NULL)
      delete[] mpLampName;

        mpLampName = new char[strlen(pLampName) + 1];
        strcpy(mpLampName, pLampName);
}

// Set the lamp mode
void PsLampInfo::setMode(LampMode mode)
{
   mLampMode = mode;
}

// Returns the lamp mode
PsLampInfo::LampMode PsLampInfo::getMode(void) const
{
   return mLampMode;
}
/* ============================ FUNCTIONS ================================= */
