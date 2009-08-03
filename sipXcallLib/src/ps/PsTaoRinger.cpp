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
#include "ps/PsTaoRinger.h"
#include <os/OsLock.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
PsTaoRinger::PsTaoRinger()
{
        mVolume = 0;
}

PsTaoRinger::PsTaoRinger(const UtlString& rComponentName, int componentType) :
PsTaoComponent(rComponentName, componentType)
{
        mVolume = 0;
}

// Copy constructor
PsTaoRinger::PsTaoRinger(const PsTaoRinger& rPsTaoRinger)
{
        mVolume = rPsTaoRinger.mVolume;
}

// Destructor
PsTaoRinger::~PsTaoRinger()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
PsTaoRinger&
PsTaoRinger::operator=(const PsTaoRinger& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

OsStatus PsTaoRinger::setRingerInfo(int patternIndex, char* info)
{
        return OS_SUCCESS;
}

OsStatus PsTaoRinger::setRingerPattern(int patternIndex)
{
        return OS_SUCCESS;
}

OsStatus PsTaoRinger::setRingerVolume(int volume)
{
        if (0 <= volume && volume <= 100)
                mVolume = volume;
        return OS_SUCCESS;
}

/* ============================ ACCESSORS ================================= */

OsStatus PsTaoRinger::getMaxRingPatternIndex(int& rMaxIndex)
{
        return OS_SUCCESS;
}

OsStatus PsTaoRinger::getNumberOfRings(int& rNumRingCycles)
{
        return OS_SUCCESS;
}

OsStatus PsTaoRinger::getRingerInfo(int patternIndex, char*& rpInfo)
{
        return OS_SUCCESS;
}

OsStatus PsTaoRinger::getRingerPattern(int& rPatternIndex)
{
        return OS_SUCCESS;
}

OsStatus PsTaoRinger::getRingerVolume(int& rVolume)
{
        rVolume = mVolume;
        return OS_SUCCESS;
}

/* ============================ INQUIRY =================================== */

OsStatus PsTaoRinger::isRingerOn(UtlBoolean& rIsOn)
{
        rIsOn = mIsRingerOn;
        return OS_SUCCESS;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
