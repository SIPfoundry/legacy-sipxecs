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
#include "ptapi/PtTerminalComponentListener.h"
#include "ptapi/PtComponentIntChangeEvent.h"
#include "ptapi/PtComponentStringChangeEvent.h"
#include "ptapi/PtTerminalComponentEvent.h"
#include "ptapi/PtEventMask.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

int FORCE_REFERENCE_PtTerminalComponentListener = 0 ;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
PtTerminalComponentListener::PtTerminalComponentListener(const char* name, PtEventMask* pMask) :
PtTerminalListener(name, pMask)
{
}

// Copy constructor
PtTerminalComponentListener::PtTerminalComponentListener(const PtTerminalComponentListener& rPtTerminalComponentListener)
{
}

// Destructor
PtTerminalComponentListener::~PtTerminalComponentListener()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
PtTerminalComponentListener&
PtTerminalComponentListener::operator=(const PtTerminalComponentListener& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

void PtTerminalComponentListener::phoneRingerVolumeChanged(const PtComponentIntChangeEvent& rEvent)
{
}

void PtTerminalComponentListener::phoneRingerPatternChanged(const PtComponentIntChangeEvent& rEvent)
{
}

void PtTerminalComponentListener::phoneRingerInfoChanged(const PtComponentStringChangeEvent& rEvent)
{
}

void PtTerminalComponentListener::phoneSpeakerVolumeChanged(const PtComponentIntChangeEvent& rEvent)
{
}

void PtTerminalComponentListener::phoneMicrophoneGainChanged(const PtComponentIntChangeEvent& rEvent)
{
}

void PtTerminalComponentListener::phoneLampModeChanged(const PtComponentIntChangeEvent& rEvent)
{
}

void PtTerminalComponentListener::phoneButtonInfoChanged(const PtComponentStringChangeEvent& rEvent)
{
}

void PtTerminalComponentListener::phoneButtonUp(const PtTerminalComponentEvent& rEvent)
{
}

void PtTerminalComponentListener::phoneButtonDown(const PtTerminalComponentEvent& rEvent)
{
}

void PtTerminalComponentListener::phoneButtonRepeat(const PtTerminalComponentEvent& rEvent)
{
}

void PtTerminalComponentListener::phoneHookswitchOffhook(const PtTerminalComponentEvent& rEvent)
{
}

void PtTerminalComponentListener::phoneHookswitchOnhook(const PtTerminalComponentEvent& rEvent)
{
}

void PtTerminalComponentListener::phoneDisplayChanged(const PtTerminalComponentEvent& rEvent)
{
}

void PtTerminalComponentListener::phoneHandsetVolumeChanged(const PtComponentIntChangeEvent& rEvent)
{
}

void PtTerminalComponentListener::phoneExtSpeakerConnected(const PtTerminalComponentEvent& rEvent)
{
}

void PtTerminalComponentListener::phoneExtSpeakerDisconnected(const PtTerminalComponentEvent& rEvent)
{
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

PT_IMPLEMENT_CLASS_INFO(PtTerminalComponentListener, PtTerminalListener)

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
